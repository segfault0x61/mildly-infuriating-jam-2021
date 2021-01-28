#ifndef ROOM_H_
#define ROOM_H_

#include <SDL2/SDL.h>

enum {
	ROOM_UP,
	ROOM_DOWN,
	ROOM_LEFT,
	ROOM_RIGHT,
};

void room_init(void);
void room_load(int n);
void room_update(int delta);
void room_switch(int dest);
void room_reset(void);
void room_get_powerup(int index);
SDL_Point room_get_spawn(void);

#endif