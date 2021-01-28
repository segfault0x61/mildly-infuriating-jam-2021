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
	{ "res/sfx/nope.ogg" },
	{ "res/sfx/powerup.ogg" },
};

#define NUM_CHANNELS 3
static int channel;

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

	Mix_AllocateChannels(NUM_CHANNELS);
}

void sound_play(const char* name, int loops) {
	Mix_Chunk* chunk = NULL;

	for (int i = 0; i < ARRAY_COUNT(sounds); ++i) {
		if (strcmp(name, sounds[i].name) == 0) {
			chunk = sounds[i].sfx;
			break;
		}
	}

	if (!chunk) return;

	// Don't double up sound effects
	for (int i = 0; i < NUM_CHANNELS; ++i) {
		if (Mix_GetChunk(i) == chunk) {
			Mix_HaltChannel(i);
		}
	}

	if (Mix_PlayChannel(channel, chunk, loops) == -1) {
		fprintf(stderr, "SFX error: %s\n", Mix_GetError());
	}

	channel = (channel + 1) % NUM_CHANNELS;	
}