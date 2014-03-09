#include "Platform.h"
#include "Being.h"

class RINS : public Game{
	Renderer rend;
	int move;
	double x, y;
	int dir;
	struct being_resources{
		Being b;
		int texture;
	};
	void graphicsLoop() final {
		try{
			rend.applyTexture(move, 0.45+x, 0.45+y, 0.1, 0.1);
			rend.renderScene();
		}
		catch (Error e){
			//these aren't so fatal
			cout << e.getError() << endl;
		}
	}

	void mainLoop() final {
		try {
			getdir();
			if (dir & 4)y -= 1/100.0;
			if (dir & 8)y += 1/100.0;
			if (dir & 1)x -= 1/100.0;
			if (dir & 2)x += 1/100.0;

			//cout << x << y << endl;

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
public:
	RINS() try : rend(640, 640, "RINS"), dir(0) {
		x = 0, y = 0;
		move = rend.loadTexture("Textures/testure.png");
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