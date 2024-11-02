#include "menu.h"
#include "defs.h"
#include "structs.h"
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <string.h>
#include <stdlib.h>

char selectMode[10] = "Easy";   // global variable 
int numMode = 0;
int* ptrNumMode = &numMode;

void accountDisplay(GameState* gameState) {
	if (!gameState->setDifficulty) {
		gameState->setDifficulty = 1;
		gameState->difficulty = DIFFICULTY_EASY;
	}
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	TTF_Font* font = NULL;
	SDL_Surface* menuSurface[6] = { NULL };
	SDL_Texture* menuTexture[6] = { NULL };
	Uint32 time;
	SDL_Surface* bgSurface = NULL;
	SDL_Texture* bgTexture = NULL;

	const char* labels[6] = { "Single Player" , "Story" , "Game Mode" , "Help" , "Donate" , "Exit" };
	bool selected[6] = { 0 };
	SDL_Color red = { 255 , 0 , 0 ,255 };
	SDL_Color black = { 0 , 0 , 0 , 255 };
	SDL_Rect pos[6];

	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048);
	if (!Mix_PlayingMusic()) {
		Mix_Music* music = Mix_LoadMUS("Resource\\Sound\\Soundtrack\\MainMenu2.mp3");
		Mix_VolumeMusic(32);
		Mix_PlayMusic(music, -1);
	}
	window = SDL_CreateWindow("Jungle Run", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 668, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	Mix_Music* clickSound = Mix_LoadWAV("Resource\\Sound\\Sound Effects\\click.wav");

	bgSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\ChooseBG.bmp");
	bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
	SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
	SDL_RenderPresent(renderer);

	font = TTF_OpenFont("Resource\\Fonts\\crazy-pixel.ttf", 50);

	for (int i = 0; i < 6; i++) {
		menuSurface[i] = TTF_RenderText_Solid(font, labels[i], black);
	}

	pos[0].x = 400; // single player
	pos[0].y = 170;

	pos[1].x = 440;  // story
	pos[1].y = 225;

	pos[2].x = 410;  // game mode
	pos[2].y = 280;

	pos[3].x = 450;  // help
	pos[3].y = 335;

	pos[4].x = 430;   //donate
	pos[4].y = 390;

	pos[5].x = 450;  // exit
	pos[5].y = 445;

	for (int i = 0; i < 6; i++) {
		menuTexture[i] = SDL_CreateTextureFromSurface(renderer, menuSurface[i]);
		SDL_QueryTexture(menuTexture[i], NULL, NULL, &pos[i].w, &pos[i].h);
		SDL_RenderCopy(renderer, menuTexture[i], NULL, &pos[i]);
	}

	SDL_RenderPresent(renderer);

	int x, y;
	SDL_Event event;
	bool quit = false;
	while (!quit) {
		time = SDL_GetTicks();
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				for (int i = 0; i < 6; i++) {
					SDL_FreeSurface(menuSurface[i]);
				}
				quit = true;
				exit(EXIT_SUCCESS);
				break;
			case SDL_MOUSEMOTION:
				x = event.motion.x; y = event.motion.y;
				for (int i = 0; i < 6; i++) {
					if (x >= pos[i].x && x <= pos[i].x + pos[i].w &&
						y >= pos[i].y && y <= pos[i].y + pos[i].h) {

						selected[i] = true;
						SDL_FreeSurface(menuSurface[i]);
						menuSurface[i] = TTF_RenderText_Solid(font, labels[i], red);
						menuTexture[i] = SDL_CreateTextureFromSurface(renderer, menuSurface[i]);
						SDL_RenderCopy(renderer, menuTexture[i], NULL, &pos[i]);
						SDL_RenderPresent(renderer);
					}
					else {
						selected[i] = false;
						SDL_FreeSurface(menuSurface[i]);
						menuSurface[i] = TTF_RenderText_Solid(font, labels[i], black);
						menuTexture[i] = SDL_CreateTextureFromSurface(renderer, menuSurface[i]);
						SDL_RenderCopy(renderer, menuTexture[i], NULL, &pos[i]);
						SDL_RenderPresent(renderer);
					}
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				x = event.button.x; y = event.button.y;
				if (x >= pos[5].x && x <= pos[5].x + pos[5].w &&
					y >= pos[5].y && y <= pos[5].y + pos[5].h) {
					Mix_PlayChannel(-1, clickSound, 0);
					SDL_DestroyWindow(window);
					loginDisplay(gameState);
				}
				if (x >= pos[4].x && x <= pos[4].x + pos[4].w &&
					y >= pos[4].y && y <= pos[4].y + pos[4].h) {
					Mix_PlayChannel(-1, clickSound, 0);
					SDL_RenderClear(renderer);
					QRcode(window, renderer, gameState);
				}
				if (x >= pos[2].x && x <= pos[2].x + pos[2].w &&
					y >= pos[2].y && y <= pos[2].y + pos[2].h) {
					Mix_PlayChannel(-1, clickSound, 0);
					SDL_RenderClear(renderer);
					gameMode(window, renderer, selectMode, ptrNumMode, gameState);
				}
				if (x >= pos[1].x && x <= pos[1].x + pos[1].w &&
					y >= pos[1].y && y <= pos[1].y + pos[1].h) {
					Mix_PlayChannel(-1, clickSound, 0);
					SDL_RenderClear(renderer);
					storyDisplay(window, renderer, gameState);
				}
				if (x >= pos[3].x && x <= pos[3].x + pos[3].w &&
					y >= pos[3].y && y <= pos[3].y + pos[3].h) {
					Mix_PlayChannel(-1, clickSound, 0);
					SDL_RenderClear(renderer);
					helpDisplay(window, renderer, gameState);
				}
				if (x >= pos[0].x && x <= pos[0].x + pos[0].w &&
					y >= pos[0].y && y <= pos[0].y + pos[0].h) {
					Mix_PlayChannel(-1, clickSound, 0);
					Mix_HaltMusic();
					SDL_RenderClear(renderer);
					SDL_DestroyWindow(window);
					PreGameplay(gameState);
				}
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					SDL_DestroyWindow(window);
					loginDisplay(gameState);
				}
				break;
			}

		}

	}

	for (int i = 0; i < 6; i++) {
		menuSurface[i] = NULL;
		menuTexture[i] = NULL;
		SDL_FreeSurface(menuSurface[i]);
		SDL_DestroyTexture(menuTexture);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void loginDisplay(GameState* gameState) {

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Surface* imageSurface, * miniSurface = NULL;
    SDL_Texture* imageTexture, * miniTexture = NULL;
    TTF_Font* font = NULL;
    SDL_Surface* textSurface, * textSurface2, * loginSurface, * registerSurface, * temp, * temp2 = NULL;
    SDL_Texture* textTexture, * textTexture2, * loginTexture, * registerTexture, * userTexture, * passTexture = NULL;

    SDL_Surface* arrowSurface, * deleteSurface = NULL;
    SDL_Texture* arrowTexture, * deleteTexture = NULL;

    SDL_Rect deleteRect;
    deleteRect.x = 670;
    deleteRect.y = 275;
    deleteRect.w = 48;
    deleteRect.h = 25;

    SDL_Rect deleteRect2;
    deleteRect2.x = 670;
    deleteRect2.y = 350;
    deleteRect2.w = 42;
    deleteRect2.h = 25;

    SDL_Rect arrowRect;
    arrowRect.x = 670;
    arrowRect.y = 275;
    arrowRect.w = 25;
    arrowRect.h = 25;

    SDL_Rect arrowRect2;
    arrowRect2.x = 670;
    arrowRect2.y = 350;
    arrowRect2.w = 25;
    arrowRect2.h = 25;

    SDL_Surface* trickSurface = NULL;
    SDL_Texture* trickTexture = NULL;
    SDL_Rect trickRect;
    trickRect.x = 262;
    trickRect.y = 388;
    trickRect.w = 442;
    trickRect.h = 51;

    SDL_Surface* informSurface = NULL;
    SDL_Texture* informTexture = NULL;
    SDL_Rect informRect;
    informRect.x = 350;
    informRect.y = 390;

    SDL_Rect informRect2;
    informRect2.x = 378;
    informRect2.y = 390;

    SDL_Surface* bar = NULL;
    SDL_Texture* barTexture = NULL;

    SDL_Rect userRect;
    userRect.x = 313;
    userRect.y = 268;

    SDL_Rect passRect;
    passRect.x = 313;
    passRect.y = 343;

    char password[25] = { 0 };
    char username[21] = { 0 };

    int textLength = 0;
    int passLength = 0;

    int x = 0, y = 0;

    SDL_Color red = { 255  , 0 , 0 , 255 };
    SDL_Color aqua = { 0  , 255 , 255 , 255 };

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    window = SDL_CreateWindow("Jungle Run", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 668, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Mix_Music* clickSound = Mix_LoadWAV("Resource\\Sound\\Sound Effects\\click.wav");

    // load image
    SDL_Rect imageRect;
    imageSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\LoginBG.bmp");
    if (imageSurface == NULL) {
        printf("Error : %s\n", SDL_GetError());
        return 1;
    }

    imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_RenderCopy(renderer, imageTexture, NULL, NULL);
    SDL_RenderPresent(renderer);

    // fix back ground
    trickSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\trick.bmp");
    trickTexture = SDL_CreateTextureFromSurface(renderer, trickSurface);
    SDL_RenderCopy(renderer, trickTexture, NULL, &trickRect);
    SDL_RenderPresent(renderer);

    //fix back arrow
    deleteSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\delete2.bmp");
    deleteTexture = SDL_CreateTextureFromSurface(renderer, deleteSurface);
    SDL_RenderCopy(renderer, deleteTexture, NULL, &deleteRect2);
    SDL_RenderPresent(renderer);

    //fix back arrow
    deleteSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\delete1.bmp");
    deleteTexture = SDL_CreateTextureFromSurface(renderer, deleteSurface);
    SDL_RenderCopy(renderer, deleteTexture, NULL, &deleteRect);
    SDL_RenderPresent(renderer);

    // load text
    SDL_Rect textRect1;
    font = TTF_OpenFont("Resource\\Fonts\\Kanit-MediumItalic.ttf", 24);
    SDL_Color black = { 0,0,0,255 };

    textSurface = NULL;
    textTexture = NULL;
    char textUsername[20] = "Username";
    x = 305;
    y = 238;
    getText(renderer, textSurface, textTexture, font, textUsername, 305, 238, black);

    SDL_Rect textRect2;

    char  textPassword[20] = "Password";
    x = 305;
    y = 313;
    getText(renderer, textSurface, textTexture, font, textPassword, x, y, black);

    SDL_Rect loginRect, registerRect;
    font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);

    loginSurface = TTF_RenderText_Solid(font, "Login", black);
    loginTexture = SDL_CreateTextureFromSurface(renderer, loginSurface);
    SDL_QueryTexture(loginTexture, NULL, NULL, &loginRect.w, &loginRect.h);
    loginRect.x = 362;
    loginRect.y = 439;
    SDL_RenderCopy(renderer, loginTexture, NULL, &loginRect);

    SDL_RenderPresent(renderer);

    registerSurface = TTF_RenderText_Solid(font, "Register", black);
    registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
    SDL_QueryTexture(registerTexture, NULL, NULL, &registerRect.w, &registerRect.h);
    registerRect.x = 523;
    registerRect.y = 439;
    SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
    SDL_RenderPresent(renderer);

    arrowSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\arrow.bmp");

    font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 22);
    SDL_Event event;
    int done = 0;

    SDL_StartTextInput();

    while (!done) {
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
                done = 1;
            }
            else
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        printf("Left mouse button clicked at (%d, %d)\n", event.button.x, event.button.y);
                    }

                    ///// access login

                    SDL_Rect accessLogin = { 309 , 273 , 345 , 28 };
                    SDL_Rect accessPass = { 309 , 348 , 345 , 28 };

                    if (event.button.x >= accessLogin.x && event.button.x <= accessLogin.x + 345 &&
                        event.button.y >= accessLogin.y && event.button.y <= accessLogin.y + 28) {

                        // delete arrow pass
                        deleteSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\delete2.bmp");
                        deleteTexture = SDL_CreateTextureFromSurface(renderer, deleteSurface);
                        SDL_RenderCopy(renderer, deleteTexture, NULL, &deleteRect2);
                        SDL_RenderPresent(renderer);

                        // appear arrow login 
                        arrowTexture = SDL_CreateTextureFromSurface(renderer, arrowSurface);
                        SDL_RenderCopy(renderer, arrowTexture, NULL, &arrowRect);
                        SDL_RenderPresent(renderer);

                        trickSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\trick.bmp");
                        trickTexture = SDL_CreateTextureFromSurface(renderer, trickSurface);
                        SDL_RenderCopy(renderer, trickTexture, NULL, &trickRect);
                        SDL_RenderPresent(renderer);

                        bool outLogin = false;
                        SDL_Event event2;

                        while (!outLogin) {
                            while (SDL_PollEvent(&event2)) {
                                if (event2.type == SDL_QUIT) { outLogin = 1; done = 1; }
                                //
                                if (event2.type == SDL_MOUSEBUTTONDOWN) {
                                    // press register button
                                    if (event2.button.x >= registerRect.x && event2.button.x <= registerRect.x + registerRect.w &&
                                        event2.button.y >= registerRect.y && event2.button.y <= registerRect.y + registerRect.h) {
                                        Mix_PlayChannel(-1, clickSound, 0);
                                        TTF_CloseFont(font);
                                        SDL_FreeSurface(textSurface);
                                        SDL_FreeSurface(imageSurface);
                                        SDL_DestroyRenderer(renderer);
                                        SDL_DestroyWindow(window);
                                        registerDisplay(gameState);
                                    }
                                    // press login button
                                    if (event2.button.x >= loginRect.x && event2.button.x <= loginRect.x + loginRect.w &&
                                        event2.button.y >= loginRect.y && event2.button.y <= loginRect.y + loginRect.h) {

                                        Mix_PlayChannel(-1, clickSound, 0);

                                        font = TTF_OpenFont("Resource\\Fonts\\Kanit-MediumItalic.ttf", 24);
                                        int check = checkRepeatedUsername(username);
                                        printf("%d\n", check);

                                        if (check == 1) {
                                            printf("Wrong Username!\n");
                                            informSurface = TTF_RenderText_Solid(font, "Username does not exist!", (SDL_Color) { 255, 0, 0, 255 });
                                            informTexture = SDL_CreateTextureFromSurface(renderer, informSurface);
                                            SDL_QueryTexture(informTexture, NULL, NULL, &informRect.w, &informRect.h);
                                            SDL_RenderCopy(renderer, informTexture, NULL, &informRect);
                                            SDL_RenderPresent(renderer);

                                            SDL_FreeSurface(informSurface);
                                            SDL_DestroyTexture(informTexture);

                                        }
                                        else if (check == 0) {

                                            int check2 = checkPassword(username, password);
                                            printf("%d\n", check2);
                                            if (check2 == 1) {
                                                printf("Wrong Pass!\n");
                                                informSurface = TTF_RenderText_Solid(font, "Incorrect Password!", (SDL_Color) { 255, 0, 0, 255 });
                                                informTexture = SDL_CreateTextureFromSurface(renderer, informSurface);
                                                SDL_QueryTexture(informTexture, NULL, NULL, &informRect2.w, &informRect2.h);
                                                SDL_RenderCopy(renderer, informTexture, NULL, &informRect2);
                                                SDL_RenderPresent(renderer);

                                            }
                                            else
                                                if (check2 == 0) {
                                                    SDL_DestroyWindow(window);
                                                    accountDisplay(gameState);
                                                }
                                        }
                                    }
                                }
                                //
                                if (event2.type == SDL_MOUSEBUTTONDOWN)
                                    if (event2.button.button == SDL_BUTTON_LEFT)
                                        if (!(event2.button.x >= accessLogin.x && event2.button.x <= accessLogin.x + 345 &&
                                            event2.button.y >= accessLogin.y && event2.button.y <= accessLogin.y + 28)) {
                                            printf("OK\n");
                                            outLogin = true;
                                            break;
                                        }
                                //
                                userTexture = NULL;
                                if (event2.type == SDL_TEXTINPUT || event2.type == SDL_KEYDOWN) {

                                    if (event2.type == SDL_KEYDOWN && event2.key.keysym.sym == SDLK_KP_ENTER) {
                                        printf("OK\n");
                                        outLogin = true;
                                        break;
                                    }
                                    if (event2.type == SDL_KEYDOWN && event2.key.keysym.sym == SDLK_BACKSPACE && strlen(username) > 0) {
                                        printf("You just press BackSpace\n");
                                        textLength -= 1;
                                        username[textLength] = '\0';
                                        printf("%s\n", username);

                                        bar = SDL_LoadBMP("Resource\\Menu\\LoginImage\\bar.bmp");
                                        barTexture = SDL_CreateTextureFromSurface(renderer, bar);
                                        SDL_RenderCopy(renderer, barTexture, NULL, &accessLogin);

                                        SDL_RenderPresent(renderer);

                                        if (userTexture) {
                                            SDL_DestroyTexture(userTexture);
                                        }

                                        temp = TTF_RenderText_Solid(font, username, (SDL_Color) { 255, 255, 255, 255 });
                                        userTexture = SDL_CreateTextureFromSurface(renderer, temp);
                                        SDL_QueryTexture(userTexture, NULL, NULL, &userRect.w, &userRect.h);

                                        SDL_RenderCopy(renderer, userTexture, NULL, &userRect);
                                        SDL_RenderPresent(renderer);

                                        SDL_DestroyTexture(userTexture);
                                        SDL_FreeSurface(temp);

                                    }
                                    else {
                                        if (event2.type == SDL_TEXTINPUT && textLength < 20) {
                                            strcat_s(username, sizeof(username), event2.text.text);
                                            textLength += strlen(event2.text.text);
                                            if (userTexture) {
                                                SDL_DestroyTexture(userTexture);
                                            }

                                            temp = TTF_RenderText_Solid(font, username, (SDL_Color) { 255, 255, 255, 255 });
                                            userTexture = SDL_CreateTextureFromSurface(renderer, temp);
                                            SDL_QueryTexture(userTexture, NULL, NULL, &userRect.w, &userRect.h);

                                            SDL_RenderCopy(renderer, userTexture, NULL, &userRect);
                                            SDL_RenderPresent(renderer);

                                            SDL_DestroyTexture(userTexture);
                                            SDL_FreeSurface(temp);

                                            temp = NULL;
                                        }
                                    }
                                }
                                //
                                if (event2.type == SDL_MOUSEMOTION) {
                                    font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
                                    if (event2.motion.x >= loginRect.x && event2.motion.x <= loginRect.x + loginRect.w &&
                                        event2.motion.y >= loginRect.y && event2.motion.y <= loginRect.y + loginRect.h) {
                                        SDL_FreeSurface(loginSurface);
                                        loginSurface = TTF_RenderText_Solid(font, "Login", aqua);
                                        loginTexture = SDL_CreateTextureFromSurface(renderer, loginSurface);
                                        SDL_RenderCopy(renderer, loginTexture, NULL, &loginRect);
                                        SDL_RenderPresent(renderer);
                                    }
                                    else {
                                        SDL_FreeSurface(loginSurface);
                                        loginSurface = TTF_RenderText_Solid(font, "Login", black);
                                        loginTexture = SDL_CreateTextureFromSurface(renderer, loginSurface);
                                        SDL_RenderCopy(renderer, loginTexture, NULL, &loginRect);
                                        SDL_RenderPresent(renderer);
                                    }
                                    // login
                                    if (event2.motion.x >= registerRect.x && event2.motion.x <= registerRect.x + registerRect.w &&
                                        event2.motion.y >= registerRect.y && event2.motion.y <= registerRect.y + registerRect.h) {
                                        SDL_FreeSurface(registerSurface);
                                        registerSurface = TTF_RenderText_Solid(font, "Register", red);
                                        registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
                                        SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
                                        SDL_RenderPresent(renderer);
                                    }
                                    else {
                                        SDL_FreeSurface(registerSurface);
                                        registerSurface = TTF_RenderText_Solid(font, "Register", black);
                                        registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
                                        SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
                                        SDL_RenderPresent(renderer);
                                    }
                                }
                            }
                        }
                    }

                    // access password
                    if (event.button.x >= accessPass.x && event.button.x <= accessPass.x + 345 &&
                        event.button.y >= accessPass.y && event.button.y <= accessPass.y + 28) {

                        // delete arrow login
                        deleteSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\delete1.bmp");
                        deleteTexture = SDL_CreateTextureFromSurface(renderer, deleteSurface);
                        SDL_RenderCopy(renderer, deleteTexture, NULL, &deleteRect);
                        SDL_RenderPresent(renderer);

                        // appear arrow pass
                        arrowTexture = SDL_CreateTextureFromSurface(renderer, arrowSurface);
                        SDL_RenderCopy(renderer, arrowTexture, NULL, &arrowRect2);
                        SDL_RenderPresent(renderer);

                        trickSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\trick.bmp");
                        trickTexture = SDL_CreateTextureFromSurface(renderer, trickSurface);
                        SDL_RenderCopy(renderer, trickTexture, NULL, &trickRect);
                        SDL_RenderPresent(renderer);

                        SDL_StartTextInput();

                        bool outPass = false;
                        SDL_Event event3;
                        //passLength = 0;

                        while (!outPass) {
                            while (SDL_PollEvent(&event3)) {
                                if (event3.type == SDL_QUIT) { outPass = 1; done = 1; }
                                //
                                if (event3.type == SDL_MOUSEBUTTONDOWN) {
                                    // press register button
                                    if (event3.button.x >= registerRect.x && event3.button.x <= registerRect.x + registerRect.w &&
                                        event3.button.y >= registerRect.y && event3.button.y <= registerRect.y + registerRect.h) {
                                        Mix_PlayChannel(-1, clickSound, 0);
                                        TTF_CloseFont(font);
                                        SDL_FreeSurface(textSurface);
                                        SDL_FreeSurface(imageSurface);
                                        SDL_DestroyRenderer(renderer);
                                        SDL_DestroyWindow(window);
                                        registerDisplay(gameState);
                                    }
                                    // press login button
                                    if (event3.button.x >= loginRect.x && event3.button.x <= loginRect.x + loginRect.w &&
                                        event3.button.y >= loginRect.y && event3.button.y <= loginRect.y + loginRect.h) {

                                        Mix_PlayChannel(-1, clickSound, 0);

                                        font = TTF_OpenFont("Resource\\Fonts\\Kanit-MediumItalic.ttf", 24);
                                        int check = checkRepeatedUsername(username);
                                        printf("%d\n", check);

                                        if (check == 1) {
                                            printf("Wrong Username!\n");
                                            informSurface = TTF_RenderText_Solid(font, "Username does not exist!", (SDL_Color) { 255, 0, 0, 255 });
                                            informTexture = SDL_CreateTextureFromSurface(renderer, informSurface);
                                            SDL_QueryTexture(informTexture, NULL, NULL, &informRect.w, &informRect.h);
                                            SDL_RenderCopy(renderer, informTexture, NULL, &informRect);
                                            SDL_RenderPresent(renderer);

                                            SDL_FreeSurface(informSurface);
                                            SDL_DestroyTexture(informTexture);

                                        }
                                        else if (check == 0) {

                                            int check2 = checkPassword(username, password);
                                            printf("%d\n", check2);
                                            if (check2 == 1) {
                                                printf("Wrong Pass!\n");
                                                informSurface = TTF_RenderText_Solid(font, "Incorrect Password!", (SDL_Color) { 255, 0, 0, 255 });
                                                informTexture = SDL_CreateTextureFromSurface(renderer, informSurface);
                                                SDL_QueryTexture(informTexture, NULL, NULL, &informRect2.w, &informRect2.h);
                                                SDL_RenderCopy(renderer, informTexture, NULL, &informRect2);
                                                SDL_RenderPresent(renderer);

                                            }
                                            else
                                                if (check2 == 0) {
                                                    SDL_DestroyWindow(window);
                                                    accountDisplay(gameState);
                                                }
                                        }
                                    }
                                }
                                //
                                if (event3.type == SDL_MOUSEBUTTONDOWN)
                                    if (event3.button.button == SDL_BUTTON_LEFT)
                                        if (!(event3.button.x >= accessPass.x && event3.button.x <= accessPass.x + 345 &&
                                            event3.button.y >= accessPass.y && event3.button.y <= accessPass.y + 28)) {
                                            printf("OK\n");
                                            outPass = true;
                                            break;
                                        }

                                if (event3.type == SDL_TEXTINPUT || event3.type == SDL_KEYDOWN) {

                                    if (event3.type == SDL_KEYDOWN && event3.key.keysym.sym == SDLK_KP_ENTER) {
                                        printf("OK\n");
                                        outPass = true;
                                        break;
                                    }

                                    if (event3.type == SDL_KEYDOWN && event3.key.keysym.sym == SDLK_BACKSPACE && strlen(password) > 0) {
                                        printf("You just press BackSpace\n");
                                        passLength -= 1;
                                        password[passLength] = '\0';
                                        printf("%s\n", password);

                                        bar = SDL_LoadBMP("Resource\\Menu\\LoginImage\\bar.bmp");
                                        barTexture = SDL_CreateTextureFromSurface(renderer, bar);
                                        SDL_RenderCopy(renderer, barTexture, NULL, &accessPass);

                                        if (passTexture) {
                                            SDL_DestroyTexture(passTexture);
                                        }

                                        temp2 = TTF_RenderText_Solid(font, password, (SDL_Color) { 255, 255, 255, 255 });
                                        passTexture = SDL_CreateTextureFromSurface(renderer, temp2);
                                        SDL_QueryTexture(passTexture, NULL, NULL, &passRect.w, &passRect.h);

                                        SDL_RenderCopy(renderer, passTexture, NULL, &passRect);
                                        SDL_RenderPresent(renderer);

                                        SDL_DestroyTexture(passTexture);
                                        SDL_FreeSurface(temp2);
                                    }
                                    else {
                                        if (event3.type == SDL_TEXTINPUT && passLength < 20) {
                                            strcat_s(password, sizeof(password), event3.text.text);
                                            passLength += strlen(event3.text.text);
                                            if (passTexture) {
                                                SDL_DestroyTexture(passTexture);
                                            }

                                            temp2 = TTF_RenderText_Solid(font, password, (SDL_Color) { 255, 255, 255, 255 });
                                            passTexture = SDL_CreateTextureFromSurface(renderer, temp2);
                                            SDL_QueryTexture(passTexture, NULL, NULL, &passRect.w, &passRect.h);

                                            SDL_RenderCopy(renderer, passTexture, NULL, &passRect);
                                            SDL_RenderPresent(renderer);

                                            SDL_DestroyTexture(passTexture);
                                            SDL_FreeSurface(temp2);

                                            temp2 = NULL;
                                        }
                                    }
                                }
                                //
                                if (event3.type == SDL_MOUSEMOTION) {
                                    font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
                                    if (event3.motion.x >= loginRect.x && event3.motion.x <= loginRect.x + loginRect.w &&
                                        event3.motion.y >= loginRect.y && event3.motion.y <= loginRect.y + loginRect.h) {
                                        SDL_FreeSurface(loginSurface);
                                        loginSurface = TTF_RenderText_Solid(font, "Login", aqua);
                                        loginTexture = SDL_CreateTextureFromSurface(renderer, loginSurface);
                                        SDL_RenderCopy(renderer, loginTexture, NULL, &loginRect);
                                        SDL_RenderPresent(renderer);
                                    }
                                    else {
                                        SDL_FreeSurface(loginSurface);
                                        loginSurface = TTF_RenderText_Solid(font, "Login", black);
                                        loginTexture = SDL_CreateTextureFromSurface(renderer, loginSurface);
                                        SDL_RenderCopy(renderer, loginTexture, NULL, &loginRect);
                                        SDL_RenderPresent(renderer);
                                    }
                                    // login
                                    if (event3.motion.x >= registerRect.x && event3.motion.x <= registerRect.x + registerRect.w &&
                                        event3.motion.y >= registerRect.y && event3.motion.y <= registerRect.y + registerRect.h) {
                                        SDL_FreeSurface(registerSurface);
                                        registerSurface = TTF_RenderText_Solid(font, "Register", red);
                                        registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
                                        SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
                                        SDL_RenderPresent(renderer);
                                    }
                                    else {
                                        SDL_FreeSurface(registerSurface);
                                        registerSurface = TTF_RenderText_Solid(font, "Register", black);
                                        registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
                                        SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
                                        SDL_RenderPresent(renderer);
                                    }
                                }
                                //
                            }
                        }
                    }

                    // press login button
                    if (event.button.x >= loginRect.x && event.button.x <= loginRect.x + loginRect.w &&
                        event.button.y >= loginRect.y && event.button.y <= loginRect.y + loginRect.h) {
                        Mix_PlayChannel(-1, clickSound, 0);
                        font = TTF_OpenFont("Resource\\Fonts\\Kanit-MediumItalic.ttf", 24);
                        int check = checkRepeatedUsername(username);
                        printf("%d\n", check);

                        if (check == 1) {
                            printf("Wrong Username!\n");
                            informSurface = TTF_RenderText_Solid(font, "Username does not exist!", (SDL_Color) { 255, 0, 0, 255 });
                            informTexture = SDL_CreateTextureFromSurface(renderer, informSurface);
                            SDL_QueryTexture(informTexture, NULL, NULL, &informRect.w, &informRect.h);
                            SDL_RenderCopy(renderer, informTexture, NULL, &informRect);
                            SDL_RenderPresent(renderer);

                            SDL_FreeSurface(informSurface);
                            SDL_DestroyTexture(informTexture);

                        }
                        else if (check == 0) {
                            int check2 = checkPassword(username, password);
                            printf("%d\n", check2);
                            if (check2 == 1) {
                                printf("Wrong Pass!\n");
                                informSurface = TTF_RenderText_Solid(font, "Incorrect Password!", (SDL_Color) { 255, 0, 0, 255 });
                                informTexture = SDL_CreateTextureFromSurface(renderer, informSurface);
                                SDL_QueryTexture(informTexture, NULL, NULL, &informRect2.w, &informRect2.h);
                                SDL_RenderCopy(renderer, informTexture, NULL, &informRect2);
                                SDL_RenderPresent(renderer);

                            }
                            else
                                if (check2 == 0) {
                                    SDL_DestroyWindow(window);
                                    accountDisplay(gameState);
                                }
                        }
                    }

                    // press register button
                    if (event.button.x >= registerRect.x && event.button.x <= registerRect.x + registerRect.w &&
                        event.button.y >= registerRect.y && event.button.y <= registerRect.y + registerRect.h) {
                        Mix_PlayChannel(-1, clickSound, 0);
                        TTF_CloseFont(font);
                        SDL_FreeSurface(textSurface);
                        SDL_FreeSurface(imageSurface);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        registerDisplay(gameState);
                    }
                }
                else if (event.type == SDL_MOUSEMOTION) {
                    font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
                    if (event.motion.x >= loginRect.x && event.motion.x <= loginRect.x + loginRect.w &&
                        event.motion.y >= loginRect.y && event.motion.y <= loginRect.y + loginRect.h) {
                        SDL_FreeSurface(loginSurface);
                        loginSurface = TTF_RenderText_Solid(font, "Login", aqua);
                        loginTexture = SDL_CreateTextureFromSurface(renderer, loginSurface);
                        SDL_RenderCopy(renderer, loginTexture, NULL, &loginRect);
                        SDL_RenderPresent(renderer);
                    }
                    else {
                        SDL_FreeSurface(loginSurface);
                        loginSurface = TTF_RenderText_Solid(font, "Login", black);
                        loginTexture = SDL_CreateTextureFromSurface(renderer, loginSurface);
                        SDL_RenderCopy(renderer, loginTexture, NULL, &loginRect);
                        SDL_RenderPresent(renderer);
                    }
                    // login
                    if (event.motion.x >= registerRect.x && event.motion.x <= registerRect.x + registerRect.w &&
                        event.motion.y >= registerRect.y && event.motion.y <= registerRect.y + registerRect.h) {
                        SDL_FreeSurface(registerSurface);
                        registerSurface = TTF_RenderText_Solid(font, "Register", red);
                        registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
                        SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
                        SDL_RenderPresent(renderer);
                    }
                    else {
                        SDL_FreeSurface(registerSurface);
                        registerSurface = TTF_RenderText_Solid(font, "Register", black);
                        registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
                        SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
                        SDL_RenderPresent(renderer);
                    }
                }
            // Mouse Button
        }
        // While running
    }

    SDL_StopTextInput();
    TTF_CloseFont(font);

    SDL_FreeSurface(imageSurface);
    SDL_FreeSurface(textSurface);

    SDL_FreeSurface(loginSurface);
    SDL_FreeSurface(registerSurface);
    SDL_FreeSurface(temp2);
    SDL_FreeSurface(arrowSurface);
    SDL_FreeSurface(deleteSurface);

    SDL_DestroyTexture(loginTexture);
    SDL_DestroyTexture(registerTexture);
    SDL_DestroyTexture(passTexture);
    SDL_DestroyTexture(deleteTexture);

    SDL_DestroyTexture(textTexture);
    SDL_DestroyTexture(imageTexture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();

}

void registerDisplay(GameState* gameState) {
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	TTF_Font* font = NULL;

	SDL_Surface* imageSurface = NULL;
	SDL_Texture* imageTexture = NULL;

	SDL_Surface* textSurface = NULL;
	SDL_Texture* textTexture = NULL;

	SDL_Surface* returnSurface, * registerSurface = NULL;
	SDL_Texture* returnTexture, * registerTexture = NULL;

	SDL_Surface* temp, * temp2, * temp3, * bar = NULL;
	SDL_Texture* barTexture, * userTexture, * passTexture, * repeatTexture = NULL;

	SDL_Surface* informSurface = NULL;
	SDL_Texture* informTexture = NULL;

	SDL_Surface* arrowSurface = NULL;
	SDL_Texture* arrowTexture = NULL;

	SDL_Rect userRect;
	userRect.x = 313;
	userRect.y = 231;

	SDL_Rect passRect;
	passRect.x = 313;
	passRect.y = 305;

	SDL_Rect repeatRect;
	repeatRect.x = 313;
	repeatRect.y = 379;

	SDL_Rect arrowRect1;
	arrowRect1.x = 670;
	arrowRect1.y = 238;
	arrowRect1.w = 24;
	arrowRect1.h = 24;

	SDL_Rect arrowRect2;
	arrowRect2.x = 670;
	arrowRect2.y = 313;
	arrowRect2.w = 24;
	arrowRect2.h = 24;

	SDL_Rect arrowRect3;
	arrowRect3.x = 670;
	arrowRect3.y = 388;
	arrowRect3.w = 24;
	arrowRect3.h = 24;

	SDL_Surface* delete3Surface, * delete4Surface, * delete5Surface = NULL;
	SDL_Texture* delete3Texture, * delete4Texture, * delete5Texture = NULL;
	SDL_Rect delete3Rect, delete4Rect, delete5Rect;

	delete3Rect.x = 665;
	delete3Rect.y = 238;
	delete3Rect.w = 45;
	delete3Rect.h = 24;

	delete4Rect.x = 665;
	delete4Rect.y = 313;
	delete4Rect.w = 43;
	delete4Rect.h = 30;

	delete5Rect.x = 670;
	delete5Rect.y = 388;
	delete5Rect.w = 24;
	delete5Rect.h = 24;

	//SDL_Rect textRect1 , textRect2, textRect3;

	char username[21] = { 0 };
	char password[25] = { 0 };
	char repeatPass[25] = { 0 };

	int lengthUser = 0, lengthPass = 0, lengthRepeat = 0;;

	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

	window = SDL_CreateWindow("Jungle Run", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 668, 0);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	Mix_Music* clickSound = Mix_LoadWAV("Resource\\Sound\\Sound Effects\\click.wav");

	//load background
	imageSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\RegisterBG.bmp");
	if (imageSurface == NULL) {
		printf("Error : %s\n", SDL_GetError());
		return 1;
	}
	imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
	SDL_RenderCopy(renderer, imageTexture, NULL, NULL);
	SDL_RenderPresent(renderer);

	// load static text
	font = TTF_OpenFont("Resource\\Fonts\\Kanit-MediumItalic.ttf", 24);

	SDL_Color black = { 0,0,0,255 }, red = { 255 , 0 , 0 , 255 }, aqua = { 0 , 255 , 255 , 255 };

	// fix background
	SDL_Rect trick2Rect;
	trick2Rect.x = 195;
	trick2Rect.y = 520;
	trick2Rect.w = 580;
	trick2Rect.h = 50; // 46
	imageSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\trick2.bmp");
	imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
	SDL_RenderCopy(renderer, imageTexture, NULL, &trick2Rect);
	SDL_RenderPresent(renderer);

	int x = 0, y = 0;

	char text[20] = "Enter Username";
	x = 305;
	y = 202;
	getText(renderer, textSurface, textTexture, font, text, x, y, black);

	char text2[20] = "Enter Password";
	x = 305;
	y = 276;
	getText(renderer, textSurface, textTexture, font, text2, x, y, black);

	char text3[20] = "Repeat Password";
	x = 305;
	y = 350;
	getText(renderer, textSurface, textTexture, font, text3, x, y, black);

	// load interactive text

	SDL_Rect returnRect, registerRect;

	font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);

	returnSurface = TTF_RenderText_Solid(font, "Return", black);
	returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
	SDL_QueryTexture(returnTexture, NULL, NULL, &returnRect.w, &returnRect.h);
	returnRect.x = 358;
	returnRect.y = 439;
	SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);

	registerSurface = TTF_RenderText_Solid(font, "Register", black);
	registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
	SDL_QueryTexture(registerTexture, NULL, NULL, &registerRect.w, &registerRect.h);
	registerRect.x = 523;
	registerRect.y = 439;
	SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);

	arrowSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\arrow.bmp");
	arrowTexture = SDL_CreateTextureFromSurface(renderer, arrowSurface);

	delete3Surface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\delete3.bmp");
	delete3Texture = SDL_CreateTextureFromSurface(renderer, delete3Surface);
	SDL_RenderCopy(renderer, delete3Texture, NULL, &delete3Rect);

	delete4Surface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\delete4.bmp");
	delete4Texture = SDL_CreateTextureFromSurface(renderer, delete4Surface);
	SDL_RenderCopy(renderer, delete4Texture, NULL, &delete4Rect);

	delete5Surface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\delete5.bmp");
	delete5Texture = SDL_CreateTextureFromSurface(renderer, delete5Surface);
	SDL_RenderCopy(renderer, delete5Texture, NULL, &delete5Rect);

	SDL_RenderPresent(renderer);

	SDL_StartTextInput();

	SDL_Event event;
	bool quit = false;

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
			if (event.type == SDL_QUIT) {
				quit = true;
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					printf("Left mouse button clicked at (%d, %d)\n", event.button.x, event.button.y);
				}

				SDL_Rect accessUser = { 309 , 236 , 345 , 28 };
				SDL_Rect accessPass = { 309 , 310 , 345 , 28 };
				SDL_Rect accessRepeat = { 309 , 384 , 345 , 28 };

				// access Username
				if (event.button.x >= accessUser.x && event.button.x <= accessUser.x + 345 &&
					event.button.y >= accessUser.y && event.button.y <= accessUser.y + 28) {

					SDL_Rect trick2Rect;
					trick2Rect.x = 195;
					trick2Rect.y = 520;
					trick2Rect.w = 580;
					trick2Rect.h = 50; // 46
					imageSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\trick2.bmp");
					imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
					SDL_RenderCopy(renderer, imageTexture, NULL, &trick2Rect);
					SDL_RenderPresent(renderer);

					SDL_RenderCopy(renderer, delete4Texture, NULL, &delete4Rect);
					SDL_RenderPresent(renderer);

					SDL_RenderCopy(renderer, delete5Texture, NULL, &delete5Rect);
					SDL_RenderPresent(renderer);

					SDL_RenderCopy(renderer, arrowTexture, NULL, &arrowRect1);
					SDL_RenderPresent(renderer);

					bool outUser = false;
					SDL_Event event2;

					while (!outUser) {
						while (SDL_PollEvent(&event2)) {

							if (event2.type == SDL_QUIT) {
								outUser = true;
								quit = 1;
							}
							//
							if (event2.type == SDL_MOUSEBUTTONDOWN) {

								if (event2.button.x >= returnRect.x && event2.button.x <= returnRect.x + returnRect.w &&
									event2.button.y >= returnRect.y && event2.button.y <= returnRect.y + returnRect.h) {
									Mix_PlayChannel(-1, clickSound, 0);
									SDL_DestroyWindow(window);
									loginDisplay(gameState);
								}

								if (event2.button.x >= registerRect.x && event2.button.x <= registerRect.x + registerRect.w &&
									event2.button.y >= registerRect.y && event2.button.y <= registerRect.y + registerRect.h) {

									Mix_PlayChannel(-1, clickSound, 0);

									int check1 = checkUsername(username);
									int check2 = checkRepeatedUsername(username);
									int check3 = checkRepeatPassword(password, repeatPass);

									imageSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\trick2.bmp");
									imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
									SDL_RenderCopy(renderer, imageTexture, NULL, &trick2Rect);
									SDL_RenderPresent(renderer);


									font = TTF_OpenFont("Resource\\Fonts\\Kanit-Medium.ttf", 23);
									if (check1 == false) {
										char inform[100] = "Username is not valid!";
										x = 370;
										y = 510;
										getText(renderer, informSurface, informTexture, font, inform, x, y, red);
									}
									else
										if (check2 == 0) {
											//printf("Username has been existed!\n");
											char inform[100] = "Username has been existed!";
											x = 340;
											y = 510;
											getText(renderer, informSurface, informTexture, font, inform, x, y, red);
										}
										else if (strlen(password) == 0) {
											char inform[100] = "Password Mustn't Be Empty!";
											x = 340;
											y = 510;
											getText(renderer, informSurface, informTexture, font, inform, x, y, red);
										}
										else if (check3 == false) {
											//printf("Incorrect Repeated Password!\n");
											char inform[100] = "Incorrect Repeat Password!";
											x = 340;
											y = 510;
											getText(renderer, informSurface, informTexture, font, inform, x, y, red);
										}
										else {
											writeIntoText(username, password);
											char inform[100] = "Successfully Register";
											x = 360;
											y = 510;
											getText(renderer, informSurface, informTexture, font, inform, x, y, red);
										}
								}
							}
							//
							if (event2.type == SDL_MOUSEBUTTONDOWN)
								if (event2.button.button == SDL_BUTTON_LEFT)
									if (!(event2.button.x >= accessUser.x && event2.button.x <= accessUser.x + 345 &&
										event2.button.y >= accessUser.y && event2.button.y <= accessUser.y + 28)) {

										printf("OK\n");
										outUser = true;
										break;

									}
							//
							userTexture = NULL;
							if (event2.type == SDL_TEXTINPUT || event2.type == SDL_KEYDOWN) {

								if (event2.type == SDL_KEYDOWN && event2.key.keysym.sym == SDLK_KP_ENTER) {
									printf("OK\n");
									outUser = true;
									break;
								}

								if (event2.type == SDL_KEYDOWN && event2.key.keysym.sym == SDLK_BACKSPACE && strlen(username) > 0) {
									printf("You just press BackSpace\n");
									lengthUser -= 1;
									username[lengthUser] = '\0';
									printf("%s\n", username);

									bar = SDL_LoadBMP("Resource\\Menu\\LoginImage\\bar.bmp");
									barTexture = SDL_CreateTextureFromSurface(renderer, bar);
									SDL_RenderCopy(renderer, barTexture, NULL, &accessUser);

									SDL_RenderPresent(renderer);

									if (userTexture) {
										SDL_DestroyTexture(userTexture);
									}

									temp = TTF_RenderText_Solid(font, username, (SDL_Color) { 255, 255, 255, 255 });
									userTexture = SDL_CreateTextureFromSurface(renderer, temp);
									SDL_QueryTexture(userTexture, NULL, NULL, &userRect.w, &userRect.h);

									SDL_RenderCopy(renderer, userTexture, NULL, &userRect);
									SDL_RenderPresent(renderer);


									SDL_DestroyTexture(userTexture);
									SDL_FreeSurface(temp);

								}
								else {
									if (event2.type == SDL_TEXTINPUT && lengthUser < 20) {
										strcat_s(username, sizeof(username), event2.text.text);
										lengthUser += strlen(event2.text.text);
										if (userTexture) {
											SDL_DestroyTexture(userTexture);
										}

										temp = TTF_RenderText_Solid(font, username, (SDL_Color) { 255, 255, 255, 255 });
										userTexture = SDL_CreateTextureFromSurface(renderer, temp);
										SDL_QueryTexture(userTexture, NULL, NULL, &userRect.w, &userRect.h);

										SDL_RenderCopy(renderer, userTexture, NULL, &userRect);
										SDL_RenderPresent(renderer);


										SDL_DestroyTexture(userTexture);
										SDL_FreeSurface(temp);

										temp = NULL;
									}
								}
							}
							//
							if (event2.type == SDL_MOUSEMOTION) {
								if (event2.motion.x >= returnRect.x && event2.motion.x <= returnRect.x + returnRect.w &&
									event2.motion.y >= returnRect.y && event2.motion.y <= returnRect.y + returnRect.h) {
									SDL_FreeSurface(returnSurface);
									returnSurface = TTF_RenderText_Solid(font, "Return", aqua);
									returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
									SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
									SDL_RenderPresent(renderer);
								}
								else {
									SDL_FreeSurface(returnSurface);
									returnSurface = TTF_RenderText_Solid(font, "Return", black);
									returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
									SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
									SDL_RenderPresent(renderer);
								}
								//
								if (event2.motion.x >= registerRect.x && event2.motion.x <= registerRect.x + registerRect.w &&
									event2.motion.y >= registerRect.y && event2.motion.y <= registerRect.y + registerRect.h) {
									SDL_FreeSurface(registerSurface);
									registerSurface = TTF_RenderText_Solid(font, "Register", red);
									registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
									SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
									SDL_RenderPresent(renderer);
								}
								else {
									SDL_FreeSurface(registerSurface);
									registerSurface = TTF_RenderText_Solid(font, "Register", black);
									registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
									SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
									SDL_RenderPresent(renderer);
								}

							}
						}
					}
				}

				// access Password
				if (event.button.x >= accessPass.x && event.button.x <= accessPass.x + 345 &&
					event.button.y >= accessPass.y && event.button.y <= accessPass.y + 28) {

					SDL_RenderCopy(renderer, delete3Texture, NULL, &delete3Rect);
					SDL_RenderPresent(renderer);

					SDL_RenderCopy(renderer, delete5Texture, NULL, &delete5Rect);
					SDL_RenderPresent(renderer);

					SDL_RenderCopy(renderer, arrowTexture, NULL, &arrowRect2);
					SDL_RenderPresent(renderer);

					SDL_Rect trick2Rect;
					trick2Rect.x = 195;
					trick2Rect.y = 520;
					trick2Rect.w = 580;
					trick2Rect.h = 50; // 46
					imageSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\trick2.bmp");
					imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
					SDL_RenderCopy(renderer, imageTexture, NULL, &trick2Rect);
					SDL_RenderPresent(renderer);

					bool outPass = false;
					SDL_Event event3;

					while (!outPass) {
						while (SDL_PollEvent(&event3)) {

							if (event3.type == SDL_QUIT) {
								outPass = true;
								quit = 1;
							}

							passTexture = NULL;
							if (event3.type == SDL_MOUSEBUTTONDOWN) {

								if (event3.button.x >= returnRect.x && event3.button.x <= returnRect.x + returnRect.w &&
									event3.button.y >= returnRect.y && event3.button.y <= returnRect.y + returnRect.h) {
									Mix_PlayChannel(-1, clickSound, 0);
									SDL_DestroyWindow(window);
									loginDisplay(gameState);
								}

								if (event3.button.x >= registerRect.x && event3.button.x <= registerRect.x + registerRect.w &&
									event3.button.y >= registerRect.y && event3.button.y <= registerRect.y + registerRect.h) {

									Mix_PlayChannel(-1, clickSound, 0);

									int check1 = checkUsername(username);
									int check2 = checkRepeatedUsername(username);
									int check3 = checkRepeatPassword(password, repeatPass);

									imageSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\trick2.bmp");
									imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
									SDL_RenderCopy(renderer, imageTexture, NULL, &trick2Rect);
									SDL_RenderPresent(renderer);


									font = TTF_OpenFont("Resource\\Fonts\\Kanit-Medium.ttf", 23);
									if (check1 == false) {
										char inform[100] = "Username is not valid!";
										x = 370;
										y = 510;
										getText(renderer, informSurface, informTexture, font, inform, x, y, red);
									}
									else
										if (check2 == 0) {
											//printf("Username has been existed!\n");
											char inform[100] = "Username has been existed!";
											x = 340;
											y = 510;
											getText(renderer, informSurface, informTexture, font, inform, x, y, red);
										}
										else if (strlen(password) == 0) {
											char inform[100] = "Password Mustn't Be Empty!";
											x = 340;
											y = 510;
											getText(renderer, informSurface, informTexture, font, inform, x, y, red);
										}
										else if (check3 == false) {
											//printf("Incorrect Repeated Password!\n");
											char inform[100] = "Incorrect Repeat Password!";
											x = 340;
											y = 510;
											getText(renderer, informSurface, informTexture, font, inform, x, y, red);
										}
										else {
											writeIntoText(username, password);
											char inform[100] = "Successfully Register";
											x = 360;
											y = 510;
											getText(renderer, informSurface, informTexture, font, inform, x, y, red);
										}
								}
							}
							//
							if (event3.type == SDL_MOUSEBUTTONDOWN)
								if (event3.button.button == SDL_BUTTON_LEFT)
									if (!(event3.button.x >= accessPass.x && event3.button.x <= accessPass.x + 345 &&
										event3.button.y >= accessPass.y && event3.button.y <= accessPass.y + 28)) {
										printf("OK\n");
										outPass = true;
										break;
									}

							if (event3.type == SDL_TEXTINPUT || event3.type == SDL_KEYDOWN) {

								if (event3.type == SDL_KEYDOWN && event3.key.keysym.sym == SDLK_KP_ENTER) {
									printf("OK\n");
									outPass = true;
									break;
								}

								if (event3.type == SDL_KEYDOWN && event3.key.keysym.sym == SDLK_BACKSPACE && strlen(password) > 0) {
									printf("You just press BackSpace\n");
									lengthPass -= 1;
									password[lengthPass] = '\0';
									printf("%s\n", password);

									bar = SDL_LoadBMP("Resource\\Menu\\LoginImage\\bar.bmp");
									barTexture = SDL_CreateTextureFromSurface(renderer, bar);
									SDL_RenderCopy(renderer, barTexture, NULL, &accessPass);

									if (passTexture) {
										SDL_DestroyTexture(passTexture);
									}

									temp2 = TTF_RenderText_Solid(font, password, (SDL_Color) { 255, 255, 255, 255 });
									passTexture = SDL_CreateTextureFromSurface(renderer, temp2);
									SDL_QueryTexture(passTexture, NULL, NULL, &passRect.w, &passRect.h);

									SDL_RenderCopy(renderer, passTexture, NULL, &passRect);
									SDL_RenderPresent(renderer);


									SDL_DestroyTexture(passTexture);
									SDL_FreeSurface(temp2);
									//temp2 = NULL;
								}
								else {
									if (event3.type == SDL_TEXTINPUT && lengthPass < 20) {
										strcat_s(password, sizeof(password), event3.text.text);
										lengthPass += strlen(event3.text.text);
										if (passTexture) {
											SDL_DestroyTexture(passTexture);
										}

										temp2 = TTF_RenderText_Solid(font, password, (SDL_Color) { 255, 255, 255, 255 });
										passTexture = SDL_CreateTextureFromSurface(renderer, temp2);
										SDL_QueryTexture(passTexture, NULL, NULL, &passRect.w, &passRect.h);

										SDL_RenderCopy(renderer, passTexture, NULL, &passRect);
										SDL_RenderPresent(renderer);

										SDL_DestroyTexture(passTexture);
										SDL_FreeSurface(temp2);

										temp2 = NULL;
									}
								}
							}
							//
							if (event3.type == SDL_MOUSEMOTION) {

								if (event3.motion.x >= returnRect.x && event3.motion.x <= returnRect.x + returnRect.w &&
									event3.motion.y >= returnRect.y && event3.motion.y <= returnRect.y + returnRect.h) {
									SDL_FreeSurface(returnSurface);
									returnSurface = TTF_RenderText_Solid(font, "Return", aqua);
									returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
									SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
									SDL_RenderPresent(renderer);
								}
								else {
									SDL_FreeSurface(returnSurface);
									returnSurface = TTF_RenderText_Solid(font, "Return", black);
									returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
									SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
									SDL_RenderPresent(renderer);
								}
								//
								if (event3.motion.x >= registerRect.x && event3.motion.x <= registerRect.x + registerRect.w &&
									event3.motion.y >= registerRect.y && event3.motion.y <= registerRect.y + registerRect.h) {
									SDL_FreeSurface(registerSurface);
									registerSurface = TTF_RenderText_Solid(font, "Register", red);
									registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
									SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
									SDL_RenderPresent(renderer);
								}
								else {
									SDL_FreeSurface(registerSurface);
									registerSurface = TTF_RenderText_Solid(font, "Register", black);
									registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
									SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
									SDL_RenderPresent(renderer);
								}
							}
						}
					}
				}

				// access Repeat
				if (event.button.x >= accessRepeat.x && event.button.x <= accessRepeat.x + 345 &&
					event.button.y >= accessRepeat.y && event.button.y <= accessRepeat.y + 28) {

					SDL_RenderCopy(renderer, delete3Texture, NULL, &delete3Rect);
					SDL_RenderPresent(renderer);

					SDL_RenderCopy(renderer, delete4Texture, NULL, &delete4Rect);
					SDL_RenderPresent(renderer);

					SDL_RenderCopy(renderer, arrowTexture, NULL, &arrowRect3);
					SDL_RenderPresent(renderer);

					SDL_Rect trick2Rect;
					trick2Rect.x = 195;
					trick2Rect.y = 520;
					trick2Rect.w = 580;
					trick2Rect.h = 50; // 46
					imageSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\trick2.bmp");
					imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
					SDL_RenderCopy(renderer, imageTexture, NULL, &trick2Rect);
					SDL_RenderPresent(renderer);

					bool outRepeat = false;
					SDL_Event event4;
					//passLength = 0;

					while (!outRepeat) {
						while (SDL_PollEvent(&event4)) {

							if (event4.type == SDL_QUIT) {
								outRepeat = true;
								quit = 1;
							}

							repeatTexture = NULL;
							//
							if (event4.type == SDL_MOUSEBUTTONDOWN) {

								if (event4.button.x >= returnRect.x && event4.button.x <= returnRect.x + returnRect.w &&
									event4.button.y >= returnRect.y && event4.button.y <= returnRect.y + returnRect.h) {
									Mix_PlayChannel(-1, clickSound, 0);
									SDL_DestroyWindow(window);
									loginDisplay(gameState);
								}

								if (event4.button.x >= registerRect.x && event4.button.x <= registerRect.x + registerRect.w &&
									event4.button.y >= registerRect.y && event4.button.y <= registerRect.y + registerRect.h) {

									Mix_PlayChannel(-1, clickSound, 0);

									int check1 = checkUsername(username);
									int check2 = checkRepeatedUsername(username);
									int check3 = checkRepeatPassword(password, repeatPass);

									imageSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\trick2.bmp");
									imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
									SDL_RenderCopy(renderer, imageTexture, NULL, &trick2Rect);
									SDL_RenderPresent(renderer);

									font = TTF_OpenFont("Resource\\Fonts\\Kanit-Medium.ttf", 23);
									if (check1 == false) {
										char inform[100] = "Username is not valid!";
										x = 370;
										y = 510;
										getText(renderer, informSurface, informTexture, font, inform, x, y, red);
									}
									else
										if (check2 == 0) {
											//printf("Username has been existed!\n");
											char inform[100] = "Username has been existed!";
											x = 340;
											y = 510;
											getText(renderer, informSurface, informTexture, font, inform, x, y, red);
										}
										else if (strlen(password) == 0) {
											char inform[100] = "Password Mustn't Be Empty!";
											x = 340;
											y = 510;
											getText(renderer, informSurface, informTexture, font, inform, x, y, red);
										}
										else if (check3 == false) {
											//printf("Incorrect Repeated Password!\n");
											char inform[100] = "Incorrect Repeat Password!";
											x = 340;
											y = 510;
											getText(renderer, informSurface, informTexture, font, inform, x, y, red);
										}
										else {
											writeIntoText(username, password);
											char inform[100] = "Successfully Register";
											x = 360;
											y = 510;
											getText(renderer, informSurface, informTexture, font, inform, x, y, red);
										}
								}
							}
							//
							if (event4.type == SDL_MOUSEBUTTONDOWN)
								if (event4.button.button == SDL_BUTTON_LEFT)
									if (!(event4.button.x >= accessRepeat.x && event4.button.x <= accessRepeat.x + 345 &&
										event4.button.y >= accessRepeat.y && event4.button.y <= accessRepeat.y + 28)) {
										printf("OK\n");
										outRepeat = true;
										break;
									}

							if (event4.type == SDL_TEXTINPUT || event4.type == SDL_KEYDOWN) {

								if (event4.type == SDL_KEYDOWN && event4.key.keysym.sym == SDLK_KP_ENTER) {
									printf("OK\n");
									outRepeat = true;
									break;
								}

								if (event4.type == SDL_KEYDOWN && event4.key.keysym.sym == SDLK_BACKSPACE && strlen(repeatPass) > 0) {
									printf("You just press BackSpace\n");
									lengthRepeat -= 1;
									repeatPass[lengthRepeat] = '\0';
									printf("%s\n", repeatPass);

									bar = SDL_LoadBMP("Resource\\Menu\\LoginImage\\bar.bmp");
									barTexture = SDL_CreateTextureFromSurface(renderer, bar);
									SDL_RenderCopy(renderer, barTexture, NULL, &accessRepeat);

									if (repeatTexture) {
										SDL_DestroyTexture(repeatTexture);
									}

									temp3 = TTF_RenderText_Solid(font, repeatPass, (SDL_Color) { 255, 255, 255, 255 });
									repeatTexture = SDL_CreateTextureFromSurface(renderer, temp3);
									SDL_QueryTexture(repeatTexture, NULL, NULL, &repeatRect.w, &repeatRect.h);

									SDL_RenderCopy(renderer, repeatTexture, NULL, &repeatRect);
									SDL_RenderPresent(renderer);


									SDL_DestroyTexture(repeatTexture);
									SDL_FreeSurface(temp3);
									//temp2 = NULL;
								}
								else {
									if (event4.type == SDL_TEXTINPUT && lengthRepeat < 20) {
										strcat_s(repeatPass, sizeof(repeatPass), event4.text.text);
										lengthRepeat += strlen(event4.text.text);
										if (repeatTexture) {
											SDL_DestroyTexture(repeatTexture);
										}

										temp3 = TTF_RenderText_Solid(font, repeatPass, (SDL_Color) { 255, 255, 255, 255 });
										repeatTexture = SDL_CreateTextureFromSurface(renderer, temp3);
										SDL_QueryTexture(repeatTexture, NULL, NULL, &repeatRect.w, &repeatRect.h);

										SDL_RenderCopy(renderer, repeatTexture, NULL, &repeatRect);
										SDL_RenderPresent(renderer);


										SDL_DestroyTexture(repeatTexture);
										SDL_FreeSurface(temp3);

										temp3 = NULL;
									}
								}
							}
							//
							if (event4.type == SDL_MOUSEMOTION) {
								if (event4.motion.x >= returnRect.x && event4.motion.x <= returnRect.x + returnRect.w &&
									event4.motion.y >= returnRect.y && event4.motion.y <= returnRect.y + returnRect.h) {
									SDL_FreeSurface(returnSurface);
									returnSurface = TTF_RenderText_Solid(font, "Return", aqua);
									returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
									SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
									SDL_RenderPresent(renderer);
								}
								else {
									SDL_FreeSurface(returnSurface);
									returnSurface = TTF_RenderText_Solid(font, "Return", black);
									returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
									SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
									SDL_RenderPresent(renderer);
								}
								//
								if (event4.motion.x >= registerRect.x && event4.motion.x <= registerRect.x + registerRect.w &&
									event4.motion.y >= registerRect.y && event4.motion.y <= registerRect.y + registerRect.h) {
									SDL_FreeSurface(registerSurface);
									registerSurface = TTF_RenderText_Solid(font, "Register", red);
									registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
									SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
									SDL_RenderPresent(renderer);
								}
								else {
									SDL_FreeSurface(registerSurface);
									registerSurface = TTF_RenderText_Solid(font, "Register", black);
									registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
									SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
									SDL_RenderPresent(renderer);
								}

							}
						}
					}
				}

				// click return 
				if (event.button.x >= returnRect.x && event.button.x <= returnRect.x + returnRect.w &&
					event.button.y >= returnRect.y && event.button.y <= returnRect.y + returnRect.h) {
					Mix_PlayChannel(-1, clickSound, 0);
					SDL_DestroyWindow(window);
					loginDisplay(gameState);
				}
				// click register -> write into txt
				if (event.button.x >= registerRect.x && event.button.x <= registerRect.x + registerRect.w &&
					event.button.y >= registerRect.y && event.button.y <= registerRect.y + registerRect.h) {

					Mix_PlayChannel(-1, clickSound, 0);

					int check1 = checkUsername(username);
					int check2 = checkRepeatedUsername(username);
					int check3 = checkRepeatPassword(password, repeatPass);

					imageSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\trick2.bmp");
					imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
					SDL_RenderCopy(renderer, imageTexture, NULL, &trick2Rect);
					SDL_RenderPresent(renderer);


					font = TTF_OpenFont("Resource\\Fonts\\Kanit-Medium.ttf", 23);
					if (check1 == false) {
						char inform[100] = "Username is not valid!";
						x = 370;
						y = 510;
						getText(renderer, informSurface, informTexture, font, inform, x, y, red);
					}
					else
						if (check2 == 0) {
							//printf("Username has been existed!\n");
							char inform[100] = "Username has been existed!";
							x = 340;
							y = 510;
							getText(renderer, informSurface, informTexture, font, inform, x, y, red);
						}
						else if (strlen(password) == 0) {
							char inform[100] = "Password Mustn't Be Empty!";
							x = 340;
							y = 510;
							getText(renderer, informSurface, informTexture, font, inform, x, y, red);
						}
						else if (check3 == false) {
							//printf("Incorrect Repeated Password!\n");
							char inform[100] = "Incorrect Repeat Password!";
							x = 340;
							y = 510;
							getText(renderer, informSurface, informTexture, font, inform, x, y, red);
						}
						else {
							writeIntoText(username, password);
							char inform[100] = "Successfully Register";
							x = 360;
							y = 510;
							getText(renderer, informSurface, informTexture, font, inform, x, y, red);
						}
				}

			}
			else if (event.type == SDL_MOUSEMOTION) {
				if (event.motion.x >= returnRect.x && event.motion.x <= returnRect.x + returnRect.w &&
					event.motion.y >= returnRect.y && event.motion.y <= returnRect.y + returnRect.h) {
					SDL_FreeSurface(returnSurface);
					returnSurface = TTF_RenderText_Solid(font, "Return", aqua);
					returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
					SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
					SDL_RenderPresent(renderer);
				}
				else {
					SDL_FreeSurface(returnSurface);
					returnSurface = TTF_RenderText_Solid(font, "Return", black);
					returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
					SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
					SDL_RenderPresent(renderer);
				}
				//
				if (event.motion.x >= registerRect.x && event.motion.x <= registerRect.x + registerRect.w &&
					event.motion.y >= registerRect.y && event.motion.y <= registerRect.y + registerRect.h) {
					SDL_FreeSurface(registerSurface);
					registerSurface = TTF_RenderText_Solid(font, "Register", red);
					registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
					SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
					SDL_RenderPresent(renderer);
				}
				else {
					SDL_FreeSurface(registerSurface);
					registerSurface = TTF_RenderText_Solid(font, "Register", black);
					registerTexture = SDL_CreateTextureFromSurface(renderer, registerSurface);
					SDL_RenderCopy(renderer, registerTexture, NULL, &registerRect);
					SDL_RenderPresent(renderer);
				}
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					SDL_DestroyWindow(window);
					loginDisplay(gameState);
				}
			}
		}
	}

	SDL_StopTextInput();
	SDL_DestroyTexture(imageTexture);
	SDL_FreeSurface(imageSurface);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	TTF_Quit();
	SDL_Quit();
	return;
}

void QRcode(SDL_Window* window, SDL_Renderer* renderer, GameState* gameState) {

	SDL_RenderClear(renderer);

	SDL_Surface* qrSurface, * bgSurface, * textSurface, * thankSurface, * logoSurface = NULL;
	SDL_Texture* qrTexture, * bgTexture, * textTexture, * thankTextTure, * logoTexture = NULL;
	SDL_Rect textRect, thankRect;
	TTF_Font* font = NULL;
	SDL_Color black = { 0  , 0 , 0 , 255 };
	SDL_Color red = { 255  , 0 , 0 , 255 };
	SDL_Color orange = { 0  , 255 , 0 , 255 };

	SDL_Color blue = { 0 , 255 , 255 , 255 };

	SDL_Rect qrRect = { 398 , 255 , 170 , 170 };
	SDL_Rect logoRect = { 328 , 300 , 60 , 60 };

	Mix_Music* clickSound = Mix_LoadWAV("Resource\\Sound\\Sound Effects\\click.wav");
	char thanks[50] = "";
	sprintf_s(thanks, sizeof(thanks), "^^Thank You For Your Donation^^");

	bgSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\donate.bmp");
	bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
	SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

	qrSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\QR.bmp");
	qrTexture = SDL_CreateTextureFromSurface(renderer, qrSurface);
	SDL_RenderCopy(renderer, qrTexture, NULL, &qrRect);

	logoSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\momologo.bmp");
	logoTexture = SDL_CreateTextureFromSurface(renderer, logoSurface);
	SDL_RenderCopy(renderer, logoTexture, NULL, &logoRect);

	font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);

	textSurface = TTF_RenderText_Solid(font, "Return", black);
	textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	textRect.x = 447;
	textRect.y = 440;
	SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

	font = TTF_OpenFont("Resource\\Fonts\\Kanit-MediumItalic.ttf", 30);
	thankSurface = TTF_RenderText_Solid(font, thanks, orange);
	thankTextTure = SDL_CreateTextureFromSurface(renderer, thankSurface);
	thankRect.x = 247;
	thankRect.y = 200;
	SDL_QueryTexture(thankTextTure, NULL, NULL, &thankRect.w, &thankRect.h);
	SDL_RenderCopy(renderer, thankTextTure, NULL, &thankRect);

	font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
	SDL_RenderPresent(renderer);

	SDL_Event event;
	bool quit = false;

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_MOUSEMOTION:
				if (event.motion.x >= textRect.x && event.motion.x <= textRect.x + textRect.w &&
					event.motion.y >= textRect.y && event.motion.y <= textRect.y + textRect.h) {
					SDL_FreeSurface(textSurface);
					textSurface = TTF_RenderText_Solid(font, "Return", blue);
					textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
					SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
					SDL_RenderPresent(renderer);
				}
				else {
					SDL_FreeSurface(textSurface);
					textSurface = TTF_RenderText_Solid(font, "Return", black);
					textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
					SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
					SDL_RenderPresent(renderer);
				}

				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.x >= textRect.x && event.button.x <= textRect.x + textRect.w &&
					event.button.y >= textRect.y && event.button.y <= textRect.y + textRect.h) {
					Mix_PlayChannel(-1, clickSound, 0);
					SDL_FreeSurface(textSurface);
					SDL_FreeSurface(bgSurface);
					SDL_FreeSurface(qrSurface);
					SDL_FreeSurface(thankSurface);
					SDL_DestroyTexture(thankTextTure);
					SDL_DestroyTexture(textTexture);
					SDL_DestroyTexture(bgTexture);
					SDL_DestroyTexture(qrTexture);
					SDL_RenderClear(renderer);
					SDL_DestroyWindow(window);
					accountDisplay(gameState);
				}
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					SDL_FreeSurface(textSurface);
					SDL_FreeSurface(bgSurface);
					SDL_FreeSurface(qrSurface);
					SDL_FreeSurface(thankSurface);
					SDL_DestroyTexture(thankTextTure);
					SDL_DestroyTexture(textTexture);
					SDL_DestroyTexture(bgTexture);
					SDL_DestroyTexture(qrTexture);
					SDL_RenderClear(renderer);
					SDL_DestroyWindow(window);
					accountDisplay(gameState);
				}
				break;
			}
		}
	}
}

void storyDisplay(SDL_Window* window, SDL_Renderer* renderer, GameState* gameState) {

	SDL_RenderClear(renderer);

	TTF_Font* font = NULL;
	SDL_Color aqua = { 0 , 255 , 255 , 255 };
	SDL_Color black = { 0 , 0 , 0 , 255 };

	SDL_Surface* storySurface[2] = { NULL };
	SDL_Texture* storyTexture[2] = { NULL };

	SDL_Surface* redArrowSurface, * greenArrowSurface, * returnSurface = NULL;
	SDL_Texture* redArrowTexture, * greenArrowTexture, * returnTexture = NULL;
	SDL_Rect rightRect, leftRect, returnRect;
	rightRect.x = 714;
	rightRect.y = 481;
	rightRect.w = 73;
	rightRect.h = 55;

	leftRect.x = 206;
	leftRect.y = 480;
	leftRect.w = 74;
	leftRect.h = 55;

	returnRect.x = 450;
	returnRect.y = 503;

	SDL_Rect fixScreen;  // 960 668
	fixScreen.x = -70;
	fixScreen.y = 0;
	fixScreen.w = 1110;
	fixScreen.h = 668;

	int page = 1;

	Mix_Music* clickSound = Mix_LoadWAV("Resource\\Sound\\Sound Effects\\click.wav");

	storySurface[0] = SDL_LoadBMP("Resource\\Menu\\Story\\story1.bmp");
	storySurface[1] = SDL_LoadBMP("Resource\\Menu\\Story\\story2.bmp");

	storyTexture[0] = SDL_CreateTextureFromSurface(renderer, storySurface[0]);
	storyTexture[1] = SDL_CreateTextureFromSurface(renderer, storySurface[1]);

	SDL_RenderCopy(renderer, storyTexture[0], NULL, &fixScreen);

	SDL_FreeSurface(storySurface[0]);
	SDL_FreeSurface(storySurface[1]);

	redArrowSurface = SDL_LoadBMP("Resource\\Menu\\Story\\red.bmp");
	redArrowTexture = SDL_CreateTextureFromSurface(renderer, redArrowSurface);

	greenArrowSurface = SDL_LoadBMP("Resource\\Menu\\Story\\green.bmp");
	greenArrowTexture = SDL_CreateTextureFromSurface(renderer, greenArrowSurface);
	SDL_RenderCopy(renderer, greenArrowTexture, NULL, &rightRect);

	font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
	returnSurface = TTF_RenderText_Solid(font, "Return", black);
	returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
	SDL_QueryTexture(returnTexture, NULL, NULL, &returnRect.w, &returnRect.h);
	SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);

	SDL_RenderPresent(renderer);

	SDL_Event event;
	bool quit = false;

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_MOUSEMOTION:
				if (event.motion.x >= rightRect.x && event.motion.x <= rightRect.x + rightRect.w &&
					event.motion.y >= rightRect.y && event.motion.y <= rightRect.y + rightRect.h) {
					if (page == 1) {
						SDL_RenderCopyEx(renderer, redArrowTexture, NULL, &rightRect, 0, NULL, 1);
						SDL_RenderPresent(renderer);
					}
				}
				else {
					if (page == 1) {
						SDL_RenderCopy(renderer, greenArrowTexture, NULL, &rightRect);
						SDL_RenderPresent(renderer);
					}
				}
				//
				if (event.motion.x >= leftRect.x && event.motion.x <= leftRect.x + leftRect.w &&
					event.motion.y >= leftRect.y && event.motion.y <= leftRect.y + leftRect.h) {
					if (page == 2) {
						SDL_RenderCopyEx(renderer, redArrowTexture, NULL, &leftRect, 0, NULL, 0);
						SDL_RenderPresent(renderer);
					}
				}
				else {
					if (page == 2) {
						SDL_RenderCopyEx(renderer, greenArrowTexture, NULL, &leftRect, 0, NULL, 1);
						SDL_RenderPresent(renderer);
					}
				}
				// return button
				if (event.motion.x >= returnRect.x && event.motion.x <= returnRect.x + returnRect.w &&
					event.motion.y >= returnRect.y && event.motion.y <= returnRect.y + returnRect.h) {
					SDL_FreeSurface(returnSurface);
					returnSurface = TTF_RenderText_Solid(font, "Return", aqua);
					returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
					SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
					SDL_RenderPresent(renderer);
				}
				else {
					SDL_FreeSurface(returnSurface);
					returnSurface = TTF_RenderText_Solid(font, "Return", black);
					returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
					SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
					SDL_RenderPresent(renderer);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.x >= rightRect.x && event.button.x <= rightRect.x + rightRect.w &&
					event.button.y >= rightRect.y && event.button.y <= rightRect.y + rightRect.h) {
					Mix_PlayChannel(-1, clickSound, 0);
					page = 2;
					SDL_RenderClear(renderer);
					SDL_RenderCopy(renderer, storyTexture[1], NULL, &fixScreen);
					SDL_RenderCopyEx(renderer, greenArrowTexture, NULL, &leftRect, 0, NULL, 1);
					returnSurface = TTF_RenderText_Solid(font, "Return", black);
					returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
					SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
					SDL_RenderPresent(renderer);
				}
				//
				if (event.button.x >= leftRect.x && event.button.x <= leftRect.x + leftRect.w &&
					event.button.y >= leftRect.y && event.button.y <= leftRect.y + leftRect.h) {
					Mix_PlayChannel(-1, clickSound, 0);
					page = 1;
					SDL_RenderClear(renderer);
					SDL_RenderCopy(renderer, storyTexture[0], NULL, &fixScreen);
					SDL_RenderCopyEx(renderer, greenArrowTexture, NULL, &rightRect, 0, NULL, 0);
					returnSurface = TTF_RenderText_Solid(font, "Return", black);
					returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
					SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
					SDL_RenderPresent(renderer);
				}
				// click return
				if (event.button.x >= returnRect.x && event.button.x <= returnRect.x + returnRect.w &&
					event.button.y >= returnRect.y && event.button.y <= returnRect.y + returnRect.h) {
					Mix_PlayChannel(-1, clickSound, 0);
					SDL_DestroyTexture(returnTexture);
					SDL_DestroyTexture(redArrowTexture);
					SDL_DestroyTexture(greenArrowTexture);
					SDL_DestroyTexture(storyTexture[0]);
					SDL_DestroyTexture(storyTexture[1]);
					;					
					SDL_DestroyWindow(window);
					accountDisplay(gameState);
				}
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					SDL_DestroyWindow(window);
					accountDisplay(gameState);
				}
				break;
			}

		}

	}
	SDL_RenderClear(renderer);
}

void gameMode(SDL_Window* window, SDL_Renderer* renderer, char selectMode[], int* ptrNumMode, GameState* gameState) {
	gameState->difficulty = DIFFICULTY_EASY;
	SDL_RenderClear(renderer);

	SDL_Surface* modeSurface, * returnSurface, * bgSurface, * trickSurface, * textSurface = NULL;
	SDL_Texture* modeTexture, * returnTexture, * bgTexture, * trickTexture, * textTexture = NULL;

	SDL_Surface* arrowSurface[2] = { NULL };
	SDL_Texture* arrowTexture[2] = { NULL };

	SDL_Rect returnRect;
	SDL_Rect trickRect = { 390, 300 ,  190 , 70 };
	SDL_Rect textRect;
	textRect.x = 350;
	textRect.y = 130;

	SDL_Color black = { 0 , 0 , 0 ,255 };
	SDL_Color red = { 255 , 0 , 0 ,255 };
	SDL_Color yellow = { 0 , 200 , 255 , 255 };
	SDL_Color aqua = { 0 , 255 , 255 , 255 };

	SDL_Color modeColor[3] = { {0 , 255 , 0 , 255} , { 255 , 200 , 0 , 255 } , { 255 , 0 , 0 , 255} };

	SDL_Rect pos[2];
	pos[0].x = 310;   //  <<
	pos[0].y = 290;

	pos[1].x = 595;  // >>
	pos[1].y = 290;

	SDL_Rect modeRect[3];
	modeRect[0].x = 435; // easy
	modeRect[0].y = 265;

	modeRect[1].x = 415; // medium
	modeRect[1].y = 265;

	modeRect[2].x = 435; // hard
	modeRect[2].y = 265;

	Mix_Music* clickSound = Mix_LoadWAV("Resource\\Sound\\Sound Effects\\click.wav");

	char* arrow[2] = { "<<" , ">>" };

	char* mode[3] = { "Easy" , "Medium" , "Hard" };

	TTF_Font* font = NULL;

	bgSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\gamemode.bmp");
	bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
	SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

	trickSurface = SDL_LoadBMP("Resource\\Menu\\LoginImage\\trick3.bmp");
	trickTexture = SDL_CreateTextureFromSurface(renderer, trickSurface);
	SDL_RenderCopy(renderer, trickTexture, NULL, &trickRect);

	SDL_RenderPresent(renderer);

	font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
	returnSurface = TTF_RenderText_Solid(font, "Return", black);
	returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
	returnRect.x = 446;
	returnRect.y = 439;
	SDL_QueryTexture(returnTexture, NULL, NULL, &returnRect.w, &returnRect.h);
	SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);

	font = TTF_OpenFont("Resource\\Fonts\\Kanit-Medium.ttf", 60);

	for (int i = 0; i < 2; i++) {
		arrowSurface[i] = TTF_RenderText_Solid(font, arrow[i], black);
		arrowTexture[i] = SDL_CreateTextureFromSurface(renderer, arrowSurface[i]);
		SDL_QueryTexture(arrowTexture[i], NULL, NULL, &pos[i].w, &pos[i].h);
		SDL_RenderCopy(renderer, arrowTexture[i], NULL, &pos[i]);
	}

	font = TTF_OpenFont("Resource\\Fonts\\crazy-pixel.ttf", 100);
	textSurface = TTF_RenderText_Solid(font, "Game Mode", yellow);
	textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

	font = TTF_OpenFont("Resource\\Fonts\\crazy-pixel.ttf", 80);
	modeSurface = TTF_RenderText_Solid(font, selectMode, modeColor[*(ptrNumMode)]);
	modeTexture = SDL_CreateTextureFromSurface(renderer, modeSurface);

	if (strcmp(selectMode, mode[0]) == 0) {
		SDL_QueryTexture(modeTexture, NULL, NULL, &modeRect[0].w, &modeRect[0].h);
		SDL_RenderCopy(renderer, modeTexture, NULL, &modeRect[0]);
		*(ptrNumMode) = 0;
	}
	else if (strcmp(selectMode, mode[1]) == 0) {
		SDL_QueryTexture(modeTexture, NULL, NULL, &modeRect[1].w, &modeRect[1].h);
		SDL_RenderCopy(renderer, modeTexture, NULL, &modeRect[1]);
		*(ptrNumMode) = 1;
	}
	else if (strcmp(selectMode, mode[2]) == 0) {
		SDL_QueryTexture(modeTexture, NULL, NULL, &modeRect[2].w, &modeRect[2].h);
		SDL_RenderCopy(renderer, modeTexture, NULL, &modeRect[2]);
		*(ptrNumMode) = 2;
	}

	SDL_RenderPresent(renderer);

	SDL_Event event;
	bool quit = false;
	int x, y;

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				for (int i = 0; i < 2; i++) {
					SDL_FreeSurface(arrowSurface[i]);
				}
				quit = true;
				break;
			case SDL_MOUSEMOTION:
				x = event.motion.x; y = event.motion.y;
				for (int i = 0; i < 2; i++) {
					font = TTF_OpenFont("Resource\\Fonts\\Kanit-Medium.ttf", 40);
					if (x >= pos[i].x && x <= pos[i].x + pos[i].w &&
						y >= pos[i].y && y <= pos[i].y + pos[i].h) {
						SDL_FreeSurface(arrowSurface[i]);
						arrowSurface[i] = TTF_RenderText_Solid(font, arrow[i], red);
						arrowTexture[i] = SDL_CreateTextureFromSurface(renderer, arrowSurface[i]);
						SDL_RenderCopy(renderer, arrowTexture[i], NULL, &pos[i]);
						SDL_RenderPresent(renderer);
					}
					else {
						SDL_FreeSurface(arrowSurface[i]);
						arrowSurface[i] = TTF_RenderText_Solid(font, arrow[i], black);
						arrowTexture[i] = SDL_CreateTextureFromSurface(renderer, arrowSurface[i]);
						SDL_RenderCopy(renderer, arrowTexture[i], NULL, &pos[i]);
						SDL_RenderPresent(renderer);
					}
				}
				//
				if (x >= returnRect.x && x <= returnRect.x + returnRect.w &&
					y >= returnRect.y && y <= returnRect.y + returnRect.h) {
					SDL_FreeSurface(returnSurface);
					font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
					returnSurface = TTF_RenderText_Solid(font, "Return", aqua);
					returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
					SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
					SDL_RenderPresent(renderer);
				}
				else {
					SDL_FreeSurface(returnSurface);
					font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
					returnSurface = TTF_RenderText_Solid(font, "Return", black);
					returnTexture = SDL_CreateTextureFromSurface(renderer, returnSurface);
					SDL_RenderCopy(renderer, returnTexture, NULL, &returnRect);
					SDL_RenderPresent(renderer);
				}

				break;
			case SDL_MOUSEBUTTONDOWN:
				x = event.button.x; y = event.button.y;
				if (x >= returnRect.x && x <= returnRect.x + returnRect.w &&
					y >= returnRect.y && y <= returnRect.y + returnRect.h) {
					Mix_PlayChannel(-1, clickSound, 0);
					SDL_FreeSurface(returnSurface);
					SDL_FreeSurface(bgSurface);
					SDL_FreeSurface(trickSurface);
					SDL_FreeSurface(arrowSurface[0]);
					SDL_FreeSurface(arrowSurface[1]);
					SDL_DestroyTexture(returnTexture);
					SDL_DestroyTexture(bgTexture);
					SDL_DestroyTexture(trickTexture);
					SDL_DestroyTexture(arrowTexture[0]);
					SDL_DestroyTexture(arrowTexture[1]);
					SDL_RenderClear(renderer);
					SDL_DestroyWindow(window);
					accountDisplay(gameState);
				}

				if (x >= pos[0].x && x <= pos[0].x + pos[0].w &&  //  left arrow
					y >= pos[0].y && y <= pos[0].y + pos[0].h) {
					*(ptrNumMode) = *(ptrNumMode)-1;
					if (*(ptrNumMode) < 0) {
						*(ptrNumMode) = 2;
					}
					memset(selectMode, '\0', 10);
					strcpy(selectMode, mode[*(ptrNumMode)]);
					if (strcmp(selectMode, "Easy") == 0) {
						gameState->setDifficulty = 1;
						gameState->difficulty = DIFFICULTY_EASY;
					}
					else if (strcmp(selectMode, "Medium") == 0) {
						gameState->setDifficulty = 1;
						gameState->difficulty = DIFFICULTY_MEDIUM;
					}
					else if (strcmp(selectMode, "Hard") == 0) {
						gameState->setDifficulty = 1;
						gameState->difficulty = DIFFICULTY_HARD;
					}
					SDL_RenderCopy(renderer, trickTexture, NULL, &trickRect);
					SDL_RenderPresent(renderer);

					font = TTF_OpenFont("Resource\\Fonts\\crazy-pixel.ttf", 80);
					modeSurface = TTF_RenderText_Solid(font, selectMode, modeColor[*(ptrNumMode)]);
					modeTexture = SDL_CreateTextureFromSurface(renderer, modeSurface);
					SDL_QueryTexture(modeTexture, NULL, NULL, &modeRect[*(ptrNumMode)].w, &modeRect[*(ptrNumMode)].h);
					SDL_RenderCopy(renderer, modeTexture, NULL, &modeRect[*(ptrNumMode)]);

					SDL_RenderPresent(renderer);
				}

				if (x >= pos[1].x && x <= pos[1].x + pos[1].w &&  //  right arrow
					y >= pos[1].y && y <= pos[1].y + pos[1].h) {
					*(ptrNumMode) = *(ptrNumMode)+1;
					if (*(ptrNumMode) > 2) {
						*(ptrNumMode) = 0;
					}
					memset(selectMode, '\0', 10);
					strcpy(selectMode, mode[*(ptrNumMode)]);
					if (strcmp(selectMode, "Easy") == 0) {
						gameState->setDifficulty = 1;
						gameState->difficulty = DIFFICULTY_EASY;
					}
					else if (strcmp(selectMode, "Medium") == 0) {
						gameState->setDifficulty = 1;
						gameState->difficulty = DIFFICULTY_MEDIUM;
					}
					else if (strcmp(selectMode, "Hard") == 0) {
						gameState->setDifficulty = 1;
						gameState->difficulty = DIFFICULTY_HARD;
					}
					SDL_RenderCopy(renderer, trickTexture, NULL, &trickRect);
					SDL_RenderPresent(renderer);

					font = TTF_OpenFont("Resource\\Fonts\\crazy-pixel.ttf", 80);
					modeSurface = TTF_RenderText_Solid(font, selectMode, modeColor[*(ptrNumMode)]);
					modeTexture = SDL_CreateTextureFromSurface(renderer, modeSurface);
					SDL_QueryTexture(modeTexture, NULL, NULL, &modeRect[*(ptrNumMode)].w, &modeRect[*(ptrNumMode)].h);
					SDL_RenderCopy(renderer, modeTexture, NULL, &modeRect[*(ptrNumMode)]);

					SDL_RenderPresent(renderer);
				}
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					SDL_FreeSurface(returnSurface);
					SDL_FreeSurface(bgSurface);
					SDL_FreeSurface(trickSurface);
					SDL_FreeSurface(arrowSurface[0]);
					SDL_FreeSurface(arrowSurface[1]);
					SDL_DestroyTexture(returnTexture);
					SDL_DestroyTexture(bgTexture);
					SDL_DestroyTexture(trickTexture);
					SDL_DestroyTexture(arrowTexture[0]);
					SDL_DestroyTexture(arrowTexture[1]);
					SDL_RenderClear(renderer);
					SDL_DestroyWindow(window);
					accountDisplay(gameState);
				}
				break;
			}
		}
	}
}