#include "Weapon.h"
using namespace std;
#include <iostream>
Projectile::Projectile(unsigned int ptype, int pdmg, int pfly_t, int pdet_t, unsigned int pdir, double px, double py): 
type(ptype), dmg(pdmg),
fly_t(pfly_t), det_t(pdet_t),
dir(pdir), x(px),
y(py)
{}

unsigned int Projectile::getType() const {
	return type;
}

void Projectile::move() {
	if(dir == LEFT)x-=1.0/64.0;
	else if(dir == RIGHT)x+=1.0/64.0;
	else if(dir == UP)y-=1.0/64.0;
	else if(dir == DOWN)y+=1.0/64.0;
}

double Projectile::getX() {
	return x;
}

double Projectile::getY() {
	return y;
}

int Projectile::getDamage() const {
	return dmg;
}

int Projectile::getFlyTime() const {
	return fly_t;
}

int Projectile::getDetonationTime() const {
	return det_t;
}

WeaponBase::WeaponBase(int wtype, int wskill, int wbase_dmg,  int ammo_mag): 
type(wtype), skill_points(wskill),
base_dmg(wbase_dmg), ammo_per_mag(ammo_mag) {
	dmg = skill_points + base_dmg;
}

void WeaponBase::updateSkillPoints(int uskill) {
	skill_points = uskill;
	dmg = skill_points;
}

int WeaponBase::getDamage() const {
	return dmg;
}

int WeaponBase::getAmmoPerMag() const {
	return ammo_per_mag;
}

bool WeaponBase::isPickedUp() const {
	return picked_up;
}

void WeaponBase::pickUp() {
	picked_up = true;
}

AssaultRifle::AssaultRifle(int wskill): WeaponBase(BULLET,wskill,15,30) {
}

Projectile* AssaultRifle::shoot(int dir, double px, double py) {
	cout << "STRELQM" << endl;
	return new Projectile(type, dmg, 80, 100, dir, px, py);
}

Pyrokinesis::Pyrokinesis(int wskill): WeaponBase(FIRE,wskill,15,30) {
}

Projectile* Pyrokinesis::shoot(int dir, double px, double py) {
	return new Projectile(type, dmg, 80, 100, dir, px, py);
}

Molotov::Molotov(int wskill): WeaponBase(FIRE,wskill,15,30) {
}

Projectile* Molotov::shoot(int dir, double px, double py) {
	return new Projectile(type, dmg, 80, 100, dir, px, py);
}

Bite::Bite(int wskill): WeaponBase(BULLET,wskill,40,30) {
}

Projectile* Bite::shoot(int dir, double px, double py) {
	return new Projectile(type, dmg, 1, 0, dir, px, py);
}

map<unsigned int, int> Projectile::textures;

void Projectile::addTexture(unsigned int i, int tid ) {
	textures[i] = tid;
}
int Projectile::getTexture(int i) {
	return textures[i];
}

map<const char*, int> WeaponResources::textures;

int WeaponResources::getTextureID(const char* ti) {
	return textures[ti];
}

void WeaponResources::addTextureID(int newID, const char* ti) {
	textures[ti] = newID;
}