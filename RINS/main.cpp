#include "Platform.h"
#include "Being.h"
#include "Map.h"
#include <math.h>
#include <mutex>
class RINS : public Game, public Map{
	Renderer rend;

	int bg[3];
	int wall[3];
	int side[3][2];

	int dir, tmpdir2;
	double lastxpos, lastypos;
	Coord c;
	Being* player;

	list<unique_ptr<Being>> monsters;
	mt19937 pattern;

	int last_tick = 0, timer2 = 0, projectile_tick = 0;

	mutex lock1, lock2;

	int highscore = 0;
	int main_font;

	void graphicsLoop() final {
		try{

			lock1.lock();
			renderMap();
			lock1.unlock();

			int offset = 0;
			if (player->getWalk())offset = 2;
			rend.renderPart(4, 2, offset+((int)log2(player->getOrientation()) >> 1), 1 - ((int)log2(player->getOrientation()) % 2));

			if (player->getWalk())rend.applyTexture(BeingResources::getTextureID(typeid(*player).name()), alterBeingPosX(player->getX()), alterBeingPosY(player->getY()), 1.0 / xsize, 1.0 / ysize);
			else                  rend.applyTexture(BeingResources::getTextureID(typeid(*player).name()), alterBeingPosX(player->getX()), alterBeingPosY(player->getY()), 1.0 / xsize, 1.0 / ysize);
			rend.renderScene();
		}
		catch (Error e){
			cout << e.getError() << endl;
		}
	}

	void mainLoop() final {
		try {

			tmpdir2 = dir;
			getdir();

			lastxpos = player->getX();
			lastypos = player->getY();

			if (getTicks() - last_tick > 33){
				player->move(dir, false);
				last_tick = getTicks();
				if(lastxpos == player->getX() && lastypos == player->getY())player->resetWalk();
			}

			if (updateInternalMapState()) dir = 0;

			int x_colide, y_colide;
			int event = player->checkCollisions(lastxpos, lastypos, getMapIndex(), x_colide, y_colide);
			switch (event){
			case OUT_OF_BOUNDS:
				lock1.lock();
				if (tryRoomChange(x_colide, y_colide)){
					c = getMapEntry();
					player->setX(c.x);
					player->setY(c.y);
					monsters.clear();
				}
				lock1.unlock();
				break;
			case X_COLIDE:
			case Y_COLIDE:
			case XY_COLIDE:
				if(lastxpos == player->getX() && lastypos == player->getY())player->resetWalk();
				break;
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
		if (getKey(1) == ' ')dir |= 1 << 4;
		if (getKey(0) == ' ')dir &= ~(1 << 4);
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
	void renderMap(){
		int maptype = getMapType();
		double deltax = player->getX() - alterBeingPosX(player->getX());
		double deltay = player->getY() - alterBeingPosY(player->getY());
		rend.renderPart(0, 0, 0, 0);
		rend.applyTexture(bg[maptype], - deltax, -deltay, (double)(getMapIndex().size() / (double)xsize), (double)(getMapIndex()[0].size() / (double)ysize));

		for (int i = 0; i < getMapObjects().size(); ++i){
			double x = getMapObjects().at(i).x - deltax;
			double y = getMapObjects().at(i).y - deltay;
			switch (getMapObjects().at(i).type){
			case 1:
				rend.applyTexture(wall[maptype], x - 1.0 / (2 * xsize), y, 1.0 / xsize, 1.0 / ysize);
				rend.applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			case 2:
				rend.applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			case 3:
				rend.applyTexture(wall[maptype], x, y - 1.0 / (2 * ysize), 1.0 / xsize, 1.0 / ysize);
				rend.applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			case 4:
				rend.applyTexture(wall[maptype], x, y - 1.0 / (2 * ysize), 1.0 / xsize, 1.0 / ysize);
				rend.applyTexture(wall[maptype], x - 1.0 / (2 * xsize), y, 1.0 / xsize, 1.0 / ysize);
				rend.applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			default:
				rend.applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			}
		}
		rend.applyTexture(side[maptype][0], -1, 0, 1, 1);
		rend.applyTexture(side[maptype][1], 1 , 0, 1, 1);
	}
public:
	RINS() try : 
		rend(640, 640, "RINS"), dir(0), c(0, 0, 0) {
		loadMap("do u even seed, bro?");
		c = getMapEntry();

		Being::setNumTiles(xsize, ysize);

		int pclass;
		cout << "Chose class: " << endl << "0. Marine " << endl << "1. Pyro " << endl << "2. Psychokinetic" << endl << "3. Android" << endl;
		//cin >> pclass;
		pclass = pclass%3;
		if(pclass == 0) player = new Marine(c.x,c.y);
		else if(pclass == 1) player = new Pyro(c.x, c.y);
		else if(pclass == 2) player = new Psychokinetic(c.x, c.y);
		else if(pclass == 3) player = new Android(c.x, c.y);


		BeingResources::addTextureID(rend.loadTexture("Textures/devil2.png"), typeid(Marine).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/devil2.png"), typeid(Pyro).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/devil2.png"), typeid(Psychokinetic).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/devil2.png"), typeid(Android).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/gangsta.png"), typeid(Zombie).name());

		bg[0] = rend.loadTexture("Textures/floor1.jpg");
		bg[1] = rend.loadTexture("Textures/cement.jpg");
		bg[2] = rend.loadTexture("Textures/dirt2.jpg");

		wall[0]  = rend.loadTexture("Textures/brick3.png");
		wall[1]  = rend.loadTexture("Textures/brick4.png");
		wall[2]  = rend.loadTexture("Textures/brick5.png");

		side[0][0] = rend.loadTexture("Textures/school_1.png");
		side[0][1] = rend.loadTexture("Textures/school_2.png");
		side[1][0] = rend.loadTexture("Textures/hospital_1.png");
		side[1][1] = rend.loadTexture("Textures/hospital_2.png");
		side[2][0] = rend.loadTexture("Textures/forest_1.png");
		side[2][1] = rend.loadTexture("Textures/forest_2.png");

		Projectile::addTexture(BULLET, rend.loadTexture("Textures/bullet.png"));
		Projectile::addTexture(FIRE, rend.loadTexture("Textures/bullet2.png"));

		main_font = rend.loadFont("Fonts/ARIALUNI.TTF", 40);

		Being::monsters[ZOMBIE] = &createInstance<Zombie>;

		Being::targets.push_back(player);


		//::xsize = xsize;
		//::ysize = ysize;

		loop();
	}
	catch (Error e){
		cout << e.getError() << endl;
	}

	~RINS() {
		delete player;
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