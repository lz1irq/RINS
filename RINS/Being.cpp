#include "Being.h"
//0.015625 = 1/64 (one being = one screen square; //what? one screen has 16 squares; one step is 1/4 square

using namespace std;
#include <iostream>
#include <typeinfo>

Hitbox* Being::bx;

Hitbox::Hitbox(int tiles_x, int tiles_y, int tile_granularity) : tile_granularity(tile_granularity),
	tiles_x(tiles_x), tiles_y(tiles_y), move_step_x(1.0 / (tile_granularity * tiles_x)),
	move_step_y(1.0 / (tile_granularity * tiles_y)){
}

void Hitbox::setX(double x){
	this->x = x;
}

void Hitbox::setY(double y){
	this->y = y;
}

double Hitbox::getX() const{
	return x;
}

double Hitbox::getY() const{
	return y;
}

int Hitbox::getTileX(){
	return ((x + move_step_x) / move_step_x) / ((1.0 / tiles_x) / move_step_x);
}

int Hitbox::getTileY(){
	return ((y + move_step_y * 3) / move_step_y) / ((1.0 / tiles_y) / move_step_y);
}

double Hitbox::getStepX() const{
	return move_step_x;
}

double Hitbox::getStepY() const{
	return move_step_y;
}

int Hitbox::checkCollisions(double comp_to_x, double comp_to_y, const vector<vector<char>>& index){
	double curr_x = x;
	double curr_y = y;
	x = comp_to_x;
	y = comp_to_y;
	int last_tile_x = getTileX();
	int last_tile_y = getTileY();
	x = curr_x;
	y = curr_y;
	int curr_tile_x = getTileX();
	int curr_tile_y = getTileY();
	if (!(curr_tile_x < 0 || curr_tile_x >= index.size())){
		if (!(curr_tile_y < 0 || curr_tile_y >= index[curr_tile_x].size())){
			if (index[curr_tile_x][curr_tile_y]){
				if (index[curr_tile_x][curr_tile_y] < 16){  //16 = #wall combinations; the magic tiles' ID's are > than 16
					if (index[last_tile_x][curr_tile_y] && index[curr_tile_x][last_tile_y]){
						setY(comp_to_y);
						setX(comp_to_x);
						return XY_COLLIDE;
					}
					if (index[last_tile_x][curr_tile_y]){
						setY(comp_to_y);
						return Y_COLLIDE;
					}
					if (index[curr_tile_x][last_tile_y]){
						setX(comp_to_x);
						return X_COLLIDE;
					}
				}
				return TRIGGER;
			}
			return STATUS_OK;
		}
		else {
			setX(comp_to_x);
			return OUT_OF_BOUNDS;
		}
	}
	else {
		setY(comp_to_y);
		return OUT_OF_BOUNDS;
	}
}

Being::Being(double x, double y, int speed) :Hitbox(*bx),
	orientation(UP), level(1), prim_stats(Primary()), 
	der_stats(prim_stats, level), curr_weapon(0), speed(speed) {
		this->x = x;
		this->y = y;
		rnd.seed(time(0));
	}

IDs::IDs() {
	ids[0] = 0;
	ids[1] = 0;
}

map<const type_info*, IDs> BeingResources::textures;

Derived& Being::getDerivedStats() {
	return der_stats;
}
Primary& Being::getPrimaryStats() {
	return prim_stats;
}

bool Being::move(int dir, bool reverse) {
	dir = dir & 15;
	if ((count + speed) < start_time) {
		int newdir = dir & 15;
		if (newdir)orientation = newdir;
		double move_x = reverse ? -move_step_x : move_step_x;
		double move_y = reverse ? -move_step_y : move_step_y;
		if (dir == LEFT) x -= move_x;
		else if (dir == RIGHT) x += move_x;
		else if (dir == UP) y -= move_y;
		else if (dir == DOWN) y += move_y;
		else{
			if (dir & LEFT) x -= move_x * 0.70710678118;
			if (dir & RIGHT) x += move_x * 0.70710678118;
			if (dir & UP) y -= move_y * 0.70710678118;
			if (dir & DOWN) y += move_y * 0.70710678118;
		}

		HAJA++;
		if (newdir && !(HAJA % 4))walk = !walk;
		count = start_time;
		return true;
	}
	return false;
}

int Being::getHealth() {
	return der_stats.health;
}

int Being::getMaxHealth() {
	return der_stats.max_health;
}

int Being::getOrientation() const {
	return orientation;
}

Projectile& Being::shootWeapon(double deg, Hitbox& h) {
	return (weapons.at(curr_weapon)->shoot(deg, x, y, h));
}

int Being::tryToShoot(Being* target, Projectile** p, const vector<vector<char>>& map_index){
	if (target){
		double dx = target->x - x;
		double dy = y - target->y;
		double deg = (atan2(dx, dy) - 0.5*M_PI);
		if (!isTargetBehindBack(dx, dy)){
			if (wallInFront(target->x, target->y, deg, map_index)){
				resetFire();
				return NOT_IN_LOS;
			}
			return internalShoot(target->x, target->y, deg, p);
		}
		else{
			resetFire();
			return NOT_IN_FOV;
		}
	}
	else resetFire();
}

void Being::resetFire(){
	weapons.at(curr_weapon)->getCount() = start_time;
}

int Being::internalShoot(double target_x, double target_y, double deg, Projectile** p){
	double range = sqrt(pow(target_x - x, 2) + pow(target_y - y, 2));
	double roundstep = (move_step_x + move_step_y) / 2.0;
	if (range > weapons.at(curr_weapon)->getFlyT()*roundstep)return OUT_OF_RANGE;
	if ((weapons.at(curr_weapon)->getCount() + weapons.at(curr_weapon)->getSpeed()) < start_time){
		resetFire();
		*p = &(shootWeapon(deg, *new Hitbox(tiles_x, tiles_y, tile_granularity)));
		return BANG;
	}
	else{
		int* percent = new int(((start_time - weapons.at(curr_weapon)->getCount()) * 100) / weapons.at(curr_weapon)->getSpeed());
		*p = (Projectile*)percent;
		return CASTING;
	}
}

bool Being::setCurrentWeapon(int new_wp) {
	if(new_wp > 0 && new_wp < weapons.size()) {
		curr_weapon = new_wp;
		return true;
	}
	return false;
}

int Being::getCurrentWeapon() {
	return curr_weapon;
}

WeaponBase& Being::getWeapon(int wp_id) {
	return *(weapons.at(wp_id));
}

int Being::weaponCount() {
	return weapons.size();
}

bool Being::wallInFront(double target_x, double target_y, double deg, const vector<vector<char>>& map_index){
	double being_x = x;
	double being_y = y;
	static Hitbox h1(*this);
	h1.setX(being_x);
	h1.setY(being_y);
	while (abs(being_x - target_x) > h1.getStepX() || abs(being_y - target_y) > h1.getStepY()){
		h1.setX(being_x + cos(deg) * h1.getStepX());
		h1.setY(being_y + sin(deg) * h1.getStepY());
		int event = h1.checkCollisions(being_x, being_y, map_index);
		if (event == STATUS_OK || event == TRIGGER){
			being_x = h1.getX();
			being_y = h1.getY();
		}
		else return true;
	}
	return false;
}

bool Being::isTargetBehindBack(double delta_x, double delta_y){
	int tx = 0, ty = 0;
	if (false);
	else if (orientation & LEFT)tx = -1;
	else if (orientation & RIGHT)tx = 1;
	else if (orientation & UP)ty = 1;
	else if (orientation & DOWN)ty = -1;
	if (delta_x*tx >= 0 && delta_y*ty >= 0)return false;
	else return true;
}

Item& Being::getNextItem() {
	inv.lock();
	++it;
	if(it == items.end()) it = items.begin();
	inv.unlock();
	return *(*it);
}

int Being::itemCount() {
	return items.size();
}

Item& Being::getItem(int item) {
	return *(items.at(item));
}

bool Being::buyItem(Item& item) {
	if(money < item.getPrice()) return false;
	else {
		money -= item.getPrice();
		return addItem(item);
	}

}

Item& Being::sellItem(int item) {
	inv.lock();
	Item& tosell = *(items.at(item));
	money += tosell.getPrice();
	if(tosell.isEquipped())unequipItem(tosell);
	items.erase(items.begin() + item);
	cout << "Sold " << tosell.getName() << endl;
	it = items.end();
	--it;
	inv.unlock();
	return tosell;
}

bool Being::addItem(Item& i){
	inv.lock();
	if(items.size() < MAX_ITEMS) {
		items.push_back(&i);
		it = items.end();
		--it;
		inv.unlock();
		return true;
		
	}
	inv.unlock();
	return false;
}

Item& Being::removeItem(int item) {
	inv.lock();
	Item& torm = *(items.at(item));
	items.erase(items.begin() + item);
	it = items.end();
	--it;
	inv.unlock();
	return torm;
}

int Being::getMoney() {
	return money;
}

bool Being::equipItem(Item& item) {
	for(auto &i: items) {
		if(i->isEquipped() && i->getName() == item.getName()) {
			cout << "You can't equip the same type of item more than once!" << endl;
			return false;
		}
	}

	if( item.checkClass(&typeid(*this))) {
		Primary prim = item.getPrimaryBonuses();
		Derived der = item.getDerivedBonuses();
		Specific spec = item.getSpecificBonuses();

		prim_stats.agility_bonus += prim.agility_bonus;
		prim_stats.endurance_bonus += prim.endurance_bonus;
		prim_stats.intelligence_bonus += prim.intelligence_bonus;
		prim_stats.luck_bonus += prim.luck_bonus;
		prim_stats.perception_bonus += prim.perception_bonus;
		prim_stats.strength_bonus += prim.strength_bonus;

		der_stats.crit_bonus += der.crit_bonus;
		der_stats.dmg_res_bonus += der.dmg_res_bonus;
		der_stats.fire_res_bonus += der.fire_res_bonus;
		der_stats.melee_dmg_bonus += der.melee_dmg_bonus;

		item.setEquipped(true);
		cout << "Player equipped " << item.getName() << endl;
		return true;
	}
	else { 
		cout << "Can't equip this item" << endl;
		return false;
	}

}

void Being::unequipItem(Item& item) {
	for(auto &i : items) {
		if(i == &item) {
			Primary prim = item.getPrimaryBonuses();
			Derived der = item.getDerivedBonuses();
			Specific spec = item.getSpecificBonuses();
			
			prim_stats.agility_bonus -= prim.agility_bonus;
			prim_stats.endurance_bonus -= prim.endurance_bonus;
			prim_stats.intelligence_bonus -= prim.intelligence_bonus;
			prim_stats.luck_bonus -= prim.luck_bonus;
			prim_stats.perception_bonus -= prim.perception_bonus;
			prim_stats.strength_bonus -= prim.strength_bonus;
			
			der_stats.crit_bonus -= der.crit_bonus;
			der_stats.dmg_res_bonus -= der.dmg_res_bonus;
			der_stats.fire_res_bonus -= der.fire_res_bonus;
			der_stats.melee_dmg_bonus -= der.melee_dmg_bonus;
			
			item.setEquipped(false);
			cout << "Player unequipped " << item.getName() << endl;
		}
	}

}

void Being::takeProjectile(Projectile& bullet) {
	int def_skill = 0;
	unsigned int dmg_type = bullet.getType();

	if(dmg_type == BULLET || dmg_type == ENERGY) def_skill = der_stats.dmg_res+ der_stats.dmg_res_bonus;
	else if(dmg_type == PSYCHO) def_skill = 0;
	else if(dmg_type == FIRE) def_skill = der_stats.fire_res + der_stats.fire_res_bonus;

	if((bullet.getDamage() - def_skill) > der_stats.health) der_stats.health = 0;
	else {
		if(bullet.getDamage() - def_skill > 0)der_stats.health -= (bullet.getDamage() - def_skill);
	}
	//cout << bullet.getShooter() << " " << bullet.getDamage() - def_skill << endl;

	if (bullet.getDamage() - def_skill > curr_threat || curr_target == nullptr){
		curr_threat = bullet.getDamage() - def_skill;
		curr_target = bullet.getShooter();
	}
}

int Being::getLevel(){
	return level;
}

bool Being::getWalk(){
	return walk;
}

void Being::resetWalk(){
	walk = false;
}

mt19937 Being::rnd;
void Being::addExperience(int xp) {
	experience += xp;
}

void Being::levelup() {
	++level;
	experience = 0;
	//der_stats.max_health = 90 + prim_stats.endurance*2 + 10*level;
	der_stats.dmg_res =  prim_stats.agility*1.5 + level/2;
	for( auto it = weapons.begin(); it != weapons.end(); it++) {
		(*it)->updateDMG();
	}

}

int Being::getExperience() {
	return experience;
}

void Being::walkAround(const vector<vector<char>>& map_index){
	if (!move_dist){
	A:
		int t = rnd() % 4;
		if ((move_dir == LEFT) && ((1 << t) == RIGHT) || (move_dir == RIGHT) && ((1 << t) == LEFT))goto A;
		if ((move_dir == UP) && ((1 << t) == DOWN) || (move_dir == DOWN) && ((1 << t) == UP))goto A;
		move_dir = 1 << t;
		int mx = getTileX();
		int my = getTileY();
		switch (move_dir){
		case UP:
			for (int i = my; i > 0; --i){
				if (map_index[mx][i] == 0 || map_index[mx][i] > 16)++move_dist;
				else break;
			}
			break;
		case DOWN:
			for (int i = my; i < map_index[mx].size(); ++i){
				if (map_index[mx][i] == 0 || map_index[mx][i] > 16)++move_dist;
				else break;
			}
			break;
		case LEFT:
			for (int i = mx; i > 0; --i){
				if (map_index[i][my] == 0 || map_index[i][my] > 16)++move_dist;
				else break;
			}
			break;
		case RIGHT:
			for (int i = mx; i < map_index.size(); ++i){
				if (map_index[i][my] == 0 || map_index[i][my] > 16)++move_dist;
				else break;
			}
			break;
		}
		move_dist = rnd() % move_dist;
		move_dist *= tile_granularity;
	}
	else{
		double curr_x = getX();
		double curr_y = getY();
		if (move(move_dir, false)){
			--move_dist;
			int state = checkCollisions(curr_x, curr_y, map_index);
			if (state == OUT_OF_BOUNDS){
				x = curr_x;
				y = curr_y;
			}
		}
	}
}

void Being::updateTarget(const vector<vector<char>>& map_index, const list<unique_ptr<Being>>& targets){
	if (curr_target == nullptr){
		walkAround(map_index);
		for (auto& i : targets){
			Being* ptr = &*i;
			double dx = ptr->getX() - x;
			double dy = y - ptr->getY();
			double deg = (atan2(dx, dy) - 0.5*M_PI);
			if (!isTargetBehindBack(dx, dy)){
				if (!wallInFront(ptr->getX(), ptr->getY(), deg, map_index)){
					if (abs(ptr->getTileX() - getTileX()) < tiles_x / 2 && abs(ptr->getTileY() - getTileY()) < tiles_y / 2){
						move_dist = 0;
						curr_threat = 0;
						curr_target = ptr;
						break;
					}
				}
			}
		}
	}
	else{
		bool flag = false;
		for (auto& i : targets){
			Being* ptr = &*i;
			if (ptr == curr_target){
				flag = true;
				break;
			}
		}
		if (!flag)curr_target = nullptr;//target dead!
	}
}

map<const char*, int>& Being::getClassSkills() {
	return classSkills;
}

Being::~Being() {
	weapons.clear();
}

Marine::Marine(double sx, double yx): Being(sx,yx, 33) {
	classSkills["Small Guns"] = 2 + prim_stats.agility*2 + prim_stats.luck/2;
	classSkills["Small Guns Bonus"] = 0;
	classSkills["Big Guns"] = 2 + prim_stats.endurance*2 + prim_stats.luck/2;
	classSkills["Big Guns Bonus"] = 0;
	classSkills["Energy Weapons"] = 2 + prim_stats.perception*2 + prim_stats.luck/2;
	classSkills["Energy Weapons Bonus"] = 0;
	money = 1000;
	weapons.push_back(std::unique_ptr<WeaponBase>(new AssaultRifle(classSkills.at("Small Guns"), this)));
}

void Marine::setRange(){
	range = 100;
}             

bool Marine::action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) {
	this->start_time = start_time;
	if(experience == (level+1)*100 )levelup();
	if (der_stats.health == 0){
		return false;
	}
	return true;
}

void Marine::levelup() {
	classSkills.at("Small Guns") += 1 ;
	classSkills.at("Big Guns") += 1 ;
	classSkills.at("Energy Weapons") += 1 ;
	Being::levelup();
	der_stats.max_health += 10*level;
	der_stats.health += 10*level;
	cout << "Marine is now level " << level << endl;
}

Pyro::Pyro(double sx, double yx): Being(sx,yx, 33) {
	classSkills["Explosives"] = 2 + prim_stats.perception*2 + prim_stats.luck/2;
	classSkills["Big Guns"] = 2 + prim_stats.endurance*2 + prim_stats.luck/2;
	classSkills["Fire"] = 2 + prim_stats.agility + prim_stats.luck/2;
	classSkills["Explosives Bonus"] = 0;
	classSkills["Big Guns Bonus"] = 0;
	classSkills["Fire Bonus"] = 0;

	der_stats.dmg_res += 3;
	money = 1000;
	weapons.push_back(std::unique_ptr<Molotov>(new Molotov(classSkills.at("Explosives"), this)));
}

void Pyro::setRange(){
	range = 100;
}

bool Pyro::action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) {
	this->start_time = start_time;
	if(experience == (level+1)*100 )levelup();
	if (der_stats.health == 0){
		return false;
	}
	return true;
}

void Pyro::levelup() {
	classSkills.at("Explosives") += 1 ;
	classSkills.at("Big Guns") += 1 ;
	classSkills.at("Fire") += 1 ;
	Being::levelup();
	der_stats.max_health += 10*level;
	der_stats.health += 10*level; 
	cout << "Pyro is now level " << level << endl;
}

Psychokinetic::Psychokinetic(double sx, double yx): Being(sx,yx, 33) {
	classSkills["Mind Infiltration"] = 2 + prim_stats.intelligence*2 + prim_stats.luck/2;
	classSkills["Mental Power"] = 2 + prim_stats.endurance + prim_stats.intelligence + prim_stats.luck/2;
	classSkills["Fire"] = 2 + prim_stats.agility*2 + prim_stats.luck/2;
	classSkills["Mind Infiltration Bonus"] = 0;
	classSkills["Mental Power Bonus"] = 0;
	classSkills["Fire Bonus"] = 0;
	money = 1000;
	weapons.push_back(std::unique_ptr<WeaponBase>(new Pyrokinesis(classSkills.at("Fire"), this)));
}

void Psychokinetic ::setRange(){
	range = 100;
}

bool Psychokinetic::action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) {
	this->start_time = start_time;
	if(experience == (level+1)*100 )levelup();
	if (der_stats.health == 0){
		return false;
	}
	return true;
}

void Psychokinetic::levelup() {
	classSkills.at("Mind Infiltration") += 1 ;
	classSkills.at("Mental Power") += 1 ;
	classSkills.at("Fire") += 1 ;
	Being::levelup();
	der_stats.max_health += 10*level;
	der_stats.health += 10*level;
	cout << "Psycho is now level " << level << endl;
}

Android::Android(double sx, double yx): Being(sx,yx, 33) {
	classSkills["Punch"] = 2 + prim_stats.strength/2 + prim_stats.luck/2;
	classSkills["Big Guns"] = 2 + prim_stats.endurance*2 + prim_stats.luck/2;
	classSkills["Energy Weapons"] = 2 + prim_stats.perception*2 + prim_stats.luck/2;
	classSkills["Punch Bonus"] = 0;
	classSkills["Big Guns Bonus"] = 0;
	classSkills["Energy Weapons Bonus"] = 0;
	money = 1000;
	weapons.push_back(std::unique_ptr<WeaponBase>(new Punch(classSkills.at("Punch"), this)));
}

void Android::setRange(){
	range = 100;
}

bool Android::action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) {
	this->start_time = start_time;
	if(experience == (level+1)*100 )levelup();
	if (der_stats.health == 0){
		cout << "DROID DEAD" << endl;
		return false;
	}
	return true;
}

void Android::levelup() {
	classSkills.at("Punch") += 1 ;
	classSkills.at("Big Guns") += 1 ;
	classSkills.at("Energy Weapons") += 1 ;
	Being::levelup();
	der_stats.max_health += 10*level;
	der_stats.health += 10*level;
	cout << "Android is now level " << level << endl;
}

Zombie::Zombie(double sx, double yx): 
	Being(sx,yx, 50), target(nullptr) {
	classSkills["Biting"] = 2 + prim_stats.strength/2 + prim_stats.luck/2;

	weapons.push_back(std::unique_ptr<WeaponBase>(new Bite(classSkills.at("Biting"), this)));
}

void Zombie::setRange(){
	range = 100;
}

bool Zombie::action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) {
	this->start_time = start_time;
	bool revdir = false;
	if (der_stats.health < 50)revdir = true;
	if(der_stats.health == 0) {
		cout << "ZOMBIE DEAD" << endl;
		return false;
	}
	updateTarget(map_index, targets);
	if (curr_target){

		double dx = curr_target->getX() - x;
		double dy = y - curr_target->getY();
		double deg = (atan2(dx, dy) - 0.5*M_PI);
		if (abs(dx) > tiles_x || abs(dy) > tiles_y){//give up!
			curr_target = nullptr;
		}
		int ori;
		if (!wallInFront(curr_target->getX(), curr_target->getY(), deg, map_index)){
			if (dx < 0)ori = LEFT;
			else ori = RIGHT;
			if (dy < 0)ori += DOWN;
			else ori = UP;
			if (dx || dy){
				double curr_x = getX();
				double curr_y = getY();
				if (move(ori, revdir)){
					int state = checkCollisions(curr_x, curr_y, map_index);
					if (state == OUT_OF_BOUNDS){
						x = curr_x;
						y = curr_y;
					}
				}
			}
		}
		else{
				double curr_x = getX();
				double curr_y = getY();
				if (move(orientation, revdir)){
					int state = checkCollisions(curr_x, curr_y, map_index);
					if (state == OUT_OF_BOUNDS){
						x = curr_x;
						y = curr_y;
					}
					if (state == X_COLLIDE){
						if (dy < 0)orientation = DOWN;
						else orientation = UP;
					}
					if (state == Y_COLLIDE){
						if (dx < 0)orientation = LEFT;
						else orientation = RIGHT;
					}
					if (state == XY_COLLIDE){
						if (dy < 0)orientation = UP;
						else orientation = DOWN;
						if (dx < 0)orientation += RIGHT;
						else orientation += LEFT;
					}
				}
		}
		Projectile* p;
		int event = internalShoot(curr_target->getX(), curr_target->getY(), deg, &p);
		switch (event){
		case BANG:
			projectiles.push_back(*p);
			break;
		case CASTING:
			break;
		//default:
			//resetFire();
		}
	}
	else resetFire();
	return true; 
	
}

void Zombie::levelup() {
	biting += 1;
	Being::levelup();
}

const int BeingResources::getTextureID(const type_info* ti) {
	return textures.at(ti).ids[0];
}

void BeingResources::addTextureID(int newID, const type_info* ti) {
	textures[ti].ids[0] = newID;
}

const int BeingResources::getSoundID(const type_info* si) {
	return textures.at(si).ids[1];
}

void BeingResources::addSoundID(int newID, const type_info* si) {
	textures[si].ids[1] = newID;
}