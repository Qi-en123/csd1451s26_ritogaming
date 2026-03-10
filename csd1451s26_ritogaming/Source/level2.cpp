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

f32 totalWorldW2;
f32 totalWorldH2;
extern f32 dt;

void level2_load()
{
	for (int i = 0; i < NUM_TILE_TYPES; i++)
		TileTextures[i] = nullptr;

	// Load textures properly
	char filename[128];
	for (int i = 0; i < NUM_TILE_TYPES; ++i)
	{
		sprintf_s(filename, "Assets/Tiles/tile_%04d.png", i);
		TileTextures[i] = AEGfxTextureLoad(filename);
		if (!TileTextures[i])
			printf("Failed to load: %s\n", filename);
	}
	load_puzzle_textures();
    load_ui_textures(&ui_manager);
}

//void level2_initialize()
//{
//	zoom = 1.0f;
//	// Determine tile size to fit both width and height
//	tile_size = 64.f;
//	player_system_init(&player_instance);
//	player_instance.stats.character_size = tile_size - 4.0f;
//	//cam = init_cam(&player_instancer->x, &player_instance->y, 60.0f, 5.0f);
//	init_mesh();
//
//	//zoomlevel2 = 2.0f;
//	totalWorldW2 = ((f32)TILE_COLS * tile_size) / ZOOM_LEVEL;
//	totalWorldH2 = ((f32)TILE_ROWS * tile_size) / ZOOM_LEVEL;
//	Minimap_Init(totalWorldW2, totalWorldH2, 200.0f); // 200px wide map
//	//printf("Walls created: %zu\n", g_walls.size());
//
//	//js to test camera (UPDATE CURR_PLAYER TO CHANGE CAMERA VIEW)
//	//f32* curr_player_x{ &player_instance.char1_x }, * curr_player_y{ &player_instance.char1_y };
//
//	//cam2= init_cam(curr_player_x, curr_player_y, 60.0f, 0.0f);
//
//	//////////////////////////////////////////////////
//
//	initialize_boss_map();
//	generate_walls_from_map(boss_map, 64.0f); // step 2: make walls from map
//
//	intialize_puzzle_blocks();
//
//	//ShiftMapRight(2);
//}

//void level2_initialize()
//{
//	zoom = 1.0f;
//	// Determine tile size to fit both width and height
//	tile_size = 64.f;
//	player_system_init(&player_instance);
//	player_instance.stats.character_size = tile_size - 4.0f;
//
//	// FIX 1: Set player positions to bottom of boss room
//	// Based on initialize_boss_map(): rows 2-11, columns 6-15
//	// Map origin: X = -800, Y = 450
//	// Bottom-center position would be:
//	f32 bottomX = -800.0f + (10.5f * tile_size);  // Column 10.5 (center)
//	f32 bottomY = 450.0f - (10.0f * tile_size);   // Row 10 (near bottom, avoiding wall)
//
//	player_instance.char1.x = bottomX - 25.0f;     // Center left
//	player_instance.char1.y = bottomY;
//
//	player_instance.char2.x = bottomX + 25.0f;     // Center right  
//	player_instance.char2.y = bottomY;
//
//	// Ensure current player is set correctly
//	player_instance.current_player = &player_instance.char1;
//	player_instance.current_character = 1;
//
//	// FIX 2: Initialize camera with player position
//	cam = init_cam(&player_instance.current_player->x, &player_instance.current_player->y, 60.0f, 5.0f);
//
//	init_mesh();
//
//	//zoomlevel2 = 2.0f;
//	totalWorldW2 = ((f32)TILE_COLS * tile_size) / ZOOM_LEVEL;
//	totalWorldH2 = ((f32)TILE_ROWS * tile_size) / ZOOM_LEVEL;
//	Minimap_Init(totalWorldW2, totalWorldH2, 200.0f); // 200px wide map
//	//printf("Walls created: %zu\n", g_walls.size());
//
//	//js to test camera (UPDATE CURR_PLAYER TO CHANGE CAMERA VIEW)
//	//f32* curr_player_x{ &player_instance.char1_x }, * curr_player_y{ &player_instance.char1_y };
//
//	//cam2= init_cam(curr_player_x, curr_player_y, 60.0f, 0.0f);
//
//	//////////////////////////////////////////////////
//
//	initialize_boss_map();
//	generate_walls_from_map(boss_map, 64.0f); // step 2: make walls from map
//
//	//intialize_puzzle_blocks();
//
//	//ShiftMapRight(2);
//}

void level2_init()
{
    zoom = 1.0f;
    tile_size = 64.f;
    player_system_init(&player_instance, TileTextures[4], TileTextures[3]);
    player_instance.stats.character_size = tile_size - 4.0f;

    // FIX 1: Set player positions to bottom of boss room
    // Based on initialize_boss_map(): rows 2-11, columns 6-15
    // Map origin: X = -800, Y = 450
    const f32 TILE_SIZE = 64.0f;
    const f32 MAP_START_X = -1600.0f / 2.0f;  // -800
    const f32 MAP_START_Y = 900.0f / 2.0f;    // 450

    // Player at bottom of boss room
    f32 bottomX = MAP_START_X + (10.5f * TILE_SIZE);  // Column 10.5 (center)
    f32 bottomY = MAP_START_Y - (10.0f * TILE_SIZE);   // Row 10 (near bottom, avoiding wall)

    player_instance.char1.x = bottomX - 25.0f;     // Center left
    player_instance.char1.y = bottomY;

    player_instance.char2.x = bottomX + 25.0f;     // Center right  
    player_instance.char2.y = bottomY;

    // Ensure current player is set correctly
    player_instance.current_player = &player_instance.char1;
    player_instance.current_character = 1;

    // FIX 2: Initialize camera with player position
    cam = init_cam(&player_instance.current_player->x, &player_instance.current_player->y, 60.0f, 5.0f);

    init_mesh();

    totalWorldW2 = ((f32)TILE_COLS * tile_size) / ZOOM_LEVEL;
    totalWorldH2 = ((f32)TILE_ROWS * tile_size) / ZOOM_LEVEL;
    minimap_init(totalWorldW2, totalWorldH2, 200.0f);

    initialize_boss_map();
    generate_walls_from_map(boss_map, 64.0f);

    // Clear any existing enemies
    enemy_manager_instance.clear_all();

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

    enemy_manager_instance.spawn_dragon(dragon_x, dragon_y,
        room_left, room_right, room_top, room_bottom);
}

void level2_update()
{
    dt = (f32)AEFrameRateControllerGetFrameTime();

    get_zoom(dt);

    // Bounds for boss room
    simulate_camera(&cam, &player_instance.current_player->x, &player_instance.current_player->y,
        -6000.0f, 6000.0f, -5000.0f, 5000.0f);

    player_system_update(&player_instance);
    entity_system_update(&player_instance, &enemy_manager_instance);

    //check_button_collision(player_instance.char1.x, player_instance.char1.y,
    //    player_instance.char2.x, player_instance.char2.y,
    //    player_instance.config.character_size / 2.0f);
    puzzle_button_manager();

    check_and_open_doors(&player_instance);

    // Update enemy manager (for the dragon)
    enemy_manager_instance.update(dt, &player_instance, &cam);

    // Lose condition
    if (player_is_dead(&player_instance))
    {
        is_win = false;
        next = GS_LOSE;
    }

    // Win condition (level 2 only)
    if (enemy_manager_instance.get_active_enemy_count() == 0)
    {
        is_win = true;
        next = GS_WIN;
    }
}

void level2_draw()
{
    draw_boss_map(tile_size);

    player_system_render(&player_instance);
    minimap_draw(&player_instance, map, TileTextures);
    //Minimap_DrawWalls(map);
    // Render enemies (dragon)
    enemy_manager_instance.render();
    render_enemy_hp(&ui_manager, &enemy_manager_instance);
    render_player_hud(&ui_manager, &player_instance);
}

void level2_free()
{
    // Cleanup enemies
    free_ui_textures(&ui_manager);
    enemy_manager_instance.clear_all();
}

void level2_unload()
{
	for (int i = 0; i < NUM_TILE_TYPES; ++i)
		if (TileTextures[i])
			AEGfxTextureUnload(TileTextures[i]);
	free_mesh();
	unload_puzzle_textures();
	player_system_free(&player_instance);
}