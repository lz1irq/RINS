#ifndef _GLIBCXX_WEAPON_H
#define _GLIBCXX_WEAPON_H
#include <map>
#include <vector>
#include <list>
#include <string.h>
#include <memory>
#include <cmath>
using namespace std;
enum {BULLET, FIRE, PSYCHO, ENERGY};
enum {LEFT=1,RIGHT=2,UP=4,DOWN=8};
#define M_PI    3.14159265358979323846264338327950288   /* pi */
class Hitbox;
class Being;
double deg_to_rad(double deg);
class Projectile {
	unsigned int type;
	int dmg;
	int fly_t;
	int det_t;
	double dir;
	double x,y;
	const char* shooter;
public:
	Projectile(unsigned int ptype, int pdamage, int pfly_t, int pdet_t, double angle, double px, double py, const char* shooter, Hitbox& h);
	bool update(const vector<vector<char>>& map_index, list<unique_ptr<Being>>& targets);
	double getX();
	double getY();
	unsigned int getType() const;
	int getDamage() const;
	virtual ~Projectile(){}
	Hitbox& box;
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
	virtual Projectile& shoot(double angle, double px, double py, Hitbox& h) = 0;
};

class AssaultRifle: public WeaponBase{
public:
	AssaultRifle(int wskill, const char* assoc_class);
	Projectile& shoot(double angle, double px, double py, Hitbox& h);
};

class Pyrokinesis : public WeaponBase{
public:
	Pyrokinesis(int wskill, const char* assoc_class);
	Projectile& shoot(double angle, double px, double py, Hitbox& h);
};

class Molotov : public WeaponBase{
public:
	Molotov(int wskill, const char* assoc_class);
	Projectile& shoot(double angle, double px, double py, Hitbox& h);
};

class Bite : public WeaponBase{
public:
	Bite(int wskill, const char* assoc_class);
	Projectile& shoot(double angle, double px, double py, Hitbox& h);
};

class Punch : public WeaponBase{
public:
	Punch(int wskill, const char* assoc_class);
	Projectile& shoot(double angle, double px, double py, Hitbox& h);
};

#endif