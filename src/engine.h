#ifndef ENGINE_INCLUDED
#define ENGINE_INCLUDED

#include "main.h"

#include "context.h"
#include "mesh.h"

#include "tetris.h"
#include "breakout.h"

#include "audio.h"

#include "glm/glm.hpp"

class Engine {
public:
  Engine();
  ~Engine();

  // init, uninit

  void init();
  void quit();

  // network

  void runServer(int port);
  void runClient(char* ip, int port);

  void processMessage(unsigned char msg[4]);
  void passMessage(unsigned char msgID, unsigned char p1, unsigned char p2, unsigned char p3);

  // game loop

  void gameLoop();

  void gameOver();

  bool _iterate();
  static void _c_iterate();

  // frame stuffs

  int intLength(int i);
  int drawInt(int i, int color, float x, float y);
  int _drawString(const char* str, int color, float x, float y, int tx);
  int drawString(const char* str, int color, float x, float y);
  int drawStringWhite(const char* str, int color, float x, float y);

  void update(float deltatime);
  void draw();

  void drawMesh(int count);

  // key processing

  void keyDown(Uint32 key);
  void keyUp(Uint32 key);

  // mouse

  void mouseMovement(Uint32 x, Uint32 y);
  void mouseDown();

  // graphics

  void drawCube(glm::mat4& model);
  void drawQuadXY(float x, float y, float z, float w, float h);
  void drawQuad(glm::mat4& model, int side);

  // state

  int state;

  void changeState(game_info* gi, int newState);
  void initState(game_info* gi);
  void uninitState(game_info* gi);

  // common game logic

  void clearGameData(game_info* player);

  // textures

  void useTexture(int textureIndex);
  void useTextureUpsideDown(int textureIndex, int startx, int starty, int width, int height);
  void enableTextures();
  void disableTextures();
  int addTexture(const char* fname);

  void sendAttack(int severity);
  void performAttack(int severity);

  void displayMessage(int stringIndex);

  Context* _context;

  Mesh*    _cube_mesh;
  Mesh*    _hud_mesh;
  Mesh*    _ship_mesh;

  // vars

  static int gamecount;
  static Game* games[];

  static Tetris tetris;
  static BreakOut breakout;
  static Audio audio;

  static game_info player1;
  static game_info player2;

  static int _quit;

  static int inplay;

  static GLuint* textures;
  static int* texture_widths;
  static int* texture_heights;
  static int texture_count;
  static int texture_capacity;

  static GLfloat tu[2];
  static GLfloat tv[2];

  static double repeatTime;
  static double time;

  static int keys[0xffff];

  // background
  static float bg1x;
  static float bg1y;

  static float bg2x;
  static float bg2y;

  // board background tile
  static float bg_tile_opacity;
  static bool bg_tile_opacity_direction;

  // networking

#ifndef NO_NETWORK
  static IPaddress ip;
  static TCPsocket tcpsock;
  static TCPsocket client_tcpsock;
#endif
  static SDL_Thread *network_thread;
};
#endif //ENGINE_INCLUDED
