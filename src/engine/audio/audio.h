#ifndef AUDIO_H
#define AUDIO_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "../types.h"

void audio_init(void);
void audio_sound_load(Mix_Chunk **chunk, const char *path);
void audio_music_load(Mix_Music **music, const char *path);
void audio_play_sound(Mix_Chunk *chunk);
void audio_play_music(Mix_Music *music, int32 loop);
void audio_exit(void);

#endif // !AUDIO_H
