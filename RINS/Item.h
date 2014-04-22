#ifndef _GLIBCXX_ITEM_H
#define _GLIBCXX_ITEM_H

#include <iostream>
#include <vector>
#include <map>
#include<typeinfo>
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

class ItemResources {
	static map<const type_info*, int> textures;
public:
	static void addTextureID(int tid, const type_info* item);
	static int getTextureID(const type_info* item);
};

class Item {
protected:
	Primary prim;
	Derived der;
	Specific spec;
	vector<const type_info*> classes;
	string name;
	unsigned int price;
	bool equipped;
public:
	Item(string iname, int iprice);
	Primary& getPrimaryBonuses();
	Derived& getDerivedBonuses();
	Specific& getSpecificBonuses();
	bool checkClass(const type_info* cl);
	string getName();
	int getPrice();
	bool isEquipped();
	void setEquipped(bool eq);
	virtual ~Item() {};
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