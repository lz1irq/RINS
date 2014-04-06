#include "Platform.h"

Error::Error(const char* err) : err(err){}

const char* Error::getError(){ return err; }

Renderer::Renderer(int width, int height, const char* title) : textures(), fonts(){
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)throw Error(SDL_GetError());
	if ((win = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI)) == nullptr)throw Error(SDL_GetError());
	if ((ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == nullptr)throw Error(SDL_GetError());
	SDL_GetWindowSize(win, &W, &H);
	part.x = H;
	part.y = H;
	cout << W << " " << H << endl;
	if (TTF_Init() != 0)throw Error(TTF_GetError());
	rotate = 0;
}

int Renderer::loadTexture(const char* path) {
	if (current_textures == MAX_TEXTURES) return -1;
	SDL_Texture* tex;
	if ((tex = IMG_LoadTexture(ren, path)) == nullptr)throw Error(IMG_GetError());
	textures[current_textures] = tex;
	return current_textures++;
}

void Renderer::applyTexture(SDL_Texture* t, double x, double y, double width, double height){
	SDL_Rect dst;
	SDL_Rect src;
	if (SDL_QueryTexture(t, NULL, NULL, &dst.w, &dst.h) != 0)throw Error(SDL_GetError());
	dst.x = H*x;
	dst.y = H*y;
	if (part.x != 0 && part.y != 0){
		src.x = 1.0 / part.x*dst.w*part.w;
		src.y = 1.0 / part.y*dst.h*part.h;
		src.w = 1.0 / part.x*dst.w;
		src.h = 1.0 / part.y*dst.h;
	}
	else{
		src.x = 0;
		src.y = 0;
		src.w = dst.w;
		src.h = dst.h;
	}
	double w = H*width;
	double h = H*height;
	double ratio = dst.w / dst.h;
	if (w == 0)w = h*ratio;
	if (h == 0)h = w / ratio;
	if (w != 0 && h != 0){
		dst.w = w;
		dst.h = h;
	}
	dst.x += (W - H) / 2;
	if (SDL_RenderCopyEx(ren, t, &src, &dst, rotate, NULL, SDL_FLIP_NONE) != 0)throw Error(SDL_GetError());
	//SDL_SetTextureColorMod(t, 40, 40, 40);
}

void Renderer::setModulateBlending(int texture_ID){
	if (textures[texture_ID] == nullptr)throw Error("Bad texture ID!");
	if (SDL_SetTextureBlendMode(textures[texture_ID], SDL_BLENDMODE_MOD) != 0)throw Error(SDL_GetError());
}

void Renderer::applyTexture(int texture_ID, double x, double y, double width, double height) {
	if (textures[texture_ID] == nullptr)throw Error("Bad texture ID!");
	applyTexture(textures[texture_ID], x, y, width, height);
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

int Renderer::loadFont(const char* font, int size){
	TTF_Font *Font;
	if ((Font = TTF_OpenFont(font, size)) == nullptr)throw Error(TTF_GetError());
	fonts[current_fonts] = Font;
	return current_fonts++;
}

void Renderer::displayText(int font, const Uint16* text, RGBA color, double x, double y, double w, double h){
	SDL_Surface *surf;
	SDL_Texture *texture;
	if ((surf = TTF_RenderUNICODE_Blended(fonts[font], text, color.col)) == nullptr)throw Error(TTF_GetError());
	if ((texture = SDL_CreateTextureFromSurface(ren, surf)) == 0)throw Error(SDL_GetError());
	SDL_FreeSurface(surf);
	applyTexture(texture, x, y, w, h);
	SDL_DestroyTexture(texture);
}

void Renderer::setRotationAngle(double deg){
	rotate = deg;
}

Renderer::~Renderer() {
	for (int i = 0; i<current_textures; ++i) SDL_DestroyTexture(textures[i]);
	for (int i = 0; i<current_fonts; ++i) TTF_CloseFont(fonts[i]);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	TTF_Quit();
}

int Renderer::current_textures = 0;
int Renderer::current_fonts = 0;


Game::Game() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)throw Error(SDL_GetError());
}

void Game::loop(){
	SDL_Thread* thread;
	if( (thread = SDL_CreateThread(secondaryLoop, "secondaryThread", (void *)this)) == NULL) throw Error( SDL_GetError() );
	while (!quit) {
		has_event = SDL_PollEvent(&event);
		if (has_event && event.type == SDL_QUIT) quit = true;
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

char Game::getKey(bool pressed){
	if (!has_event)return 0;
	switch (event.type) {
		case SDL_KEYDOWN:
			if (pressed) return event.key.keysym.sym;
		break;
		case SDL_KEYUP:
			if (!pressed) return event.key.keysym.sym;
		break;
		default:
			return 0;
	}
}

Game::~Game(){
	SDL_Quit();
}