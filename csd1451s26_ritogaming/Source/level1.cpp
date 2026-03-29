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

//f32 totalWorldW;
//f32 totalWorldH;

static AEGfxTexture *skeleton_texture = nullptr;

namespace
{
	s32 mouse_x, mouse_y;
	bool cursor_active = false;

	// Transition
	enum TransitionState { NONE, FADE_OUT, FADE_IN };
	TransitionState transition_state = NONE;
	f32 transition_alpha = 0.f;
	const f32 TRANSITION_SPEED = 2.0f;
	int pending_next_state = -1;
}



void level1_load()
{
	end_screen_load();
	if (!load_tile_textures("Assets/Tiles"))
		next = GS_QUIT; // If tile textures fail to load, quit the game
	enemy_manager_instance.load_textures();
	load_puzzle_textures();
	load_ui_textures(&ui_manager);
	skeleton_texture = AEGfxTextureLoad("Assets/Tiles/dead_last_location.png");
}

void level1_init()
{
	AEInputShowCursor(false);
	transition_state = FADE_IN;
	transition_alpha = 1.f;
	pending_next_state = -1;
	door_triggered = false;
	is_win = false;
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	zoom = 1.1f;
	ensure_level_paths_loaded(); // for both puzzles and map tile data



	if (selected_level_path.empty() || selected_level_puzzle_path.empty())
	{
		selected_level_path = "Assets/Levels/default.txt";
		selected_level_puzzle_path = "Assets/Levels/puzzle.txt";
	}
	load_map_from_file(selected_level_path.c_str(), map);
	initialize_puzzle_blocks(selected_level_puzzle_path.c_str());

	generate_walls_from_map(map, 64.0f);



	player_system_init(&player_instance, TileTextures[4], TileTextures[3], skeleton_texture);
	spawn_player_from_map(&player_instance, tile_size);
	player_instance.stats.character_size = tile_size - 8.0f;
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

	// for enemies
	std::string spawn_path;
	if (selected_level_path == "Assets/Levels/default.txt")
	{
		// Ensure this path matches your folder exactly (check capitalization!)
		spawn_path = "Assets/Enemy_spawn_location/Enemy_spawning_coordinates.txt";
	}
	else
	{
		// Use the PUZZLE path as the base to match "l1_puzzles_enemy_spawns.txt"
		size_t last_dot = selected_level_path.find_last_of('.');
		spawn_path = selected_level_path.substr(0, last_dot) + "_enemy_spawns.txt";
	}

	enemy_manager_instance.load_spawn_positions(spawn_path.c_str());
	enemy_manager_instance.initialize_tutorial_area();

	// lights
	light l1{{-300.0f, -300.0f}, {0.5f, 0.5f, 0.0f, 1.0f}, 11, 2.0f};
	light l2{{-700.0f, -300.0f}, {white}, 11, 2.0f};
	light l3{{-700.0f, 300.0f}, {white}, 11, 2.0f};
	lights.clear();
	lights.push_back(l1);
	lights.push_back(l2);
	lights.push_back(l3);

	AEInputShowCursor(false);
}

void level1_update()
{
	dt = (f32)AEFrameRateControllerGetFrameTime();
	AEInputGetCursorPosition(&mouse_x, &mouse_y);
	pause_update();
	if (is_paused)
		return;

	if (transition_state == FADE_OUT)
	{
		transition_alpha += dt * TRANSITION_SPEED;
		if (transition_alpha >= 1.f)
		{
			transition_alpha = 1.f;
			next = pending_next_state;
		}
		return;
	}
	if (transition_state == FADE_IN)
	{
		transition_alpha -= dt * TRANSITION_SPEED;
		if (transition_alpha <= 0.f)
		{
			transition_alpha = 0.f;
			transition_state = NONE;
		}
	}

	

	cursor_active = AEInputCheckCurr(AEVK_LBUTTON);
	get_zoom(dt);

	// Camera follows current_player - automatically updates when you switch characters!
	simulate_camera(&cam, &player_instance.current_player->x, &player_instance.current_player->y,
					-6000.0f, 6000.0f, -5000.0f, 5000.0f);

	player_system_update(&player_instance);
	entity_system_update(&player_instance, &enemy_manager_instance);

	puzzle_button_manager();
	puzzle_audio_update();

	// printf("%f %f\n", player_instance.char1_x, player_instance.char1_y);

	check_and_open_doors(&player_instance);

	enemy_manager_instance.update(dt, &player_instance, &cam);

	// Lose condition
	if (player_is_dead(&player_instance))
	{
		is_win = false;
		restart_level_state = GS_LEVEL1;
		next = GS_LOSE;
	}
}

void level1_draw()
{
	AEGfxReset();
	draw_map(tile_size);
	// Debug: draw all walls as semi-transparent red
	// debug_draw_all_walls();
	// Render player system

	

	enemy_manager_instance.render();

	render_player_hud(&ui_manager, &player_instance);
	render_enemy_hp(&ui_manager, &enemy_manager_instance);
	render_character_ui(&ui_manager, &player_instance);
	
	render_puzzle_blocks();
	player_system_render(&player_instance);
	minimap_draw(&player_instance, map, TileTextures);

	pause_draw();

	if (transition_state != NONE && transition_alpha > 0.f)
		draw_black_overlay(transition_alpha);

	render_cursor(&ui_manager, &player_instance, mouse_x, mouse_y, cursor_active);

}

void level1_free()
{
	player_system_free(&player_instance);
	enemy_manager_instance.clear_all();

	free_mesh();
	free_ui_textures(&ui_manager);
}

void level1_unload()
{
	end_screen_unload();
	free_tile_textures();
	// free_mesh();
	minimap_unload();
	unload_puzzle_textures();

	player_system_free(&player_instance);

	if (skeleton_texture)
	{
		AEGfxTextureUnload(skeleton_texture);
		skeleton_texture = nullptr;
	}

	
}

bool is_transitioning()
{
	return transition_state != NONE;
}
void trigger_level_transition(int target_state)
{
	transition_state = FADE_OUT;
	pending_next_state = target_state;
}