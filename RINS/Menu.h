#ifndef _GLIBCXX_MENU_H
#define _GLIBCXX_MENU_H
#include <string>
#include <vector>
using namespace std;

class MenuObject{
public:
	virtual ~MenuObject(){}
};

class MenuControl{
protected:
	wstring name;
	MenuObject* assoc;
	MenuControl(wstring& name, MenuObject& assoc) : name(name), assoc(&assoc){}
public:
	MenuObject& getObject(){
		return *assoc;
	}
	wstring getText(){
		return name;
	}
	virtual ~MenuControl(){};
};

class Button: public MenuControl{
	MenuObject assoc;//menu; action
public:
	Button(wstring name, MenuObject& assoc) : MenuControl(name, assoc){}

};
class Radio: public MenuControl{

};

class Command: public MenuObject{
public:
	virtual void* exec() = 0;
};

class Menu: public MenuObject{
	vector<MenuControl*> options;
	vector<bool> hitbox;
public:
	Menu& addField(MenuControl& m){
		options.push_back(&m);
		hitbox.push_back(false);
		return *this;
	}
	int getNumOptions(){
		return options.size();
	}
	MenuControl& selectOption(int num){
		return *options.at(num);
	}
	vector<bool>& getHitbox(){
		return hitbox;
	}

};

#endif