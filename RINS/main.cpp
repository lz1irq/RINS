#include "Platform.h"

class RINS : public Game{
	Renderer rend;
	void graphicsLoop() final {
		try{

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