#include "Item.h"
#include "Being.h"
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
	max_health = 90 + prim.endurance*2 + 10*level;
	health = max_health;
	melee_dmg = prim.strength/2;
	dmg_res = prim.agility*1.5;
}

Specific::Specific(): small_guns(0), big_guns(0), energy_weapons(0), 
		explosives(0), fire(0),  mind_infiltration(0), mental_power(0),
		punch(0) {}

map<const type_info*, int> ItemResources::textures;

void ItemResources::addTextureID(int tid, const type_info* item) {
	cout << item->name() << endl;
	textures[item] = tid;
}

int ItemResources::getTextureID(const type_info* item) {
	return textures.at(item);
}

Item::Item(string iname, int iprice): prim(Primary()), der(Derived(prim, 0)), spec(Specific()), name(iname), price(iprice), equipped(false) {}

bool Item::isEquipped() {
	return equipped;
}
void Item::setEquipped(bool eq) {
	equipped = eq;
}

string Item::getName() {
	return name;
}

Primary& Item::getPrimaryBonuses() {
	return prim;
}

Derived& Item::getDerivedBonuses() {
	return der;
}

Specific& Item::getSpecificBonuses() {
	return spec;
}

bool Item::checkClass(const type_info* cl) {
	for(int i=0;i<classes.size();++i) {
		if(classes.at(i) == cl) return true;
	}
	return false;
}

int Item::getPrice() {
	return price;
}

BodyArmour::BodyArmour(): Item("Body Armour", 200) {
	classes.push_back(&typeid(Marine));
	classes.push_back(&typeid(Pyro));
	classes.push_back(&typeid(Android));
	classes.push_back(&typeid(Psychokinetic));
	der.dmg_res_bonus = 5;
}

Scope::Scope(): Item("Scope", 450) {
	classes.push_back(&typeid(Marine));
	classes.push_back(&typeid(Pyro));
	classes.push_back(&typeid(Android));
	prim.perception_bonus = 4;
}

PsychoAmp::PsychoAmp(): Item("Psycho Amp", 600) {
	classes.push_back(&typeid(Psychokinetic));
	spec.mental_power = 4;
}



