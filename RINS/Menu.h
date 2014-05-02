#ifndef _GLIBCXX_MENU_H
#define _GLIBCXX_MENU_H
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cmath>
#include "Platform.h"
using namespace std;
//
//class MenuObject{
//public:
//	virtual ~MenuObject(){}
//};
//

wstring utf8_to_utf16(const string& utf8);

class Menu;
enum Textures{ON_CLICK=0, IS_SET, IS_UNSET, MAXT};
class MenuResources {
	static map<const type_info*, int[MAXT]> textures;
public:
	static double optionysize;
	static double optionspacing;
	static double optionxsize;
	static double hsize;
	static int background;
	static int overlay;
	static int back;
	static double backxsize;
	static double backysize;
	static RGBA* textcol[2];

	static void addTexture(int ID, const type_info* control, int type){
		textures[control][type] = ID;
	}
	const int getTexture(const type_info* control, int type){
		return textures[control][type];
	}
};
class MenuControl: protected MenuResources{
protected:
	string name;
	function<void()> func;
	virtual void action(Menu& m) = 0;
public:
	void exec(Menu& m){
		action(m);
		func();
	}
	MenuControl(string name, function<void()> lambda) : name(name), func(lambda){}
	virtual void Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over) = 0;
	virtual ~MenuControl(){};
};
class Menu: private MenuResources{
	vector<MenuControl*> options;
	Menu* prev = nullptr;
	Menu* curr = this;
	double yoffset;
	int  menu_select = -1, lastm = -1;
	friend class MenuButton;
public:
	void setCurr(Menu& m){
		curr = &m;
	}
	Menu& addField(MenuControl& m){
		options.push_back(&m);
		return *this;
	}
	void Render(Renderer& rend, int font){
		rend.applyTexture(background, 0, 0, 1, 1);
		for(int i = 0; i < options.size(); ++i){
			Textures t = MAXT;
			double x = 0.5 - optionxsize / 2.0;
			double y = 0.5 + (optionysize + optionspacing)*i - (optionysize + optionspacing) / 2.0 - yoffset;
			double w = optionxsize;
			double h = optionysize;
			bool mover = lastm == i;
			if(menu_select == i)t = ON_CLICK;
			options.at(i)->Render( x, y, w, h, rend, font, t, mover);
			if(mover)rend.applyTexture(overlay, x, y, w, h);
		}
		if(prev){
			rend.applyTexture(back, 0.1, 0.5-backysize/2.0, backxsize, backysize);
			if(lastm == -2){
				rend.applyTexture(overlay, 0.1, 0.5-backysize/2.0, backxsize, backysize);
			}
		}
	}
	Menu* Check(double mx, double my, bool pressed, bool canpress){
		yoffset = (options.size()-1)*(optionysize + optionspacing) / 2.0;
		bool z = false;
		for(int i = 0; i < options.size(); ++i){
			double optofs = (optionysize + optionspacing) / 2.0;
			double optpos = 0.5 + (optionysize + optionspacing)*i;
			if (mx > 0.5 - optionxsize / 2.0 && mx < 0.5 + optionxsize / 2.0
				&& my > optpos - optofs - yoffset && my < optpos + optofs - yoffset - optionspacing){
					lastm = i;
					z = true;
			}
		}
		if(mx > 0.1 && mx < 0.1+backxsize && my < 0.5+backysize/2.0 && my > 0.5-backysize/2.0){
			lastm = -2;
			z = true;
		}
		if(!z)lastm = -1;
		if(lastm != -1 && pressed)menu_select = lastm;
		if(menu_select != -1 && canpress){
			if(lastm == menu_select){
				if(lastm > -1)options.at(lastm)->exec(*this);
				else if(prev)curr = prev;
			}
			menu_select = -1;
		}
		Menu* m = curr;
		curr = this;
		return m;
	}
};
class MenuButton: public MenuControl{
	void action(Menu& m){
		assoc.prev = &m;
		m.setCurr(assoc);
	}
public:
	Menu& assoc;
	MenuButton(Menu& assoc, string name, function<void()> lambda): MenuControl(name, lambda), assoc(assoc){}
	void Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over){
		if(t != ON_CLICK)t = IS_UNSET;
		rend.applyTexture(getTexture(&typeid(*this), t), x, y, w, h);
		double tw, th;
		rend.getTextWH(font, name.c_str(), tw, th);
		tw *= hsize / th;
		th = hsize;
		RGBA* color;
		if(m_over)color = textcol[0];
		else color = textcol[1];
		rend.displayText(font, name.c_str(), *color, x+w/2.0-tw/2.0, y+h/2.0-th/2.0, tw, th);
	}
};
//class MenuControl{
//protected:
//	string name;
//	MenuObject* assoc;
//	bool set = false;
//	int id = 0;
//	MenuControl(string& name, MenuObject& assoc) : name(name), assoc(&assoc){}
//public:
//	MenuObject& getObject(){
//		return *assoc;
//	}
//	string& getText(){
//		return name;
//	}
//	bool& checked(){
//		return set;
//	}
//	int& getID(){
//		return id;
//	}
//	virtual ~MenuControl(){};
//};
//
//class Button: public MenuControl{
//	MenuObject assoc;//menu; action
//public:
//	Button(string name, MenuObject& assoc) : MenuControl(name, assoc){}
//};
//
//class Checkbox : public MenuControl{
//public:
//	Checkbox(string name, MenuObject& assoc, bool isSet) : MenuControl(name, assoc){
//		set = isSet;
//	}
//};
//
//class TextBox : public MenuControl{
//public:
//	TextBox(string name, MenuObject& assoc) : MenuControl(name, assoc){ 
//		id = name.size(); 
//		set = false;
//	}
//
//};
//
//class Radio: public MenuControl{
//
//};
//
//class Command: public MenuObject{
//	function<void(MenuControl&)> func;
//public:
//	Command(function<void(MenuControl&)> lambda) : func(lambda){}
//	void exec(MenuControl& mc){
//		Checkbox* cb = dynamic_cast<Checkbox*>(&mc);
//		if(cb)mc.checked() = !mc.checked();
//
//		func(mc);
//	}
//};
//
//class Menu: public MenuObject{
//	vector<MenuControl*> options;
//	vector<bool> hitbox;
//public:
//	Menu& addField(MenuControl& m){
//		options.push_back(&m);
//		hitbox.push_back(false);
//		return *this;
//	}
//	int getNumOptions(){
//		return options.size();
//	}
//	MenuControl& selectOption(int num){
//		return *options.at(num);
//	}
//	vector<bool>& getHitbox(){
//		return hitbox;
//	}
//
//};
//
#endif