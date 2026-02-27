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
#include "src/engine/entity.h"
#include "src/engine/animation.h"
#include "src/engine/audio.h"

static Mix_Music *MUSIC_STAGE_1;
static Mix_Chunk *SOUND_JUMP;

static const f32 SPEED_ENEMY_LARGE = 200;
static const f32 SPEED_ENEMY_SMALL = 4000;
static const f32 HEALTH_ENEMY_LARGE = 7;
static const f32 HEALTH_ENEMY_SMALL = 3;


typedef enum collision_layer
{
    COLLISION_LAYER_PLAYER = 1,
    COLLISION_LAYER_ENEMY = 1 << 1,
    COLLISION_LAYER_TERRAIN = 1 << 2
} Collision_Layer;

static bool should_quit = false;

vec4 player_color = {0, 1, 1, 1};
bool player_is_grounded = false;

// Add logging function
void log_to_file(const char* msg) {
    FILE* f = fopen("debug_log.txt", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

static void input_handle(Body *body_player)
{
    if(global.input.escape){
        should_quit = true;
    }
    f32 velx = 0;
    f32 vely = body_player->velocity[1];

    if (global.input.right){
        velx += 600;
    }
    if (global.input.left){
        velx -= 600;
    }
    if (global.input.up && player_is_grounded){
            player_is_grounded = false;
            vely = 2000;
            audio_sound_play(SOUND_JUMP);
    }
    body_player->velocity[0] = velx;
    body_player->velocity[1] = vely;
}

void player_on_hit(Body *self, Body *other, Hit hit)
{
    if (other->collision_layer == COLLISION_LAYER_ENEMY){
        player_color[0] = 1;
        player_color[2] = 0;
    }
}

void player_on_hit_static(Body *self, Static_Body *other, Hit hit)
{
    if (hit.normal[1] > 0){
        player_is_grounded = true;
    }
}

void enemy_small_on_hit_static(Body *self, Static_Body *other, Hit hit)
{
    if (hit.normal[0] > 0){
        self->velocity[0] = SPEED_ENEMY_SMALL;
    }
    if (hit.normal[0] < 0){
        self->velocity[0] = -SPEED_ENEMY_SMALL;
    }
}

void enemy_large_on_hit_static(Body *self, Static_Body *other, Hit hit)
{
    if (hit.normal[0] > 0){
        self->velocity[0] = SPEED_ENEMY_LARGE;
    }
    if (hit.normal[0] < 0){
        self->velocity[0] = -SPEED_ENEMY_LARGE;
    }
}

// === FIRE WHERE THE ENEMIES FALL INTO ===
void fire_on_hit(Body *self, Body *other, Hit hit)
{
    if (other->collision_layer == COLLISION_LAYER_ENEMY)
    {
        for (usize i = 0; i < entity_count(); ++i)
        {
            Entity *entity = entity_get(i);

            if (entity->body_id == hit.other_id){
                Body *body = physics_body_get(entity->body_id);
                body->is_active = false;
                entity->is_active = false;
                break;
            }
        }
    }
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

    SDL_Window *window = render_init();
    log_to_file("render_init complete");

    log_to_file("About to call physics_init");
    physics_init();
    log_to_file("physics_init complete");

    log_to_file("About to call entity_init");
    entity_init();
    log_to_file("entity_init complete");

    log_to_file("About to call animation_init");
    animation_init();
    log_to_file("animation_init complete");

    log_to_file("About to call audio_init");
    audio_init();
    log_to_file("audio_init complete");

    audio_sound_load(&SOUND_JUMP, "assets/jump.wav");
    audio_music_load(&MUSIC_STAGE_1, "assets/assets_breezys_mega_quest_2_stage_1.mp3");
    audio_music_play(MUSIC_STAGE_1);

    SDL_ShowCursor(false);

    u8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
    u8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;
    u8 fire_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_PLAYER;

    usize player_id = entity_create((vec2){100, 200}, (vec2){24, 24}, (vec2){0, 0}, COLLISION_LAYER_PLAYER, player_mask, false, player_on_hit, player_on_hit_static);

    i32 window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);
    f32 width = window_width / render_get_scale();
    f32 height = window_height / render_get_scale();

    // Bottom floor
    u32 static_body_a_id = physics_static_body_create(
        (vec2){width * 0.5, 12.5},
        (vec2){width, 25},
        COLLISION_LAYER_TERRAIN);

    // Right wall
    u32 static_body_b_id = physics_static_body_create(
        (vec2){width - 12.5, height * 0.5},
        (vec2){25, height},
        COLLISION_LAYER_TERRAIN);

    // Top ceiling
    u32 static_body_c_id = physics_static_body_create(
        (vec2){width * 0.5, height - 12.5},
        (vec2){width, 25},
        COLLISION_LAYER_TERRAIN);

    // Left wall
    u32 static_body_d_id = physics_static_body_create(
        (vec2){12.5, height * 0.5},
        (vec2){25, height},
        COLLISION_LAYER_TERRAIN);

    // Middle obstacle
    u32 static_body_e_id = physics_static_body_create(
        (vec2){width * 0.5, height * 0.5},
        (vec2){62.5, 62.5},
        COLLISION_LAYER_TERRAIN);

    usize entity_fire = entity_create((vec2){370, 50}, (vec2){25, 25}, (vec2){0}, 0, fire_mask, true, fire_on_hit, NULL);

    Sprite_Sheet sprite_sheet_player;
    render_sprite_sheet_init(&sprite_sheet_player, "assets/player.png", 24, 24);

    usize adef_player_walk_id = animation_definition_create(
         &sprite_sheet_player,
         (f32[]){0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1},
         (u8[]){0, 0, 0, 0, 0, 0, 0},
         (u8[]){1, 2, 3, 4, 5, 6, 7},
         7
    );
    usize adef_player_idle_id = animation_definition_create(&sprite_sheet_player, (f32[]){0}, (u8[]){0}, (u8[]){0}, 1);
    usize anim_player_walk_id = animation_create(adef_player_walk_id, true);
    usize anim_player_idle_id = animation_create(adef_player_idle_id, false);

    Entity *player = entity_get(player_id);
    player->animation_id = anim_player_idle_id;

    f32 spawn_timer = 0;

    log_to_file("Entering main loop");

    while (!should_quit){
        time_update();

        log_to_file("Done with time_update");

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

        Entity *player = entity_get(player_id);
        Body *body_player = physics_body_get(player->body_id);

        if (body_player->velocity[0] != 0){
            player->animation_id = anim_player_walk_id;
        } else {
            player->animation_id = anim_player_idle_id;
        }

        Static_Body *static_body_a = physics_static_body_get(static_body_a_id);
        Static_Body *static_body_b = physics_static_body_get(static_body_b_id);
        Static_Body *static_body_c = physics_static_body_get(static_body_c_id);
        Static_Body *static_body_d = physics_static_body_get(static_body_d_id);
        Static_Body *static_body_e = physics_static_body_get(static_body_e_id);

        input_update();
        log_to_file("Done with input_update");
        input_handle(body_player);
        log_to_file("Done with input_handle");

        log_to_file("call physics_update");
        physics_update();
        log_to_file("Done with physics_update");

        animation_update(global.time.delta);
        log_to_file("Done with animation_update");

        // === spawn eenemies ===
        {
            spawn_timer -= global.time.delta;
            if (spawn_timer <= 0)
            {
                spawn_timer = (f32)((rand() % 200) + 200) / 100.f;
                spawn_timer += 0.2;

                for (u32 i = 0; i < 5; ++i)
                {
                    bool is_flipped = rand() % 100 >= 50;

                    f32 spawn_x = is_flipped ? 540 : 100;

                    usize entity_id = entity_create(
                        (vec2){spawn_x, 200},
                        (vec2){20, 20},
                        (vec2){0, 0},
                        COLLISION_LAYER_ENEMY,
                        enemy_mask,
                        false,
                        NULL,
                        enemy_small_on_hit_static
                    );

                    Entity *entity = entity_get(entity_id);
                    Body *body = physics_body_get(entity->body_id);
                    float speed = SPEED_ENEMY_SMALL *((rand() % 100) * 0.01) + 100;
                    body->velocity[0] = is_flipped ? -speed : speed;
                }
            }
        }

        log_to_file("Start render_begin");
        render_begin();
        log_to_file("Done with render_begin");

        for (usize i = 0; i < entity_count(); ++i)
        {
            Entity *entity = entity_get(i);
            Body *body = physics_body_get(entity->body_id);

            if (body->is_active){
                render_aabb((f32*)body, TORQUOISE);
            } else {
                render_aabb((f32*)body, RED);
            }
        }
        render_aabb((f32*)static_body_a, WHITE);
        render_aabb((f32*)static_body_b, WHITE);
        render_aabb((f32*)static_body_c, WHITE);
        render_aabb((f32*)static_body_d, WHITE);
        render_aabb((f32*)static_body_e, WHITE);
        render_aabb((f32*)body_player, player_color);

        log_to_file("Render animated entities");

//       ======== RENDER ANIMATED ENTITIES =========
        for (usize i = 0; i < entity_count(); ++i){
            Entity *entity = entity_get(i);
            if (!entity->is_active){
                continue;
            }

            if (entity->animation_id == (usize)-1){
                continue;
            }

            Body *body = physics_body_get(entity->body_id);
            Animation *anim = animation_get(entity->animation_id);
            Animation_Definition *adef = anim->definition;
            Animation_Frame *aframe = &adef->frames[anim->current_frame_index];

            if (body->velocity[0] < 0){
                anim->is_flipped = true;
            } else if (body->velocity[0] > 0){
                anim->is_flipped = false;
            }

            render_sprite_sheet_frame(adef->sprite_sheet, aframe->row, aframe->column, body->aabb.position, anim->is_flipped);
        }
        log_to_file("about to render_end");

        render_end(window, sprite_sheet_player.texture_id);
        log_to_file("Finished render_end");
        player_color[0] = 0;
        player_color[2] = 1;

        time_update_late();
    }

    log_to_file("Exiting normally");
    printf("Exiting normally\n");
    return 0;
}
