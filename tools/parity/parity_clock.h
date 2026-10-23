#pragma once
/* Forced-included (/FI) into every game source in ParityCheck: both the C reference and the
   C++ port read the same fake clock, so animation frames and timers match exactly. */
#include <SDL.h>
#ifdef __cplusplus
extern "C"
#endif
Uint64 parity_ticks(void);
#define SDL_GetTicks64 parity_ticks
