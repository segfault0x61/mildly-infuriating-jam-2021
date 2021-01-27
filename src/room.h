#ifndef ROOM_H_
#define ROOM_H_

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

#endif