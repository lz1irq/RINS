#include "Platform.h"
#include "Being.h"
#include <math.h>

class RINS : public Game{
	Renderer rend;
	int move;
	int dir;
	int main_font;
	struct being_resources{
		Being b;
		int texture;
		being_resources(Being b, int texture): b(b), texture(texture){}
	};
	//========game
	being_resources Player;
	unsigned int last_tick;
	//========game
	void graphicsLoop() final {
		try{
			int x, y;
			int ori = Player.b.getOrientation();
			if (ori == 8){ x = 0; y = 0; }
			if (ori == 4){ x = 1; y = 0; }
			if (ori == 2){ x = 0; y = 1; }
			if (ori == 1){ x = 1; y = 1; }

			rend.renderPart(2, 2, x, y);
			//rend.applyTexture(Player.texture, 0, 0, 1, 1);
			rend.applyTexture(Player.texture, Player.b.getX(), Player.b.getY(), Player.b.getStep()*4, Player.b.getStep()*4);
			rend.renderPart(1, 1, 0, 0);

			rend.renderScene();
		}
		catch (Error e){
			//these aren't so fatal
			cout << e.getError() << endl;
		}
	}

	void mainLoop() final {
		try {
			int tmpdir = dir;
			getdir();

			if (getTicks() - last_tick > 33 || tmpdir!= dir){
				Player.b.move(dir);
				last_tick = getTicks();
			}
			SDL_Delay(10);
		}
		catch (Error e) {
			cout << e.getError() << endl;
		}
	}

	void getdir(){
		if (getKey(1) == 'a')dir |= 1 << 0;
		if (getKey(0) == 'a')dir &= ~(1 << 0);
		if (getKey(1) == 'd')dir |= 1 << 1;
		if (getKey(0) == 'd')dir &= ~(1 << 1);
		if (getKey(1) == 'w')dir |= 1 << 2;
		if (getKey(0) == 'w')dir &= ~(1 << 2);
		if (getKey(1) == 's')dir |= 1 << 3;
		if (getKey(0) == 's')dir &= ~(1 << 3);
	}
	Uint16* itow(unsigned int h){
		int a = log10(h) + 1;
		Uint16* text = new Uint16[a + 1];
		for (int i = a - 1; i >= 0; --i){
			text[i] = (h % 10) + 48;
			h /= 10;
		}
		text[a] = 0;
		return text;
	}
public:
	RINS() try : 
		rend(640, 480, "RINS"), dir(0), 
		Player(Being(0, 0), rend.loadTexture("Textures/devil.png")) {
		move = rend.loadTexture("Textures/testure.png");
		last_tick = getTicks();
		main_font = rend.loadFont("Fonts/ARIALUNI.ttf", 15);
		loop();
	}
	catch (Error e){
		cout << e.getError() << endl;
	}
};

int main(int argc, char** argv) {
	try{
		RINS rins;
	}
	catch (...){
		system("pause");
	}
	return 0;
}