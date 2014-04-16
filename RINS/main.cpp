#include "Platform.h"
#include "Being.h"
#include "Map.h"
#include "Menu.h"
#include "Machine.h"
#include <math.h>
#include <mutex>

using namespace std;
class RINS : public Game, public Renderer, public Audio, public Map{

	int bg[3];
	int wall[3];
	int side[3][2];
	int entrytex, exittex, red, vendtex, droptex;

	int dir, tmpdir2;
	double lastxpos, lastypos, deltax, deltay;
	Coord c;
	Being* player;

	list<unique_ptr<Being>> monsters;
	list<unique_ptr<Being>> targets;
	mt19937 pattern;

	int last_tick = 0, timer2 = 0, projectile_tick = 0;

	mutex lock1, monster, projectile, menux;

	int highscore = 0, spawned = 0, lastroom = 0;
	int main_font;
	bool completed = false;

	bool cangetpress = true, pressed = false;

	Hitbox box;

	Being* curr_target = nullptr;
	array<Being*(*)(double, double), MAXSIZE> monster_types;
	array<Item*(*)(), MAXITEMS> item_types;
	list<Projectile> projectiles;

	int menu_bg, button, overlay, b2, b3;
	Menu menu;
	Menu* curr_m;
	int  menu_select = -1;
	double optionysize = 0.1;
	double optionspacing = 0.01;
	double optionxsize = 0.3;
	double yoffset;
	double texth = 0.08;
	bool show_menu = true;
	bool enable_music = false;

	int song1;
	map<pair<int, int>, Machine> machines;

	void renderHUD() {
		RGBA mecol(128, 0, 0, 0);
		displayText(main_font, (Uint16*)L"HEALTH:",mecol, 0.002, 0.9,0,0);
		displayText(main_font, (Uint16*)to_wstring(player->getHealth()).c_str(),mecol, 0.005, 0.94,0,0);
		displayText(main_font, (Uint16*)L"SCORE:",mecol, 0.84, 0.9,0,0);
		displayText(main_font, (Uint16*)to_wstring(highscore).c_str(),mecol, 0.89, 0.94,0,0);
	}

	void graphicsLoop() final {
		try{

			lock1.lock();
			renderMap();
			lock1.unlock();

			int offset = 0;
			if (player->getWalk())offset = 2;
			renderPart(4, 2, offset + ((int)log2(player->getOrientation()) >> 1), 1 - ((int)log2(player->getOrientation()) % 2));

			applyTexture(BeingResources::getTextureID(typeid(*player).name()), alterBeingPosX(player->getX()), alterBeingPosY(player->getY()), 1.0 / xsize, 1.0 / ysize);

			monster.lock();//lock the monster!
			for (auto &i : monsters) {
				if (i->getWalk())renderPart(4, 2, 2 + ((int)log2(i->getOrientation()) >> 1), 1 - ((int)log2(i->getOrientation()) % 2));
				else renderPart(4, 2, ((int)log2(i->getOrientation()) >> 1), 1 - ((int)log2(i->getOrientation()) % 2));
				applyTexture(BeingResources::getTextureID(typeid(*i).name()), i->getX() - deltax, i->getY() - deltay, 1.0 / xsize, 1.0 / ysize);
			}
			monster.unlock();
			renderPart(0, 0, 0, 0);
			projectile.lock();
			for (auto &i : projectiles){
				setRotationAngle(i.getAngleInDeg());
				applyTexture(WeaponResources::getTexture(i.getType()), i.getX() - deltax + 1.5*player->getStepX(), i.getY() - deltay + 1.5*player->getStepY(), player->getStepX(), player->getStepY());
				setRotationAngle(0);
			}
			projectile.unlock();

			applyTexture(side[getMapType()][0], -1, 0, 1, 1);
			applyTexture(side[getMapType()][1], 1, 0, 1, 1);

			if (show_menu){
				menux.lock();
				renderMenu();
				menux.unlock();
			}
			lock1.lock();
			renderHUD();
			lock1.unlock();
			renderScene();
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

			if (updateInternalMapState()) dir = 0;

			if (getTicks() - last_tick > 33){
				player->move(dir, false);
				last_tick = getTicks();
				if(lastxpos == player->getX() && lastypos == player->getY())player->resetWalk();
				else{
					int event = player->checkCollisions(lastxpos, lastypos, getMapIndex());
					switch (event){
					case OUT_OF_BOUNDS:
						if (!completed)break;
						lock1.lock();
						if (tryRoomChange(player->getTileX(), player->getTileY())){
							c = getMapEntry();
							player->setX(c.x);
							player->setY(c.y);
							if (getLastExploredRoom() > lastroom){
								completed = false;
								spawned = 0;
								machines.clear();
							}
						}
						lock1.unlock();
						break;
					case X_COLLIDE:
					case Y_COLLIDE:
					case XY_COLLIDE:
						if (lastxpos == player->getX() && lastypos == player->getY())player->resetWalk();
						break;
					case TRIGGER:
						if (getMapIndex()[player->getTileX()][player->getTileY()] == VENDING || getMapIndex()[player->getTileX()][player->getTileY()] == DROP){//dropped items are free and can contain money?
							pair<int, int> p = make_pair(player->getTileX(), player->getTileY());
							if (machines.find(p) == machines.end()){
								machines[p] = *new Machine();
								int items = pattern() % 10;
								for (int i = 0; i < 10; ++i){
									int item = pattern() % MAXITEMS;
									machines[p].addItem(*item_types[item]());
								}
							}
							else{

							}
						}
						break;
					}
				}
			}
			if (dir & 16){
				if (getTicks() - projectile_tick > 15){
					Projectile* p;
					int event = player->tryToShoot(curr_target, &p);
					switch (event){
					case BANG:
						//projectile.lock();
						projectiles.push_back(*p);
						//projectile.unlock();
					}
					projectile_tick = getTicks();
				}
			}
			else projectile_tick = getTicks();
			for (auto p = begin(projectiles); p != end(projectiles); ++p){
				bool res = p->update(getMapIndex(), monsters, targets);
				if (!res){
					projectile.lock();
					p = projectiles.erase(p);
					projectile.unlock();
				}
			}

			bool mustspawn = pattern()%getSpawnRate()==false;
			int spawntype = pattern()%monster_types.size();
			int x = pattern()%getMapIndex().size();
			int y = pattern()%getMapIndex()[x].size();
			if(mustspawn && !getMapIndex()[x][y]){
				if (spawned == getMaxMonsters()){
					if (monsters.size() == 0){
						completed = true;
						lastroom = getLastExploredRoom();
					}
				}
				else{
					monster.lock();
					monsters.push_back(unique_ptr<Being>(monster_types[spawntype]((double)x / xsize, (double)y / ysize)));
					++spawned;
					monster.unlock();
				}
			}
			int tar = 0;
			monster.lock();
			for (auto m = begin(monsters); m != end(monsters); ++m, ++tar){
				bool res = (*m)->action(getMapIndex(), projectiles, targets, getTicks());
				if (!res){
					addLoot((*m)->getTileX(), (*m)->getTileY());
					m = monsters.erase(m);
					++highscore;
					curr_target = nullptr;
				}
				else{
					if (mouseOverTarget((*m)->getX(), (*m)->getY()) && pressed){
						curr_target = &**m;
					}
				}
			}
			monster.unlock();

			if (show_menu){
				menux.lock();
				checkMenu();
				menux.unlock();
			}

			updatePress();
			SDL_Delay(10);
		}
		catch (Error e) {
			cout << e.getError() << endl;
		}
	}

	void renderMenu(){
		applyTexture(menu_bg, 0, 0, 1, 1);
		yoffset = (curr_m->getNumOptions() - 1)*(optionysize + optionspacing) / 2.0;
		double w, h;
		RGBA mecol(255, 100, 255, 0);
		for (int i = 0; i < curr_m->getNumOptions(); ++i){
			Button* bb = dynamic_cast<Button*>(&curr_m->selectOption(i));
			Checkbox* cc = dynamic_cast<Checkbox*>(&curr_m->selectOption(i));
			if (curr_m->getHitbox().at(i))mecol.setR(255);
			else mecol.setR(0);
			if (bb || cc){
				int tex;
				if (bb)tex = button;
				if (cc){
					if (cc->access())tex = b2;
					else tex = b3;
				}
				applyTexture(tex, 0.5 - optionxsize / 2.0, 0.5 + (optionysize + optionspacing)*i - (optionysize + optionspacing) / 2.0 - yoffset, optionxsize, optionysize);
				getTextWH(main_font, (Uint16*)curr_m->selectOption(i).getText().c_str(), w, h);
				if (curr_m->getHitbox().at(i)){
					applyTexture(overlay, 0.5 - optionxsize / 2.0, 0.5 + (optionysize + optionspacing)*i - (optionysize + optionspacing) / 2.0 - yoffset, optionxsize, optionysize);
					getTextWH(main_font, (Uint16*)curr_m->selectOption(i).getText().c_str(), w, h);
				}
				displayText(main_font, (Uint16*)curr_m->selectOption(i).getText().c_str(),
					mecol, 0.5 -w/2 , 0.5 + (optionysize + optionspacing)*i - (optionysize + optionspacing) / 2.0 - yoffset +h/2 , w, h);
			}
		}
	}

	void checkMenu(){
		double w, h;
		for (int i = 0; i < curr_m->getNumOptions(); ++i){
			Button* bb = dynamic_cast<Button*>(&curr_m->selectOption(i));
			Checkbox* cz = dynamic_cast<Checkbox*>(&curr_m->selectOption(i));
			if (bb || cz){
				curr_m->getHitbox().at(i) = false;
				if (getMouseX() > 0.5 - optionxsize / 2.0 && getMouseX() < 0.5 + optionxsize / 2.0){
					if (getMouseY() > 0.5 + (optionysize + optionspacing)*i - (optionysize + optionspacing) / 2.0 - yoffset && getMouseY() < 0.5 +
						(optionysize + optionspacing)*i + (optionysize + optionspacing) / 2.0 - yoffset - optionspacing){
						curr_m->getHitbox().at(i) = true;
						if (pressed)menu_select = i;
						Command* cc = dynamic_cast<Command*>(&curr_m->selectOption(i).getObject());
						Menu* mm = dynamic_cast<Menu*>(&curr_m->selectOption(i).getObject());
						if (menu_select == i && cangetpress){
							if (cc){
								cc->exec();
								if(cz)cz->access() = !cz->access();
							}
							if (mm){
								curr_m = mm;
							}
							menu_select = -1;
						}
					}
				}
			}
		}
		if (cangetpress)menu_select = -1;
	}

	void getdir(){
		if (isPressed("A"))dir |= 1 << 0;
		else dir &= ~(1 << 0);
		if (isPressed("D"))dir |= 1 << 1;
		else dir &= ~(1 << 1);
		if (isPressed("W"))dir |= 1 << 2;
		else dir &= ~(1 << 2);
		if (isPressed("S"))dir |= 1 << 3;
		else dir &= ~(1 << 3);
		if (isPressed("SPACE"))dir |= 1 << 4;
		else dir &= ~(1 << 4);
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
	bool mouseOverTarget(double tx, double ty){
		double mx = getMouseX() + deltax;
		double my = getMouseY() + deltay;
		if (mx > tx && mx < tx + 1.0 / xsize && my > ty && my < ty + 1.0 / ysize)return true;
		return false;
	}
	void updatePress(){
		pressed = false;
		if (getLeftClick() && cangetpress){
			cangetpress = false;
			pressed = true;
		}
		if (!getLeftClick()){
			cangetpress = true;
			pressed = false;
		}
	}
	void renderMap(){
		int maptype = getMapType();
		deltax = player->getX() - alterBeingPosX(player->getX());
		deltay = player->getY() - alterBeingPosY(player->getY());
		renderPart(0, 0, 0, 0);
		applyTexture(bg[maptype], - deltax, -deltay, (double)(getMapIndex().size() / (double)xsize), (double)(getMapIndex()[0].size() / (double)ysize));
		renderPart(0, 0, 0, 0);
		if(completed)applyTexture(red, -deltax, -deltay, (double)(getMapIndex().size() / (double)xsize), (double)(getMapIndex()[0].size() / (double)ysize));
		double room_x, room_y;
		char wpos, hpos;
		getRoomSize(room_x, room_y);
		for (int i = 0; i < getMapObjects().size(); ++i){
			double block_x = getMapObjects().at(i).x;
			double block_y = getMapObjects().at(i).y;
			if (block_x == 0)wpos = 0;
			else if (block_x == room_x-1.0/xsize)wpos = 1;
			else wpos = 2;
			if (block_y == 0)hpos = 0;
			else if (block_y == room_y-1.0/ysize)hpos = 1;
			else hpos = 2;
			double x = block_x - deltax;
			double y = block_y - deltay;
			switch (getMapObjects().at(i).type){
			case 1:
				applyTexture(wall[maptype], x - 1.0 / (2 * xsize), y, 1.0 / xsize, 1.0 / ysize);
				applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			case 2:
				applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			case 3:
				applyTexture(wall[maptype], x, y - 1.0 / (2 * ysize), 1.0 / xsize, 1.0 / ysize);
				applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			case 4:
				applyTexture(wall[maptype], x, y - 1.0 / (2 * ysize), 1.0 / xsize, 1.0 / ysize);
				applyTexture(wall[maptype], x - 1.0 / (2 * xsize), y, 1.0 / xsize, 1.0 / ysize);
				applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			case ENTRY:
				if (wpos == 0)setRotationAngle(0);
				if (hpos == 0)setRotationAngle(90);
				if (wpos == 1)setRotationAngle(180);
				if (hpos == 1)setRotationAngle(270);
				applyTexture(entrytex, x, y, 1.0 / xsize, 1.0 / ysize);
				setRotationAngle(0);
				break;
			case EXIT:
				if (wpos == 1)setRotationAngle(0);
				if (hpos == 1)setRotationAngle(90);
				if (wpos == 0)setRotationAngle(180);
				if (hpos == 0)setRotationAngle(270);
				applyTexture(exittex, x, y, 1.0 / xsize, 1.0 / ysize);
				setRotationAngle(0);
				break;
			case VENDING:
				applyTexture(vendtex, x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			case DROP:
				applyTexture(droptex, x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			default:
				//applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			}
		}
	}
public:
	RINS() try : box(xsize, ysize, 4),
		Renderer(640, 480, "RINS"), dir(0), c(0, 0, 0) {
		//Projectile::box = &box;
		Being::box = &box;
		loadMap("do u even seed, bro?");
		c = getMapEntry();

		int pclass;
		cout << "Chose class: " << endl << "0. Marine " << endl << "1. Pyro " << endl << "2. Psychokinetic" << endl << "3. Android" << endl;
		//cin >> pclass;
		pclass = pclass%3;
		if (pclass == 0)targets.push_back(unique_ptr<Being>(new Marine(c.x, c.y)));
		else if (pclass == 1)targets.push_back(unique_ptr<Being>(new Pyro(c.x, c.y)));
		else if (pclass == 2)targets.push_back(unique_ptr<Being>(new Psychokinetic(c.x, c.y)));
		else if (pclass == 3)targets.push_back(unique_ptr<Being>(new Android(c.x, c.y)));
		player = &**targets.begin();


		BeingResources::addTextureID(loadTexture("Textures/devil2.png"), typeid(Marine).name());
		BeingResources::addTextureID(loadTexture("Textures/devil2.png"), typeid(Pyro).name());
		BeingResources::addTextureID(loadTexture("Textures/devil2.png"), typeid(Psychokinetic).name());
		BeingResources::addTextureID(loadTexture("Textures/devil2.png"), typeid(Android).name());
		BeingResources::addTextureID(loadTexture("Textures/gangsta2.png"), typeid(Zombie).name());

		bg[0] = loadTexture("Textures/floor1.jpg");
		bg[1] = loadTexture("Textures/cement.jpg");
		bg[2] = loadTexture("Textures/dirt2.jpg");

		wall[0]  = loadTexture("Textures/brick3.png");
		wall[1]  = loadTexture("Textures/brick4.png");
		wall[2]  = loadTexture("Textures/brick5.png");

		side[0][0] = loadTexture("Textures/school_1.png");
		side[0][1] = loadTexture("Textures/school_2.png");
		side[1][0] = loadTexture("Textures/hospital_1.png");
		side[1][1] = loadTexture("Textures/hospital_2.png");
		side[2][0] = loadTexture("Textures/forest_1.png");
		side[2][1] = loadTexture("Textures/forest_2.png");

		WeaponResources::addTexture(loadTexture("Textures/bullet.png"), BULLET);
		WeaponResources::addTexture(loadTexture("Textures/bullet2.png"), FIRE);

		entrytex = loadTexture("Textures/entry.png");
		exittex = loadTexture("Textures/exit.png");
		vendtex = loadTexture("Textures/vendtex.png");
		droptex = loadTexture("Textures/drop.png");
		red = loadTexture("Textures/red.png");
		setModulateBlending(red);

		main_font = loadFont("Fonts/ARIALUNI.TTF", 20);

		monster_types[ZOMBIE] = &createInstance<Zombie>;
		item_types[BODYARMOR] = &createItem<BodyArmour>;
		item_types[SCOPE] = &createItem<Scope>;
		item_types[PSYCHOAMP] = &createItem<PsychoAmp>;

		setMusicVolume(MAX_VOL/8);
		song1 = loadSong("Sounds/level1.mid");
		//playSong(song1);

		menu_bg = loadTexture("Textures/background1.png");
		button = loadTexture("Textures/button1.png");
		b2 = loadTexture("Textures/button2.png");
		b3 = loadTexture("Textures/button3.png");
		overlay = loadTexture("Textures/overlay1.png");
		setModulateBlending(overlay);

		Menu& m2 = *new Menu();
		m2.addField(*new Button(L"Server name", *new Command([](){ cout << "noname" << endl; })))
			.addField(*new Button(L"Main menu", menu));

		Menu& m3 = *new Menu();
		m3.addField(*new Checkbox(L"Enabled", *new Command([this](){ enable_music = !enable_music; if (enable_music)playSong(song1); else stopMusic(); cout << isPlayingMusic() << endl; }), false))
			.addField(*new Button(L"Main menu", menu));

		menu.addField(*new Button(L"Singleplayer", *new Command([this](){ show_menu = false; })))
			.addField(*new Button(L"Multiplayer", m2))
			.addField(*new Button(L"Sound", m3));

		curr_m = &menu;

		//::xsize = xsize;
		//::ysize = ysize;

		//loop();
	}
	catch (Error e){
		cout << e.getError() << endl;
	}

	void centrifuge(){
		loop();
	}

	~RINS() {
		targets.clear();
		projectiles.clear();
		monsters.clear();
	}
};

int main(int argc, char** argv) {
	try{
		RINS().centrifuge();
	}
	catch (...){
		system("pause");
	}
	return 0;
}