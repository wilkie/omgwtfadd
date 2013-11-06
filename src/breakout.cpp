#include "components.h"
#include "breakout.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <vector>

using namespace std;
vector <float> xs;
vector <float> ys;

// conventions:
void BreakOut::initGame(game_info* gi) {
  gi->fine = 2.5;

  gi->ball_x = 0;
  gi->ball_y = 0;

  gi->ball_dx = (BREAKOUT_BALL_SPEED_X + (LEVEL * 0.2));
  gi->ball_dy = (BREAKOUT_BALL_SPEED_Y + (LEVEL * 0.2));

  gi->break_out_time = BREAK_OUT_SECONDS;

  gi->break_out_consecutives = 0;
}

float BreakOut::checkBallAgainst(game_info* gi, float t, float x1, float y1, float x2, float y2) {
  // OK !!!
  // COLLISION DETECTION
  // RAYBASED?!

  float n_t;

  float b_x;
  float b_y;

#define SPHERE 0.125

  float sp;

  if (x1 == x2) {
    // VERTICAL LINE

    if (gi->ball_dx > 0) {
      sp = -SPHERE;
    }
    else {
      sp = SPHERE;
    }

    n_t = (x1 - (gi->ball_x + sp)) / gi->ball_dx;

    b_y = (gi->ball_y + sp) + (n_t * gi->ball_dy);

    if ((b_y <= y1) && (b_y >= y2)){
      //printf("%f %f %f %f %f\n", n_t, t, b_y, y1,y2);

      if ((n_t < t) && (n_t >= 0)) {
        return n_t;
      }
    }
    /*
       if ((b_y < y1) && (b_y > y2) && (n_t < t) && (n_t >= 0))
       {
    // YEP!
    return n_t;
    }*/
  }
  else if (y1 == y2) {
    // HORIZONTAL LINE

    if (gi->ball_dy < 0) {
      sp = -SPHERE;
    }
    else {
      sp = SPHERE;
    }

    n_t = (y1 - (gi->ball_y + sp)) / gi->ball_dy;

    b_x = (gi->ball_x + sp) + (n_t * gi->ball_dx);

    if (b_x > x1 && b_x < x2 && (n_t < t) && (n_t >= 0)) {
      // YEP!
      return n_t;
    }
  }
  else {
    // GENERAL

    // do we have any???
    // no?
    // no!
    // YAY!
    printf("collision detection error... i'm lazy\n");
  }

  return 1001.0;
}

bool BreakOut::checkBallAgainstBlock(game_info* gi, float t, float &cur_t, int &type_t, int last_type, float x, float y, int isPaddle) {
  float chk;

  // p = t * d
  // general line equation
  // to be solved against these easy horizontal and vertical lines

  if (isPaddle) {
    y += 1.5;
  }

  // check left edge
  chk = checkBallAgainst(gi, t, (float)(x - 0.5), (float)(y - 0.25), (float)(x - 0.5), (float)(y-0.5) - 0.25);
  if (chk <= cur_t && !((1 << (3 + (isPaddle * 4))) & last_type) && gi->ball_dx > 0) {
    if (isPaddle) {
      type_t |= 128;
    }
    else {
      type_t |= 8;
    }
  }

  if (chk <= cur_t) {
    cur_t = chk;
  }

  // check top edge
  chk = checkBallAgainst(gi, t, (float)(x - 0.5), (float)(y - 0.25), (float)(x + 0.45), (float)(y - 0.25));
  if (chk <= cur_t && !((1 << (4 + (isPaddle * 4))) & last_type) && gi->ball_dy < 0) {
    //type_t |= 1 << (4 + (isPaddle * 4));

    if (isPaddle) {
      type_t |= 256;
    }
    else {
      type_t |= 16;
    }
  }

  if (chk <= cur_t) {
    cur_t = chk;
  }

  // check right edge
  chk = checkBallAgainst(gi, t, (float)(x + 0.45), (float)(y - 0.25), (float)(x + 0.45), (float)(y-0.5) - 0.25);
  if (chk <= cur_t && !((1 << (5 + (isPaddle * 4))) & last_type) && gi->ball_dx < 0) {
    //type_t |= 1 << (5 + (isPaddle * 4));

    if (isPaddle) {
      type_t |= 512;
    }
    else {
      type_t |= 32;
    }
  }

  if (chk <= cur_t) {
    cur_t = chk;
  }

  // check bottom edge
  chk = checkBallAgainst(gi, t, (float)(x - 0.5), (float)(y-0.5) - 0.25, (float)(x + 0.45), (float)(y-0.5) - 0.25);
  if (chk <= cur_t && !((1 << (6 + (isPaddle * 4))) & last_type) && gi->ball_dy > 0) {
    //type_t |= 1 << (6 + (isPaddle * 4));

    if (isPaddle) {
      type_t |= 1024;
    }
    else {
      type_t |= 64;
    }
  }

  if (chk <= cur_t) {
    cur_t = chk;
  }

  if (isPaddle) {
    if (type_t & 0x780) {
      return true;
    }
  }
  else {
    if (type_t & 0x78) {
      return true;
    }
  }

  return false;
}

void BreakOut::moveBall(game_info* gi, float t, int last_type) {
  //printf("moveball start! %f %d\n", t, last_type);

  float cur_t = 1000.0;
  int type_t = 0;
  int board_i = -1;
  int board_j = -1;

  float up_t = checkBallAgainst(gi, t, -5, 11.75, 10, 11.75);
  float left_t = checkBallAgainst(gi, t, 0, 20, 0, -5);
  float right_t = checkBallAgainst(gi, t, 4.5, 20, 4.5, -5);

  // check against borders

  if (gi->ball_dy > 0 && up_t <= cur_t && !(1 & last_type)) {
    cur_t = up_t;
    type_t |= 1;
  }

  if (gi->ball_dx > 0 && right_t <= cur_t && !(2 & last_type)) {
    cur_t = right_t;
    type_t |= 2;
  }

  if (gi->ball_dx < 0 && left_t <= cur_t && !(4 & last_type)) {
    cur_t = left_t;
    type_t |= 4;
  }

  int i,j;

  // check against the blocks

  for (i=0; i<10; i++) {
    for (j=0; j<24; j++) {
      if (gi->board[i][j] != -1) {
        if (checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, (float)(i * 0.5f), (float)(j+1) * 0.5f, 0)) {
          board_i = i;
          board_j = j;

          j = 24;
          i = 10;
        }
      }
    }
  }

  // collision against paddle

  // for all of the blocks that make up the paddle... check against their edges

  float x,y;

  x = gi->fine;
  y = 0;

  switch (gi->curpiece) {
    case 0:
      if (gi->curdir % 2) {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-1.0, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y, 1);
      }
      else {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y+0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y+1.0, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y-0.5, 1);
      }
      break;
    case 1:
      if (gi->curdir % 2) {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y+0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y-0.5, 1);
      }
      else {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y-0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y-0.5, 1);
      }
      break;
    case 2:
      if (gi->curdir % 2) {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y+0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y-0.5, 1);
      }
      else {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y-0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y-0.5, 1);
      }
      break;
    case 3:
      checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y, 1);
      checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y+0.5, 1);
      checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
      checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y+0.5, 1);
      break;
    case 4:
      if (gi->curdir == 0) {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y+0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y-0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y+0.5, 1);
      }
      else if (gi->curdir == 1) {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y+0.5, 1);
      }
      else if (gi->curdir == 2) {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y+0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y-0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y-0.5, 1);
      }
      else {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y-0.5, 1);
      }
      break;
    case 5:
      if (gi->curdir == 0) {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y+0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y-0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y+0.5, 1);
      }
      else if (gi->curdir == 1) {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y-0.5, 1);
      }
      else if (gi->curdir == 2) {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y+0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y-0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y-0.5, 1);
      }
      else {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y+0.5, 1);
      }
      break;
    case 6:
      if (gi->curdir == 0) {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y-0.5, 1);
      }
      else if (gi->curdir == 1) {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y+0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y-0.5, 1);
      }
      else if (gi->curdir == 2) {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x+0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y+0.5, 1);
      }
      else {
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x-0.5, y, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y+0.5, 1);
        checkBallAgainstBlock(gi, t, cur_t, type_t, last_type, x, y-0.5, 1);
      }
      break;
  }

  // adjust ball, call again if required

  // no collisions?
  if (type_t == 0) {
    // use up all t!
    gi->ball_x += t * gi->ball_dx;
    gi->ball_y += t * gi->ball_dy;
    return;
  }

  // we have a collision, move as far as we can
  gi->ball_x += cur_t * gi->ball_dx;
  gi->ball_y += cur_t * gi->ball_dy;

  //xs.push_back(gi->ball_x);
  //ys.push_back(gi->ball_y);

  engine.audio.playSound(SND_BOUNCE);

  // then, change direction, and move the rest of the way

  t -= cur_t;

  if (type_t & ~0x7) {
    gi->break_out_consecutives++;
  }

  if (type_t & 1) { // top
    gi->ball_dy = -gi->ball_dy;
  }
  if (type_t & 2) { // right
    gi->ball_dx = -gi->ball_dx;
  }
  if (type_t & 4) { // left
    gi->ball_dx = -gi->ball_dx;
  }

  if (type_t & 8) { // left side block
    gi->ball_dx = -gi->ball_dx;
    //gi->ball_dy = 0;

    // get rid of block???
    gi->board[board_i][board_j] = -1;

    engine.passMessage(MSG_REMOVEBLOCK, board_i, board_j, 0);

    gi->score += (2 * 100);
    engine.passMessage(MSG_APPENDSCORE, 100, 2, 0);
  }
  if (type_t & 16) { // top side block
    gi->ball_dy = -gi->ball_dy;
    //gi->ball_dx = 0; //-gi->ball_dx;
    //gi->ball_dy = 0;

    // get rid of block???
    gi->board[board_i][board_j] = -1;

    engine.passMessage(MSG_REMOVEBLOCK, board_i, board_j, 0);

    gi->score += (2 * 100);
    engine.passMessage(MSG_APPENDSCORE, 100, 2, 0);
  }
  if (type_t & 32) { // right side block
    gi->ball_dx = -gi->ball_dx;
    //gi->ball_dx = -gi->ball_dx;
    //gi->ball_dy = 0;

    // get rid of block???
    gi->board[board_i][board_j] = -1;

    engine.passMessage(MSG_REMOVEBLOCK, board_i, board_j, 0);

    gi->score += (2 * 100);
    engine.passMessage(MSG_APPENDSCORE, 100, 2, 0);
  }
  if (type_t & 64) { // bottom side block
    gi->ball_dy = -gi->ball_dy;

    // get rid of block???
    gi->board[board_i][board_j] = -1;

    engine.passMessage(MSG_REMOVEBLOCK, board_i, board_j, 0);

    gi->score += (2 * 100);
    engine.passMessage(MSG_APPENDSCORE, 100, 2, 0);
  }


  // paddle
  if (type_t & 128) {
    if (gi->break_out_consecutives >= 7) {
      engine.sendAttack(3);
    }
    else if (gi->break_out_consecutives >= 5) {
      engine.sendAttack(2);
    }
    else if (gi->break_out_consecutives >= 4) {
      engine.sendAttack(1);
    }

    gi->break_out_consecutives = 0;

    gi->ball_dx = -gi->ball_dx;
  }

  if (type_t & 256) {
    if (gi->break_out_consecutives >= 7) {
      engine.sendAttack(3);
    }
    else if (gi->break_out_consecutives >= 5) {
      engine.sendAttack(2);
    }
    else if (gi->break_out_consecutives >= 4) {
      engine.sendAttack(1);
    }

    gi->break_out_consecutives = 0;

    gi->ball_dy = -gi->ball_dy;
  }

  if (type_t & 512) {
    if (gi->break_out_consecutives >= 7) {
      engine.sendAttack(3);
    }
    else if (gi->break_out_consecutives >= 5) {
      engine.sendAttack(2);
    }
    else if (gi->break_out_consecutives >= 4) {
      engine.sendAttack(1);
    }

    gi->break_out_consecutives = 0;

    gi->ball_dx = -gi->ball_dx;
  }

  if (type_t & 1024) {
    if (gi->break_out_consecutives >= 7) {
      engine.sendAttack(3);
    }
    else if (gi->break_out_consecutives >= 5) {
      engine.sendAttack(2);
    }
    else if (gi->break_out_consecutives >= 4) {
      engine.sendAttack(1);
    }

    gi->break_out_consecutives = 0;

    gi->ball_dy = -gi->ball_dy;
  }

  //printf("moveball? %f %d\n", t, type_t);

  //SDL_Delay(3000);
  // call this again
  moveBall(gi, t, type_t);
}

void BreakOut::update(game_info* gi, float deltatime) {
  bool move = false;

  if (gi->state == STATE_BREAKOUT_TRANS) {
    gi->rot2 -= TRANSITION_SPEED * deltatime;

    if (gi->rot2 <= 0) {
      gi->rot2 = 0;
      engine.changeState(gi, STATE_TETRIS);
    }

    engine.passMessage(MSG_ROT_BOARD2, (gi->rot2 / 180.0f) * 255.0f, 0,0);
    return;
  }

  if (engine.keys[SDLK_LEFT]) {
    gi->fine -= BREAKOUT_PADDLE_SPEED * deltatime;

    float amt = getLeftBounds(gi);

    if (gi->fine < amt) {
      gi->fine = amt;
    }

    move = true;
  }

  if (engine.keys[SDLK_RIGHT]) {
    gi->fine += BREAKOUT_PADDLE_SPEED * deltatime;

    float amt = getRightBounds(gi);

    if (gi->fine > amt) {
      gi->fine = amt;
    }

    move = true;
  }

  gi->break_out_time -= deltatime;

  if (gi->break_out_time <= 0) {
    gi->break_out_time = 0;

    engine.displayMessage(STR_YOUSURVIVED);
    engine.audio.playSound(SND_CHANGEVIEW);
    engine.changeState(gi, STATE_BREAKOUT_TRANS);

    gi->pos = (int)(gi->fine / 0.5f);
    gi->fine = 0;
    return;
  }

  if (gi->ball_fast > 0) {
    gi->ball_fast -= deltatime;

    if (gi->ball_fast < 0) {
      gi->ball_fast = 0;
      if (gi->ball_dx < 0) {
        //gi->ball_dx = -BREAKOUT_BALL_SPEED_X;
      }
      else {
        //gi->ball_dx = BREAKOUT_BALL_SPEED_X;
      }
      if (gi->ball_dy < 0) {
        //gi->ball_dy = -BREAKOUT_BALL_SPEED_Y;
      }
      else {
        //gi->ball_dy = BREAKOUT_BALL_SPEED_Y;
      }
    }
  }

  if (move) {
    engine.passMessage(MSG_UPDATEPADDLE, (unsigned char)((gi->fine / 11.0f) * 255.0f), 0, 0);
  }

  // move ball

  // solve for all collisions!

  // wall collisions!

  if (!engine.network_thread) {
    if (gi->ball_dx < 0) {
      //gi->ball_dx = -(BREAKOUT_BALL_SPEED_X + (LEVEL * 0.2));
    }
    else {
      //gi->ball_dx = (BREAKOUT_BALL_SPEED_X + (LEVEL * 0.2));
    }
    if (gi->ball_dy < 0) {
      //gi->ball_dy = -(BREAKOUT_BALL_SPEED_Y + (LEVEL * 0.2));
    }
    else {
      //gi->ball_dy = (BREAKOUT_BALL_SPEED_Y + (LEVEL * 0.2));
    }
  }

  moveBall(gi,deltatime,0);

  engine.passMessage(MSG_UPDATEBALL, ((float)(gi->ball_x) / 20.0f) * 255.0f, ((float)(gi->ball_y) / 20.0f) * 255.0f, 0);


  if (gi->ball_y < -1.5) {
    engine.tetris.attack(gi, 1);
  }
}

void BreakOut::drawBall(game_info* gi) {
  // translate
  glm::mat4 model;

  model = glm::mat4(1.0f);
//  model = glm::translate(model, glm::vec3(gi->side * 5.0f, 0.0f, 0.0f));

  // rotate
  model = glm::rotate(model, gi->side * gi->rot, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, -gi->rot2, glm::vec3(1.0f,0.0f,0.0f));

  model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));

  glm::mat4 base = model;

  model = glm::translate(model, glm::vec3(-2.25f + (gi->ball_x), 6.375f - (gi->ball_y), 0.0f));
  model = glm::scale(model, glm::vec3(0.125f, 0.125f, 0.125f));

  glUniformMatrix4fv(engine._model_uniform, 1, GL_FALSE, &model[0][0]);
  engine.drawCube();
}

void BreakOut::draw(game_info* gi) {
  engine.tetris.drawBoard(gi);

  if (gi->state == STATE_BREAKOUT_TRANS) {
    engine.tetris.drawPiece(gi, (0.5) * (double)gi->pos, gi->fine, gi->curpiece);
  }
  else {
    engine.tetris.drawPiece(gi, gi->fine, 1.0f, gi->curpiece);
  }

  drawBall(gi);

  unsigned int i;

  float aspect = (600.0f/800.0f);

  for (i=0; i<xs.size(); i++) {
    glPushMatrix();

    float x, y;

    x = xs[i];
    y = ys[i];

    // translate to world
    glTranslatef(gi->side * 3.75f,0,0);

    // rotate
    glRotatef(gi->side * gi->rot,0,1,0);
    glRotatef(-gi->rot2,1,0,0);

    // translate
    glTranslatef(-2.25 + (x), 5.875 - (y),0);

    // scale
    glScalef(aspect,1,1);

    //glutSolidSphere(SPHERE_SIZE,30,30);

    glPopMatrix();
  }
}

void BreakOut::drawOrtho(game_info* gi) {
  if (gi->side == -1) {
    engine.drawString(":CONS", 0,(400.0f - 32.0f) - (6 * 16),25.0f);

    int x  = engine.intLength(gi->break_out_consecutives);

    engine.drawInt(gi->break_out_consecutives, 1, (400.0f - 32.0f) - ((6 + x) * 16), 25.0f);

    int x2;

    x2 = (400 - (14 * 16)) / 2;

    engine.drawString("SURVIVE:", 2, x2, 600.0f - 25.0f);
    x2 += (8 * 16);
    engine.drawInt(gi->break_out_time, 3, x2, 600.0f - 25.0f);
    x2 += (engine.intLength(gi->break_out_time + 0.5) * 16);
    engine.drawString(" SEC", 2, x2, 600.0f - 25.0f);
  }
}

void BreakOut::keyRepeat(game_info* gi) {
  if (engine.keys[SDLK_UP]) {
    gi->curdir++;
    gi->curdir %= 4;

    double amt = getRightBounds(gi);

    if (gi->fine > amt) {
      gi->curdir--;
      gi->curdir %= 4;
    }

    amt = getLeftBounds(gi);

    if (gi->fine < amt) {
      gi->curdir--;
      gi->curdir %= 4;
    }

    engine.passMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
  }
}

void BreakOut::keyDown(game_info* gi, Uint32 key) {
}

void BreakOut::keyUp(game_info* gi, Uint32 key) {
}

void BreakOut::mouseDown(game_info* gi) {
}

void BreakOut::mouseMovement(game_info* gi, Uint32 x, Uint32 y) {
}

float BreakOut::getLeftBounds(game_info* gi) {
  int sx;
  sx = (int)(gi->fine / 0.5);

  switch (gi->curpiece) {
    case 0:
      if (gi->curdir % 2) {
        if (sx < 2) { return 1.0; }
      }
      else {
        if (sx < 0) { return 0; }
      }
      break;
    case 1:
      if (gi->curdir % 2) {
        if (sx < 0) { return 0; }
      }
      else {
        if (sx < 1) { return 0.5; }
      }
      break;
    case 2:
      if (sx < 1) { return 0.5; }
      break;
    case 3:
      if (sx < 0) { return 0; }
      break;
    case 4:
      if (gi->curdir == 0) {
        if (sx < 0) { return 0; }
      }
      else {
        if (sx < 1) { return 0.5; }
      }
      break;
    case 5:
      if (gi->curdir == 2) {
        if (sx < 0) { return 0; }
      }
      else {
        if (sx < 1) { return 0.5; }
      }
      break;
    case 6:
      if (gi->curdir == 1) {
        return 0;
      }
      else {
        return 0.5;
      }
      break;
  }

  return 0;
}

float BreakOut::getRightBounds(game_info* gi) {
  int sx;
  sx = (int)(gi->fine / 0.5);

  switch (gi->curpiece) {
    case 0:
      if (gi->curdir % 2) {
        return 4.0;
      }
      else {
        return 4.5;
      }
      break;
    case 1:
      return 4.0;
      break;
    case 2:
      if (gi->curdir % 2) {
        return 4.5;
      }
      else {
        return 4.0;
      }
      break;
    case 3:
      return 4.0;
      break;
    case 4:
      if (gi->curdir == 2) {
        return 4.5;
      }
      else {
        return 4.0;
      }
      break;
    case 5:
      if (gi->curdir == 0) {
        return 4.5;
      }
      else {
        return 4.0;
      }
      break;
    case 6:
      if (gi->curdir == 3) {
        return 4.5;
      }
      else {
        return 4.0;
      }
      break;
  }

  return 10;
}

void BreakOut::attack(game_info* gi, int severity) {
  if (severity == 1) {
    int pos = gi->pos;
    float fine = gi->fine;

    engine.tetris.getNewPiece(gi);

    gi->pos = pos;
    gi->fine = fine;

    float amt = getLeftBounds(gi);

    if (gi->fine < amt) {
      gi->fine = amt;
    }

    amt = getRightBounds(gi);

    if (gi->fine > amt) {
      gi->fine = amt;
    }

    engine.passMessage(MSG_UPDATEPADDLE, (unsigned char)((gi->fine / 11.0f) * 255.0f), 0, 0);
  }
  else if (severity == 2) {
    engine.tetris.attack(gi, 1);
  }
  else if (severity == 3) {
    gi->ball_fast = 7;
    if (gi->ball_dx < 0) {
      gi->ball_dx = -BREAKOUT_BALL_SPEEDY_X;
    }
    else {
      gi->ball_dx = BREAKOUT_BALL_SPEEDY_X;
    }
    if (gi->ball_dy < 0) {
      gi->ball_dy = -BREAKOUT_BALL_SPEEDY_Y;
    }
    else {
      gi->ball_dy = BREAKOUT_BALL_SPEEDY_Y;
    }
  }
}
