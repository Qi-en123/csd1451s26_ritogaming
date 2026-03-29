/*!************************************************************************
\file:			guide.cpp
\author:		Jerod
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			guidee
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

int DEBUGGING = 0;

static AEGfxTexture* skeleton_texture = nullptr;

namespace
{
	s32 mouse_x, mouse_y;
	bool cursor_active = false;
	static s8 guide_text_font = -1;
	static s8 tooltip_font = -1;
}

//will move this to a tutorial.cpp i swear
void render_guide_text(s32 row, s32 col, const char* text, color color) {
	f32 world_x = col_to_world(col);
	f32 world_y = row_to_world(row);

	// Mirror the sprite transform pipeline
	f32 text_x = (world_x - cam.pos.x) * zoom / (AEGfxGetWindowWidth() * 0.5f);
	f32 text_y = (world_y - cam.pos.y) * zoom / (AEGfxGetWindowHeight() * 0.5f);

	render_text_center(text, guide_text_font, text_x, text_y, 1.0f * zoom, color);
}

void render_all_guide_texts() {

	//WASD
	render_guide_text(29, 3, "WASD to move", yellow);

	//Can push block
	render_guide_text(18, 4, "Graves can be pushed", yellow);

	//Can pull block
	render_guide_text(19, 22, "Walk while dragging", yellow);
	render_guide_text(20, 22, "to pull", yellow);

	//Elements introduction (For knight)
	render_guide_text(28, 15, "Blue blocks", blue);
	render_guide_text(28, 19, "can only be", yellow);
	render_guide_text(29, 17, "interacted by the knight", yellow);

	//Element introduction (For wizard)
	render_guide_text(28, 26, "Red blocks", red);
	render_guide_text(29, 26, "by the wizard", yellow);

	//How to switch characters
	render_guide_text(38, 15, "Press 1&2 to switch", yellow);
	render_guide_text(39, 15, "between characters", yellow);

	//Invisible blocks explaination (can add invisible graves js to show oso)
	render_guide_text(26, 36, "Some blocks are hidden", yellow);
	render_guide_text(27, 36, "from either characters", yellow);

	//LMB to attack
	render_guide_text(17, 36, "Left click to attack", yellow);

	//Enter door to go next floor
	render_guide_text(7, 36, "Enter door to advance", yellow);
}

void guide_load() {
	end_screen_load();
	if (!load_tile_textures("Assets/Tiles"))
		next = GS_QUIT; // If tile textures fail to load, quit the game
	enemy_manager_instance.load_textures();
	Puzzle::load_puzzle_textures();
	load_ui_textures(&ui_manager); //removing param since ui_manager is from same file as definition
	skeleton_texture = AEGfxTextureLoad("Assets/Tiles/dead_last_location.png");

	// Load font from Assets folder
	guide_text_font = AEGfxCreateFont("Assets/Kenney Mini Square.ttf", 36);
	printf("Font value: %d\n", guide_text_font);
	if (guide_text_font == -1)
	{
		return;
	}

	tooltip_font = AEGfxCreateFont("Assets/Kenney Mini Square.ttf", 28);
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
	zoom = 1.2f;
	init_mesh();

	//Initialize minimap values
	totalWorldW = ((f32)TILE_COLS * tile_size) / ZOOM_LEVEL;
	totalWorldH = ((f32)TILE_ROWS * tile_size) / ZOOM_LEVEL;
	minimap_init(totalWorldW, totalWorldH, 300.0f);

	//Load tutorial level paths
	selected_level_path = "Assets/Levels/tutorial_level.txt";
	selected_level_puzzle_path = "Assets/Levels/tutorial_level_puzzles.txt";

	//Load tutorial levels
	load_map_from_file(selected_level_path.c_str(), map);
	Puzzle::load_puzzle_data(selected_level_puzzle_path.c_str());
	generate_walls_from_map(map, tile_size);

	//Initialize & spawn player/characters
	player_instance.stats.character_size = tile_size - 8.0f; //(consider moving to player system init)
	player_system_init(&player_instance,
		TileTextures[4], TileTextures[3],
		skeleton_texture);
	spawn_player_from_map(&player_instance, tile_size);

	// Camera follows the active character via the pointer
	cam = init_cam(
		&player_instance.current_player->x,
		&player_instance.current_player->y,
		60.0f, 5.0f); 
	//(consider removing cam lim x,y, change to constexpr in cam.cpp)

	//Initialize & spawn enemies
	enemy_manager_instance.clear_all();
	enemy_manager_instance.initialize_tutorial_area();
}

void guide_update() {
	//Get mouse position
	AEInputGetCursorPosition(&mouse_x, &mouse_y);

	//Check if game is paused
	pause_update();
	if (is_paused)
		return;

	//Update delta time
	dt = (f32)AEFrameRateControllerGetFrameTime();

	//Check if player left-clicked
	cursor_active = AEInputCheckCurr(AEVK_LBUTTON);

	//Update zoom levels
	get_zoom(dt);

	// Camera follows current_player - automatically updates when you switch characters!
	simulate_camera(&cam,
		&player_instance.current_player->x,
		&player_instance.current_player->y,
		-6000.0f, 6000.0f,	//left & right camera limits (calculate from map tiles tk u)
		-5000.0f, 5000.0f); //top & bottom camera limits

	player_system_update(&player_instance);
	entity_system_update(&player_instance, &enemy_manager_instance);

	Puzzle::puzzle_button_manager();
	Puzzle::puzzle_audio_update();

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

void guide_draw() {
	AEGfxReset();

	//Renders map tiles
	draw_map(tile_size);

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

	//Renders the pause ui when game is paused
	pause_draw(); //consider putting is_pause condition here: if not look like drawing pause every frame

	//Renders the custom cursor
	render_cursor(&ui_manager, &player_instance, mouse_x, mouse_y, cursor_active);

#if DEBUGGING
	debug_draw_all_walls();
#endif
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
