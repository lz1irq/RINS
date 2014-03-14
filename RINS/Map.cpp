#include "Map.h"

Map::Map() : roomX(2), roomY(2){}

double Map::alterBeingPosX(double absoluteX){
	if (absoluteX < 0.5)return absoluteX;
	if (absoluteX > roomX - 0.5)return absoluteX - roomX + 1;
	return 0.5;
}

double Map::alterBeingPosY(double absoluteY){
	if (absoluteY < 0.5)return absoluteY;
	if (absoluteY > roomY - 0.5)return absoluteY - roomY + 1;
	return 0.5;
}

void Map::loadMap(string seed){
	seed_seq Seed(seed.begin(), seed.end());
	pattern.seed(Seed);
}

vector<Map::Coord> Map::generateRoom(){
	int num_tiles = 16;//HARDCODE!
	int coef = (2*roomX*roomY);
	int num_structures = coef + pattern() % coef;
	int yplaces = roomY * num_tiles;	
	int xplaces = roomX * num_tiles;	
	vector<Coord> blocks;
	int direction;
	int X, Y;
	int num_blocks;
	for (int i = 0; i < num_structures; ++i){
		//int structure_type = pattern() % 6;
		int structure_type = 0; ///!!!
		switch (structure_type){
		case 0://STRAIGHT LINE FROM WALL
			num_blocks = pattern() % num_tiles-1;
			direction =pattern() % 4;
			switch (direction){
			case 0://UP
				X = pattern() % xplaces;
				for (int a = 0; a < num_blocks; ++a){
					blocks.push_back(Coord((double)X / xplaces, roomY - 1 / num_tiles - (double)a / yplaces));
				}
				break;
			case 1://DOWN
				X = pattern() % xplaces;
				for (int a = 0; a < num_blocks; ++a){
					blocks.push_back(Coord((double)X / xplaces, (double)a / yplaces));
				}
				break;
			case 2://LEFT
				Y = pattern() % yplaces;
				for (int a = 0; a < num_blocks; ++a){
					blocks.push_back(Coord((double)a / xplaces, (double)Y / yplaces));
				}
				break;
			case 3://RIGHT
				Y = pattern() % yplaces;
				for (int a = 0; a < num_blocks; ++a){
					blocks.push_back(Coord(roomX - 1 / num_tiles - (double)a / xplaces, (double)Y / yplaces));
				}
				break;
			}
			break;
		case 1://THREE-LINE ZIG-ZAG FROM WALL
			//num_blocks = pattern() % (num_tiles*3 - 10);
			break;
		case 2://TWO-LINE ZIG-ZAG FROM WALL
			break;
		case 3://STRAIGHT LINE
			break;
		case 4://THREE-LINE ZIG-ZAG
			break;
		case 5://THREE-LINE PERPENDICULAR
			break;
		}
	}
	return blocks;
}