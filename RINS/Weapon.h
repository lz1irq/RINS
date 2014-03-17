#include <map>
#include <iostream>
using namespace std;
using namespace std;
#ifndef _GLIBCXX_WEAPON_H
#define _GLIBCXX_WEAPON_H
enum {BULLET, FIRE, PSYCHO, ENERGY};
enum {LEFT=1,RIGHT=2,UP=4,DOWN=8};

class Projectile {
	unsigned int type;
	int dmg;
	int fly_t;
	int det_t;
	unsigned int dir;
	double x,y;
	static map<unsigned int, int> textures;
public:
	Projectile(unsigned int ptype, int pdamage, int pfly_t, int pdet_t, unsigned int pdir, double px, double py);
	void move(double step);
	double getX();
	double getY();
	unsigned int getType() const;
	int getDamage() const;
	int& modFlyTime();
	int& modDetonationTime();
	static void addTexture(unsigned int i, int tid );
	static int getTexture(int i);
	virtual ~Projectile(){}
};

class WeaponResources {
	static map<const char*, int> textures;
public:
	static int getTextureID(const char* ti);
	static void addTextureID(int newID, const char* ti);
};

class WeaponBase {
protected:
	unsigned int type;
	int skill_points;
	int base_dmg;
	int dmg;
	int ammo;
	int ammo_per_mag;
	bool picked_up;
public:
	WeaponBase(int wtype, int wskill, int wbase_dmg,  int ammo_mag);
	void updateSkillPoints(int uskill);
	int getDamage() const;
	int getAmmoPerMag() const;
	bool isPickedUp() const;
	void pickUp();
	virtual Projectile& shoot(int dir, double px, double py)=0;
};

class AssaultRifle: public WeaponBase, public WeaponResources {
public:
	AssaultRifle(int wskill);
	Projectile& shoot(int dir, double px, double py);
};

class Pyrokinesis : public WeaponBase, public WeaponResources {
public:
	Pyrokinesis(int wskill);
	Projectile& shoot(int dir, double px, double py);
};

class Molotov : public WeaponBase, public WeaponResources {
public:
	Molotov(int wskill);
	Projectile& shoot(int dir, double px, double py);
};

class Bite : public WeaponBase, public WeaponResources {
public:
	Bite(int wskill);
	Projectile& shoot(int dir, double px, double py);
};

class Punch : public WeaponBase, public WeaponResources {
public:
	Punch(int wskill);
	Projectile& shoot(int dir, double px, double py);
};

#endif