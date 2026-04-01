/*!************************************************************************
\file:						level2.cpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This file implements Level 2 (Boss Room) functions to load the assets from a file,
                            initialize the level, update game logic, render, free resources
**************************************************************************/
#include "AEEngine.h"
#include <cstdio>
#include "map.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "collision.hpp"
#include "game_state_manager.hpp"
#include "puzzle.hpp"
#include "utils.hpp"
#include "minimap.hpp"
#include "enemy_manager.hpp"
#include "ui_manager.hpp"
#include "game_state_list.hpp"
#include "end_screen.hpp"

#define NUM_TILE_TYPES 9
#define TILE_ROWS 36
#define TILE_COLS 64

static bool enemy_spawned = false;

static AEGfxTexture* skeleton_texture = nullptr;

void level2_load()
{
    for (int i = 0; i < NUM_TILE_TYPES; i++)
        TileTextures[i] = nullptr;

    // Load textures properly
    char filename[128];
    for (int i = 0; i < NUM_TILE_TYPES; ++i)
    {
        sprintf_s(filename, "Assets/Sprites/Tiles/tile_%04d.png", i);
        TileTextures[i] = AEGfxTextureLoad(filename);
        if (!TileTextures[i])
            printf("Failed to load: %s\n", filename);
    }
    Puzzle::load_puzzle_textures();
    load_ui_textures(&ui_manager);
    skeleton_texture = AEGfxTextureLoad("Assets/Sprites/Tiles/dead_last_location.png");
    end_screen_load();
}

void level2_init()
{
    enemy_spawned = false;
    zoom = 1.0f;
    // tile_size = 64.f;
    player_system_init(&player_instance, TileTextures[4], TileTextures[3], skeleton_texture);

    player_instance.stats.character_size = tile_size - 8.0f;

    // FIX 1: Set player positions to bottom of boss room
    // Based on initialize_boss_map(): rows 2-11, columns 6-15
    // Map origin: X = -800, Y = 450
    const f32 TILE_SIZE = 64.0f;
    const f32 MAP_START_X = -1600.0f / 2.0f; // -800
    const f32 MAP_START_Y = 900.0f / 2.0f;   // 450

    // Player at bottom of boss room
    f32 bottomX = MAP_START_X + (10.5f * TILE_SIZE); // Column 10.5 (center)
    f32 bottomY = MAP_START_Y - (10.0f * TILE_SIZE); // Row 10 (near bottom, avoiding wall)

    player_instance.char1.x = bottomX - 25.0f; // Center left
    player_instance.char1.y = bottomY;

    player_instance.char2.x = bottomX + 25.0f; // Center right
    player_instance.char2.y = bottomY;

    // Ensure current player is set correctly
    player_instance.current_player = &player_instance.char1;
    player_instance.current_character = 1;

    // FIX 2: Initialize camera with player position
    cam = init_cam(&player_instance.current_player->x, &player_instance.current_player->y, 60.0f, 5.0f);

    init_mesh();

    totalWorldW = ((f32)TILE_COLS * tile_size) / minimap_zoom;
    totalWorldH = ((f32)TILE_ROWS * tile_size) / minimap_zoom;
    minimap_init(totalWorldW, totalWorldH, 200.0f);

    initialize_boss_map();
    generate_walls_from_map(boss_map, 64.0f);

    // Clear any existing enemies then lock zone so ghost/bat respawn never fires here
    boss_proj_manager.clear();
    enemy_manager_instance.clear_all();
    enemy_manager_instance.set_zone(boss_room);

    // Boss room coordinates from initialize_boss_map(): rows 3-10, cols 7-15
    // Center of boss room
    f32 dragon_x = MAP_START_X + (11 * TILE_SIZE);  // Column 11 (center of cols 7-15)
    f32 dragon_y = MAP_START_Y - (6.5 * TILE_SIZE); // Row 6.5 (center of rows 3-10)

    // Room bounds: approximate boundaries of the boss room
    // Adjusted for the boss room layout in initialize_boss_map()
    f32 room_left = MAP_START_X + (7 * TILE_SIZE) + (TILE_SIZE / 2.0f);
    f32 room_right = MAP_START_X + (15 * TILE_SIZE) - (TILE_SIZE / 2.0f);
    f32 room_top = MAP_START_Y - (3 * TILE_SIZE) + (TILE_SIZE / 2.0f);
    f32 room_bottom = MAP_START_Y - (10 * TILE_SIZE) - (TILE_SIZE / 2.0f);

    enemy_manager_instance.spawn_dragon(dragon_x, dragon_y, room_left, room_right, room_top, room_bottom);
}

void level2_update()
{
    pause_update();
    if (is_paused)
        return;
    dt = (f32)AEFrameRateControllerGetFrameTime();

    get_zoom(dt);

    // Bounds for boss room
    simulate_camera(&cam, &player_instance.current_player->x, &player_instance.current_player->y,
        -6000.0f, 6000.0f, -5000.0f, 5000.0f);

    player_system_update(&player_instance);
    entity_system_update(&player_instance, &enemy_manager_instance);

    // check_button_collision(player_instance.char1.x, player_instance.char1.y,
    //     player_instance.char2.x, player_instance.char2.y,
    //     player_instance.config.character_size / 2.0f);
    Puzzle::puzzle_button_manager();

    check_and_open_doors(&player_instance);

    // Update enemy manager (for the dragon)
    enemy_manager_instance.update(dt, &player_instance, &cam);
    boss_proj_manager.update(dt, &cam);
    boss_proj_manager.check_player_collision(&player_instance);

    if (enemy_manager_instance.get_active_enemy_count() > 0)
        enemy_spawned = true;

    // Lose condition
    if (player_is_dead(&player_instance))
    {
        is_win = false;
        restart_level_state = GS_BOSS_ROOM;
        next = GS_LOSE;
    }

    // Win condition (level 2 only)
    if (enemy_spawned && enemy_manager_instance.get_active_enemy_count() == 0)
    {
        is_win = true;
        restart_level_state = GS_BOSS_ROOM;
        next = GS_WIN;
    }
}

void level2_draw()
{
    draw_boss_map(tile_size);

    player_system_render(&player_instance);
    minimap_draw(&player_instance, map, TileTextures);
    // Minimap_DrawWalls(map);

    enemy_manager_instance.render();
    render_boss_projectiles(boss_proj_manager);
    render_enemy_hp(&ui_manager, &enemy_manager_instance);
    render_player_hud(&ui_manager, &player_instance);

    pause_draw();
}

void level2_free()
{
    // Cleanup enemies
    free_ui_textures(&ui_manager);
    boss_proj_manager.clear();
    enemy_manager_instance.clear_all();
}

void level2_unload()
{
    for (int i = 0; i < NUM_TILE_TYPES; ++i)
    {
        if (TileTextures[i])
        {
            AEGfxTextureUnload(TileTextures[i]);
            TileTextures[i] = nullptr;
        }
    }
    free_mesh();
    Puzzle::unload_puzzle_textures();
    player_system_free(&player_instance);

    if (skeleton_texture)
    {
        AEGfxTextureUnload(skeleton_texture);
        skeleton_texture = nullptr;
    }

    end_screen_unload();
    minimap_unload();
}