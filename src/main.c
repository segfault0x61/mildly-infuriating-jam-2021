#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "game.h"
#include "sound.h"
#include "sprite.h"

bool running = true;

SDL_Window* win;
SDL_Renderer* renderer;
SDL_Rect viewport;

static float titlescreen_timer = 2000;

static void handle_event(SDL_Event* e) {
    switch (e->type) {
        case SDL_QUIT: {
            running = false;
        } break;
    }
}

static double timer_start;
static uint64_t timer_freq;
static Sprite titlescreen;

static double get_ms(void) {
    return (uint64_t)(SDL_GetPerformanceCounter() / timer_freq) / 1000.0;
}

static void main_loop(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        handle_event(&e);
    }

    double now, timer_diff;

    while ((now = get_ms()), (timer_diff = (now - timer_start)) < 16.67) {
        double delay = (16.67 - timer_diff);
        if (delay > 1) SDL_Delay(delay);
    }
    timer_start = now;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (titlescreen_timer > 0) {
        titlescreen_timer -= timer_diff;
        sprite_draw(&titlescreen);
    } else {
        game_update(timer_diff);
        game_draw();
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv) {
    srand(time(0));

    SDL_Init(SDL_INIT_EVERYTHING);

    if (SDL_CreateWindowAndRenderer(WIN_WIDTH, WIN_HEIGHT, 0, &win, &renderer) == -1) {
        fprintf(stderr, "SDL error creating window/renderer: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetWindowTitle(win, "The Struggles of Alucard");
    
    SDL_Surface *icon = IMG_Load("res/sprites/icon.png");
    SDL_SetWindowIcon(win, icon);
    SDL_FreeSurface(icon);

    SDL_RenderGetViewport(renderer, &viewport);

    timer_freq = SDL_GetPerformanceFrequency() / (1000 * 1000);

    sound_init();
    game_init();

    Sprite ts = {
        .x = (WIN_WIDTH / 2) - (512 / 2),
        .y = (WIN_HEIGHT / 2) - (256 / 2),
        .w = 512,
        .h = 256,
    };
    titlescreen = ts;

    sprite_set_tex(&titlescreen, "res/sprites/title.png", 1);

    timer_start = get_ms();

    while (running) main_loop();
}
