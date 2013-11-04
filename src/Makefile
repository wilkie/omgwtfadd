CFLAGS =
CC = g++
CLINK = -lGL -lSDL -lSDL_net -lSDL_mixer -lSDL_image -lGLU

all: audio.cpp breakout.cpp components.cpp engine.cpp game.cpp main.cpp tetris.cpp
	$(CC) audio.cpp -c $(CFLAGS) -I.
	$(CC) breakout.cpp -c $(CFLAGS) -I.
	$(CC) components.cpp -c $(CFLAGS) -I.
	$(CC) engine.cpp -c $(CFLAGS) -I.
	$(CC) game.cpp -c $(CFLAGS) -I.
	$(CC) main.cpp -c $(CFLAGS) -I.
	$(CC) tetris.cpp -c $(CFLAGS) -I.
	$(CC) glew/glew.c -c $(CFLAGS) -I.
	$(CC) -o ../omgwtfadd audio.o glew.o breakout.o components.o engine.o game.o main.o tetris.o $(CLINK)

clean:
	rm *.o
