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
	string name;
	MenuObject* assoc;
	bool set = false;
	int id = 0;
	MenuControl(string& name, MenuObject& assoc) : name(name), assoc(&assoc){}
public:
	MenuObject& getObject(){
		return *assoc;
	}
	string& getText(){
		return name;
	}
	bool& checked(){
		return set;
	}
	int& getID(){
		return id;
	}
	virtual ~MenuControl(){};
};

class Button: public MenuControl{
	MenuObject assoc;//menu; action
public:
	Button(string name, MenuObject& assoc) : MenuControl(name, assoc){}
};

class Checkbox : public MenuControl{
public:
	Checkbox(string name, MenuObject& assoc, bool isSet) : MenuControl(name, assoc){
		set = isSet;
	}
};

class TextBox : public MenuControl{
public:
	TextBox(string name, MenuObject& assoc) : MenuControl(name, assoc){ 
		id = name.size(); 
		set = false;
	}

};

class Radio: public MenuControl{

};

class Command: public MenuObject{
	function<void(MenuControl&)> func;
public:
	Command(function<void(MenuControl&)> lambda) : func(lambda){}
	void exec(MenuControl& mc){
		Checkbox* cb = dynamic_cast<Checkbox*>(&mc);
		if(cb)mc.checked() = !mc.checked();

		func(mc);
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