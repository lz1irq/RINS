#include "Graphics.h"
Graphics::Graphics(int width, int height, const char* title):textures() {
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) std::cout << SDL_GetError() << std::endl;
	if ((win = SDL_CreateWindow(title, 100, 100, width, height, SDL_WINDOW_SHOWN)) == nullptr) std::cout << SDL_GetError() << std::endl;
	if ((ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == nullptr) std::cout << SDL_GetError() << std::endl;
}

int Graphics::loadTexture(const char* path) {
	if(current_textures == MAX_TEXTURES) return -1;
	SDL_Texture* tex;
	if ((tex = IMG_LoadTexture(ren, path)) == nullptr)cout << "Failed to load image: " <<  path <<  IMG_GetError() << endl;
	textures[current_textures] = tex;
	return current_textures++;
}

void Graphics::applyTexture(int texture_ID, int x, int y) {
	if(textures[texture_ID] == nullptr) return;
	SDL_RenderCopy(ren, textures[texture_ID], NULL, NULL);
}

void Graphics::renderScene() {
	SDL_RenderPresent(ren);
	SDL_RenderClear(ren);
}

Graphics::~Graphics() {
	for(int i=0;i<current_textures;++i) SDL_DestroyTexture(textures[i]);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
}
int Graphics::current_textures = 0;
