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
enum Detonations{WAIT_WITHOUT_INTERACT, WAIT_WITH_INTERACT, NOWAIT };
#define M_PI    3.14159265358979323846264338327950288   /* pi */
class Hitbox;
class Being;
double deg_to_rad(double deg);
double rad_to_deg(double rad);
class Projectile {
	unsigned int type;
	int dmg;
	int fly_t;
	int det_t;
	double dir;
	double x,y;
	int wait_on_det;
	int speed = 1;
	unsigned int last_time = 0;
	int range, det_duration;
	Being* shooter;//unsafe!!!
	const type_info& sh;
	bool trigger = false;
	bool rflag = true;
public:
	Projectile(unsigned int ptype, int pdamage, int pfly_t, int pdet_t, double angle, double px, double py, Being* shooter, Hitbox& h, int  wait_on_det, int range, int det_duration, int speed);
	bool update(const vector<vector<char>>& map_index, list<unique_ptr<Being>>& targets, list<unique_ptr<Being>>& players, unsigned int time);
	double getX();
	double getY();
	unsigned int getType() const;
	int getDamage() const;
	double getAngleInDeg();
	const Being* getShooter();//unsafe!!!
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
	int& skill_points;
	int base_dmg;
	int dmg;
	int ammo;
	int ammo_per_mag;
	bool picked_up;
	Being* assoc_class;
	unsigned int count = 0, speed;
	int fly_t_init, det_t_init;
public:
	WeaponBase(int wtype, int& wskill, int wbase_dmg,  int ammo_mag, int speed, int fly_t_init, int det_t_init);
	void updateDMG();	
	int getDamage() const;
	int getAmmoPerMag() const;
	bool isPickedUp() const;
	void pickUp();
	unsigned int& getCount();
	unsigned int& getSpeed();
	int getFlyT();
	virtual Projectile& shoot(double angle, double px, double py, Hitbox& h) = 0;
};

class AssaultRifle: public WeaponBase{
public:
	AssaultRifle(int& wskill, Being* assoc_class);
	Projectile& shoot(double angle, double px, double py, Hitbox& h);
};

class Pyrokinesis : public WeaponBase{
public:
	Pyrokinesis(int& wskill, Being* assoc_class);
	Projectile& shoot(double angle, double px, double py, Hitbox& h);
};

class Molotov : public WeaponBase{
public:
	Molotov(int& wskill, Being* assoc_class);
	Projectile& shoot(double angle, double px, double py, Hitbox& h);
};

class Bite : public WeaponBase{
public:
	Bite(int& wskill, Being* assoc_class);
	Projectile& shoot(double angle, double px, double py, Hitbox& h);
};

class Punch : public WeaponBase{
public:
	Punch(int& wskill, Being* assoc_class);
	Projectile& shoot(double angle, double px, double py, Hitbox& h);
};

#endif