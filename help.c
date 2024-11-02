#include "help.h"
#include "menu.h"
#include "structs.h"
#include <stdio.h>
#include <stdbool.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

SDL_Surface* helpSurface, * returnSurface = NULL;
SDL_Texture* helpTexture, * returnTexture = NULL;

SDL_Surface* healthSurface[7] = { NULL };
SDL_Texture* healthTexture[7] = { NULL };

SDL_Surface* goodSurface[8] = { NULL };
SDL_Texture* goodTexture[8] = { NULL };

SDL_Surface* runningSurface[8] = { NULL };
SDL_Texture* runningTexture[8] = { NULL };

SDL_Surface* jumpingSurface[5] = { NULL };
SDL_Texture* jumpingTexture[5] = { NULL };

SDL_Surface* fireSurface, * groundSurface, * lightningSurface, * sawSurface = NULL;
SDL_Texture* fireTexture, * groundTexture, * lightningTexture, * sawTexture = NULL;

SDL_Surface* ceilingSurface = NULL;
SDL_Texture* ceilingTexture = NULL;

TTF_Font* font = NULL;
SDL_Rect returnRect;

SDL_Rect healthRect, goodRect, runningRect, jumpingRect, ceilingRect, fireRect, groundRect, lightningRect, sawRect;

// health object
void getHealthSurface(SDL_Surface* images[]) {
	for (int i = 0; i < 7; i++) {
		char filename[50] = "";
		sprintf_s(filename, sizeof(filename), "Resource\\Menu\\item\\health%d.bmp", i + 1);
		images[i] = SDL_LoadBMP(filename);
		SDL_SetColorKey(images[i], SDL_TRUE, SDL_MapRGB(images[i]->format, 0, 0, 0));
	}
}

// good ending object
void getGoodSurface(SDL_Surface* images[]) {
	for (int i = 0; i < 8; i++) {
		char filename[50] = "";
		sprintf_s(filename, sizeof(filename), "Resource\\Menu\\item\\good%d.bmp", i + 1);
		images[i] = SDL_LoadBMP(filename);
		SDL_SetColorKey(images[i], SDL_TRUE, SDL_MapRGB(images[i]->format, 0, 0, 0));
	}
}


void getRunningSurface(SDL_Surface* images[]) {
	for (int i = 0; i < 8; i++) {
		char filename[50] = "";
		sprintf_s(filename, sizeof(filename), "Resource\\Menu\\running\\run%d.bmp", i + 1);
		//printf("%s\n", filename);
		images[i] = SDL_LoadBMP(filename);
	}
}

void getJumpingSurface(SDL_Surface* images[]) {
	for (int i = 0; i < 5; i++) {
		char filename[50] = "";
		sprintf_s(filename, sizeof(filename), "Resource\\Menu\\jump\\jump%d.bmp", i + 1);
		//printf("%s\n", filename);
		images[i] = SDL_LoadBMP(filename);
	}
}

void getTexture(SDL_Renderer* renderer, SDL_Texture* imageTexture[], SDL_Surface* imageSurface[], int num) {
	for (int i = 0; i < num; i++) {
		imageTexture[i] = SDL_CreateTextureFromSurface(renderer, imageSurface[i]);
	}
}

void clearSurface(SDL_Surface* surface[], int num) {
	for (int i = 0; i < num; i++) {
		surface[i] = NULL;
	}
}

void Render(SDL_Renderer* renderer) {

	SDL_RenderClear(renderer);
	// background
	SDL_RenderCopy(renderer, helpTexture, NULL, NULL);
	// return
	SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
	// object
	SDL_RenderCopy(renderer, lightningTexture, NULL, &lightningRect);
	SDL_RenderCopy(renderer, groundTexture, NULL, &groundRect);
	SDL_RenderCopy(renderer, fireTexture, NULL, &fireRect);
	SDL_RenderCopy(renderer, sawTexture, NULL, &sawRect);

	size_t animation_health = SDL_GetTicks64() / 120;
	int k = animation_health % 7;
	SDL_RenderCopy(renderer, healthTexture[k], NULL, &healthRect);

	size_t animation_good = SDL_GetTicks64() / 120;
	int i = animation_good % 8;
	SDL_RenderCopy(renderer, goodTexture[i], NULL, &goodRect);

	size_t animation_running = SDL_GetTicks64() / 120;
	int j = animation_running % 8;
	SDL_RenderCopy(renderer, runningTexture[j], NULL, &runningRect);

	size_t animation_jumping = SDL_GetTicks64() / 200;
	int h = animation_jumping % 5;
	SDL_RenderCopy(renderer, jumpingTexture[h], NULL, &jumpingRect);

	SDL_RenderCopy(renderer, ceilingTexture, NULL, &ceilingRect);

	SDL_RenderPresent(renderer);

}

void helpDisplay(SDL_Window* window, SDL_Renderer* renderer, GameState* gameState) {

	returnRect.x = 446;
	returnRect.y = 503;

	healthRect.x = 550;
	healthRect.y = 302;
	healthRect.w = 40;
	healthRect.h = 40;

	goodRect.x = 460;
	goodRect.y = 365;
	goodRect.w = 40;
	goodRect.h = 40;

	runningRect.x = 500;
	runningRect.y = 130;
	runningRect.w = 35;
	runningRect.h = 65;

	jumpingRect.x = 430;
	jumpingRect.y = 214;
	jumpingRect.w = 35;
	jumpingRect.h = 65;

	ceilingRect.x = 610;
	ceilingRect.y = 410;
	ceilingRect.w = 60;
	ceilingRect.h = 50;

	fireRect.x = 670;
	fireRect.y = 370;
	fireRect.w = 55;
	fireRect.h = 90;

	sawRect.x = 720;
	sawRect.y = 410;
	sawRect.w = 50;
	sawRect.h = 50;

	groundRect.x = 750;
	groundRect.y = 410;
	groundRect.w = 70;
	groundRect.h = 50;

	lightningRect.x = 793;
	lightningRect.y = 400;
	lightningRect.w = 70;
	lightningRect.h = 60;

	SDL_Color black = { 0, 0 , 0,255 };
	SDL_Color aqua = { 0 , 255 , 255 , 255 };

	Mix_Music* clickSound = Mix_LoadWAV("Resource\\Sound\\Sound Effects\\click.wav");

	helpSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\help.bmp");
	helpTexture = SDL_CreateTextureFromSurface(renderer, helpSurface);
	SDL_RenderCopy(renderer, helpTexture, NULL, NULL);

	// ceiling , fire , saw , ground , lightning 

	fireSurface = SDL_LoadBMP("Resource\\Menu\\traps\\fire.bmp");
	fireTexture = SDL_CreateTextureFromSurface(renderer, fireSurface);

	sawSurface = SDL_LoadBMP("Resource\\Menu\\traps\\saw.bmp");
	sawTexture = SDL_CreateTextureFromSurface(renderer, sawSurface);

	groundSurface = SDL_LoadBMP("Resource\\Menu\\traps\\ground.bmp");
	groundTexture = SDL_CreateTextureFromSurface(renderer, groundSurface);

	lightningSurface = SDL_LoadBMP("Resource\\Menu\\traps\\lightning.bmp");
	lightningTexture = SDL_CreateTextureFromSurface(renderer, lightningSurface);

	ceilingSurface = SDL_LoadBMP("Resource\\Menu\\traps\\ceiling.bmp");
	ceilingTexture = SDL_CreateTextureFromSurface(renderer, ceilingSurface);


	font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
	returnSurface = TTF_RenderText_Solid(font, "Return", black);
	returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
	SDL_QueryTexture(returnTexture, NULL, NULL, &returnRect.w, &returnRect.h);
	SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
	// get surface
	getHealthSurface(healthSurface);
	getGoodSurface(goodSurface);
	getJumpingSurface(jumpingSurface);
	getRunningSurface(runningSurface);

	// get texture
	getTexture(renderer, healthTexture, healthSurface, 7);
	getTexture(renderer, goodTexture, goodSurface, 8);
	getTexture(renderer, runningTexture, runningSurface, 8);
	getTexture(renderer, jumpingTexture, jumpingSurface, 5);

	//clear
	helpSurface = NULL;
	clearSurface(healthSurface, 7);
	clearSurface(runningSurface, 8);
	clearSurface(goodSurface, 8);
	clearSurface(jumpingSurface, 5);

	SDL_Event event;
	bool quit = false;
	while (!quit) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_MOUSEMOTION:
				if (event.motion.x >= returnRect.x && event.motion.x <= returnRect.x + returnRect.w &&
					event.motion.y >= returnRect.y && event.motion.y <= returnRect.y + returnRect.h) {
					SDL_FreeSurface(returnSurface);
					returnSurface = TTF_RenderText_Solid(font, "Return", aqua);
					returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);;
					SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
					SDL_RenderPresent(renderer);
				}
				else {
					SDL_FreeSurface(returnSurface);
					returnSurface = TTF_RenderText_Solid(font, "Return", black);
					returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);;
					SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
					SDL_RenderPresent(renderer);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.x >= returnRect.x && event.button.x <= returnRect.x + returnRect.w &&
					event.button.y >= returnRect.y && event.button.y <= returnRect.y + returnRect.h) {
					Mix_PlayChannel(-1, clickSound, 0);
					SDL_DestroyTexture(helpTexture);
					SDL_DestroyTexture(returnTexture);
					for (int i = 0; i < 7; i++) {
						SDL_DestroyTexture(healthTexture[i]);
					}
					for (int i = 0; i < 8; i++) {
						SDL_DestroyTexture(runningTexture[i]);
					}
					for (int i = 0; i < 8; i++) {
						SDL_DestroyTexture(goodTexture[i]);
					}
					for (int i = 0; i < 5; i++) {
						SDL_DestroyTexture(jumpingTexture[i]);
					}
					SDL_FreeSurface(fireSurface);
					SDL_FreeSurface(groundSurface);
					SDL_FreeSurface(lightningSurface);
					SDL_FreeSurface(sawSurface);
					SDL_FreeSurface(ceilingSurface);
					SDL_DestroyTexture(ceilingTexture);
					SDL_DestroyTexture(fireTexture);
					SDL_DestroyTexture(groundTexture);
					SDL_DestroyTexture(lightningTexture);
					SDL_DestroyTexture(sawTexture);
					SDL_DestroyWindow(window);
					accountDisplay(gameState);
				}
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					SDL_DestroyTexture(helpTexture);
					SDL_DestroyTexture(returnTexture);
					for (int i = 0; i < 7; i++) {
						SDL_DestroyTexture(healthTexture[i]);
					}
					for (int i = 0; i < 8; i++) {
						SDL_DestroyTexture(runningTexture[i]);
					}
					for (int i = 0; i < 8; i++) {
						SDL_DestroyTexture(goodTexture[i]);
					}
					for (int i = 0; i < 5; i++) {
						SDL_DestroyTexture(jumpingTexture[i]);
					}
					SDL_FreeSurface(fireSurface);
					SDL_FreeSurface(groundSurface);
					SDL_FreeSurface(lightningSurface);
					SDL_FreeSurface(sawSurface);
					SDL_DestroyTexture(fireTexture);
					SDL_DestroyTexture(groundTexture);
					SDL_DestroyTexture(lightningTexture);
					SDL_DestroyTexture(sawTexture);
					SDL_DestroyWindow(window);
					accountDisplay(gameState);
				}
				break;
			}
		}
		Render(renderer);
	}
}