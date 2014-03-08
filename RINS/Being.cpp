#include "Being.h"
Being::Being(double x, double y, Direction orient): x(x), y(y), orientation(orient) {}

void Being::move(Direction dir) {
	orientation = dir;
	if (dir & LEFT) x-= move_step;
	if (dir & RIGHT) x+= move_step;
	if (dir & UP) y+= move_step;
	if (dir & DOWN) y-= move_step;
}

double Being::getX() {
	return x;
}

double Being::getY() {
	return y;
}

Being::Direction Being::getOrientation() {
	return orientation;
}