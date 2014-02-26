#include "Graphics.h"
#include <iostream>
using namespace std;
int main(int argc, char** argv) {
	Graphics gui(640,480, "RINS");
	gui.renderScene();
	return 0;
}