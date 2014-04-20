#include "Machine.h"

void Machine::addItem(Item& i){
	items.push_back(&i);
	it = items.end();
}

Item& Machine::getNextItem() {
	++it;
	if(it == items.end()) it = items.begin();
	return *(*it);
}

int Machine::itemCount() {
	return items.size();
}
