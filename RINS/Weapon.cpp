#include "Weapon.h"
#include "Being.h"

double deg_to_rad(double deg){
	return deg * M_PI / 180.0;
}

//Hitbox* Projectile::box;

Projectile::Projectile(unsigned int ptype, int pdmg, int pfly_t, int pdet_t, double angle, double px, double py, const char* shooter, Hitbox& h) :
type(ptype), dmg(pdmg), box(h),
fly_t(pfly_t), det_t(pdet_t),
dir(angle), x(px), y(py), shooter(shooter)
{}

unsigned int Projectile::getType() const {
	return type;
}

//void Projectile::move(double step) {
//	if(dir & LEFT)x-= step;
//	if (dir & RIGHT)x += step;
//	if(dir & UP)y-= step;
//	if(dir & DOWN)y+= step;
//}
#include <iostream>
using namespace std;

bool Projectile::update(const vector<vector<char>>& map_index, list<unique_ptr<Being>>& targets){
	//double newx = x;
	//double newy = y;
	//cout << x << " " << y << endl;
	//if (dir & LEFT)newx -= box->getStepX();
	//if (dir & RIGHT)newx += box->getStepX();
	//if (dir & UP)newy -= box->getStepY();
	//if (dir & DOWN)newy += box->getStepY();
	//newx += cos(deg_to_rad(dir)) * box.getStepX();
	//newy += sin(deg_to_rad(dir)) * box.getStepY();
	box.setX(x + cos(dir) * box.getStepX());
	box.setY(y + sin(dir) * box.getStepY());
	if (fly_t){
		--fly_t;
		int event = box.checkCollisions(x, y, map_index);
		if (event == STATUS_OK || event == TRIGGER){
			x = box.getX();
			y = box.getY();
		}
		//else cout << "cc" << endl;
	}
	if (det_t)--det_t;
	else return false; //explode!

	for (auto m = begin(targets); m != end(targets); ++m){

		int mx = (*m)->getX() / (*m)->getStepX() + 1;
		int my = (*m)->getY() / (*m)->getStepY() + 1;
		int px = box.getX() / box.getStepX() + 1;
		int py = box.getY() / box.getStepY() + 1;

		if ((px - mx) <= 1 && (px - mx) >= 0 && (py - my) <= 1 && (py - my) >= 0){
			if (strcmp(typeid(*m).name(), shooter)){
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

AssaultRifle::AssaultRifle(int wskill, const char* assoc_class) : WeaponBase(BULLET, wskill, 15, 30) {
	this->assoc_class = assoc_class;
}

Projectile& AssaultRifle::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, 80, 100, angle, px, py, assoc_class, h);
}

Pyrokinesis::Pyrokinesis(int wskill, const char* assoc_class) : WeaponBase(FIRE, wskill, 15, 30) {
	this->assoc_class = assoc_class;
}

Projectile& Pyrokinesis::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, 80, 100, angle, px, py, assoc_class, h);
}

Molotov::Molotov(int wskill, const char* assoc_class) : WeaponBase(FIRE, wskill, 15, 30) {
	this->assoc_class = assoc_class;
}

Projectile& Molotov::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, 80, 100, angle, px, py, assoc_class, h);
}

Punch::Punch(int wskill, const char* assoc_class) : WeaponBase(BULLET, wskill, 15, 30) {
	this->assoc_class = assoc_class;
}

Projectile& Punch::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, 80, 100, angle, px, py, assoc_class, h);
}

Bite::Bite(int wskill, const char* assoc_class) : WeaponBase(BULLET, wskill, 40, 30) {
	this->assoc_class = assoc_class;
}

Projectile& Bite::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, 1, 0, angle, px, py, assoc_class, h);
}


map<int, int> WeaponResources::textures;

int WeaponResources::getTexture(int ti){
	return textures[ti];
}

void WeaponResources::addTexture(int newID, int ti) {
	textures[ti] = newID;
}