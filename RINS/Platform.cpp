#include "Platform.h"

namespace shared_sdl{
	int W, H;
}

Error::Error(const char* err) : err(err){}

const char* Error::getError(){ return err; }

Renderer::Renderer(int width, int height, const char* title) : textures(), fonts(){
	if ((win = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI)) == nullptr)throw Error(SDL_GetError());
	if ((ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == nullptr)throw Error(SDL_GetError());
	SDL_GetWindowSize(win, &W, &H);
	shared_sdl::W = W;
	shared_sdl::H = H;
	part.x = H;
	part.y = H;
	cout << W << " " << H << endl;
	if (TTF_Init())throw Error(TTF_GetError());
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
	if (SDL_QueryTexture(t, NULL, NULL, &dst.w, &dst.h))throw Error(SDL_GetError());
	dst.x = H*x;
	dst.y = H*y;
	if (part.x && part.y){
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
	if (w && h){
		dst.w = w;
		dst.h = h;
	}
	dst.x += (W - H) / 2;
	if (SDL_RenderCopyEx(ren, t, &src, &dst, rotate, NULL, SDL_FLIP_NONE))throw Error(SDL_GetError());
	//SDL_SetTextureColorMod(t, 40, 40, 40);
}

void Renderer::setModulateBlending(int texture_ID){
	if (textures[texture_ID] == nullptr)throw Error("Bad texture ID!");
	if (SDL_SetTextureBlendMode(textures[texture_ID], SDL_BLENDMODE_MOD))throw Error(SDL_GetError());
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
	if (SDL_RenderClear(ren))throw Error(SDL_GetError());
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

void Renderer::getTextWH(int font, const Uint16* text, double& w, double& h){
	int wi, hi;
	if (TTF_SizeUNICODE(fonts[font], text, &wi, &hi))throw Error(TTF_GetError());

	w = wi / (double)H;
	h = hi / (double)H;
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
int Audio::current_sounds = 0;
int Audio::current_songs = 0;


Game::Game() {
	if (SDL_Init(SDL_INIT_EVERYTHING))throw Error(SDL_GetError());
	SDL_EventState(SDL_KEYUP, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);

}

void Game::loop(){
	SDL_Thread* thread;
	char curs;
	int offs;
	char tmp[1024] = { 0 };
	if( (thread = SDL_CreateThread(secondaryLoop, "secondaryThread", (void *)this)) == NULL) throw Error( SDL_GetError() );
	while (!quit) {
		//has_event = SDL_PollEvent(&event);
		if (SDL_PollEvent(&event)){
			switch (event.type){
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym){
				case SDLK_BACKSPACE:
					if (cursor == 0)break;
					offs = strlen(utf8text) - 1 - cursor;
					memset(tmp, 0, strlen(tmp));
					memcpy(tmp, &utf8text[cursor], strlen(&utf8text[cursor]));
					utf8text[cursor+offs] = 0;
					--cursor;
					while (((unsigned char)utf8text[cursor] >> 6) == 2){
						utf8text[cursor+offs] = 0;
						--cursor;
					}
					memcpy(&utf8text[cursor], tmp, strlen(tmp));
					break;
				case SDLK_RETURN:
					ret_text = true;
					break;
				case SDLK_LEFT:
					if (cursor == 0)break;
					curs = utf8text[cursor];
					--cursor;
					while (((unsigned char)utf8text[cursor] >> 6) == 2){
						utf8text[cursor + 1] = utf8text[cursor];
						--cursor;
					}
					utf8text[cursor + 1] = utf8text[cursor];
					utf8text[cursor] = curs;
					break;
				case SDLK_RIGHT:
					if (cursor == strlen(utf8text)-1)break;
					curs = utf8text[cursor];
					++cursor;
					utf8text[cursor - 1] = utf8text[cursor];
					while (((unsigned char)utf8text[cursor + 1] >> 6) == 2){
						++cursor;
						utf8text[cursor - 1] = utf8text[cursor];
					}
					utf8text[cursor] = curs;
					break;
				}
				break;
			case SDL_TEXTINPUT:
				if (strlen(utf8text) + strlen(event.text.text) > maxtext)break;
				memset(tmp, 0, strlen(tmp));
				memcpy(tmp, &utf8text[cursor], strlen(&utf8text[cursor]));
				utf8text[cursor] = 0;
				strcat(utf8text, event.text.text);
				strcat(utf8text, tmp);
				cursor += strlen(event.text.text);
				break;
			}
		}
		buttons = SDL_GetMouseState(&mousex, &mousey);
		if (SDL_QuitRequested()) quit = true;
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

void Game::startTyping(const char* initial){
	//if (maxlen > 1023)maxlen = 1023;
	maxtext = 1023;
	SDL_StartTextInput();
	memset(utf8text, 0, 1024);
	strcat(utf8text, initial);
	cursor = strlen(utf8text);
	utf8text[cursor] = '|';
	ret_text = false;
}

const char* Game::getText(){
	return utf8text;
}


const char* Game::getRawText(bool& ret){
	ret = ret_text;
	char tmp[1024] = { 0 };
	memcpy(tmp, &utf8text[cursor], strlen(&utf8text[cursor]));
	memset(raw, 0, strlen(raw));
	memcpy(raw, utf8text, cursor);
	strcat(raw, &tmp[1]);
}

void Game::endTyping(bool reset){
	if(reset)memset(utf8text, 0, strlen(utf8text));
	ret_text = false;
	SDL_StopTextInput();
}

bool Game::isPressed(const char* key){
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	return state[SDL_GetScancodeFromName(key)];
}

double Game::getMouseX(){
	return (mousex - ((shared_sdl::W - shared_sdl::H) / 2)) / (double)shared_sdl::H;
}

double Game::getMouseY(){
	return mousey / (double)shared_sdl::H;
}

bool Game::getLeftClick(){
	return buttons&SDL_BUTTON_LMASK;
}

bool Game::getRightClick(){
	return buttons&SDL_BUTTON_RMASK;
}

Game::~Game(){
	SDL_Quit();
}

Audio::Audio() {
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1) throw Error(Mix_GetError());
}

int Audio::loadSong(const char* song) {
	if(current_songs == MAX_SONGS) throw Error(Mix_GetError());
	if((songs[current_songs] = Mix_LoadMUS(song)) == nullptr) throw Error(Mix_GetError());
	return current_songs++;
}

int Audio::loadSound(const char* sound) {
	if(current_sounds == MAX_SOUNDS) throw Error(Mix_GetError());
	if((sounds[current_sounds] = Mix_LoadWAV(sound)) == nullptr) throw Error(Mix_GetError());
	return current_sounds++;
}

void Audio::playSound(int sound_id) {
	if(Mix_PlayChannel(-1, sounds[sound_id], 0) == -1) throw Error(Mix_GetError());
}

void Audio::setMusicVolume(int vol) {
	Mix_VolumeMusic(vol);
}

void Audio::playSong(int song_id) {
	if( Mix_PlayingMusic() == 0 ) {
		if( Mix_PlayMusic(songs[song_id], -1 ) == -1 ) throw Error(Mix_GetError());
	}
}

bool Audio::isPlayingMusic() {
	return Mix_PlayingMusic();
}

void Audio::pauseMusic() {
	if(Mix_PlayingMusic())Mix_PauseMusic();
}
void Audio::resumeMusic() {
	Mix_ResumeMusic();
}

void Audio::stopMusic() {
	if(Mix_HaltMusic() == -1) throw Error(Mix_GetError());
}

Audio::~Audio(){
	Mix_Quit();
}

Socket::Socket(){
	if (SDLNet_Init())throw Error(SDLNet_GetError());
	if ((socketset = SDLNet_AllocSocketSet(16)) == nullptr)throw Error(SDLNet_GetError());
}

void Socket::startServer(int port){
	if (SDLNet_ResolveHost(&ip, NULL, port))throw Error(SDLNet_GetError());
	if (!(sd = SDLNet_TCP_Open(&ip)))throw Error(SDLNet_GetError());
}

int Socket::gatherPlayers(){
	TCPsocket csd;
	if ((csd = SDLNet_TCP_Accept(sd))){
		int i;
		if ((i = SDLNet_TCP_AddSocket(socketset, csd)) == -1)throw Error(SDLNet_GetError());
		else{
			++numused;
			clients.push_back(*new Client(csd));
		}
	}
	return numused;
}

char* Socket::getNextCommand(Client& c){
	if (c.len < 4){
		return nullptr;
	}
	unsigned short extract;
	memcpy(&extract, &c.buf[2], 2);
	extract += 4;
	if (extract > c.len){
		return nullptr;
	}
	c.len -= extract;
	memcpy(command, c.buf, extract);
	memmove(c.buf, &c.buf[extract], c.len);
	return command;

}

void Socket::updateClients(){
	int active;
	if (!numused)return;
	if ((active = SDLNet_CheckSockets(socketset, 1)) == -1)throw Error(SDLNet_GetError());
	else if(active > 0){
		for (auto i = begin(clients); i != end(clients); ++i){
			if (SDLNet_SocketReady((*i).sock)){
				int len;
				if ((*i).len == (*i).bf)continue;
				if ((len = SDLNet_TCP_Recv((*i).sock, &(*i).buf[(*i).len], (*i).bf - (*i).len)) > 0){
					(*i).len += len;
				}
				else {
					if ((active=SDLNet_TCP_DelSocket(socketset, (*i).sock)) == -1)throw Error(SDLNet_GetError());
					else cout << active << endl;
					i = clients.erase(i);
					--numused;
					return;
				}
			}
		}
	}
}

void Socket::ConnectToServer(int port, const char* ip_) {
	if (SDLNet_ResolveHost(&ip, ip_, port))throw Error(SDLNet_GetError());
	if (!(sd = SDLNet_TCP_Open(&ip)))throw Error(SDLNet_GetError());

}

void Socket::sendCommand(short num, short datasz, const char* data){
	char *buf = new char[datasz + 4];
	memcpy(&buf[0], &num, 2);
	memcpy(&buf[2], &datasz, 2);
	memcpy(&buf[4], data, datasz);
	sendToServer(buf, datasz+4);
	delete buf;
}

void Socket::sendToServer(char* text, int len){
	if (SDLNet_TCP_Send(sd, (void *)text, len) < len){
		SDLNet_TCP_Close(sd);
		throw Error(SDLNet_GetError());
	}
}

list<Socket::Client>& Socket::getClients(){
	return clients;
}

void Socket::disconncet(){
	SDLNet_TCP_Close(sd);
}

Socket::~Socket(){
	SDLNet_Quit();
}