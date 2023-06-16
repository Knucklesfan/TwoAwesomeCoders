#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <cstring>
#include "background.h"
#include "font.h"

class titlescreen
{
public:
	font* buttonfont;
	font* headerfont;
	font* bodyfont;
	font* versfont;

	Mix_Chunk** sound;
	double time = 0;
	int currentsetting = 0;
	bg* background;
	SDL_Renderer* renderer;
	SDL_Window* window;
	std::vector<SDL_Texture*> textures;
	int currentscreen = 0;
	int currentselection = 0;
	int bgnum = 0;
	const int selections = 2;
	std::string settings[2] = {
		"START",
		"EXIT"
	};
	bool quit = false;
	bool loadgame = false;
	const int settingssize = 4;

	titlescreen(SDL_Renderer* render, SDL_Window* windows, bg* backg, std::vector<SDL_Texture*> texture, Mix_Music* musicVec[], Mix_Chunk* soundVec[], int backgr, std::vector<font*> fonts);
	double layerpos[10];
	void keyPressed(SDL_Keycode key);
	void render();
	void logic(double deltatime);
	int endlogic();
	void reset();
private:
	void drawTexture(SDL_Texture* texture, int x, int y, double angle, double scale, bool center);
	void drawTexture(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, double angle, double scale, bool center, int srcx, int srcy, int srcw, int srch);
	void debuginput(SDL_Keycode key);
};

