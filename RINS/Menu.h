#ifndef _GLIBCXX_MENU_H
#define _GLIBCXX_MENU_H
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cmath>
#include "Platform.h"
using namespace std;
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
	static int mframe;
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
	function<void(MenuControl&)> func;
	virtual void action(Menu& m) = 0;
public:
	void exec(Menu& m){
		action(m);
		func(*this);
	}
	MenuControl(string name, function<void(MenuControl&)> lambda) : name(name), func(lambda){}
	virtual void Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over) = 0;
	virtual ~MenuControl(){};
};
class Menu: private MenuResources{
	vector<MenuControl*> options;
	Menu* prev = nullptr;
	Menu* curr = this;
	double yoffset;
	int  menu_select = -1, lastm = -1;
	bool textflag = false;
	int textdone = 0;
	int textfield = -1;
	const char* text = "";
	friend class MenuButton;
	Game* base;
	string title;
public:
	Menu(string title): title(title){}
	void setCurr(Menu& m){
		curr = &m;
	}
	void getText(const char* text){
		textflag = true;
		textfield = lastm;
		base->startTyping(text);
		textdone = 0;
	}
	const char* textVisual(){
		return base->getText();
	}
	const char* textRes(){
		return text;
	}
	int textDone(){
		return textdone;
	}
	void ResetText(const char* old){
		base->endTyping(true);
		base->startTyping(old);
	}
	Menu& addField(MenuControl& m){
		options.push_back(&m);
		return *this;
	}
	void Render(Renderer& rend, int font){
		double x = 0.5 - optionxsize / 2.0;
		rend.applyTexture(background, 0, 0, 1, 1);
		double tw, th;
		rend.getTextWH(font, title.c_str(), tw, th);
		tw *= hsize / th;
		th = hsize;
		rend.displayText(font, title.c_str(), *textcol[0], x+0.005, 0.5-yoffset-1.2*(optionysize + optionspacing), tw, th);
		rend.applyTexture(mframe, x, 0.5-yoffset-1.2*(optionysize + optionspacing), optionxsize, 
		(optionysize + optionspacing)*options.size()+optionysize);
		for(int i = 0; i < options.size(); ++i){
			Textures t = MAXT;
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
	Menu* Check(double mx, double my, bool pressed, bool canpress, Game& base){
		this->base = &base;
		if(textflag && base.textChange()){
			bool ret;
			text = base.getRawText(ret);
			if(ret){
				base.endTyping(true);
				textflag = false;
				textdone = 1;
				options.at(textfield)->exec(*this);
				textfield = -1;
			}
			else{
				textdone = 0;
				options.at(textfield)->exec(*this);
			}
		}
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
		if(pressed && lastm != textfield && textfield != -1){
			base.endTyping(true);
			textflag = false;
			textdone = 2;
			options.at(textfield)->exec(*this);
			textfield = -1;
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
	MenuButton(Menu& assoc, string name, function<void(MenuControl&)> lambda): MenuControl(name, lambda), assoc(assoc){}
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
class TextBox: public MenuControl{
	bool enable = false;
	string disp;
	char old[1024];
	bool flag = true;
	void action(Menu& m){
		if(!enable){
			done = false;
			enable = true;
			m.getText(text.c_str());
		}
		int res = m.textDone();
		if(res){
			enable = false;
			text = m.textRes();
			if(res == 1)done = true;
		}
		else{
			disp = m.textVisual();
		}
		if(flag || strlen(old) >= strlen(m.textRes()))strcpy(old,  m.textRes());
		else {
			m.ResetText(old);
			disp = m.textVisual();
		}
	}
public:
	bool done = false;
	string text;
	TextBox(string name, function<void(MenuControl&)> lambda): MenuControl(name, lambda){}
	void Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over){
		if(t != ON_CLICK){
			if(!enable)t = IS_UNSET;
			else t = IS_SET;
		}
		rend.applyTexture(getTexture(&typeid(*this), t), x, y, w, h);
		double tw, th;
		string j;
		if(enable)j = name+disp;
		else j = name+text;
		rend.getTextWH(font, j.c_str(), tw, th);
		tw *= hsize / th;
		th = hsize;
		if(tw > w-0.05)flag = false;
		else flag = true;
		RGBA* color;
		if(m_over)color = textcol[0];
		else color = textcol[1];
		rend.displayText(font, j.c_str(), *color, x+w/2.0-tw/2.0, y+h/2.0-th/2.0, tw, th);
	}
};
class CheckBox: public MenuControl{
	void action(Menu& m){
		is_on = !is_on;
	}
public:
	bool is_on;
	CheckBox(string name, bool is_on, function<void(MenuControl&)> lambda): MenuControl(name, lambda), is_on(is_on){}
	void Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over){
		if(t != ON_CLICK){
			if(is_on)t = IS_SET;
			else t = IS_UNSET;
		}
		rend.applyTexture(getTexture(&typeid(*this), t), x, y, w, h);
		double tw, th;
		string msg;
		if(is_on) msg = "on";
		else msg = "off";
		rend.getTextWH(font, (name+msg).c_str(), tw, th);
		tw *= hsize / th;
		th = hsize;
		RGBA* color;
		if(m_over)color = textcol[0];
		else color = textcol[1];
		rend.displayText(font, (name+msg).c_str(), *color, x+w/2.0-tw/2.0, y+h/2.0-th/2.0, tw, th);
	}
};
class ClickBox: public MenuControl{
	void action(Menu& m){
		//stop punching my face!
	}
public:
	ClickBox(string name, function<void(MenuControl&)> lambda): MenuControl(name, lambda){}
	void Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over){
		if(t != ON_CLICK)t = IS_SET;
		rend.applyTexture(getTexture(&typeid(*this), t), x, y, w, h);
		double tw, th;
		string msg;
		rend.getTextWH(font, name.c_str(), tw, th);
		tw *= hsize / th;
		th = hsize;
		RGBA* color;
		if(m_over)color = textcol[0];
		else color = textcol[1];
		rend.displayText(font, name.c_str(), *color, x+w/2.0-tw/2.0, y+h/2.0-th/2.0, tw, th);
	}
};
#endif