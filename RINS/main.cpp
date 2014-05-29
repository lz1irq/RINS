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

	Menu* curr_m;
	bool typing = false, muststop = false;
	bool show_menu = true;
	bool enable_music = false;

	bool SP_init = false;
	bool MP_server_init = false;
	bool MP_numplayers = 0;
	//bool has_MP_server = false;
	int SP_class;
	//bool MP_noplayers = false;
	time_t seed;
	//int MP_numplayers = 0;
	//bool MP_init = false;
	//bool MP_mode = false;


	bool render_inv = false, pre_inv = false;
	int itemsel = -1, itemseli = -1;
	int itemover = -1;

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
		if(box.getTileX() == tx && box.getTileY() == ty)return true;
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
						 if(machines.isRendering()) player->sellItem(i);
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
					 }
					 else itemseli = -1;
				 }
			 }

		 }
		 return;
	 }

	void renderHUD() {
		RGBA mecol(255, 255, 0, 0);
		double w,h;
		double hudmul = 0.05;
		/* numeric health display
		getTextWH(main_font, to_string(player->getHealth()).c_str(), w, h);
		w *= 0.06/h;
		h = 0.06;
		displayText(main_font, to_string(player->getHealth()).c_str(),mecol, 0.005, 0.94,w,h); */

		renderPart(0,0,0,0);
		double hp_percent = (double) player->getHealth()/player->getMaxHealth();
		applyTexture(hpred, 0.005, 0.9, 0.15, 0.05);
		if(player->getHealth() > 0)applyTexture(hpgreen, 0.005, 0.9, hp_percent*0.15, 0.05);

		getTextWH(main_font, "HEALTH", w, h);
		w *= hudmul/h;
		h = hudmul;
		displayText(main_font, "HEALTH",mecol, 0.005, 0.9,w,h);
		
		// score
		getTextWH(main_font, "SCORE:", w, h);
		w *= hudmul/h;
		h = hudmul;
		displayText(main_font, "SCORE:",mecol, 1 - w, 0.9,w,h);
				
		getTextWH(main_font, to_string(highscore).c_str(), w, h);
		w *= hudmul/h;
		h = hudmul;
		displayText(main_font, to_string(highscore).c_str(),mecol, 1-w, 0.94,w,h);
	}

	void renderInventory() {
		double ystart = 0.01;
		double xstart = 0.51;
		double itemx = 0.145;
		double itemy = 0.145;
		double framesp = 0.01;
		int rows = -1;
		applyTexture(MenuResources::background, xstart, ystart, 0.48,0.85);

		double w,h;
		RGBA mecol(255, 255, 0, 0);
		getTextWH(main_font, "$", w, h);
		w *= 0.06/h;
		h = 0.06;
		displayText(main_font, "$",mecol, 0.75, 0.76,w,h);

		getTextWH(main_font, to_string(player->getMoney()).c_str(), w, h);
		w *= 0.06/h;
		h = 0.06;
		displayText(main_font, to_string(player->getMoney()).c_str(),mecol, 0.77, 0.76,w,h);


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
			if(itemseli == i) applyTexture(MachineResources::frame_sel, xp, yp, itemx, itemy);
			else applyTexture(MachineResources::frame, xp, yp, itemx, itemy);

			if(mx>xp && mx<nextx && my>yp && my<nexty) itemover = i;
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

	void dispStat(const char* name, int stat, double xstart, double& ystart, double offset) {
		RGBA mecol(255, 255, 0, 0);
		double w,h;
		renderPart(0,0,0,0);
		getTextWH(main_font, name, w, h);
		w *= 0.04/h;
		h = 0.02;
		displayText(main_font, name, mecol, xstart, ystart,w,h);

		getTextWH(main_font, to_string(stat).c_str(), w, h);
		w *= 0.04/h;
		h = 0.02;
		displayText(main_font, to_string(stat).c_str(),mecol, xstart+20*offset, ystart,w,h);

		ystart += h*1.5;
	}

	void renderPlayerStats() {
		double ystart = 0.01;
		double xstart = 0.01;
		double itemx = 0.145;
		double itemy = 0.145;
		double framesp = 0.01;
		double pxstart = xstart+8*framesp+1.5/xsize;
		double pystart = ystart+6*framesp;
		int rows = -1;
		applyTexture(MenuResources::background, xstart, ystart, 0.48,0.85);
		renderPart(4,2,1,0);
		applyTexture(BeingResources::getTextureID(&typeid(*player)), xstart + 4*framesp, pystart+2*framesp, 1.5/xsize, 1.5/ysize);

		Derived der = player->getDerivedStats();
		Primary prim = player->getPrimaryStats();

		dispStat("Strength", prim.strength, pxstart, pystart, framesp);
		dispStat("Perception", prim.perception, pxstart, pystart, framesp);
		dispStat("Endurance", prim.endurance, pxstart, pystart, framesp);
		dispStat("Intelligence", prim.intelligence, pxstart, pystart, framesp);
		dispStat("Agility", prim.agility, pxstart, pystart, framesp);
		dispStat("Luck", prim.luck, pxstart, pystart, framesp);

		pxstart = xstart + 4.0*framesp;
		pystart = 4.8/xsize;
		double off = 1.7;
		dispStat("Crit Chance", der.crit_chance, pxstart, pystart, framesp*off);
		if(der.crit_bonus > 0) dispStat("Crit Chance Bonus", der.crit_bonus, pxstart, pystart, framesp*off);
		dispStat("Damage Resistance", der.dmg_res, pxstart, pystart, framesp*off);
		if(der.dmg_res_bonus > 0) dispStat("Damage Resistance Bonus", der.dmg_res_bonus, pxstart, pystart, framesp*off);
		dispStat("Fire Resistance", der.fire_res, pxstart, pystart, framesp*off);									  
		if(der.fire_res_bonus > 0) dispStat("Fire Resistance Bonus", der.fire_res_bonus, pxstart, pystart, framesp*off);

		pxstart = xstart + 4.0*framesp;
		pystart = 10.5/xsize;
		for (auto& kv : player->getClassSkills()) {
			if(kv.second > 0) dispStat(kv.first, kv.second, pxstart, pystart, framesp*off);
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
			renderPart(0,0,0,0);
			double hp_percent = (double) i->getHealth()/i->getMaxHealth();
			applyTexture(hpred,   i->getX() - deltax, i->getY() - 0.02- deltay , 0.08, 0.012);
			applyTexture(hpgreen, i->getX() - deltax, i->getY() - 0.02- deltay , 0.08*hp_percent, 0.012);
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
			if (!show_menu){
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
				if(render_inv) renderInventory();
				if(pstats) renderPlayerStats();
				lock1.lock();
				displayHUD();
				lock1.unlock();
				machines.render();
			}
			else if (show_menu){
				renderPart(0, 0, 0, 0);
				menux.lock();
				curr_m->Render(*this, main_font);
				//renderMenu();
				menux.unlock();
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
				machines.unset();
				render_inv = false;
				int event = player->checkCollisions(lastxpos, lastypos, getMapIndex());
				switch (event){
				case OUT_OF_BOUNDS:
					if (!completed)break;
					lock1.lock();
					if (tryRoomChange(player->getTileX(), player->getTileY())){
						c = getMapEntry();
						player->setX(c.x);
						player-> setY(c.y);
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
					if (getMapIndex()[player->getTileX()][player->getTileY()] == VENDING || getMapIndex()[player->getTileX()][player->getTileY()] == DROP) {
							pair<int, int> p = make_pair(player->getTileX(),player->getTileY());
							if(!machines.exists(p)) {
								machines.add(p);
								int items = pattern() % 10;
								for (int i = 0; i < 10; ++i){
									int item = pattern() % MAXITEMS;
									machines.addItem(p,*item_types[item]());
								}
							}
							machines.set(p);
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
			bool res = p->update(getMapIndex(), monsters, targets, getTicks());
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

	void networkLoop(){


	}

	void mainLoop() final {
		try {
			if (SP_init){
				loadMap(seed);
				c = getMapEntry();
				targets.push_back(unique_ptr<Being>(player_types[SP_class](c.x, c.y))); player = &**targets.begin();
				show_menu = false;
				SP_init = false;
			}
			if (!show_menu){
				if (updateInternalMapState()) dir = 0;
				player->action(getMapIndex(), projectiles, targets, getTicks());
				moveAndColide();
				machines.updateVars(deltax, deltay, pressed, cangetpress);
				machines.control(player);
				playerShoot();
				tryToSpawn();
				updateMonsters();
				updateProjectiles();

				if (isPressed("I") && pre_inv == false) pre_inv = true;
				if(pre_inv && isPressed("I") == false) {
					render_inv = !render_inv;
					pre_inv = false;
				}
				if(machines.isRendering()) render_inv = true;
				if (render_inv) controlInventory();

				if (isPressed("P") && pre_pstats== false) pre_pstats = true;
				if(pre_pstats && isPressed("P") == false) {
					pstats = !pstats;
					pre_pstats = false;
				}

					box.setX(getMouseX() + deltax);
					box.setY(getMouseY() + deltay);
				getdir();
				machines.check(deltax, deltay, player->getTileX(), player->getTileY());
			}
			else if (show_menu){
				menux.lock();
				curr_m = curr_m->Check(getMouseX(), getMouseY(), pressed, cangetpress, *this);
				menux.unlock();
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
		Renderer(900, 900, "RINS"), 
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

		main_font = loadFont("Fonts/ARIALUNI.TTF", 42);

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

		MachineResources::bg = MenuResources::background;
		MachineResources::frame = loadTexture("Textures/itemframe.png");
		MachineResources::frame_sel = loadTexture("Textures/itemframesel.png");
		
		hpgreen = loadTexture("Textures/hp_green.png");
		hpred = loadTexture("Textures/hp_red.png");

		Menu& m3 = *new Menu("Sounds like a menu");
		m3.addField(*new CheckBox("Music: ", false, [this](MenuControl& mc){  
			enable_music = static_cast<CheckBox&>(mc).is_on; if (enable_music)playSong(song1); else stopMusic(); }));
		//	.addField(*new Button("Main menu", menu));

		Menu& m4 = *new Menu("Choose wisely!");
		m4.addField(*new ClickBox("Pyro!", [this](MenuControl& mc){ SP_class = 1; SP_init = true; }))
			.addField(*new ClickBox("Marine!", [this](MenuControl& mc){ SP_class = 0; SP_init = true; }))
			.addField(*new ClickBox("Android!", [this](MenuControl& mc){ SP_class = 3; SP_init = true; }))
			.addField(*new ClickBox("Psychokinetic!", [this](MenuControl& mc){ SP_class = 2; SP_init = true; }));

		Menu& m5 = *new Menu("Are you... Yeah, sure.");
		m5.addField(*new ClickBox("Yes!", [this](MenuControl& mc){ quit = true; }))
			.addField(*new ClickBox("Yes!", [this](MenuControl& mc){ quit = true; }));
		
		Menu& m1 = *new Menu("NumPlayers");
		m1.addField(*new MenuButton(m4, "xx", [this](MenuControl& mc){MP_numplayers = 2; MP_server_init = true; }))
			.addField(*new MenuButton(m4, "xxxx", [this](MenuControl& mc){MP_numplayers = 4; MP_server_init = true;  }))
			.addField(*new MenuButton(m4, "xxxxxxxx", [this](MenuControl& mc){MP_numplayers = 8; MP_server_init = true;  }))
			.addField(*new MenuButton(m4, "xxxxxxxxxxxxxxxx", [this](MenuControl& mc){MP_numplayers = 16; MP_server_init = true;  }));


		Menu& m2 = *new Menu("Multiplayer");
		m2.addField(*new MenuButton(m1, "Start server...", [this](MenuControl& mc){}));

		curr_m = new Menu("Main");
		//Menu* m = new Menu();
		//Menu* t = new Menu();

		//t->addField(*new MenuButton(*m, "Link", [this](MenuControl& mc){cout << "cli-cli" << endl;}));
		//m->addField(*new TextBox("Type: ", [this](MenuControl& mc){cout << dynamic_cast<TextBox&>(mc).done << endl;}))
		//	.addField(*new CheckBox("You turn me: ", true, [this](MenuControl& mc){cout << "cli-cli" << endl;}));

		curr_m->addField(*new MenuButton(m4, "Singleplayer...", [this](MenuControl& mc){ MP_server_init = false; }))
			.addField(*new MenuButton(m2, "Multiplayer...", [this](MenuControl& mc){cout << "c02" << endl; }))
			.addField(*new MenuButton(m3, "Music...", [this](MenuControl& mc){cout << "c03" << endl; }))
			.addField(*new MenuButton(m5, "Quit...", [this](MenuControl& mc){cout << "c04" << endl; }));
		//	.addField(*new MenuButton(*m, "B2B", [this](MenuControl& mc){cout << "cli-cli" << endl;}));
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