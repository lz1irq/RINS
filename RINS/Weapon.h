#include <map>
using namespace std;
#ifndef _GLIBCXX_WEAPON_H
#define _GLIBCXX_WEAPON_H
enum {BULLET, FIRE, PSYCHO, ENERGY};

class Projectile {
	unsigned int type;
	int dmg;
	int fly_t;
	int det_t;
	unsigned int dir;
public:
	Projectile(unsigned int ptype, int pdamage, int pfly_t, int pdet_t, unsigned int pdir);
	unsigned int getType() const;
	int getDamage() const;
	int getFlyTime() const;
	int getDetonationTime() const;
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
	virtual Projectile shoot(int dir)=0;
};

class AssaultRifle: public WeaponBase, public WeaponResources {
public:
	AssaultRifle(int wskill);
	Projectile shoot(int dir);
};

class Pyrokinesis : public WeaponBase, public WeaponResources {
public:
	Pyrokinesis(int wskill);
	Projectile shoot(int dir);
};

class Molotov : public WeaponBase, public WeaponResources {
public:
	Molotov(int wskill);
	Projectile shoot(int dir);
};

class Bite : public WeaponBase, public WeaponResources {
public:
	Bite(int wskill);
	Projectile shoot(int dir);
};
#endif