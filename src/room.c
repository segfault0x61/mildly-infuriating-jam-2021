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
    TILE_BLOOD,
};

enum {
	TILE_TEX_MODE_NONE,
	TILE_TEX_ADJACENCY,
	TILE_TEX_ANIMATE,
	TILE_TEX_STATIC = 0x1000,
};

typedef struct {
	unsigned int color;
    const char* texture;
	int collision_type;
	int collision_response;
    int tex_mode;
} Tile;

Tile tile_desc[] = {
	[TILE_AIR]   = {
		.color          = 0xff,
		.collision_type = COLLISION_NONE
	},

	[TILE_WALL]  = {
		.texture            = "res/sprites/walls.png",
		.collision_type     = COLLISION_BOX,
		.collision_response = RESP_BLOCK,
        .tex_mode           = TILE_TEX_ADJACENCY,
	},

	[TILE_SPIKE] = {
        .texture            = "res/sprites/spikes.png",
		.collision_type     = COLLISION_BOX,
		.collision_response = RESP_KILL,
        .tex_mode           = TILE_TEX_ADJACENCY,
	},

	[TILE_BLOOD] = {
		.texture            = "res/sprites/blood.png",
		.collision_type     = COLLISION_BOX,
		.collision_response = RESP_POWERUP,
		.tex_mode           = TILE_TEX_ANIMATE,
	}
};

static int current_room;
static int tile_types[ROOM_WIDTH * ROOM_HEIGHT];
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
                case 'b': {
					tile_type = TILE_BLOOD;
				} break;
                case '.': 
                default: {
                    tile_type = TILE_AIR;
                } break;
            }

			tile_types[i] = tile_type;
			Tile*   t = tile_desc + tile_type;
			Sprite* s;
			
			if (t->texture) {
				s = sprite_push_tex(x, y, 32, 32, t->texture);
				if (t->tex_mode & TILE_TEX_STATIC) {
					s->cur_frame = t->tex_mode - TILE_TEX_STATIC;
				}
			} else {
				s = sprite_push_col(x, y, 32, 32, t->color);
			}

            s->collision_type = t->collision_type;
            s->collision_response = t->collision_response;

            ++i;
            x += 32;
        }

        x = 0;
        y += 32;
    }

    for (int j = 0; j < (ROOM_WIDTH * ROOM_HEIGHT); ++j) {

        Tile* t = tile_desc + tile_types[j];

        if (t->tex_mode == TILE_TEX_ADJACENCY) {
            int tex_choice = 0;

            if ((j % ROOM_WIDTH) == 0 || tile_types[j-1] != TILE_AIR) {
                tex_choice |= 1;
            }
            if ((j % ROOM_WIDTH) == (ROOM_WIDTH-1) || tile_types[j+1] != TILE_AIR) {
                tex_choice |= 2;
            }
            if (j <= ROOM_WIDTH || tile_types[j - ROOM_WIDTH] != TILE_AIR) {
                tex_choice |= 4;
            }
            if (j >= ((ROOM_WIDTH - 1) * ROOM_HEIGHT - 1) || tile_types[j+ROOM_WIDTH] != TILE_AIR) {
                tex_choice |= 8;
            }

            struct {
                int frame;
                int rotation;
            } tex_lookup[] = {
                [0] = { 3, 0 },
                [1] = { 2, 90 },
                [2] = { 2, -90 },
                [3] = { 5, 0 },
                [4] = { 2, 189 },
                [5] = { 1, 180 },
                [6] = { 1, -90 },
                [7] = { 0, 180 },
                [8] = { 2, 0 },
                [9] = { 1, 90 },
                [10] = { 1, 0 },
                [11] = { 0, 0 },
                [12] = { 5, 90 },
                [13] = { 0, 90 },
                [14] = { 0, -90 },
                [15] = { 4, 0 }, 
            };

            printf("lookup %d %d = %d %d\n", j, tex_choice, tex_lookup[tex_choice].frame, tex_lookup[tex_choice].rotation);

            sprites[sprite_offset + j].cur_frame = tex_lookup[tex_choice].frame;
            sprites[sprite_offset + j].rotation = tex_lookup[tex_choice].rotation;
        }
    }

    fclose(f);
}

static int anim_timer; 

void room_update(int delta){

	anim_timer += delta;

	if(anim_timer > 200){
		for(int i = 0; i < (ROOM_WIDTH * ROOM_HEIGHT); ++i){
			Tile*   t = tile_desc + tile_types[i];
			Sprite* s = sprites + sprite_offset + i;

			if(t->tex_mode == TILE_TEX_ANIMATE){
				s->cur_frame = (s->cur_frame + 1) % s->num_frames;
			}
		}
		anim_timer = 0;
	}
}

void room_switch(int dest) {
    printf("switch %d\n", dest);

    int id = room_map[current_room].neighbours[dest];

    room_load(id);
}