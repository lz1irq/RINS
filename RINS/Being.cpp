#include "Being.h"
//0.015625 = 1/64 (one being = one screen square; //what? one screen has 16 squares; one step is 1/4 square

using namespace std;
#include <iostream>

vector<Being*> Being::targets;
list<Projectile> Being::projectiles;
Hitbox* Being::box;

Primary::Primary():	
	strength(5), strength_bonus(0),
	perception(5), perception_bonus(0),
	endurance(5), endurance_bonus(0),
	intelligence(5), intelligence_bonus(0),
	agility(5), agility_bonus(0),
	luck(5), luck_bonus(0) {}

Derived::Derived(Primary prim, int level):
	crit_bonus(0), dmg_res_bonus(0),
	melee_dmg_bonus(0), fire_res(0), fire_res_bonus(0) {
	crit_chance = prim.luck * 0.01;
	health = 90 + prim.endurance*2 + 10*level;
	melee_dmg = prim.strength/2;
	dmg_res = prim.agility*1.5;
}

Being::Being(double x, double y) : Hitbox(*box),
	orientation(UP), level(1), prim_stats(Primary()), 
	der_stats(prim_stats, level), curr_weapon(0) {
		this->x = x;
		this->y = y;
		rnd.seed(time(0));
	}

map<const char*, int> BeingResources::textures;

void Being::move(int dir, bool reverse) {
	int newdir = dir & 15;
	if(newdir)orientation = newdir;
	double move_x = reverse ? -move_step_x : move_step_x;
	double move_y = reverse ? -move_step_y : move_step_y;
	if (dir & LEFT) x-= move_x;
	if (dir & RIGHT) x += move_x;
	if (dir & UP) y -= move_y;
	if (dir & DOWN) y += move_y;
	if (newdir)walk = !walk;
}

int Being::getHealth() {
	return der_stats.health;
}

int Being::getOrientation() const {
	return orientation;
}

void Being::shootWeapon(double deg, Hitbox& h) {
	projectiles.push_back((weapons.at(curr_weapon)->shoot(deg, x, y, h)));
}

void Being::nextWeapon() {
	if(curr_weapon < weapons.size()) ++curr_weapon;
	else curr_weapon = 0;
}

void Being::prevWeapon() {
	if(curr_weapon > 0) ++curr_weapon;
	else curr_weapon = weapons.size();
}

void Being::takeProjectile(Projectile& bullet) {
	int def_skill = 0;
	unsigned int dmg_type = bullet.getType();

	if(dmg_type == BULLET || dmg_type == ENERGY) def_skill = der_stats.dmg_res+ der_stats.dmg_res_bonus;
	else if(dmg_type == PSYCHO) def_skill = 0;
	else if(dmg_type == FIRE) def_skill = der_stats.fire_res + der_stats.fire_res_bonus;

	if((bullet.getDamage() - def_skill) > der_stats.health) der_stats.health = 0;
	else der_stats.health -= (bullet.getDamage() - def_skill);
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

array<Being*(*)(double, double), MAXSIZE> Being::monsters;

Being::~Being() {
	weapons.clear();
	projectiles.clear();
}

Marine::Marine(double sx, double yx): 
	Being(sx,yx), small_guns_bonus(0), 
	big_guns_bonus(0), energy_weapons_bonus(0)	{
	small_guns = 2 + prim_stats.agility<<1 + prim_stats.luck>>1;
	big_guns = 2 + prim_stats.endurance<<1 + prim_stats.luck>>1;
	energy_weapons = 2 + prim_stats.perception* + prim_stats.luck>>1;

	weapons.push_back(std::unique_ptr<WeaponBase>(new AssaultRifle(small_guns, typeid(*this).name())));
}

bool Marine::action(const vector<vector<char>>& map_index) {
	if (der_stats.health == 0){
		cout << "MARINE DEAD" << endl;
		return false;
	}
	return true;
}

Pyro::Pyro(double sx, double yx): 
	Being(sx,yx), explosives_bonus(0),
	big_guns_bonus(0), fire_bonus(0)	{
	explosives = 2 + prim_stats.perception<<1 + prim_stats.luck>>1;
	big_guns = 2 + prim_stats.endurance<<1 + prim_stats.luck>>1;
	fire = 2 + prim_stats.agility* + prim_stats.luck>>1;

	weapons.push_back(std::unique_ptr<Molotov>(new Molotov(explosives, typeid(*this).name())));
}

bool Pyro::action(const vector<vector<char>>& map_index) {
	return true;
}

Psychokinetic::Psychokinetic(double sx, double yx): 
	Being(sx,yx), mind_infiltration_bonus(0), 
	mental_power_bonus(0), fire_bonus(0)	{
	mind_infiltration = 2 + prim_stats.intelligence<<1 + prim_stats.luck>>1;
	mental_power = 2 + prim_stats.endurance + prim_stats.intelligence + prim_stats.luck>>1;
	fire = 2 + prim_stats.agility* + prim_stats.luck>>1;

	weapons.push_back(std::unique_ptr<WeaponBase>(new Pyrokinesis(fire, typeid(*this).name())));
}

bool Psychokinetic::action(const vector<vector<char>>& map_index) {
	return true;
}

Android::Android(double sx, double yx): 
	Being(sx,yx), punch_bonus(0), 
	big_guns_bonus(0), energy_weapons_bonus(0)	{
	punch = 2 + prim_stats.strength/2 + prim_stats.luck>>1;
	big_guns = 2 + prim_stats.endurance<<1 + prim_stats.luck>>1;
	energy_weapons = 2 + prim_stats.perception* + prim_stats.luck>>1;

	weapons.push_back(std::unique_ptr<WeaponBase>(new Punch(punch, typeid(*this).name())));
}

bool Android::action(const vector<vector<char>>& map_index) {
	if (der_stats.health == 0){
		cout << "DROID DEAD" << endl;
		return false;
	}
	return true;
}

Zombie::Zombie(double sx, double yx): 
	Being(sx,yx), target(nullptr) {
	biting = 2 + prim_stats.strength<<1 + prim_stats.luck>>1;

	weapons.push_back(std::unique_ptr<WeaponBase>(new Bite(biting, typeid(*this).name())));
}

bool Zombie::action(const vector<vector<char>>& map_index) {

	if(der_stats.health == 0) {
		cout << "ZOMBIE DEAD" << endl;
		return false;
	}

	//if(target == nullptr) target = targets.at(rnd()%targets.size());
	//double tx = target->getX();
	//double ty = target->getY();
	//extern int ysize;
	//extern int xsize;

	//int myx = getTileX(tiles_x);
	//int myy = getTileY(tiles_y);

	//bool there=true;

	//if (x < tx) {
	//	if (!map_index[myx + 1][myy])move(RIGHT, false);
	//	if (map_index[myx + 1][myy])move(UP, true);
	//	there = false;
	//}
	//if(y < ty) {
	//	if (!map_index[myx][myy + 1])move(DOWN, false);
	//	if (map_index[myx][myy + 1])move(RIGHT, true);
	//	there = false;
	//}
	//if(x > tx) {
	//	if (!map_index[myx - 1][myy])move(LEFT, false);
	//	if (map_index[myx - 1][myy])move(DOWN, true);
	//	there = false;
	//}
	//if(y > ty) {
	//	if (!map_index[myx][myy - 1])move(UP, false);
	//	if (map_index[myx][myy - 1])move(LEFT, true);
	//	there = false;
	//}

	//if(there) {
	double curr_x = getX();
	double curr_y = getY();
	int colpos = rnd() % 16;
	//move(colpos, false);
	int state = checkCollisions(curr_x, curr_y, map_index);
	//	orientation = LEFT;
	//	shootWeapon();
	//}
	return true;
	
}

const int BeingResources::getTextureID(const char* ti) {
	return textures[ti];
}

void BeingResources::addTextureID(int newID, const char* ti) {
	textures[ti] = newID;
}