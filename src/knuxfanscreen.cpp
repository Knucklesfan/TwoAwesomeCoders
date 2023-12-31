#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <cstring>
#include "background.h"
#include "knuxfanscreen.h"

knuxfanscreen::knuxfanscreen(SDL_Renderer* render, std::vector<SDL_Texture*> texture, std::vector<bg>  backg, Mix_Chunk* soundVec[], int background, font* foont) {
    renderer = render; 
    textures = texture;
    sound = soundVec;
    backgrounds = backg;
    backnum = background;
    active = true;
    godown = false;
    splash = rand() % 35;
    front = foont;
    
}
void knuxfanscreen::render() {
    SDL_RenderClear(renderer);
    backgrounds[backnum].render(renderer, false);
    front->render(320, 450, splashes[splash], true, renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255*alpha);
    SDL_Rect splashbox = { 0, 0, 640, 480 };
    SDL_RenderFillRect(renderer, &splashbox);

    if(showTwo) {
        drawTexture(textures[6],0,0,0,1.0,false);
    }
    if(showgamers) {
        drawTexture(textures[7],0,0,0,1.0,false);
    }

    if(showblock) {
        drawTexture(textures[0],320,240,blockangle,blockscale,true);
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255*otheralpha);
    splashbox = { 0, 0, 640, 480 };
    SDL_RenderFillRect(renderer, &splashbox);


}
void knuxfanscreen::keyPressed(SDL_Keycode key) {
    active = false;
}
void knuxfanscreen::logic(double deltatime) {
	if (active) {
        if(lifetime > 200 && lifetime < 250) {
            showTwo = 1;
            Mix_PlayChannel(-1, sound[2], 0);
        }
		lifetime += deltatime/5;
		if (alpha > 0.0 && godown) {
			alpha-=deltatime/1500;
		}
        if(lifetime > 250 && lifetime < 275) {
            rotateInBlock = true;
            showblock = true;
        }
        if(lifetime > 750 && lifetime < 760) {
            showgamers = true;
            Mix_PlayChannel(-1, sound[4], 0);

        }
        if(rotateInBlock) {
            if(blockscale > 1) {
                blockscale -= deltatime/2.5;
            }
            else {
                blockscale = 1;
                rotateInBlock = false;
                Mix_PlayChannel(-1, sound[3], 0);

            }
            if(blockangle > 0) {
                blockangle-= deltatime/5;
            }

        }
        if(lifetime > 500 && lifetime < 515) {
            godown = true;
        }
		else if (alpha <= 0.0 && godown) {
			godown = false;
            alpha = 0;
		}
		if (otheralpha < 1.0 && goup) {
			otheralpha+=deltatime / 1500;
		}
		else if (otheralpha >= 1.0 && goup) {
			godown = false;
			active = false;
		}

		if ((!godown && !goup) && lifetime > 2000) {
			goup = true;
		}
        //std::cout << lifetime << "\n";
        backgrounds[backnum].logic(deltatime);
        if(alpha < 0.0) {
            alpha = 0.0;
        }
        if(alpha > 1.0) {
            alpha = 1.0;
        }

	}
}

int knuxfanscreen::endlogic() {
    if(!active) {
        return 1;
    }
    return 0;
}
void knuxfanscreen::drawTexture(SDL_Texture* texture, int x, int y, double angle, double scale, bool center) {
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
