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
#include "debug.hpp"

#include "levels.hpp"

static AEGfxTexture *skeleton_texture = nullptr;

namespace
{
	s32 mouse_x, mouse_y;
	bool cursor_active = false;
	s8 tooltip_font = -1;

	// Transition
	enum TransitionState { NONE, FADE_OUT, FADE_IN };
	TransitionState transition_state = NONE;
	f32 transition_alpha = 0.f;
	const f32 TRANSITION_SPEED = 2.0f;
	int pending_next_state = -1;
	std::string current_level_name = "";

	// Level text fade in -> hold -> fade out
	enum TextState { TEXT_NONE, TEXT_FADE_IN, TEXT_HOLD, TEXT_FADE_OUT };
	TextState level_text_state = TEXT_NONE;
	f32 level_text_alpha = 0.f;
	const f32 TEXT_FADE_IN_SPEED = 3.0f;  // faster fade in
	const f32 TEXT_HOLD_DUR = 1.0f;  // hold for 1 second
	const f32 TEXT_FADE_OUT_SPEED = 1.5f;  // faster fade out
	f32 text_hold_timer = 0.f;



	// Boss warning
	bool show_boss_warning = false;
	f32 boss_warning_timer = 0.f;
	const f32 BOSS_WARNING_DUR = 3.f;
	s8 warning_font = -1;

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

void trigger_boss_warning()
{
	show_boss_warning = true;
	boss_warning_timer = BOSS_WARNING_DUR;
}

void level1_load()
{
	end_screen_load();
	if (!load_tile_textures("Assets/Sprites/Tiles"))
		next = GS_QUIT; // If tile textures fail to load, quit the game
	enemy_manager_instance.load_textures();
	Puzzle::load_puzzle_textures();
	load_ui_textures(&ui_manager);
	skeleton_texture = AEGfxTextureLoad("Assets/Sprites/Tiles/dead_last_location.png");
	warning_font = AEGfxCreateFont("Assets/Fonts/Kenney Mini Square.ttf", 60);
	tooltip_font = AEGfxCreateFont("Assets/Fonts/Kenney Mini Square.ttf", 28);
	printf("Font value: %d\n", tooltip_font);
	if (tooltip_font == -1)
	{
		return;
	}
}

void level1_init()
{
	AEInputShowCursor(false);
	transition_state = FADE_IN;
	transition_alpha = 1.f;
	level_text_alpha = 0.f;
	level_text_state = TEXT_FADE_IN;
	text_hold_timer = 0.f;
	pending_next_state = -1;
	door_triggered = false;
	is_win = false;
	show_boss_warning = false;
	boss_warning_timer = 0.f;

	
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	zoom = 1.1f;
	ensure_level_paths_loaded(); // for both puzzles and map tile data

	if (selected_level_path.empty() || selected_level_puzzle_path.empty())
	{
		selected_level_path = "Assets/Levels/default.txt";
		selected_level_puzzle_path = "Assets/Levels/puzzle.txt";
	}

	// Set display name
	if (selected_level_path == "Assets/Levels/default.txt")
		current_level_name = "Level 1";
	else
	{
		for (int i = 0; i < (int)level_paths.size(); ++i)
		{
			if (level_paths[i] == selected_level_path)
			{
				current_level_name = "Level " + std::to_string(i + 1);
				break;
			}
		}
	}
	load_map_from_file(selected_level_path.c_str(), map);
	Puzzle::load_puzzle_data(selected_level_puzzle_path.c_str());

	generate_walls_from_map(map, 64.0f);

	for (int r = 0; r < TILE_ROWS; ++r)
		for (int c = 0; c < TILE_COLS; ++c)
			if (map[r][c] == DOOR || map[r][c] == OPENED_DOOR)
				printf("door at row:%d col:%d\n", r, c);

	player_system_init(&player_instance, TileTextures[4], TileTextures[3], skeleton_texture);
	spawn_player_from_map(&player_instance, tile_size);

	player_instance.stats.character_size = tile_size - 8.0f;
	// Camera follows the active character via the pointer
	cam = init_cam(&player_instance.current_player->x, &player_instance.current_player->y, 60.0f, 5.0f);
	init_mesh();

	totalWorldW = ((f32)TILE_COLS * tile_size) / minimap_zoom;
	totalWorldH = ((f32)TILE_ROWS * tile_size) / minimap_zoom;
	minimap_init(totalWorldW, totalWorldH, 300.0f);
	
	// for enemies
	std::string spawn_path;
	if (selected_level_path == "Assets/Levels/default.txt")
	{
		// Ensure this path matches your folder exactly (check capitalization!)
		spawn_path = "Assets/Levels/.txt";
	}
	else
	{
		// Use the PUZZLE path as the base to match "l1_puzzles_enemy_spawns.txt"
		size_t last_dot = selected_level_path.find_last_of('.');
		spawn_path = selected_level_path.substr(0, last_dot) + "_enemy_spawns.txt";
	}

	// Add this temporary printf to your console so you can see if the path is correct
	printf("Loading Spawn File: %s\n", spawn_path.c_str());

	enemy_manager_instance.clear_all();
	enemy_manager_instance.load_spawn_positions(spawn_path.c_str());
	enemy_manager_instance.initialize_tutorial_area();
}

void level1_update()
{
	AEInputGetCursorPosition(&mouse_x, &mouse_y);


	dt = (f32)AEFrameRateControllerGetFrameTime();


	// Text fade in -> hold -> fade out (independent of black overlay)
	if (level_text_state == TEXT_FADE_IN)
	{
		level_text_alpha += dt * TEXT_FADE_IN_SPEED;
		if (level_text_alpha >= 1.f)
		{
			level_text_alpha = 1.f;
			level_text_state = TEXT_HOLD;
			text_hold_timer = TEXT_HOLD_DUR;
		}
	}
	else if (level_text_state == TEXT_HOLD)
	{
		text_hold_timer -= dt;
		if (text_hold_timer <= 0.f)
			level_text_state = TEXT_FADE_OUT;
	}
	else if (level_text_state == TEXT_FADE_OUT)
	{
		level_text_alpha -= dt * TEXT_FADE_OUT_SPEED;
		if (level_text_alpha <= 0.f)
		{
			level_text_alpha = 0.f;
			level_text_state = TEXT_NONE;
		}
	}
	pause_update();
	if (is_paused)
		return;

	// Handle fade out
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

	// Handle fade in
	if (transition_state == FADE_IN)
	{
		transition_alpha -= dt * TRANSITION_SPEED;
		if (transition_alpha <= 0.f)
		{
			transition_alpha = 0.f;
			transition_state = NONE;
		}
	}



	// Handle boss warning
	if (show_boss_warning)
	{
		boss_warning_timer -= dt;
		if (boss_warning_timer <= 0.f)
		{
			show_boss_warning = false;
			trigger_level_transition(GS_BOSS_ROOM);
		}
		return;
	}
	cursor_active = AEInputCheckCurr(AEVK_LBUTTON);
	get_zoom(dt);

	// Camera follows current_player - automatically updates when you switch characters!
	simulate_camera(&cam, &player_instance.current_player->x, &player_instance.current_player->y,
					-6000.0f, 6000.0f, -5000.0f, 5000.0f);

	player_system_update(&player_instance);
	entity_system_update(&player_instance, &enemy_manager_instance);

	Puzzle::puzzle_button_manager();
	Puzzle::puzzle_audio_update();

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

	//Renders map tiles
	draw_map(tile_size);

	if (DEBUG::DEBUGGING) {
		// Debug: draw all walls as semi-transparent red
		DEBUG::draw_all_walls();
		DEBUG::draw_player_tile();
		DEBUG::draw_puzzle_grab_range();
		DEBUG::draw_puzzle_blocks();
		DEBUG::draw_button_spike_links();
		DEBUG::draw_enemies();
	}

	//Renders enemies
	enemy_manager_instance.render();
	render_enemy_hp(&ui_manager, &enemy_manager_instance);

	//Renders the puzzle blocks
	Puzzle::render_puzzle_blocks();

	//Renders the actual player/characters
	player_system_render(&player_instance);

	//Renders minimap
	minimap_draw(&player_instance, map, TileTextures);

	//Renders the player health & skills UI
	render_player_hud(&ui_manager, &player_instance);
	render_character_ui(&ui_manager, &player_instance);
	render_tooltip(tooltip_font);

	//Renders the pause ui when game is paused
	pause_draw();
	if (!is_paused)

	// Draw black fade overlay
	if (transition_state != NONE && transition_alpha > 0.f)
		draw_black_overlay(transition_alpha);

	// Draw level name independently with its own fade in/hold/fade out
	if (level_text_alpha > 0.f && warning_font != -1)
	{
		f32 tw, th;
		const char* lvl = current_level_name.c_str();
		AEGfxGetPrintSize(warning_font, lvl, 2.0f, &tw, &th);
		AEGfxPrint(warning_font, lvl,
			-tw * 0.5f, 0.05f,
			2.0f, 1.f, 1.f, 1.f, level_text_alpha);
	}

	// Draw boss warning
	if (show_boss_warning)
	{
		draw_black_overlay(0.5f);

		f32 flash = (sinf(boss_warning_timer * 10.f) + 1.f) * 0.5f;
		f32 tw, th;
		AEGfxGetPrintSize(warning_font, "! DANGER !", 1.5f, &tw, &th);
		AEGfxPrint(warning_font, "! DANGER !",
			-tw * 0.5f, 0.15f,
			1.5f, 1.f, 0.f, 0.f, flash);

		f32 tw2, th2;
		AEGfxGetPrintSize(warning_font, "! ABOUT TO ENTER BOSS ROOM !", 1.5f, &tw2, &th2);
		AEGfxPrint(warning_font, "! ABOUT TO ENTER BOSS ROOM !",
			-tw2 * 0.5f, -0.15f,
			1.5f, 1.f, 0.f, 0.f, flash);
	}

	

	//Renders the custom cursor
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
	Puzzle::unload_puzzle_textures();

	player_system_free(&player_instance);

	if (skeleton_texture)
	{
		AEGfxTextureUnload(skeleton_texture);
		skeleton_texture = nullptr;
	}
	AEGfxDestroyFont(tooltip_font);	
}