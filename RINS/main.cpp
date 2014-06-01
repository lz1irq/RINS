#include "Menu.h"
#include "Being.h"
#include "Map.h"
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
	list<Projectile> projectiles;
	mt19937 pattern;

	int last_tick = 0, timer2 = 0, projectile_tick = 0;

	mutex lock1, monster, projectile, menux, machinem, inv, playerm;
	int highscore = 0, spawned = 0, lastroom = 0;
	int main_font;
	bool completed = false;

	bool cangetpress = true, pressed = false;

	Hitbox box;

	Being* curr_target = nullptr;
	array<Being*(*)(double, double), MAXSIZE> monster_types;
	array<Being*(*)(double, double), PLAYEND> player_types;
	array<Item*(*)(), MAXITEMS> item_types;

	Menu* curr_m;
	bool typing = false, muststop = false;
	bool show_menu = true;
	bool enable_music = false;

	bool SP_init = false;
	bool MP_server_init = false;
	int MP_numplayers = 0;
	int SP_class;
	time_t seed;
	int MP_numconn = 1;
	bool MP_init = false;
	int conn_confirmed = 0;

	struct login{
		int mp_class;
	};
	struct info{
		bool is_room_active;
		int last_room;
		bool game_end;
		unsigned int seed;
		int nplayers;
	};

	bool end_of_game = false;
	enum Commands{ MOVE, SHOOT, GETITEM, SELF, MONSTERS, PLAYERS, LOGIN, INFO, BULLETS };

	bool render_inv = false, pre_inv = false;
	double cast_prog = -1;
	int itemsel = -1, itemseli = -1;
	int itemover = -1;
	int hud_bg;

	bool pstats = false, pre_pstats = false;

	struct player_info{
		int keyboard;
		int class_;
		short last_command;
	};

	int song1;
	MachineManager machines;

	int hpgreen, hpred;

	bool mouseOverTile(int tx, int ty){
		box.setX(getMouseX() + deltax);
		box.setY(getMouseY() + deltay);
		if (box.getTileX() == tx && box.getTileY() == ty)return true;
		return false;
	}

	void controlInventory() {
		double ystart = 0.01;
		double xstart = 0.51;
		double itemx = 0.145;
		double itemy = 0.145;
		double framesp = 0.01;
		int rows = -1;

		int itemc = player->itemCount();
		for (int i = 0; i<itemc; ++i) {
			if (i % 3 == 0) ++rows;
			Item& it = player->getNextItem();
			double xp = xstart + 0.027 + (i % 3)*itemx + framesp;
			double yp = rows*itemy + 5 * framesp;

			double nextx = xstart + 0.01 + ((i + 1) % 3)*itemx + framesp;
			if ((i + 1) % 3 == 0 && i>0) nextx += 3 * itemx;
			double nexty = (rows + 1)*itemy + 5 * framesp;
			double mx = getMouseX();
			double my = getMouseY();
			if (itemseli == -1) {
				if (mx > xp && mx<nextx && my>yp && my<nexty && pressed) {
					itemseli = i;
				}
			}
			if (itemseli == i) {
				if (cangetpress){
					if (mx>xp && mx<nextx && my>yp && my < nexty) {
						itemseli = -1;
						Item& selected = player->getItem(i);
						if (machines.isRendering()) {
							if(machines.currentIsPaid()) {
								machinem.lock();
								if(!machines.addItem(machines.getCurrentCoords(), selected)) cout << "Machine is full!" << endl;
								else player->sellItem(i);
								machinem.unlock();
							}
							else {
								machinem.lock();
								if(!machines.addItem(machines.getCurrentCoords(), selected)) cout << "Machine is full!" << endl;
								else player->removeItem(i);
								machinem.unlock();
							}
						}
						else { //(un)equip the item
							if (it.isEquipped()) player->unequipItem(it);
							else player->equipItem(it);
						}
					}
					else itemseli = -1;
				}       
			}
		}
	}

	void renderHUD() {
		RGBA mecol(255, 100, 255, 0);
		double w, h;
		double hudmul = 0.05;
		double ys = 0.2;
		double xs = 0.05;
		renderPart(0,0,0,0);
		//background
		applyTexture(hud_bg, 0.0, 1-ys, 1, ys);

		//hp
		getTextWH(main_font, "HP:", w, h);
		w *= hudmul / h;
		h = hudmul;
		displayText(main_font, "HP:", mecol, xs, 0.9, w, h);
		double pw = w;

		int hp_percent = player->getHealth()*100/player->getMaxHealth();
		getTextWH(main_font, (to_string(hp_percent)).c_str(), w, h);
		w *= hudmul / h;
		h = hudmul;
		displayText(main_font, to_string(hp_percent).c_str(), mecol, xs+pw, 0.9, w, h);
		pw += w;

		getTextWH(main_font, "%", w, h);
		w *= hudmul / h;
		h = hudmul;
		displayText(main_font, "%", mecol, xs+pw, 0.9, w, h);

		//score
		getTextWH(main_font, "SC:", w, h);
		w *= hudmul / h;
		h = hudmul;
		displayText(main_font, "SC:", mecol, 1-4.2*xs, 0.9, w, h);
		pw = w;
		getTextWH(main_font, to_string(highscore).c_str(), w, h);
		w *= hudmul / h;
		h = hudmul;
		displayText(main_font, to_string(highscore).c_str(), mecol, 1 - 4.2*xs + pw, 0.9, w, h);

		//weapons


		//attack cast bar
		if(cast_prog >= 0) {
			//applyTexture(hpgreen, 0.2, 0.9, cast_prog*0.15, 0.05);
		}

	}

	void renderInventory() {
		double ystart = 0.01;
		double xstart = 0.51;
		double itemx = 0.145;
		double itemy = 0.145;
		double framesp = 0.01;
		int rows = -1;
		applyTexture(MenuResources::background, xstart, ystart, 0.48, 0.85);

		double w, h;
		RGBA mecol(255, 255, 0, 0);
		getTextWH(main_font, "$", w, h);
		w *= 0.06 / h;
		h = 0.06;
		displayText(main_font, "$", mecol, 0.75, 0.76, w, h);

		getTextWH(main_font, to_string(player->getMoney()).c_str(), w, h);
		w *= 0.06 / h;
		h = 0.06;
		displayText(main_font, to_string(player->getMoney()).c_str(), mecol, 0.77, 0.76, w, h);


		int itemc = player->itemCount();
		for (int i = 0; i < itemc; ++i) {

			if (i % 3 == 0) ++rows;
			if (inv.try_lock()){
				Item& it = player->getNextItem();
				int tid = ItemResources::getTextureID(&typeid(it));
				inv.unlock();

				double xp = xstart + 0.027 + (i % 3)*itemx + framesp;
				double yp = rows*itemy + 5 * framesp;

				double nextx = xstart + 0.01 + ((i + 1) % 3)*itemx + framesp;
				if ((i + 1) % 3 == 0 && i > 0) nextx += 3 * itemx;
				double nexty = (rows + 1)*itemy + 5 * framesp;
				double mx = getMouseX();
				double my = getMouseY();

				applyTexture(tid, xp, yp, itemx, itemy);
				if (itemseli == i) applyTexture(MachineResources::frame_sel, xp, yp, itemx, itemy);
				else applyTexture(MachineResources::frame, xp, yp, itemx, itemy);

				if (mx > xp && mx<nextx && my>yp && my < nexty) itemover = i;
				else itemover = -1;

				double overx = 0.145, overy = 0.145;
				// Hover window with item data.
				//if(itemover == i) {
				//Derived& der = it.getDerivedBonuses();
				//if(1.0-nextx > overx)applyTexture(menu_bg, nextx, nexty, overx, overy);
				//else applyTexture(menu_bg, xp, nexty, overx, overy);

				//}
			}
		}

	}

	void dispStat(const char* name, int stat, double xstart, double& ystart, double offset) {
		RGBA mecol(255, 100, 255, 0);
		double w, h;
		renderPart(0, 0, 0, 0);
		getTextWH(main_font, name, w, h);
		w *= 0.03 / h;
		h = 0.03;
		displayText(main_font, name, mecol, xstart, ystart, w, h);

		getTextWH(main_font, to_string(stat).c_str(), w, h);
		w *= 0.03 / h;
		h = 0.03;
		displayText(main_font, to_string(stat).c_str(), mecol, xstart + 20 * offset, ystart, w, h);

		ystart += h*1.5;
	}

	void renderPlayerStats() {
		double ystart = 0.01;
		double xstart = 0.01;
		double itemx = 0.145;
		double itemy = 0.145;
		double framesp = 0.01;
		double pxstart = xstart + 8 * framesp + 1.5 / xsize;
		double pystart = ystart + 6* framesp;
		int rows = -1;
		applyTexture(MenuResources::background, xstart, ystart, 0.48, 0.85);
		renderPart(4, 2, 1, 0);
		applyTexture(BeingResources::getTextureID(&typeid(*player)), xstart + 4 * framesp, pystart + 2 * framesp, 1.5 / xsize, 1.5 / ysize);

		Derived der = player->getDerivedStats();
		Primary prim = player->getPrimaryStats();

		dispStat("Strength", prim.strength, pxstart, pystart, framesp);
		dispStat("Perception", prim.perception, pxstart, pystart, framesp);
		dispStat("Endurance", prim.endurance, pxstart, pystart, framesp);
		dispStat("Intelligence", prim.intelligence, pxstart, pystart, framesp);
		dispStat("Agility", prim.agility, pxstart, pystart, framesp);
		dispStat("Luck", prim.luck, pxstart, pystart, framesp);

		pxstart = xstart + 4.0*framesp;
		pystart = 5.4 / xsize;
		double off = 1.7;
		dispStat("Crit Chance", der.crit_chance, pxstart, pystart, framesp*off);
		dispStat("Crit Chance Bonus", der.crit_bonus, pxstart, pystart, framesp*off);
		dispStat("Damage Resistance", der.dmg_res, pxstart, pystart, framesp*off);
		dispStat("Damage Resistance Bonus", der.dmg_res_bonus, pxstart, pystart, framesp*off);
		dispStat("Fire Resistance", der.fire_res, pxstart, pystart, framesp*off);
		dispStat("Fire Resistance Bonus", der.fire_res_bonus, pxstart, pystart, framesp*off);

		pxstart = xstart + 4.0*framesp;
		pystart = 10.5 / xsize;
		for (auto& kv : player->getClassSkills()) {
			if (kv.second > 0) dispStat(kv.first, kv.second, pxstart, pystart, framesp*off);
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
			renderPart(0, 0, 0, 0);
			double hp_percent = (double)i->getHealth() / i->getMaxHealth();
			applyTexture(hpred, i->getX() - deltax, i->getY() - 0.02 - deltay, 0.08, 0.012);
			applyTexture(hpgreen, i->getX() - deltax, i->getY() - 0.02 - deltay, 0.08*hp_percent, 0.012);
		}
	}
	void displayProjectiles(){
		renderPart(0, 0, 0, 0);
		for (auto &i : projectiles){
			setRotationAngle(i.getAngleInDeg());
			applyTexture(WeaponResources::getAmmoTexture(i.getType()), i.getX() - deltax + 1.5*box.getStepX(), i.getY() - deltay + 1.5*box.getStepY(), box.getStepX() * 2, box.getStepY() * 2);
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
			if (!show_menu && !end_of_game){
				if (lock1.try_lock()){
					renderMap();
					lock1.unlock();
				}
				if (monster.try_lock()){
					displayMonsters();
					monster.unlock();
				}
				if (projectile.try_lock()){
					displayProjectiles();
					projectile.unlock();
				}
				if (playerm.try_lock()){
					displayPlayer();
					renderPart(0, 0, 0, 0);
					playerm.unlock();
				}
				displayHUD();
				if (render_inv) renderInventory();
				if (pstats) renderPlayerStats();
				machinem.lock();
					machines.render();
				machinem.unlock();
				
			}
			else if (show_menu){
				renderPart(0, 0, 0, 0);
				if (!((MP_server_init || MP_init) && SP_init)){
					menux.lock();
					curr_m->Render(*this, main_font);
					menux.unlock();
				}
				else{
					if (MP_server_init)displayText(main_font, ("Waiting for players to begin..." + to_string(MP_numconn) + "/" + to_string(MP_numplayers)).c_str(), RGBA(100, 255, 150, 0), 0.0, 0.0, 0, 0.1);
					else displayText(main_font, "Waiting for players to begin...", RGBA(100, 255, 150, 0), 0.0, 0.0, 0, 0.1);
				}
			}
			else if (end_of_game){
				displayText(main_font, ("Your score: " + to_string(highscore)).c_str(), RGBA(100, 255, 150, 0), 0.0, 0.0, 0, 0.1);
			}
			//else if (MP_noplayers)displayText(main_font, ("Waiting for players to begin..."+to_string(MP_numplayers)).c_str(), RGBA(100, 255, 150, 0), 0.0, 0.0, 0, 0.1);
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
				if(machinem.try_lock()) {
					machines.unset();
					machinem.unlock();
				}
				render_inv = false;
				int event = player->checkCollisions(lastxpos, lastypos, getMapIndex());
				switch (event){
				case OUT_OF_BOUNDS:
					if (!completed)break;
					if (tryRoomChange(player->getTileX(), player->getTileY())){
						c = getMapEntry();
						lock1.lock();
						player->setX(c.x);
						player->setY(c.y);
						lock1.unlock();
						if (getLastExploredRoom() > lastroom){
							completed = false;
							spawned = 0;
							if(machinem.try_lock()) {
								machines.clear();
								machinem.unlock();
							}
						}
					}
					break;
				case X_COLLIDE:
				case Y_COLLIDE:
				case XY_COLLIDE:
					if (lastxpos == player->getX() && lastypos == player->getY())player->resetWalk();  
					break;
				case TRIGGER:
					if (getMapIndex()[player->getTileX()][player->getTileY()] == VENDING || getMapIndex()[player->getTileX()][player->getTileY()] == DROP) {
						pair<int, int> p = make_pair(player->getTileX(), player->getTileY());
						if(machinem.try_lock()) {
							if (!machines.exists(p)) {
								machines.add(p,getMapIndex()[player->getTileX()][player->getTileY()] == VENDING ? true:false);
								machines.set(p);
								int items = pattern() % 10;
								for (int i = 0; i < 10; ++i){
									int item = pattern() % MAXITEMS;
									machines.addItem(machines.getCurrentCoords(), *item_types[item]());
								}
							}
							else machines.set(p);
							machinem.unlock();
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
			int event = player->tryToShoot(curr_target, &p, getMapIndex());
			switch (event){
			case BANG:
				projectiles.push_back(*p);
				cast_prog = -1;
				break;
			case CASTING:
				int* percent;
				percent = (int*)p;
				cast_prog = (double)(*percent)/100.0;
				delete percent;
				break;
			}
		}
		else {
			player->resetFire();
			cast_prog = -1;
		}
	}

	void tryToSpawn(){
		bool mustspawn = pattern() % getSpawnRate() == false;
		int spawntype = pattern() % monster_types.size();
		int x = pattern() % getMapIndex().size();
		int y = pattern() % getMapIndex()[x].size();
		if (mustspawn && !getMapIndex()[x][y]){
			//cout << getMaxMonsters() << endl;
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
			bool res = p->update(getMapIndex(), monsters, targets, getTicks());
			if (!res){
				projectile.lock();
				p = projectiles.erase(p);
				projectile.unlock();
			}
		}
	}

	void updateMonsters(){
		Being* closest = nullptr; //no targets in range
		double lastdist = 0.1; //0.1 is what??? idk but it works!
		//monster.lock();
		int counter = 0;
		for (auto m = begin(monsters); m != end(monsters); ++m){
			bool res = (*m)->action(getMapIndex(), projectiles, targets, getTicks());
			if (!res){
				addLoot((*m)->getTileX(), (*m)->getTileY());
				Being* ptr = &**m;
				//cout << ptr << endl;
				++counter;
				monster.lock();
				m = monsters.erase(m);
				monster.unlock();
				lock1.lock();
				++highscore;
				player->addExperience(50);
				lock1.unlock();
				curr_target = nullptr;
			}
			else{
				//if (mouseOverTarget((*m)->getX(), (*m)->getY()) && pressed){
				//curr_target = &**m;
				//}
				if (pressed){
					double mx = getMouseX() + deltax;
					double my = getMouseY() + deltay;
					double dist = sqrt(pow(((*m)->getX() - mx), 2) + pow(((*m)->getY() - my), 2));
					if (dist < lastdist){
						lastdist = dist;
						closest = &**m;
						curr_target = closest;
					}
				}
			}
		}
		if (counter)cout << counter << endl;
		//monster.unlock();
	}

	void networkLoop(){
		if (MP_server_init && SP_init){
			if (MP_numconn != MP_numplayers){
				MP_numconn = gatherPlayers() + 1;
				updateClients(targets, false);
			}
			else{
				updateClients(targets, false);
				char* cmd;
				short cmd_num;
				short data;
				list<Client>& cl = getClients();
				auto i = begin(cl);
				advance(i, conn_confirmed);
				for (; i != end(cl); ++i){
					cmd = getNextCommand(*i);
					if (cmd){
						memcpy(&cmd_num, &cmd[0], 2);
						memcpy(&data, &cmd[2], 2);
						switch (cmd_num){
						case LOGIN:
							if (data != sizeof(login))throw Error("Nope!");
							login l;
							memcpy(&l, &cmd[4], data);
							targets.push_back(unique_ptr<Being>(player_types[l.mp_class](0, 0)));
							++conn_confirmed;
							break;
						default:
							return;
						}
					}
					else return;
				}
				//all the clients are connected and initialized!
				if (conn_confirmed != MP_numplayers - 1){
					//someone has disconnected, reset server
					playerm.lock();
					targets.clear();
					playerm.unlock();
					MP_numconn = 1;
				}
				else{
					//finally ok
					list<Client>& cl = getClients();
					auto j = begin(targets);
					for (auto i = begin(cl); i != end(cl); ++i, ++j){
						info inf;
						inf.seed = seed;
						if (!commandToClient(i, INFO, sizeof(info), (char*)&inf)){
							playerm.lock();
							j = targets.erase(j);
							playerm.unlock();
						}
					}
					MP_server_init = false;
				}
			}
		}
		else MP_numconn = 1;
		if (MP_init && SP_init){
			login l;
			l.mp_class = SP_class;
			if (!sendCommand(LOGIN, sizeof(login), (char*)&l)){
				//server is down
				SP_init = false;
			}
			else {
				char* cmd;
				short cmd_num;
				short data;
				cmd = receiveCommand();
				memcpy(&cmd_num, &cmd[0], 2);
				memcpy(&data, &cmd[2], 2);
				switch (cmd_num){
				case INFO:
					if (data != sizeof(info))throw Error("Nope!");
					info i;
					memcpy(&i, &cmd[4], data);
					seed = i.seed;
					break;
				}
				MP_init = false;
			}
		}
		//initialize - ok

	}

	void mainLoop() final {
		try {
			if (SP_init && !MP_server_init && !MP_init){
				loadMap(seed);
				c = getMapEntry();
				targets.push_back(unique_ptr<Being>(player_types[SP_class](c.x, c.y))); player = &**targets.rbegin();
				for (auto i = targets.begin(); i != --targets.end(); ++i){
					(*i)->setX(c.x);
					(*i)->setY(c.y);
				}
				show_menu = false;
				SP_init = false;
			}
			if ((MP_init || MP_server_init) && SP_init){
				if (isPressed("ESCAPE")){
					SP_init = false;
				}
			}
			if (!show_menu && !end_of_game){
				if (updateInternalMapState()) dir = 0;
				bool dead = player->action(getMapIndex(), projectiles, targets, getTicks());
				if (!dead)end_of_game = true;
				moveAndColide();
				if(machinem.try_lock()) {
					machines.updateVars(deltax, deltay, pressed, cangetpress);
					machines.check(deltax, deltay, player->getTileX(), player->getTileY());
					machines.control(player);
					machinem.unlock();
				}
				playerShoot();
				tryToSpawn();
				updateMonsters();
				updateProjectiles();

				if (isPressed("I") && pre_inv == false) pre_inv = true;
				if (pre_inv && isPressed("I") == false) {
					render_inv = !render_inv;
					pre_inv = false;
				}
				if (machines.isRendering()) render_inv = true;
				if (render_inv) controlInventory();

				if (isPressed("P") && pre_pstats == false) pre_pstats = true;
				if (pre_pstats && isPressed("P") == false) {
					pstats = !pstats;
					pre_pstats = false;
				}

				//box.setX(getMouseX() + deltax);
				//box.setY(getMouseY() + deltay);
				getdir();

			}
			else if (show_menu){
				if (!((MP_server_init || MP_init) && SP_init)){
					menux.lock();
					curr_m = curr_m->Check(getMouseX(), getMouseY(), pressed, cangetpress, *this);
					menux.unlock();
				}
			}
			else if (end_of_game){
				if (isPressed("ESCAPE")){
					show_menu = true;
					end_of_game = false;
					targets.clear();
					monsters.clear();
					projectiles.clear();
					seed = system_clock::to_time_t(system_clock::now());
					highscore = 0, spawned = 0, lastroom = 0;
				}
			}
			updatePress();
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
		//if (c == nullptr){
		//	pi.last_command = -1;
		//	return true;
		//}
		//short cmd;
		//short data;
		//memcpy(&cmd, &c[0], 2);
		//memcpy(&data, &c[2], 2);
		//server_info i;
		//char* cd;
		//pi.last_command = cmd;
		//switch (cmd){
		//case KEYBOARD:
		//	if (data != 4)throw Error("Nope!");
		//	memcpy(&pi.keyboard, &c[4], data);
		//	break;
		//case GETINFO:
		//	if (data != 4)throw Error("Nope!");
		//	cout << (int)c[4] << endl;
		//	memcpy(&pi.class_, &c[4], data);
		//	i.n_players = MP_numplayers;
		//	i.gathering = MP_noplayers;
		//	i.game_end = false;
		//	i.seed = seed;
		//	cd = (char*)&i;
		//	return commandToClient(cl, SERVERINFO, sizeof(i), cd);
		//	break;
		//default:
		//	throw Error("Nope!");
		//}
		return true;
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
		if (a < 0) a = 1;
		Uint16* text = new Uint16[a + 1];
		for (int i = a - 1; i >= 0; --i){
			text[i] = (h % 10) + 48;
			h /= 10;
		}
		if (text[0] == 0) text[0] = '0';
		text[a] = 0;
		return text;
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
		//applyTexture(bg[maptype], - deltax, -deltay, (double)(getMapIndex().size() / (double)xsize), (double)(getMapIndex()[0].size() / (double)ysize));
		double room_x, room_y;
		char wpos, hpos;
		getRoomSize(room_x, room_y);
		for (int i = 0; i < getMapObjects().size(); ++i){
			double block_x = getMapObjects().at(i).x;
			double block_y = getMapObjects().at(i).y;
			if (block_x == 0)wpos = 0;
			else if (block_x == room_x - 1.0 / xsize)wpos = 1;
			else wpos = 2;
			if (block_y == 0)hpos = 0;
			else if (block_y == room_y - 1.0 / ysize)hpos = 1;
			else hpos = 2;
			double x = block_x - deltax;
			double y = block_y - deltay;
			switch (getMapObjects().at(i).type){
			case 0:
				applyTexture(bg[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				if (completed){
					renderPart(getMapIndex().size(), getMapIndex()[0].size(), block_x*xsize, block_y*ysize);
					applyTexture(red, x, y, 1.0 / xsize, 1.0 / ysize);
					renderPart(0, 0, 0, 0);
				}
				break;
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
				//cout << "??" << endl;
				//applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			}
			renderPart(0, 0, 0, 0);
			//if (completed && pattern() % 2)applyTexture(red, x, y, 1.0 / xsize, 1.0 / ysize);
		}
		//if (completed)applyTexture(red, -deltax, -deltay, (double)(getMapIndex().size() / (double)xsize), (double)(getMapIndex()[0].size() / (double)ysize));
	}
public:
	RINS() try : box(xsize, ysize, 4),
		Renderer(640, 640, "RINS"),
		dir(0), c(0, 0, 0),
		machines(*this, *this, box, 0){
			//Projectile::box = &box;
			Being::bx = &box;
			seed = system_clock::to_time_t(system_clock::now());

			BeingResources::addTextureID(loadTexture("Textures/hitler.png"), &typeid(Marine));
			BeingResources::addTextureID(loadTexture("Textures/devil2.png"), &typeid(Pyro));
			BeingResources::addTextureID(loadTexture("Textures/devil2.png"), &typeid(Psychokinetic));
			BeingResources::addTextureID(loadTexture("Textures/devil2.png"), &typeid(Android));
			BeingResources::addTextureID(loadTexture("Textures/gangsta2.png"), &typeid(Zombie));

			ItemResources::addTextureID(loadTexture("Textures/armour.png"), &typeid(BodyArmour));
			ItemResources::addTextureID(loadTexture("Textures/amp.png"), &typeid(PsychoAmp));
			ItemResources::addTextureID(loadTexture("Textures/scope.png"), &typeid(Scope));

			bg[BUILDING] = loadTexture("Textures/floor_tile.png");
			bg[HOSPITAL] = loadTexture("Textures/cement_tile.png");
			bg[LABYRINTH] = loadTexture("Textures/dirt_tile.png");

			wall[BUILDING] = loadTexture("Textures/brick3.png");
			wall[HOSPITAL] = loadTexture("Textures/brick4.png");
			wall[LABYRINTH] = loadTexture("Textures/brick5.png");

			side[BUILDING][0] = loadTexture("Textures/school_1.png");
			side[BUILDING][1] = loadTexture("Textures/school_2.png");
			side[HOSPITAL][0] = loadTexture("Textures/hospital_1.png");
			side[HOSPITAL][1] = loadTexture("Textures/hospital_2.png");
			side[LABYRINTH][0] = loadTexture("Textures/forest_1.png");
			side[LABYRINTH][1] = loadTexture("Textures/forest_2.png");

			//enum {BULLET, FIRE, PSYCHO, ENERGY};
			WeaponResources::addAmmoTexture(loadTexture("Textures/bullet.png"), BULLET);
			WeaponResources::addAmmoTexture(loadTexture("Textures/bullet4.png"), FIRE);
			WeaponResources::addAmmoTexture(loadTexture("Textures/bullet3.png"), PSYCHO);
			WeaponResources::addAmmoTexture(loadTexture("Textures/bullet2.png"), ENERGY);
			WeaponResources::addWeaponTexture(loadTexture("Textures/assault.png"), &typeid(AssaultRifle));

			entrytex = loadTexture("Textures/entry.png");
			exittex = loadTexture("Textures/exit.png");
			vendtex = loadTexture("Textures/vendtex.png");
			droptex = loadTexture("Textures/drop.png");
			red = loadTexture("Textures/red2.png");
			setModulateBlending(red);

			main_font = loadFont("Fonts/ARIALUNI.TTF", 42);

			monster_types[ZOMBIE] = &createInstance<Zombie>;
			item_types[BODYARMOR] = &createItem<BodyArmour>;
			item_types[SCOPE] = &createItem<Scope>;
			item_types[PSYCHOAMP] = &createItem<PsychoAmp>;
			player_types[MARINE] = &createInstance<Marine>;
			player_types[PYRO] = &createInstance<Pyro>;
			player_types[PSYCHOKINETIC] = &createInstance<Psychokinetic>;
			player_types[ANDROID] = &createInstance<Android>;


			setMusicVolume(MAX_VOL / 2);
			song1 = loadSong("Sounds/level1.mid");
			MenuResources::optionysize = 0.05;
			MenuResources::optionspacing = 0.01;
			MenuResources::optionxsize = 0.5;
			MenuResources::hsize = 0.8*MenuResources::optionysize;
			MenuResources::backxsize = 0.1;
			MenuResources::backysize = 0.1;
			MenuResources::background = loadTexture("Textures/background1.png");
			MenuResources::overlay = loadTexture("Textures/overlay1.png");
			MenuResources::back = loadTexture("Textures/back.png");
			MenuResources::mframe = loadTexture("Textures/mframe.png");
			setModulateBlending(MenuResources::overlay);
			MenuResources::textcol[0] = new RGBA(255, 100, 255, 0);
			MenuResources::textcol[1] = new RGBA(000, 100, 255, 0);
			MenuResources::addTexture(loadTexture("Textures/button1.png"), &typeid(MenuButton), IS_UNSET);
			MenuResources::addTexture(loadTexture("Textures/button1.png"), &typeid(MenuButton), IS_SET);
			MenuResources::addTexture(loadTexture("Textures/textc.png"), &typeid(MenuButton), ON_CLICK);
			MenuResources::addTexture(loadTexture("Textures/textn.png"), &typeid(TextBox), IS_UNSET);
			MenuResources::addTexture(loadTexture("Textures/button4.png"), &typeid(TextBox), IS_SET);
			MenuResources::addTexture(loadTexture("Textures/textc.png"), &typeid(TextBox), ON_CLICK);
			MenuResources::addTexture(loadTexture("Textures/button2.png"), &typeid(CheckBox), IS_UNSET);
			MenuResources::addTexture(loadTexture("Textures/button3.png"), &typeid(CheckBox), IS_SET);
			MenuResources::addTexture(loadTexture("Textures/textc.png"), &typeid(CheckBox), ON_CLICK);
			MenuResources::addTexture(loadTexture("Textures/button1.png"), &typeid(ClickBox), IS_UNSET);
			MenuResources::addTexture(loadTexture("Textures/button1.png"), &typeid(ClickBox), IS_SET);
			MenuResources::addTexture(loadTexture("Textures/textc.png"), &typeid(ClickBox), ON_CLICK);
			MenuResources::addTexture(loadTexture("Textures/button1.png"), &typeid(SlideBar), IS_UNSET);
			MenuResources::addTexture(loadTexture("Textures/slide.png"), &typeid(SlideBar), IS_SET);
			MenuResources::addTexture(loadTexture("Textures/textc.png"), &typeid(SlideBar), ON_CLICK);

			MachineResources::bg = MenuResources::background;
			MachineResources::frame = loadTexture("Textures/itemframe.png");
			MachineResources::frame_sel = loadTexture("Textures/itemframesel.png");
			hpgreen = loadTexture("Textures/hp_green.png");
			hpred = loadTexture("Textures/hp_red.png");
			hud_bg = loadTexture("Textures/hud_bg.png");

			Menu& m3 = *new Menu("Sounds like a menu", [this](){});
			m3.addField(*new CheckBox("Music: ", false, [this](CheckBox& mc){  enable_music = mc.is_on; if (mc.is_on)playSong(song1); else stopMusic(); }))
				.addField(*new SlideBar(50, "Volume:", [this](SlideBar& mc){   setMusicVolume((MAX_VOL / 100.0)*mc.slide_percent);   }));

			Menu& m4 = *new Menu("Choose wisely!", [this](){ disconncet();  });
			m4.addField(*new ClickBox("Pyro!", [this](ClickBox& mc){ SP_class = 1; SP_init = true; }))
				.addField(*new ClickBox("Marine!", [this](ClickBox& mc){ SP_class = 0; SP_init = true; }))
				.addField(*new ClickBox("Android!", [this](ClickBox& mc){ SP_class = 3; SP_init = true; }))
				.addField(*new ClickBox("Psychokinetic!", [this](ClickBox& mc){ SP_class = 2; SP_init = true; }));

			Menu& m5 = *new Menu("Are you... Yeah, sure.", [this](){});
			m5.addField(*new ClickBox("Yes!", [this](ClickBox& mc){ quit = true; }))
				.addField(*new ClickBox("Yes!", [this](ClickBox& mc){ quit = true; }));

			Menu& m1 = *new Menu("NumPlayers", [this](){MP_server_init = false; });
			m1.addField(*new MenuButton(m4, "xx", [this](MenuButton& mc){MP_numplayers = 2; MP_server_init = true; startServer(1337); }))
				.addField(*new MenuButton(m4, "xxxx", [this](MenuButton& mc){MP_numplayers = 4; MP_server_init = true;  startServer(1337); }))
				.addField(*new MenuButton(m4, "xxxxxxxx", [this](MenuButton& mc){MP_numplayers = 8; MP_server_init = true;  startServer(1337); }))
				.addField(*new MenuButton(m4, "xxxxxxxxxxxxxxxx", [this](MenuButton& mc){MP_numplayers = 16; MP_server_init = true;  startServer(1337); }));

			Menu& m6 = *new Menu("Serverlist", [this](){ disconncet();  });
			m6.addField(*new MenuButton(m4, "localhost", [this](MenuButton& mc){ ConnectToServer(1337, "127.0.0.1"); MP_init = true; }))
				.addField(*new TextBox("Add another: ", [this, &m6, &m4](TextBox& mt){ if (mt.done){ m6.addField(*new MenuButton(m4, mt.text, [&mt, this](MenuButton& mc){ ConnectToServer(1337, mt.text.c_str()); MP_init = true; })); mt.text = ""; } }));

			Menu& m2 = *new Menu("Multiplayer", [this](){ disconncet(); });
			m2.addField(*new MenuButton(m1, "Start server...", [this](MenuButton& mc){}))
				.addField(*new MenuButton(m6, "Connect to...", [this](MenuButton& mc){}));

			curr_m = new Menu("Main", [this](){});
			curr_m->addField(*new MenuButton(m4, "Singleplayer...", [this](MenuButton& mc){cout << "c01" << endl; }))
				.addField(*new MenuButton(m2, "Multiplayer...", [this](MenuButton& mc){cout << "c02" << endl; }))
				.addField(*new MenuButton(m3, "Music...", [this](MenuButton& mc){cout << "c03" << endl; }))
				.addField(*new MenuButton(m5, "Quit...", [this](MenuButton& mc){cout << "c04" << endl; }));
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