#ifndef _GLIBCXX_MENU_H
#define _GLIBCXX_MENU_H
#include <string>
#include <vector>
#include <functional>
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

class Checkbox : public MenuControl{
	bool set;
public:
	Checkbox(wstring name, MenuObject& assoc, bool isSet) : MenuControl(name, assoc), set(isSet){}
	bool& access(){
		return set;
	}
};

class Radio: public MenuControl{

};

class Command: public MenuObject{
	function<void()> func;
public:
	Command(function<void()> lambda) : func(lambda){}
	void exec(){
		func();
	}
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