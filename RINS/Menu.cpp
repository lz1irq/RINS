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