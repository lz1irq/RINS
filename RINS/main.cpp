#include "Platform.h"

class RINS : public Game{
	Renderer rend;
	int move;
	double x, y;
	bool w, s, a, d;
	void graphicsLoop() final {
		try{
			rend.applyTexture(move, 0.5+x, 0.5+y, 0.01, 0.01);
			rend.renderScene();
		}
		catch (Error e){
			//these aren't so fatal
			cout << e.getError() << endl;
		}
	}

	void mainLoop() final {
		try {
			if (getKey(1) == 'w')w = true;
			if (getKey(0) == 'w')w = false;
			if (getKey(0) == 's')s = false;
			if (getKey(1) == 's')s = true;
			if (getKey(1) == 'a')a = true;
			if (getKey(0) == 'a')a = false;
			if (getKey(1) == 'd')d = true;
			if (getKey(0) == 'd')d = false;
			if (w)y -= 1/100.0;
			if (s)y += 1/100.0;
			if (a)x -= 1/100.0;
			if (d)x += 1/100.0;

			//cout << x << y << endl;

			SDL_Delay(10);
		}
		catch (Error e) {
			cout << e.getError() << endl;
		}
	}
public:
	RINS() try : rend(640, 640, "RINS") {
		x = 0, y = 0;
		w = false, s = false, a = false, d = false;
		move = rend.loadTexture("Textures/doge.jpeg");
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