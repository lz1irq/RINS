#ifndef _GLIBCXX_BEING_H
#define _GLIBCXX_BEING_H
#include "Weapon.h"
#include "Item.h"
#include <ctime>
#include <random>
#include <array>
#include <typeinfo>
#include <algorithm>
using namespace std;

struct IDs {
	int ids[2];
	IDs();
};


class BeingResources {
	static map<const type_info*, IDs> textures;
public:
	static const int getTextureID(const type_info* ti);
	static void addTextureID(int newID, const type_info*);
	static const int getSoundID(const type_info* si);
	static void addSoundID(int newID, const type_info* si);
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
	int range;
	Primary prim_stats;
	Derived der_stats;
	int curr_weapon;
	std::vector<std::unique_ptr<WeaponBase>> weapons;
	static mt19937 rnd;
	bool walk = false;
	virtual void setRange() = 0;
	Projectile& shootWeapon(double deg, Hitbox& h);
	int HAJA = 0;

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
	void equipItem(Item& item);
	void unequipItem(Item& item);
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
	void setRange() final;
};

class Pyro:public Being, BeingResources {
private:
	int explosives, explosives_bonus;
	int big_guns, big_guns_bonus;
	int fire, fire_bonus;
public:
	Pyro(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
	void setRange() final;
};

class Psychokinetic: public Being, BeingResources {
private:
	int mind_infiltration, mind_infiltration_bonus;
	int mental_power, mental_power_bonus;
	int fire, fire_bonus;
public:
	Psychokinetic(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
	void setRange() final;
};

class Zombie: public Being, BeingResources {
private:
	int speed = 20;
	int count = 0;
	int biting;
	Being* target;
public:
	Zombie(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
	void setRange() final;
};

class Android: public Being, BeingResources {
private:
	int punch, punch_bonus;
	int big_guns, big_guns_bonus;
	int energy_weapons, energy_weapons_bonus;
public:
	Android(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
	void setRange() final;
};

#endif