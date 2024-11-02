#include "checkAccount.h"
#include <string.h>

int checkRepeatedUsername(char username[]) {
    FILE* file;
    int m;

    bool check = false;
    char buffer[100][50] = { 0 };
    int i = 0;
    file = fopen("user.txt", "r");
    while (fgets(buffer, 100, file)) {
        buffer[i][strcspn(buffer[i], "\n")] = '\0';
        i++;
    }
    fclose(file);
    int lineBuffer;
    int indxBuffer;

    for (lineBuffer = 0; lineBuffer < i; lineBuffer++) {
        int j = 0;
        int numChar = 0;
        char tempUser[30] = { 0 };
        for (indxBuffer = 0; indxBuffer <= strlen(buffer[lineBuffer]); indxBuffer++) {
            if (buffer[lineBuffer][indxBuffer] == 32) { break; }
            else {
                tempUser[j] = buffer[lineBuffer][indxBuffer];
                j++;
            }
        }
        int check = strcmp(username, tempUser);
        if (check == 0) { return 0; }
        int m;
        for (m = 0; m <= j; m++) {
            tempUser[m] = '\0';
        }
    }

    return 1;
}

int checkPassword(char username[], char pass[]) {
    FILE* file;
    char buffer[100][100] = { 0 };
    int i = 0;

    file = fopen("user.txt", "r");

    while (fgets(buffer, 100, file)) {
        buffer[i][strcspn(buffer[i], "\n")] = '\0';
        i++;
    }

    int lineBuffer;

    for (lineBuffer = 0; lineBuffer < i; lineBuffer++) {
        char tempUser[30] = { 0 };
        int indxBuffer;
        char tempLine[30] = { 0 };

        strcpy(tempLine, buffer[lineBuffer]);

        for (indxBuffer = 0; indxBuffer <= strlen(tempLine); indxBuffer++) {
            int j = 0;
            int c = tempLine[indxBuffer];
            if (c == 32) { break; }
            else {
                tempUser[indxBuffer] = tempLine[indxBuffer];
                j++;
            }
        }

        char tempPass[30] = { 0 };

        int checkUser = strcmp(username, tempUser);
        int indx2;
        if (checkUser == 0) {
            int n = 0;

            for (indx2 = strlen(tempUser) + 3; indx2 <= strlen(tempLine); indx2++) {
                int c = tempLine[indx2];
                if (c == 32) { break; }
                else {
                    tempPass[n] = tempLine[indx2];
                    n++;
                }
            }

            int checkPass = strcmp(pass, tempPass);
            if (checkPass == 0) {
                return 0;
            }
            else {
                return 1;
                break;
            }
        }
    }
    return 0;
}

void getText(SDL_Renderer* renderer,
    SDL_Surface* textSurface,
    SDL_Texture* textTexture,
    TTF_Font* font,
    char text[],
    int x, int y, SDL_Color color) {

    SDL_Rect textRect;
    textRect.x = x;
    textRect.y = y;
    textSurface = TTF_RenderText_Solid(font, text, color);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_RenderPresent(renderer);
}

void writeIntoText(char username[], char password[]) {

    FILE* file;

    file = fopen("user.txt", "a");
    fprintf(file, username);
    fclose(file);

    file = fopen("user.txt", "a");
    fprintf(file, " , ");
    fclose(file);

    file = fopen("user.txt", "a");
    fprintf(file, password);
    fclose(file);

    file = fopen("user.txt", "a");
    fprintf(file, " , \n");
    fclose(file);

}

bool checkUsername(char username[]) {
    int i = 0;

    if (strlen(username) == 0) { return false; }
    if (strlen(username) > 22) { return false; }
    if (username[0] >= '0' && username[0] <= '9') {
        return false;
    }
    else {
        if (!((username[i] >= '0' && username[i] <= '9') || (username[i] >= 'a' && username[i] <= 'z') || (username[i] >= 'A' && username[i] <= 'Z') || (username[i] == '_'))) {
            return false;
        }
        else {
            i = 1;
            for (i; i < strlen(username); i++) {
                if ((username[i] >= '0' && username[i] <= '9') || (username[i] >= 'a' && username[i] <= 'z') || (username[i] >= 'A' && username[i] <= 'Z') || (username[i] == '_')) {
                    continue;
                }
                else
                    return false;
            }
        }
    }
    return true;
}


bool checkRepeatPassword(char pass[], char repeatPass[]) {
    int i;
    if (strlen(pass) != strlen(repeatPass)) {
        return false;
    }
    else {
        for (i = 0; i < strlen(pass); i++) {
            if (pass[i] != repeatPass[i]) {
                return false;
                break;
            }
        }
    }
    return true;
}