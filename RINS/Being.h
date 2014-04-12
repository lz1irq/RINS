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
enum Collisions{ STATUS_OK, OUT_OF_BOUNDS, TRIGGER, X_COLLIDE, Y_COLLIDE, XY_COLLIDE };
enum Shoot{BANG, NOT_IN_FOV};

class Hitbox{
protected:
	double x, y;
	const double move_step_x;
	const double move_step_y;
	const int tiles_x;
	const int tiles_y;
	const int tile_granularity;
public:
	Hitbox(int tiles_x, int tiles_y, int tile_granularity);
	void setX(double x);
	void setY(double y);
	double getX();
	double getY();
	int getTileX();
	int getTileY();
	double getStepX() const;
	double getStepY() const;
	int checkCollisions(double comp_to_x, double comp_to_y, const vector<vector<char>>& index);
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
	Projectile& shootWeapon(double deg, Hitbox& h);

public:
	Being(double x, double y);
	int getHealth();
	virtual bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) = 0;
	void addWeapon(WeaponBase* wpn);
	void move(int dir, bool reverse);
	int getLevel();
	bool getWalk();
	void resetWalk();
	int getOrientation() const;
	void takeProjectile(Projectile& bullet);
	int tryToShoot(Being* target, Projectile** p);

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
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
	int getTextureID();
};

class Pyro:public Being, BeingResources {
private:
	int explosives, explosives_bonus;
	int big_guns, big_guns_bonus;
	int fire, fire_bonus;
public:
	Pyro(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
};

class Psychokinetic: public Being, BeingResources {
private:
	int mind_infiltration, mind_infiltration_bonus;
	int mental_power, mental_power_bonus;
	int fire, fire_bonus;
public:
	Psychokinetic(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
};

class Zombie: public Being, BeingResources {
private:
	int biting;
	Being* target;
public:
	Zombie(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
};

class Android: public Being, BeingResources {
private:
	int punch, punch_bonus;
	int big_guns, big_guns_bonus;
	int energy_weapons, energy_weapons_bonus;
public:
	Android(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
};

#endif