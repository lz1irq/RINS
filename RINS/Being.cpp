#include "Being.h"
//0.015625 = 1/64 (one being = one screen square; //what? one screen has 16 squares; one step is 1/4 square

using namespace std;
#include <iostream>

vector<Being*> Being::targets;
list<Projectile> Being::projectiles;

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

Being::Being(double x, double y): 
	x(x), y(y),
	orientation(UP), move_step_x(1.0/(4*tiles_x)), 
	move_step_y(1.0 / (4 * tiles_y)),
	level(1), prim_stats(Primary()), 
	der_stats(prim_stats, level), curr_weapon(0) {
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

double Being::getX() const {
	return x;
}

double Being::getY() const {
	return y;
}

void Being::setX(double newX) {
	x = newX;
}

void Being::setY(double newY) {
	y = newY;
}

int Being::getOrientation() const {
	return orientation;
}

double Being::getStepX() const{
	return move_step_x;
}

double Being::getStepY() const{
	return move_step_y;
}

void Being::shootWeapon() {
	projectiles.push_back((weapons.at(curr_weapon)->shoot(orientation, x, y)));
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

int Being::getTileX(int xsize){
	return ((x + move_step_x) / move_step_x) / ((1.0 / xsize) / move_step_x);
}

int Being::getTileY(int ysize){
	return ((y + move_step_y * 3) / move_step_y) / ((1.0 / ysize) / move_step_y);
}

void Being::setNumTiles(int x, int y){
	tiles_x = x;
	tiles_y = y;
}

int Being::checkCollisions(double comp_to_x, double comp_to_y, const vector<vector<char>>& index, int& on_x_tile, int& on_y_tile){
	double curr_x = x;
	double curr_y = y;
	x = comp_to_x;
	y = comp_to_y;
	int last_tile_x = getTileX(tiles_x);
	int last_tile_y = getTileY(tiles_y);
	x = curr_x;
	y = curr_y;
	int curr_tile_x = getTileX(tiles_x);
	int curr_tile_y = getTileY(tiles_y);
	on_x_tile = curr_tile_x;
	on_y_tile = curr_tile_y;
	if (!(curr_tile_x < 0 || curr_tile_x >= index.size())){
		if (!(curr_tile_y < 0 || curr_tile_y >= index[curr_tile_x].size())){
			if (index[curr_tile_x][curr_tile_y]){
				if (index[curr_tile_x][curr_tile_y] < 16){  //16 = #wall combinations; the magic tiles' ID's are > than 16
					if(index[last_tile_x][curr_tile_y] && index[curr_tile_x][last_tile_y]){
						setY(comp_to_y);
						setX(comp_to_x);
						return XY_COLIDE;
					}
					if (index[last_tile_x][curr_tile_y]){
						setY(comp_to_y);
						return Y_COLIDE;
					}
					if (index[curr_tile_x][last_tile_y]){
						setX(comp_to_x);
						return X_COLIDE;
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

int Being::tiles_x;

int Being::tiles_y;

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

void Marine::action(const vector<vector<char>>& map_index) {
	if(der_stats.health == 0) cout << "MARINE DEAD" << endl;
}

Pyro::Pyro(double sx, double yx): 
	Being(sx,yx), explosives_bonus(0),
	big_guns_bonus(0), fire_bonus(0)	{
	explosives = 2 + prim_stats.perception<<1 + prim_stats.luck>>1;
	big_guns = 2 + prim_stats.endurance<<1 + prim_stats.luck>>1;
	fire = 2 + prim_stats.agility* + prim_stats.luck>>1;

	weapons.push_back(std::unique_ptr<Molotov>(new Molotov(explosives, typeid(*this).name())));
}

void Pyro::action(const vector<vector<char>>& map_index) {
	return;
}

Psychokinetic::Psychokinetic(double sx, double yx): 
	Being(sx,yx), mind_infiltration_bonus(0), 
	mental_power_bonus(0), fire_bonus(0)	{
	mind_infiltration = 2 + prim_stats.intelligence<<1 + prim_stats.luck>>1;
	mental_power = 2 + prim_stats.endurance + prim_stats.intelligence + prim_stats.luck>>1;
	fire = 2 + prim_stats.agility* + prim_stats.luck>>1;

	weapons.push_back(std::unique_ptr<WeaponBase>(new Pyrokinesis(fire, typeid(*this).name())));
}

void Psychokinetic::action(const vector<vector<char>>& map_index) {
	return;
}

Android::Android(double sx, double yx): 
	Being(sx,yx), punch_bonus(0), 
	big_guns_bonus(0), energy_weapons_bonus(0)	{
	punch = 2 + prim_stats.strength/2 + prim_stats.luck>>1;
	big_guns = 2 + prim_stats.endurance<<1 + prim_stats.luck>>1;
	energy_weapons = 2 + prim_stats.perception* + prim_stats.luck>>1;

	weapons.push_back(std::unique_ptr<WeaponBase>(new Punch(punch, typeid(*this).name())));
}

void Android::action(const vector<vector<char>>& map_index) {
	if(der_stats.health == 0) cout << "DROID DEAD" << endl;
}

Zombie::Zombie(double sx, double yx): 
	Being(sx,yx), target(nullptr) {
	biting = 2 + prim_stats.strength<<1 + prim_stats.luck>>1;

	weapons.push_back(std::unique_ptr<WeaponBase>(new Bite(biting, typeid(*this).name())));
}

void Zombie::action(const vector<vector<char>>& map_index) {

	//if(der_stats.health == 0) {
	//	cout << "ZOMBIE DEAD" << endl;
	//}

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
	//	move(RIGHT, false);
	//	orientation = LEFT;
	//	shootWeapon();
	//}
	
}

const int BeingResources::getTextureID(const char* ti) {
	return textures[ti];
}

void BeingResources::addTextureID(int newID, const char* ti) {
	textures[ti] = newID;
}