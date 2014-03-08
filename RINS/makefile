LIBPATH = lib/SDL/
LIBS = -L"$(LIBPATH)" $(LIBPATH)SDL2.dll -lSDL2_mixer -lmingw32 -lSDL2main $(LIBPATH)libfreetype-6.dll -lSDL2_ttf -lSDL2_image -lgdi32  -g3 -m3dnow -mmmx -msse
CXXINCS = -I"include/SDL2"
RM = rm -f

all:
	g++ Platform.cpp Being.cpp main.cpp -o Release/RINS.exe $(LIBS) $(CXXINCS) -std=c++11

clean:
	RM *.exe

rebuild: clean all