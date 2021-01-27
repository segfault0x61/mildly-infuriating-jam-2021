#include "sound.h"
#include "game.h"

#include <SDL2/SDL_mixer.h>

#include <stdio.h>
#include <string.h>

typedef struct {
	const char* name;
	Mix_Chunk* sfx;
} Sound;

Sound sounds[] = {
	{ "res/sfx/bat.ogg" },
	{ "res/sfx/die.ogg" },
	{ "res/sfx/shapeshift.ogg" },
	{ "res/sfx/unshapeshift.ogg" },
};

void sound_init(void) {
    int flags = Mix_Init(MIX_INIT_OGG);
	if (!(flags & MIX_INIT_OGG)) {
		fprintf(stderr, "Couldn't load ogg vorbis codec\n");
	}

	if (Mix_OpenAudio(44100, AUDIO_S16, 2, 1024) == -1) {
		fprintf(stderr, "Couldn't open audio: %s\n", Mix_GetError());
	}

	for (int i = 0; i < ARRAY_COUNT(sounds); ++i) {
		sounds[i].sfx = Mix_LoadWAV(sounds[i].name);
		if (!sounds[i].sfx) {
			fprintf(stderr, "Couldn't load %s: %s\n", sounds[i].name, Mix_GetError());
		}
		Mix_VolumeChunk(sounds[i].sfx, 64);
	}
}

void sound_play(const char* name, int loops) {
	for (int i = 0; i < ARRAY_COUNT(sounds); ++i) {
		if (strcmp(name, sounds[i].name) == 0) {
			if (Mix_PlayChannel(-1, sounds[i].sfx, loops) == -1) {
				fprintf(stderr, "SFX error: %s\n", Mix_GetError());
			}
			break;
		}
	}
}