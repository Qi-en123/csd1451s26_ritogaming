/*!************************************************************************
\file:						level1.cpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This file implements Level 1 functions to load the assests from a file, initialize the level,
update game logic, render, free resources
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


#define NUM_TILE_TYPES 10
#define TILE_ROWS 36
#define TILE_COLS 64

f32 totalWorldW;
f32 totalWorldH;
f32 dt;

void level1_load()
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
}

//void level1_initialize()
//{
//	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
//	zoom = 1.1f;
//	// Determine tile size to fit both width and height
//	
//	tile_size = 64.f;
//	player_system_init(&player_instance);
//	player_instance.config.character_size = tile_size - 8.0f;
//	// Camera follows the active character via the pointer
//	cam = init_cam(&player_instance.current_player->x, &player_instance.current_player->y, 60.0f, 5.0f);
//	init_mesh();
//
//	totalWorldW = ((f32)TILE_COLS * tile_size) / ZOOM_LEVEL;
//	totalWorldH = ((f32)TILE_ROWS * tile_size) / ZOOM_LEVEL;
//	Minimap_Init(totalWorldW, totalWorldH, 200.0f); // 200px wide map
//
//	initialize_dungeon_map();
//	generate_walls_from_map(map, 64.0f);
//
//	intialize_puzzle_blocks();
//
//	// Clear any existing enemies
//	g_enemy_manager.clear_all();
//}
void level1_initialize()
{
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	zoom = 1.1f;

	tile_size = 64.f;
	player_system_init(&player_instance);
	player_instance.config.character_size = tile_size - 8.0f;

	// Camera follows the active character via the pointer
	cam = init_cam(&player_instance.current_player->x, &player_instance.current_player->y, 60.0f, 5.0f);
	init_mesh();

	totalWorldW = ((f32)TILE_COLS * tile_size) / ZOOM_LEVEL;
	totalWorldH = ((f32)TILE_ROWS * tile_size) / ZOOM_LEVEL;
	Minimap_Init(totalWorldW, totalWorldH, 200.0f);

	initialize_dungeon_map();
	generate_walls_from_map(map, 64.0f);

	intialize_puzzle_blocks();

	// Clear any existing enemies and reinitialize with ghosts in top left room
	g_enemy_manager.clear_all();
	g_enemy_manager.set_level(true);  
	g_enemy_manager.initialize_ghost_slots();
}

void level1_update()
{
	dt = (f32)AEFrameRateControllerGetFrameTime();
	get_zoom(dt);

	// Camera follows current_player - automatically updates when you switch characters!
	simulate_camera(&cam, &player_instance.current_player->x, &player_instance.current_player->y,
		-6000.0f, 6000.0f, -5000.0f, 5000.0f);

	player_system_update(&player_instance);

	check_button_collision(player_instance.char1.x, player_instance.char1.y,
		player_instance.char2.x, player_instance.char2.y,
		player_instance.config.character_size / 2.0f);
	puzzle_button_manager();
	

	////EHERHERHERHEHHRHERHERHEHHREHREHRHEHHEHR
	//check_puzzle_collision(&player_instance);
	//check_and_open_doors();
	//circle_to_rectangle(200, 100, 50, player_instance.x, player_instance.y, 25.f, 25.f);

	//printf("%f %f\n", player_instance.char1_x, player_instance.char1_y);

	check_and_open_doors(&player_instance);

	g_enemy_manager.update(dt, &player_instance, &cam);
}

void level1_draw()
{
	draw_map(tile_size);	
	// Debug: draw all walls as semi-transparent red
	//debug_draw_all_walls();
	// Render player system

	render_puzzle_blocks();
	player_system_render(&player_instance);
	//Minimap_Draw(&player_instance, map, TileTextures);
	//Minimap_DrawWalls(map);

	g_enemy_manager.render();
}

void level1_free()
{
	player_system_free(&player_instance);
	g_enemy_manager.clear_all();
	free_mesh();
}

void level1_unload()
{
	for (int i = 0; i < NUM_TILE_TYPES; ++i)
		if (TileTextures[i])
			AEGfxTextureUnload(TileTextures[i]);
	unload_puzzle_textures();
}