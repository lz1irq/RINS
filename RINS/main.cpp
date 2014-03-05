#include "Platform.h"

class RINS : public Game{
	int doge;
	Renderer rend;
	char ak;
	unsigned int a;
	void graphicsLoop() final {
		try{
			++ak;
			if (ak < 0){
				ak = 0;
				bool A = a & 1;
				bool B = a & 2;
				++a;
				rend.renderPart(2, 2, A, B);
			}
			rend.applyTexture(doge, 0.25, 0.25, 0.5, 0.5);
			rend.renderScene();
		}
		catch (Error e){
			//these aren't so fatal
			cout << e.getError() << endl;
		}
	}

	void mainLoop() final {
		try {
			SDL_Delay(10);
		}
		catch (Error e) {
			cout << e.getError() << endl;
		}
	}
public:
	RINS() try : rend(640, 640, "RINS") {
		doge = rend.loadTexture("Textures/testure.png");
		a = 0;
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