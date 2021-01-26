#include "room.h"
#include "sprite.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ROOM_WIDTH 26
#define ROOM_HEIGHT 15

enum {
    TILE_AIR,
    TILE_WALL,
    TILE_SPIKE,
};

struct Tile {
    unsigned int color;
} tile_desc[] = {
    [TILE_AIR]   = { .color = 0xFF },
    [TILE_WALL]  = { .color = 0xFFFFFFFF },
    [TILE_SPIKE] = { .color = 0xFF0000FF }
};

typedef struct {
    int id;
    int tiles[ROOM_WIDTH * ROOM_HEIGHT];
} Room;

Room curr_room;

void room_load(int n) {
    char* fileName;
    asprintf(&fileName, "res/maps/room%d.txt", n);
    FILE* f = fopen(fileName, "r");
    free(fileName);

    char line[256];
    int x  = 0, y = 0;
    int i = 0;

    int* tiles = curr_room.tiles;
    while (fgets(line, sizeof(line), f)) {
        for (const char* c = line; *c; ++c) {
            if (*c == '\n') continue;
            switch (*c) {
                case '#': {
                    tiles[i] = TILE_WALL;
                } break;
                case 'x': {
                    tiles[i] = TILE_SPIKE;
                } break;
                case '.': 
                default: {
                    tiles[i] = TILE_AIR;
                } break;
            }

            sprite_push_col(x, y, 32, 32, tile_desc[tiles[i]].color);
            ++i;

            x += 32;
        }

        x = 0;
        y += 32;
    }

    fclose(f);
}