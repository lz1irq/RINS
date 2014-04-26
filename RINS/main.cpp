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

	mutex lock1, monster, projectile, menux, machine, inv, playerm;
	int highscore = 0, spawned = 0, lastroom = 0;
	int main_font;
	bool completed = false;

	bool cangetpress = true, pressed = false;

	Hitbox box;

	Being* curr_target = nullptr;
	array<Being*(*)(double, double), MAXSIZE> monster_types;
	array<Being*(*)(double, double), PLAYEND> player_types;
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
	Machine* curr_machine = nullptr;
	bool over_machine = false;
	bool render_machine = false;

	bool SP_init = false;
	bool MP_server_init = false;
	bool has_MP_server = false;
	int SP_class;
	bool MP_noplayers = false;
	time_t seed;
	int MP_numplayers = 0;
	bool MP_init = false;
	bool MP_mode = false;

	int iframe, iframesel;
	bool render_inv = false;
	int itemsel = -1, itemseli = -1;
	int itemover = -1;

	struct player_info{
		int keyboard;
		int class_;
		short last_command;
	};

	int song1;
	map<pair<int, int>, Machine> machines;

	
	 bool mouseOverTile(int tx, int ty){
		box.setX(getMouseX() + deltax);
		box.setY(getMouseY() + deltay);
		if(box.getTileX() == tx && box.getTileY() == ty)return true;
		return false;
	}

	 void controlVendingMachine() {
		 double ystart = 0.01;
		 double xstart = 0.01;
		 double itemx = 0.145;
		 double itemy = 0.145;
		 double framesp = 0.01;
		 int rows = -1;
		 int itemc = curr_machine->itemCount();
		 machine.lock();
		 for(int i=0;i<itemc;++i) {
			 if(i%3 == 0) ++rows;
			 Item& it = curr_machine->getNextItem();
			 double xp = 0.024+(i%3)*itemx+framesp;
			 double yp = rows*itemy + 5*framesp;

			 double nextx = 0.024+((i+1)%3)*itemx+framesp;
			 if((i+1)%3 == 0 && i>0) nextx+=3*itemx;
			 double nexty = (rows+1)*itemy+5*framesp;
			 double mx = getMouseX();
			 double my = getMouseY();

			 if(itemsel == -1) {
				 if(mx>xp && mx<nextx && my>yp && my<nexty && pressed) {
					 itemsel = i;
				 }
			 }
			 if(itemsel == i) {
				 if(cangetpress){
					 if(mx>xp && mx<nextx && my>yp && my<nexty) {
						 if(player->buyItem(it)) cout << "Player bought " << it.getName() << endl;
						 else cout << "Not enough money to buy " << it.getName() << endl;
						 itemsel = -1;
					 }
					 else itemsel = -1;
				 }
			 }

		 }
		 machine.unlock();
	 }

	 void controlInventory() {
		 double ystart = 0.01;
		 double xstart = 0.51;
		 double itemx = 0.145;
		 double itemy = 0.145;
		 double framesp = 0.01;
		 int rows = -1;

		 int itemc = player->itemCount();
		 for(int i=0;i<itemc;++i) {
			 if(i%3 == 0) ++rows;
			 Item& it = player->getNextItem();
			 double xp = xstart+0.027+(i%3)*itemx+framesp;
			 double yp = rows*itemy + 5*framesp;

			 double nextx = xstart+0.01+((i+1)%3)*itemx+framesp;
			 if((i+1)%3 == 0 && i>0) nextx+=3*itemx;
			 double nexty = (rows+1)*itemy+5*framesp;
			 double mx = getMouseX();
			 double my = getMouseY();





			 if(itemseli == -1) {
				 if(mx>xp && mx<nextx && my>yp && my<nexty && pressed) {
					 itemseli = i;
				 }
			 }
			 if(itemseli == i) {
				 if(cangetpress){
					 if(mx>xp && mx<nextx && my>yp && my<nexty) {
						 itemseli = -1;
						 inv.lock();
						 if(render_machine) player->sellItem(i);
						 else { //(un)equip the item
							 if(it.isEquipped()) {
								 player->unequipItem(it);
								 cout << "Player unequipped " << it.getName() << endl;
							 }
							 else {
								 player->equipItem(it);
								 cout << "Player equipped " << it.getName() << endl;
							 }
						 }
						 inv.unlock();
					 }
					 else itemseli = -1;
				 }
			 }

		 }
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
		double itemx = 0.145;
		double itemy = 0.145;
		double framesp = 0.01;
		int rows = -1;
		applyTexture(menu_bg, xstart, ystart, 0.48,0.85);
		int itemc = curr_machine->itemCount();
		machine.lock();
		for(int i=0;i<itemc;++i) {
			if(i%3 == 0) ++rows;

			Item& it = curr_machine->getNextItem();
			int tid = ItemResources::getTextureID(&typeid(it));

			double xp = 0.027+(i%3)*itemx+framesp;
			double yp = rows*itemy + 5*framesp;

			double nextx = xstart+0.027+((i+1)%3)*itemx+framesp;
			if((i+1)%3 == 0 && i>0) nextx+=3*itemx;
			double nexty = (rows+1)*itemy+5*framesp;
			double mx = getMouseX();
			double my = getMouseY();

			applyTexture(tid, xp, yp, itemx, itemy);
			if(itemsel == i) applyTexture(iframesel, xp, yp, itemx, itemy);
			else applyTexture(iframe, xp, yp, itemx, itemy);
		}
		machine.unlock();
	}

	void renderInventory() {
		double ystart = 0.01;
		double xstart = 0.51;
		double itemx = 0.145;
		double itemy = 0.145;
		double framesp = 0.01;
		int rows = -1;
		applyTexture(menu_bg, xstart, ystart, 0.48,0.85);

		double w,h;
		RGBA mecol(255, 255, 0, 0);
		getTextWH(main_font, (Uint16*)L"$", w, h);
		w *= 0.06/h;
		h = 0.06;
		displayText(main_font, (Uint16*)L"$",mecol, 0.75, 0.76,w,h);

		getTextWH(main_font, (Uint16*)to_wstring(player->getMoney()).c_str(), w, h);
		w *= 0.06/h;
		h = 0.06;
		displayText(main_font, (Uint16*)to_wstring(player->getMoney()).c_str(),mecol, 0.77, 0.76,w,h);


		int itemc = player->itemCount();
		for(int i=0;i<itemc;++i) {

			if(i%3 == 0) ++rows;
			inv.lock();
			Item& it = player->getNextItem();
			int tid = ItemResources::getTextureID(&typeid(it));
			inv.unlock();

			double xp = xstart+0.027+(i%3)*itemx+framesp;
			double yp = rows*itemy + 5*framesp;

			double nextx = xstart+0.01+((i+1)%3)*itemx+framesp;
			if((i+1)%3 == 0 && i>0) nextx+=3*itemx;
			double nexty = (rows+1)*itemy+5*framesp;
			double mx = getMouseX();
			double my = getMouseY();

			applyTexture(tid, xp, yp, itemx, itemy);
			if(itemseli == i) applyTexture(iframesel, xp, yp, itemx, itemy);
			else applyTexture(iframe, xp, yp, itemx, itemy);

			if(mx>xp && mx<nextx && my>yp && my<nexty) itemover = i;
			else itemover = -1;

			double overx = 0.145, overy = 0.145;

			cout << i << " " << itemover << endl;
			// Hover window with item data.
			//if(itemover == i) {
			//Derived& der = it.getDerivedBonuses();
			//if(1.0-nextx > overx)applyTexture(menu_bg, nextx, nexty, overx, overy);
			//else applyTexture(menu_bg, xp, nexty, overx, overy);

			//}
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

	void displayPlayer(){
		int offset = 0;
		bool pla = true;
		for (auto &i : targets){
			if ((*i).getWalk())offset = 2;
			renderPart(4, 2, offset + ((int)log2((*i).getOrientation()) >> 1), 1 - ((int)log2((*i).getOrientation()) % 2));
			if (pla)applyTexture(BeingResources::getTextureID(&typeid((*i))), alterBeingPosX((*i).getX()), alterBeingPosY((*i).getY()), 1.0 / xsize, 1.0 / ysize);
			else applyTexture(BeingResources::getTextureID(&typeid(*i)), i->getX() - deltax, i->getY() - deltay, 1.0 / xsize, 1.0 / ysize);
			pla = false;
		}
	}
	void displayMonsters(){
		for (auto &i : monsters) {
			if (i->getWalk())renderPart(4, 2, 2 + ((int)log2(i->getOrientation()) >> 1), 1 - ((int)log2(i->getOrientation()) % 2));
			else renderPart(4, 2, ((int)log2(i->getOrientation()) >> 1), 1 - ((int)log2(i->getOrientation()) % 2));
			applyTexture(BeingResources::getTextureID(&typeid(*i)), i->getX() - deltax, i->getY() - deltay, 1.0 / xsize, 1.0 / ysize);
		}
	}
	void displayProjectiles(){
		renderPart(0, 0, 0, 0);
		for (auto &i : projectiles){
			setRotationAngle(i.getAngleInDeg());
			applyTexture(WeaponResources::getTexture(i.getType()), i.getX() - deltax + 1.5*box.getStepX(), i.getY() - deltay + 1.5*box.getStepY(), box.getStepX()*2, box.getStepY()*2);
			setRotationAngle(0);
		}
	}
	void displayHUD(){
		applyTexture(side[getMapType()][0], -1, 0, 1, 1);
		applyTexture(side[getMapType()][1], 1, 0, 1, 1);
		renderHUD();
	}

	void graphicsLoop() final {
		try{
			if (!show_menu && !SP_init && !MP_server_init && !MP_noplayers && !MP_init){
				lock1.lock();
				renderMap();
				lock1.unlock();
				playerm.lock();
				displayPlayer();
				playerm.unlock();
				monster.lock();
				displayMonsters();
				monster.unlock();
				projectile.lock();
				displayProjectiles();
				projectile.unlock();
				if(render_machine) {
					renderVendingMachine();
					renderInventory();
				}
				else if(render_inv) renderInventory();
				lock1.lock();
				displayHUD();
				lock1.unlock();
			}
			else if (show_menu){
				renderPart(0, 0, 0, 0);
				menux.lock();
				renderMenu();
				menux.unlock();
			}
			else if (MP_noplayers)displayText(main_font, (Uint16*)(L"Waiting for players to begin..."+to_wstring(MP_numplayers)).c_str(), RGBA(100, 255, 150, 0), 0.0, 0.0, 0, 0.1);


			renderScene();
		}
		catch (Error e){
			cout << e.getError() << endl;
		}
	}

	void moveAndColide(){
		lastxpos = player->getX();
		lastypos = player->getY();
		bool b = player->move(dir, false);
		if (b){
			if (lastxpos == player->getX() && lastypos == player->getY())player->resetWalk();
			else{
				render_inv = false;
				render_machine = false;
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
	}

	void playerShoot(){
		if (dir & 16){
			Projectile* p;
			int event = player->tryToShoot(curr_target, &p);
			switch (event){
			case BANG:
				projectiles.push_back(*p);
				break;
			case CASTING:
				int* percent;
				percent = (int*)p;
				delete percent;
				break;
			}
		}
		else player->resetFire();
	}

	void tryToSpawn(){
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
	}

	void updateProjectiles(){
		for (auto p = begin(projectiles); p != end(projectiles); ++p){
			bool res = p->update(getMapIndex(), monsters, targets);
			if (!res){
				projectile.lock();
				p = projectiles.erase(p);
				projectile.unlock();
			}
		}
	}

	void updateMonsters(){
		monster.lock();
		for (auto m = begin(monsters); m != end(monsters); ++m){
			bool res = (*m)->action(getMapIndex(), projectiles, targets, getTicks());
			if (!res){
				addLoot((*m)->getTileX(), (*m)->getTileY());
				Being* ptr = &**m;
				cout << ptr << endl;
				m = monsters.erase(m);
				lock1.lock();
				++highscore;
				player->addExperience(50);
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

	void mainLoop() final {
		try {
			if (SP_init){
				if (MP_init){
					server_info si;
					char* c = (char*)&SP_class;
					sendCommand(GETINFO, 4, c);
					c = receiveCommand();
					short cmd;
					short data;
					memcpy(&cmd, &c[0], 2);
					memcpy(&data, &c[2], 2);
					switch (cmd){
					case SERVERINFO:
						if (data != sizeof(si))throw Error("Nope!");
						memcpy(&si, &c[4], sizeof(si));
						seed = si.seed;
						MP_numplayers = si.n_players;
						MP_noplayers = si.gathering;
						//must insert si.game_end!
						break;
					default: throw Error("Nope!");
					}
					if (!MP_noplayers)MP_init = false;
				}
				if (has_MP_server)MP_noplayers = true;
				loadMap(seed);
				c = getMapEntry();
				targets.push_back(unique_ptr<Being>(player_types[SP_class](c.x, c.y))); player = &**targets.begin();
				show_menu = false;
				SP_init = false;
			}
			if (MP_init && !show_menu){
				server_info si;
				char* c = (char*)&SP_class;
				sendCommand(GETINFO, 4, c);
				c = receiveCommand();
				short cmd;
				short data;
				memcpy(&cmd, &c[0], 2);
				memcpy(&data, &c[2], 2);
				switch (cmd){
				case SERVERINFO:
					if (data != sizeof(si))throw Error("Nope!");
					memcpy(&si, &c[4], sizeof(si));
					seed = si.seed;
					MP_numplayers = si.n_players;
					MP_noplayers = si.gathering;
					//must insert si.game_end!
					break;
				default: throw Error("Nope!");
				}
				if (!MP_noplayers){
					MP_init = false;
					MP_mode = true;
				}
			}
			if (MP_server_init){
				startServer(1337);
				MP_server_init = false;
				cout << "Server init -> success!" << endl;
			}
			if (MP_noplayers && has_MP_server){
				MP_numplayers = gatherPlayers();
				bool good = true;
				if (!updateClients())good = false;
				list<Socket::Client>& lsc = getClients();
				list<player_info> lpi;
				for (auto i = begin(lsc); i != end(lsc); ++i){
					player_info pi;
					pi.last_command = -1;
					if (!processCommand(getNextCommand(*i), i, pi)){
						good = false;
						break;
					}
					if (pi.last_command != GETINFO){
						good = false;
						break;
					}
					lpi.push_back(pi);
				}
				if (MP_numplayers == 1 && good == true){
					for (auto& i : lpi){
						targets.push_back(unique_ptr<Being>(player_types[i.class_](c.x, c.y)));
					}
					MP_noplayers = false;
				}
			}
			if (!show_menu && !SP_init && !MP_server_init && !MP_noplayers && !MP_init){
				if (updateInternalMapState()) dir = 0;
				if (!MP_mode){
					player->action(getMapIndex(), projectiles, targets, getTicks());
					moveAndColide();
					playerShoot();
					tryToSpawn();
					updateMonsters();
					updateProjectiles();
				}
				else{
					char* c = (char*)&dir;
					sendCommand(KEYBOARD, 4, c);
					cout << "sending" << endl;
					projectile.lock();
					projectiles.clear();//mutex here?!?
					projectile.unlock();
					short cmd;
					short data;
					Projectile* p;
					for (bool loop = true; loop;){
						c = receiveCommand();
						memcpy(&cmd, &c[0], 2);
						memcpy(&data, &c[2], 2);
						switch (cmd){
						case SELF:
							if (data != sizeof(Being))throw Error("Nope!");
							playerm.lock();
							loop = false;
							//memcpy(player, &c[4], sizeof(Being));
							playerm.unlock();
							break;
						case ENDBIT:
							loop = false;
							break;
						case BULLET:
							if (data != sizeof(Projectile))throw Error("Nope!");
							p = (Projectile*)&c[4];
							//projectiles.push_back(*p);//mutex here?!?
							break;
						default: throw Error("Nope!");
						}
					}
				}
				if (curr_machine)checkVendingMachines(player->getTileX(), player->getTileY());
				if (isPressed("P")) render_inv = !render_inv;
				if (render_machine) {
					controlInventory();
					controlVendingMachine();
				}
				else if (render_inv) controlInventory();
				if (has_MP_server){
					if (!updateClients()){
						cout << "end game!" << endl;
						exit(0);
					}
					list<Socket::Client>& lsc = getClients();
					list<unique_ptr<Being>>::iterator it2 = targets.begin();
					if (it2 != targets.end())++it2;
					for (auto i = begin(lsc); i != end(lsc); ++i, ++it2){
						player_info pi;
						if (!processCommand(getNextCommand(*i), i, pi)){
							cout << "end game!" << endl;
							exit(0);
						}
						if (pi.last_command == KEYBOARD){
							cout << pi.keyboard << endl;
							dir = pi.keyboard;
							player = &**it2;
							player->action(getMapIndex(), projectiles, targets, getTicks());
							moveAndColide();
							playerShoot();
							commandToClient(i, SELF, sizeof(Being), (char*)player);
							for (auto& j : projectiles){
								commandToClient(i, BULLET, sizeof(Projectile), (char*)&j);
							}
							commandToClient(i, ENDBIT, 0, NULL);
							player = &**targets.begin();
						}
					}
				}
				getdir();
			}
			else if(show_menu){
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

	struct server_info{
		int n_players;
		bool gathering;
		bool game_end;
		time_t seed;
	};

	bool processCommand(char* c, list<Client>::iterator& cl, player_info& pi){
		if (c == nullptr){
			pi.last_command = -1;
			return true;
		}
		short cmd;
		short data;
		memcpy(&cmd, &c[0], 2);
		memcpy(&data, &c[2], 2);
		server_info i;
		char* cd;
		pi.last_command = cmd;
		switch (cmd){
		case KEYBOARD:
			if (data != 4)throw Error("Nope!");
			memcpy(&pi.keyboard, &c[4], data);
			break;
		case GETINFO:
			if (data != 4)throw Error("Nope!");
			cout << (int)c[4] << endl;
			memcpy(&pi.class_, &c[4], data);
			i.n_players = MP_numplayers;
			i.gathering = MP_noplayers;
			i.game_end = false;
			i.seed = seed;
			cd = (char*)&i;
			return commandToClient(cl, SERVERINFO, sizeof(i), cd);
			break;
		default:
			throw Error("Nope!");
		}
		return true;
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
		Renderer(640, 640, "RINS"), dir(0), c(0, 0, 0) {
		//Projectile::box = &box;
		Being::bx = &box;
		seed = system_clock::to_time_t(system_clock::now());

		BeingResources::addTextureID(loadTexture("Textures/hitler.png"), &typeid(Marine));
		BeingResources::addTextureID(loadTexture("Textures/devil2.png"), &typeid(Pyro));
		BeingResources::addTextureID(loadTexture("Textures/devil2.png"), &typeid(Psychokinetic));
		BeingResources::addTextureID(loadTexture("Textures/devil2.png"), &typeid(Android));
		BeingResources::addTextureID(loadTexture("Textures/gangsta2.png"), &typeid(Zombie));

		ItemResources::addTextureID(loadTexture("Textures/scope.png"), &typeid(Scope));
		ItemResources::addTextureID(loadTexture("Textures/armour.png"), &typeid(BodyArmour));
		ItemResources::addTextureID(loadTexture("Textures/amp.png"), &typeid(PsychoAmp));

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

		iframe = loadTexture("Textures/itemframe.png");
		iframesel = loadTexture("Textures/itemframesel.png");

		//enum {BULLET, FIRE, PSYCHO, ENERGY};
		WeaponResources::addTexture(loadTexture("Textures/bullet.png"), BULLET);
		WeaponResources::addTexture(loadTexture("Textures/bullet4.png"), FIRE);
		WeaponResources::addTexture(loadTexture("Textures/bullet3.png"), PSYCHO);
		WeaponResources::addTexture(loadTexture("Textures/bullet2.png"), ENERGY);

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
		player_types[MARINE] = &createInstance<Marine>;
		player_types[PYRO] = &createInstance<Pyro>;
		player_types[PSYCHOKINETIC] = &createInstance<Psychokinetic>;
		player_types[ANDROID] = &createInstance<Android>;


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
		m2.addField(*new Button("Start server", *new Command([this](MenuControl& mc){ if (has_MP_server)return; MP_server_init = true; has_MP_server = true;  })))
			.addField(*new TextBox("Connect to: ", *new Command([this](MenuControl& mc){ if (mc.checked()){ ConnectToServer(1337, mc.getText().substr(mc.getID(), string::npos).c_str()); MP_init = true; }})))
			.addField(*new Button("Main menu", menu));

		Menu& m3 = *new Menu();
		m3.addField(*new Checkbox("Enabled", *new Command([this](MenuControl& mc){ enable_music = mc.checked(); if (enable_music)playSong(song1); else stopMusic();}), false))
			.addField(*new Button("Main menu", menu));

		Menu& m4 = *new Menu();
		m4.addField(*new Button("Marine", *new Command([this](MenuControl& mc){ SP_class = 0; SP_init = true; })))
			.addField(*new Button("Pyro", *new Command([this](MenuControl& mc){ SP_class = 1; SP_init = true; })))
			.addField(*new Button("Psychokinetic", *new Command([this](MenuControl& mc){ SP_class = 2; SP_init = true; })))
			.addField(*new Button("Android", *new Command([this](MenuControl& mc){ SP_class = 3; SP_init = true; })))
			.addField(*new Button("Main menu", menu));

		menu.addField(*new Button("Singleplayer", m4))
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