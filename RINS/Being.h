#ifndef _GLIBCXX_BEING_H
#define _GLIBCXX_BEING_H
#include "Weapon.h"
#include <vector>
#include <memory>
#include <ctime>
#include <random>
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



template<class T> class BeingResources {
	static int texture_ID;
	int texture_x, texture_y;
public:
	int getTextureID() const;
	static void setTextureID(int newID);

	int getTextureX() const;
	void setTextureX(int newx);

	int getTextureY() const;
	void setTextureY(int newy);
	
};

class Being{
protected:
	int level;
	double x,y;
	const double move_step;
	int orientation;
	enum {LEFT=1,RIGHT=2,UP=4,DOWN=8};
	Primary prim_stats;
	Derived der_stats;
	int curr_weapon;
	std::vector<std::unique_ptr<WeaponBase>> weapons;
	static mt19937 rnd;

public:
	Being(double x, double y);
	int getHealth();
	virtual void action() = 0;
	void addWeapon(WeaponBase* wpn);
	void move(int dir);
	double getX() const;
	double getY() const;
	int getOrientation() const;
	double getStep() const;
	void takeProjectile(Projectile bullet);
	void shootWeapon(unsigned int dir);
	void nextWeapon();
	void prevWeapon();
	~Being();
};

class Marine: public Being, BeingResources<Marine> {
private:
	int small_guns, small_guns_bonus;
	int big_guns, big_guns_bonus;
	int energy_weapons, energy_weapons_bonus;
public:
	Marine(double sx, double sy);
	void action();
};

class Pyro:public Being, BeingResources<Pyro> {
private:
	int explosives, explosives_bonus;
	int big_guns, big_guns_bonus;
	int fire, fire_bonus;
public:
	Pyro(double sx, double sy);
	void action();
};

class Psychokinetic: public Being, BeingResources<Psychokinetic> {
private:
	int mind_infiltration, mind_infiltration_bonus;
	int mental_power, mental_power_bonus;
	int fire, fire_bonus;
public:
	Psychokinetic(double sx, double sy);
	void action();
};

class Zombie: public Being, BeingResources<Zombie> {
private:
	int biting;
	Being* target;
public:
	Zombie(double sx, double sy);
	void action();
};

#endif