#include "core/res.h"
#include <SDL_image.h>

#ifdef _DEBUG
enum { K_TEX, K_FONT, K_WAV, K_MUS, K_COUNT };
static int live[K_COUNT];
#define TRACK(k, d) (live[k] += (d))
#else
#define TRACK(k, d) ((void)0)
#endif

static SDL_Texture* from_surface(SDL_Renderer* r, SDL_Surface* s, const char* what, int keyBlack) {
	if (!s) {
		SDL_Log("load fail: %s: %s", what, SDL_GetError());
		return NULL;
	}
	if (keyBlack) {
		SDL_SetColorKey(s, SDL_TRUE, SDL_MapRGB(s->format, 0, 0, 0));
	}
	SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
	SDL_FreeSurface(s);
	if (!t) {
		SDL_Log("texture fail: %s: %s", what, SDL_GetError());
		return NULL;
	}
	TRACK(K_TEX, +1);
	return t;
}

SDL_Texture* res_texture(SDL_Renderer* r, const char* path, int keyBlack) {
	return from_surface(r, IMG_Load(path), path, keyBlack);
}

void res_texture_seq(SDL_Renderer* r, SDL_Texture* out[], int n, const char* fmt, int keyBlack) {
	char path[256];
	for (int i = 0; i < n; i++) {
		SDL_snprintf(path, sizeof path, fmt, i + 1);
		out[i] = res_texture(r, path, keyBlack);
	}
}

SDL_Texture* res_text(SDL_Renderer* r, TTF_Font* font, const char* text, SDL_Color color) {
	return from_surface(r, font ? TTF_RenderText_Solid(font, text, color) : NULL, text, 0);
}

void res_destroy_texture(SDL_Texture** t) {
	if (*t) {
		SDL_DestroyTexture(*t);
		TRACK(K_TEX, -1);
		*t = NULL;
	}
}

void res_destroy_seq(SDL_Texture* t[], int n) {
	for (int i = 0; i < n; i++) {
		res_destroy_texture(&t[i]);
	}
}

TTF_Font* res_font(const char* path, int size) {
	TTF_Font* f = TTF_OpenFont(path, size);
	if (!f) {
		SDL_Log("font fail: %s: %s", path, SDL_GetError());
		return NULL;
	}
	TRACK(K_FONT, +1);
	return f;
}

void res_close_font(TTF_Font** f) {
	if (*f) {
		TTF_CloseFont(*f);
		TRACK(K_FONT, -1);
		*f = NULL;
	}
}

Mix_Chunk* res_wav(const char* path, int volume) {
	Mix_Chunk* c = Mix_LoadWAV(path);
	if (!c) {
		SDL_Log("sound fail: %s: %s", path, SDL_GetError());
		return NULL;
	}
	if (volume >= 0) {
		Mix_VolumeChunk(c, volume);
	}
	TRACK(K_WAV, +1);
	return c;
}

void res_free_wav(Mix_Chunk** c) {
	if (*c) {
		Mix_FreeChunk(*c);
		TRACK(K_WAV, -1);
		*c = NULL;
	}
}

Mix_Music* res_mus(const char* path) {
	Mix_Music* m = Mix_LoadMUS(path);
	if (!m) {
		SDL_Log("music fail: %s: %s", path, SDL_GetError());
		return NULL;
	}
	TRACK(K_MUS, +1);
	return m;
}

void res_free_mus(Mix_Music** m) {
	if (*m) {
		Mix_FreeMusic(*m);
		TRACK(K_MUS, -1);
		*m = NULL;
	}
}

void res_counts(char* buf, size_t len) {
#ifdef _DEBUG
	SDL_snprintf(buf, len, "tex %d font %d wav %d mus %d | sdl_alloc %d",
		live[K_TEX], live[K_FONT], live[K_WAV], live[K_MUS], SDL_GetNumAllocations());
#else
	SDL_snprintf(buf, len, "");
#endif
}
