#include "SDL.h"
#include "SDL_image.h"
#include<iostream>
using namespace std;
#ifndef _GLIBCXX_GRAPHICS_H
#define _GLIBCXX_GRAPHICS_H
class Graphics {
	static const int MAX_TEXTURES = 1024;
	static int current_textures;
	SDL_Window* win;
	SDL_Renderer* ren;
	SDL_Texture* textures[MAX_TEXTURES];
public:
	Graphics(int width, int height, const char* title);
	int loadTexture(const char* path);
	void applyTexture(int texture_ID, int x, int y);
	void renderScene();
	~Graphics();
};
#endif