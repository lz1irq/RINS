#ifndef _GLIBCXX_MACHINE_H
#define _GLIBCXX_MACHINE_H
#include <list>
#include "Item.h"
using namespace std;

class Machine{
	list<Item*> items;
	list<Item*>::iterator it = items.end();
public:
	void addItem(Item& i);
	Item& getNextItem();
	int itemCount();
};

#endif