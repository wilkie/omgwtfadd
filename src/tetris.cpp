#include "main.h"
#include "tetris.h"
#include "components.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

void Tetris::Update(game_info* gi, float deltatime) {
  if (gi->state == STATE_TETRIS_TRANS) {
    gi->rot2 += TRANSITION_SPEED * deltatime;

    if (gi->rot2 >= 180) {
      gi->rot2 = 180;
      engine.ChangeState(gi, STATE_BREAKOUT);
    }

    engine.PassMessage(MSG_ROT_BOARD2, (gi->rot2 / 180.0f) * 255.0f, 0,0);
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
      engine.PassMessage(MSG_APPENDSCORE, 100, 10, 0);
    }

    engine.PassMessage(MSG_ROT_BOARD, (gi->rot / 360.0f) * 255.0f, 0,0);
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

  if (TestCollision(gi)) {
    // we collided! oh no!
    if (TestGameOver(gi)) {
      engine.GameOver();
    }
    else {
      AddPiece(gi);
    }
  }

  engine.PassMessage(MSG_UPDATEPIECEY, (unsigned char)((gi->fine / 11.0f) * 255.0f), 0, 0);
}

void Tetris::DropLine(game_info* gi, int lineIndex) {
  int j,i;
  for (i=0; i<10; i++) {
    for (j=lineIndex; j>1;j--) {
      gi->board[i][j] = gi->board[i][j-1];
    }
  }

  if (gi->side == -1) {
    engine.PassMessage(MSG_DROPLINE, (unsigned char)lineIndex, 0,0);
  }
}

int Tetris::ClearLines(game_info* gi) {
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
      engine.PassMessage(MSG_APPENDSCORE, 100, lines, 0);
      DropLine(gi,j);

      engine.audio.PlaySound(SND_TINK);
    }
  }

  return lines;
}

// init!
void Tetris::InitGame(game_info* gi) {
}

void Tetris::DropPiece(game_info* gi) {
  gi->fine = DetermineDropPosition(gi);

  engine.PassMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
  engine.PassMessage(MSG_UPDATEPIECEY, (unsigned char)((gi->fine / 11.0f) * 255.0f), 0, 0);

  AddPiece(gi);
}

float Tetris::DetermineDropPosition(game_info* gi) {
  float phantom_fine = gi->fine;
  while(!TestCollision(gi, (0.5) * (double)gi->pos, phantom_fine)) {
    phantom_fine+=0.25;
  }

  return phantom_fine - fmod(phantom_fine, 0.5f);
}

// draw 3D
void Tetris::Draw(game_info* gi) {
  if (!engine.network_thread && gi->side == 1) {
    return;
  }

  DrawBoard(gi);

  DrawPiece(gi, (0.5) * (double)gi->pos, gi->fine, gi->curpiece);

  if (gi->state == STATE_TETRIS) {
    DrawPiece(gi, (0.5) * (double)gi->pos, DetermineDropPosition(gi), 18);
  }
}

void Tetris::DrawBlock(int type, game_info* gi, double x, double y) {
  engine.UseTexture(type, 0, 0, 32,32);

  // translate to world
  glm::mat4 model = glm::mat4(1.0f);

  //model = glm::translate(model, glm::vec3(gi->side * 5.0f, 0.0f, 0.0f));

  // rotate
  model = glm::rotate(model, gi->side * gi->rot, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, -gi->rot2, glm::vec3(1.0f,0.0f,0.0f));

  model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));

  // translate
  model = glm::translate(model, glm::vec3(-2.25f + (x), 6.375f - (y), 0.0f));

  // scale (make them 0.5 unit cubes, since our unit cube is 2x2x2)
  model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));

  glUniformMatrix4fv(engine._model_uniform, 1, GL_FALSE, &model[0][0]);

  engine.DrawCube();
}

// draw interface
void Tetris::DrawOrtho(game_info* gi) {
  if (!engine.network_thread && gi->side == 1) {
    int y = 200;
    engine.DrawString("SINGLE PLAYER GAME", 1, 440, y);
    y+= 40;
    engine.DrawString("FOR MULTIPLAYER", 2, 440, y);
    y+= 40;
    engine.DrawString("HOST USING COMMAND", 2, 440, y);
    y+= 20;
    engine.DrawString("OMGWTFADD -S -P 2000", 4, 440, y);
    y+= 40;
    engine.DrawString("CONNECT USING COMMAND", 2, 440, y);
    y+= 20;
    engine.DrawString("OMGWTFADD -P 2000", 4, 440, y);
    y+= 20;
    engine.DrawString("127.0.0.1", 4, 475, y);

    y += 60;
    engine.DrawString("LEVEL: ", 5, 440, y);
    engine.DrawInt(LEVEL, 1, 440 + (16 * 6), y);

    return;
  }

  if (gi->side == -1) {
    engine.DrawString(":LINES", 0,(400.0f - 32.0f) - (6 * 16),25.0f);

    int x  = engine.IntLength(gi->total_lines);

    engine.DrawInt(gi->total_lines, 1, (400.0f - 32.0f) - ((6 + x) * 16), 25.0f);

    int x2;

    x2 = (400 - (16 * 16)) / 2;

    engine.DrawString("COMPLETE", 2, x2, 600.0f - 25.0f);
    x2 += (8 * 16);
    engine.DrawInt(TETRIS_LINES_NEEDED - gi->state_lines, 3, x2, 600.0f - 25.0f);
    x2 += (engine.IntLength(TETRIS_LINES_NEEDED - gi->state_lines) * 16);
    engine.DrawString("LINES", 2, x2, 600.0f - 25.0f);
  }
}

bool Tetris::TestGameOver(game_info* gi) {
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

void Tetris::AddPiece(game_info* gi) {
  int start_y;
  int start_x =  gi->pos;

  start_y = (gi->fine) / (0.5);

  AddPiece(gi, start_x, start_y);
}

void Tetris::AddPiece(game_info* gi, int start_x, int start_y) {
  switch (gi->curpiece) {
    case 0:
      if (gi->curdir % 2) {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x-1, start_y, gi->curpiece);
        AddBlock(gi, start_x-2, start_y, gi->curpiece);
        AddBlock(gi, start_x+1, start_y, gi->curpiece);
      }
      else {
        AddBlock(gi, start_x, start_y+1, gi->curpiece);
        AddBlock(gi, start_x, start_y+2, gi->curpiece);
        AddBlock(gi, start_x, start_y-1, gi->curpiece);
        AddBlock(gi, start_x, start_y, gi->curpiece);
      }
      break;
    case 1:
      if (gi->curdir % 2) {
        AddBlock(gi, start_x+1, start_y, gi->curpiece);
        AddBlock(gi, start_x+1, start_y-1, gi->curpiece);
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y+1, gi->curpiece);
      }
      else {
        AddBlock(gi, start_x+1, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y-1, gi->curpiece);
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x-1, start_y-1, gi->curpiece);
      }
      break;
    case 2:
      if (gi->curdir % 2) {
        AddBlock(gi, start_x-1, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y+1, gi->curpiece);
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x-1, start_y-1, gi->curpiece);
      }
      else {
        AddBlock(gi, start_x-1, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y-1, gi->curpiece);
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x+1, start_y-1, gi->curpiece);
      }
      break;
    case 3:
      AddBlock(gi, start_x+1, start_y, gi->curpiece);
      AddBlock(gi, start_x, start_y+1, gi->curpiece);
      AddBlock(gi, start_x, start_y, gi->curpiece);
      AddBlock(gi, start_x+1, start_y+1, gi->curpiece);
      break;
    case 4:
      if (gi->curdir == 0) {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y+1, gi->curpiece);
        AddBlock(gi, start_x, start_y-1, gi->curpiece);
        AddBlock(gi, start_x+1, start_y+1, gi->curpiece);
      }
      else if (gi->curdir == 1) {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x+1, start_y, gi->curpiece);
        AddBlock(gi, start_x-1, start_y, gi->curpiece);
        AddBlock(gi, start_x-1, start_y+1, gi->curpiece);
      }
      else if (gi->curdir == 2) {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y+1, gi->curpiece);
        AddBlock(gi, start_x, start_y-1, gi->curpiece);
        AddBlock(gi, start_x-1, start_y-1, gi->curpiece);
      }
      else {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x+1, start_y, gi->curpiece);
        AddBlock(gi, start_x-1, start_y, gi->curpiece);
        AddBlock(gi, start_x+1, start_y-1, gi->curpiece);
      }
      break;
    case 5:
      if (gi->curdir == 0) {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y+1, gi->curpiece);
        AddBlock(gi, start_x, start_y-1, gi->curpiece);
        AddBlock(gi, start_x-1, start_y+1, gi->curpiece);
      }
      else if (gi->curdir == 1) {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x+1, start_y, gi->curpiece);
        AddBlock(gi, start_x-1, start_y, gi->curpiece);
        AddBlock(gi, start_x-1, start_y-1, gi->curpiece);
      }
      else if (gi->curdir == 2) {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y+1, gi->curpiece);
        AddBlock(gi, start_x, start_y-1, gi->curpiece);
        AddBlock(gi, start_x+1, start_y-1, gi->curpiece);
      }
      else {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x+1, start_y, gi->curpiece);
        AddBlock(gi, start_x-1, start_y, gi->curpiece);
        AddBlock(gi, start_x+1, start_y+1, gi->curpiece);
      }
      break;
    case 6:
      if (gi->curdir == 0) {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y-1, gi->curpiece);
        AddBlock(gi, start_x+1, start_y, gi->curpiece);
        AddBlock(gi, start_x-1, start_y, gi->curpiece);
      }
      else if (gi->curdir == 1) {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y+1, gi->curpiece);
        AddBlock(gi, start_x+1, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y-1, gi->curpiece);
      }
      else if (gi->curdir == 2) {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x-1, start_y, gi->curpiece);
        AddBlock(gi, start_x+1, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y+1, gi->curpiece);
      }
      else {
        AddBlock(gi, start_x, start_y, gi->curpiece);
        AddBlock(gi, start_x-1, start_y, gi->curpiece);
        AddBlock(gi, start_x, start_y-1, gi->curpiece);
        AddBlock(gi, start_x, start_y+1, gi->curpiece);
      }
      break;

  }

  if (gi->side == -1) {
    engine.PassMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
    //engine.PassMessage(MSG_UPDATEPIECEY, , 0, 0);
    engine.PassMessage(MSG_ADDPIECE, start_x, start_y,0);

    int lines = ClearLines(gi);

    gi->state_lines += lines;
    gi->total_lines += lines;

    GetNewPiece(gi);

    if (lines > 1) {
      engine.SendAttack(lines-1);
    }

    if (gi->state_lines >= TETRIS_LINES_NEEDED) {
      gi->state_lines = 0;

      engine.DisplayMessage(STR_TRANSITION);
      engine.audio.PlaySound(SND_CHANGEVIEW);
      engine.ChangeState(gi, STATE_TETRIS_TRANS);
    }
  }
}

void Tetris::AddBlock(game_info* gi, int i, int j, int type) {
  gi->board[i][j] = type;
}

void Tetris::DrawBoard(game_info* gi) {
  engine.UseTexture(16, 0, 0, 32,32);

  // left
  glm::mat4 model;

  model = glm::mat4(1.0f);
//  model = glm::translate(model, glm::vec3(gi->side * 5.0f, 0.0f, 0.0f));

  // rotate
  model = glm::rotate(model, gi->side * gi->rot, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, -gi->rot2, glm::vec3(1.0f,0.0f,0.0f));

  model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));

  glm::mat4 base = model;

  model = glm::translate(model, glm::vec3(-2.625,-0.125,0));
  model = glm::scale(model, glm::vec3(0.25,11,0.5));
  model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

  glUniformMatrix4fv(engine._model_uniform, 1, GL_FALSE, &model[0][0]);
  engine.DrawCube();

  // right
  model = base;
  model = glm::translate(model, glm::vec3(2.625,-0.125,0));
  model = glm::scale(model, glm::vec3(0.25,11,0.5));
  model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

  glUniformMatrix4fv(engine._model_uniform, 1, GL_FALSE, &model[0][0]);
  engine.DrawCube();

  // bottom
  model = base;
  model = glm::translate(model, glm::vec3(0,-5.5,0));
  model = glm::scale(model, glm::vec3(5,0.25,0.5));
  model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

  glUniformMatrix4fv(engine._model_uniform, 1, GL_FALSE, &model[0][0]);
  engine.DrawCube();

  int i,j;

  for (i=0; i<10; i++) {
    for (j=0; j<24; j++) {
      if(gi->board[i][j] != -1) {
        DrawBlock(gi->board[i][j], gi, 0.5 * (double)i, 0.5 * (double)j);
      }
    }
  }

  for (i=0; i<10; i++) {
    for (j=0; j<24; j++) {
      if(gi->board[i][j] == -1) {
        DrawBackgroundBlock(gi, i, j);
      }
    }
  }
}

void Tetris::DrawBackgroundBlock(game_info* gi, double x, double y) {
  engine.UseTexture(17,0,0,0,0);

  // translate to world
  glm::mat4 model = glm::mat4(1.0f);

  //model = glm::translate(model, glm::vec3(gi->side * 5.0f, 0.0f, 0.0f));

  // rotate
  model = glm::rotate(model, gi->side * gi->rot, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, -gi->rot2, glm::vec3(1.0f,0.0f,0.0f));

  model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));

  float z = -0.8f;
  float rot_percent = (float)gi->rot2 / 180.0f;

  z = 1.0f * rot_percent - 0.8f;

  // translate
  model = glm::translate(model, glm::vec3(-2.25f + (x*0.5), 6.375f - (y*0.5), z));

  // scale (make them 0.5 unit cubes, since our unit cube is 2x2x2)
  model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));

  glUniformMatrix4fv(engine._model_uniform, 1, GL_FALSE, &model[0][0]);

  engine.DrawQuad(0,0,0,0);
}

void Tetris::DrawPiece(game_info* gi, double x, double y, int texture) {
  switch (gi->curpiece) {
    case 0:
      if (gi->curdir % 2) {
        DrawBlock(texture, gi, x-0.5, y);
        DrawBlock(texture, gi, x-1.0, y);
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x+0.5, y);
      }
      else {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x, y+0.5);
        DrawBlock(texture, gi, x, y+1.0);
        DrawBlock(texture, gi, x, y-0.5);
      }
      break;
    case 1:
      if (gi->curdir % 2) {
        DrawBlock(texture, gi, x+0.5, y);
        DrawBlock(texture, gi, x, y+0.5);
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x+0.5, y-0.5);
      }
      else {
        DrawBlock(texture, gi, x+0.5, y);
        DrawBlock(texture, gi, x, y-0.5);
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x-0.5, y-0.5);
      }
      break;
    case 2:
      if (gi->curdir % 2) {
        DrawBlock(texture, gi, x-0.5, y);
        DrawBlock(texture, gi, x, y+0.5);
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x-0.5, y-0.5);
      }
      else {
        DrawBlock(texture, gi, x-0.5, y);
        DrawBlock(texture, gi, x, y-0.5);
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x+0.5, y-0.5);
      }
      break;
    case 3:
      DrawBlock(texture, gi, x+0.5, y);
      DrawBlock(texture, gi, x, y+0.5);
      DrawBlock(texture, gi, x, y);
      DrawBlock(texture, gi, x+0.5, y+0.5);
      break;
    case 4:
      if (gi->curdir == 0) {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x, y+0.5);
        DrawBlock(texture, gi, x, y-0.5);
        DrawBlock(texture, gi, x+0.5, y+0.5);
      }
      else if (gi->curdir == 1) {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x+0.5, y);
        DrawBlock(texture, gi, x-0.5, y);
        DrawBlock(texture, gi, x-0.5, y+0.5);
      }
      else if (gi->curdir == 2) {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x, y+0.5);
        DrawBlock(texture, gi, x, y-0.5);
        DrawBlock(texture, gi, x-0.5, y-0.5);
      }
      else {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x+0.5, y);
        DrawBlock(texture, gi, x-0.5, y);
        DrawBlock(texture, gi, x+0.5, y-0.5);
      }
      break;
    case 5:
      if (gi->curdir == 0) {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x, y+0.5);
        DrawBlock(texture, gi, x, y-0.5);
        DrawBlock(texture, gi, x-0.5, y+0.5);
      }
      else if (gi->curdir == 1) {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x+0.5, y);
        DrawBlock(texture, gi, x-0.5, y);
        DrawBlock(texture, gi, x-0.5, y-0.5);
      }
      else if (gi->curdir == 2) {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x, y+0.5);
        DrawBlock(texture, gi, x, y-0.5);
        DrawBlock(texture, gi, x+0.5, y-0.5);
      }
      else {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x+0.5, y);
        DrawBlock(texture, gi, x-0.5, y);
        DrawBlock(texture, gi, x+0.5, y+0.5);
      }
      break;
    case 6:
      if (gi->curdir == 0) {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x+0.5, y);
        DrawBlock(texture, gi, x-0.5, y);
        DrawBlock(texture, gi, x, y-0.5);
      }
      else if (gi->curdir == 1) {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x+0.5, y);
        DrawBlock(texture, gi, x, y+0.5);
        DrawBlock(texture, gi, x, y-0.5);
      }
      else if (gi->curdir == 2) {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x+0.5, y);
        DrawBlock(texture, gi, x-0.5, y);
        DrawBlock(texture, gi, x, y+0.5);
      }
      else {
        DrawBlock(texture, gi, x, y);
        DrawBlock(texture, gi, x-0.5, y);
        DrawBlock(texture, gi, x, y+0.5);
        DrawBlock(texture, gi, x, y-0.5);
      }
      break;
  }
}

bool Tetris::TestCollisionBlock(game_info* gi, double x, double y) {
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

bool Tetris::TestCollision(game_info *gi) {
  return TestCollision(gi, (0.5) * (double)gi->pos, gi->fine);
}

double Tetris::TestSideCollision(game_info* gi, double x, double y) {
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

bool Tetris::TestCollision(game_info* gi, double x, double y) {
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

  double ret = TestSideCollision(gi, x, y);

  if (ret!=-10) {
    return 1;
  }

  // COLLISION AMONG BLOCKS!

  bool coll = false;

  switch (gi->curpiece)
  {
    case 0:
      if (gi->curdir % 2) {
        coll |= TestCollisionBlock(gi, x-0.5, y);
        coll |= TestCollisionBlock(gi, x-1.0, y);
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x+0.5, y);
      }
      else {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x, y+0.5);
        coll |= TestCollisionBlock(gi, x, y+1.0);
        coll |= TestCollisionBlock(gi, x, y-0.5);
      }
      break;
    case 1:
      if (gi->curdir % 2) {
        coll |= TestCollisionBlock(gi, x+0.5, y);
        coll |= TestCollisionBlock(gi, x, y+0.5);
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x+0.5, y-0.5);
      }
      else {
        coll |= TestCollisionBlock(gi, x+0.5, y);
        coll |= TestCollisionBlock(gi, x, y-0.5);
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x-0.5, y-0.5);
      }
      break;
    case 2:
      if (gi->curdir % 2) {
        coll |= TestCollisionBlock(gi, x-0.5, y);
        coll |= TestCollisionBlock(gi, x, y+0.5);
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x-0.5, y-0.5);
      }
      else {
        coll |= TestCollisionBlock(gi, x-0.5, y);
        coll |= TestCollisionBlock(gi, x, y-0.5);
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x+0.5, y-0.5);
      }
      break;
    case 3:
      coll |= TestCollisionBlock(gi, x+0.5, y);
      coll |= TestCollisionBlock(gi, x, y+0.5);
      coll |= TestCollisionBlock(gi, x, y);
      coll |= TestCollisionBlock(gi, x+0.5, y+0.5);
      break;
    case 4:
      if (gi->curdir == 0) {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x, y+0.5);
        coll |= TestCollisionBlock(gi, x, y-0.5);
        coll |= TestCollisionBlock(gi, x+0.5, y+0.5);
      }
      else if (gi->curdir == 1) {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x+0.5, y);
        coll |= TestCollisionBlock(gi, x-0.5, y);
        coll |= TestCollisionBlock(gi, x-0.5, y+0.5);
      }
      else if (gi->curdir == 2) {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x, y+0.5);
        coll |= TestCollisionBlock(gi, x, y-0.5);
        coll |= TestCollisionBlock(gi, x-0.5, y-0.5);
      }
      else {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x+0.5, y);
        coll |= TestCollisionBlock(gi, x-0.5, y);
        coll |= TestCollisionBlock(gi, x+0.5, y-0.5);
      }
      break;
    case 5:
      if (gi->curdir == 0) {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x, y+0.5);
        coll |= TestCollisionBlock(gi, x, y-0.5);
        coll |= TestCollisionBlock(gi, x-0.5, y+0.5);
      }
      else if (gi->curdir == 1) {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x+0.5, y);
        coll |= TestCollisionBlock(gi, x-0.5, y);
        coll |= TestCollisionBlock(gi, x-0.5, y-0.5);
      }
      else if (gi->curdir == 2) {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x, y+0.5);
        coll |= TestCollisionBlock(gi, x, y-0.5);
        coll |= TestCollisionBlock(gi, x+0.5, y-0.5);
      }
      else {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x+0.5, y);
        coll |= TestCollisionBlock(gi, x-0.5, y);
        coll |= TestCollisionBlock(gi, x+0.5, y+0.5);
      }
      break;
    case 6:
      if (gi->curdir == 0) {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x+0.5, y);
        coll |= TestCollisionBlock(gi, x-0.5, y);
        coll |= TestCollisionBlock(gi, x, y-0.5);
      }
      else if (gi->curdir == 1) {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x+0.5, y);
        coll |= TestCollisionBlock(gi, x, y+0.5);
        coll |= TestCollisionBlock(gi, x, y-0.5);
      }
      else if (gi->curdir == 2) {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x+0.5, y);
        coll |= TestCollisionBlock(gi, x-0.5, y);
        coll |= TestCollisionBlock(gi, x, y+0.5);
      }
      else {
        coll |= TestCollisionBlock(gi, x, y);
        coll |= TestCollisionBlock(gi, x-0.5, y);
        coll |= TestCollisionBlock(gi, x, y+0.5);
        coll |= TestCollisionBlock(gi, x, y-0.5);
      }
      break;
  }

  return coll;
}

void Tetris::KeyRepeat(game_info* gi) {
  if (gi->state != STATE_TETRIS) {
    return;
  }

  if (engine.keys[SDLK_UP]) {
    gi->curdir++;
    gi->curdir %= 4;

    if (TestCollision(gi)) {
      gi->curdir--;
      gi->curdir += 4;
      gi->curdir %= 4;
    }

    engine.PassMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
  }
  else if (engine.keys[SDLK_LEFT]) {
    gi->pos--;

    // test collisions!
    if (TestCollision(gi)) {
      gi->pos++;
    }

    engine.PassMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
  }
  else if (engine.keys[SDLK_RIGHT]) {
    gi->pos++;


    // test collisions!
    if (TestCollision(gi)) {
      gi->pos--;
    }

    engine.PassMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
  }
}

void Tetris::KeyDown(game_info* gi, Uint32 key) {
  if (gi->state != STATE_TETRIS) {
    return;
  }

  if (key == SDLK_SPACE) {
    DropPiece(gi);
  }
}

void Tetris::KeyUp(game_info* gi, Uint32 key) {
}

void Tetris::MouseMovement(game_info* gi, Uint32 x, Uint32 y) {
}

void Tetris::MouseDown(game_info* gi) {
}

void Tetris::GetNewPiece(game_info* gi) {
  gi->curpiece = rand() % 7;
  gi->curdir = 1;

  gi->pos = 5;
  gi->fine = 1.0;

  engine.PassMessage(MSG_UPDATEPIECE, gi->pos, gi->curdir, gi->curpiece);
  engine.PassMessage(MSG_UPDATEPIECEY, 0, 0, 0);
}

void Tetris::PushUp(game_info* gi, int num) {
  int i,j;
  for (j=0; j<(24-num); j++) {
    for (i=0; i<10; i++) {
      gi->board[i][j] = gi->board[i][j+num];
    }
  }
}

void Tetris::Attack(game_info* gi, int severity) {
  // add a line!
  // add two lines!!
  // rotate board!!!

  int gameover = 0;
  int good = 0;

  engine.audio.PlaySound(SND_ADDLINE);

  if (severity == 1) {
    // ok dokey
    int i;

    for (i=0; i<10; i++) {
      if (gi->board[i][0] != -1) {
        // game over!
        gameover = 1;
        break;
      }
    }

    PushUp(gi, 1);

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
    engine.PassMessage(MSG_PUSHUP, 1,0,0);
    engine.PassMessage(MSG_ADDBLOCKS_A,gi->board[0][23], gi->board[1][23],gi->board[2][23]);
    engine.PassMessage(MSG_ADDBLOCKS_B,gi->board[3][23], gi->board[4][23],gi->board[5][23]);
    engine.PassMessage(MSG_ADDBLOCKS_C,gi->board[6][23], gi->board[7][23],gi->board[8][23]);
    engine.PassMessage(MSG_ADDBLOCKS_D,gi->board[9][23], 0,0);
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

    PushUp(gi, 2);

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
    engine.PassMessage(MSG_PUSHUP, 2,0,0);
    engine.PassMessage(MSG_ADDBLOCKS_A,gi->board[0][23], gi->board[1][23],gi->board[2][23]);
    engine.PassMessage(MSG_ADDBLOCKS_B,gi->board[3][23], gi->board[4][23],gi->board[5][23]);
    engine.PassMessage(MSG_ADDBLOCKS_C,gi->board[6][23], gi->board[7][23],gi->board[8][23]);
    engine.PassMessage(MSG_ADDBLOCKS_D,gi->board[9][23], 0,0);
    engine.PassMessage(MSG_ADDBLOCKS2_A,gi->board[0][22], gi->board[1][22],gi->board[2][22]);
    engine.PassMessage(MSG_ADDBLOCKS2_B,gi->board[3][22], gi->board[4][22],gi->board[5][22]);
    engine.PassMessage(MSG_ADDBLOCKS2_C,gi->board[6][22], gi->board[7][22],gi->board[8][22]);
    engine.PassMessage(MSG_ADDBLOCKS2_D,gi->board[9][22], 0,0);
  }
  else if (severity == 3) {
    // rotate!
    gi->attacking = 1;
    gi->attack_rot = 0;
  }

  if (gameover) {
    engine.GameOver();
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
