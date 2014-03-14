#include <iostream>
#include <random>
using namespace std;
#ifndef _GLIBCXX_MAP_H
#define _GLIBCXX_MAP_H

class Map{
	double roomX, roomY;
	mt19937 pattern;
public:
	struct Coord{
		double x;
		double y;
		Coord(double x, double y) : x(x), y(y){}
	};
	Map();
	double alterBeingPosX(double absoluteX);
	double alterBeingPosY(double absoluteY);
	void loadMap(string seed);
	vector<Coord> generateRoom();

};
#endif