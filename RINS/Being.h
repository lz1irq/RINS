#ifndef _GLIBCXX_BEING_H
#define _GLIBCXX_BEING_H

class Being{
	enum Direction{LEFT=1,RIGHT=2,UP=4,DOWN=8};
	double x,y;
	const double move_step;
	Direction orientation;
public:
	Being(double x, double y, Direction dir);
	void move(Direction dir);
	double getX();
	double getY();
	Direction getOrientation();
	~Being();
};
#endif