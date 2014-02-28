#include "Platform.h"

Error::Error(const char* err) : err(err){}

const char* Error::getError(){ return err; }

Renderer::Renderer(int width, int height, const char* title) : textures(), W(width), H(height) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)throw Error(SDL_GetError());
	if ((win = SDL_CreateWindow(title, 100, 100, width, height, SDL_WINDOW_SHOWN)) == nullptr)throw Error(SDL_GetError());
	if ((ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == nullptr)throw Error(SDL_GetError());
	part.x = 0;
	part.y = 0;
}

int Renderer::loadTexture(const char* path) {
	if (current_textures == MAX_TEXTURES) return -1;
	SDL_Texture* tex;
	if ((tex = IMG_LoadTexture(ren, path)) == nullptr)throw Error(IMG_GetError());
	textures[current_textures] = tex;
	return current_textures++;
}

void Renderer::applyTexture(int texture_ID, int x, int y, double width, double height) {
	if (textures[texture_ID] == nullptr)throw Error("Bad texture ID!");
	SDL_Rect dst;
	SDL_Rect src;
	if (SDL_QueryTexture(textures[texture_ID], NULL, NULL, &dst.w, &dst.h) != 0)throw Error(SDL_GetError());
	dst.x = x;
	dst.y = y;
	if (part.x != 0 && part.y != 0){
		src.x = ((1.0 / (double)part.x)*dst.w)*part.w;
		src.y = ((1.0 / (double)part.y)*dst.h)*part.h;
		src.w = ((1.0 / (double)part.x)*dst.w)*(part.w + 1);
		src.h = ((1.0 / (double)part.y)*dst.h)*(part.h + 1);
	}
	else{
		part.x = 0;
		part.y = 0;
		part.w = W;
		part.h = H;
	}
	double w = W*width;
	double h = H*height;
	double ratio = dst.w / dst.h;
	if (w == 0)w = h*ratio;
	if (h == 0)h = w / ratio;
	if (w != 0 && h != 0){
		dst.w = w;
		dst.h = h;
	}
	if (SDL_RenderCopy(ren, textures[texture_ID], &src, &dst) != 0)throw Error(SDL_GetError());
}

int Renderer::getRendererWidth(){
	return W;
}

int Renderer::getRendererHeight(){
	return H;
}

void Renderer::renderPart(int xparts, int yparts, int xpartnum, int ypartnum){
	part.x = xparts;
	part.y = yparts;
	part.w = xpartnum;
	part.h = ypartnum;
}

void Renderer::renderScene() {
	SDL_RenderPresent(ren);
	if (SDL_RenderClear(ren) != 0)throw Error(SDL_GetError());
}

Renderer::~Renderer() {
	for (int i = 0; i<current_textures; ++i) SDL_DestroyTexture(textures[i]);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
}
int Renderer::current_textures = 0;

Game::Game(){
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)throw Error(SDL_GetError());
}

void Game::loop(){
	bool quit = false;
	while (!quit) {
		while (SDL_PollEvent(&event)) if (event.type == SDL_QUIT) quit = true;
		mainLoop();
	}
}

Game::~Game(){
	SDL_Quit();
}