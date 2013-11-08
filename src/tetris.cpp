#include "main.h"
#include "tetris.h"
#include "components.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define GAMEOVER_SPREAD_RATE 0.1
#define GAMEOVER_VELOCITY    3.0

static void gl_check_errors(const char* msg) {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    const char* errorString;
    switch ( error ) {
      case GL_INVALID_ENUM: errorString = "invalid enumerant"; break;
      case GL_INVALID_VALUE: errorString = "invalid value"; break;
      case GL_INVALID_OPERATION: errorString = "invalid operation"; break;
      case GL_STACK_OVERFLOW: errorString = "stack overflow"; break;
      case GL_STACK_UNDERFLOW: errorString = "stack underflow"; break;
      case GL_OUT_OF_MEMORY: errorString = "out of memory"; break;
      case GL_TABLE_TOO_LARGE: errorString = "table too large"; break;
      case GL_INVALID_FRAMEBUFFER_OPERATION: errorString = "invalid framebuffer operation"; break;
      default: errorString = "unknown GL error"; break;
    }
    fprintf(stderr, "GL Error: %s: %s\n", msg, errorString);
  }
}

void Tetris::update(game_info* gi, float deltatime) {
  if (gi->state == STATE_GAMEOVER) {
    // Shoot out the blocks
    gi->gameover_position += GAMEOVER_VELOCITY * deltatime;

    return;
  }

  if (gi->state == STATE_TETRIS_TRANS) {
    gi->rot2 += TRANSITION_SPEED * deltatime;

    if (gi->rot2 >= 180) {
      gi->rot2 = 180;
      engine.changeState(gi, STATE_BREAKOUT);
    }

    engine.passMessage(MSG_ROT_BOARD2, (gi->rot2 / 180.0f) * 255.0f, 0,0);
    return;
  }

  if (gi->attacking) {
    gi->rot += TETRIS_ATTACK_ROT_SPEED * deltatime;
    gi->attack_rot += TETRIS_ATTACK_ROT_SPEED * deltatime;

    if (gi->attack_rot >= 360) {
      gi->rot = -BOARD_NORMAL_ROT;
      gi->attack_rot = 0;

      gi->attacking = 0;

      gi->score += (10 * 100);
      engine.passMessage(MSG_APPENDSCORE, 100, 10, 0);
    }

    engine.passMessage(MSG_ROT_BOARD, (gi->rot / 360.0f) * 255.0f, 0,0);
  }

  if (!engine.network_thread) {
    if (engine.keys[SDLK_DOWN]) {
      gi->fine += deltatime * (TETRIS_SPEED + 4.3 + 0.3 * LEVEL);
    }
    else {
      gi->fine += deltatime * (TETRIS_SPEED + 0.3 * LEVEL);
    }
  }
  else {
    if (engine.keys[SDLK_DOWN]) {
      gi->fine += deltatime * (TETRIS_SPEED + 4.3);
    }
    else {
      gi->fine += deltatime * TETRIS_SPEED;
    }
  }

  //gi->rot2+=50.0 * deltatime;

  if (testCollision(gi)) {
    // we collided! oh no!
    if (testGameOver(gi)) {
      engine.gameOver();
    }
    else {
      addPiece(gi);
    }
  }

  engine.passMessage(MSG_UPDATEPIECEY, (unsigned char)((gi->fine / 11.0f) * 255.0f), 0, 0);
}

void Tetris::dropLine(game_info* gi, int lineIndex) {
  int j,i;
  for (i=0; i<10; i++) {
    for (j=lineIndex; j>1;j--) {
      gi->board[i][j] = gi->board[i][j-1];
    }
  }

  if (gi->side == -1) {
    engine.passMessage(MSG_DROPLINE, (unsigned char)lineIndex, 0,0);
  }
}

int Tetris::clearLines(game_info* gi) {
  // check each row

  int i,j;

  int lines = 0;

  for (j=0;j<24;j++) {
    for (i=0; i<10; i++) {
      if (gi->board[i][j] == -1) {
        break;
      }
    }
    if (i==10) {
      // this line needs to be cleared!

      // move everything above it down
      lines++;
      gi->score += (lines * 100);
      engine.passMessage(MSG_APPENDSCORE, 100, lines, 0);
      dropLine(gi,j);

      engine.audio.playSound(SND_TINK);
    }
  }

  return lines;
}

// init!
void Tetris::initGame(game_info* gi) {
}

void Tetris::dropPiece(game_info* gi) {
  gi->fine = determineDropPosition(gi);

  engine.passMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
  engine.passMessage(MSG_UPDATEPIECEY, (unsigned char)((gi->fine / 11.0f) * 255.0f), 0, 0);

  addPiece(gi);
}

float Tetris::determineDropPosition(game_info* gi) {
  float phantom_fine = gi->fine;
  while(!testCollision(gi, (0.5) * (double)gi->pos, phantom_fine)) {
    phantom_fine+=0.25;
  }

  return phantom_fine - fmod(phantom_fine, 0.5f);
}

// draw 3D
void Tetris::draw(Context* context, game_info* gi) {
  if (!engine.network_thread && gi->side == 1) {
    return;
  }

  if (gi->state == STATE_GAMEOVER) {
    drawBoard(context, gi);
  }
  else {
    drawBoard(context, gi);

    drawPiece(context, gi, (0.5) * (double)gi->pos, gi->fine, gi->curpiece);

    if (gi->state == STATE_TETRIS) {
      drawPiece(context, gi, (0.5) * (double)gi->pos, determineDropPosition(gi), 18);
    }
  }
}

void Tetris::drawBlock(Context* context,
                       int type, game_info* gi, double x, double y, bool hasLeft   = true,
                                                                    bool hasRight  = true,
                                                                    bool hasTop    = true,
                                                                    bool hasBottom = true) {
  engine.useTexture(type);

  // translate to world
  glm::mat4 model = glm::mat4(1.0f);

  // rotate
  model = glm::rotate(model, gi->side * gi->rot, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, -gi->rot2, glm::vec3(1.0f,0.0f,0.0f));

  model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));

  // translate
  model = glm::translate(model, glm::vec3(-2.25f + (x), 6.325 - (y), 0.0f));

  // scale (make them 0.5 unit cubes, since our unit cube is 2x2x2)
  model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));

  if (gi->rot2 > 90) {
    engine.drawQuad(model, 5); // back
  }
  else {
    engine.drawQuad(model, 0); // front
  }
  if (hasLeft) {
    engine.drawQuad(model, 3); // left
  }
  if (hasTop) {
    engine.drawQuad(model, 2); // top
  }
  if (hasBottom) {
    engine.drawQuad(model, 4); // bottom
  }
  if (hasRight) {
    engine.drawQuad(model, 1); // right
  }
}

// draw interface
void Tetris::drawOrtho(Context* context, game_info* gi) {
}

bool Tetris::testGameOver(game_info* gi) {
  int starty;

  starty = (gi->fine - 1.0f) / (0.5);

  switch (gi->curpiece) {
    case 0:
      if (gi->curdir % 2) {
        if (starty < 0)	{ return 1; }
      }
      else {
        if (starty < 1) { return 1; }
      }
      break;
    case 1:
      if (gi->curdir % 2) {
        if (starty < 1) { return 1; }
      }
      else {
        if (starty < 0) { return 1; }
      }
      break;
    case 2:
      if (gi->curdir % 2) {
        if (starty < 1) { return 1; }
      }
      else {
        if (starty < 0) { return 1; }
      }
      break;
    case 3:
      if (starty < 0) { return 1; }
      break;
    case 4:
      if (gi->curdir == 1) {
        if (starty < 0) { return 1; }
      }
      else {
        if (starty < 1) { return 1; }
      }
      break;
    case 5:
      if (gi->curdir == 3) {
        if (starty < 0) { return 1; }
      }
      else {
        if (starty < 1) { return 1; }
      }
      break;
  }

  return 0;
}

void Tetris::addPiece(game_info* gi) {
  int start_y;
  int start_x =  gi->pos;

  start_y = (gi->fine) / (0.5);

  addPiece(gi, start_x, start_y);
}

void Tetris::addPiece(game_info* gi, int start_x, int start_y) {
  switch (gi->curpiece) {
    case 0:
      if (gi->curdir % 2) {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x-1, start_y, gi->curpiece);
        addBlock(gi, start_x-2, start_y, gi->curpiece);
        addBlock(gi, start_x+1, start_y, gi->curpiece);
      }
      else {
        addBlock(gi, start_x, start_y+1, gi->curpiece);
        addBlock(gi, start_x, start_y+2, gi->curpiece);
        addBlock(gi, start_x, start_y-1, gi->curpiece);
        addBlock(gi, start_x, start_y, gi->curpiece);
      }
      break;
    case 1:
      if (gi->curdir % 2) {
        addBlock(gi, start_x+1, start_y, gi->curpiece);
        addBlock(gi, start_x+1, start_y-1, gi->curpiece);
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y+1, gi->curpiece);
      }
      else {
        addBlock(gi, start_x+1, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y-1, gi->curpiece);
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x-1, start_y-1, gi->curpiece);
      }
      break;
    case 2:
      if (gi->curdir % 2) {
        addBlock(gi, start_x-1, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y+1, gi->curpiece);
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x-1, start_y-1, gi->curpiece);
      }
      else {
        addBlock(gi, start_x-1, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y-1, gi->curpiece);
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x+1, start_y-1, gi->curpiece);
      }
      break;
    case 3:
      addBlock(gi, start_x+1, start_y, gi->curpiece);
      addBlock(gi, start_x, start_y+1, gi->curpiece);
      addBlock(gi, start_x, start_y, gi->curpiece);
      addBlock(gi, start_x+1, start_y+1, gi->curpiece);
      break;
    case 4:
      if (gi->curdir == 0) {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y+1, gi->curpiece);
        addBlock(gi, start_x, start_y-1, gi->curpiece);
        addBlock(gi, start_x+1, start_y+1, gi->curpiece);
      }
      else if (gi->curdir == 1) {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x+1, start_y, gi->curpiece);
        addBlock(gi, start_x-1, start_y, gi->curpiece);
        addBlock(gi, start_x-1, start_y+1, gi->curpiece);
      }
      else if (gi->curdir == 2) {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y+1, gi->curpiece);
        addBlock(gi, start_x, start_y-1, gi->curpiece);
        addBlock(gi, start_x-1, start_y-1, gi->curpiece);
      }
      else {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x+1, start_y, gi->curpiece);
        addBlock(gi, start_x-1, start_y, gi->curpiece);
        addBlock(gi, start_x+1, start_y-1, gi->curpiece);
      }
      break;
    case 5:
      if (gi->curdir == 0) {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y+1, gi->curpiece);
        addBlock(gi, start_x, start_y-1, gi->curpiece);
        addBlock(gi, start_x-1, start_y+1, gi->curpiece);
      }
      else if (gi->curdir == 1) {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x+1, start_y, gi->curpiece);
        addBlock(gi, start_x-1, start_y, gi->curpiece);
        addBlock(gi, start_x-1, start_y-1, gi->curpiece);
      }
      else if (gi->curdir == 2) {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y+1, gi->curpiece);
        addBlock(gi, start_x, start_y-1, gi->curpiece);
        addBlock(gi, start_x+1, start_y-1, gi->curpiece);
      }
      else {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x+1, start_y, gi->curpiece);
        addBlock(gi, start_x-1, start_y, gi->curpiece);
        addBlock(gi, start_x+1, start_y+1, gi->curpiece);
      }
      break;
    case 6:
      if (gi->curdir == 0) {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y-1, gi->curpiece);
        addBlock(gi, start_x+1, start_y, gi->curpiece);
        addBlock(gi, start_x-1, start_y, gi->curpiece);
      }
      else if (gi->curdir == 1) {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y+1, gi->curpiece);
        addBlock(gi, start_x+1, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y-1, gi->curpiece);
      }
      else if (gi->curdir == 2) {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x-1, start_y, gi->curpiece);
        addBlock(gi, start_x+1, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y+1, gi->curpiece);
      }
      else {
        addBlock(gi, start_x, start_y, gi->curpiece);
        addBlock(gi, start_x-1, start_y, gi->curpiece);
        addBlock(gi, start_x, start_y-1, gi->curpiece);
        addBlock(gi, start_x, start_y+1, gi->curpiece);
      }
      break;

  }

  if (gi->side == -1) {
    engine.passMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
    //engine.passMessage(MSG_UPDATEPIECEY, , 0, 0);
    engine.passMessage(MSG_ADDPIECE, start_x, start_y,0);

    int lines = clearLines(gi);

    gi->state_lines += lines;
    gi->total_lines += lines;

    getNewPiece(gi);

    if (lines > 1) {
      engine.sendAttack(lines-1);
    }

    if (gi->state_lines >= TETRIS_LINES_NEEDED) {
      gi->state_lines = 0;

      engine.displayMessage(STR_TRANSITION);
      engine.audio.playSound(SND_CHANGEVIEW);
      engine.changeState(gi, STATE_TETRIS_TRANS);
    }
  }
}

void Tetris::addBlock(game_info* gi, int i, int j, int type) {
  gi->board[i][j] = type;
}

void Tetris::drawBoard(Context* context, game_info* gi) {
  engine.useTexture(16);

  // left
  glm::mat4 model;

  model = glm::mat4(1.0f);
//  model = glm::translate(model, glm::vec3(gi->side * 5.0f, 0.0f, 0.0f));

  // rotate
  model = glm::rotate(model, gi->side * gi->rot, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, -gi->rot2, glm::vec3(1.0f,0.0f,0.0f));

  model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));

  glm::mat4 base = model;

  model = glm::translate(model, glm::vec3(-2.625,0.125,0));
  model = glm::scale(model, glm::vec3(0.25,11.5,0.5));
  model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

  engine.drawCube(model);

  // right
  model = base;
  model = glm::translate(model, glm::vec3(2.625,0.125,0));
  model = glm::scale(model, glm::vec3(0.25,11.5,0.5));
  model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

  engine.drawCube(model);

  // bottom
  model = base;
  model = glm::translate(model, glm::vec3(0,-5.5,0));
  model = glm::scale(model, glm::vec3(5,0.25,0.5));
  model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

  engine.drawCube(model);

  // top
  model = base;
  model = glm::translate(model, glm::vec3(0,5.75,0));
  model = glm::scale(model, glm::vec3(5,0.03125,0.0625));
  model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

  engine.useTexture(3);
  engine.drawCube(model);

  int i,j;
  if (gi->state != STATE_GAMEOVER) {
    for (i=0; i<10; i++) {
      for (j=0; j<24; j++) {
        if(gi->board[i][j] != -1) {
          bool hasLeft = true;
          bool hasRight = true;
          bool hasTop = true;
          bool hasBottom = true;
          if (i > 0 && gi->board[i-1][j] != -1) {
            hasLeft = false;
          }
          if (i < 9 && gi->board[i+1][j] != -1) {
            hasRight = false;
          }
          if (j > 0 && gi->board[i][j-1] != -1) {
            hasTop = false;
          }
          if (j < 23 && gi->board[i][j+1] != -1) {
            hasBottom = false;
          }
          drawBlock(context,
                    gi->board[i][j], gi, 0.5 * (double)i, 0.5 * (double)j, hasLeft,
                                                                           hasRight,
                                                                           hasTop,
                                                                           hasBottom);
        }
      }
    }
  }
  else {
    for (i=0; i<10; i++) {
      for (j=0; j<24; j++) {
        if(gi->board[i][j] != -1) {
          float offset_x = (float)(i - 5) * gi->gameover_position * GAMEOVER_SPREAD_RATE;
          float offset_y = (float)(11 - j) * gi->gameover_position * GAMEOVER_SPREAD_RATE;
          float z = gi->gameover_position;
          if (gi->rot2 > 90) {
            z = -z;
          }
          model = base;
          model = glm::translate(model, glm::vec3(-2.25f + (0.5f*i) + offset_x, 6.375f - (0.5f*j) + offset_y, z));
          model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
          engine.useTexture(gi->board[i][j]);
          engine.drawCube(model);
        }
      }
    }
  }

  for (i=0; i<10; i++) {
    for (j=2; j<24; j++) {
      if(gi->board[i][j] == -1) {
        drawBackgroundBlock(context, gi, i, j);
      }
    }
  }
}

void Tetris::drawBackgroundBlock(Context* context, game_info* gi, double x, double y) {
  engine.useTexture(17);

  // translate to world
  glm::mat4 model = glm::mat4(1.0f);

  // rotate
  model = glm::rotate(model, gi->side * gi->rot, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, -gi->rot2, glm::vec3(1.0f,0.0f,0.0f));

  model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));

  float z = -0.8f;

  float rot_percent = (float)gi->rot2 / 180.0f;
  z = 1.6f * rot_percent - 0.8f;

  // translate
  model = glm::translate(model, glm::vec3(-2.25f + (x*0.5), 6.375f - (y*0.5), z));

  // scale (make them 0.5 unit cubes, since our unit cube is 2x2x2)
  model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));

  context->setOpacity(engine.bg_tile_opacity);

  if (gi->rot2 > 90) {
    engine.drawQuad(model, 5);
  }
  else {
    engine.drawQuad(model, 0);
  }

  context->setOpacity(1.0f);
}

void Tetris::drawPiece(Context* context,
                       game_info* gi, double x, double y, int texture) {
  switch (gi->curpiece) {
    case 0:
      /*
       *   ##x#
       */
      if (gi->curdir % 2) {
        drawBlock(context, texture, gi, x-0.5, y, false, false, true, true);
        drawBlock(context, texture, gi, x-1.0, y, true,  false, true, true);
        drawBlock(context, texture, gi, x, y,     false, false, true, true);
        drawBlock(context, texture, gi, x+0.5, y, false, true,  true, true);
      }
      /*
       *   #
       *   x
       *   #
       *   #
       */
      else {
        drawBlock(context, texture, gi, x, y,     true, true, false, false);
        drawBlock(context, texture, gi, x, y+0.5, true, true, false, false);
        drawBlock(context, texture, gi, x, y+1.0, true, true, false, true);
        drawBlock(context, texture, gi, x, y-0.5, true, true, true,  false);
      }
      break;
    case 1:
      /*
       *    #
       *   x#
       *   #
       */
      if (gi->curdir % 2) {
        drawBlock(context, texture, gi, x+0.5, y,     false, true,  false, true);
        drawBlock(context, texture, gi, x, y+0.5,     true,  true,  false, true);
        drawBlock(context, texture, gi, x, y,         true,  false, true,  false);
        drawBlock(context, texture, gi, x+0.5, y-0.5, true,  true,  true,  false);
      }
      /*   ##
       *    x#
       */
      else {
        drawBlock(context, texture, gi, x+0.5, y,     false, true,  true,  true);
        drawBlock(context, texture, gi, x, y-0.5,     false, true,  true,  false);
        drawBlock(context, texture, gi, x, y,         true,  false, false, true);
        drawBlock(context, texture, gi, x-0.5, y-0.5, true,  false, true,  true);
      }
      break;
    case 2:
      /*
       *   #
       *   #x
       *    #
       */
      if (gi->curdir % 2) {
        drawBlock(context, texture, gi, x-0.5, y,     true,  false, false, true);
        drawBlock(context, texture, gi, x, y+0.5,     true,  true,  false, true);
        drawBlock(context, texture, gi, x, y,         false, true,  true,  false);
        drawBlock(context, texture, gi, x-0.5, y-0.5, true,  true,  true,  false);
      }
      /*
       *    ##
       *   #x
       */
      else {
        drawBlock(context, texture, gi, x-0.5, y,     true,  false, true,  true);
        drawBlock(context, texture, gi, x, y-0.5,     true,  false, true,  false);
        drawBlock(context, texture, gi, x, y,         false, true,  false, true);
        drawBlock(context, texture, gi, x+0.5, y-0.5, false, true,  true,  true);
      }
      break;
    case 3:
      /*
       *   x#
       *   ##
       */
      drawBlock(context, texture, gi, x+0.5, y,     false, true,  true,  false);
      drawBlock(context, texture, gi, x, y+0.5,     true,  false, false, true);
      drawBlock(context, texture, gi, x, y,         true,  false, true,  false);
      drawBlock(context, texture, gi, x+0.5, y+0.5, false, true,  false, true);
      break;
    case 4:
      /*
       *   #
       *   x
       *   ##
       */
      if (gi->curdir == 0) {
        drawBlock(context, texture, gi, x, y,         true,  true,  false, false);
        drawBlock(context, texture, gi, x, y+0.5,     true,  false, false, true);
        drawBlock(context, texture, gi, x, y-0.5,     true,  true,  true,  false);
        drawBlock(context, texture, gi, x+0.5, y+0.5, false, true,  true,  true);
      }
      /*
       *   #x#
       *   #
       */
      else if (gi->curdir == 1) {
        drawBlock(context, texture, gi, x, y,         false, false, true,  true);
        drawBlock(context, texture, gi, x+0.5, y,     false, true,  true,  true);
        drawBlock(context, texture, gi, x-0.5, y,     true,  false, true,  false);
        drawBlock(context, texture, gi, x-0.5, y+0.5, true,  true,  false, true);
      }
      /*
       *   ##
       *    x
       *    #
       */
      else if (gi->curdir == 2) {
        drawBlock(context, texture, gi, x, y,         true,  true,  false, false);
        drawBlock(context, texture, gi, x, y+0.5,     true,  true,  false, true);
        drawBlock(context, texture, gi, x, y-0.5,     false, true,  true,  false);
        drawBlock(context, texture, gi, x-0.5, y-0.5, true,  false, true,  true);
      }
      /*
       *     #
       *   #x#
       */
      else {
        drawBlock(context, texture, gi, x, y,         false, false, true,  true);
        drawBlock(context, texture, gi, x+0.5, y,     false, true,  false, true);
        drawBlock(context, texture, gi, x-0.5, y,     true,  false, true, true);
        drawBlock(context, texture, gi, x+0.5, y-0.5, true,  true,  true, false);
      }
      break;
    case 5:
      /*
       *    #
       *    x
       *   ##
       */
      if (gi->curdir == 0) {
        drawBlock(context, texture, gi, x, y,         true,  true,  false, false);
        drawBlock(context, texture, gi, x, y+0.5,     false, true,  false, true);
        drawBlock(context, texture, gi, x, y-0.5,     true,  true,  true,  false);
        drawBlock(context, texture, gi, x-0.5, y+0.5, true,  false, true,  true);
      }
      /*
       *   #
       *   #x#
       */
      else if (gi->curdir == 1) {
        drawBlock(context, texture, gi, x, y,         false, false, true,  true);
        drawBlock(context, texture, gi, x+0.5, y,     false, true,  true,  true);
        drawBlock(context, texture, gi, x-0.5, y,     true,  false, false, true);
        drawBlock(context, texture, gi, x-0.5, y-0.5, true,  true,  true,  false);
      }
      /*
       *   ##
       *   x
       *   #
       */
      else if (gi->curdir == 2) {
        drawBlock(context, texture, gi, x, y,         true,  true,  false, false);
        drawBlock(context, texture, gi, x, y+0.5,     true,  true,  false, true);
        drawBlock(context, texture, gi, x, y-0.5,     true,  false, true,  false);
        drawBlock(context, texture, gi, x+0.5, y-0.5, false, true,  true,  true);
      }
      /*
       *   #x#
       *     #
       */
      else {
        drawBlock(context, texture, gi, x, y,         false, false, true,  true);
        drawBlock(context, texture, gi, x+0.5, y,     false, true,  true,  false);
        drawBlock(context, texture, gi, x-0.5, y,     true,  false, true,  true);
        drawBlock(context, texture, gi, x+0.5, y+0.5, true,  true,  false, true);
      }
      break;
    case 6:
      /*
       *    #
       *   #x#
       */
      if (gi->curdir == 0) {
        drawBlock(context, texture, gi, x, y,     false, false, false, true);
        drawBlock(context, texture, gi, x+0.5, y, false, true,  true,  true);
        drawBlock(context, texture, gi, x-0.5, y, true,  false, true,  true);
        drawBlock(context, texture, gi, x, y-0.5, true,  true,  true,  false);
      }
      /*
       *   #
       *   x#
       *   #
       */
      else if (gi->curdir == 1) {
        drawBlock(context, texture, gi, x, y,     true,  false, false, false);
        drawBlock(context, texture, gi, x+0.5, y, false, true,  true,  true);
        drawBlock(context, texture, gi, x, y+0.5, true,  true,  false, true);
        drawBlock(context, texture, gi, x, y-0.5, true,  true,  true,  false);
      }
      /*
       *   #x#
       *    #
       */
      else if (gi->curdir == 2) {
        drawBlock(context, texture, gi, x, y,     false, false, true,  false);
        drawBlock(context, texture, gi, x+0.5, y, false, true,  true,  true);
        drawBlock(context, texture, gi, x-0.5, y, true,  false, true,  true);
        drawBlock(context, texture, gi, x, y+0.5, true,  true,  false, true);
      }
      /*
       *    #
       *   #x
       *    #
       */
      else {
        drawBlock(context, texture, gi, x, y,     false, true,  false, false);
        drawBlock(context, texture, gi, x-0.5, y, true,  false, true,  true);
        drawBlock(context, texture, gi, x, y+0.5, true,  true,  false, true);
        drawBlock(context, texture, gi, x, y-0.5, true,  true,  true,  false);
      }
      break;
  }
}

bool Tetris::testCollisionBlock(game_info* gi, double x, double y) {
  int s_x;
  int s_y;

  s_x = (x / 0.5);
  s_y = (y / 0.5);
  s_y++;

  if (gi->board[s_x][s_y] != -1) {
    return 1;
  }

  return 0;
}

bool Tetris::testCollision(game_info *gi) {
  return testCollision(gi, (0.5) * (double)gi->pos, gi->fine);
}

double Tetris::testSideCollision(game_info* gi, double x, double y) {
  int sx = x / 0.5;

  switch (gi->curpiece) {
    case 0:
      if (gi->curdir % 2) {
        if (sx < 2) { return 0.5; }
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
        if (sx < 1) { return 0.25; }
      }
      break;
    case 2:
      if (sx < 1) { return 0.25; }
      break;
    case 3:
      if (sx < 0) { return 0; }
      break;
    case 4:
      if (gi->curdir == 0) {
        if (sx < 0) { return 0; }
      }
      else {
        if (sx < 1) { return 0.25; }
      }
      break;
    case 5:
      if (gi->curdir == 2) {
        if (sx < 0) { return 0; }
      }
      else {
        if (sx < 1) { return 0.25; }
      }
      break;
    case 6:
      if (gi->curdir == 1) {
        if (sx < 0) { return 0; }
      }
      else {
        if (sx < 1) { return 0.25; }
      }
      break;
  }

  switch (gi->curpiece) {
    case 0:
      if (gi->curdir % 2) {
        if (sx > 8) { return 1; }
      }
      else {
        if (sx > 9) { return 1; }
      }
      break;
    case 1:
      if (sx > 8) { return 1; }
      break;
    case 2:
      if (gi->curdir % 2) {
        if (sx > 9) { return 1; }
      }
      else {
        if (sx > 8) { return 1; }
      }
      break;
    case 3:
      if (sx > 8) { return 1; }
      break;
    case 4:
      if (gi->curdir == 2) {
        if (sx > 9) { return 1; }
      }
      else
      {
        if (sx > 8) { return 1; }
      }
      break;
    case 5:
      if (gi->curdir == 0) {
        if (sx > 9) { return 1; }
      }
      else {
        if (sx > 8) { return 1; }
      }
      break;
    case 6:
      if (gi->curdir == 3) {
        if (sx > 9) { return 1; }
      }
      else {
        if (sx > 8) { return 1; }
      }
      break;
  }

  return -10;
}

bool Tetris::testCollision(game_info* gi, double x, double y) {
  // check collision with bottom

  static double bottom_y = (0.5) * 24;

  double test_y = y;

  switch (gi->curpiece) {
    case 0:
      if (gi->curdir % 2) {
        test_y = 0;
      }
      else {
        test_y = 2;
      }
      break;
    case 1:
    case 2:
      if (gi->curdir % 2) {
        test_y = 1;
      }
      else {
        test_y = 0;
      }
      break;
    case 3:
      test_y = 1;
      break;
    case 4:
      if (gi->curdir == 3) {
        test_y = 0;
      }
      else {
        test_y = 1;
      }
      break;
    case 5:
      if (gi->curdir == 1) {
        test_y = 0;
      }
      else {
        test_y = 1;
      }
      break;
    case 6:
      if (gi->curdir == 0) {
        test_y = 0;
      }
      else {
        test_y = 1;
      }
      break;
  }

  test_y++;

  test_y *= 0.5;
  test_y += y;

  if (test_y > bottom_y) {
    return 1;
  }

  // OK!

  // check collision with sides

  double ret = testSideCollision(gi, x, y);

  if (ret!=-10) {
    return 1;
  }

  // COLLISION AMONG BLOCKS!

  bool coll = false;

  switch (gi->curpiece)
  {
    case 0:
      if (gi->curdir % 2) {
        coll |= testCollisionBlock(gi, x-0.5, y);
        coll |= testCollisionBlock(gi, x-1.0, y);
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x+0.5, y);
      }
      else {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x, y+0.5);
        coll |= testCollisionBlock(gi, x, y+1.0);
        coll |= testCollisionBlock(gi, x, y-0.5);
      }
      break;
    case 1:
      if (gi->curdir % 2) {
        coll |= testCollisionBlock(gi, x+0.5, y);
        coll |= testCollisionBlock(gi, x, y+0.5);
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x+0.5, y-0.5);
      }
      else {
        coll |= testCollisionBlock(gi, x+0.5, y);
        coll |= testCollisionBlock(gi, x, y-0.5);
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x-0.5, y-0.5);
      }
      break;
    case 2:
      if (gi->curdir % 2) {
        coll |= testCollisionBlock(gi, x-0.5, y);
        coll |= testCollisionBlock(gi, x, y+0.5);
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x-0.5, y-0.5);
      }
      else {
        coll |= testCollisionBlock(gi, x-0.5, y);
        coll |= testCollisionBlock(gi, x, y-0.5);
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x+0.5, y-0.5);
      }
      break;
    case 3:
      coll |= testCollisionBlock(gi, x+0.5, y);
      coll |= testCollisionBlock(gi, x, y+0.5);
      coll |= testCollisionBlock(gi, x, y);
      coll |= testCollisionBlock(gi, x+0.5, y+0.5);
      break;
    case 4:
      if (gi->curdir == 0) {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x, y+0.5);
        coll |= testCollisionBlock(gi, x, y-0.5);
        coll |= testCollisionBlock(gi, x+0.5, y+0.5);
      }
      else if (gi->curdir == 1) {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x+0.5, y);
        coll |= testCollisionBlock(gi, x-0.5, y);
        coll |= testCollisionBlock(gi, x-0.5, y+0.5);
      }
      else if (gi->curdir == 2) {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x, y+0.5);
        coll |= testCollisionBlock(gi, x, y-0.5);
        coll |= testCollisionBlock(gi, x-0.5, y-0.5);
      }
      else {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x+0.5, y);
        coll |= testCollisionBlock(gi, x-0.5, y);
        coll |= testCollisionBlock(gi, x+0.5, y-0.5);
      }
      break;
    case 5:
      if (gi->curdir == 0) {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x, y+0.5);
        coll |= testCollisionBlock(gi, x, y-0.5);
        coll |= testCollisionBlock(gi, x-0.5, y+0.5);
      }
      else if (gi->curdir == 1) {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x+0.5, y);
        coll |= testCollisionBlock(gi, x-0.5, y);
        coll |= testCollisionBlock(gi, x-0.5, y-0.5);
      }
      else if (gi->curdir == 2) {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x, y+0.5);
        coll |= testCollisionBlock(gi, x, y-0.5);
        coll |= testCollisionBlock(gi, x+0.5, y-0.5);
      }
      else {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x+0.5, y);
        coll |= testCollisionBlock(gi, x-0.5, y);
        coll |= testCollisionBlock(gi, x+0.5, y+0.5);
      }
      break;
    case 6:
      if (gi->curdir == 0) {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x+0.5, y);
        coll |= testCollisionBlock(gi, x-0.5, y);
        coll |= testCollisionBlock(gi, x, y-0.5);
      }
      else if (gi->curdir == 1) {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x+0.5, y);
        coll |= testCollisionBlock(gi, x, y+0.5);
        coll |= testCollisionBlock(gi, x, y-0.5);
      }
      else if (gi->curdir == 2) {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x+0.5, y);
        coll |= testCollisionBlock(gi, x-0.5, y);
        coll |= testCollisionBlock(gi, x, y+0.5);
      }
      else {
        coll |= testCollisionBlock(gi, x, y);
        coll |= testCollisionBlock(gi, x-0.5, y);
        coll |= testCollisionBlock(gi, x, y+0.5);
        coll |= testCollisionBlock(gi, x, y-0.5);
      }
      break;
  }

  return coll;
}

void Tetris::keyRepeat(game_info* gi) {
  if (gi->state != STATE_TETRIS) {
    return;
  }

  if (engine.keys[SDLK_UP]) {
    gi->curdir++;
    gi->curdir %= 4;

    if (testCollision(gi)) {
      gi->curdir--;
      gi->curdir += 4;
      gi->curdir %= 4;
    }

    engine.passMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
  }
  else if (engine.keys[SDLK_LEFT]) {
    gi->pos--;

    // test collisions!
    if (testCollision(gi)) {
      gi->pos++;
    }

    engine.passMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
  }
  else if (engine.keys[SDLK_RIGHT]) {
    gi->pos++;


    // test collisions!
    if (testCollision(gi)) {
      gi->pos--;
    }

    engine.passMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
  }
}

void Tetris::keyDown(game_info* gi, Uint32 key) {
  if (gi->state != STATE_TETRIS) {
    return;
  }

  if (key == SDLK_SPACE) {
    dropPiece(gi);
  }
}

void Tetris::keyUp(game_info* gi, Uint32 key) {
}

void Tetris::mouseMovement(game_info* gi, Uint32 x, Uint32 y) {
}

void Tetris::mouseDown(game_info* gi) {
}

void Tetris::getNewPiece(game_info* gi) {
  gi->curpiece = rand() % 7;
  gi->curdir = 1;

  gi->pos = 5;
  gi->fine = 1.0;

  engine.passMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
  engine.passMessage(MSG_UPDATEPIECEY, 0, 0, 0);
}

void Tetris::pushUp(game_info* gi, int num) {
  int i,j;
  for (j=0; j<(24-num); j++) {
    for (i=0; i<10; i++) {
      gi->board[i][j] = gi->board[i][j+num];
    }
  }
}

void Tetris::attack(game_info* gi, int severity) {
  // add a line!
  // add two lines!!
  // rotate board!!!

  int gameover = 0;
  int good = 0;

  engine.audio.playSound(SND_ADDLINE);

  if (severity == 1) {
    // ok dokey
    int i;

    for (i=0; i<10; i++) {
      if (gi->board[i][2] != -1) {
        // game over!
        gameover = 1;
        break;
      }
    }

    pushUp(gi, 1);

    for (i=0; i<10; i++) {
      gi->board[i][23] = rand() % 7;
      if (gi->board[i][23] == 6) {
        good = 1;
        gi->board[i][23] = -1;
      }
    }

    if (!good) {
      gi->board[rand() % 10][23] = -1;
    }

    // send this line!
    engine.passMessage(MSG_PUSHUP, 1,0,0);
    engine.passMessage(MSG_ADDBLOCKS_A,gi->board[0][23], gi->board[1][23],gi->board[2][23]);
    engine.passMessage(MSG_ADDBLOCKS_B,gi->board[3][23], gi->board[4][23],gi->board[5][23]);
    engine.passMessage(MSG_ADDBLOCKS_C,gi->board[6][23], gi->board[7][23],gi->board[8][23]);
    engine.passMessage(MSG_ADDBLOCKS_D,gi->board[9][23], 0,0);
  }
  else if (severity == 2) {
    // move two lines
    // ok dokey
    int i;

    for (i=0; i<10; i++) {
      if (gi->board[i][0] != -1) {
        // game over!
        gameover = 1;
        break;
      }
      if (gi->board[i][1] != -1) {
        // game over!
        gameover = 1;
        break;
      }
    }

    pushUp(gi, 2);

    for (i=0; i<10; i++) {
      gi->board[i][23] = rand() % 7;
      if (gi->board[i][23] == 6) {
        good |= 1;
        gi->board[i][23] = -1;
      }

      gi->board[i][22] = rand() % 7;
      if (gi->board[i][22] == 6) {
        good |= 2;
        gi->board[i][22] = -1;
      }
    }

    if (!(good & 1)) {
      gi->board[rand() % 10][23] = -1;
    }

    if (!(good & 2)) {
      gi->board[rand() % 10][22] = -1;
    }

    // send these lines!
    engine.passMessage(MSG_PUSHUP, 2,0,0);
    engine.passMessage(MSG_ADDBLOCKS_A,gi->board[0][23], gi->board[1][23],gi->board[2][23]);
    engine.passMessage(MSG_ADDBLOCKS_B,gi->board[3][23], gi->board[4][23],gi->board[5][23]);
    engine.passMessage(MSG_ADDBLOCKS_C,gi->board[6][23], gi->board[7][23],gi->board[8][23]);
    engine.passMessage(MSG_ADDBLOCKS_D,gi->board[9][23], 0,0);
    engine.passMessage(MSG_ADDBLOCKS2_A,gi->board[0][22], gi->board[1][22],gi->board[2][22]);
    engine.passMessage(MSG_ADDBLOCKS2_B,gi->board[3][22], gi->board[4][22],gi->board[5][22]);
    engine.passMessage(MSG_ADDBLOCKS2_C,gi->board[6][22], gi->board[7][22],gi->board[8][22]);
    engine.passMessage(MSG_ADDBLOCKS2_D,gi->board[9][22], 0,0);
  }
  else if (severity == 3) {
    // rotate!
    gi->attacking = 1;
    gi->attack_rot = 0;
  }

  if (gameover) {
    engine.gameOver();
  }
}

GLfloat Tetris::board_piece_amb[4] = {0.0, 0.0, 0.0, 1};
GLfloat Tetris::board_piece_diff[4] = {0.6, 0.6, 0.6, 1};
GLfloat Tetris::board_piece_spec[4] = {0.0, 0.0, 0.0, 1};
GLfloat Tetris::board_piece_emi[4] = {0.3, 0.3, 0.3, 1};
GLfloat Tetris::board_piece_shine = 0;

GLfloat Tetris::tet_piece_amb[4] = {0.2, 0.2, 0.2, 1};
GLfloat Tetris::tet_piece_diff[6][4] = {
  {0.2, 0.2, 0.8, 1},
  {0.8, 0.2, 0.8, 1},
  {0.8, 0.2, 0.2, 1},
  {0.2, 0.8, 0.2, 1},
  {0.8, 0.8, 0.2, 1},
  {0.2, 0.8, 0.8, 1},
};
GLfloat Tetris::tet_piece_spec[4] = {0.3, 0.3, 0.3, 1};
GLfloat Tetris::tet_piece_emi[4] = {0.0, 0.0, 0.0, 1};
GLfloat Tetris::tet_piece_shine = 1;
