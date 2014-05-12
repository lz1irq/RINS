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
Projectile::Projectile(unsigned int ptype, int pdmg, int pfly_t, int pdet_t, double angle, double px, double py, Being* shooter, Hitbox& h, int  wait_on_det, int range, int det_duration, int speed) :
type(ptype), dmg(pdmg), box(h), wait_on_det(wait_on_det), range(range),
fly_t(pfly_t), det_t(pdet_t), sh(typeid(*shooter)), det_duration(det_duration),
dir(angle), x(px), y(py), shooter(shooter), speed(speed)
{}

unsigned int Projectile::getType() const {
	return type;
}

double Projectile::getAngleInDeg(){
	return rad_to_deg(dir);
}

const Being* Projectile::getShooter(){
	return shooter;
}

bool Projectile::update(const vector<vector<char>>& map_index, list<unique_ptr<Being>>& targets, list<unique_ptr<Being>>& players, unsigned int time){
	if (last_time + speed < time){
		rflag = true;
		last_time = time;
		box.setX(x + cos(dir) * box.getStepX());
		box.setY(y + sin(dir) * box.getStepY());
		if (trigger){
			if (!det_duration)return false;
			--det_duration;
		}
		if (det_t)--det_t;
		else{
			trigger = true;
			fly_t = 0;
			wait_on_det = NOWAIT;
		}
		if (fly_t){
			--fly_t;
			int event = box.checkCollisions(x, y, map_index);
			if (event == STATUS_OK || event == TRIGGER){
				x = box.getX();
				y = box.getY();
			}
			else if (wait_on_det == NOWAIT)trigger = true;
		}
		else if (wait_on_det == NOWAIT)trigger = true;
	}
	if (wait_on_det != WAIT_WITHOUT_INTERACT && rflag){
		rflag = false;
		for (auto m = begin(targets); m != end(targets); ++m){
			int mx = ((*m)->getX() + (*m)->getStepX()*1.5) / (*m)->getStepX();
			int my = ((*m)->getY() + (*m)->getStepY()*1.5) / (*m)->getStepY();
			int px = (box.getX() + (*m)->getStepX()*1.5) / box.getStepX();
			int py = (box.getY() + (*m)->getStepY()*1.5) / box.getStepY();
			int dist = sqrt(pow(px - mx, 2) + pow(py - my, 2));
			if (dist <= range){
				if (typeid(*(*m)) != sh){
					(*m)->takeProjectile(*this);
					trigger = true;
					if (fly_t > dist)fly_t = dist;
				}
			}
		}
		for (auto m = begin(players); m != end(players); ++m){
			int mx = ((*m)->getX() + (*m)->getStepX()*1.5) / (*m)->getStepX();
			int my = ((*m)->getY() + (*m)->getStepY()*1.5) / (*m)->getStepY();
			int px = (box.getX() + (*m)->getStepX()*1.5) / box.getStepX();
			int py = (box.getY() + (*m)->getStepY()*1.5) / box.getStepY();
			int dist = sqrt(pow(px - mx, 2) + pow(py - my, 2));
			if (dist <= range){
				if (typeid(*(*m)) != sh){
					(*m)->takeProjectile(*this);
					trigger = true;
					if (fly_t > dist)fly_t = dist;
				}
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

WeaponBase::WeaponBase(int wtype, int wskill, int wbase_dmg, int ammo_mag, int speed, int fly_t_init, int det_t_init) :
type(wtype), skill_points(wskill), speed(speed), count(0), fly_t_init(fly_t_init), det_t_init(det_t_init),
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

unsigned int& WeaponBase::getCount(){
	return count;
}
unsigned int& WeaponBase::getSpeed(){
	return speed;
}

int WeaponBase::getAmmoPerMag() const {
	return ammo_per_mag;
}

bool WeaponBase::isPickedUp() const {
	return picked_up;
}

int WeaponBase::getFlyT(){
	return fly_t_init;
}

void WeaponBase::pickUp() {
	picked_up = true;
}

AssaultRifle::AssaultRifle(int wskill, Being* assoc_class) : WeaponBase(BULLET, wskill, 9, 30, 15, 80, 80) {
	this->assoc_class = assoc_class;
}

Projectile& AssaultRifle::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, fly_t_init, det_t_init, angle, px, py, assoc_class, h, NOWAIT, 1, 0, 1);
}

Pyrokinesis::Pyrokinesis(int wskill, Being* assoc_class) : WeaponBase(FIRE, wskill, 8, 30, 300, 80, 80) {
	this->assoc_class = assoc_class;
}

Projectile& Pyrokinesis::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, fly_t_init, det_t_init, angle, px, py, assoc_class, h, NOWAIT, 5, 0, 10);
}

Molotov::Molotov(int wskill, Being* assoc_class) : WeaponBase(FIRE, wskill, 1, 30, 100, 40, 70) {
	this->assoc_class = assoc_class;
}

Projectile& Molotov::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, fly_t_init, det_t_init, angle, px, py, assoc_class, h, WAIT_WITH_INTERACT, 8, 300, 12);
}

Punch::Punch(int wskill, Being* assoc_class) : WeaponBase(BULLET, wskill, 7, 30, 40, 5, 5) {
	this->assoc_class = assoc_class;
}

Projectile& Punch::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, fly_t_init, det_t_init, angle, px, py, assoc_class, h, NOWAIT, 1, 0, 4);
}

Bite::Bite(int wskill, Being* assoc_class) : WeaponBase(BULLET, wskill, 8, 30, 3000, 5, 5) {
	this->assoc_class = assoc_class;
}

Projectile& Bite::shoot(double angle, double px, double py, Hitbox& h) {
	return *new Projectile(type, dmg, fly_t_init, det_t_init, angle, px, py, assoc_class, h, NOWAIT, 1, 0, 8);
}


map<int, int> WeaponResources::textures;

int WeaponResources::getTexture(int ti){
	return textures[ti];
}

void WeaponResources::addTexture(int newID, int ti) {
	textures[ti] = newID;
}