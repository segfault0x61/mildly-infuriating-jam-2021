#include "particles.h"
#include "game.h"
#include "sprite.h"

#define PARTICLE_SIZE 6
#define PARTICLE_TTL_LIMIT 1000

typedef struct {
    Sprite sprite;
    float x_velocity, x_position;
    float y_velocity, y_position;
    int timeToLive;
} Particle;

Particle particles[4096];
int particle_cursor;

void particles_spawn(SDL_Point pos, float xv, float yv, int amount) {
    for (int i = 0; i < amount; ++i) {
        xv += ((rand() % 800) - 400) / 100.0f;
        yv += ((rand() % 800) - 400) / 100.0f;

        Particle p = {
            .sprite = {
                .x = pos.x,
                .y = pos.y,
                .w = PARTICLE_SIZE,
                .h = PARTICLE_SIZE,
            },
            .x_position = pos.x,
            .y_position = pos.y,
            .x_velocity = xv,
            .y_velocity = yv,
            .timeToLive = PARTICLE_TTL_LIMIT + (rand() % 400)
        };

        particles[particle_cursor] = p;
        particle_cursor = (particle_cursor + 1) % array_count(particles);
    }
}

void particles_update(int delta) {
    for (int i = 0; i < array_count(particles); ++i) {
        Particle* p = particles + i;
        if (p->timeToLive <= 0) continue;

        if (p->timeToLive < PARTICLE_TTL_LIMIT) {
            p->y_velocity -= (delta / 8.0f);
            p->x_position += (p->x_velocity / (float)delta);
            p->y_position -= (p->y_velocity / (float)delta);

            p->sprite.x = p->x_position;
            p->sprite.y = p->y_position;
        }
        p->timeToLive -= delta;
    }
}

void particles_draw(void) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    for (int i = 0; i < array_count(particles); ++i) {
        Particle* p = particles + i;
        if (p->timeToLive <= 0 || p->timeToLive >= PARTICLE_TTL_LIMIT) continue;

        SDL_RenderFillRect(renderer, &p->sprite.rect);
    }
}
