#ifndef _GLIBCXX_BEING_H
#define _GLIBCXX_BEING_H
#include "Weapon.h"
#include <vector>
#include <memory>
#include <ctime>
#include <random>
#include <array>
#include <list>
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
	static int getTextureID(const char* ti);
	static void addTextureID(int newID, const char* ti);
};

enum Monsters{ ZOMBIE = 0, MAXSIZE};

class Being{
protected:
	int level;
	double x,y;
	const double move_step_x;
	const double move_step_y;
	int orientation;
	Primary prim_stats;
	Derived der_stats;
	int curr_weapon;
	std::vector<std::unique_ptr<WeaponBase>> weapons;
	static mt19937 rnd;
	static int tiles_x;
	static int tiles_y;

public:
	Being(double x, double y);
	int getHealth();
	virtual void action(const vector<vector<char>>& map_index) = 0;
	void addWeapon(WeaponBase* wpn);
	void move(int dir, bool reverse);

	double getX() const;
	double getY() const;
	void setX(double newX);
	void setY(double newY);
	int getTileX(int xsize);
	int getTileY( int ysize);

	int getOrientation() const;
	double getStepX() const;
	double getStepY() const;
	void takeProjectile(Projectile& bullet);
	static void setNumTiles(int x, int y);

	void shootWeapon();
	void nextWeapon();
	void prevWeapon();

	static array<Being*(*)(double, double), MAXSIZE> monsters;
	static vector<Being*> targets;
	static list<Projectile> projectiles;

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
	void action(const vector<vector<char>>& map_index) final;
	int getTextureID();
};

class Pyro:public Being, BeingResources {
private:
	int explosives, explosives_bonus;
	int big_guns, big_guns_bonus;
	int fire, fire_bonus;
public:
	Pyro(double sx, double sy);
	void action(const vector<vector<char>>& map_index) final;
	int getTextureID();
};

class Psychokinetic: public Being, BeingResources {
private:
	int mind_infiltration, mind_infiltration_bonus;
	int mental_power, mental_power_bonus;
	int fire, fire_bonus;
public:
	Psychokinetic(double sx, double sy);
	void action(const vector<vector<char>>& map_index) final;
	int getTextureID();
};

class Zombie: public Being, BeingResources {
private:
	int biting;
	Being* target;
public:
	Zombie(double sx, double sy);
	void action(const vector<vector<char>>& map_index) final;
	int getTextureID();
};

class Android: public Being, BeingResources {
private:
	int punch, punch_bonus;
	int big_guns, big_guns_bonus;
	int energy_weapons, energy_weapons_bonus;
public:
	Android(double sx, double sy);
	void action(const vector<vector<char>>& map_index) final;
	int getTextureID();
};

#endif