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

template <typename T>class GetFunctionArgumentVal;

template <class T, typename U >class GetFunctionArgumentVal<std::function<U(T)>>{
public:
	typedef T arg;
	typedef U returnVal;
};

template <typename FUNCTION, typename BASE>void castAndCall(FUNCTION bf, BASE& temp){
	bf(static_cast< typename GetFunctionArgumentVal<FUNCTION>::arg >(temp));
}

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

	static void addTexture(int ID, const type_info* control, int type);
	const int getTexture(const type_info* control, int type);
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
	template<class T>MenuControl(string name, function<void(T&)> lambda) : name(name), 
		func(bind(castAndCall<decltype(lambda), MenuControl>, lambda, std::placeholders::_1)) 
	{}
	virtual void Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over) = 0;
	virtual ~MenuControl(){};
};

class Menu : private MenuResources{
	function<void()> reset;
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
	double lastmx = 0;
	static bool stop_press;
public:
	Menu(string title, function<void()> reset);
	void setCurr(Menu& m);
	void getText(const char* text);
	const char* textVisual();
	const char* textRes();
	int textDone();
	void ResetText(const char* old);
	Menu& addField(MenuControl& m);
	void Render(Renderer& rend, int font);
	Menu* Check(double mx, double my, bool pressed, bool canpress, Game& base);
	double getLastMx();
};

class MenuButton: public MenuControl{
	void action(Menu& m);
public:
	Menu& assoc;
	MenuButton(Menu& assoc, string name, function<void(MenuButton&)> lambda);
	void Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over);
};
class TextBox: public MenuControl{
	bool enable = false;
	string disp;
	char old[1024];
	bool flag = true;
	void action(Menu& m);
public:
	bool done = false;
	string text;
	TextBox(string name, function<void(TextBox&)> lambda);
	void Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over);
};
class CheckBox: public MenuControl{
	void action(Menu& m);
public:
	bool is_on;
	CheckBox(string name, bool is_on, function<void(CheckBox&)> lambda);
	void Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over);
};
class ClickBox: public MenuControl{
	void action(Menu& m);
public:
	ClickBox(string name, function<void(ClickBox&)> lambda);
	void Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over);
};
class SlideBar : public MenuControl{
	double slidew = 0.01;
	double lastx;
	double slide_len = MenuResources::optionxsize - 2*slidew;
	double slide_part = slide_len / 100.0;
	double slide_disp = slide_len / 101.0;
	double res = 0;
	void action(Menu& m);
public:
	int slide_percent = 0;
	SlideBar(int initial, string name, function<void(SlideBar&)> lambda);
	void Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over);
};
#endif