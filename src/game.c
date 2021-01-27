#include "game.h"
#include "sprite.h"
#include "room.h"
#include "particles.h"

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    Sprite *sprite;
    int speed;
    int anim_timer;
	bool is_bat;
	float x_velocity;
	float y_velocity;
	int bat_timer;
} Player;

static Player player = {
    .sprite = sprites,
    .speed = 2,
	.is_bat = false,
};

#define PLAYER_TERMINAL_VEL 12.0f
#define PLAYER_ACCEL 0.2f
#define PLAYER_MAX_IMPULSE 12.0f
#define PLAYER_MAX_IMPULSE_BAT 15.0f
#define BAT_BAR_WIDTH 100
#define BAT_BAR_HEIGHT 20

void game_init(void) {

    // Draw player
    sprite_push_tex(400, 200, 64, 64, "res/sprites/vamp.png");
	sprites[0].radius = 16;
	sprites[0].hitbox_scale_x = 0.5f;
	sprites[0].hitbox_scale_y = 1.0f;

	room_init();
    room_load(1);
}

static void game_do_player_death(void){
	SDL_Point pos = sprite_get_center(player.sprite);
	particles_spawn(pos, 0.f, 10.0f, 100); 

	player.sprite->x = (WINDOW_WIDTH / 2);
	player.sprite->y = (WINDOW_HEIGHT / 2);
}

void game_update(int delta) {

	particles_update(delta);

    const uint8_t *keys = SDL_GetKeyboardState(NULL);
    Sprite *player_s = player.sprite;

	if (player.is_bat) {
		player_s->hitbox_scale_x = player_s->hitbox_scale_y = 0.5f;
	} else {
		player_s->hitbox_scale_x = 0.5f;
		player_s->hitbox_scale_y = 1.0f;
	}

	const float accel   = (PLAYER_ACCEL * (delta / 16.0f));
	const float max_imp = player.is_bat ? PLAYER_MAX_IMPULSE_BAT : PLAYER_MAX_IMPULSE;

	bool pressed = false;

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
		if (player.is_bat) {
			player.y_velocity = MAX(
				player.y_velocity - accel,
				-(max_imp * accel)
			);
		}
		pressed = true;
    } else if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
		if (player.is_bat) {
			player.y_velocity = MIN(
				player.y_velocity + accel,
				max_imp * accel
			);
		}
		pressed = true;
    } else if (player.is_bat) {
		int sign = player.y_velocity > 0 ? 1 : -1;
		if(abs(player.y_velocity) < 0.01f){
			player.y_velocity = 0;
		} else {
			player.y_velocity -= MIN(abs(player.y_velocity), accel / 2.0f) * sign;
		}
	}

    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) {
		player.x_velocity = MAX(
			player.x_velocity - accel,
			-(max_imp * accel)
		);
		player_s->flip_mode = SDL_FLIP_HORIZONTAL;
		pressed = true;
    } else if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
		player.x_velocity = MIN(
			player.x_velocity + accel,
			max_imp * accel
		);
		player_s->flip_mode = SDL_FLIP_NONE;
		pressed = true;
    } else {
		int sign = player.x_velocity > 0 ? 1 : -1;
		if(abs(player.x_velocity) < 0.01f){
			player.x_velocity = 0;
		} else {
			player.x_velocity -= MIN(abs(player.x_velocity), accel) * sign;
		}
	}

	if (player.bat_timer > 0) {
		player.bat_timer -= delta;
		if (player.bat_timer <= 0) {
			player.bat_timer = 0;
			player.is_bat = false;
			sprite_set_tex(player_s, "res/sprites/vamp.png", 0);
		}
	}

	if (!player.bat_timer && keys[SDL_SCANCODE_SPACE]) {
		sprite_set_tex(player_s, "res/sprites/bat.png", 0);
		player.is_bat = true;
		player.bat_timer = 2000;
		player.y_velocity = 0;
	}


	if (!player.is_bat) {
		if (player.y_velocity < PLAYER_TERMINAL_VEL) {
			player.y_velocity = MAX(PLAYER_TERMINAL_VEL, player.y_velocity + (float)delta / 16.0f);
		} else {
			player.y_velocity = PLAYER_TERMINAL_VEL;
		}
	}

	// Animation
	if (player.is_bat || pressed) {
		player.anim_timer += delta;

		if (player.anim_timer > 200) {
			player_s->cur_frame = (player_s->cur_frame + 1) % player_s->num_frames;
			player.anim_timer = 0;
		}
	} else {
		player_s->cur_frame = 0;
	}

	// Collision with tiles
	SDL_Rect x_rect = sprite_get_hit_box(player_s), y_rect = x_rect;

	x_rect.x += player.x_velocity;
	y_rect.y += player.y_velocity;

	bool collision_x = false;
	bool collision_y = false;

    bool should_kill = false;
    
	for (int i = 1; i < num_sprites; ++i) {
		if (sprites[i].collision_type == COLLISION_BOX) {

			SDL_Rect s_rect = sprite_get_hit_box(sprites + i);
			SDL_Rect intersect;

			if (!collision_x && SDL_IntersectRect(&x_rect, &s_rect, &intersect)) {
				collision_x = true;

				if (sprites[i].collision_response == RESP_KILL) {
					should_kill = true;
					break;
				}

				player_s->x += player.x_velocity;
				player.x_velocity = 0;
				if (intersect.x <= x_rect.x) {
					player_s->x += intersect.w;
				} else {
					player_s->x -= intersect.w;
				}
			}

			if (!collision_y && SDL_IntersectRect(&y_rect, &s_rect, &intersect)) {
				collision_y = true;

				if (sprites[i].collision_response == RESP_KILL) {
					should_kill = true;
					break;
				}

				player_s->y += player.y_velocity;
				player.y_velocity = 0;
				if (intersect.y <= y_rect.y) {
					player_s->y += intersect.h;
				} else {
					player_s->y -= intersect.h;
				}
			}

		}
	}

    if (should_kill) {
		game_do_player_death();
	} else {
		if (!collision_x) {
			player_s->x += player.x_velocity;
		}

		if (!collision_y) {
			player_s->y += player.y_velocity;
		}
	}

	// Collision with edges of screen
	SDL_Point p = sprite_get_center(player_s);
	if (p.x > WINDOW_WIDTH) {
		room_switch(ROOM_RIGHT);
		player_s->x = 1;
		printf("setting player x %d\n", player_s->x);
	} else if(p.x < 0) {
		room_switch(ROOM_LEFT);
		player_s->x = WINDOW_WIDTH - (player_s->w + 1);
		printf("setting player x %d\n", player_s->x);
	}

	if (p.y > WINDOW_HEIGHT) {
		room_switch(ROOM_DOWN);
		player_s->y = 1;
		printf("setting player y %d\n", player_s->y);
	} else if(p.y < 0) {
		room_switch(ROOM_UP);
		player_s->y = WINDOW_HEIGHT - (player_s->h + 1);
		printf("setting player y %d\n", player_s->y);
	}
}

void game_draw(void) {

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	SDL_RenderDrawRect(renderer, NULL);

	for (int i = 0; i < 32; ++i) {
		SDL_RenderDrawLine(renderer, i * 32, 0, i * 32, WINDOW_HEIGHT);
	}

	for (int i = 0; i < 32; ++i) {
		SDL_RenderDrawLine(renderer, 0, i * 32, WINDOW_WIDTH, i * 32);
	}

	for(int i = 1; i < num_sprites; ++i){
		sprite_draw(sprites + i);
	}

	particles_draw();

	// Draw player on top
	sprite_draw(sprites);

	if (player.bat_timer) {
		SDL_Rect bar_rect = {
			.x = (WINDOW_WIDTH / 2) - (BAT_BAR_WIDTH / 2),
			.y = 400,
			.w = BAT_BAR_WIDTH * (player.bat_timer / 2000.0f),
			.h = BAT_BAR_HEIGHT,
		};

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &bar_rect);

		bar_rect.w = BAT_BAR_WIDTH;

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &bar_rect); 
	}

	// Debug
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_Rect r = sprite_get_hit_box(sprites);
	SDL_RenderDrawRect(renderer, &r);	
}
