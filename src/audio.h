#ifndef AUDIO_INCLUDED
#define AUDIO_INCLUDED

#define NUM_SOUNDS 10

class Audio {
public:
  Audio();
  ~Audio();

  void loadMusic(const char* fname);
  void playMusic();

  void init();

  void playSound(int soundIndex);

  int loadSound(const char *file);

  // sounds
  static Mix_Chunk* sounds[NUM_SOUNDS];
  static int soundcount;

  // music file
  Mix_Music *music;
};
#endif //#ifndef AUDIO_INCLUDED
