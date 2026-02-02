/*!************************************************************************
\file:						level1.cpp
\author:					Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This file implements Level 1 functions to load the assests from a file, initialize the level,
update game logic, render, free resources,

UPDATED: Now works with refactored player structure using current_player pointer
**************************************************************************/
#include "AEEngine.h"
#include <cstdio>
#include "map.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "enemy_manager.hpp"
#include "collision.hpp"
#include "game_state_manager.hpp"
#include "puzzle.hpp"
#include "utils.hpp"
#include "Minimap.hpp"

#define NUM_TILE_TYPES 9
#define TILE_ROWS 36
#define TILE_COLS 64

// Player player_instance;
// camera cam{};
// f32 tile_size;
// f32 zoom;
// f32 zoomlevel;
f32 totalWorldW;
f32 totalWorldH;
// AEGfxTexture* TileTextures[NUM_TILE_TYPES];
// AEGfxVertexList* TileMesh = nullptr;
f32 dt;

void level1_load()
{
	// Tell the Alpha Engine to set the background to black.
	// AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < NUM_TILE_TYPES; i++)
		TileTextures[i] = nullptr;
	// TileMesh = nullptr;

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
}

void level1_initialize()
{
	// zoom = 1.0f;
	//  Determine tile size to fit both width and height
	tile_size = 64.f;
	player_system_init(&player_instance);
	player_instance.config.character_size = tile_size - 4.0f;

	// UPDATED: Use current_player pointer for camera initialization
	// Camera now follows the active character via the pointer
	cam = init_cam(&player_instance.current_player->x, &player_instance.current_player->y, 60.0f, 5.0f);
	init_mesh();

	// zoomlevel = 2.0f;
	totalWorldW = ((f32)TILE_COLS * tile_size) / ZOOM_LEVEL;
	totalWorldH = ((f32)TILE_ROWS * tile_size) / ZOOM_LEVEL;
	Minimap_Init(totalWorldW, totalWorldH, 200.0f); // 200px wide map
	// printf("Walls created: %zu\n", g_walls.size());

	// js to test camera (UPDATE CURR_PLAYER TO CHANGE CAMERA VIEW)
	//  UPDATED: With the new structure, you can easily switch camera target:
	//  cam = init_cam(&player_instance.char1.x, &player_instance.char1.y, 60.0f, 5.0f);  // Follow char1
	//  cam = init_cam(&player_instance.char2.x, &player_instance.char2.y, 60.0f, 5.0f);  // Follow char2
	//  cam = init_cam(&player_instance.current_player->x, &player_instance.current_player->y, 60.0f, 5.0f);  // Follow active

	//////////////////////////////////////////////////

	initialize_dungeon_map();
	generate_walls_from_map(map, 64.0f); // step 2: make walls from map

	intialize_puzzle_blocks();

	// ShiftMapRight(2);

	// Clear any existing enemies
	g_enemy_manager.clear_all();
}

void level1_update()
{
	dt = (f32)AEFrameRateControllerGetFrameTime();
	get_zoom(dt);

	// UPDATED: Camera follows current_player - automatically updates when you switch characters!
	simulate_camera(&cam, &player_instance.current_player->x, &player_instance.current_player->y,
					-6000.0f, 6000.0f, -5000.0f, 5000.0f);

	player_system_update(&player_instance);

	check_button_collision(player_instance.char1_x, player_instance.char1_y,
						   player_instance.char2_x, player_instance.char2_y,
						   player_instance.config.character_size / 2.0f);
	puzzle_button_manager();

	// check_puzzle_collision(&player_instance);
	check_and_open_doors();
	circle_to_rectangle(200, 100, 50, player_instance.x, player_instance.y, 25.f, 25.f);
	player_system_render(&player_instance);

	g_enemy_manager.update(dt, &player_instance, &cam);
}

void level1_draw()
{
	draw_map(tile_size);

	// Debug: draw all walls as semi-transparent red
	// debug_draw_all_walls();
	// Render player system

	render_puzzle_blocks();
	player_system_render(&player_instance);
	Minimap_Draw(&player_instance, map, TileTextures);

	Minimap_DrawWalls(map);
	g_enemy_manager.render();
}

void level1_free()
{
	// Cleanup
	player_system_free(&player_instance);
	g_enemy_manager.clear_all();
}

void level1_unload()
{
	for (int i = 0; i < NUM_TILE_TYPES; ++i)
		if (TileTextures[i])
			AEGfxTextureUnload(TileTextures[i]);
	free_mesh();
	unload_puzzle_textures();
	// player_system_free(&player_instance);
	// if (TileMesh) AEGfxMeshFree(TileMesh);
}