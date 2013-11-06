#ifndef GAME_INCLUDED
#define GAME_INCLUDED

#include "main.h"

class Game {
public:
	virtual void update(game_info* gi, float deltatime) = 0;
	virtual void draw(game_info* gi) = 0;
	virtual void drawOrtho(game_info* gi) = 0;

	virtual void keyDown(game_info* gi, Uint32 key)=0;
	virtual void keyUp(game_info* gi, Uint32 key)=0;

	virtual void mouseMovement(game_info* gi, Uint32 x, Uint32 y)=0;
	virtual void mouseDown(game_info* gi)=0;

	virtual void keyRepeat(game_info* gi)=0;

	virtual void attack(game_info* gi, int severity)=0;

	virtual void initGame(game_info* gi)=0;
};
#endif //GAME_INCLUDED
