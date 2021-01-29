#include "sound.h"
#include "game.h"

#include <SDL2/SDL_mixer.h>

#include <stdio.h>
#include <string.h>

typedef struct {
	const char* name;
	int volume;
	Mix_Chunk* sfx;
} Sound;

Sound sounds[] = {
	{ "res/sfx/bat.wav", 80 },
	{ "res/sfx/die.wav", 64 },
	{ "res/sfx/shapeshift.wav", 40 },
	{ "res/sfx/unshapeshift.wav", 40 },
	{ "res/sfx/nope.wav", 20 },
	{ "res/sfx/powerup.wav", 64 },
};

#define NUM_CHANNELS 3
static int channel;
static Mix_Chunk* sfx_chans[128];

void sound_init(void) {
	SDL_Init(SDL_INIT_AUDIO);

	if (Mix_Init(MIX_INIT_MOD | MIX_INIT_MP3) == 0) {
		fprintf(stderr, "Failed to initialize Mixer %s: %d\n", Mix_GetError());
		Mix_Init(0);
	}

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	for (int i = 0; i < array_count(sounds); ++i) {
		sounds[i].sfx = Mix_LoadWAV(sounds[i].name);
		if (!sounds[i].sfx) {
			fprintf(stderr, "Couldn't load %s: %s\n", sounds[i].name, Mix_GetError());
		}
		Mix_VolumeChunk(sounds[i].sfx, sounds[i].volume ? sounds[i].volume : 64);
	}

	Mix_ReserveChannels(NUM_CHANNELS);

	Mix_Music* main_music = Mix_LoadMUS("res/music/twentyone_loop.mp3");
	Mix_PlayMusic(main_music, -1);
	Mix_HookMusicFinished(NULL);
}

void sound_play(const char* name, int loops) {
	Mix_Chunk* chunk = NULL;

	for (int i = 0; i < array_count(sounds); ++i) {
		if (strcmp(name, sounds[i].name) == 0) {
			chunk = sounds[i].sfx;
			break;
		}
	}

	if (!chunk) return;

	// Don't double up sound effects
	for (int i = 0; i < NUM_CHANNELS; ++i) {
		if (sfx_chans[i] == chunk) {
			Mix_HaltChannel(i);
		}
	}

	int i = Mix_PlayChannel(channel, chunk, loops);
	if (i == -1) {
		fprintf(stderr, "SFX error: %s\n", Mix_GetError());
	} else {
		sfx_chans[i] = chunk;
	}

	channel = (channel + 1) % NUM_CHANNELS;	
}