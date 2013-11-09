CFLAGS =
CC = g++
CLINK = -lGL -lSDL -lSDL_mixer -lSDL_image -lGLU
CLINK_NET = -lSDL_net

all: audio.cpp breakout.cpp components.cpp engine.cpp game.cpp main.cpp tetris.cpp
	$(CC) audio.cpp -c $(CFLAGS) -I.
	$(CC) breakout.cpp -c $(CFLAGS) -I.
	$(CC) components.cpp -c $(CFLAGS) -I.
	$(CC) engine.cpp -c $(CFLAGS) -I.
	$(CC) mesh.cpp -c $(CFLAGS) -I.
	$(CC) context.cpp -c $(CFLAGS) -I.
	$(CC) flame.cpp -c $(CFLAGS) -I.
	$(CC) game.cpp -c $(CFLAGS) -I.
	$(CC) main.cpp -c $(CFLAGS) -I.
	$(CC) tetris.cpp -c $(CFLAGS) -I.
	$(CC) glew/glew.c -c $(CFLAGS) -I.
	$(CC) -o ../omgwtfadd audio.o context.o mesh.o flame.o glew.o breakout.o components.o engine.o game.o main.o tetris.o $(CLINK) $(CLINK_NET)

js: audio.cpp breakout.cpp components.cpp engine.cpp game.cpp main.cpp tetris.cpp
	em++ audio.cpp -c $(CFLAGS) -I. -DNO_NETWORK -O2
	em++ breakout.cpp -c $(CFLAGS) -I. -DNO_NETWORK -O2
	em++ components.cpp -c $(CFLAGS) -I. -DNO_NETWORK -O2
	em++ engine.cpp -c $(CFLAGS) -I. -DNO_NETWORK -O2
	em++ mesh.cpp -c $(CFLAGS) -I. -DNO_NETWORK -O2
	em++ context.cpp -c $(CFLAGS) -I. -DNO_NETWORK -O2
	em++ game.cpp -c $(CFLAGS) -I. -DNO_NETWORK -O2
	em++ flame.cpp -c $(CFLAGS) -I. -DNO_NETWORK -O2
	em++ main.cpp -c $(CFLAGS) -I. -DNO_NETWORK -O2
	em++ tetris.cpp -c $(CFLAGS) -I. -DNO_NETWORK -O2
	emcc -o ../omgwtfadd.js audio.o mesh.o flame.o context.o breakout.o components.o engine.o game.o main.o tetris.o -s ALLOW_MEMORY_GROWTH=1 --preload-file ../sounds@/sounds --preload-file ../images@/images --preload-file ../music@/music --preload-file ../assets@/assets $(CLINK)

clean:
	rm *.o
