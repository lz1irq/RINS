#include "Weapon.h"
Weapon::Weapon(char* wname, int wdmg, int ammo_mag): name(wname), dmg(wdmg), ammo_per_mag(ammo_mag) {}

char* Weapon::getName() { 
	return name;
}

int Weapon::getDamage() {
	return dmg;
}

int Weapon::getAmmoPerMag() {
	return ammo_per_mag;
}

bool Weapon::isPickedUp() {
	return picked_up;
}

void Weapon::pickUp() {
	picked_up = true;
}

