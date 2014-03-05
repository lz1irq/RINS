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

void Renderer::applyTexture(int texture_ID, float x, float y, double width, double height) {
	if (textures[texture_ID] == nullptr)throw Error("Bad texture ID!");
	SDL_Rect dst;
	SDL_Rect src;
	if (SDL_QueryTexture(textures[texture_ID], NULL, NULL, &dst.w, &dst.h) != 0)throw Error(SDL_GetError());
	dst.x = W*x;
	dst.y = H*y;
	if (part.x != 0 && part.y != 0){
		src.x = 1.0/part.x*dst.w*part.w;
		src.y = 1.0/part.y*dst.h*part.h;
		src.w = 1.0/part.x*dst.w*(part.w + 1);
		src.h = 1.0/part.y*dst.h*(part.h + 1);
	}
	else{
		src.x = 0;
		src.y = 0;
		src.w = dst.w;
		src.h = dst.h;
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

Game::Game() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)throw Error(SDL_GetError());
}

void Game::loop(){
	SDL_Thread* thread;
	if( (thread = SDL_CreateThread(secondaryLoop, "secondaryThread", (void *)this)) == NULL) throw Error( SDL_GetError() );
	while (!quit) {
		while (SDL_PollEvent(&event)) if (event.type == SDL_QUIT) quit = true;
		mainLoop();
	}
	 SDL_WaitThread(thread, NULL);
}

int Game::secondaryLoop(void* param) {
	Game* instance = (Game*)param;
	while(!(instance->quit)) {
		instance->graphicsLoop();
	}
	return 0;
}

void Game::graphicsLoop() {

}

unsigned int Game::getTicks() {
	return SDL_GetTicks();
}

Game::~Game(){
	SDL_Quit();
}