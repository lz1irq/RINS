#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
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

struct RGBA{
	SDL_Color col;
	RGBA(Uint8 R, Uint8 G, Uint8 B, Uint8 A){
		col.r = R;
		col.g = G;
		col.b = B;
		col.a = A;
	}
};

class Renderer {
	static const int MAX_TEXTURES = 1024;
	static int current_textures;
	SDL_Window* win;
	SDL_Renderer* ren;
	SDL_Texture* textures[MAX_TEXTURES];
	int W, H;
	SDL_Rect part;
	TTF_Font* fonts[MAX_TEXTURES];
	static int current_fonts;
	double rotate;
	void applyTexture(SDL_Texture* t, double x, double y, double width, double height);
public:
	Renderer(int width, int height, const char* title);
	int loadTexture(const char* path);
	void applyTexture(int texture_ID, double x, double y, double width, double heigth);
	void renderScene();
	void renderPart(int xparts, int yparts, int xpartnum, int ypartnum);
	int loadFont(const char* font, int size);
	void displayText(int font, const Uint16* text, RGBA color, double x, double y, double w, double h);
	void setRotationAngle(double deg);
	~Renderer();
};

class Game{
	SDL_Event event;
	static int secondaryLoop(void* param);
	bool quit;
	bool has_event;
public:
	virtual void mainLoop() = 0;
	virtual void graphicsLoop() = 0;
	unsigned int getTicks();
	char getKey(bool pressed);
	Game();
	void loop();
	~Game();
};
#endif