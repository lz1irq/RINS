#include "Graphics.h"
#include <iostream>
using namespace std;
int main(int argc, char** argv) {
	Graphics gui(640,640, "RINS");
	int doge = gui.loadTexture("doge.jpeg");
	cout << doge;
	SDL_Event event;
	bool quit = false;
	while(!quit) {
		while (SDL_PollEvent(&event) ) if ( event.type == SDL_QUIT ) quit = true;
		gui.applyTexture(doge, 10, 100);
		gui.renderScene();
	}
	return 0;
}