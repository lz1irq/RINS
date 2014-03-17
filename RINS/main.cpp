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

	double step = 1.0/64.0;

	vector<unique_ptr<Being>> monsters;
	mt19937 pattern;

	int last_tick = 0, timer2 = 0, projectile_tick = 0;

	mutex lock1;

	void graphicsLoop() final {
		try{
			int maptype = getMapType();
			rend.renderPart(0,0,0,0);
			double deltax = -alterBeingPosX(player->getX());
			double deltay = -alterBeingPosY(player->getY());
			rend.applyTexture(bg[maptype], -(player->getX() + deltax), -(player->getY() + deltay), (double)(getMapIndex().size() / 16.0), (double)(getMapIndex()[0].size() / 16.0));
			lock1.lock();
			for (int i = 0; i < getMapObjects().size(); ++i){
				double x = getMapObjects().at(i).x - (player->getX() + deltax);
				double y = getMapObjects().at(i).y - (player->getY() + deltay);
				switch (getMapObjects().at(i).type){
				case 1:
					 rend.applyTexture(wall[maptype], x - 1.0/(2*xsize), y, 1.0/xsize, 1.0/ysize);
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
					 rend.applyTexture(wall[maptype], x, y, 1.0/xsize, 1.0/ysize);
				 break;
				}
		}
   lock1.unlock();

			rend.renderPart(2,2,(int)log2(lastdir)>>1,1-((int)log2(lastdir)%2));
			rend.applyTexture(BeingResources::getTextureID(typeid(player).name()), alterBeingPosX(player->getX()), alterBeingPosY(player->getY()), 1.0/xsize, 1.0/ysize);


			for (auto &i : monsters) {
				if(i->getHealth()) {
					rend.renderPart(2, 2, (int)log2(i->getOrientation()) >> 1, 1 - ((int)log2(i->getOrientation()) % 2));
					rend.applyTexture(BeingResources::getTextureID(typeid(Zombie).name()), i->getX() -(player->getX() + deltax), i->getY() -(player->getY() + deltay), 1.0 / xsize, 1.0 / ysize);
				}
			}

			for(auto &p : Being::projectiles) {
				rend.applyTexture(Projectile::getTexture(p->getType()),  p->getX() -(player->getX() + deltax), p->getY() -(player->getY() + deltay), 1.0 / xsize, 1.0 / ysize);
			}

			
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



			if(dir != 0) lastdir = dir;


			lastxpos = player->getX();
			lastypos = player->getY();

			if (getTicks() - last_tick > 33 || tmpdir2!= dir){
				player->move(dir, false);
				if(getKey(1) == SDLK_SPACE) player->shootWeapon();
				last_tick = getTicks();

			}

			//monsters.push_back(unique_ptr<Being>(player));
			if(getTicks() - projectile_tick > 33) {
				for(auto &m : monsters) {
					double mx = m->getX()*m->getStep();
					double my = m->getY()*m->getStep();
					for(auto &p : Being::projectiles) {
						double px = p->getX()*step;
						double py = p->getY()*step;
						if((px - mx == step || mx - px == step || px - mx == 0) && (py - my == step || my - py == step || py - my == 0)) {
							m->takeProjectile(p);
						}
						else {
							p->move();
						}
					}
				}


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

			 int a = pattern() % 420;
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
		cout << "Chose class: " << endl << "0. Marine " << endl << "1. Pyro " << endl << "2. Psychokinetic" << endl;
		//cin >> pclass;
		pclass = 0;
		pclass = pclass%3;
		if(pclass == 0) player = new Marine(c.x,c.y);
		else if(pclass == 1) player = new Pyro(c.x, c.y);
		else if(pclass == 2) player = new Psychokinetic(c.x, c.y);

		BeingResources::addTextureID(rend.loadTexture("Textures/devil.png"), typeid(Marine).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/devil.png"), typeid(Pyro).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/devil.png"), typeid(Psychokinetic).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/gangsta.png"), typeid(Zombie).name());

		bg[0] = rend.loadTexture("Textures/floor1.jpg");
		bg[1] = rend.loadTexture("Textures/cement.jpg");
		bg[2] = rend.loadTexture("Textures/dirt2.jpg");

		wall[0]  = rend.loadTexture("Textures/brick3.png");
		wall[1]  = rend.loadTexture("Textures/brick4.png");
		wall[2]  = rend.loadTexture("Textures/brick5.png");

		Projectile::addTexture(BULLET, rend.loadTexture("Textures/bullet.png"));

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