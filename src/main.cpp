#include "engine.h"
#include "main.h"

#include "components.h"

const char* strings[] = {
  "ATTACK",
  "TRANSITION",
  "SUPER",
  "TETRIS",
  "YOU LOSE",
  "YOU WIN",
  "YOU SURVIVED",
};

int main(int argc, char** argv) {
  int port;
  int isServer = 0;
  char* ip=NULL;

  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_WM_SetCaption("OMGWTFADD", NULL);

  if (argc > 1) {
    int i;
    for (i=1; i<argc; i++) {
      if (strcmp(argv[i], "-p")==0) {
        i++;
        if (i==argc) {break;}

        port = atoi(argv[i]);
      }
      else if (strcmp(argv[i], "-s") == 0) {
        printf("hosting...\n");
        isServer = 1;
      }
      else {
        ip = argv[i];
      }
    }

    if (isServer && (ip != NULL)) {
      // NO!
      printf("Invalid Parameters, ignoring ip\n");
    }

#ifndef NO_NETWORK
    if(SDLNet_Init()==-1) {
      printf("SDLNet_Init: %s\n", SDLNet_GetError());
      return -1;
    }

    if (isServer==0) {
      printf("connecting to port %d\n", port);
      printf("connecting to %s\n", ip);

      // Try to make a connection
      engine.runClient(ip, port);
    }
    else {
      engine.runServer(port);
    }
#endif
  }

  // Create a double-buffered draw context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_SetVideoMode(WIDTH, HEIGHT, 0, SDL_OPENGL);// | SDL_FULLSCREEN);

  engine.init();

  engine.gameLoop();

#ifndef EMSCRIPTEN
  SDL_Quit();
#endif

  return 0;
}
