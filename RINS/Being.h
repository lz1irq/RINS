#ifndef _GLIBCXX_BEING_H
#define _GLIBCXX_BEING_H
#include "Weapon.h"
#include "Item.h"
#include <ctime>
#include <random>
#include <array>
#include <typeinfo>
#include <algorithm>
#include <mutex>
using namespace std;

struct IDs {
	int ids[2];
	IDs();
};

class BeingResources {
	static map<const type_info*, IDs> textures;
public:
	static const int getTextureID(const type_info* ti);
	static void addTextureID(int newID, const type_info*);
	static const int getSoundID(const type_info* si);
	static void addSoundID(int newID, const type_info* si);
};

enum Monsters{ ZOMBIE = 0, MAXSIZE };
enum Playable{ MARINE=0, PYRO, PSYCHOKINETIC, ANDROID, PLAYEND};
enum Collisions{ STATUS_OK, OUT_OF_BOUNDS, TRIGGER, X_COLLIDE, Y_COLLIDE, XY_COLLIDE };
enum Shoot{BANG, NOT_IN_FOV, CASTING, OUT_OF_RANGE, NOT_IN_LOS};

class Hitbox{
protected:
	double x, y;
	const double move_step_x;
	const double move_step_y;
	const int tiles_x;
	const int tiles_y;
	const int tile_granularity;
public:
	Hitbox(int tiles_x, int tiles_y, int tile_granularity);
	void setX(double x);
	void setY(double y);
	double getX() const;
	double getY() const;
	int getTileX();
	int getTileY();
	double getStepX() const;
	double getStepY() const;
	int checkCollisions(double comp_to_x, double comp_to_y, const vector<vector<char>>& index);
	//friend class Being;
};

class Being: public Hitbox{
protected:
	int level;
	int orientation;
	int range;
	int money = 0;
	Primary prim_stats;
	Derived der_stats;
	int curr_weapon;
	vector<std::unique_ptr<WeaponBase>> weapons;
	static mt19937 rnd;
	bool walk = false;
	virtual void setRange() = 0;
	Projectile& shootWeapon(double deg, Hitbox& h);
	void walkAround(const vector<vector<char>>& map_index);
	bool wallInFront(double target_x, double target_y, double deg, const vector<vector<char>>& map_index);
	bool isTargetBehindBack(double delta_x, double delta_y);
	int internalShoot(double target_x, double target_y, double deg, Projectile** p);
	void updateTarget(const vector<vector<char>>& map_index, const list<unique_ptr<Being>>& targets);
	
	int HAJA = 0;

	int move_dist = 0;
	int move_dir = 0;
	const Being* curr_target = nullptr;
	int curr_threat = 0;

	vector<Item*> items;
	vector<Item*>::iterator it = items.end();
	int experience = 0; 

	unsigned int speed, count = 0;
	unsigned int start_time;

	mutex inv;
	map<const char*, int> classSkills;

public:
	Being(double x, double y, int speed);
	int getHealth();
	int getMaxHealth();
	virtual bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) = 0;
	bool move(int dir, bool reverse);
	int getLevel();
	bool getWalk();
	void resetWalk();
	int getOrientation() const;
	virtual void levelup();
	void addExperience(int xp);
	int getExperience();
	Derived& getDerivedStats();
	Primary& getPrimaryStats();
	map<const char*, int>& getClassSkills();

	//weapons
	void takeProjectile(Projectile& bullet);
	int tryToShoot(Being* target, Projectile** p, const vector<vector<char>>& map_index);
	bool setCurrentWeapon(int new_wp);
	int getCurrentWeapon();
	int weaponCount();
	WeaponBase& getWeapon(int wp_id);

	//items and money
	Item& getNextItem();
	int itemCount();
	Item& getItem(int item);
	bool buyItem(Item& item);
	Item& sellItem(int item);
	int getMoney();
	bool addItem(Item& i);
	Item& removeItem(int item);
	bool equipItem(Item& item);
	void unequipItem(Item& item);

	void resetFire();
	static Hitbox* bx;
	virtual ~Being();
};

template<typename T> Being * createInstance(double x, double y) { return new T(x, y); }
template<class T> Being* copyInstance(T& copy){ return new T(copy); }

class Marine: public Being, BeingResources {
public:
	int small_guns, small_guns_bonus;
	int big_guns, big_guns_bonus;
	int energy_weapons, energy_weapons_bonus;
	Marine(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
	void setRange() final;
	void levelup();
};

class Pyro:public Being, BeingResources {
public:
	int explosives, explosives_bonus;
	int big_guns, big_guns_bonus;
	int fire, fire_bonus;
	Pyro(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
	void setRange() final;
	void levelup();
};

class Psychokinetic: public Being, BeingResources {
public:
	int mind_infiltration, mind_infiltration_bonus;
	int mental_power, mental_power_bonus;
	int fire, fire_bonus;
	Psychokinetic(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
	void setRange() final;
	void levelup();
};

class Zombie: public Being, BeingResources {
private:
	int biting;
	Being* target;
public:
	Zombie(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
	void setRange() final;
	void levelup();
};

class Android: public Being, BeingResources {
private:
	int punch, punch_bonus;
	int big_guns, big_guns_bonus;
	int energy_weapons, energy_weapons_bonus;
public:
	Android(double sx, double sy);
	bool action(const vector<vector<char>>& map_index, list<Projectile>& projectiles, const list<unique_ptr<Being>>& targets, unsigned int start_time) final;
	void setRange() final;
	void levelup();
};

#endif