#ifndef _GLIBCXX_BEING_H
#define _GLIBCXX_BEING_H

#include "Weapon.h"

struct primary {
	int strength, strength_bonus;
	int perception, perception_bonus;
	int endurance, endurance_bonus;
	int intelligence, intelligence_bonus;
	int agility, agility_bonus;
	int luck, luck_bonus;
	primary();
	};

struct derived {
	double crit_chance, crit_bonus;
	int dmg_res, dmg_res_bonus;
	int health, health_bonus;
	int melee_dmg, melee_dmg_bonus;
	derived(primary prim, int level);
	};

class BeingResources {
	int id, x, y;
public:
	BeingResources(int tid);
};

class Being{
protected:
	int level;
	double x,y;
	const double move_step;
	int orientation;
	enum {LEFT=1,RIGHT=2,UP=4,DOWN=8};
	primary prim_stats;
	derived der_stats;
	Weapon** weapons;

public:
	Being(double x, double y);
	virtual void action() = 0;
	void move(int dir);
	double getX();
	double getY();
	int getOrientation();
	double getStep();
	~Being();
};

class Marine: public Being, BeingResources {
private:
	int small_guns, small_guns_bonus;
	int big_guns, big_guns_bonus;
	int energy_weapons, energy_weapons_bonus;
public:
	Marine(double sx, double sy, int tid);
};

class Pyro:public Being, BeingResources {
private:
	int explosives, explosives_bonus;
	int big_guns, big_guns_bonus;
	int fire, fire_bonus;
public:
	Pyro(double sx, double sy, int tid);
};

class Psychokinetic: public Being, BeingResources {
private:
	int mind_infiltration, mind_infiltration_bonus;
	int mental_power, mental_power_bonus;
	int fire, fire_bonus;
public:
	Psychokinetic(double sx, double sy, int tid);
};

#endif