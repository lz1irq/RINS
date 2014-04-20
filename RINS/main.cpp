#include "Platform.h"
#include "Being.h"
#include "Map.h"
#include "Menu.h"
#include "Machine.h"
#include <math.h>
#include <bitset>
#include <mutex>


using namespace std;
class RINS : public Game, public Renderer, public Audio, public Map, public Socket{

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

	mutex lock1, monster, projectile, menux, machine;

	int highscore = 0, spawned = 0, lastroom = 0;
	int main_font;
	bool completed = false;

	bool cangetpress = true, pressed = false;

	Hitbox box;

	Being* curr_target = nullptr;
	array<Being*(*)(double, double), MAXSIZE> monster_types;
	array<Item*(*)(), MAXITEMS> item_types;
	list<Projectile> projectiles;

	int menu_bg, button, overlay, b2, b3, tbox;
	Menu menu;
	Menu* curr_m;
	int  menu_select = -1, lastm = -1;
	double optionysize = 0.05;
	double optionspacing = 0.01;
	double optionxsize = 0.5;
	double hsize = 0.8*optionysize;
	double yoffset;
	bool typing = false, muststop = false;
	bool show_menu = true;
	bool enable_music = false;
	bool server = false;
	bool started = false;
	Machine* curr_machine = nullptr;
	bool over_machine = false;
	bool render_machine = false;

	int song1;
	map<pair<int, int>, Machine> machines;

	
	 bool mouseOverTile(int tx, int ty){
		box.setX(getMouseX() + deltax);
		box.setY(getMouseY() + deltay);
		if(box.getTileX() == tx && box.getTileY() == ty)return true;
		return false;
	}

	void renderHUD() {
		RGBA mecol(255, 255, 0, 0);
		double w,h;

		getTextWH(main_font, (Uint16*)L"HEALTH:", w, h);
		w *= 0.06/h;
		h = 0.06;
		displayText(main_font, (Uint16*)L"HEALTH:",mecol, 0.0, 0.9,w,h);

		getTextWH(main_font, (Uint16*)to_wstring(player->getHealth()).c_str(), w, h);
		w *= 0.06/h;
		h = 0.06;
		displayText(main_font, (Uint16*)to_wstring(player->getHealth()).c_str(),mecol, 0.005, 0.94,w,h);
		
		getTextWH(main_font, (Uint16*)L"SCORE:", w, h);
		w *= 0.06/h;
		h = 0.06;
		displayText(main_font, (Uint16*)L"SCORE:",mecol, 1 - w, 0.9,w,h);
				
		getTextWH(main_font, (Uint16*)to_wstring(highscore).c_str(), w, h);
		w *= 0.06/h;
		h = 0.06;
		displayText(main_font, (Uint16*)to_wstring(highscore).c_str(),mecol, 1-w, 0.94,w,h);
	}

	void renderVendingMachine() {
		double ystart = 0.01;
		double xstart = 0.01;
		double itemx = 0.16;
		double itemy = 0.16;
		int rows = 0;
		applyTexture(menu_bg, xstart, ystart, 0.48,0.85);
		applyTexture(menu_bg, xstart+0.5,ystart, 0.48,0.85);
		int itemc = curr_machine->itemCount();
		cout << itemc << endl;
		for(int i=0;i<itemc;++i) {
			Item& it = curr_machine->getNextItem();
			int tid = ItemResources::getTextureID(&typeid(it));
			double xp = 0.003+(i%3)*itemx;
			double yp = 0.04+rows*itemy;
			if(i%3 == 0) ++rows;
			applyTexture(tid, xp, yp, itemx, itemy);
			
		}
	}
		
	void checkVendingMachines(int x, int y) {
		if(!over_machine) {
			if(mouseOverTile(x,y) && pressed) over_machine = true;
		}
		else {
			if (cangetpress){
				over_machine = false;
				if (mouseOverTile(x, y)) {
					render_machine = true;
				}
			}
		}
	}


	void graphicsLoop() final {
		try{

			lock1.lock();
			renderMap();
			lock1.unlock();
			int offset = 0;
			if (player->getWalk())offset = 2;
			renderPart(4, 2, offset + ((int)log2(player->getOrientation()) >> 1), 1 - ((int)log2(player->getOrientation()) % 2));

			applyTexture(BeingResources::getTextureID(&typeid(*player)), alterBeingPosX(player->getX()), alterBeingPosY(player->getY()), 1.0 / xsize, 1.0 / ysize);

			monster.lock();//lock the monster!
			for (auto &i : monsters) {
				if (i->getWalk())renderPart(4, 2, 2 + ((int)log2(i->getOrientation()) >> 1), 1 - ((int)log2(i->getOrientation()) % 2));
				else renderPart(4, 2, ((int)log2(i->getOrientation()) >> 1), 1 - ((int)log2(i->getOrientation()) % 2));
				applyTexture(BeingResources::getTextureID(&typeid(*i)), i->getX() - deltax, i->getY() - deltay, 1.0 / xsize, 1.0 / ysize);
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

			if(render_machine) renderVendingMachine();

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
			if (!show_menu){
				tmpdir2 = dir;
				getdir();
				lastxpos = player->getX();
				lastypos = player->getY();

				if (updateInternalMapState()) dir = 0;

				if (getTicks() - last_tick > 33){
					player->move(dir, false);
					last_tick = getTicks();
					if (lastxpos == player->getX() && lastypos == player->getY())player->resetWalk();
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
									;
									curr_machine = &machines.at(make_pair(player->getTileX(), player->getTileY()));
									;
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

				bool mustspawn = pattern() % getSpawnRate() == false;
				int spawntype = pattern() % monster_types.size();
				int x = pattern() % getMapIndex().size();
				int y = pattern() % getMapIndex()[x].size();
				if (mustspawn && !getMapIndex()[x][y]){
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

				if (curr_machine)checkVendingMachines(player->getTileX(), player->getTileY());

				int tar = 0;
				monster.lock();
				for (auto m = begin(monsters); m != end(monsters); ++m, ++tar){
					bool res = (*m)->action(getMapIndex(), projectiles, targets, getTicks());
					if (!res){
						addLoot((*m)->getTileX(), (*m)->getTileY());
						Being* ptr = &**m;
						cout << ptr << endl;
						m = monsters.erase(m);
						lock1.lock();
						++highscore;
						lock1.unlock();
						curr_target = nullptr;
					}
					else{
						if (mouseOverTarget((*m)->getX(), (*m)->getY()) && pressed){
							curr_target = &**m;
						}
					}
				}
				monster.unlock();

			}

			if (show_menu){
				menux.lock();
				checkMenu();
				menux.unlock();
			}

			if (server){
				if (!started){
					startServer(4, 1337);
					started = true;
				}
				cout << gatherPlayers() << endl;
			}

			updatePress();
			SDL_Delay(10);
		}
		catch (Error e) {
			cout << e.getError() << endl; 
		}
	}

	wstring utf8_to_utf16(const string& utf8){
		Uint16* unicode = new Uint16[utf8.size()]();
		int count = 0;
		size_t i = 0;
		while (i < utf8.size()){
			Uint16 uni;
			size_t todo;
			bool error = false;
			unsigned char ch = utf8[i++];
			if (ch <= 0x7F){
				uni = ch;
				todo = 0;
			}
			else if (ch <= 0xBF)throw Error("not a UTF-8 string");
			else if (ch <= 0xDF){
				uni = ch & 0x1F;
				todo = 1;
			}
			else if (ch <= 0xEF){
				uni = ch & 0x0F;
				todo = 2;
			}
			else if (ch <= 0xF7){
				uni = ch & 0x07;
				todo = 3;
			}
			else throw Error("not a UTF-8 string");
			for (size_t j = 0; j < todo; ++j){
				if (i == utf8.size())throw Error("not a UTF-8 string");
				unsigned char ch = utf8[i++];
				if (ch < 0x80 || ch > 0xBF)throw Error("not a UTF-8 string");
				uni <<= 6;
				uni += ch & 0x3F;
			}
			if (uni >= 0xD800 && uni <= 0xDFFF) throw Error("not a UTF-8 string");
			if (uni > 0x10FFFF) throw Error("not a UTF-8 string");
			unicode[count] = uni;
			++count;
		}
		wstring utf16;
		for (size_t i = 0; i < count; ++i){
			Uint16 uni = unicode[i];
			if (uni <= 0xFFFF) utf16 += (wchar_t)uni;
			else{
				uni -= 0x10000;
				utf16 += (wchar_t)((uni >> 10) + 0xD800);
				utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
			}
		}
		return utf16;
	}


	void renderMenu(){
		applyTexture(menu_bg, 0, 0, 1, 1);
		yoffset = (curr_m->getNumOptions() - 1)*(optionysize + optionspacing) / 2.0;
		double w, h;
		RGBA mecol(255, 100, 255, 0);
		for (int i = 0; i < curr_m->getNumOptions(); ++i){
			Button* bb = dynamic_cast<Button*>(&curr_m->selectOption(i));
			Checkbox* cc = dynamic_cast<Checkbox*>(&curr_m->selectOption(i));
			TextBox* tt = dynamic_cast<TextBox*>(&curr_m->selectOption(i));
			string text = curr_m->selectOption(i).getText();
			if (curr_m->getHitbox().at(i))mecol.setR(255);
			else mecol.setR(0);
			if (bb || cc || tt){
				int tex;
				if (tt){
					tex = tbox;
					if(!tt->checked())text = tt->getText().substr(0, tt->getID()) + getText();
					else text = tt->getText();
				}
				if (bb)tex = button;
				if (cc){
					if (cc->checked()){
						tex = b2;
						text += ": ДА";
					}
					else{
						tex = b3;
						text += ": НЕ";
					}
				}
				applyTexture(tex, 0.5 - optionxsize / 2.0, 0.5 + (optionysize + optionspacing)*i - (optionysize + optionspacing) / 2.0 - yoffset, optionxsize, optionysize);
				getTextWH(main_font, (Uint16*)utf8_to_utf16(text).c_str(), w, h);
				if (curr_m->getHitbox().at(i)){
					applyTexture(overlay, 0.5 - optionxsize / 2.0, 0.5 + (optionysize + optionspacing)*i - (optionysize + optionspacing) / 2.0 - yoffset, optionxsize, optionysize);
				}
				w *= hsize / h;
				h = hsize;
				if (tt){
					if (w > optionxsize - 0.06 && typing)muststop = true;
				}
				displayText(main_font, (Uint16*)utf8_to_utf16(text).c_str(),
					mecol, 0.5 -w/2 , 0.5 + (optionysize + optionspacing)*i - (optionysize + optionspacing) / 2.0 - yoffset, w, h);
			}
		}
	}

	void checkMenu(){
		double w, h;
		bool hover = false;
		static string s;
		for (int i = 0; i < curr_m->getNumOptions(); ++i){
			Button* bb = dynamic_cast<Button*>(&curr_m->selectOption(i));
			Checkbox* cz = dynamic_cast<Checkbox*>(&curr_m->selectOption(i));
			TextBox* tt = dynamic_cast<TextBox*>(&curr_m->selectOption(i));
			if (bb || cz || tt){
				if (lastm == i && tt){
					bool enter;
					if (muststop){
						endTyping(false);
						muststop = false;
						startTyping(s.c_str());
					}
					s = getRawText(enter);
					if (enter){
						tt->getText() = tt->getText().substr(0, tt->getID()) + s;
						endTyping(true);
						typing = false;
						tt->checked() = true;
						Command* cc = dynamic_cast<Command*>(&tt->getObject());
						if (cc)cc->exec(curr_m->selectOption(i));
					}
				}
				curr_m->getHitbox().at(i) = false;
				if (getMouseX() > 0.5 - optionxsize / 2.0 && getMouseX() < 0.5 + optionxsize / 2.0){
					if (getMouseY() > 0.5 + (optionysize + optionspacing)*i - (optionysize + optionspacing) / 2.0 - yoffset && getMouseY() < 0.5 +
						(optionysize + optionspacing)*i + (optionysize + optionspacing) / 2.0 - yoffset - optionspacing){
						curr_m->getHitbox().at(i) = true;
						if (pressed){
							menu_select = i;
							lastm = i;
						}
						if (pressed && !tt){
							endTyping(true);
							typing = false;
						}
						Command* cc = dynamic_cast<Command*>(&curr_m->selectOption(i).getObject());
						Menu* mm = dynamic_cast<Menu*>(&curr_m->selectOption(i).getObject());
						if (menu_select == i && cangetpress){
							if (cc){
								if (tt){
									startTyping(tt->getText().substr(tt->getID(), string::npos).c_str());
									typing = true;
									tt->checked() = false;
								}
								cc->exec(curr_m->selectOption(i));
							}
							if (mm){
								curr_m = mm;
								lastm = -1;
							}
							menu_select = -1;
						}
						hover = true;
					}
				}
			}
		}
		if (cangetpress)menu_select = -1;
		if (pressed && !hover){
			endTyping(true);
			typing = false;
			lastm = -1;
		}
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

		BeingResources::addTextureID(loadTexture("Textures/devil2.png"), &typeid(Marine));
		BeingResources::addTextureID(loadTexture("Textures/devil2.png"), &typeid(Pyro));
		BeingResources::addTextureID(loadTexture("Textures/devil2.png"), &typeid(Psychokinetic));
		BeingResources::addTextureID(loadTexture("Textures/devil2.png"), &typeid(Android));
		BeingResources::addTextureID(loadTexture("Textures/gangsta2.png"), &typeid(Zombie));

		ItemResources::addTextureID(loadTexture("Textures/scope.png"), &typeid(Scope));
		ItemResources::addTextureID(loadTexture("Textures/armour.jpeg"), &typeid(BodyArmour));
		ItemResources::addTextureID(loadTexture("Textures/amp.jpg"), &typeid(PsychoAmp));

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

		main_font = loadFont("Fonts/ARIALUNI.TTF", 90);

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
		tbox = loadTexture("Textures/button4.png");
		overlay = loadTexture("Textures/overlay1.png");
		setModulateBlending(overlay);

		Menu& m2 = *new Menu();
		m2.addField(*new Button("Start server", *new Command([this](MenuControl& mc){ server = true; })))
			.addField(*new TextBox("Connect to: ", *new Command([this](MenuControl& mc){ if (mc.checked())ConnectToServer(1337, mc.getText().substr(mc.getID(), string::npos).c_str());})))
			.addField(*new Button("Main menu", menu));

		Menu& m3 = *new Menu();
		m3.addField(*new Checkbox("Enabled", *new Command([this](MenuControl& mc){ enable_music = mc.checked(); if (enable_music)playSong(song1); else stopMusic();}), false))
			.addField(*new Button("Main menu", menu));

		menu.addField(*new Button("Singleplayer", *new Command([this](MenuControl& mc){ show_menu = false; })))
			.addField(*new Button("Multiplayer", m2))
			.addField(*new Button("Sound", m3));

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