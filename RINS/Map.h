#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
using namespace std;
#ifndef _GLIBCXX_MAP_H
#define _GLIBCXX_MAP_H

class Map{
	double roomX, roomY;
	mt19937 pattern;
	const int xsize = 16, ysize = 16;
	uint32_t curr_seed;
	enum walls{ LEFT, DOWN, UP, RIGHT };
	bool last_entry = true;
	double offsetx = 0, offsety = 0;
public:
	struct Coord{
		double x;
		double y;
		char type;
		Coord(){}
		Coord(double x, double y, char type) : x(x), y(y), type(type){}
	};
	Map();
	double alterBeingPosX(double absoluteX);
	double alterBeingPosY(double absoluteY);
	void loadMap(string seed);
	const vector<Coord>& getMapObjects() const;
	const vector<vector<char>>& getMapIndex() const;
	Coord getMapEntry();
	bool tryRoomChange(int x, int y);
	bool updateInternalMapState();
private:
	vector<Coord> blocks;
	vector<vector<char>> room;
	int room_entry_x, room_entry_y, room_exit_x, room_exit_y;
	void generateRoom(uint32_t seed, bool exited);
};
#endif