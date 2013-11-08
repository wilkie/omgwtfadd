#ifndef BREAKOUT_INCLUDED
#define BREAKOUT_INCLUDED

#include "game.h"
#include "context.h"

class BreakOut : public Game {
public:
  // conventions:
  void initGame(game_info* gi);

  void update(game_info* gi, float deltatime);
  void draw(Context* context, game_info* gi);
  void drawOrtho(Context* context, game_info* gi);

  void keyDown(game_info* gi, Uint32 key);
  void keyUp(game_info* gi, Uint32 key);
  void keyRepeat(game_info* gi);

  void mouseMovement(game_info* gi, Uint32 x, Uint32 y);
  void mouseDown(game_info* gi);

  void attack(game_info* gi, int severity);

  // ---

  float getLeftBounds(game_info* gi);
  float getRightBounds(game_info* gi);

  void drawBall(Context* context, game_info* gi);
  void moveBall(game_info* gi, float t, int last_type);

  float checkBallAgainst(game_info* gi,
                         float t, float x1, float y1, float x2, float y2);
  bool checkBallAgainstBlock(game_info* gi,
                             float t, float &cur_t, int &type_t,
                             int last_type, float x, float y, int isPaddle);

};
#endif //BREAKOUT_INCLUDED
