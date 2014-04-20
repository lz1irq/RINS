#include "Weapon.h"
#include "Being.h"

double deg_to_rad(double deg){
	return deg * M_PI / 180.0;
}

double rad_to_deg(double rad){
	return rad * 180.0 / M_PI;
}

//Hitbox* Projectile::box;
#include <iostream>
using namespace std;
Projectile::Projectile(unsigned int ptype, int pdmg, int pfly_t, int pdet_t, double angle, double px, double py, Being* shooter, Hitbox& h) :
type(ptype), dmg(pdmg), box(h),
fly_t(pfly_t), det_t(pdet_t), sh(typeid(*shooter)),
dir(angle), x(px), y(py), shooter(shooter)
{}

unsigned int Projectile::getType() const {
	return type;
}

double Projectile::getAngleInDeg(){
	return rad_to_deg(dir);
}

bool Projectile::update(const vector<vector<char>>& map_index, list<unique_ptr<Being>>& targets, list<unique_ptr<Being>>& players){
	box.setX(x + cos(dir) * box.getStepX());
	box.setY(y + sin(dir) * box.getStepY());
	if (fly_t){
		--fly_t;
		int event = box.checkCollisions(x, y, map_index);
		if (event == STATUS_OK || event == TRIGGER){
			x = box.getX();
			y = box.getY();
		}
	}
	if (det_t)--det_t;
	else return false; //explode!
	for (auto m = begin(targets); m != end(targets); ++m){
		int mx = ((*m)->getX()+(*m)->getStepX()*1.5) / (*m)->getStepX();
		int my = ((*m)->getY() + (*m)->getStepY()*1.5) / (*m)->getStepY();
		int px = (box.getX() + (*m)->getStepX()*1.5) / box.getStepX();
		int py = (box.getY() + (*m)->getStepY()*1.5) / box.getStepY();
		if ((px - mx) <= 1 && (px - mx) >= 0 && (py - my) <= 1 && (py - my) >= 0){
			if (typeid(*(*m)) != sh){
				(*m)->takeProjectile(*this);
				return false;//explode!
			}
		}
	}
	for (auto m = begin(players); m != end(players); ++m){
		int mx = ((*m)->getX() + (*m)->getStepX()*1.5) / (*m)->getStepX();
		int my = ((*m)->getY() + (*m)->getStepY()*1.5) / (*m)->getStepY();
		int px = (box.getX() + (*m)->getStepX()*1.5) / box.getStepX();
		int py = (box.getY() + (*m)->getStepY()*1.5) / box.getStepY();
		if ((px - mx) <= 1 && (px - mx) >= 0 && (py - my) <= 1 && (py - my) >= 0){
			if (typeid(*(*m)) != sh){
				(*m)->takeProjectile(*this);
				return false;//explode!
			}
		}
	}
	return true;//miss
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

AssaultRifle::AssaultRifle(int wskill, Being* assoc_class) : WeaponBase(BULLET, wskill, 15, 30) {
	this->assoc_class = assoc_class;
}

Projectile& AssaultRifle::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, 80, 100, angle, px, py, assoc_class, h);
}

Pyrokinesis::Pyrokinesis(int wskill, Being* assoc_class) : WeaponBase(FIRE, wskill, 15, 30) {
	this->assoc_class = assoc_class;
}

Projectile& Pyrokinesis::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, 80, 100, angle, px, py, assoc_class, h);
}

Molotov::Molotov(int wskill, Being* assoc_class) : WeaponBase(FIRE, wskill, 15, 30) {
	this->assoc_class = assoc_class;
}

Projectile& Molotov::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, 80, 100, angle, px, py, assoc_class, h);
}

Punch::Punch(int wskill, Being* assoc_class) : WeaponBase(BULLET, wskill, 15, 30) {
	this->assoc_class = assoc_class;
}

Projectile& Punch::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, 80, 100, angle, px, py, assoc_class, h);
}

Bite::Bite(int wskill, Being* assoc_class) : WeaponBase(BULLET, wskill, 40, 30) {
	this->assoc_class = assoc_class;
}

Projectile& Bite::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, 100, 100, angle, px, py, assoc_class, h);
}


map<int, int> WeaponResources::textures;

int WeaponResources::getTexture(int ti){
	return textures[ti];
}

void WeaponResources::addTexture(int newID, int ti) {
	textures[ti] = newID;
}