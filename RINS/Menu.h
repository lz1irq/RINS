#ifndef _GLIBCXX_MENU_H
#define _GLIBCXX_MENU_H
#include <string>
#include <vector>
using namespace std;

class MenuObject{
};

class MenuControl{
	string name;
public:
	MenuControl(string name): name(name){

};

class Button: public MenuControl{
	MenuObject assoc;//menu; action

};
class Radio: public MenuControl{

};

class Command: public MenuObject{
	virtual void* exec() = 0;
};

class Menu: public MenuObject{
	vector<MenuControl> options;
public:
	Menu& addField(MenuControl& m){
		options.push_back(m);
		return *this;
	}
	int getNumOptions(){
		return options.size();
	}
	MenuControl& selectOption(int num){
		return options.at(num);
	}

};

#endif