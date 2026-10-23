"""One-off: export src/game/levels.c grids to Resource/Levels/lvNN.txt, then verify.

Run from the repo root:  python tools/export_levels.py
Exit code 0 and "OK" lines on success.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LEVELS_C = ROOT / "src" / "game" / "levels.c"
OUT = ROOT / "Resource" / "Levels"
ROWS, COLS = 16, 24

LEGEND = """\
# Jungle Run level grid: 16 rows x 24 numbers. Row 0 is the top; 24 columns = one 1200 px screen; each cell 50 px.
# 0 empty | 1 brick | 2 slope | 3 wide platform (3 cells)
# 4 fire trap lv1 | 5 fire trap lv2 | 6 fire trap lv3 (draws 1 cell above)
# 7 ceiling spike, upside down | 20 ceiling spike
# 8 / 9 / 10 falling ceiling slow / medium / fast (draws 2 cells above; solid while down)
# 11 floor saw | 12 ceiling saw
# 13 first aid kit (+1 life) | 14 health potion (5 = good ending) | 21 key (triggers 'onkey' in course.txt)
# 15 / 16 / 17 / 18 lightning super slow / slow / medium / fast (draws 2 cells above)
# 19 small golem (chases the player) | 22 sparkle (decoration)
# Do not put 8-10 or 15-18 in rows 0-1, or 6 in row 0: they would be drawn off screen."""


def read_c_grids():
    text = LEVELS_C.read_text(encoding="utf-8")
    grids = {}
    for name, body in re.findall(r"int (lv\d+)\[MAP_HEIGHT\]\[MAP_WIDTH\] = \{(.*?)\};", text, re.S):
        nums = [int(n) for n in re.findall(r"-?\d+", body)]
        assert len(nums) == ROWS * COLS, (name, len(nums))
        grids["lv%02d" % int(name[2:])] = [nums[r * COLS:(r + 1) * COLS] for r in range(ROWS)]
    assert len(grids) == 16, len(grids)
    return grids


def write_grids(grids):
    OUT.mkdir(parents=True, exist_ok=True)
    for name, rows in grids.items():
        lines = [LEGEND] + [" ".join("%2d" % v for v in row) for row in rows]
        with open(OUT / (name + ".txt"), "w", encoding="utf-8", newline="\r\n") as f:
            f.write("\n".join(lines) + "\n")


def read_txt_grid(path):
    rows = []
    for line in path.read_text(encoding="utf-8-sig").splitlines():
        data = line.split("#", 1)[0].split()
        if data:
            rows.append([int(v) for v in data])
    return rows


def c_course():
    """Python port of drawMap_SingleplayerMode (src/game/map.c)."""
    w3, w = 336.0, 1200.0
    single = {3: "lv00", 5: "lv01", 7: "lv02", 9: "lv03", 11: "lv04", 17: "lv09", 19: "lv10"}
    double = {13: ("lv05", "lv06"), 15: ("lv07", "lv08"), 21: ("lv11", "lv12"), 23: ("lv13", "lv15")}
    ledges, maps = [0.0], []
    for i in range(1, 50):
        prev, add = ledges[i - 1], 0.0
        if i in single:
            add = w
            maps.append((single[i], prev + w3))
        elif i in double:
            add = 2 * w
            maps.append((double[i][0], prev + w3))
            maps.append((double[i][1], prev + w3 + w))
        ledges.append(prev + w3 + add)
    return ledges, maps


def txt_course():
    """Same cursor rules as jr::loadCourse (src/game/LevelLoader.cpp)."""
    cursor, ledges, maps, finish, onkey = 0.0, [], [], None, None
    for line in (OUT / "course.txt").read_text(encoding="utf-8-sig").splitlines():
        t = line.split("#", 1)[0].split()
        if not t:
            continue
        if t[0] == "ground":
            for _ in range(int(t[1])):
                ledges.append(cursor)
                cursor += 336.0
        elif t[0] == "map":
            for name in t[1:]:
                maps.append((name, cursor))
                cursor += 1200.0
        elif t[0] == "onkey":
            onkey = (t[1], t[2])
        elif t[0] == "finish":
            finish = int(t[1])
    return ledges, maps, finish, onkey, cursor


def main():
    grids = read_c_grids()
    write_grids(grids)
    bad = [n for n, rows in grids.items() if read_txt_grid(OUT / (n + ".txt")) != rows]
    print("grids: %d/16 OK" % (16 - len(bad)) + ("" if not bad else "  MISMATCH: " + ", ".join(bad)))

    c_ledges, c_maps = c_course()
    t_ledges, t_maps, finish, onkey, length = txt_course()
    ok = c_ledges == t_ledges and c_maps == t_maps and finish == 25500 and onkey == ("lv15", "lv14")
    print("course: %s (%d grounds, %d maps, length %d)" % ("OK" if ok else "MISMATCH", len(t_ledges), len(t_maps), length))
    if not ok:
        print("C   :", c_ledges, c_maps)
        print("TXT :", t_ledges, t_maps, finish, onkey)
    sys.exit(0 if ok and not bad else 1)


if __name__ == "__main__":
    main()
