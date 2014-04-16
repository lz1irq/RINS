#ifndef _GLIBCXX_ITEM_H
#define _GLIBCXX_ITEM_H

#include <iostream>
#include <vector>
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

struct Specific {
	int small_guns, big_guns, energy_weapons;
	int explosives, fire;
	int mind_infiltration, mental_power;
	int punch;
	Specific(); 
};

class Marine;
class Psychokinetic;
class Pyro;
class Android;

enum Items{ BODYARMOR = 0, SCOPE, PSYCHOAMP, MAXITEMS };

class Item {
protected:
	Primary prim;
	Derived der;
	Specific spec;
	vector<string> classes;
	string name;
public:
	Item(string iname);
	Primary& getPrimaryBonuses();
	Derived& getDerivedBonuses();
	Specific& getSpecificBonuses();
	bool checkClass(std::string cl);
};

template<typename T> Item * createItem() { return new T(); }

class BodyArmour: public Item {
public:
	BodyArmour();
};

class Scope: public Item {
public:
	Scope();
};

class PsychoAmp: public Item {
public:
	PsychoAmp();
};


#endif