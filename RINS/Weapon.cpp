#include "Weapon.h"

Projectile::Projectile(unsigned int ptype, int pdmg, int pfly_t, int pdet_t, unsigned int pdir): 
type(ptype), dmg(pdmg),
fly_t(pfly_t), det_t(pdet_t),
dir(pdir)
{}

unsigned int Projectile::getType() const {
	return type;
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

AssaultRifle::AssaultRifle(int wskill): WeaponBase(BULLET,wskill,15,30), WeaponResources<AssaultRifle>() {
}

Projectile AssaultRifle::shoot(int dir) {
	return Projectile(type, dmg, 80, 100, dir);
}

Pyrokinesis::Pyrokinesis(int wskill): WeaponBase(FIRE,wskill,15,30), WeaponResources<Pyrokinesis>() {
}

Projectile Pyrokinesis::shoot(int dir) {
	return Projectile(type, dmg, 80, 100, dir);
}

Molotov::Molotov(int wskill): WeaponBase(FIRE,wskill,15,30), WeaponResources<Molotov>() {
}

Projectile Molotov::shoot(int dir) {
	return Projectile(type, dmg, 80, 100, dir);
}

Bite::Bite(int wskill): WeaponBase(BULLET,wskill,15,30), WeaponResources<Bite>() {
}

Projectile Bite::shoot(int dir) {
	return Projectile(type, dmg, 1, 0, dir);
}

