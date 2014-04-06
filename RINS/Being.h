#ifndef _GLIBCXX_BEING_H
#define _GLIBCXX_BEING_H
#include "Weapon.h"
#include <ctime>
#include <random>
#include <array>
#include <typeinfo>
using namespace std;
struct Primary {
	int strength, strength_bonus;
	int perception, perception_bonus;
	int endurance, endurance_bonus;
	int intelligence, intelligence_bonus;
	int agility, agility_bonus;
	int luck, luck_bonus;
	Primary();
	};

struct Derived {
	double crit_chance, crit_bonus;
	int dmg_res, dmg_res_bonus;
	int health;
	int melee_dmg, melee_dmg_bonus;
	int fire_res, fire_res_bonus;
	Derived(Primary prim, int level);
	};



class BeingResources {
	static map<const char*, int> textures;
public:
	static const int getTextureID(const char* ti);
	static void addTextureID(int newID, const char* ti);
};

enum Monsters{ ZOMBIE = 0, MAXSIZE };
enum Collisions{ STATUS_OK, OUT_OF_BOUNDS, TRIGGER, X_COLIDE, Y_COLIDE, XY_COLIDE };

class Hitbox{
protected:
	double x, y;
	const double move_step_x;
	const double move_step_y;
	const int tiles_x;
	const int tiles_y;
public:
	Hitbox(int tiles_x, int tiles_y, int tile_granularity) : tiles_x(tiles_x), tiles_y(tiles_y), 
		move_step_x(1.0 / (tile_granularity * tiles_x)), move_step_y(1.0 / (tile_granularity * tiles_y)){}
	void setX(double x){
		this->x = x;
	}
	void setY(double y){
		this->y = y;
	}
	double getX(){
		return x;
	}
	double getY(){
		return y;
	}
	int getTileX(){
		return ((x + move_step_x) / move_step_x) / ((1.0 / tiles_x) / move_step_x);
	}
	int getTileY(){
		return ((y + move_step_y * 3) / move_step_y) / ((1.0 / tiles_y) / move_step_y);
	}
	double getStepX() const{
		return move_step_x;
	}
	double getStepY() const{
		return move_step_y;
	}
	int checkCollisions(double comp_to_x, double comp_to_y, const vector<vector<char>>& index){
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
};

class Being: public Hitbox{
protected:
	int level;
	int orientation;
	Primary prim_stats;
	Derived der_stats;
	int curr_weapon;
	std::vector<std::unique_ptr<WeaponBase>> weapons;
	static mt19937 rnd;
	bool walk = false;

public:
	Being(double x, double y);
	int getHealth();
	virtual bool action(const vector<vector<char>>& map_index) = 0;
	void addWeapon(WeaponBase* wpn);
	void move(int dir, bool reverse);
	int getLevel();
	bool getWalk();
	void resetWalk();
	int getOrientation() const;
	void takeProjectile(Projectile& bullet);
	void shootWeapon();
	void nextWeapon();
	void prevWeapon();

	static array<Being*(*)(double, double), MAXSIZE> monsters;
	static vector<Being*> targets;
	static list<Projectile> projectiles;
	static Hitbox* box;

	virtual ~Being();
};

template<typename T> Being * createInstance(double x, double y) { return new T(x, y); }

class Marine: public Being, BeingResources {
private:
	int small_guns, small_guns_bonus;
	int big_guns, big_guns_bonus;
	int energy_weapons, energy_weapons_bonus;
public:
	Marine(double sx, double sy);
	bool action(const vector<vector<char>>& map_index) final;
	int getTextureID();
};

class Pyro:public Being, BeingResources {
private:
	int explosives, explosives_bonus;
	int big_guns, big_guns_bonus;
	int fire, fire_bonus;
public:
	Pyro(double sx, double sy);
	bool action(const vector<vector<char>>& map_index) final;
};

class Psychokinetic: public Being, BeingResources {
private:
	int mind_infiltration, mind_infiltration_bonus;
	int mental_power, mental_power_bonus;
	int fire, fire_bonus;
public:
	Psychokinetic(double sx, double sy);
	bool action(const vector<vector<char>>& map_index) final;
};

class Zombie: public Being, BeingResources {
private:
	int biting;
	Being* target;
public:
	Zombie(double sx, double sy);
	bool action(const vector<vector<char>>& map_index) final;
};

class Android: public Being, BeingResources {
private:
	int punch, punch_bonus;
	int big_guns, big_guns_bonus;
	int energy_weapons, energy_weapons_bonus;
public:
	Android(double sx, double sy);
	bool action(const vector<vector<char>>& map_index) final;
};

#endif