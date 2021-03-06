#include "Map.h"
#include <bitset>

//int xsize, ysize; //SHOULD BE FIXED
Map::Map() : roomX(1), roomY(1){
	room_exit_x = 0;
	room_exit_y = roomX/2*ysize;
	room_entry_x = 0;
	room_entry_y = roomX / 2 * ysize;
	//::xsize = xsize;
	//::ysize = ysize;
}

double Map::alterBeingPosX(double absoluteX){
	absoluteX += offsetx;
	//cout << absoluteX - roomX + 1 << endl;
	if (absoluteX < 0.5)return absoluteX;
	if (absoluteX > roomX - 0.5)return absoluteX - roomX + 1;
	return 0.5;
}

double Map::alterBeingPosY(double absoluteY){
	absoluteY += offsety+0.005;
	if (absoluteY < 0.5)return absoluteY;
	if (absoluteY > roomY - 0.5)return absoluteY - roomY + 1;
	return 0.5;
}

void Map::loadMap(time_t seed){
	//seed_seq Seed(seed.begin(), seed.end());
	//uint32_t seeds[1];
	//Seed.generate(&seeds[0], &seeds[1]);
	//curr_seed = seeds[0];
	last_room = 1, curr_room = 1;
	curr_seed = seed;
	generateRoom(curr_seed, true);
}

Map::Coord Map::getMapEntry(){
	if (!last_entry){
		offsetx = (double)(room_entry_x-room_exit_x) / xsize;
		offsety = (double)(room_entry_y-room_exit_y) / ysize;
		return Coord((double)(room_entry_x) / xsize, (double)(room_entry_y) / ysize, 0);
	}
	offsetx = (double)(room_exit_x - room_entry_x) / xsize;
	offsety = (double)(room_exit_y - room_entry_y) / ysize;
	return Coord((double)(room_exit_x) / xsize, (double)(room_exit_y) / ysize, 0);
}

bool Map::updateInternalMapState(){
	bool needs_time = false;
	if (offsetx > 0.01){ offsetx -= 0.01; needs_time = true; }
	if (offsety > 0.01){ offsety -= 0.01; needs_time = true; }
	if (offsetx < -0.01){ offsetx += 0.01; needs_time = true; }
	if (offsety < -0.01){ offsety += 0.01; needs_time = true; }
	if (!needs_time){
		offsetx = 0;
		offsety = 0;
	}
	return needs_time;
}

bool Map::tryRoomChange(int x, int y){
	bool exit = false;
	bool isexit;
	if (x < 0 || x >= room.size()){
		isexit = abs(room_entry_x-x) > abs(room_exit_x-x) ? true : false;
		exit = true;
	}
	if (y < 0 || y >= room[0].size()){		//SHOULD BE FIXED
		isexit = abs(room_entry_y - y) > abs(room_exit_y - y) ? true : false;
		exit = true;
	}
	if (exit){
		if (isexit){
			if (curr_room == last_room)++last_room;
			++curr_room;
			generateRoom(++curr_seed, 1);
			last_entry = false;
		}
		if (!isexit){
			--curr_room;
			generateRoom(--curr_seed, 0);
			last_entry = true;
		}
	}
	return exit;

}

int Map::getMaxMonsters(){
	return last_room*10;
}

int Map::getSpawnRate(){
	return 42/(1+0.1*last_room);
}

long long int Map::getLastExploredRoom(){
	return last_room;
}

long long int Map::getCurrentRoomNumber(){
	return curr_room;

}

void Map::getRoomSize(double& x, double& y){
	x = roomX;
	y = roomY;
}

int Map::getMapType(){
	return map_type;
}

void Map::addLoot(int xtile, int ytile){
	room[xtile][ytile] = DROP;
	blocks.push_back(Coord((double)xtile / xsize, (double)ytile / ysize, DROP));

}

void Map::generateRoom(uint32_t seed_, bool exited){
	pattern.seed(seed_);
	uint32_t cx = pattern() % 2000;
	uint32_t cy = pattern() % 2000;
	uint32_t r1 = pattern();
	uint32_t r2 = 1 + (pattern() % 3);
	uint32_t r3 = pattern();
	int last_exit_x = room_exit_x;
	int last_exit_y = room_exit_y;
	int last_entry_x = room_entry_x;
	int last_entry_y = room_entry_y;
	int xmax = xsize*roomX - 1;
	int ymax = ysize*roomY - 1;

	int curr_wall, curr_x, curr_y;
	if (exited){
		curr_x = last_exit_x;
		curr_y = last_exit_y;
	}
	else{
		curr_x = last_entry_x;
		curr_y = last_entry_y;
	}

	if (curr_x == 0)curr_wall = LEFT;
	if (curr_x == xmax)curr_wall = RIGHT;
	if (curr_y == 0)curr_wall = UP;
	if (curr_y == ymax)curr_wall = DOWN;


	roomX = 1 + (double)cx/1000.0;
	roomY = 1 + (double)cy/1000.0;

	xmax = xsize*roomX - 1;
	ymax = ysize*roomY - 1;

	roomX = (xmax + 1) / (double)xsize;
	roomY = (ymax + 1) / (double)ysize;

	int temp = min(ymax, xmax) - 4;
	int newplace = 2;

	if (exited)newplace += r1 % temp;
	else newplace += r3 % temp;

	switch(curr_wall){
		case LEFT:
			curr_x = xmax;
			curr_y = newplace;
			break;
		case RIGHT:
			curr_x = 0;
			curr_y = newplace;
			break;
		case UP:
			curr_y = ymax;
			curr_x = newplace;
			break;
		case DOWN:
			curr_y = 0;
			curr_x = newplace;
			break;
	}

	if (exited){
		room_entry_x = curr_x;
		room_entry_y = curr_y;
	}
	else{
		room_exit_x = curr_x;
		room_exit_y = curr_y;
	}

	newplace = 2;

	int opposite_wall = 3 - curr_wall;

	if (exited){
		opposite_wall += r2;
		opposite_wall %= 4;
	}
	else{
		opposite_wall += (4-r2);
		opposite_wall %= 4;
	}
	newplace = 2;
	if (exited)newplace += r3 % temp;
	else newplace += r1 % temp;

	switch (opposite_wall){
	case UP:
		curr_y = 0;
		curr_x = newplace;
		break;
	case DOWN:
		curr_y = (ysize*roomY - 1);
		curr_x = newplace;
		break;
	case RIGHT:
		curr_x = (xsize*roomX - 1);
		curr_y = newplace;
		break;
	case LEFT:
		curr_x = 0;
		curr_y = newplace;
		break;
	}

	if (exited){
		room_exit_x = curr_x;
		room_exit_y = curr_y;
	}
	else{
		room_entry_x = curr_x;
		room_entry_y = curr_y;
	}
	room = *new vector<vector<char>>(roomX*xsize, vector<char>(roomY*ysize, 0));
	int xplaces = roomX*xsize, yplaces = roomY*ysize;
	for (int i = 0; i < xplaces; ++i){
		room[i][0] = 2;
		room[i][yplaces - 1] = 2;
	}
	for (int i = 0; i < yplaces; ++i){
		room[0][i] = 2;
		room[xplaces - 1][i] = 2;
	}
	room[room_entry_x][room_entry_y] = ENTRY;
	room[room_exit_x][room_exit_y] = EXIT;

	map_type = pattern() % 3;

	if (map_type == LABYRINTH){
		const int fieldsize = (roomX*xsize - 4) / 3;
		const int fysize = (roomY*ysize - 5) / 3;
		int index[1024];
		int index2[1024];
		int maxind = 0;
		for (int i = 0; i < fieldsize; ++i){
			index[i] = i;
			++maxind;
		}
		int ypos = 2;
		for (int h = 0; h < fysize; ++h){

			for (int i = 1; i < fieldsize; ++i){
				bool join = pattern() % 2;
				if (index[i - 1] != index[i] && join)index[i] = index[i - 1];
			}
			for (int i = 1; i < fieldsize; ++i){
				if (index[i - 1] != index[i]){
					room[2 + i * 3][ypos + 0] = 2;
					room[2 + i * 3][ypos + 1] = 2;
					room[2 + i * 3][ypos + 2] = 2;
					room[2 + i * 3][ypos + 3] = 2;
				}
			}
			fill_n(index2, fieldsize, -1);
			int count = 1;
			for (int i = 1; i < fieldsize; ++i){
				if (index[i - 1] == index[i])++count;
				if (index[i - 1] != index[i] || i == fieldsize - 1){
					//for (int j = 0; j < count; ++j){
					int where = pattern() % count;
					index2[i + where - count] = index[i + where - count];
					//}
					count = 1;
				}
			}
			for (int i = 1; i < fieldsize; ++i){
				if (index2[i - 1] != index2[i] && index2[i - 1] == -1 || index2[i] == -1)++maxind;
				if (index2[i - 1] == -1)index2[i - 1] = maxind;
				if (index2[i] == -1)index2[i] = maxind;
			}
			for (int i = 0; i < fieldsize; ++i){
				if (index2[i] != index[i]){
					room[2 + i * 3 + 0][ypos + 3] = 2;
					room[2 + i * 3 + 1][ypos + 3] = 2;
					room[2 + i * 3 + 2][ypos + 3] = 2;
				}
				index[i] = index2[i];
			}
			maxind = 0;
			int lastind = -1;
			bool change;
			for (int i = 0; i < fieldsize; ++i){
				if (lastind != index2[i])change = true;
				lastind = index2[i];
				if (change){
					index2[i] = maxind;
					change = false;
					++maxind;
				}
			}
			ypos += 3;
		}
	}
	else{
		int numstructs = pattern() % 20;
		numstructs *= roomX*roomY;
		for (int i = 0; i < numstructs; ++i){
			int structsize = 2 + (pattern() % 5);
			int structori = pattern() % 2;//X:Y
			int structdir = pattern() % 2;
			int initialx = pattern() % xmax;
			int initialy = pattern() % ymax;
			int endx, endy;
			bool begin;
			switch (structori){
			case 0: //orientation X
				endx = initialx + structsize;
				endy = initialy;
				if (!structdir)endx = initialx + structsize;
				else endx = initialx - structsize;
				begin = true;
				for (int a = min(initialx, endx); a < max(initialx, endx); ++a){
					int x = a;
					int y = endy;
					try{
						if (!room.at(x-1).at(y-1)){
							if (!room.at(x).at(y - 1)){
								if (!room.at(x + 1).at(y - 1)){
									if (!room.at(x + 1).at(y)){
										if (!room.at(x + 1).at(y + 1)){
											if (!room.at(x).at(y + 1)){
												if (!room.at(x - 1).at(y + 1)){
													if (!room.at(x - 1).at(y) || begin == false){
														room[x][y] = 2;
													}
												}
											}
										}
									}
								}
							}
						}
					}
					catch (...){}
					begin = false;
				}
				break;
			case 1: //orientation Y
				endx = initialx;
				if (!structdir)endy = initialy + structsize;
				else endy = initialy - structsize;
				begin = true;
				for (int a = min(initialy, endy); a < max(initialy, endy); ++a){
					int x = endx;
					int y = a;
					try{
						if (!room.at(x - 1).at(y - 1)){
							if (!room.at(x).at(y - 1) || begin == false){
								if (!room.at(x + 1).at(y - 1)){
									if (!room.at(x + 1).at(y)){
										if (!room.at(x + 1).at(y + 1)){
											if (!room.at(x).at(y + 1)){
												if (!room.at(x - 1).at(y + 1)){
													if (!room.at(x - 1).at(y)){
														room[x][y] = 2;
													}
												}
											}
										}
									}
								}
							}
						}
					}
					catch (...){}
					begin = false;
				}
				break;
			}
		}
	}

	int numvending = pattern() % 4;
	for (int i = 0; i < numvending; ++i){
		int sx = 1 + (pattern() % (xmax - 2));
		int sy = 1 + (pattern() % (ymax - 2));
		room[sx][sy] = VENDING;
	}

	//generate some basic structures

	//create map map
	blocks.clear();
	for (int x = 0; x < room.size(); ++x){
		for (int y = 0; y < room[x].size(); ++y){
			blocks.push_back(Coord((double)x / xsize, (double)y / ysize, 0));
			if (room[x][y]){
				if (room[x][y] < 16){
					bool hasleft = x == 0 || room[x - 1][y] > 15 ? false : room[x - 1][y];
					bool hasright = x == room.size() - 1 || room[x + 1][y]  > 15 ? false : room[x + 1][y];
					bool hasup = y == 0 || room[x][y - 1] > 15 ? false : room[x][y - 1];
					bool hasdown = y == room[x].size() - 1 || room[x][y + 1] > 15 ? false : room[x][y + 1];
					char edges = (hasleft << 0) | (hasright << 1) | (hasup << 2) | (hasdown << 3);
					switch (edges){
					case 1:
						room[x][y] = 1;
						break;
					case 2:
						room[x][y] = 2;
						break;
					case 3:
						room[x][y] = 1;
						break;
					case 4:
						room[x][y] = 3;
						break;
					case 5:
						room[x][y] = 4;
						break;
					case 6:
						room[x][y] = 3;
						break;
					case 7:
						room[x][y] = 4;
						break;
					case 8:
						room[x][y] = 2;
						break;
					case 9:
						room[x][y] = 1;
						break;
					case 10:
						room[x][y] = 2;
						break;
					case 11:
						room[x][y] = 1;
						break;
					case 12:
						room[x][y] = 3;
						break;
					case 13:
						room[x][y] = 4;
						break;
					case 14:
						room[x][y] = 3;
						break;
					case 15:
						room[x][y] = 4;
						break;
					}
				}
				blocks.push_back(Coord((double)x / xsize, (double)y / ysize, room[x][y]));
			}
		}
	}
}

const vector<Map::Coord>& Map::getMapObjects() const{
	return blocks;
}

const vector<vector<char>>& Map::getMapIndex() const{
	return room;
}