#include "Platform.h"
#include "Being.h"
#include <math.h>

class RINS : public Game{
	Renderer rend;
	int move;
	int dir;
	int main_font;
	int bg;
	struct being_resources{
		Being b;
		int texture;
		int x,y;
		being_resources(Being b, int texture): b(b), texture(texture){}
	};
	//========game
	being_resources Player;
	being_resources Mob;
	unsigned int last_tick;
	unsigned mticks;
	//========game

	void set_texture(being_resources& bres) {
			int ori = bres.b.getOrientation();
			if (ori == 8){ bres.x = 0; bres.y = 0; }
			if (ori == 4){ bres.x = 1; bres.y = 0; }
			if (ori == 2){ bres.x = 0; bres.y = 1; }
			if (ori == 1){ bres.x = 1; bres.y = 1; }
	}

	unsigned int get_tile_x(being_resources bres) {
		return (bres.b.getX()/bres.b.getStep() + bres.b.getStep()*2)/4;
	}
	unsigned int get_tile_y(being_resources bres) {
		return (bres.b.getY()/bres.b.getStep() + bres.b.getStep()*4)/4;
	}

	void graphicsLoop() final {
		try{

			set_texture(Player);
			set_texture(Mob);

			rend.renderPart(0, 0, 0, 0);
			rend.applyTexture(bg,0,0,1,1);

			rend.renderPart(2,2,Mob.x,Mob.y);
			rend.applyTexture(Mob.texture, Mob.b.getX(), Mob.b.getY(), Mob.b.getStep()*4, Mob.b.getStep()*4);

			rend.renderPart(2, 2, Player.x, Player.y);
			rend.applyTexture(Player.texture, Player.b.getX(), Player.b.getY(), Player.b.getStep()*4, Player.b.getStep()*4);


			Uint16* xtext = itow(get_tile_x(Player));
			Uint16* ytext = itow(get_tile_y(Player));
			rend.renderPart(0, 0, 0, 0);
			rend.displayText(main_font, xtext, RGBA(1,127,255,1), 0.1, 0.1, 0, 0);
			rend.displayText(main_font, ytext, RGBA(1,127,255,1), 0.3, 0.1, 0, 0);
			delete [] xtext;
			delete [] ytext;
			rend.renderScene();
		}
		catch (Error e){
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

			if(getTicks() - mticks > 80) {
				Mob.b.stepTo(Player.b.getX(),Player.b.getY());
				mticks = getTicks();
				
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
		if(a < 0) a = 1;
		Uint16* text = new Uint16[a + 1];
		for (int i = a - 1; i >= 0; --i){
			text[i] = (h % 10) + 48;
			h /= 10;
		}
		if(text[0] == 0) text[0] = '0';
		text[a] = 0;
		return text;
	}
public:
	RINS() try : 
		rend(1024, 768, "RINS"), dir(0), 
		Player(Being(0, 0), rend.loadTexture("Textures/devil.png")),
		Mob(Being(0.7,0.7),rend.loadTexture("Textures/gangsta.png")) {
		move = rend.loadTexture("Textures/testure.png");
		last_tick = getTicks();
		mticks = getTicks();
		main_font = rend.loadFont("Fonts/ARIALUNI.ttf", 70);
		bg = rend.loadTexture("Textures/bg.jpg");

		//marine class weapons

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