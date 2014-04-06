#ifndef _GLIBCXX_WEAPON_H
#define _GLIBCXX_WEAPON_H
#include <map>
#include <vector>
#include <list>
#include <string.h>
#include <memory>
using namespace std;
enum {BULLET, FIRE, PSYCHO, ENERGY};
enum {LEFT=1,RIGHT=2,UP=4,DOWN=8};
class Hitbox;
class Being;
class Projectile {
	unsigned int type;
	int dmg;
	int fly_t;
	int det_t;
	unsigned int dir;
	double x,y;
	const char* shooter;
public:
	Projectile(unsigned int ptype, int pdamage, int pfly_t, int pdet_t, unsigned int pdir, double px, double py, const char* shooter);
	bool update(const vector<vector<char>>& map_index, list<unique_ptr<Being>>& targets);
	double getX();
	double getY();
	unsigned int getType() const;
	int getDamage() const;
	virtual ~Projectile(){}
	static Hitbox* box;
};

class WeaponResources {
	static map<int, int> textures;
public:
	static int getTexture(int ti);
	static void addTexture(int newID, int ti);
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
	const char* assoc_class;
public:
	WeaponBase(int wtype, int wskill, int wbase_dmg,  int ammo_mag);
	void updateSkillPoints(int uskill);
	int getDamage() const;
	int getAmmoPerMag() const;
	bool isPickedUp() const;
	void pickUp();
	virtual Projectile& shoot(int dir, double px, double py)=0;
};

class AssaultRifle: public WeaponBase{
public:
	AssaultRifle(int wskill, const char* assoc_class);
	Projectile& shoot(int dir, double px, double py);
};

class Pyrokinesis : public WeaponBase{
public:
	Pyrokinesis(int wskill, const char* assoc_class);
	Projectile& shoot(int dir, double px, double py);
};

class Molotov : public WeaponBase{
public:
	Molotov(int wskill, const char* assoc_class);
	Projectile& shoot(int dir, double px, double py);
};

class Bite : public WeaponBase{
public:
	Bite(int wskill, const char* assoc_class);
	Projectile& shoot(int dir, double px, double py);
};

class Punch : public WeaponBase{
public:
	Punch(int wskill, const char* assoc_class);
	Projectile& shoot(int dir, double px, double py);
};

#endif