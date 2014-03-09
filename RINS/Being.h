#ifndef _GLIBCXX_BEING_H
#define _GLIBCXX_BEING_H

class Being{
	enum {LEFT=1,RIGHT=2,UP=4,DOWN=8};
	double x,y;
	const double move_step;
	int orientation;
public:
	Being(double x, double y);
	void move(int dir);
	double getX();
	double getY();
	int getOrientation();
};
#endif