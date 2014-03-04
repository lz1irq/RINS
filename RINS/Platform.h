#include "SDL.h"
#include "SDL_image.h"
#include<iostream>
using namespace std;
#ifndef _GLIBCXX_PLATFORM_H
#define _GLIBCXX_PLATFORM_H

class Error{
	const char* err;
public:
	Error(const char* err);
	const char* getError();
};

class Renderer {
	static const int MAX_TEXTURES = 1024;
	static int current_textures;
	SDL_Window* win;
	SDL_Renderer* ren;
	SDL_Texture* textures[MAX_TEXTURES];
	int W, H;
	SDL_Rect part;
public:
	Renderer(int width, int height, const char* title);
	int loadTexture(const char* path);
	void applyTexture(int texture_ID, float x, float y, double width, double heigth);
	void renderScene();
	void renderPart(int xparts, int yparts, int xpartnum, int ypartnum);
	~Renderer();
};

class Game{
	SDL_Event event;
public:
	virtual void mainLoop() = 0;
	int getTicks();
	Game();
	void loop();
	~Game();
};
#endif