#include "main.h"
#include "engine.h"
#include "components.h"

#include <math.h>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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

#ifndef NO_NETWORK
// threading code for networking
int thread_func(void *unused) {
  for (;;) {
    // receive some text from sock
    //TCPsocket sock;
    int result;
    unsigned char msg[4];

    result=SDLNet_TCP_Recv(engine.client_tcpsock,msg,4);
    if(result<=0) {
      // An error may have occured, but sometimes you can just ignore it
      // It may be good to disconnect sock because it is likely invalid now.
      //break;
      //printf("socket error\n");
    }
    else {
      //printf("recv: %d\n", result);
      engine.processMessage(msg);
    }
  }

  return(0);
}
#endif

Engine::Engine() {
}

Engine::~Engine() {
}

static
const GLfloat _cube_data[] = {
  // cube ///////////////////
  //  v6----- v5
  // /|       /|
  // v1------v0|
  // | |     | |
  // | |v7---|-|v4
  // |/      |/
  // v2------v3
  // ////////////////////////

  // Face v0-v1-v2-v3 (front)
   1, 1, 1, 0, 0, 1, 0, 0,
  -1, 1, 1, 0, 0, 1, 1, 0,
  -1,-1, 1, 0, 0, 1, 1, 1,
   1,-1, 1, 0, 0, 1, 0, 1,
  // Face v0-v3-v4-v5 (right)
   1, 1, 1, 1, 0, 0, 0, 0,
   1,-1, 1, 1, 0, 0, 0, 1,
   1,-1,-1, 1, 0, 0, 1, 1,
   1, 1,-1, 1, 0, 0, 1, 0,
  // Face v0-v5-v6-v1 (top)
   1, 1, 1, 0, 1, 0, 0, 0,
   1, 1,-1, 0, 1, 0, 0, 1,
  -1, 1,-1, 0, 1, 0, 1, 1,
  -1, 1, 1, 0, 1, 0, 1, 0,
  // Face v1-v6-v7-v2 (left)
  -1, 1, 1,-1, 0, 0, 0, 0,
  -1, 1,-1,-1, 0, 0, 0, 1,
  -1,-1,-1,-1, 0, 0, 1, 1,
  -1,-1, 1,-1, 0, 0, 1, 0,
  // Face v7-v4-v3-v2 (bottom)
  -1,-1,-1, 0,-1, 0, 1, 1,
   1,-1,-1, 0,-1, 0, 0, 1,
   1,-1, 1, 0,-1, 0, 0, 0,
  -1,-1, 1, 0,-1, 0, 1, 0,
  // Face v4-v7-v6-v5 (back)
   1,-1,-1, 0, 0,-1, 0, 0,
  -1,-1,-1, 0, 0,-1, 1, 0,
  -1, 1,-1, 0, 0,-1, 1, 1,
   1, 1,-1, 0, 0,-1, 0, 1
};

static
const GLushort _cube_elements[] = {
  0, 1, 2,
  2, 3, 0,
  4, 5, 6,
  6, 7, 4,
  8, 9, 10,
  10, 11, 8,
  12, 13, 14,
  14, 15, 12,
  16, 17, 18,
  18, 19, 16,
  20, 21, 22,
  22, 23, 20
};

static
const GLfloat _hud_data[] = {
  // 0
  -0.5, 0.5, 0.0, 0, 0, 1, 230.0f/512.0f,  0.0f/512.0f,
   0.5, 0.5, 0.0, 0, 0, 1, 260.0f/512.0f,  0.0f/512.0f,
   0.5,-0.5, 0.0, 0, 0, 1, 260.0f/512.0f, 38.0f/512.0f,
  -0.5,-0.5, 0.0, 0, 0, 1, 230.0f/512.0f, 38.0f/512.0f,

  // 1
  -0.5, 0.5, 0.0, 0, 0, 1, 196.0f/512.0f, 41.0f/512.0f,
   0.5, 0.5, 0.0, 0, 0, 1, 222.0f/512.0f, 41.0f/512.0f,
   0.5,-0.5, 0.0, 0, 0, 1, 222.0f/512.0f, 78.0f/512.0f,
  -0.5,-0.5, 0.0, 0, 0, 1, 196.0f/512.0f, 78.0f/512.0f,

  // 2
  -0.5, 0.5, 0.0, 0, 0, 1,  55.0f/512.0f,  98.0f/512.0f,
   0.5, 0.5, 0.0, 0, 0, 1,  87.0f/512.0f,  98.0f/512.0f,
   0.5,-0.5, 0.0, 0, 0, 1,  87.0f/512.0f, 136.0f/512.0f,
  -0.5,-0.5, 0.0, 0, 0, 1,  55.0f/512.0f, 136.0f/512.0f,

  // 3
  -0.5, 0.5, 0.0, 0, 0, 1, 239.0f/512.0f,  80.0f/512.0f,
   0.5, 0.5, 0.0, 0, 0, 1, 267.0f/512.0f,  80.0f/512.0f,
   0.5,-0.5, 0.0, 0, 0, 1, 267.0f/512.0f, 118.0f/512.0f,
  -0.5,-0.5, 0.0, 0, 0, 1, 239.0f/512.0f, 118.0f/512.0f,

  // 4
  -0.5, 0.5, 0.0, 0, 0, 1, 238.0f/512.0f, 122.0f/512.0f,
   0.5, 0.5, 0.0, 0, 0, 1, 267.0f/512.0f, 122.0f/512.0f,
   0.5,-0.5, 0.0, 0, 0, 1, 267.0f/512.0f, 160.0f/512.0f,
  -0.5,-0.5, 0.0, 0, 0, 1, 238.0f/512.0f, 160.0f/512.0f,

  // 5
  -0.5, 0.5, 0.0, 0, 0, 1, 238.0f/512.0f, 162.0f/512.0f,
   0.5, 0.5, 0.0, 0, 0, 1, 266.0f/512.0f, 162.0f/512.0f,
   0.5,-0.5, 0.0, 0, 0, 1, 266.0f/512.0f, 200.0f/512.0f,
  -0.5,-0.5, 0.0, 0, 0, 1, 238.0f/512.0f, 200.0f/512.0f,

  // 6
  -0.5, 0.5, 0.0, 0, 0, 1, 230.0f/512.0f, 40.0f/512.0f,
   0.5, 0.5, 0.0, 0, 0, 1, 260.0f/512.0f, 40.0f/512.0f,
   0.5,-0.5, 0.0, 0, 0, 1, 260.0f/512.0f, 78.0f/512.0f,
  -0.5,-0.5, 0.0, 0, 0, 1, 230.0f/512.0f, 78.0f/512.0f,

  // 7
  -0.5, 0.5, 0.0, 0, 0, 1, 226.0f/512.0f, 206.0f/512.0f,
   0.5, 0.5, 0.0, 0, 0, 1, 258.0f/512.0f, 206.0f/512.0f,
   0.5,-0.5, 0.0, 0, 0, 1, 258.0f/512.0f, 245.0f/512.0f,
  -0.5,-0.5, 0.0, 0, 0, 1, 226.0f/512.0f, 245.0f/512.0f,

  // 8
  -0.5, 0.5, 0.0, 0, 0, 1, 192.0f/512.0f, 206.0f/512.0f,
   0.5, 0.5, 0.0, 0, 0, 1, 224.0f/512.0f, 206.0f/512.0f,
   0.5,-0.5, 0.0, 0, 0, 1, 224.0f/512.0f, 246.0f/512.0f,
  -0.5,-0.5, 0.0, 0, 0, 1, 192.0f/512.0f, 246.0f/512.0f,

  // 9
  -0.5, 0.5, 0.0, 0, 0, 1, 196.0f/512.0f,  0.0f/512.0f,
   0.5, 0.5, 0.0, 0, 0, 1, 228.0f/512.0f,  0.0f/512.0f,
   0.5,-0.5, 0.0, 0, 0, 1, 228.0f/512.0f, 39.0f/512.0f,
  -0.5,-0.5, 0.0, 0, 0, 1, 196.0f/512.0f, 39.0f/512.0f,
};

static
const GLushort _hud_elements[] = {
  0, 1, 2,
  2, 3, 0,

  4, 5, 6,
  6, 7, 4,

  8, 9, 10,
  10, 11, 8,

  12, 13, 14,
  14, 15, 12,

  16, 17, 18,
  18, 19, 16,

  20, 21, 22,
  22, 23, 20,

  24, 25, 26,
  26, 27, 24,

  28, 29, 30,
  30, 31, 28,

  32, 33, 34,
  34, 35, 32,

  36, 37, 38,
  38, 39, 36
};

void Engine::init() {
  inplay = true;

  // INITIALIZE OPENGL!!!

  // Init GLEW
#ifndef EMSCRIPTEN
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return;
  }
#endif

  // clear color
  glClearColor(0,1,0,1);
  gl_check_errors("glClearColor");

  clearGameData(&player1);
  clearGameData(&player2);

  player1.side = -1;
  player2.side = 1;

  player1.pos = 5;
  player2.pos = 5;

  player1.rot = -BOARD_NORMAL_ROT;
  player2.rot = -BOARD_NORMAL_ROT;

  srand(SDL_GetTicks());

  addTexture("images/block_01.png");
  addTexture("images/block_02.png");
  addTexture("images/block_03.png");
  addTexture("images/block_04.png");
  addTexture("images/block_05.png");
  addTexture("images/block_06.png");
  addTexture("images/block_07.png");

  addTexture("images/stars-layer.png");
  addTexture("images/nebula-layer.png");

  addTexture("images/ball.png");

  addTexture("images/space-penguinsmall.png");

  addTexture("images/numbers.png");
  addTexture("images/letters.png");

  addTexture("images/penguinsmall.png");
  addTexture("images/speech-right.png");

  addTexture("images/letters_w.png");

  addTexture("images/block08.png");
  addTexture("images/block_09.png");
  addTexture("images/block10.png");

  addTexture("images/hud_spritesheet.png");

  audio.init();

  audio.loadSound("sounds/addline.wav");
  audio.loadSound("sounds/tink.wav");
  audio.loadSound("sounds/penguin-short.wav");
  audio.loadSound("sounds/bounce.wav");
  audio.loadSound("sounds/changeview.wav");

  audio.loadMusic("music/bsh.ogg");
  audio.playMusic();

  // enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  player1.state = STATE_TETRIS;
  initState(&player1);

  player1.pos = 5;
  player1.fine = 0;

  tetris.getNewPiece(&player1);

  _context   = new Context();

  _cube_mesh = new Mesh(_cube_data, sizeof(_cube_data)/sizeof(float),
                        _cube_elements, sizeof(_cube_elements)/sizeof(short));
  _hud_mesh  = new Mesh(_hud_data, sizeof(_hud_data)/sizeof(float),
                        _hud_elements, sizeof(_hud_elements)/sizeof(short));
  _ship_mesh = new Mesh("assets/ship_final.obj");

  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, textures[0]);
  gl_check_errors("glBindTexture");
  glDisable(GL_CULL_FACE);

  _ship_engine_one = new Flame(-9.0, -0.5, 0.0);
  _ship_engine_two = new Flame( 9.0, -0.5, 0.0);
}

void Engine::sendAttack(int severity) {
  if (severity == 3 && player1.state == STATE_TETRIS) {
    displayMessage(STR_TETRIS);
  }
  else if (severity == 3) {
    displayMessage(STR_SUPER);
  }

  //printf("ATTACK! %d\n", severity);

  passMessage(MSG_ATTACK, (unsigned char)severity, 0, 0);
}

void Engine::performAttack(int severity) {
  printf("PERFORM ATTACK! %d\n", severity);

  displayMessage(STR_ATTACK);

  games[player1.curgame]->attack(&player1, severity);
}

void Engine::quit() {
  SDL_Quit();
  _quit = 1;
}

#ifdef EMSCRIPTEN
// Unsafe singleton pointer for C/js systems
static Engine* _c_this;
#endif

void Engine::_c_iterate() {
#ifdef EMSCRIPTEN
  _c_this->_iterate();
#endif
}

void Engine::gameLoop() {
#ifdef EMSCRIPTEN
  _c_this = this;
  emscripten_set_main_loop(Engine::_c_iterate, 30, 0);
#else
  while(_iterate()) {}
#endif
}

bool Engine::_iterate() {
  static SDL_Event event;

  static Uint32 lasttime = SDL_GetTicks();
  static Uint32 curtime;

  static float deltatime;

  if (!_quit) {
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN:
          keyDown(event.key.keysym.sym);
          break;
        case SDL_KEYUP:
          keyUp(event.key.keysym.sym);
          break;
        case SDL_MOUSEMOTION:
          mouseMovement(event.motion.x, event.motion.y);
          break;
        case SDL_MOUSEBUTTONDOWN:
          mouseDown();
          break;
        case SDL_MOUSEBUTTONUP:
          break;
        case SDL_QUIT:
          quit();
          return false;
      }
    }

    if (_quit) { return false; }

    // Draw Frame, tell engine stuff

    curtime = SDL_GetTicks();

    deltatime = (float)(curtime - lasttime) / 1000.0f;

    // CALL ENGINE
    update(deltatime);
    draw();

    lasttime = curtime;
  }

  return true;
}

void Engine::drawMesh(int count) {
  glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, 0);
  gl_check_errors("glDrawElements mesh");
}

void Engine::update(float deltatime) {
  // scroll background
  bg1x += BG1_SPEED_X * deltatime;
  bg1y += BG1_SPEED_Y * deltatime;

  bg2x += BG2_SPEED_X * deltatime;
  bg2y += BG2_SPEED_Y * deltatime;

  while (bg1x > SCROLL_CONSTRAINT) {
    bg1x -= 30;
  }

  while (bg1x < -SCROLL_CONSTRAINT) {
    bg1x += 30;
  }

  while (bg1y > SCROLL_CONSTRAINT) {
    bg1y -= 30;
  }

  while (bg1y < -SCROLL_CONSTRAINT) {
    bg1y += 30;
  }

  while (bg2x > SCROLL_CONSTRAINT) {
    bg2x -= 30;
  }

  while (bg2x < -SCROLL_CONSTRAINT) {
    bg2x += 30;
  }

  while (bg2y > SCROLL_CONSTRAINT) {
    bg2y -= 30;
  }

  while (bg2y < -SCROLL_CONSTRAINT) {
    bg2y += 30;
  }

  if (repeatTime < 0.35) {
    repeatTime += deltatime;
  }
  else {
    time += deltatime;

    if (time >= 0.05) {
      time = 0;

      games[player1.curgame]->keyRepeat(&player1);
    }
  }

  if (bg_tile_opacity_direction) {
    bg_tile_opacity -= deltatime * 0.02;
  }
  else {
    bg_tile_opacity += deltatime * 0.02;
  }

  if (bg_tile_opacity < 0.1) {
    bg_tile_opacity = 0.1;
    bg_tile_opacity_direction = false;
  }
  if (bg_tile_opacity > 0.14) {
    bg_tile_opacity = 0.14;
    bg_tile_opacity_direction = true;
  }

  // update current game
  games[player1.curgame]->update(&player1, deltatime);

  _ship_engine_one->update(deltatime);
  _ship_engine_two->update(deltatime);
}

int Engine::intLength(int i) {
  char buffer[255];

  sprintf(buffer, "%d", i);

  return (int)strlen(buffer);
}

int Engine::drawInt(int i, int color, float x, float y) {
  useTexture(19);

  static int hud_widths[]  = {30.0f, 26.0f, 32.0f, 28.0f, 29.0f,
                              28.0f, 30.0f, 32.0f, 32.0f, 32.0f};

  static int hud_heights[] = {38.0f, 37.0f, 38.0f, 38.0f, 38.0f,
                              38.0f, 38.0f, 39.0f, 40.0f, 39.0f};

  int length = 0;
  int width = 0;

  float scale = 0.5f;

  int tmp = i;
  do {
    int digit = tmp % 10;
    length += 1;
    width += hud_widths[digit]*scale;
    tmp /= 10;
  } while (tmp > 0);

  tmp = i;
  do {
    int digit = tmp % 10;
    width -= hud_widths[digit]*scale;

    glm::mat4 model = glm::scale(
        glm::translate(
          glm::mat4(1.0f),
          glm::vec3(x+width, y, 1.0f)),
        glm::vec3(hud_widths[digit]*scale, hud_heights[digit]*scale, 1.0f));

    _hud_mesh->drawSubset(_context, model, digit * 6, 6);

    tmp /= 10;
  } while (width > 0);

  return 0;
}

void Engine::displayMessage(int stringIndex) {
  player1.message = strings[stringIndex];

  player1.message_uptime = 3;

  audio.playSound(SND_PENGUIN);
}

void Engine::draw() {
  // clear buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);

  // Perspective
  _context->usePerspective();

  // BACKGROUND!!!
  useTexture(TEXTURE_BG1);

  drawQuadXY(bg1x, bg1y, -12.3f, 30, 30);
  drawQuadXY(bg1x - 30, bg1y, -12.3f, 30, 30);
  drawQuadXY(bg1x, bg1y-30, -12.3f, 30, 30);
  drawQuadXY(bg1x - 30, bg1y-30, -12.3f, 30, 30);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  useTexture(TEXTURE_BG2);

  // draw current game
  games[player1.curgame]->draw(_context, &player1);
  games[player2.curgame]->draw(_context, &player2);

  // Ship left
  useTexture(TEXTURE_BLOCK1);
  glm::mat4 model = glm::mat4(1.0f);

  model = glm::rotate(model, -player1.rot, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::translate(model, glm::vec3(-3.8f, -1.0f, 0.0f));
  model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));
  model = glm::rotate(model, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));

  _ship_mesh->draw(_context, model);

  // Ship right
  model = glm::mat4(1.0f);

  model = glm::rotate(model, -player1.rot, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::translate(model, glm::vec3(3.8f, -1.0f, 0.0f));
  model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));
  model = glm::rotate(model, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));

  _ship_mesh->draw(_context, model);

  // Ship engines
  _ship_engine_one->setColor(LEVEL);
  _ship_engine_one->setRotationY(-player1.rot);
  _ship_engine_one->draw(_context);

  _ship_engine_two->setColor(LEVEL);
  _ship_engine_two->setRotationY(-player1.rot);
  _ship_engine_two->draw(_context);

  // Orthographic (UI)

  _context->useOrthographic();

  gl_check_errors("glUniformMatrix4fv orthographic");

  games[player1.curgame]->drawOrtho(_context, &player1);
  games[player2.curgame]->drawOrtho(_context, &player2);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  drawInt(player1.score, 0, -(float)WIDTH/2.0f + 30, (float)HEIGHT/2.0f - 30);

  SDL_GL_SwapBuffers();
}

void Engine::keyDown(Uint32 key) {
  bool gameover = !inplay;
#ifndef NO_NETWORK
  gameover = gameover && !client_tcpsock;
#endif

  if (gameover) {
    if (player1.gameover_position > 1.0f) {
      clearGameData(&player1);
    }
    return;
  }

  if (key == SDLK_4) {
    performAttack(3);
  }

  if (key == SDLK_ESCAPE) {
    quit();
    return;
  }

  repeatTime = 0;

  if (key < 0xffff) {
    keys[key] = 1;
  }

  if (!inplay) { return; }

  games[player1.curgame]->keyDown(&player1, key);
  games[player1.curgame]->keyRepeat(&player1);
}

void Engine::keyUp(Uint32 key) {
  if (key < 0xffff) {
    keys[key] = 0;
  }

  if (!inplay) { return; }

  games[player1.curgame]->keyUp(&player1, key);
}

void Engine::mouseDown() {
  bool gameover = !inplay;
#ifndef NO_NETWORK
  gameover = gameover && !client_tcpsock;
#endif

  if (gameover) {
    if (player1.gameover_position > 1.0f) {
      clearGameData(&player1);
    }
  }
}

void Engine::mouseMovement(Uint32 x, Uint32 y) {
  games[player1.curgame]->mouseMovement(&player1, x,y);
}

void Engine::gameOver() {
  player1.state = STATE_GAMEOVER;
  inplay = false;

  displayMessage(STR_YOULOSE);

  passMessage(MSG_GAMEOVER, 0,0,0);
}

void Engine::drawQuadXY(float x, float y, float z, float w, float h) {
  glm::mat4 model = glm::scale(
                      glm::translate(
                        glm::mat4(1.0f),
                        glm::vec3(x, y, z)),
                      glm::vec3(w, h, 1.0f));

  _cube_mesh->drawSubset(_context, model, 0, 6);
}

void Engine::drawQuad(glm::mat4& model, int side) {
  _cube_mesh->drawSubset(_context, model, side * 6, 6);
}

void Engine::drawCube(glm::mat4& model) {
  _cube_mesh->draw(_context, model);
}

void Engine::clearGameData(game_info* player) {
  int i, j;

  for (i=0; i<10; i++) {
    for(j=0;j<24;j++) {
      player->board[i][j] = -1;
    }
  }

  player->gameover_position = 0.0f;

  changeState(player, STATE_TETRIS);

  player->score = 0;
  player->state_lines = 0;
  player->break_out_consecutives = 0;

  player1.rot = -BOARD_NORMAL_ROT;
  player1.rot2 = 0;

  player1.pos = 5;
  player1.fine = 0;

  tetris.getNewPiece(&player1);

  inplay = true;
}

void Engine::changeState(game_info* gi, int newState) {
  uninitState(gi);

  // tell networked opponent (IF PLAYER 1)
  if (gi->side == -1) {
    passMessage(MSG_CHANGE_STATE, newState,0,0);
  }

  gi->state = newState;

  // init state (IF PLAYER 1)
  if (gi->side == -1) {
    initState(gi);
  }
}

void Engine::initState(game_info* gi) {
  switch (gi->state) {
    case STATE_BREAKOUT:
      gi->curgame = 1;
      breakout.initGame(&player1);
      break;
    case STATE_TETRIS:
      gi->curgame = 0;
      tetris.initGame(&player1);
      break;
  }
}

void Engine::uninitState(game_info* gi) {
}

void Engine::useTexture(int textureIndex) {
  if (textureIndex < 0 || textureIndex >= texture_count) { return; }

  glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);
  gl_check_errors("glBindTexture");
}

// from tutorial on interwebz:
int Engine::addTexture(const char* fname) {
  GLuint texture;  // This is a handle to our texture object
  SDL_Surface *surface; // This surface will tell us the details of the image
  GLenum texture_format;
  GLint  nOfColors;

  if ( (surface = IMG_Load(fname)) ) {
    // Check that the image's width is a power of 2
    if ( (surface->w & (surface->w - 1)) != 0 ) {
//      printf("warning: image.bmp's width is not a power of 2\n");
    }

    // Also check if the height is a power of 2
    if ( (surface->h & (surface->h - 1)) != 0 ) {
//      printf("warning: image.bmp's height is not a power of 2\n");
    }

    // get the number of channels in the SDL surface
    nOfColors = surface->format->BytesPerPixel;
    if (nOfColors == 4) {    // contains an alpha channel
      if (surface->format->Rmask == 0x000000ff)
        texture_format = GL_RGBA;
      else
        texture_format = GL_BGRA;
    }
    else if (nOfColors == 3) {    // no alpha channel
      if (surface->format->Rmask == 0x000000ff)
        texture_format = GL_RGB;
      else
        texture_format = GL_BGR;
    }
    else {
      printf("warning: the image is not truecolor..  this will probably break\n");
      // this error should not go unhandled
    }

    // Have OpenGL generate a texture object handle for us
    glGenTextures( 1, &texture );
    gl_check_errors("glGenTextures");

    // Bind the texture object
    glActiveTexture(GL_TEXTURE0);
    glBindTexture( GL_TEXTURE_2D, texture );
    gl_check_errors("glBindTexture");

    // Set the texture's stretching properties
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    gl_check_errors("glTexParameteri");

    // Edit the texture object's image data using the information SDL_Surface gives us
#ifdef EMSCRIPTEN
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
#else
    glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
        texture_format, GL_UNSIGNED_BYTE, surface->pixels );
#endif
    gl_check_errors("glTexImage2D");
  }
  else {
    printf("SDL could not load texture: %s\n", SDL_GetError());
    return -1;
  }

  // ADD TEXTURE

  if (textures == NULL) {
    textures = new GLuint[texture_capacity];
    texture_widths = new int[texture_capacity];
    texture_heights = new int[texture_capacity];
    texture_count = 0;
  }

  if (texture_count == texture_capacity) {
    GLuint* tmp = textures;
    int* tmp2 = texture_widths;
    int* tmp3 = texture_heights;

    texture_capacity *= 2;

    textures = new GLuint[texture_capacity];
    texture_widths = new int[texture_capacity];
    texture_heights = new int[texture_capacity];

    memcpy(textures, tmp, sizeof(GLuint) * texture_count);
    memcpy(texture_widths, tmp2, sizeof(int) * texture_count);
    memcpy(texture_heights, tmp3, sizeof(int) * texture_count);

    delete tmp;
    delete tmp2;
    delete tmp3;
  }

  textures[texture_count] = texture;
  texture_widths[texture_count] = surface->w;
  texture_heights[texture_count] = surface->h;
  texture_count++;

  // Free the SDL_Surface only if it was successfully created
  if ( surface ) {
    SDL_FreeSurface( surface );
  }

  return 0;
}

// networking

void Engine::runServer(int port) {
#ifndef NO_NETWORK
  // create a listening TCP socket on port 9999 (server)

  if(SDLNet_ResolveHost(&ip,NULL,port)==-1) {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return;
  }

  tcpsock=SDLNet_TCP_Open(&ip);
  if(!tcpsock) {
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return;
  }

  // WAIT FOR CONNECTION
  printf("waiting for client to connect...\n");

  // accept a connection coming in on tcpsock

  for (;;) {
    client_tcpsock=SDLNet_TCP_Accept(tcpsock);
    if(client_tcpsock) {
      break;
    }
  }

  network_thread = SDL_CreateThread(thread_func, NULL);
#endif
}

void Engine::runClient(char* ipname, int port) {
#ifndef NO_NETWORK
  if(SDLNet_ResolveHost(&ip,ipname,port)==-1) {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return;
  }

  client_tcpsock=SDLNet_TCP_Open(&ip);
  if(!client_tcpsock) {
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return;
  }

  printf("connected...\n");

  // start a thread
  // which will receive and receive!!

  network_thread = SDL_CreateThread(thread_func, NULL);
#endif
}

void Engine::processMessage(unsigned char msg[4]) {
  unsigned char msgID = msg[0];

  //printf("Msg Recv: %d, %d, %d, %d\n", msgID, msg[1], msg[2], msg[3]);

  switch (msgID) {
    case MSG_ADDPIECE: // add piece to tetris Board
      //printf("Msg: ADDPIECE\n");

      tetris.addPiece(&player2, msg[1], msg[2]);
      break;
    case MSG_DROPLINE:
      tetris.dropLine(&player2, msg[1]);
      break;
    case MSG_ATTACK:
      // ATTACK!!!
      performAttack(msg[1]);
      break;
    case MSG_PUSHUP:
      tetris.pushUp(&player2, msg[1]);
      break;
    case MSG_ADDBLOCKS_A:
      tetris.addBlock(&player2, 0, 23, msg[1]);
      tetris.addBlock(&player2, 1, 23, msg[2]);
      tetris.addBlock(&player2, 2, 23, msg[3]);
      break;
    case MSG_ADDBLOCKS_B:
      tetris.addBlock(&player2, 3, 23, msg[1]);
      tetris.addBlock(&player2, 4, 23, msg[2]);
      tetris.addBlock(&player2, 5, 23, msg[3]);
      break;
    case MSG_ADDBLOCKS_C:
      tetris.addBlock(&player2, 6, 23, msg[1]);
      tetris.addBlock(&player2, 7, 23, msg[2]);
      tetris.addBlock(&player2, 8, 23, msg[3]);
      break;
    case MSG_ADDBLOCKS_D:
      tetris.addBlock(&player2, 9, 23, msg[1]);
      break;
    case MSG_ADDBLOCKS2_A:
      tetris.addBlock(&player2, 0, 22, msg[1]);
      tetris.addBlock(&player2, 1, 22, msg[2]);
      tetris.addBlock(&player2, 2, 22, msg[3]);
      break;
    case MSG_ADDBLOCKS2_B:
      tetris.addBlock(&player2, 3, 22, msg[1]);
      tetris.addBlock(&player2, 4, 22, msg[2]);
      tetris.addBlock(&player2, 5, 22, msg[3]);
      break;
    case MSG_ADDBLOCKS2_C:
      tetris.addBlock(&player2, 6, 22, msg[1]);
      tetris.addBlock(&player2, 7, 22, msg[2]);
      tetris.addBlock(&player2, 8, 22, msg[3]);
      break;
    case MSG_ADDBLOCKS2_D:
      tetris.addBlock(&player2, 9, 22, msg[1]);
      break;
    case MSG_UPDATEPIECE:
      player2.pos = msg[1];
      player2.curdir = msg[2];
      player2.curpiece = msg[3];
      break;

    case MSG_UPDATEPIECEY:
      player2.fine = ((float)msg[1] / 255.0f) * 11.0f;
      break;

    case MSG_ROT_BOARD:
      player2.rot = ((float)msg[1] / 255.0f) * 360.0f;
      break;

    case MSG_CHANGE_STATE:
      changeState(&player2, msg[1]);
      break;

    case MSG_ROT_BOARD2:
      player2.rot2 = ((float)msg[1] / 255.0f) * 180.0f;
      break;

    case MSG_UPDATEBALL:
      player2.ball_x = ((float)msg[1] / 255.0f) * 20.0f;
      player2.ball_y = ((float)msg[2] / 255.0f) * 20.0f;
      break;

    case MSG_UPDATEPADDLE:
      player2.fine = ((float)msg[1] / 255.0f) * 11.0f;
      break;

    case MSG_REMOVEBLOCK:
      player2.board[msg[1]][msg[2]] = -1;
      break;

    case MSG_GAMEOVER:
      inplay = false;
      displayMessage(STR_YOUWIN);
      break;

    case MSG_APPENDSCORE:
      player2.score += ((int)msg[1] * (int)msg[2]);
      break;
  }
}

void Engine::passMessage(unsigned char msgID, unsigned char p1, unsigned char p2, unsigned char p3) {
#ifndef NO_NETWORK
  if (!client_tcpsock) { return; }

  int len,result;
  char msg[4]={msgID,p1,p2,p3};

  len=(int)strlen(msg)+1; // add one for the terminating NULL
  result=SDLNet_TCP_Send(client_tcpsock,msg,4);
  if(result<len) {
    //printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    // It may be good to disconnect sock because it is likely invalid now.
  }
#endif
}

// classes

Game* Engine::games[] = { &Engine::tetris, &Engine::breakout };
int Engine::gamecount = sizeof(Engine::games) / sizeof(Game*);

Tetris Engine::tetris = Tetris();
BreakOut Engine::breakout = BreakOut();

Audio Engine::audio = Audio();

game_info Engine::player2 = {0};
game_info Engine::player1 = {0};

int Engine::_quit = 0;

GLuint* Engine::textures = NULL;
int* Engine::texture_widths = NULL;
int* Engine::texture_heights = NULL;
int Engine::texture_count = 0;
int Engine::texture_capacity = 10;

GLfloat Engine::tu[2] = {0.0f, 1.0f};
GLfloat Engine::tv[2] = {0.0f, 1.0f};

double Engine::time = 0;
double Engine::repeatTime = 0;

int Engine::keys[0xffff] = {0};

float Engine::bg1x = 0;
float Engine::bg1y = 0;

float Engine::bg2x = 0;
float Engine::bg2y = 0;

float Engine::bg_tile_opacity = 0.2f;
bool  Engine::bg_tile_opacity_direction = true;

#ifndef NO_NETWORK
IPaddress Engine::ip = {0};
TCPsocket Engine::tcpsock = NULL;
TCPsocket Engine::client_tcpsock = NULL;
#endif

SDL_Thread *Engine::network_thread = NULL;

int Engine::inplay = 1;
