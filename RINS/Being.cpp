#include "Being.h"
//0.015625 = 1/64 (one being = one screen square;

primary::primary():	
	strength(5), strength_bonus(0),
	perception(5), perception_bonus(0),
	endurance(5), endurance_bonus(0),
	intelligence(5), intelligence_bonus(0),
	agility(5), agility_bonus(0),
	luck(5), luck_bonus(0) {}

derived::derived(primary prim, int level): crit_bonus(0), dmg_res(0), dmg_res_bonus(0), health_bonus(0), melee_dmg(0) {
	crit_chance = prim.luck * 0.01;
	health = 90 + prim.endurance<<10 + 10*level;
	melee_dmg = prim.strength >> 2;
}

BeingResources::BeingResources(int tid): id(tid), x(0), y(0) {}

Being::Being(double x, double y): 
	x(x), y(y),
	orientation(UP), move_step(1.0/64), 
	level(1), prim_stats(primary()), 
	der_stats(prim_stats, level) {}

void Being::move(int dir) {
	orientation = dir;
	if (dir & LEFT) x-= move_step;
	if (dir & RIGHT) x += move_step;
	if (dir & UP) y -= move_step;
	if (dir & DOWN) y += move_step;
}

double Being::getX() {
	return x;
}

double Being::getY() {
	return y;
}

int Being::getOrientation() {
	return orientation;
}

double Being::getStep(){
	return move_step;
}

void Being::action() {
	return;
}

Being::~Being() {
	delete [] weapons;
}

Marine::Marine(double sx, double yx, int tid): 
	Being(sx,yx), small_guns_bonus(0), 
	big_guns_bonus(0), energy_weapons_bonus(0),
	BeingResources(BeingResources(tid))			{
	small_guns = 2 + prim_stats.agility<<1 + prim_stats.luck>>1;
	big_guns = 2 + prim_stats.endurance<<1 + prim_stats.luck>>1;
	energy_weapons = 2 + prim_stats.perception* + prim_stats.luck>>1;
}

Pyro::Pyro(double sx, double yx, int tid): 
	Being(sx,yx), explosives_bonus(0),
	big_guns_bonus(0), fire_bonus(0),
	BeingResources(BeingResources(tid))			{
	explosives = 2 + prim_stats.perception<<1 + prim_stats.luck>>1;
	big_guns = 2 + prim_stats.endurance<<1 + prim_stats.luck>>1;
	fire = 2 + prim_stats.agility* + prim_stats.luck>>1;
}

Psychokinetic::Psychokinetic(double sx, double yx, int tid): 
	Being(sx,yx), mind_infiltration_bonus(0), 
	mental_power_bonus(0), fire_bonus(0),
	BeingResources(BeingResources(tid))			{
	mind_infiltration = 2 + prim_stats.intelligence<<1 + prim_stats.luck>>1;
	mental_power = 2 + prim_stats.endurance + prim_stats.intelligence + prim_stats.luck>>1;
	fire = 2 + prim_stats.agility* + prim_stats.luck>>1;
}