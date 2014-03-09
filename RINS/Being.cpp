#include "Being.h"
//0.015625 = 1/64 (one being = one screen square;
Being::Being(double x, double y): x(x), y(y),orientation(UP), move_step(1.0/64) {}

void Being::move(int dir) {
	orientation = dir;
	if (dir & LEFT) x-= move_step;
	if (dir & RIGHT) x += move_step;
	if (dir & UP) y -= move_step;
	if (dir & DOWN) y += move_step;
}

double Being::getX() {
	return x;
}

double Being::getY() {
	return y;
}

int Being::getOrientation() {
	return orientation;
}

double Being::getStep(){
	return move_step;
}

void Being::stepTo(double tx, double ty) {
		if(tx < x) move(LEFT);
		if(tx > x) move(RIGHT);
		if(ty < y) move(UP);
		if(ty > y) move(DOWN);
}