#ifndef ROOM_H_
#define ROOM_H_

enum {
    ROOM_RIGHT,
    ROOM_LEFT,
    ROOM_DOWN,
    ROOM_UP,
};

void room_load(int n);
void room_update(int delta);
void room_switch(int dest);

#endif