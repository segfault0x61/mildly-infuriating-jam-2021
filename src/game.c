#include "game.h"
#include "sprite.h"
#include "room.h"
#include "particles.h"
#include "sound.h"

#include <SDL2/SDL.h>

#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>

typedef struct {
    Sprite *sprite;
    int speed;
    int anim_timer;
	bool is_bat;
	float x_velocity;
	float y_velocity;
	int bat_timer;
	int cooldown;
	float x_remainder;
	float y_remainder;
} Player;

static Player player = {
    .sprite = sprites,
    .speed = 2,
	.is_bat = false,
};

static int screen_shake_timer;
static int screen_shake_amount;

static bool has_landed = false;

#define PLAYER_TERMINAL_VEL 8.0f
#define BAT_TERMINAL_VEL 4.0f
#define PLAYER_ACCEL 0.2f
#define PLAYER_MAX_IMPULSE 12.0f
#define PLAYER_MAX_IMPULSE_BAT 15.0f
#define BAT_BAR_WIDTH 100
#define BAT_BAR_HEIGHT 20
#define BAT_TIMER_MAX 2000

void game_init(void) {

    // Draw player
    sprite_push_tex(400, 200, 64, 64, "res/sprites/vamp.png");
	sprites[0].radius = 16;
	sprites[0].hitbox_scale_x = 0.45f;
	sprites[0].hitbox_scale_y = 1.0f;

	room_init();
    room_load(1);
}

static void game_screen_shake(int duration, int amount){
	screen_shake_timer = duration;
	screen_shake_amount = amount;
}

static void game_do_player_death(void){
	SDL_Point pos = sprite_get_center(player.sprite);
	particles_spawn(pos, 0.f, 30.0f, 100); 

	if (player.is_bat) {
		sound_play("res/sfx/bat.ogg", 0);
	}
	sound_play("res/sfx/die.ogg", 0);

	player.sprite->x = (WINDOW_WIDTH / 2);
	player.sprite->y = (WINDOW_HEIGHT / 2);

	game_screen_shake(250, 10);
}

void game_update(int delta) {

	particles_update(delta);
	room_update(delta);

    const uint8_t *keys = SDL_GetKeyboardState(NULL);
    Sprite *player_s = player.sprite;

	const float accel   = (PLAYER_ACCEL * (delta / 16.0f));
	const float max_imp = player.is_bat ? PLAYER_MAX_IMPULSE_BAT : PLAYER_MAX_IMPULSE;

	bool moving_on_up = false;

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
		if (player.is_bat) {
			player.y_velocity = MAX(
				player.y_velocity - (accel * 1.2f),
				-(max_imp * accel * 1.2f)
			);
			moving_on_up = true;
		}
    } 
	if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
		if (player.is_bat) {
			player.y_velocity = MIN(
				player.y_velocity + accel,
				max_imp * accel
			);
		}
    } else if (player.is_bat && !moving_on_up) {
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
    } else if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
		player.x_velocity = MIN(
			player.x_velocity + accel,
			max_imp * accel
		);
		player_s->flip_mode = SDL_FLIP_NONE;
    } else {
		int sign = player.x_velocity > 0 ? 1 : -1;
		if(abs(player.x_velocity) < 0.01f){
			player.x_velocity = 0;
		} else {
			player.x_velocity -= MIN(abs(player.x_velocity), accel) * sign;
		}
	}

	if (player.cooldown > 0) {
		player.cooldown -= delta;
	}

	if (player.bat_timer) {
		player.bat_timer -= delta;
		if (player.bat_timer <= 0) {
			game_screen_shake(100, 4);
			
			player.bat_timer = 0;
			player.is_bat = false;

			player.cooldown = 200;
			
			sound_play("res/sfx/unshapeshift.ogg", 0);
			sprite_set_tex(player_s, "res/sprites/vamp.png", 0);

			// Hack to stop clipping through walls 
			int x_hack = player_s->x < 32 ? 14 : player_s->x > (WINDOW_WIDTH-32) ? -14 : 0; 
			int y_hack = player_s->y < 32 ? 14 : player_s->y > (WINDOW_HEIGHT-32) ? -14 : 0; 
			player_s->x += x_hack;
			player_s->y += y_hack;
			player.x_velocity = player.y_velocity = 0;
			player.x_remainder = player.y_remainder = 0;
		}
	}

	if (has_landed && !player.bat_timer && keys[SDL_SCANCODE_SPACE]) {
		if (player.cooldown > 0) {
			sound_play("res/sfx/nope.ogg", 0);
		} else {
			sound_play("res/sfx/shapeshift.ogg", 0);
			sound_play("res/sfx/bat.ogg", 0);
			sprite_set_tex(player_s, "res/sprites/bat.png", 0);
			player.is_bat = true;
			player.bat_timer = BAT_TIMER_MAX;
			player.y_velocity = 0;
			has_landed = false;
		}
	}

	if (player.is_bat) {
		player_s->hitbox_scale_x = 0.5f;
		player_s->hitbox_scale_y = 0.5f;
	} else {
		player_s->hitbox_scale_x = 0.5f;
		player_s->hitbox_scale_y = 1.0f;
	}

	float grav = player.is_bat ? ((float)delta / 18.0f) : ((float)delta / 16.0f);
	float tv   = player.is_bat ? BAT_TERMINAL_VEL : PLAYER_TERMINAL_VEL;

	if (!player.is_bat || !moving_on_up) {
		player.y_velocity = MIN(tv, player.y_velocity + grav);
	}

	// Animation
	if (player.is_bat || fabs(player.x_velocity) > FLT_EPSILON) {
		player.anim_timer += delta;

		if (player.anim_timer > 200) {
			player_s->cur_frame = (player_s->cur_frame + 1) % player_s->num_frames;
			player.anim_timer = 0;
		}
	} else {
		player_s->cur_frame = 0;
	}

	// Collision with tiles
	SDL_Rect hit_box = sprite_get_hit_box(player_s);
	SDL_Rect x_rect  = hit_box;
	SDL_Rect y_rect  = hit_box;

	x_rect.x += (player.x_velocity + player.x_remainder);
	y_rect.y += (player.y_velocity + player.y_remainder);

	bool collision_x = false;
	bool collision_y = false;

    bool should_kill = false;
    
	for (int i = 1; i < num_sprites; ++i) {
		if (sprites[i].collision_type == COLLISION_BOX) {

			SDL_Rect s_rect = sprite_get_hit_box(sprites + i);
			SDL_Rect intersect;

			if (SDL_IntersectRect(&y_rect, &s_rect, &intersect)) {
				collision_y = true;

				if (sprites[i].collision_response == RESP_KILL) {
					should_kill = true;
					break;
				}

				if (sprites[i].collision_response == RESP_POWERUP) {
					puts("this is the part where you get a powerup");
					
					// TODO: Fix this
					sprites[i].collision_type = COLLISION_NONE;
					sprites[i].tex = 0;
					
					if (player.is_bat) {
						player.bat_timer = BAT_TIMER_MAX;
					}
				} else {
					if (!player.is_bat && player.cooldown <= 0 && player.y_velocity > 0){
						has_landed = true;
					}

					player_s->y += player.y_velocity; 
					player.y_velocity = 0;
					player.y_remainder = 0;
					if (intersect.y <= y_rect.y) {
						player_s->y += intersect.h;
						y_rect.y += intersect.h;
						x_rect.y += intersect.h + 1;
					} else {
						player_s->y -= intersect.h;
						y_rect.y -= intersect.h;
						x_rect.y -= intersect.h + 1;
					}
				}
			}

			if (SDL_IntersectRect(&x_rect, &s_rect, &intersect)) {
				collision_x = true;

				if (sprites[i].collision_response == RESP_KILL) {
					should_kill = true;
					break;
				}

				if (sprites[i].collision_response == RESP_POWERUP) {
					puts("this is the part where you get a powerup");
					
					// TODO: Fix this
					sprites[i].collision_type = COLLISION_NONE;
					sprites[i].tex = 0;


					if (player.is_bat) {
						player.bat_timer = BAT_TIMER_MAX;
					}
				} else {
					player_s->x += (player.x_velocity + player.x_remainder);
					player.x_velocity = 0;
					player.x_remainder = 0;
					if (intersect.x <= x_rect.x) {
						player_s->x += intersect.w;
						x_rect.x += intersect.w;
					} else {
						player_s->x -= intersect.w;
						x_rect.x -= intersect.w;
					}
				}
			}

		}
	}

    if (should_kill) {
		game_do_player_death();
	} else {
		if (!collision_x) {
			float x_vel_i;
			float x_vel_f = modff(player.x_velocity, &x_vel_i);
			player_s->x += (player.x_velocity + player.x_remainder);
			player.x_remainder = x_vel_f;
		}

		if (!collision_y) {
			float y_vel_i;
			float y_vel_f = modff(player.y_velocity, &y_vel_i);
			player_s->y += (player.y_velocity + player.y_remainder);
			player.y_remainder = y_vel_f;
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
		puts("going up");
		room_switch(ROOM_UP);
		player_s->y = WINDOW_HEIGHT - (player_s->h + 1);
		printf("setting player y %d\n", player_s->y);
	}

	if (screen_shake_timer > 0) {
		screen_shake_timer -= delta;

		if (screen_shake_timer > 0) {
			SDL_Rect vp = viewport;
			vp.x += (rand() % screen_shake_amount) - (screen_shake_amount / 2);
			vp.y += (rand() % screen_shake_amount) - (screen_shake_amount / 2);
			SDL_RenderSetViewport(renderer, &vp);
		} else {
			screen_shake_timer = 0;
			SDL_RenderSetViewport(renderer, &viewport);
		}
	}
}

void game_draw(void) {

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	SDL_RenderDrawRect(renderer, NULL);

	// for (int i = 0; i < 32; ++i) {
	// 	SDL_RenderDrawLine(renderer, i * 32, 0, i * 32, WINDOW_HEIGHT);
	// }

	// for (int i = 0; i < 32; ++i) {
	// 	SDL_RenderDrawLine(renderer, 0, i * 32, WINDOW_WIDTH, i * 32);
	// }

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
			.w = BAT_BAR_WIDTH * (player.bat_timer / (float)BAT_TIMER_MAX),
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
