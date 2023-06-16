#include "titlescreen.h"
#include <iostream>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <math.h>

#ifdef __SWITCH__
#define prefix  "/"
#include <switch.h>

#else
#define prefix  "./"
#endif

titlescreen::titlescreen(SDL_Renderer* render, SDL_Window* windows, bg*  backg, std::vector<SDL_Texture*> texture, Mix_Music* musicVec[], Mix_Chunk* soundVec[], int backgr, std::vector<font*> fonts)
{
    buttonfont = fonts.at(0);
    bodyfont = fonts.at(0);
    versfont = fonts.at(2);
    headerfont = fonts.at(1);



    window = windows;
    background = backg;
    bgnum = backgr;
	renderer = render;
	textures = texture;
    sound = soundVec;

}
void titlescreen::reset()
{

}

void titlescreen::keyPressed(SDL_Keycode key)
{
    switch (currentscreen) {

    case 0: {

        switch (key) {
        case(SDLK_UP): {
            if (currentselection > 0) {
                currentselection = (currentselection - 1);
                Mix_PlayChannel( -1, sound[1], 0 );
            }
            break;
        }
        case(SDLK_DOWN): {
            if (currentselection < selections - 1) {
                currentselection = (currentselection + 1);
                Mix_PlayChannel( -1, sound[1], 0 );
            }
            break;
        }
        case(SDLK_z): {
            Mix_PlayChannel( -1, sound[0], 0 );
            switch (currentselection) {
            case 0:
                loadgame = true;
                break;
            case 1:
                exit(0);
                break;
            }
            break;
        }

        }
        break;
    }

}
}

void titlescreen::render()
{
    SDL_RenderClear(renderer);

    background[bgnum].render(renderer, false);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_Rect bx = { 160, 250, 320, 210 };
    SDL_RenderFillRect(renderer, &bx);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bx);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);

    drawTexture(textures[5], 0, 0, 0.0, 1.0, false);

    background[bgnum].render(renderer, true);

    for (int i = 0; i < selections; i++) {
        headerfont->render(renderer, settings[i], 320, 300 + (i * 32),  true, 255, ((i == currentselection && currentscreen == 0)?0:255), 255);
    }
    versfont->render(renderer, "VERS 1.0 BABY!", 490, 165, true, 0, 0, 0, 0, true, time/100, 1, 5);
    //SDL_RenderPresent(renderer);

}

void titlescreen::logic(double deltatime)
{
    background[bgnum].logic(deltatime);
    time += deltatime;
}

int titlescreen::endlogic()
{
    if (loadgame) {
        return 1;
    }
	return 0;
}

//this is the legacy drawtexture, for drawing sprites of whom's width and height are unknown
void titlescreen::drawTexture(SDL_Texture* texture, int x, int y, double angle, double scale, bool center) {
    SDL_Rect sprite;
    SDL_QueryTexture(texture, NULL, NULL, &sprite.w, &sprite.h);
    int oldwidth = sprite.w;
    int oldheight = sprite.h;
    sprite.w = sprite.w * scale;
    sprite.h = sprite.h * scale;
    if (center) {
        sprite.x = x - oldwidth / 2;
        sprite.y = y - oldheight / 2;
    }
    else {
        sprite.x = x + oldwidth / 2 - sprite.w / 2;
        sprite.y = y + oldheight / 2 - sprite.h / 2;
    }
    SDL_RenderCopyEx(renderer, texture, NULL, &sprite, angle, NULL, SDL_FLIP_NONE);
}
//this is the new drawtexture, for drawing sprite sheets and whatnot
void titlescreen::drawTexture(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, double angle, double scale, bool center, int srcx, int srcy, int srcw, int srch) {
    SDL_Rect sprite;
    SDL_Rect srcrect = { srcx, srcy, srcw, srch };
    if (SDL_QueryTexture(texture, NULL, NULL, &sprite.w, &sprite.h) < 0) {
        printf("TEXTURE ISSUES!!! \n");
        std::cout << SDL_GetError() << "\n";
    };
    sprite.w = srcw * scale;
    sprite.h = srch * scale;
    if (center) {
        sprite.x = x - srcw / 2;
        sprite.y = y - srch / 2;
    }
    else {
        sprite.x = x + srcw / 2 - sprite.w / 2;
        sprite.y = y + srch / 2 - sprite.h / 2;
    }
    SDL_RenderCopyEx(renderer, texture, &srcrect, &sprite, 0, NULL, SDL_FLIP_NONE);
}


void titlescreen::debuginput(SDL_Keycode key) {
}