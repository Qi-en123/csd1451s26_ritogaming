/*!************************************************************************
\file:						boss.cpp
\author:					Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This file implements Level 1 functions to load the assests from a file, initialize the level,
update game logic, render, free resources,
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
#include "Minimap.hpp"




Player player_instance2;
camera cam2{};
extern f32 tile_size;
f32 zoom2;
 f32 zoomlevel2;
 f32 totalWorldW2;
 f32 totalWorldH2;
extern AEGfxTexture* TileTextures[NUM_TILE_TYPES];
//AEGfxVertexList* TileMesh = nullptr;
extern f32 dt;

void boss_load()
{
	// Tell the Alpha Engine to set the background to black.
	//AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);


	for (int i = 0; i < NUM_TILE_TYPES; i++)
		TileTextures[i] = nullptr;
	//TileMesh = nullptr;

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

void boss_initialize()
{
	zoom2= 1.0f;
	// Determine tile size to fit both width and height
	tile_size = 64.f;
	player_system_init(&player_instance2);
	player_instance2.config.character_size = tile_size - 4.0f;
	cam2= init_cam(&player_instance2.x, &player_instance2.y, 60.0f, 5.0f);
	init_mesh();

	zoomlevel2 = 2.0f;
	totalWorldW2 = ((f32)TILE_COLS * tile_size) / zoomlevel2;
	totalWorldH2 = ((f32)TILE_ROWS * tile_size) / zoomlevel2;
	Minimap_Init(totalWorldW2, totalWorldH2, 200.0f); // 200px wide map
	//printf("Walls created: %zu\n", g_walls.size());

	//js to test camera (UPDATE CURR_PLAYER TO CHANGE CAMERA VIEW)
	//f32* curr_player_x{ &player_instance.char1_x }, * curr_player_y{ &player_instance.char1_y };

	//cam2= init_cam(curr_player_x, curr_player_y, 60.0f, 0.0f);

	//////////////////////////////////////////////////

	initialize_boss_map();
	generate_walls_from_map(map, 64.0f); // step 2: make walls from map

	intialize_puzzle_blocks();

	//ShiftMapRight(2);
}

void boss_update()
{
	dt = (f32)AEFrameRateControllerGetFrameTime();

	///////////////////////////
	get_zoom(dt);
	//Bounds anyhow set first
	simulate_camera(&cam, &player_instance2.x, &player_instance2.y, -6000.0f, 6000.0f, -5000.0f, 5000.0f);

	// Update player system
	player_system_update(&player_instance2);

	check_button_collision(player_instance2.char1_x, player_instance2.char1_y,
		player_instance2.char2_x, player_instance2.char2_y,
		player_instance2.config.character_size / 2.0f);
	puzzle_button_manager();

	//check_puzzle_collision(&player_instance);
	check_and_open_doors();
	circle_to_rectangle(200, 100, 50, player_instance2.x, player_instance2.y, 25.f, 25.f);
}

void boss_draw()
{
	draw_boss_map(tile_size);

	// Debug: draw all walls as semi-transparent red
	//debug_draw_all_walls();
	// Render player system

	render_puzzle_blocks();
	player_system_render(&player_instance2);
	//Minimap_Draw(&player_instance, map, TileTextures);

	//Minimap_DrawWalls(map);
}

void boss_free()
{
	// Cleanup

}

void boss_unload()
{
	for (int i = 0; i < NUM_TILE_TYPES; ++i)
		if (TileTextures[i]) AEGfxTextureUnload(TileTextures[i]);
	free_mesh();
	unload_puzzle_textures();
	player_system_free(&player_instance2);
	//if (TileMesh) AEGfxMeshFree(TileMesh);
}