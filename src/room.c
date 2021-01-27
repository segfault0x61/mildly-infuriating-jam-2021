#include "room.h"
#include "sprite.h"
#include "game.h"

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

typedef struct {
	unsigned int color;
	int collision_type;
	int collision_response;
} Tile;

Tile tile_desc[] = {
	[TILE_AIR]   = {
		.color          = 0xff,
		.collision_type = COLLISION_NONE
	},

	[TILE_WALL]  = {
		.color              = 0xffffffff,
		.collision_type     = COLLISION_BOX,
		.collision_response = RESP_BLOCK,
	},

	[TILE_SPIKE] = {
		.color              = 0xff0000ff,
		.collision_type     = COLLISION_BOX,
		.collision_response = RESP_KILL,
	}
};

static int current_room;
static int sprite_offset;

typedef struct {
    int neighbours[4];
} RoomMap;

static RoomMap room_map[256];

void room_init(void) {
    FILE* f = fopen("res/maps/map.txt", "r");

    char line[256];
    fgets(line, sizeof(line), f);

    int id;
    int n[4];
    while (fscanf(f, "%d: %d %d %d %d\n", &id, n, n + 1, n + 2, n + 3) == 5) {
        puts("loaded stuff");
        SDL_assert(id < ARRAY_COUNT(room_map));
        memcpy(room_map[id].neighbours, n, sizeof(n));
    }
}

void room_load(int n) {
    printf("Loading room %d\n", n);

    // A room already loaded
    if (sprite_offset) {
        sprite_pop(sprites + sprite_offset, ROOM_WIDTH * ROOM_HEIGHT);
    }

    char* fileName;
    asprintf(&fileName, "res/maps/room%d.txt", n);
    FILE* f = fopen(fileName, "r");

    SDL_assert(f);

    current_room = n;

    free(fileName);

    char line[256];
    int x  = 0, y = 0;
    int i = 0;

    int tile_type = 0;
    sprite_offset = num_sprites;
    
    while (fgets(line, sizeof(line), f)) {
        for (const char* c = line; *c; ++c) {
            if (*c == '\n') continue;

            switch (*c) {
                case '#': {
                    tile_type = TILE_WALL;
                } break;
                case 'x': {
                    tile_type = TILE_SPIKE;
                } break;
                case '.': 
                default: {
                    tile_type = TILE_AIR;
                } break;
            }

            Tile* t = tile_desc + tile_type;
            Sprite* s = sprite_push_col(x, y, 32, 32, t->color);

            s->collision_type = t->collision_type;
            s->collision_response = t->collision_response;

            ++i;
            x += 32;
        }

        x = 0;
        y += 32;
    }

    fclose(f);
}

void room_switch(int dest) {
    printf("switch %d\n", dest);

    int id = room_map[current_room].neighbours[dest];

    room_load(id);
}