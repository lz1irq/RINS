#include "Graphics.h"
#include <iostream>
using namespace std;

class SDLerror{
	const char* err;
public:
	SDLerror(const char* err): err(err){}
	const char* getError(){ return err; }
};

class Game{
	SDL_Event event;
public:
	virtual void mainLoop() = 0;
	Game(){
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0)throw SDLerror(SDL_GetError());
	}
	void loop(){
		bool quit = false;
		while (!quit) {
			while (SDL_PollEvent(&event)) if (event.type == SDL_QUIT) quit = true;
			mainLoop();
		}
	}
	~Game(){
		SDL_Quit();
	}
};


class RINS : public Game{
	int doge;
	Graphics gui;
	void mainLoop() final{
		gui.applyTexture(doge, 10, 100);
		gui.renderScene();
	}
public:
	RINS() : gui(640, 640, "RINS"){
		doge = gui.loadTexture("doge.jpeg");
		loop();
	}
};




int main(int argc, char** argv) {
	RINS rins;
	return 0;
}