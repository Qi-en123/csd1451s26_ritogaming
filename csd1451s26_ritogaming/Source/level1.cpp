/*!************************************************************************
\file:						level1.cpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This file implements Level 1 functions to load the assets from a file, initialize the level,
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

static AEGfxTexture* skeleton_texture = nullptr;

namespace
{
	// --- CONSTANTS (Refactored Hardcoded Values) ---
	namespace Constants {
		// Asset Paths
		const char* PATH_TILE_SPRITES = "Assets/Sprites/Tiles";
		const char* PATH_DEAD_SKELETON = "Assets/Sprites/Tiles/dead_last_location.png";
		const char* PATH_MAIN_FONT = "Assets/Fonts/Kenney Mini Square.ttf";
		const char* PATH_PLAYER_CONFIG = "Assets/Config/player_config.json";
		const char* PATH_DEFAULT_MAP = "Assets/Levels/default.txt";
		const char* PATH_DEFAULT_PUZZLE = "Assets/Levels/puzzle.txt";
		const char* PATH_DEFAULT_ENEMY = "Assets/Levels/enemy.txt";

		// Timing & Speeds
		const f32 TRANSITION_SPEED = 2.0f;
		const f32 TEXT_FADE_IN_SPEED = 3.0f;
		const f32 TEXT_HOLD_DUR = 1.0f;
		const f32 TEXT_FADE_OUT_SPEED = 1.5f;
		const f32 BOSS_WARNING_DUR = 3.0f;
		const f32 BOSS_FLASH_SPEED = 6.0f;

		// UI & Camera
		const f32 DEFAULT_ZOOM = 1.1f;
		const f32 PLAYER_SIZE_OFFSET = 8.0f;
		const f32 BGM_VOLUME = 0.1f;
		const f32 OVERLAY_ALPHA_HALF = 0.5f;

		// Font Sizes
		const int FONT_SIZE_LARGE = 60;
		const int FONT_SIZE_SMALL = 28;

		// Text Positions/Scales
		const f32 LVL_TEXT_SCALE = 2.0f;
		const f32 LVL_TEXT_Y_OFFSET = 0.05f;
		const f32 DANGER_TEXT_SCALE = 1.5f;
		const f32 DANGER_Y_TOP = 0.15f;
		const f32 DANGER_Y_BOTTOM = -0.15f;

		// Boss Warning SFX
		const f32 BOSS_WARNING_SFX_VOLUME = 0.5f;
		const f32 BOSS_WARNING_SFX_PITCH = 1.0f;
	}

	s32 mouse_x, mouse_y;
	bool cursor_active = false;
	s8 tooltip_font = -1;

	// Transition
	enum TransitionState { NONE, FADE_OUT, FADE_IN };
	TransitionState transition_state = NONE;
	f32 transition_alpha = 0.f;
	int pending_next_state = -1;
	std::string current_level_name = "";

	// Level text fade in -> hold -> fade out
	enum TextState { TEXT_NONE, TEXT_FADE_IN, TEXT_HOLD, TEXT_FADE_OUT };
	TextState level_text_state = TEXT_NONE;
	f32 level_text_alpha = 0.f;
	f32 text_hold_timer = 0.f;

	// Boss warning
	bool show_boss_warning = false;
	f32 boss_warning_timer = 0.f;
	s8 warning_font = -1;
}

// ----------------------------------------------------------------------------
// Function: is_transitioning
// ---------------------------------------------------------------------------- 
bool is_transitioning()
{
	return transition_state != NONE;
}

// ----------------------------------------------------------------------------
// Function: trigger_level_transition
// ---------------------------------------------------------------------------- 
void trigger_level_transition(int target_state)
{
	stop_sfx();
	stop_bgm();
	transition_state = FADE_OUT;
	pending_next_state = target_state;
}

// ----------------------------------------------------------------------------
// Function: trigger_boss_warning
// ---------------------------------------------------------------------------- 
void trigger_boss_warning()
{
	stop_sfx();
	stop_bgm();
	show_boss_warning = true;
	boss_warning_timer = Constants::BOSS_WARNING_DUR;

	play_sfx(g_audio.sfx_warning, Constants::BOSS_WARNING_SFX_VOLUME, Constants::BOSS_WARNING_SFX_PITCH, -1);
}

// ----------------------------------------------------------------------------
// Function: level1_load
// ---------------------------------------------------------------------------- 
void level1_load()
{
	end_screen_load();
	if (!load_tile_textures(Constants::PATH_TILE_SPRITES))
		next = GS_QUIT;

	enemy_manager_instance.load_textures();
	Puzzle::load_puzzle_textures();
	load_ui_textures(&ui_manager);

	skeleton_texture = AEGfxTextureLoad(Constants::PATH_DEAD_SKELETON);
	warning_font = AEGfxCreateFont(Constants::PATH_MAIN_FONT, Constants::FONT_SIZE_LARGE);
	tooltip_font = AEGfxCreateFont(Constants::PATH_MAIN_FONT, Constants::FONT_SIZE_SMALL);

	if (tooltip_font == -1)
	{
		return;
	}
}

// ----------------------------------------------------------------------------
// Function: level1_init
// ---------------------------------------------------------------------------- 
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
	Camera::zoom = Constants::DEFAULT_ZOOM;
	ensure_level_paths_loaded();

	if (selected_level_path.empty() || selected_level_puzzle_path.empty())
	{
		selected_level_path = Constants::PATH_DEFAULT_MAP;
		selected_level_puzzle_path = Constants::PATH_DEFAULT_PUZZLE;
	}

	// Set display name logic
	if (selected_level_path == Constants::PATH_DEFAULT_MAP)
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
	generate_walls_from_map(map, tile_size);

	player_load_config(&player_instance, Constants::PATH_PLAYER_CONFIG);
	player_system_init(&player_instance, TileTextures[4], TileTextures[3], skeleton_texture);
	spawn_player_from_map(&player_instance, tile_size);

	player_instance.stats.character_size = tile_size - Constants::PLAYER_SIZE_OFFSET;
	Camera::cam = Camera::init_cam(&player_instance.current_player->x, &player_instance.current_player->y);
	init_mesh();

	totalWorldW = ((f32)TILE_COLS * tile_size) / minimap_zoom;
	totalWorldH = ((f32)TILE_ROWS * tile_size) / minimap_zoom;
	minimap_init(totalWorldW, totalWorldH);

	// Enemy Spawn logic
	std::string spawn_path;
	if (selected_level_path == Constants::PATH_DEFAULT_MAP)
	{
		spawn_path = Constants::PATH_DEFAULT_ENEMY;
	}
	else
	{
		size_t last_dot = selected_level_path.find_last_of('.');
		spawn_path = selected_level_path.substr(0, last_dot) + "_enemy_spawns.txt";
	}

	enemy_manager_instance.clear_all();
	enemy_manager_instance.load_spawn_positions(spawn_path.c_str());
	enemy_manager_instance.initialize_tutorial_area();

	g_bgm_current_track = &g_audio.bgm_maingame;
	play_bgm(g_audio.bgm_maingame, Constants::BGM_VOLUME);
}

// ----------------------------------------------------------------------------
// Function: level1_update
// ---------------------------------------------------------------------------- 
void level1_update()
{
	AEInputGetCursorPosition(&mouse_x, &mouse_y);
	dt = (f32)AEFrameRateControllerGetFrameTime();

	// Text State Machine Logic
	if (level_text_state == TEXT_FADE_IN)
	{
		level_text_alpha += dt * Constants::TEXT_FADE_IN_SPEED;
		if (level_text_alpha >= 1.f)
		{
			level_text_alpha = 1.f;
			level_text_state = TEXT_HOLD;
			text_hold_timer = Constants::TEXT_HOLD_DUR;
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
		level_text_alpha -= dt * Constants::TEXT_FADE_OUT_SPEED;
		if (level_text_alpha <= 0.f)
		{
			level_text_alpha = 0.f;
			level_text_state = TEXT_NONE;
		}
	}

	pause_update();
	if (is_paused) return;

	// Transitions
	if (transition_state == FADE_OUT)
	{
		transition_alpha += dt * Constants::TRANSITION_SPEED;
		if (transition_alpha >= 1.f)
		{
			transition_alpha = 1.f;
			next = pending_next_state;
		}
		return;
	}

	if (transition_state == FADE_IN)
	{
		transition_alpha -= dt * Constants::TRANSITION_SPEED;
		if (transition_alpha <= 0.f)
		{
			transition_alpha = 0.f;
			transition_state = NONE;
		}
	}

	if (show_boss_warning)
	{
		boss_warning_timer -= dt;
		if (boss_warning_timer <= 0.f)
		{
			show_boss_warning = false;
			stop_sfx(); // stop the warning sound effect if it's still playing
			trigger_level_transition(GS_BOSS_ROOM);
		}
		return;
	}

	cursor_active = AEInputCheckCurr(AEVK_LBUTTON);
	get_zoom(dt);

	Camera::simulate_camera(&Camera::cam, &player_instance.current_player->x, &player_instance.current_player->y);
	player_system_update(&player_instance);
	entity_system_update(&player_instance, &enemy_manager_instance);
	Puzzle::puzzle_button_manager();
	Puzzle::puzzle_audio_update();
	check_and_open_doors(&player_instance);
	enemy_manager_instance.update(dt, &player_instance, &Camera::cam);

	if (player_is_dead(&player_instance))
	{
		is_win = false;
		restart_level_state = GS_LEVEL1;
		next = GS_LOSE;
	}

	// Debug Toggles
	if (AEInputCheckTriggered(AEVK_F1)) {
		DEBUG::DEBUGGING ^= 1;
		int val = DEBUG::DEBUGGING;
		DEBUG::DRAW_WALLS = DEBUG::DRAW_PLAYER_TILE = DEBUG::DRAW_PUZZLE_GRAB_RANGE =
			DEBUG::DRAW_PUZZLE_BLOCKS = DEBUG::DRAW_BUTTON_SPIKE_LINKS = DEBUG::DRAW_ENEMIES = val;
	}

	if (DEBUG::DEBUGGING) {
		if (AEInputCheckTriggered(AEVK_F2)) DEBUG::DRAW_WALLS ^= 1;
		if (AEInputCheckTriggered(AEVK_F3)) DEBUG::DRAW_PLAYER_TILE ^= 1;
		if (AEInputCheckTriggered(AEVK_F4)) DEBUG::DRAW_PUZZLE_GRAB_RANGE ^= 1;
		if (AEInputCheckTriggered(AEVK_F5)) DEBUG::DRAW_PUZZLE_BLOCKS ^= 1;
		if (AEInputCheckTriggered(AEVK_F6)) DEBUG::DRAW_BUTTON_SPIKE_LINKS ^= 1;
		if (AEInputCheckTriggered(AEVK_F7)) DEBUG::DRAW_ENEMIES ^= 1;
	}
}

// ----------------------------------------------------------------------------
// Function: level1_draw
// ---------------------------------------------------------------------------- 
void level1_draw()
{
	AEGfxReset();

	draw_map(tile_size);
	if (DEBUG::DEBUGGING) {
		if (DEBUG::DRAW_WALLS)              DEBUG::draw_all_walls();
		if (DEBUG::DRAW_PLAYER_TILE)        DEBUG::draw_player_tile();
		if (DEBUG::DRAW_PUZZLE_GRAB_RANGE)  DEBUG::draw_puzzle_grab_range();
		if (DEBUG::DRAW_PUZZLE_BLOCKS)      DEBUG::draw_puzzle_blocks();
		if (DEBUG::DRAW_BUTTON_SPIKE_LINKS) DEBUG::draw_button_spike_links();
		if (DEBUG::DRAW_ENEMIES)            DEBUG::draw_enemies();
	}

	enemy_manager_instance.render();
	render_enemy_hp(&ui_manager, &enemy_manager_instance);
	Puzzle::render_puzzle_blocks();
	player_system_render(&player_instance);
	minimap_draw(&player_instance, map, TileTextures);
	render_player_hud(&ui_manager, &player_instance);
	render_character_ui(&ui_manager, &player_instance);
	render_tooltip(tooltip_font);
	pause_draw();

	if (transition_state != NONE && transition_alpha > 0.f)
		draw_black_overlay(transition_alpha);

	if (level_text_alpha > 0.f && warning_font != -1)
	{
		f32 tw, th;
		const char* lvl = current_level_name.c_str();
		AEGfxGetPrintSize(warning_font, lvl, Constants::LVL_TEXT_SCALE, &tw, &th);
		AEGfxPrint(warning_font, lvl, -tw * 0.5f, Constants::LVL_TEXT_Y_OFFSET,
			Constants::LVL_TEXT_SCALE, 1.f, 1.f, 1.f, level_text_alpha);
	}

	if (show_boss_warning)
	{
		draw_black_overlay(Constants::OVERLAY_ALPHA_HALF);
		f32 flash = (sinf(boss_warning_timer * Constants::BOSS_FLASH_SPEED) + 1.f) * 0.5f;
		f32 tw, th;
		AEGfxGetPrintSize(warning_font, "! DANGER !", Constants::DANGER_TEXT_SCALE, &tw, &th);
		AEGfxPrint(warning_font, "! DANGER !", -tw * 0.5f, Constants::DANGER_Y_TOP,
			Constants::DANGER_TEXT_SCALE, 1.f, 0.f, 0.f, flash);

		AEGfxGetPrintSize(warning_font, "! ABOUT TO ENTER BOSS ROOM !", Constants::DANGER_TEXT_SCALE, &tw, &th);
		AEGfxPrint(warning_font, "! ABOUT TO ENTER BOSS ROOM !", -tw * 0.5f, Constants::DANGER_Y_BOTTOM,
			Constants::DANGER_TEXT_SCALE, 1.f, 0.f, 0.f, flash);
	}

	render_cursor(&ui_manager, &player_instance, mouse_x, mouse_y, cursor_active);
}

// ----------------------------------------------------------------------------
// Function: level1_free
// ---------------------------------------------------------------------------- 
void level1_free()
{
	player_system_free(&player_instance);
	enemy_manager_instance.clear_all();
	free_mesh();
	free_ui_textures(&ui_manager);
	stop_bgm();
	stop_sfx();
	g_bgm_current_track = nullptr;
}

// ----------------------------------------------------------------------------
// Function: level1_unload
// ---------------------------------------------------------------------------- 
void level1_unload()
{
	end_screen_unload();
	free_tile_textures();
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