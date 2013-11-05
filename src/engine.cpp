#include "main.h"
#include "engine.h"
#include "components.h"

#include <math.h>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

void gl_check_errors(const char* msg) {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    fprintf(stderr, "GL Error: %s: %s\n", msg, (char*)gluErrorString(error));
  }
}

const char* frag_shader_code[] = {
  "#version 150\n"
  "\n"
  "in vec3 Position;"
  "in vec3 Normal;"
  "in vec2 Texcoord;"
  ""
  "out vec4 outColor;"
  ""
  "uniform sampler2D tex;"
  ""
  "void main() {"
  "  outColor = texture2D(tex, Texcoord);"
  "}"
};

const char* vertex_shader_code[] = {
  "#version 150\n"
  "\n"
  "in vec3 normal;"
  "in vec3 position;"
  "in vec2 texcoord;"
  ""
  "out vec2 Texcoord;"
  "out vec3 Normal;"
  "out vec3 Position;"
  ""
  "uniform mat4 model;"
  "uniform mat4 view;"
  "uniform mat4 proj;"
  ""
  "uniform vec3 camera;"
  ""
  "void main() {"
  "  Texcoord = texcoord;"
  "  Normal = (model * vec4(normal, 1.0)).xyz;"
  "  Position = (model * vec4(position, 1.0)).xyz;"
  ""
  "  gl_Position = proj * view * model * vec4(position, 1.0);"
  "}"
};

const char* stuffs[] = {
  "OMGWTFADD!!! It is ultra cool!!! TRUST ME!", "OMGWTFADD!",
  "Do Your Part..Get your penguin spayed or neutered....",
  "Music: Stabilizer Feat. Captain Dan and the Scurvy Crew: We, Conquistadors",
  "Music: Visit Stabilizer's Website and Album Information: http://www.nonexistent-recordings.com/artists/",
  "WTF! pirate rap?/? LOLZ",
  "Space Pirate Penguins??? ROFL!",
  "HAHAHAHAHA! YOU SUCK! LOOOOLLL!!!factorial!!!!",
  "Music: Stabilizer Feat. Captain Dan and the Scurvy Crew: We, Conquistadors",
  "Music: Visit Stabilizer's Website and Album Information: http://www.nonexistent-recordings.com/artists/",
};

int num_captions = sizeof(stuffs) / sizeof(char*);

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
      engine.ProcessMessage(msg);
    }
  }

  return(0);
}
#endif

Engine::Engine() {
}

Engine::~Engine() {
}

static const GLfloat _cube_data[] = {
  // Face v0-v1-v2-v3
   1, 1, 1, 0, 0, 1, 0, 0,
  -1, 1, 1, 0, 0, 1, 1, 0,
  -1,-1, 1, 0, 0, 1, 1, 1,
   1,-1, 1, 0, 0, 1, 0, 1,
  // Face v0-v3-v4-v5
   1, 1, 1, 1, 0, 0, 0, 0,
   1,-1, 1, 1, 0, 0, 0, 1,
   1,-1,-1, 1, 0, 0, 1, 1,
   1, 1,-1, 1, 0, 0, 1, 0,
  // Face v0-v5-v6-v1
   1, 1, 1, 0, 1, 0, 0, 0,
   1, 1,-1, 0, 1, 0, 0, 1,
  -1, 1,-1, 0, 1, 0, 1, 1,
  -1, 1, 1, 0, 1, 0, 1, 0,
  // Face v1-v6-v7-v2
  -1, 1, 1,-1, 0, 0, 0, 0,
  -1, 1,-1,-1, 0, 0, 0, 1,
  -1,-1,-1,-1, 0, 0, 1, 1,
  -1,-1, 1,-1, 0, 0, 1, 0,
  // Face v7-v4-v3-v2
  -1,-1,-1, 0,-1, 0, 0, 0,
   1,-1,-1, 0,-1, 0, 0, 1,
   1,-1, 1, 0,-1, 0, 1, 1,
  -1,-1, 1, 0,-1, 0, 1, 0,
  // Face v4-v7-v6-v5
   1,-1,-1, 0, 0,-1, 0, 0,
  -1,-1,-1, 0, 0,-1, 0, 1,
  -1, 1,-1, 0, 0,-1, 1, 1,
   1, 1,-1, 0, 0,-1, 1, 0
};

static const GLushort _cube_elements[] = {
  0, 1, 2, 3,
  4, 5, 6, 7,
  8, 9, 10, 11,
  12, 13, 14, 15,
  16, 17, 18, 19,
  20, 21, 22, 23
};

void Engine::Init() {
  inplay = true;

  // INITIALIZE OPENGL!!!

  // Init GLEW
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return;
  }

  // clear color
  glClearColor(0,1,0,1);
  gl_check_errors("glClearColor");

  ClearGameData(&player1);
  ClearGameData(&player2);

  player1.side = -1;
  player2.side = 1;

  player1.pos = 5;
  player2.pos = 5;

  player1.rot = -BOARD_NORMAL_ROT;
  player2.rot = -BOARD_NORMAL_ROT;

  srand(SDL_GetTicks());

  AddTexture("images/block01.png");
  AddTexture("images/block02.png");
  AddTexture("images/block03.png");
  AddTexture("images/block04.png");
  AddTexture("images/block05.png");
  AddTexture("images/block06.png");
  AddTexture("images/block07.png");

  AddTexture("images/stars-layer.png");
  AddTexture("images/nebula-layer.png");

  AddTexture("images/ball.png");

  AddTexture("images/space-penguinsmall.png");

  AddTexture("images/numbers.png");
  AddTexture("images/letters.png");

  AddTexture("images/penguinsmall.png");
  AddTexture("images/speech-right.png");

  AddTexture("images/letters_w.png");

  audio.Init();

  audio.LoadSound("sounds/addline.wav");
  audio.LoadSound("sounds/tink.wav");
  audio.LoadSound("sounds/penguin-short.wav");
  audio.LoadSound("sounds/bounce.wav");
  audio.LoadSound("sounds/changeview.wav");

  audio.LoadMusic("music/bsh.ogg");
  audio.PlayMusic();

  player1.state = STATE_TETRIS;
  InitState(&player1);

  player1.pos = 5;
  player1.fine = 0;

  tetris.GetNewPiece(&player1);

  space_penguinx = -15 - (rand() % 10);

  space_penguindx = 0.5f * (float)(5 - (rand() % 11));

  if (space_penguindx < 0) {
    space_penguinx = -space_penguinx;
  }
  space_penguiny = -15 - (rand() % 15);

  space_penguindy = 0.5f * (float)(5 - (rand() % 11));

  if (space_penguindy < 0) {
    space_penguiny = -space_penguiny;
  }

  /* Generate VAOS */
#ifndef EMSCRIPTEN // Emscripten/GLES2 does not have VAO support
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);
  gl_check_errors("glBindVertexArray");
#endif

  /* Generate VBOS */
  glGenBuffers(1, &_vbo_vertex);
  glGenBuffers(1, &_vbo_elements_cube);

  glBindBuffer(GL_ARRAY_BUFFER, _vbo_vertex);
  glBufferData(GL_ARRAY_BUFFER, sizeof(_cube_data), _cube_data, GL_STATIC_DRAW);
  gl_check_errors("glBufferData cube_data");

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_elements_cube);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_cube_elements), _cube_elements, GL_STATIC_DRAW);
  gl_check_errors("glBufferData cube_elements");

  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, textures[0]);
  gl_check_errors("glBindTexture");

  /* Generate program */
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  GLuint frag_shader   = glCreateShader(GL_FRAGMENT_SHADER);

  glShaderSource(vertex_shader, 1, vertex_shader_code, NULL);
  glCompileShader(vertex_shader);

  GLint result = GL_FALSE;
  int infoLogLength;

  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &result);
  if (result != GL_TRUE) {
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> vertex_error_msg(infoLogLength);
    glGetShaderInfoLog(vertex_shader, infoLogLength, NULL, &vertex_error_msg[0]);
    fprintf(stdout, "%s\n", &vertex_error_msg[0]);
  }

  glShaderSource(frag_shader, 1, frag_shader_code, NULL);
  glCompileShader(frag_shader);

  glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &result);
  if (result != GL_TRUE) {
    glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> frag_error_msg(infoLogLength);
    glGetShaderInfoLog(frag_shader, infoLogLength, NULL, &frag_error_msg[0]);
    fprintf(stdout, "%s\n", &frag_error_msg[0]);
  }

  _program = glCreateProgram();
  glAttachShader(_program, vertex_shader);
  glAttachShader(_program, frag_shader);
  glBindFragDataLocation(_program, 0, "outColor");
  glLinkProgram(_program);
  gl_check_errors("glLinkProgram");

  glGetProgramiv(_program, GL_LINK_STATUS, &result);
  if (result != GL_TRUE) {
    glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> program_error_msg(infoLogLength);
    glGetProgramInfoLog(_program, infoLogLength, NULL, &program_error_msg[0]);
    fprintf(stdout, "%s\n", &program_error_msg[0]);
  }

  glUseProgram(_program);
  gl_check_errors("glUseProgram");

  glDeleteShader(vertex_shader);
  glDeleteShader(frag_shader);
  gl_check_errors("glDeleteShader");

  /* Attach/describe uniforms */
  _model_uniform = glGetUniformLocation(_program, "model");
  GLuint view_uniform = glGetUniformLocation(_program, "view");
  GLuint proj_uniform = glGetUniformLocation(_program, "proj");
  GLuint tex_uniform = glGetUniformLocation(_program, "tex");
  gl_check_errors("glGetUniformLocation");

  GLint posAttrib = glGetAttribLocation(_program, "position");
  gl_check_errors("glGetAttribLocation position");

  glEnableVertexAttribArray(posAttrib);
  gl_check_errors("glEnableVertexAttribPointer position");

  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, false,
                        (GLsizei)(8 * sizeof(float)),
                        (const GLvoid*)(size_t)(0 * sizeof(float)));
  gl_check_errors("glVertexAttribPointer position");

  posAttrib = glGetAttribLocation(_program, "normal");
  gl_check_errors("glGetAttribLocation normal");

  if (posAttrib >= 0) {
    glEnableVertexAttribArray(posAttrib);
    gl_check_errors("glEnableVertexAttribPointer normal");

    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, false,
                          (GLsizei)(8 * sizeof(float)),
                          (const GLvoid*)(size_t)(3 * sizeof(float)));
    gl_check_errors("glVertexAttribPointer normal");
  }

  posAttrib = glGetAttribLocation(_program, "texcoord");
  gl_check_errors("glGetAttribLocation texcoord");

  if (posAttrib >= 0) {
    glEnableVertexAttribArray(posAttrib);
    gl_check_errors("glEnableVertexAttribPointer texcoord");

    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, false,
                          (GLsizei)(8 * sizeof(float)),
                          (const GLvoid*)(size_t)(6 * sizeof(float)));
    gl_check_errors("glVertexAttribPointer texcoord");
  }

  /* set up perspective */
  glm::mat4 perspective = glm::perspective(40.0f, 4.0f/3.0f, 1.0f, 200.0f);
  glUniformMatrix4fv(proj_uniform, 1, GL_FALSE, &perspective[0][0]);
  gl_check_errors("glUniformMatrix4fv perspective");

  /* set up view */
  glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 21.5f),
                               glm::vec3(0.0f, 0.0f, 0.0f),
                               glm::vec3(0.0f, 1.0f, 0.0));
  glUniformMatrix4fv(view_uniform, 1, GL_FALSE, &view[0][0]);
  gl_check_errors("glUniformMatrix4fv view");

  glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
  glUniformMatrix4fv(_model_uniform, 1, GL_FALSE, &model[0][0]);
  gl_check_errors("glUniformMatrix4fv model");

  if (tex_uniform >= 0) {
    glUniform1i(tex_uniform, 0);
    gl_check_errors("glUniform1i tex");
  }
}

void Engine::SendAttack(int severity) {
  if (severity == 3 && player1.state == STATE_TETRIS) {
    DisplayMessage(STR_TETRIS);
  }
  else if (severity == 3) {
    DisplayMessage(STR_SUPER);
  }

  //printf("ATTACK! %d\n", severity);

  PassMessage(MSG_ATTACK, (unsigned char)severity, 0, 0);
}

void Engine::PerformAttack(int severity) {
  printf("PERFORM ATTACK! %d\n", severity);

  DisplayMessage(STR_ATTACK);

  games[player1.curgame]->Attack(&player1, severity);

}

void Engine::Quit() {
  SDL_Quit();
  quit = 1;
}

void Engine::GameLoop() {
  SDL_Event event;

  Uint32 lasttime = SDL_GetTicks();
  Uint32 curtime;

  float deltatime;

  // Game Loop
  while (!quit) {
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN:
          KeyDown(event.key.keysym.sym);
          break;
        case SDL_KEYUP:
          KeyUp(event.key.keysym.sym);
          break;
        case SDL_MOUSEMOTION:
          MouseMovement(event.motion.x, event.motion.y);
          break;
        case SDL_MOUSEBUTTONDOWN:
          MouseDown();
          break;
        case SDL_MOUSEBUTTONUP:
          break;
        case SDL_QUIT:
          Quit();
          return;
      }
    }

    if (quit) { return; }

    // Draw Frame, tell engine stuff

    curtime = SDL_GetTicks();

    deltatime = (float)(curtime - lasttime) / 1000.0f;

    // CALL ENGINE
    Update(deltatime);
    Draw();

    lasttime = curtime;
  }

}

void Engine::Update(float deltatime) {
  if (!inplay) { return; }

  player1.message_uptime -= deltatime;
  if (player1.message_uptime < 0) {
    player1.message_uptime = 0;
  }

  if (titleChangeTime < 30.0) {
    titleChangeTime += deltatime;
  }
  else {
    titleChangeTime = 0;

    static int cur_caption = 0;

    SDL_WM_SetCaption(stuffs[cur_caption], stuffs[cur_caption]);

    cur_caption++;
    cur_caption %= num_captions;
  }

  if (repeatTime < 0.35) {
    repeatTime += deltatime;
  }
  else {
    time += deltatime;

    if (time >= 0.05) {
      time = 0;

      games[player1.curgame]->KeyRepeat(&player1);
    }
  }

  space_penguinx += space_penguindx * deltatime;
  space_penguiny += space_penguindy * deltatime;

  space_penguinrot += 30.0f * deltatime;

  space_penguinrot = fmod(space_penguinrot, 360.0f);

  if (space_penguinx > 30 || space_penguinx < -30) {
    space_penguinx = -15 - (rand() % 10);

    space_penguindx = 0.5f * (float)(5 - (rand() % 11));

    if (space_penguindx < 0) {
      space_penguinx = -space_penguinx;
    }
  }
  if (space_penguiny > 25 || space_penguiny < -25) {
    space_penguiny = -15 - (rand() % 15);

    space_penguindy = 0.5f * (float)(5 - (rand() % 11));

    if (space_penguindy < 0) {
      space_penguiny = -space_penguiny;
    }
  }

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

  // draw current game
  games[player1.curgame]->Update(&player1, deltatime);
}

int Engine::IntLength(int i) {
  char buffer[255];

  sprintf(buffer, "%d", i);

  return (int)strlen(buffer);
}

int Engine::DrawInt(int i, int color, float x, float y) {
  char buffer[255];

  sprintf(buffer, "%d", i);

  return DrawString((const char*)buffer, color, x, y);
}

int Engine::DrawStringWhite(const char* str, int color, float x, float y) {
  EnableTextures();
  glDisable(GL_BLEND);

  return _DrawString(str,color,x,y, TEXTURE_LETTERS_WHITE);
}

int Engine::DrawString(const char* str, int color, float x, float y) {
  EnableTextures();
  glEnable(GL_BLEND);
  glBlendFunc(GL_DST_ALPHA,GL_ZERO);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE);

  return _DrawString(str,color,x,y, TEXTURE_LETTERS);
}

int Engine::_DrawString(const char* str, int color, float x, float y, int tx) {
  int a = 0;

  int x2;
  int y2;

  y2 = color * 16;

  while(*str) {
    if (str[0] == ':') {
      x2 = 26;
    }
    else if (str[0] == '-') {
      x2 = 27;
    }
    else if (str[0] == '.') {
      x2 = 28;
    }
    else if (str[0] >= 'A' && str[0] <= 'Z') {
      x2 = str[0] - 'A';
    }
    else {
      if (str[0] >= '1' && str[0] <= '9') {
        x2 = str[0] - '1';
      }
      else if (str[0] == '0') {
        x2 = 9;
      }
      else {
        x2 = -1;
      }

      if (x2 != -1) {
        UseTexture(TEXTURE_NUMBERS, (x2*16),y2,16,16);

        glBegin(GL_QUADS);

        glTexCoord2f(tu[0], tv[0]);
        glVertex3f(x, y,0);

        glTexCoord2f(tu[1], tv[0]);
        glVertex3f(x+16,y,0);

        glTexCoord2f(tu[1], tv[1]);
        glVertex3f(x+16,y+16,0);

        glTexCoord2f(tu[0], tv[1]);
        glVertex3f(x,y+16,0);

        glEnd();
      }

      x2 = -1;
    }

    if (x2!=-1) {
      UseTexture(tx, (x2*16),y2,16,16);
      glBegin(GL_QUADS);

      glTexCoord2f(tu[0], tv[0]);
      glVertex2f(x, y);

      glTexCoord2f(tu[1], tv[0]);
      glVertex2f(x+16,y);

      glTexCoord2f(tu[1], tv[1]);
      glVertex2f(x+16,y+16);

      glTexCoord2f(tu[0], tv[1]);
      glVertex2f(x,y+16);

      glEnd();
    }

    x+=16;

    str++;
    a++;
  }

  glDisable(GL_BLEND);
  DisableTextures();

  return a;
}

void Engine::DisplayMessage(int stringIndex) {
  player1.message = strings[stringIndex];

  player1.message_uptime = 3;

  audio.PlaySound(SND_PENGUIN);
}

void Engine::Draw() {
  // clear buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // BACKGROUND!!!
  UseTexture(TEXTURE_BG1, 0,0,texture_widths[TEXTURE_BG2], texture_heights[TEXTURE_BG2]);

  DrawQuadXY(bg1x, bg1y, -6.3f, 15, 15);
  DrawQuadXY(bg1x - 30, bg1y, -6.3f, 15, 15);
  DrawQuadXY(bg1x, bg1y-30, -6.3f, 15, 15);
  DrawQuadXY(bg1x - 30, bg1y-30, -6.3f, 15, 15);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  UseTexture(TEXTURE_BG2, 0,0,texture_widths[TEXTURE_BG2], texture_heights[TEXTURE_BG2]);

  DrawQuadXY(bg2x, bg2y, -6.0f, 20, 20);
  DrawQuadXY(bg2x - 30, bg2y, -6.0f, 20, 20);
  DrawQuadXY(bg2x, bg2y-30, -6.0f, 20, 20);
  DrawQuadXY(bg2x - 30, bg2y-30, -6.0f, 20, 20);

  // Draw Space Pirate Penguin!

  //UseTexture(TEXTURE_SPACEPENGUIN, 0, 0, 150, 155);

  /*
  glTranslatef(space_penguinx, space_penguiny, -5.9f);
  glRotatef(space_penguinrot, 0,0,1);

  glBegin(GL_QUADS);

  DrawQuadXY(-1.0f, 1.0333, 0, 2, 2.03333f);

  glEnd();
  */

  DisableTextures();

  glDisable(GL_BLEND);     // Turn blending Off

  // draw current game
  games[player1.curgame]->Draw(&player1);
  games[player2.curgame]->Draw(&player2);

  // enable depth testing
  glDisable(GL_DEPTH_TEST);
  DisableTextures();

  SDL_GL_SwapBuffers();
  return;

  // Orthogonal HUD!!!
  /*

  glm::ortho(0.0f, 800.0f, 600.0f, 0.0f);

  // go to model view matrix
  glMatrixMode(GL_MODELVIEW) ;
  // init to identity
  glLoadIdentity();

  DrawString("SCORE:", 0, 32.0f,25.0);
  DrawInt(player1.score, 1, 32.0f + (16 * 6),25.0);

  games[player1.curgame]->DrawOrtho(&player1);
  games[player2.curgame]->DrawOrtho(&player2);

  if (player1.message_uptime > 0) {
    // put up display
    EnableTextures();
    glEnable(GL_BLEND);

    // speech bubble
    UseTexture(TEXTURE_SPEECH, 0,0,330,110);

    glBegin(GL_QUADS);

    glTexCoord2f(tu[0],tv[1]);
    glVertex3f(400+60,50,0);
    glTexCoord2f(tu[1],tv[1]);
    glVertex3f(400+335,50,0);
    glTexCoord2f(tu[1],tv[0]);
    glVertex3f(400+335,50+110,0);
    glTexCoord2f(tu[0],tv[0]);
    glVertex3f(400+60,50.0+110,0);

    glEnd();

    // center message within bubble
    int x2 = ((int)strlen(player1.message) * 16);
    x2 = 245 - x2;
    x2 /= 2;
    x2 += 480;

    // draw message
    DrawStringWhite(player1.message, 4, x2,99);
  }

  glEnable(GL_BLEND);
  EnableTextures();
  UseTexture(TEXTURE_PENGUIN, 0,0,180,175);

  glBegin(GL_QUADS);

  glTexCoord2f(tu[0],tv[0]);
  glVertex3f(400-55,20,0);
  glTexCoord2f(tu[1],tv[0]);
  glVertex3f(400+65,20,0);
  glTexCoord2f(tu[1],tv[1]);
  glVertex3f(400+65,20.0+110.0,0);
  glTexCoord2f(tu[0],tv[1]);
  glVertex3f(400-55,20.0+110.0,0);

  glEnd();

  DisableTextures();

  glDisable(GL_BLEND);

  */
}

void Engine::KeyDown(Uint32 key) {
  bool gameover = !inplay;
#ifndef NO_NETWORK
  gameover = gameover && !client_tcpsock;
#endif

  if (gameover) {
    ClearGameData(&player1);
    return;
  }

  if (key == SDLK_4) {
    PerformAttack(2);
  }

  if (key == SDLK_ESCAPE) {
    Quit();
    return;
  }

  repeatTime = 0;

  if (key < 0xffff) {
    keys[key] = 1;
  }

  if (!inplay) { return; }

  games[player1.curgame]->KeyDown(&player1, key);
  games[player1.curgame]->KeyRepeat(&player1);
}

void Engine::KeyUp(Uint32 key) {
  if (key < 0xffff) {
    keys[key] = 0;
  }

  if (!inplay) { return; }

  games[player1.curgame]->KeyUp(&player1, key);
}

void Engine::MouseDown() {
  bool gameover = !inplay;
#ifndef NO_NETWORK
  gameover = gameover && !client_tcpsock;
#endif

  if (gameover) {
    ClearGameData(&player1);
  }
}

void Engine::MouseMovement(Uint32 x, Uint32 y) {
  games[player1.curgame]->MouseMovement(&player1, x,y);
}

void Engine::GameOver() {
  printf("Game Over!\n");

  inplay = false;

  DisplayMessage(STR_YOULOSE);

  PassMessage(MSG_GAMEOVER, 0,0,0);
}

void Engine::DrawQuadXY(float x, float y, float z, float w, float h) {
  glm::mat4 model = glm::scale(
                      glm::translate(
                        glm::mat4(1.0f),
                        glm::vec3(x, y, z)),
                      glm::vec3(w, h, 1.0f));

  glUniformMatrix4fv(_model_uniform, 1, GL_FALSE, &model[0][0]);
  gl_check_errors("glUniformMatrix4fv model");

  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_SHORT, 0);
  gl_check_errors("glDrawElements");
}

void Engine::DrawQuad(int a, int b, int c, int d) {
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_SHORT, 0);
  gl_check_errors("glDrawElements");
}

void Engine::DrawCube() {
  glDrawElements(GL_QUADS, 24, GL_UNSIGNED_SHORT, 0);
  gl_check_errors("glDrawElements");
}

void Engine::ClearGameData(game_info* player) {
  int i, j;

  for (i=0; i<10; i++) {
    for(j=0;j<24;j++) {
      player->board[i][j] = -1;
    }
  }

  ChangeState(player, STATE_TETRIS);

  player->score = 0;
  player->state_lines = 0;
  player->break_out_consecutives = 0;

  player1.rot = -BOARD_NORMAL_ROT;
  player1.rot2 = 0;

  player1.pos = 5;
  player1.fine = 0;

  tetris.GetNewPiece(&player1);

  inplay = true;
}

void Engine::ChangeState(game_info* gi, int newState) {
  UninitState(gi);

  // tell networked opponent (IF PLAYER 1)
  if (gi->side == -1) {
    PassMessage(MSG_CHANGE_STATE, newState,0,0);
  }

  gi->state = newState;

  // init state (IF PLAYER 1)
  if (gi->side == -1) {
    InitState(gi);
  }
}

void Engine::InitState(game_info* gi) {
  switch (gi->state) {
    case STATE_BREAKOUT:
      gi->curgame = 1;
      breakout.InitGame(&player1);
      break;
    case STATE_TETRIS:
      gi->curgame = 0;
      tetris.InitGame(&player1);
      break;
  }
}

void Engine::UninitState(game_info* gi) {
}

void Engine::EnableTextures() {
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void Engine::DisableTextures() {
  glDisable(GL_TEXTURE_2D);
}

void Engine::UseTextureUpsideDown(int textureIndex, int startX, int startY, int width, int height) {
  if (textureIndex < 0 || textureIndex >= texture_count) { return; }

  glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);

  tu[1] = (float)startX / (float)(texture_widths[textureIndex]-1);
  tv[1] = (float)startY / (float)(texture_heights[textureIndex]-1);

  startX += width;
  startY += height;

  tu[0] = (float)startX / (float)(texture_widths[textureIndex]-1);
  tv[0] = (float)startY / (float)(texture_heights[textureIndex]-1);
}

void Engine::UseTexture(int textureIndex, int startX, int startY, int width, int height) {
  if (textureIndex < 0 || textureIndex >= texture_count) { return; }

  glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);
  gl_check_errors("glBindTexture");
}

// from tutorial on interwebz:
int Engine::AddTexture(const char* fname) {
  GLuint texture;  // This is a handle to our texture object
  SDL_Surface *surface; // This surface will tell us the details of the image
  GLenum texture_format;
  GLint  nOfColors;

  if ( (surface = IMG_Load(fname)) ) {
    // Check that the image's width is a power of 2
    if ( (surface->w & (surface->w - 1)) != 0 ) {
      printf("warning: image.bmp's width is not a power of 2\n");
    }

    // Also check if the height is a power of 2
    if ( (surface->h & (surface->h - 1)) != 0 ) {
      printf("warning: image.bmp's height is not a power of 2\n");
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

    // Bind the texture object
    glBindTexture( GL_TEXTURE_2D, texture );

    // Set the texture's stretching properties
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // Edit the texture object's image data using the information SDL_Surface gives us
    glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
        texture_format, GL_UNSIGNED_BYTE, surface->pixels );
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

void Engine::RunServer(int port) {
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

void Engine::RunClient(char* ipname, int port) {
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

void Engine::ProcessMessage(unsigned char msg[4]) {
  unsigned char msgID = msg[0];

  //printf("Msg Recv: %d, %d, %d, %d\n", msgID, msg[1], msg[2], msg[3]);

  switch (msgID) {
    case MSG_ADDPIECE: // add piece to tetris board
      //printf("Msg: ADDPIECE\n");

      tetris.AddPiece(&player2, msg[1], msg[2]);
      break;
    case MSG_DROPLINE:
      tetris.DropLine(&player2, msg[1]);
      break;
    case MSG_ATTACK:
      // ATTACK!!!
      PerformAttack(msg[1]);
      break;
    case MSG_PUSHUP:
      tetris.PushUp(&player2, msg[1]);
      break;
    case MSG_ADDBLOCKS_A:
      tetris.AddBlock(&player2, 0, 23, msg[1]);
      tetris.AddBlock(&player2, 1, 23, msg[2]);
      tetris.AddBlock(&player2, 2, 23, msg[3]);
      break;
    case MSG_ADDBLOCKS_B:
      tetris.AddBlock(&player2, 3, 23, msg[1]);
      tetris.AddBlock(&player2, 4, 23, msg[2]);
      tetris.AddBlock(&player2, 5, 23, msg[3]);
      break;
    case MSG_ADDBLOCKS_C:
      tetris.AddBlock(&player2, 6, 23, msg[1]);
      tetris.AddBlock(&player2, 7, 23, msg[2]);
      tetris.AddBlock(&player2, 8, 23, msg[3]);
      break;
    case MSG_ADDBLOCKS_D:
      tetris.AddBlock(&player2, 9, 23, msg[1]);
      break;
    case MSG_ADDBLOCKS2_A:
      tetris.AddBlock(&player2, 0, 22, msg[1]);
      tetris.AddBlock(&player2, 1, 22, msg[2]);
      tetris.AddBlock(&player2, 2, 22, msg[3]);
      break;
    case MSG_ADDBLOCKS2_B:
      tetris.AddBlock(&player2, 3, 22, msg[1]);
      tetris.AddBlock(&player2, 4, 22, msg[2]);
      tetris.AddBlock(&player2, 5, 22, msg[3]);
      break;
    case MSG_ADDBLOCKS2_C:
      tetris.AddBlock(&player2, 6, 22, msg[1]);
      tetris.AddBlock(&player2, 7, 22, msg[2]);
      tetris.AddBlock(&player2, 8, 22, msg[3]);
      break;
    case MSG_ADDBLOCKS2_D:
      tetris.AddBlock(&player2, 9, 22, msg[1]);
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
      ChangeState(&player2, msg[1]);
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
      DisplayMessage(STR_YOUWIN);
      break;

    case MSG_APPENDSCORE:
      player2.score += ((int)msg[1] * (int)msg[2]);
      break;
  }
}

void Engine::PassMessage(unsigned char msgID, unsigned char p1, unsigned char p2, unsigned char p3) {
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

GLfloat Engine::cubecoords[8][4] = {
  {-0.5,0.5,0.5,1},
  {0.5, 0.5, 0.5, 1},
  {0.5, 0.5, -0.5, 1},
  {-0.5, 0.5, -0.5, 1},

  {-0.5,-0.5,0.5,1},
  {0.5, -0.5, 0.5, 1},
  {0.5, -0.5, -0.5, 1},
  {-0.5, -0.5, -0.5, 1}
};

GLfloat Engine::cubenorms[8][4] = {
  {-1,1,1,1},
  {1, 1, 1, 1},
  {1, 1, -1, 1},
  {-1, 1, -1, 1},

  {-1,-1,1,1},
  {1, -1, 1, 1},
  {1, -1, -1, 1},
  {-1, -1, -1, 1}
};


game_info Engine::player2 = {0};
game_info Engine::player1 = {0};

int Engine::quit = 0;


GLuint* Engine::textures = NULL;
int* Engine::texture_widths = NULL;
int* Engine::texture_heights = NULL;
int Engine::texture_count = 0;
int Engine::texture_capacity = 10;

GLfloat Engine::tu[2] = {0.0f, 1.0f};
GLfloat Engine::tv[2] = {0.0f, 1.0f};

double Engine::time = 0;
double Engine::repeatTime = 0;
double Engine::titleChangeTime = 30.0;

int Engine::keys[0xffff] = {0};

float Engine::bg1x = 0;
float Engine::bg1y = 0;

float Engine::bg2x = 0;
float Engine::bg2y = 0;

#ifndef NO_NETWORK
IPaddress Engine::ip = {0};
TCPsocket Engine::tcpsock = NULL;
TCPsocket Engine::client_tcpsock = NULL;
#endif

SDL_Thread *Engine::network_thread = NULL;

int Engine::inplay = 1;

float Engine::space_penguinx = -20;
float Engine::space_penguiny = -20;
float Engine::space_penguindx = 10.0f;
float Engine::space_penguindy = 8.0f;
float Engine::space_penguinrot = 8.0f;
