#ifndef _GLIBCXX_MACHINE_H
#define _GLIBCXX_MACHINE_H
#include <list>
#include "Item.h"
#include "Being.h"
#include "Platform.h"

using namespace std;

class Machine{
	list<Item*> items;
	list<Item*>::iterator it = items.end();
	bool paid;
public:
	Machine(bool mpaid);
	bool addItem(Item& i);
	void removeItem(int item);
	Item& getNextItem();
	int itemCount();
	bool isPaid();
};

enum MACHINE_TEXTURES{BG=0, FRAME, FRAME_SEL};

class MachineResources {
public:
	static int bg, frame, frame_sel;
};

class MachineManager {
	map<pair<int, int>, Machine*> machines;
	Renderer& rend;
	Game& game;
	Hitbox box;
	int font;
	Machine* curr_machine;
	bool over_machine, render_machine;
	int curr_x, curr_y;

	double deltax, deltay;
	bool pressed, cangetpress;

	double ystart = 0.01;
	double xstart = 0.01;
	double itemx = 0.145;
	double itemy = 0.145;
	double framesp = 0.01;
	int itemsel = -1, itemseli = -1;
	int itemover = -1;
	
	bool mouseOverTile(double dx, double dy, int tx, int ty);

public:
	MachineManager(Game& mgame, Renderer& mrend, Hitbox& mbox, int mfont);
	void add(pair<int, int> mach, bool paid);
	void check(double dx, double dy, int x, int y);
	void set(pair<int, int> mach);
	void unset();
	void render();
	void control(Being* player);
	bool addItem(pair<int, int> mach, Item& it);
	void updateVars(double deltax, double deltay, bool pressed, bool cangetpress);
	bool exists(pair<int, int> mach);
	bool isRendering();
	bool currentIsPaid();
	pair<int, int> getCurrentCoords();
	void clear();
};

#endif