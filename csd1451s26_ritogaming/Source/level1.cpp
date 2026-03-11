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
#include "game_state_list.hpp"
#include "puzzle.hpp"
#include "utils.hpp"
#include "ui_manager.hpp"
#include "minimap.hpp"
#include "audio_manager.hpp"
#include "end_screen.hpp"
#include "mainmenu.hpp"

#include "levels.hpp"

#define NUM_TILE_TYPES 10
#define TILE_ROWS 36
#define TILE_COLS 64


	f32 totalWorldW;
	f32 totalWorldH;
	f32 dt;
	
	namespace {
		s32 mouse_x, mouse_y;       
		bool cursor_active = false; 
	}

void level1_load()
{
	end_screen_load();
	if (!load_tile_textures("Assets/Tiles"))
		next = GS_QUIT; // If tile textures fail to load, quit the game
		
	load_puzzle_textures();
	load_ui_textures(&ui_manager);
}

//void level1_initialize()
//{
//	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
//	zoom = 1.1f;
//	// Determine tile size to fit both width and height
//	
//	tile_size = 64.f;
//	player_system_init(&player_instance);
//	player_instance.stats.character_size = tile_size - 8.0f;
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
void level1_init()
{
	AEInputShowCursor(false);
	f32 tile_size = 64.f;

	door_triggered = false;
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	zoom = 1.1f;
	ensure_level_paths_loaded(); //for both puzzles and map tile data

	printf("level_paths size: %d, current_level_index: %s\n",
		(int)level_paths.size(), selected_level_path.c_str());
	printf("level_puzzles size: %d, current_level_index: %s\n",
		(int)level_puzzle_paths.size(), selected_level_puzzle_path.c_str());

	if (selected_level_path.empty() || selected_level_puzzle_path.empty()) {
		selected_level_path = "Assets/Levels/default.txt";
		selected_level_puzzle_path = "Assets/Levels/puzzle.txt";
	}
	load_map_from_file(selected_level_path.c_str(), map);
	initialize_puzzle_blocks(selected_level_puzzle_path.c_str());

	generate_walls_from_map(map, 64.0f);

	for (int r = 0; r < TILE_ROWS; ++r)
		for (int c = 0; c < TILE_COLS; ++c)
			if (map[r][c] == DOOR || map[r][c] == OPENED_DOOR)
				printf("door at row:%d col:%d\n", r, c);


	player_system_init(&player_instance, TileTextures[4], TileTextures[3]);
	spawn_player_from_map(&player_instance, tile_size);
	player_instance.stats.character_size = tile_size - 8.0f;
	enemy_manager_instance.load_textures();
	// Camera follows the active character via the pointer
	cam = init_cam(&player_instance.current_player->x, &player_instance.current_player->y, 60.0f, 5.0f);
	init_mesh();

	totalWorldW = ((f32)TILE_COLS * tile_size) / ZOOM_LEVEL;
	totalWorldH = ((f32)TILE_ROWS * tile_size) / ZOOM_LEVEL;
	minimap_init(totalWorldW, totalWorldH, 300.0f);

	


	// // Clear any existing enemies and reinitialize with ghosts in top left room
	// g_enemy_manager.clear_all();
	// g_enemy_manager.set_level(true);
	// g_enemy_manager.initialize_ghost_slots();

	enemy_manager_instance.clear_all();
	enemy_manager_instance.initialize_tutorial_area();

	//lights
	light l1{ {-300.0f,-300.0f}, {0.5f,0.5f,0.0f,1.0f}, 11, 2.0f };
	light l2{ {-700.0f,-300.0f}, {white}, 11, 2.0f };
	light l3{ {-700.0f, 300.0f}, {white}, 11, 2.0f };
	lights.clear();
	lights.push_back(l1);
	lights.push_back(l2);
	lights.push_back(l3);

	AEInputShowCursor(true);
}

void level1_update()
{
	pause_update();
	if (is_paused) return;

	dt = (f32)AEFrameRateControllerGetFrameTime();
	AEInputGetCursorPosition(&mouse_x, &mouse_y);
	cursor_active = AEInputCheckCurr(AEVK_LBUTTON);
	get_zoom(dt);

	// Camera follows current_player - automatically updates when you switch characters!
	simulate_camera(&cam, &player_instance.current_player->x, &player_instance.current_player->y,
		-6000.0f, 6000.0f, -5000.0f, 5000.0f);

	player_system_update(&player_instance);
	entity_system_update(&player_instance, &enemy_manager_instance);

	puzzle_button_manager();

	//printf("%f %f\n", player_instance.char1_x, player_instance.char1_y);

	check_and_open_doors(&player_instance);

	enemy_manager_instance.update(dt, &player_instance, &cam);

	// Lose condition
	if (player_is_dead(&player_instance))
	{
		is_win = false;
		next = GS_LOSE;
	}

	

}

void level1_draw()
{
	AEGfxReset();
	draw_map(tile_size);	
	// Debug: draw all walls as semi-transparent red
	//debug_draw_all_walls();
	// Render player system

	render_puzzle_blocks();
	player_system_render(&player_instance);

	enemy_manager_instance.render();

	render_enemy_hp(&ui_manager, &enemy_manager_instance);
	render_player_hud(&ui_manager, &player_instance);
	render_character_ui(&ui_manager, &player_instance);
	minimap_draw(&player_instance, map, TileTextures);
	//Minimap_DrawWalls(map);

	render_cursor(&ui_manager, &player_instance, mouse_x, mouse_y, cursor_active);
	pause_draw();

}

void level1_free()
{
	
	//g_enemy_manager.clear_all();
	free_mesh();
	free_ui_textures(&ui_manager);
	enemy_manager_instance.clear_all();
}

void level1_unload()
{
	end_screen_unload();
	free_tile_textures();
	//free_mesh();
	minimap_unload();
	unload_puzzle_textures();
	player_system_free(&player_instance);
	enemy_manager_instance.unload_textures();
}