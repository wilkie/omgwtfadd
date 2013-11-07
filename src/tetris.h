#ifndef TETRIS_INCLUDED
#define TETRIS_INCLUDED

#include "game.h"

class Tetris : public Game {
public:
  // conventions:
  void update(game_info* gi, float deltatime);
  void draw(game_info* gi);
  void drawOrtho(game_info* gi);

  void keyDown(game_info* gi, Uint32 key);
  void keyUp(game_info* gi, Uint32 key);
  void keyRepeat(game_info* gi);

  void attack(game_info* gi, int severity);

  void mouseMovement(game_info* gi, Uint32 x, Uint32 y);
  void mouseDown(game_info* gi);

  void initGame(game_info* gi);


  // stuffs:

  void getNewPiece(game_info* gi);

  void drawBoard(game_info* gi);
  void drawPiece(game_info* gi, double x, double y, int texture);
  void drawBackgroundBlock(game_info* gi, double x, double y);
  void drawBlock(int type, game_info* gi, double x, double y, bool hasLeft,
                                                              bool hasRight,
                                                              bool hasTop,
                                                              bool hasBottom);

  float determineDropPosition(game_info* gi);

  void addPiece(game_info* gi);
  void addPiece(game_info* gi, int start_x, int start_y);
  void addBlock(game_info* gi, int i, int j, int type);

  void dropPiece(game_info* gi);

  int clearLines(game_info* gi);

  void pushUp(game_info* gi, int num);
  void dropLine(game_info* gi, int lineIndex);

  bool testGameOver(game_info* gi);

  bool testCollisionBlock(game_info* gi, double x, double y);
  bool testCollision(game_info* gi);
  bool testCollision(game_info* gi, double x, double y);

  double testSideCollision(game_info* gi, double x, double y);

  // materials:

  // board posts:
  static GLfloat board_piece_amb[4];
  static GLfloat board_piece_diff[4];
  static GLfloat board_piece_spec[4];
  static GLfloat board_piece_shine;
  static GLfloat board_piece_emi[4];

  // block materials
  static GLfloat tet_piece_amb[4];
  static GLfloat tet_piece_diff[6][4];
  static GLfloat tet_piece_spec[4];
  static GLfloat tet_piece_emi[4];
  static GLfloat tet_piece_shine;
};
#endif
