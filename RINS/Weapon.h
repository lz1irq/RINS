enum {BULLET, FIRE, ENERGY, PSYCHO };

class Projectile {
public:
	Projectile();
};

class Weapon {
	char* name;
	int dmg;
	int ammo_per_mag;
	bool picked_up;
public:
	Weapon(char* wname, int wdmg, int ammo_mag);
	char* getName();
	int getDamage();
	int getAmmoPerMag();
	bool isPickedUp();
	void pickUp();
	Projectile* shoot();
};