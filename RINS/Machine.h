#ifndef _GLIBCXX_MACHINE_H
#define _GLIBCXX_MACHINE_H
#include <list>
#include "Item.h"
using namespace std;

class Machine{
	list<Item> items;
public:
	void addItem(Item i);
};

#endif