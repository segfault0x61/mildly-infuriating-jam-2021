#ifndef SPRITE_H_
#define SPRITE_H_

#include <SDL2/SDL.h>

enum {
	COLLISION_NONE,
	COLLISION_BOX,
	COLLISION_CIRCLE,
};

enum {
	RESP_BLOCK,
	RESP_KILL,
	RESP_POWERUP,
};

typedef struct {
	union {
		SDL_Rect rect;
		struct {
			int x, y, w, h;
		};
	};

	SDL_Color color;
	SDL_Texture* tex;

	int num_frames;
	int cur_frame;
	int collision_type;
	int collision_response;

	float hitbox_scale_x;
	float hitbox_scale_y;
	float radius;

	int flip_mode;
	int rotation;

} Sprite;

#define MAX_SPRITES 4096

extern Sprite sprites[MAX_SPRITES];
extern int num_sprites;

void sprite_set_color(Sprite* s, unsigned int color);
void sprite_set_tex(Sprite* s, const char* name, int frames);

Sprite* sprite_push(int x, int y, int w, int h);
Sprite* sprite_push_col(int x, int y, int w, int h, unsigned int color);
Sprite* sprite_push_tex(int x, int y, int w, int h, const char* name);
Sprite* sprite_push_tex_frames(int x, int y, int w, int h, const char* name, int frames);

SDL_Point sprite_get_center(Sprite* s);

void sprite_draw(Sprite* s);
void sprite_pop(Sprite* start, int num);
SDL_Rect sprite_get_hit_box(Sprite* s);

#endif
