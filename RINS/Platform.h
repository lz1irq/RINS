#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include "SDL_net.h"
#include <string.h>
#include <list>
#include<iostream>
using namespace std;
#ifndef _GLIBCXX_PLATFORM_H
#define _GLIBCXX_PLATFORM_H

#define MAX_VOL MIX_MAX_VOLUME

class Error{
	const char* err;
public:
	Error(const char* err);
	const char* getError();
};

struct RGBA{
	SDL_Color col;
	RGBA(Uint8 R, Uint8 G, Uint8 B, Uint8 A){
		col.r = R;
		col.g = G;
		col.b = B;
		col.a = A;
	}
	void setR(Uint8 r){ col.r = r; }
	void setG(Uint8 g){ col.g = g; }
	void setB(Uint8 b){ col.b = b; }
	void setA(Uint8 a){ col.a = a; }
};

class Renderer {
	static const int MAX_TEXTURES = 1024;
	SDL_Texture* textures[MAX_TEXTURES];
	static int current_textures;
	SDL_Window* win;
	SDL_Renderer* ren;
	int W, H;
	SDL_Rect part;
	TTF_Font* fonts[MAX_TEXTURES];
	static int current_fonts;
	double rotate;
	void applyTexture(SDL_Texture* t, double x, double y, double width, double height);
public:
	Renderer(int width, int height, const char* title);
	int loadTexture(const char* path);
	void applyTexture(int texture_ID, double x, double y, double width, double heigth);
	void renderScene();
	void renderPart(int xparts, int yparts, int xpartnum, int ypartnum);
	int loadFont(const char* font, int size);
	void displayText(int font, const char* text, RGBA color, double x, double y, double w, double h);
	void getTextWH(int font, const char* text, double& w, double& h);
	void setRotationAngle(double deg);
	void setModulateBlending(int texture_ID);
	~Renderer();
};

class Audio {
	static const int MAX_SONGS = 1024;
	static const int MAX_SOUNDS = 1024;
	Mix_Music*	  songs[MAX_SONGS];
	Mix_Chunk*    sounds[MAX_SOUNDS];
	static int current_sounds;
	static int current_songs;
public:
	Audio();
	int loadSong(const char* song);
	int loadSound(const char* sound);
	void playSong(int song_id);
	void setMusicVolume(int vol);
	void playSound(int sound_id);
	bool isPlayingMusic();
	void pauseMusic();
	void resumeMusic();
	void stopMusic();
	~Audio();
};

class Socket{
public:
	class Client;
private:
	IPaddress ip;
	TCPsocket sd;
	SDLNet_SocketSet socketset;
	int numused = 0;
	list<Client> clients;
	int lenz = 0;
	char command[10240] = { 0 };
public:
	class Client{
		friend class Socket;
		TCPsocket sock;
		const int bf = 10240;
		Client(TCPsocket s): sock(s){
			memset(buf, 0, bf);
			len = 0;
		}
		char buf[10240];
		int len;
	public:
		~Client(){
			SDLNet_TCP_Close(sock);
		}
		bool operator==(const Client& cl) const{
			return cl.sock == sock;
		}
	};
	enum Commands{ KEYBOARD, GETINFO, SERVERINFO, SELF, BULLETZ, ENDBIT, MONSTER };
	Socket();
	void startServer(int port);
	int gatherPlayers();
	void ConnectToServer(int port, const char* ip);
	void disconncet();
	bool updateClients();
	void sendToServer(char* text, int len);
	list<Client>& getClients();
	void sendCommand(short num, short datasz, const char* data);
	char* receiveCommand();
	char* getNextCommand(Client& c);
	bool commandToClient(list<Client>::iterator&, short num, short datasz, const char* data);
	~Socket();
};

class Game{
	SDL_Event event;
	static int secondaryLoop(void* param);
	bool quit;
	bool has_event;
	int mousex, mousey;
	Uint32 buttons;
	char utf8text[1024] = { 0 };
	char raw[1024];
	int maxtext = 1024;
	int cursor = 0;
	bool ret_text = false;
public:
	virtual void mainLoop() = 0;
	virtual void graphicsLoop() = 0;
	unsigned int getTicks();
	char getKey(bool pressed);
	bool isPressed(const char* key);
	double getMouseX();
	double getMouseY();
	bool getLeftClick();
	bool getRightClick();
	void startTyping(const char* initial);
	const char* getText();
	const char* getRawText(bool &ret);
	void endTyping(bool reset);
	Game();
	void loop();
	~Game();
};
#endif