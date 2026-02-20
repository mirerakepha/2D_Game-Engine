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
        pos[0] -= 500 * global.time.delta;
    if(global.input.right == KS_PRESSED || global.input.right == KS_HELD)
        pos[0] += 500 * global.time.delta;
    if(global.input.up == KS_PRESSED || global.input.up == KS_HELD)
        pos[1] += 500 * global.time.delta;
    if(global.input.down == KS_PRESSED || global.input.down == KS_HELD)
        pos[1] -= 500 * global.time.delta;
    if(global.input.escape == KS_PRESSED || global.input.escape == KS_HELD)
        should_quit = true;
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

    printf("render_init complete!\n");
    printf("Window created successfully\n");
    fflush(stdout);

    pos[0] = global.render.width * 0.5;
    pos[1] = global.render.height * 0.5;

    log_to_file("Entering main loop");

    while (!should_quit){
        time_update();

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
            case SDL_QUIT:
                should_quit = true;
                break;
            default:
                break;
            }
        }

        input_update();
        input_handle();

        render_begin();
        render_quad(pos, (vec2){50, 50}, (vec4){0, 1, 0, 1});
        render_end();

        time_update_late();
    }

    log_to_file("Exiting normally");
    printf("Exiting normally\n");
    return 0;
}
