/*!************************************************************************
\file:			guide.cpp
\author:		Jerod lim
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			This source file implements functions for the Guide/Tutorial
				level. The level data is retrieved from text files. Guide
				texts along the levels are set in this file.
**************************************************************************/

#include "guide.hpp"
#include "AEEngine.h"
#include "map.hpp"
#include "renderer.hpp"
#include "levels.hpp"
#include "utils.hpp"
#include "collision.hpp"
#include "end_screen.hpp"
#include "enemy.hpp"
#include "camera.hpp"
#include "Minimap.hpp"
#include "enemy_manager.hpp"
#include "game_state_list.hpp"
#include "game_state_manager.hpp"
#include "ui_manager.hpp"
#include "debug.hpp"

namespace
{
	// ------------------------------------------------------------------------
	// static variables
	// ------------------------------------------------------------------------ 
	s32 mouse_x, mouse_y;
	bool cursor_active = false;
	static s8 guide_text_font = -1;
	static s8 tooltip_font = -1;
	AEGfxTexture* skeleton_texture = nullptr; //shld put this in player script or smth
	bool cleared_guide;

	// ------------------------------------------------------------------------
	// Renders text anchored to a world-space tile position (row/col),
	// projected into normalized device coordinates [-1, 1] for the text renderer.
	// ------------------------------------------------------------------------
	void render_guide_text(s32 row, s32 col, const char* text, color color) {
		f32 world_x = col_to_world(col);
		f32 world_y = row_to_world(row);

		f32 text_x = (world_x - Camera::cam.pos.x) * Camera::zoom / (AEGfxGetWindowWidth() * 0.5f);
		f32 text_y = (world_y - Camera::cam.pos.y) * Camera::zoom / (AEGfxGetWindowHeight() * 0.5f);

		render_text_center(text, guide_text_font, text_x, text_y, Camera::zoom, color);
	}

	void render_all_guide_texts() {
		//WASD
		render_guide_text(21, 15, "WASD to move", yellow);

		//Can push block
		render_guide_text(10, 16, "Graves can be pushed", yellow);

		//Can pull block
		render_guide_text(11, 34, "Walk while dragging", yellow);
		render_guide_text(12, 34, "to pull", yellow);

		//Elements introduction (For knight)
		render_guide_text(20, 27, "Blue blocks", cyan);
		render_guide_text(20, 31, "can only be", yellow);
		render_guide_text(21, 29, "interacted by the knight", yellow);

		//Element introduction (For wizard)
		render_guide_text(20, 38, "Red blocks", red);
		render_guide_text(21, 38, "by the wizard", yellow);

		//How to switch characters
		render_guide_text(30, 27, "Press 1 n 2 to switch", yellow);
		render_guide_text(31, 27, "between characters", yellow);

		//Invisible blocks explaination (can add invisible graves js to show oso)
		render_guide_text(18, 48, "Some blocks are hidden", yellow);
		render_guide_text(19, 48, "from either characters", yellow);

		//LMB to attack
		render_guide_text(9, 48, "Left click to attack", yellow);

		//Enter door to go next floor
		render_guide_text(0, 48, "Enter door to advance", yellow);
	}

	// ------------------------------------------------------------------------
	// Special function for guide, bringing the player back to main menu or
	// allowing for restart when the player reaches the door.
	// ------------------------------------------------------------------------
	void check_cleared_guide(Player* player)
	{
		if (next == GS_RESTART) return;

		int playerRow, playerCol;
		if (!world_to_tile(player->current_player->x, player->current_player->y, playerRow, playerCol))
			return;

		if (map[playerRow][playerCol] == OPENED_DOOR)
		{
			if (!door_triggered) {
				door_triggered = true;

				cleared_guide = true;
			}
		}
		else {
			door_triggered = false;
		}
	}
}

void guide_load() {
	end_screen_load();
	if (!load_tile_textures("Assets/Sprites/Tiles"))
		next = GS_QUIT; // If tile textures fail to load, quit the game
	enemy_manager_instance.load_textures();
	Puzzle::load_puzzle_textures();
	load_ui_textures(&ui_manager); //removing param since ui_manager is from same file as definition
	skeleton_texture = AEGfxTextureLoad("Assets/Sprites/Tiles/dead_last_location.png");

	// Load font from Assets folder
	guide_text_font = AEGfxCreateFont("Assets/Fonts/Kenney Mini Square.ttf", 36);
	printf("Font value: %d\n", guide_text_font);
	if (guide_text_font == -1)
	{
		return;
	}

	tooltip_font = AEGfxCreateFont("Assets/Fonts/Kenney Mini Square.ttf", 28);
	printf("Font value: %d\n", tooltip_font);
	if (tooltip_font == -1)
	{
		return;
	}
}

void guide_init() {
	//Disable cursor
	AEInputShowCursor(false);

	//Initialize default game state values
	door_triggered = false;
	is_win = false;

	//Intialize default game state graphics
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	Camera::zoom = 1.2f;
	init_mesh();

	//Initialize minimap values
	totalWorldW = ((f32)TILE_COLS * tile_size) / minimap_zoom;
	totalWorldH = ((f32)TILE_ROWS * tile_size) / minimap_zoom;
	minimap_init(totalWorldW, totalWorldH);

	//Load tutorial level paths
	selected_level_path = "Assets/Levels/tutorial_level.txt";
	selected_level_puzzle_path = "Assets/Levels/tutorial_level_puzzles.txt";
	enemy_manager_instance.load_spawn_positions("Assets/Levels/tutorial_level_enemy_spawns.txt");

	//Load tutorial levels
	load_map_from_file(selected_level_path.c_str(), map);
	Puzzle::load_puzzle_data(selected_level_puzzle_path.c_str());
	//generate_walls_from_map(map, tile_size);

	//Initialize & spawn player/characters
	player_instance.stats.character_size = tile_size - 8.0f; //(consider moving to player system init)
	player_system_init(&player_instance,
		TileTextures[4], TileTextures[3],
		skeleton_texture);
	spawn_player_from_map(&player_instance, tile_size);

	// Camera follows the active character via the pointer
	Camera::cam = Camera::init_cam(
		&player_instance.current_player->x,
		&player_instance.current_player->y); 
	//(consider removing cam lim x,y, change to constexpr in cam.cpp)

	//Initialize & spawn enemies
	enemy_manager_instance.initialize_tutorial_area();

	cleared_guide = false;
}

void guide_update() {
	//Get mouse position
	AEInputGetCursorPosition(&mouse_x, &mouse_y);

	//Check if game is paused
	pause_update();
	if (is_paused || cleared_guide)
		return;

	//Update delta time
	dt = (f32)AEFrameRateControllerGetFrameTime();

	//Check if player left-clicked
	cursor_active = AEInputCheckCurr(AEVK_LBUTTON);

	//Update zoom levels
	get_zoom(dt);

	// Camera follows current_player - automatically updates when you switch characters!
	Camera::simulate_camera(&Camera::cam,
		&player_instance.current_player->x,
		&player_instance.current_player->y);
	
	player_system_update(&player_instance);
	entity_system_update(&player_instance, &enemy_manager_instance);

	Puzzle::puzzle_button_manager();
	Puzzle::puzzle_audio_update();

	check_cleared_guide(&player_instance);

	enemy_manager_instance.update(dt, &player_instance, &Camera::cam);

	// Lose condition
	if (player_is_dead(&player_instance))
	{
		is_win = false;
		restart_level_state = GS_LEVEL1;
		next = GS_LOSE;
	}
}

void guide_draw() {
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

	//Render guide texts
	render_all_guide_texts();

	//Renders minimap
	minimap_draw(&player_instance, map, TileTextures);

	//Renders the player health & skills UI
	render_player_hud(&ui_manager, &player_instance);
	render_character_ui(&ui_manager, &player_instance);
	render_tooltip(tooltip_font);

	if (cleared_guide) {
		is_win = true;
		next = GS_WIN;
	}
	else {
		//Renders the pause ui when game is paused
		pause_draw();
	}
	
	//Renders the custom cursor
	render_cursor(&ui_manager, &player_instance, mouse_x, mouse_y, cursor_active);
}

void guide_free() {
	//Frees player assets
	player_system_free(&player_instance);

	//Frees enemy assets
	enemy_manager_instance.clear_all();

	//Frees meshes
	free_mesh();

	//Free additional UI assets
	free_ui_textures(&ui_manager);
}

void guide_unload() {
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
