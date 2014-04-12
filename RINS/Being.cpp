#include "Being.h"
//0.015625 = 1/64 (one being = one screen square; //what? one screen has 16 squares; one step is 1/4 square

using namespace std;
#include <iostream>

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

double Hitbox::getX(){
	return x;
}

double Hitbox::getY(){
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

Projectile& Being::shootWeapon(double deg, Hitbox& h) {
	return (weapons.at(curr_weapon)->shoot(deg, x, y, h));
}

int Being::tryToShoot(Being* target, Projectile** p){
	if (target){
		double dx = target->x - x;
		double dy = y - target->y;
		double deg = (atan2(dx, dy) - 0.5*M_PI);
		int di2 = orientation;
		int tx = 0, ty = 0;
		if (false);
		else if (di2 & LEFT)tx = -1;
		else if (di2 & RIGHT)tx = 1;
		else if (di2 & UP)ty = 1;
		else if (di2 & DOWN)ty = -1;
		if (dx*tx >= 0 && dy*ty >= 0){
			*p = &(shootWeapon(deg, *new Hitbox(tiles_x, tiles_y, tile_granularity)));
			return BANG;

		}
		else return NOT_IN_FOV;
	}
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


Being::~Being() {
	weapons.clear();
}

Marine::Marine(double sx, double yx): 
	Being(sx,yx), small_guns_bonus(0), 
	big_guns_bonus(0), energy_weapons_bonus(0)	{
	small_guns = 2 + prim_stats.agility<<1 + prim_stats.luck>>1;
	big_guns = 2 + prim_stats.endurance<<1 + prim_stats.luck>>1;
	energy_weapons = 2 + prim_stats.perception* + prim_stats.luck>>1;

	weapons.push_back(std::unique_ptr<WeaponBase>(new AssaultRifle(small_guns, typeid(*this).name())));
}

bool Marine::action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) {
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

bool Pyro::action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) {
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

bool Psychokinetic::action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) {
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

bool Android::action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) {
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

bool Zombie::action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) {

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
	move(colpos, false);
	int state = checkCollisions(curr_x, curr_y, map_index);
	//	orientation = LEFT;
	double deg = rnd() % 360;

	projectiles.push_back(shootWeapon(deg_to_rad(deg), *new Hitbox(tiles_x, tiles_y, tile_granularity)));
	//}
	return true;
	
}

const int BeingResources::getTextureID(const char* ti) {
	return textures[ti];
}

void BeingResources::addTextureID(int newID, const char* ti) {
	textures[ti] = newID;
}