#include <iostream>
#include <stdlib.h>
using namespace std;

int main(){
#ifdef WIN32
	cout << "launching RINS -> 640x640" << endl;
	system(".\\Release\\RINS.exe");
#endif
}
