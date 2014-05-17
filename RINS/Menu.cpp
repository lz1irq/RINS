#include "Menu.h"
using namespace std;

map<const type_info*, int[MAXT]> MenuResources::textures;
double MenuResources::optionysize;
double MenuResources::optionspacing;
double MenuResources::optionxsize;
double MenuResources::hsize;
double MenuResources::backxsize;
double MenuResources::backysize;
int MenuResources::background;
int MenuResources::overlay;
int MenuResources::back;
int MenuResources::mframe;
RGBA* MenuResources::textcol[2];

wstring utf8_to_utf16(const string& utf8){
	Uint16* unicode = new Uint16[utf8.size()]();
	int count = 0;
	size_t i = 0;
	while (i < utf8.size()){
		Uint16 uni;
		size_t todo;
		bool error = false;
		unsigned char ch = utf8[i++];
		if (ch <= 0x7F){
			uni = ch;
			todo = 0;
		}
		else if (ch <= 0xBF)throw Error("not a UTF-8 string");
		else if (ch <= 0xDF){
			uni = ch & 0x1F;
			todo = 1;
		}
		else if (ch <= 0xEF){
			uni = ch & 0x0F;
			todo = 2;
		}
		else if (ch <= 0xF7){
			uni = ch & 0x07;
			todo = 3;
		}
		else throw Error("not a UTF-8 string");
		for (size_t j = 0; j < todo; ++j){
			if (i == utf8.size())throw Error("not a UTF-8 string");
			unsigned char ch = utf8[i++];
			if (ch < 0x80 || ch > 0xBF)throw Error("not a UTF-8 string");
			uni <<= 6;
			uni += ch & 0x3F;
		}
		if (uni >= 0xD800 && uni <= 0xDFFF) throw Error("not a UTF-8 string");
		if (uni > 0x10FFFF) throw Error("not a UTF-8 string");
		unicode[count] = uni;
		++count;
	}
	wstring utf16;
	for (size_t i = 0; i < count; ++i){
		Uint16 uni = unicode[i];
		if (uni <= 0xFFFF) utf16 += (wchar_t)uni;
		else{
			uni -= 0x10000;
			utf16 += (wchar_t)((uni >> 10) + 0xD800);
			utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
		}
	}
	return utf16;
}


void MenuResources::addTexture(int ID, const type_info* control, int type){
	textures[control][type] = ID;
}

const int MenuResources::getTexture(const type_info* control, int type){
		return textures[control][type];
}


Menu::Menu(string title, function<void()> reset) : title(title), reset(reset){}
void Menu::setCurr(Menu& m){
	curr = &m;
}

void Menu::getText(const char* text){
	textflag = true;
	textfield = lastm;
	base->startTyping(text);
	textdone = 0;
}

const char* Menu::textVisual(){
	return base->getText();
}

const char* Menu::textRes(){
	return text;
}

int Menu::textDone(){
	return textdone;
}

void Menu::ResetText(const char* old){
	base->endTyping(true);
	base->startTyping(old);
}

Menu& Menu::addField(MenuControl& m){
	options.push_back(&m);
	return *this;
}

void Menu::Render(Renderer& rend, int font){
	double x = 0.5 - optionxsize / 2.0;
	rend.applyTexture(background, 0, 0, 1, 1);
	double tw, th;
	rend.getTextWH(font, title.c_str(), tw, th);
	tw *= hsize / th;
	th = hsize;
	rend.displayText(font, title.c_str(), *textcol[0], x + 0.005, 0.5 - yoffset - 1.2*(optionysize + optionspacing), tw, th);
	rend.applyTexture(mframe, x, 0.5 - yoffset - 1.2*(optionysize + optionspacing), optionxsize,
		(optionysize + optionspacing)*options.size() + optionysize);
	for (int i = 0; i < options.size(); ++i){
		Textures t = MAXT;
		double y = 0.5 + (optionysize + optionspacing)*i - (optionysize + optionspacing) / 2.0 - yoffset;
		double w = optionxsize;
		double h = optionysize;
		bool mover = lastm == i;
		if (menu_select == i)t = ON_CLICK;
		options.at(i)->Render(x, y, w, h, rend, font, t, mover);
		if (mover)rend.applyTexture(overlay, x, y, w, h);
	}
	if (prev){
		rend.applyTexture(back, 0.1, 0.5 - backysize / 2.0, backxsize, backysize);
		if (lastm == -2){
			rend.applyTexture(overlay, 0.1, 0.5 - backysize / 2.0, backxsize, backysize);
		}
	}
}

Menu* Menu::Check(double mx, double my, bool pressed, bool canpress, Game& base){
	this->base = &base;
	if (textflag && base.textChange()){
		bool ret;
		text = base.getRawText(ret);
		if (ret){
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
	yoffset = (options.size() - 1)*(optionysize + optionspacing) / 2.0;
	bool z = false;
	for (int i = 0; i < options.size(); ++i){
		double optofs = (optionysize + optionspacing) / 2.0;
		double optpos = 0.5 + (optionysize + optionspacing)*i;
		if (mx > 0.5 - optionxsize / 2.0 && mx < 0.5 + optionxsize / 2.0
			&& my > optpos - optofs - yoffset && my < optpos + optofs - yoffset - optionspacing){
			lastm = i;
			z = true;
		}
	}
	if (mx > 0.1 && mx < 0.1 + backxsize && my < 0.5 + backysize / 2.0 && my > 0.5 - backysize / 2.0){
		lastm = -2;
		z = true;
	}
	if (!z)lastm = -1;
	if (lastm != -1 && pressed)menu_select = lastm;
	if (menu_select != -1 && canpress){
		if (lastm == menu_select){
			if (lastm > -1)options.at(lastm)->exec(*this);
			else if (prev){
				reset();
				curr = prev;
			}
		}
		menu_select = -1;
	}
	if (pressed && lastm != textfield && textfield != -1){
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

void MenuButton::action(Menu& m){
	assoc.prev = &m;
	m.setCurr(assoc);
}

MenuButton::MenuButton(Menu& assoc, string name, function<void(MenuButton&)> lambda) : MenuControl(name, lambda), assoc(assoc){}

void MenuButton::Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over){
	if (t != ON_CLICK)t = IS_UNSET;
	rend.applyTexture(getTexture(&typeid(*this), t), x, y, w, h);
	double tw, th;
	rend.getTextWH(font, name.c_str(), tw, th);
	tw *= hsize / th;
	th = hsize;
	RGBA* color;
	if (m_over)color = textcol[0];
	else color = textcol[1];
	rend.displayText(font, name.c_str(), *color, x + w / 2.0 - tw / 2.0, y + h / 2.0 - th / 2.0, tw, th);
}


void TextBox::action(Menu& m){
	if (!enable){
		done = false;
		enable = true;
		m.getText(text.c_str());
	}
	int res = m.textDone();
	if (res){
		enable = false;
		text = m.textRes();
		if (res == 1)done = true;
	}
	else{
		disp = m.textVisual();
	}
	if (flag || strlen(old) >= strlen(m.textRes()))strcpy(old, m.textRes());
	else {
		m.ResetText(old);
		disp = m.textVisual();
	}
}

TextBox::TextBox(string name, function<void(TextBox&)> lambda) : MenuControl(name, lambda){}

void TextBox::Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over){
	if (t != ON_CLICK){
		if (!enable)t = IS_UNSET;
		else t = IS_SET;
	}
	rend.applyTexture(getTexture(&typeid(*this), t), x, y, w, h);
	double tw, th;
	string j;
	if (enable)j = name + disp;
	else j = name + text;
	rend.getTextWH(font, j.c_str(), tw, th);
	tw *= hsize / th;
	th = hsize;
	if (tw > w - 0.05)flag = false;
	else flag = true;
	RGBA* color;
	if (m_over)color = textcol[0];
	else color = textcol[1];
	rend.displayText(font, j.c_str(), *color, x + w / 2.0 - tw / 2.0, y + h / 2.0 - th / 2.0, tw, th);
}


void CheckBox::action(Menu& m){
	is_on = !is_on;
}

CheckBox::CheckBox(string name, bool is_on, function<void(CheckBox&)> lambda) : MenuControl(name, lambda), is_on(is_on){}
void CheckBox::Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over){
	if (t != ON_CLICK){
		if (is_on)t = IS_SET;
		else t = IS_UNSET;
	}
	rend.applyTexture(getTexture(&typeid(*this), t), x, y, w, h);
	double tw, th;
	string msg;
	if (is_on) msg = "on";
	else msg = "off";
	rend.getTextWH(font, (name + msg).c_str(), tw, th);
	tw *= hsize / th;
	th = hsize;
	RGBA* color;
	if (m_over)color = textcol[0];
	else color = textcol[1];
	rend.displayText(font, (name + msg).c_str(), *color, x + w / 2.0 - tw / 2.0, y + h / 2.0 - th / 2.0, tw, th);
}


void ClickBox::action(Menu& m){
	//stop punching my face!
}

ClickBox::ClickBox(string name, function<void(ClickBox&)> lambda) : MenuControl(name, lambda){}

void ClickBox::Render(double x, double y, double w, double h, Renderer& rend, int font, Textures t, bool m_over){
	if (t != ON_CLICK)t = IS_SET;
	rend.applyTexture(getTexture(&typeid(*this), t), x, y, w, h);
	double tw, th;
	string msg;
	rend.getTextWH(font, name.c_str(), tw, th);
	tw *= hsize / th;
	th = hsize;
	RGBA* color;
	if (m_over)color = textcol[0];
	else color = textcol[1];
	rend.displayText(font, name.c_str(), *color, x + w / 2.0 - tw / 2.0, y + h / 2.0 - th / 2.0, tw, th);
}
