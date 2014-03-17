#include "Platform.h"
#include "Being.h"
#include "Map.h"
#include <math.h>
#include <mutex>
class RINS : public Game, public Map{
	Renderer rend;

	int bg[3];
	int wall[3];

	int dir, lastdir = 1, tmpdir2;
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
			int maptype = getMapType();
			rend.renderPart(0, 0, 0, 0);
			double deltax = -alterBeingPosX(player->getX());
			double deltay = -alterBeingPosY(player->getY());
			rend.applyTexture(bg[maptype], -(player->getX() + deltax), -(player->getY() + deltay), (double)(getMapIndex().size() / 16.0), (double)(getMapIndex()[0].size() / 16.0));
			for (int i = 0; i < getMapObjects().size(); ++i){
				double x = getMapObjects().at(i).x - (player->getX() + deltax);
				double y = getMapObjects().at(i).y - (player->getY() + deltay);
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
			lock1.unlock();
			lock2.lock();
			rend.renderPart(2, 2, (int)log2(lastdir) >> 1, 1 - ((int)log2(lastdir) % 2));
			rend.applyTexture(BeingResources::getTextureID(typeid(player).name()), alterBeingPosX(player->getX()), alterBeingPosY(player->getY()), 1.0 / xsize, 1.0 / ysize);


			for (auto &i : monsters) {
				if (i->getHealth()){
					rend.renderPart(2, 2, (int)log2(i->getOrientation()) >> 1, 1 - ((int)log2(i->getOrientation()) % 2));
					rend.applyTexture(BeingResources::getTextureID(typeid(Zombie).name()), i->getX() - (player->getX() + deltax), i->getY() - (player->getY() + deltay), 1.0 / xsize, 1.0 / ysize);
				}
			}

			for (auto &p : Being::projectiles) {
				rend.applyTexture(Projectile::getTexture(p.getType()), p.getX() - (player->getX() + deltax), p.getY() - (player->getY() + deltay), player->getStep(), player->getStep());
			}

			lock2.unlock();

			rend.renderPart(0, 0, 0, 0);
			rend.displayText(main_font, itow(highscore), RGBA(0, 0, 255, 0), 0.01, 0.01, 0, 0);

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



			if(dir != 0 && dir < 16) lastdir = dir;


			lastxpos = player->getX();
			lastypos = player->getY();

			if (getTicks() - last_tick > 33 || tmpdir2!= dir){
				player->move(dir, false);
				if(dir & 16)player->shootWeapon();
				last_tick = getTicks();

			}

			//monsters.push_back(unique_ptr<Being>(player));
			if(getTicks() - projectile_tick > 1) {
				A:
				lock2.lock();
				for (auto p = begin(Being::projectiles); p != end(Being::projectiles); ++p){
					int px = p->getX() / player->getStep();
					int py = p->getY() / player->getStep();

					int pblockx = ((p->getX() + player->getStep()) / player->getStep()) / ((1.0 / xsize) / player->getStep());
					int pblocky = ((p->getY() + player->getStep()) / player->getStep()) / ((1.0 / ysize) / player->getStep());

					if (p->modFlyTime()){
						p->modFlyTime() -= 1;
						if(!getMapIndex()[pblockx][pblocky])p->move(player->getStep());
					}

					if (p->modDetonationTime()){
						p->modDetonationTime() -= 1;
					}
					else{
						p = Being::projectiles.erase(p);
						continue;
					}
					for (auto m = begin(monsters); m != end(monsters); ++m){

						int mx = (*m)->getX() / (*m)->getStep() + 2;
						int my = (*m)->getY() / (*m)->getStep() + 2;

						if (abs(mx - px) < 2 && abs(my - py) < 2){
							(*m)->takeProjectile(*p);
							p = Being::projectiles.erase(p);
							if (p == end(Being::projectiles))break;
							if (!(*m)->getHealth()){
								
								highscore++;
								lock1.lock();
								m = monsters.erase(m);
								
								lock2.unlock();
								lock1.unlock();
								goto A;
								break;
								//m = monsters.erase(m);
								//break;
							}

						}
					}
				}
				projectile_tick = getTicks();
				lock2.unlock();
			}
			//monsters.pop_back();

			if (updateInternalMapState()) dir = 0;
			int pos_tile_x = ((player->getX() + player->getStep()) / player->getStep()) / ((1.0 / xsize) / player->getStep());
			int pos_tile_y = ((player->getY() + player->getStep() * 3) / player->getStep()) / ((1.0 / ysize) / player->getStep());

			bool mustlock = false;
			if (!(pos_tile_x < 0 || pos_tile_x >= getMapIndex().size())){
				if (!(pos_tile_y < 0 || pos_tile_y >= getMapIndex()[pos_tile_x].size())){
					if (getMapIndex()[pos_tile_x][pos_tile_y]){
						player->move(dir, true);
					}
				}
				else { 
					player->setX(lastxpos);
					mustlock = true; 
				}
			}
			else {
				player->setY(lastypos);
				mustlock = true;
			}
			
			 if (mustlock){
				lock1.lock();
				if (tryRoomChange(pos_tile_x, pos_tile_y)){
					c = getMapEntry();
					player->setX(c.x);
					player->setY(c.y);
					monsters.clear();
				}
				mustlock = false;
				lock1.unlock();
			}

			 int a = pattern() % 42;
			 int b = pattern() % Being::monsters.size();

			 int x, y;

			int xsz = getMapIndex().size();
			int ysz = getMapIndex()[0].size();
			x = pattern() % xsz;
			y = pattern() % ysz;

			if (!a && !getMapIndex()[x][y]){
				monsters.push_back(unique_ptr<Being>(Being::monsters[b]((double)x / xsize, (double)y / ysize)));
				 //cout << monsters.size() << endl;
			 }


			if (getTicks() - timer2 > 66){
				for (auto &i : monsters) {
					if(!i->getHealth()) continue;
					else i->action(getMapIndex());
				}
				timer2 = getTicks();
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
public:
	RINS() try : 
		rend(640, 640, "RINS"), dir(0) {
		loadMap("do u even seed, bro?");
		c = getMapEntry();

		int pclass;
		cout << "Chose class: " << endl << "0. Marine " << endl << "1. Pyro " << endl << "2. Psychokinetic" << endl << "3. Android" << endl;
		
		pclass = pclass%3;
		if(pclass == 0) player = new Marine(c.x,c.y);
		else if(pclass == 1) player = new Pyro(c.x, c.y);
		else if(pclass == 2) player = new Psychokinetic(c.x, c.y);
		else if(pclass == 3) player = new Android(c.x, c.y);
		cin >> pclass;

		BeingResources::addTextureID(rend.loadTexture("Textures/devil.png"), typeid(Marine).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/devil.png"), typeid(Pyro).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/devil.png"), typeid(Psychokinetic).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/devil.png"), typeid(Android).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/gangsta.png"), typeid(Zombie).name());

		bg[0] = rend.loadTexture("Textures/floor1.jpg");
		bg[1] = rend.loadTexture("Textures/cement.jpg");
		bg[2] = rend.loadTexture("Textures/dirt2.jpg");

		wall[0]  = rend.loadTexture("Textures/brick3.png");
		wall[1]  = rend.loadTexture("Textures/brick4.png");
		wall[2]  = rend.loadTexture("Textures/brick5.png");

		Projectile::addTexture(BULLET, rend.loadTexture("Textures/bullet.png"));
		Projectile::addTexture(FIRE, rend.loadTexture("Textures/bullet.png"));

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