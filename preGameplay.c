#include "preGameplay.h"
#include "common_functions.h"
#include "menu.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <SDL_mixer.h>
#include <SDL_image.h>

SDL_Rect underlineRect, inforRect;
SDL_Rect lineRect[7];

TTF_Font* font2 = NULL;

SDL_Surface* blackSurface = NULL;
SDL_Texture* blackTexture = NULL;
SDL_Rect blackRect;

SDL_Surface* whitebarSurface, * blackbarSurface, * inforSurface = NULL;
SDL_Texture* whitebarTexture, * blackbarTexture, * inforTexture = NULL;

SDL_Surface* lineSurface[2] = { NULL };
SDL_Texture* lineTexture[2] = { NULL };

SDL_Color white = { 255 , 255 , 255 , 255 };

char story[7][130] = {
	"\"...In 2016, Chris, who wanted to cure his beloved sister Claire's late-stage cancer, set out for the" ,
	"forest after obtaining the location from a lucky explorer who survived the forest. He hoped to find the" ,
	"treasure in the legend and use its power to save his sister. He crossed through dangerous creatures and" ,
	"reached the relic. He advanced deep into the relic, trying to hide and find a way to defeat the Golem." ,
	"finally. The Golem chased after him and used its power to trap him in an endless loop of being pursued" ,
	"until death. Chris, with no weapons except for the sword he picked up from the relic, had to break the" ,
	"loop and defeat the Golem to reach the treasure.\""
};

void RenderUnderline(SDL_Renderer* renderer, int lineNumber) {

	SDL_Delay(100);
	SDL_RenderCopy(renderer, blackTexture, NULL, &blackRect);
	SDL_RenderCopy(renderer, lineTexture[lineNumber], NULL, &lineRect[lineNumber]);
	SDL_RenderCopy(renderer, inforTexture, NULL, &inforRect);
	SDL_RenderPresent(renderer);

	SDL_Delay(60);

	if (underlineRect.x >= 1000) {
		SDL_RenderCopy(renderer, blackbarTexture, NULL, &underlineRect);
	}
	else
		SDL_RenderCopy(renderer, whitebarTexture, NULL, &underlineRect);

	SDL_RenderPresent(renderer);
}

void UpdateText(SDL_Renderer* renderer, int charNumber, int lineNumber) {
	char temp[150] = { NULL };
	memset(temp, '\0', 150);
	for (int j = 0; j < charNumber; j++) {
		temp[j] = story[lineNumber][j];
	}

	lineSurface[lineNumber] = TTF_RenderText_Solid(font2, temp, white);
	lineTexture[lineNumber] = SDL_CreateTextureFromSurface(renderer, lineSurface[lineNumber]);
	SDL_QueryTexture(lineTexture[lineNumber], NULL, NULL, &lineRect[lineNumber].w, &lineRect[lineNumber].h);
	SDL_FreeSurface(lineSurface[lineNumber]);
}

void PreGameplay(GameState* gameState) {
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

	window = SDL_CreateWindow("Jungle Run", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH_WINDOW, HEIGHT_WINDOW, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	Mix_Music* music = Mix_LoadMUS("Resource\\Sound\\Sound Effects\\typing.mp3");
	Mix_PlayMusic(music, -1);
	Mix_VolumeMusic(64);

	lineRect[0].x = 120;
	lineRect[0].y = 76;

	lineRect[1].x = 120;
	lineRect[1].y = 116;

	lineRect[2].x = 120;
	lineRect[2].y = 156;

	lineRect[3].x = 120;
	lineRect[3].y = 196;

	lineRect[4].x = 120;
	lineRect[4].y = 236;

	lineRect[5].x = 120;
	lineRect[5].y = 276;

	lineRect[6].x = 120;
	lineRect[6].y = 316;

	underlineRect.x = 100;
	underlineRect.y = 100;
	underlineRect.w = 15;
	underlineRect.h = 5;

	blackRect.x = 0;
	blackRect.y = 100;
	blackRect.w = 1200;
	blackRect.h = 800;

	inforRect.x = 290;
	inforRect.y = 500;

	font2 = TTF_OpenFont("Resource\\Fonts\\crazy-pixel.ttf", 80);
	inforSurface = TTF_RenderText_Solid(font2, ">> Press [SPACE] to skip <<", white);
	inforTexture = SDL_CreateTextureFromSurface(renderer, inforSurface);
	SDL_QueryTexture(inforTexture, NULL, NULL, &inforRect.w, &inforRect.h);

	font2 = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 20);
	int charNumber = 0;
	int lineNumber = 0;

	blackSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\bg.bmp");
	blackTexture = SDL_CreateTextureFromSurface(renderer, blackSurface);

	whitebarSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\whitebar.bmp");
	whitebarTexture = SDL_CreateTextureFromSurface(renderer, whitebarSurface);

	blackbarSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\blackbar.bmp");
	blackbarTexture = SDL_CreateTextureFromSurface(renderer, blackbarSurface);

	blackSurface = NULL;
	whitebarSurface = NULL;
	SDL_RenderClear(renderer);
	SDL_Event event;
	bool quit = false;
	while (&quit) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					Mix_HaltMusic();
					SDL_FreeSurface(blackSurface);
					SDL_FreeSurface(whitebarSurface);
					SDL_FreeSurface(blackbarSurface);
					SDL_FreeSurface(inforSurface);
					SDL_DestroyTexture(blackTexture);
					SDL_DestroyTexture(whitebarTexture);
					SDL_DestroyTexture(blackbarTexture);
					SDL_DestroyTexture(inforTexture);
					SDL_RenderClear(renderer);
					SDL_DestroyWindow(window);
					accountDisplay(gameState);
				}
				if (event.key.keysym.sym == SDLK_SPACE) {
					Mix_HaltMusic();
					SDL_FreeSurface(blackSurface);
					SDL_FreeSurface(whitebarSurface);
					SDL_FreeSurface(blackbarSurface);
					SDL_FreeSurface(inforSurface);
					SDL_DestroyTexture(blackTexture);
					SDL_DestroyTexture(whitebarTexture);
					SDL_DestroyTexture(blackbarTexture);
					SDL_DestroyTexture(inforTexture);
					SDL_RenderClear(renderer);
					gamePlay(gameState, renderer, window);
					accountDisplay(gameState);
				}
				break;
			}
		}

		if (charNumber >= strlen(story[lineNumber]) && lineNumber < 7) {
			lineNumber++;
			charNumber = 0;
			underlineRect.y += 40;
			underlineRect.x = 100;
			blackRect.y += 40;
		}
		if (charNumber < strlen(story[lineNumber])) {
			charNumber += 5;
		}
		if (lineNumber < 7) {
			UpdateText(renderer, charNumber, lineNumber);
			underlineRect.x += 44;
			RenderUnderline(renderer, lineNumber);
		}
		if (lineNumber >= 7) {
			Mix_HaltMusic();
			gamePlay(gameState, renderer, window);
			accountDisplay(gameState);
		}
	}
}