#include "audio.h"
#include "../utils.h"

void audio_init(void) {
    if (SDL_Init(SDL_INIT_AUDIO)) {
        ERROR_EXIT_PROGRAM("Cannot initialize audio. SDL error: %s\n", SDL_GetError());
    }
    uint16 audio_format = MIX_DEFAULT_FORMAT;
    int32 audio_rate = 44100;
    int32 channels = 2, chunksize = 2048;

    if (Mix_OpenAudio(audio_rate, audio_format, channels, chunksize)) {
        ERROR_EXIT_PROGRAM("SDL_mixer error: %s", Mix_GetError());
    }

    Mix_Volume(-1, 6);
    Mix_VolumeMusic(2);
}

void audio_sound_load(Mix_Chunk **chunk, const char *path) {
    *chunk = Mix_LoadWAV(path);
    if (!*chunk) {
        ERROR_RETURN(, "Cannot open chunk: %s error: %s", path, Mix_GetError());
    }
}

void audio_music_load(Mix_Music **music, const char *path) {
    *music = Mix_LoadMUS(path);
    if (!*music) {
        ERROR_RETURN(, "Cannot open music: %s error: %s", path, Mix_GetError());
    }
}

void audio_play_sound(Mix_Chunk *chunk) {
    Mix_PlayChannel(-1, chunk, 0);
}

void audio_play_music(Mix_Music *music, int32 loop) {
    Mix_PlayMusic(music, loop);
}

