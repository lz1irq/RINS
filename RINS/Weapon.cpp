#include "Weapon.h"
#include "Being.h"
Projectile::Projectile(unsigned int ptype, int pdmg, int pfly_t, int pdet_t, unsigned int pdir, double px, double py, const char* shooter) :
type(ptype), dmg(pdmg),
fly_t(pfly_t), det_t(pdet_t),
dir(pdir), x(px), dummy(new Zombie(px, py)),
y(py), shooter(shooter)
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

bool Projectile::update(const vector<vector<char>>& map_index, vector<Being*> targets){
	double newx = dummy->getX();
	double newy = dummy->getY();
	int x_colide, y_colide;
	if (dir & LEFT)newx -= dummy->getStepX();
	if (dir & RIGHT)newx += dummy->getStepX();
	if (dir & UP)newy -= dummy->getStepY();
	if (dir & DOWN)newy += dummy->getStepY();

	if (fly_t){
		--fly_t;
		dummy->checkCollisions(newx, newy, map_index, x_colide, y_colide);
		x = dummy->getX();
		y = dummy->getY();
	}
	if (det_t)--det_t;
	else return false; //explode!

	for (auto m = begin(targets); m != end(targets); ++m){

		int mx = (*m)->getX() / (*m)->getStepX() + 1;
		int my = (*m)->getY() / (*m)->getStepY() + 1;
		int px = dummy->getX() / dummy->getStepX() + 1;
		int py = dummy->getY() / dummy->getStepY() + 1;

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

Projectile& AssaultRifle::shoot(int dir, double px, double py) {
	return *new Projectile(type, dmg, 80, 100, dir, px, py, assoc_class);
}

Pyrokinesis::Pyrokinesis(int wskill, const char* assoc_class) : WeaponBase(FIRE, wskill, 15, 30) {
	this->assoc_class = assoc_class;
}

Projectile& Pyrokinesis::shoot(int dir, double px, double py) {
	return *new Projectile(type, dmg, 80, 100, dir, px, py, assoc_class);
}

Molotov::Molotov(int wskill, const char* assoc_class) : WeaponBase(FIRE, wskill, 15, 30) {
	this->assoc_class = assoc_class;
}

Projectile& Molotov::shoot(int dir, double px, double py) {
	return *new Projectile(type, dmg, 80, 100, dir, px, py, assoc_class);
}

Punch::Punch(int wskill, const char* assoc_class) : WeaponBase(BULLET, wskill, 15, 30) {
	this->assoc_class = assoc_class;
}

Projectile& Punch::shoot(int dir, double px, double py) {
	return *new Projectile(type, dmg, 80, 100, dir, px, py, assoc_class);
}

Bite::Bite(int wskill, const char* assoc_class) : WeaponBase(BULLET, wskill, 40, 30) {
	this->assoc_class = assoc_class;
}

Projectile& Bite::shoot(int dir, double px, double py) {
	return *new Projectile(type, dmg, 1, 0, dir, px, py, assoc_class);
}


map<int, int> WeaponResources::textures;

int WeaponResources::getTexture(int ti){
	return textures[ti];
}

void WeaponResources::addTexture(int newID, int ti) {
	textures[ti] = newID;
}