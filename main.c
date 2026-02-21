#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "src/engine/global.h"
#include "src/engine/render/render.h"
#include "src/engine/input.h"
#include "src/engine/config.h"
#include "src/engine/time.h"
#include "src/engine/physics.h"
#include "src/engine/util.h"

static bool should_quit = false;
static vec2 pos;

// Add logging function
void log_to_file(const char* msg) {
    FILE* f = fopen("debug_log.txt", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

static void input_handle(void)
{
    if(global.input.left == KS_PRESSED || global.input.left == KS_HELD)
       should_quit = true;

    i32 x, y;
    SDL_GetMouseState(&x, &y);

    pos[0] = (f32)x;
    pos[1] = global.render.height - y;


}

int main(int argc, char* argv[]) {
    log_to_file("=== Program Started ===");

    printf("Starting render_init...\n");
    fflush(stdout);

    log_to_file("About to call time_init");
    time_init(60);
    log_to_file("time_init complete");

    log_to_file("About to call config_init");
    config_init();
    log_to_file("config_init complete");

    log_to_file("About to call render_init");
    render_init();
    log_to_file("render_init complete");

    log_to_file("About to call physics_init");
    physics_init();
    log_to_file("physics_init complete");

/* ====================================
       POLYGONS FOR PHYSICS SIMULATION
   ====================================
    u32 body_count = 100;
    for (u32 i = 0; i < body_count; ++i)
    {
        usize body_index = physics_body_create(
            (vec2){
                rand() % (i32)global.render.width,
                rand() % (i32)global.render.height
            },
            (vec2){
                rand() % 100,
                rand() % 100
            }
        );
        Body *body = physics_body_get(body_index);
        body->acceleration[0] = rand() % 200 - 100;
        body->acceleration[1] = rand() % 200 - 100;
    }
*/

    pos[0] = global.render.width * 0.5;
    pos[1] = global.render.height * 0.5;

    SDL_ShowCursor(false);

    AABB test_aabb = {
        .position = {global.render.width * 0.5, global.render.height * 0.5},
        .half_size = {50, 50}
    };

    log_to_file("Entering main loop");

    while (!should_quit){
        time_update();

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
            case SDL_QUIT:
                should_quit = true;
                log_to_file("Quit: SDL_QUIT event received");
                break;
            default:
                break;
            }
        }

        input_update();
        input_handle();

        // Check why we're quitting
        if (should_quit) {
            log_to_file("Loop exiting: should_quit is true");
            char buf[100];
            sprintf(buf, "Escape key state: %d", global.input.escape);
            log_to_file(buf);
        }

        physics_update();

        render_begin();

        render_aabb((f32*)&test_aabb, (vec4){1, 1, 1, 0.5});

        if (physics_point_intersect_aabb(pos, test_aabb))
            render_quad(pos, (vec2){5, 5}, MAGENTA);
        else
            render_quad(pos, (vec2){5, 5}, WHITE);



        //render_quad(pos, (vec2){50, 50}, (vec4){0, 1, 0, 1});

/* ====================================
       POLYGONS FOR PHYSICS SIMULATION
   ====================================
        for (u32 i = 0; i < body_count; ++i)
        {
            Body *body = physics_body_get(i);
            render_quad(body->aabb.position, body->aabb.half_size, (vec4){1, 0, 0, 1});

            if (body->aabb.position[0] > global.render.width || body->aabb.position[0] < 0)
                body->velocity[0] *= -1;
            if (body->aabb.position[1] > global.render.height || body->aabb.position[1] < 0)
                body->velocity[1] *= -1;

            if (body->velocity[0] > 500)
                body->velocity[0] = 500;
            if (body->velocity[0] < -500)
                body->velocity[0] = -500;

            if (body->velocity[1] > 500)
                body->velocity[1] = 500;
            if (body->velocity[1] < -500)
                body->velocity[1] = -500;
        }
*/
        render_end();

        time_update_late();
    }

    log_to_file("Exiting normally");
    printf("Exiting normally\n");
    return 0;
}
