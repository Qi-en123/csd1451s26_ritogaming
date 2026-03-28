/*!************************************************************************
\file:          level_editor.cpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         This file defines functions to load, initialize, update, draw, free, and unload
				the game's level interface which allows users to create custom maps by painting tiles on a grid and saving them to a file. 
                The editor features a tile palette, camera panning, and visual feedback for tile placement and saving status.
**************************************************************************/

#include "AEEngine.h"
#include "renderer.hpp"
#include "utils.hpp"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "map.hpp"
#include "puzzle.hpp"
#include "level_editor.hpp"
#include <string>
#include "enemy_manager.hpp"

namespace {
	/// Assets
	AEGfxTexture* erase_tile = nullptr;
	AEGfxTexture* cursor = nullptr;
	AEGfxTexture* ghost_spawn = nullptr;
	AEGfxTexture* tomb = nullptr;
	AEGfxTexture* bat_spawn = nullptr;
	s32 mouse_x, mouse_y;
	static s8 font = -1;

	// --- Tile Palette ---
	const int PALETTE_COUNT = 8;
	const int palette_ids[PALETTE_COUNT] = { 0, 1, 2, 3, 4, 7, 5, 6 };
	const char* palette_labels[PALETTE_COUNT] = { "Erase", "Wall", "Floor", "Wizard", "Knight", "Door", "Ghost", "Bat" };
	int selected_tile = 1; // default to wall

	// --- Enemy Spawn message ---
	float spawn_error_timer = 0.0f;
	const float SPAWN_ERROR_DUR = 2.0f; // Message stays for 2 seconds

	// --- Puzzle Palette ---
	const int EDITABLE_PUZZLE_COUNT = 5;
	const Puzzle::PuzzleType puzzle_palette_ids[EDITABLE_PUZZLE_COUNT] =
	{	Puzzle::PUZZLE_TYPE_COUNT,
		Puzzle::PUSHABLE,
		Puzzle::BUTTON_UP,
		Puzzle::PIT,
		Puzzle::SPIKE };

	const char* puzzle_labels[EDITABLE_PUZZLE_COUNT] =
	{ "Erase", "Grave", "Button", "Pit", "Spike" };

	Puzzle::PuzzleType selected_puzzle = Puzzle::PUSHABLE; //default

	// --- Palette UI (right side panel) ---
	const f32 PANEL_X = 650;  // center X of palette panel
	const f32 PALETTE_TOP_Y = 300.f;  // top item Y
	const f32 PAL_W = 200.f;
	const f32 PAL_H = 55.f;
	const f32 PAL_GAP = 68.f;

	// One-time placement limits
	bool wizard_placed = false;
	bool knight_placed = false;
	bool door_placed = false;


	// --- Grid rendering ---
	// calculated in init
	f32 tile_draw_size = 0.f;
	f32 grid_start_x = 0.f;
	f32 grid_start_y = 0.f;

	// --- Outer borders ---
	f32 top_border_height;
	f32 bottom_border_height;
	f32 side_border_height;
	f32 left_border_width;
	f32 right_border_width;

	// --- Camera limits ---
	f32 view_min_x;
	f32 view_max_x;
	f32 view_min_y;
	f32 view_max_y;
	f32 view_w;
	f32 view_h;

	// --- Save feedback ---
	bool save_success = false;
	f32  save_msg_timer = 0.f;
	const f32 SAVE_MSG_DUR = 2.f;

	// --- Camera pan ---
	const f32 CAM_SPEED = 300.f;

	f32 editor_zoom_min = 1.0f;
	f32 editor_zoom_max = 1.8f;

	f32 cam_x, cam_y;

	/// --- Painting control ---
	static bool can_paint = false;

	/// --- Map data ---
	s32** editor_map = nullptr; // for creating new map
	Puzzle::PuzzleType** puzzle_editor_map = nullptr;
	std::vector<Puzzle::puzzle> puzzles;
	std::vector<Puzzle::buttonTrigger> button_triggers{};

	/// --- Editing States ---
	EditorState editorState = EDIT_TILES;
	bool editing{ false };
	Puzzle::puzzle* editing_puzzle{ nullptr };
	Puzzle::puzzle old_puzzle{};


	/// --- Edit interface UI ---
	AEGfxTexture* edit_panel = nullptr;
	AEGfxTexture* edit_panel_save = nullptr;
	AEGfxTexture* edit_panel_exit = nullptr;
	AEGfxTexture* edit_panel_button_left = nullptr;
	AEGfxTexture* edit_panel_button_right = nullptr;

	const f32 edit_panel_w = 350.0f;
	const f32 edit_panel_h = 175.0f;
	f32 edit_panel_x;
	f32 edit_panel_y;
	f32 ep_button_x;
}

// -------------------------------------------------------
// Helpers: convert world mouse pos to map row/col
// -------------------------------------------------------
static void mouse_to_world(f32& wx, f32& wy)
{
	wx = (f32)mouse_x - 800.f; // convert from screen (0 to 1600) to world (-800 to 800)
	wy = -(f32)mouse_y + 450.f; // convert from screen (0 to 900) to world (450 to -450)
}

// ------------------------------------------------
// Returns true if the world coordinates are within the grid bounds and sets row/col accordingly
// The positions are offset according to editor space, hence function is specific for level editor
// ------------------------------------------------
static bool editor_screen_to_tile(f32 wx, f32 wy, int& row, int& col)
{
	f32 world_x = (wx / zoom) + cam.pos.x;
	f32 world_y = (wy / zoom) + cam.pos.y;

	// convert world X to column index
	col = static_cast<int>((world_x - grid_start_x) / tile_draw_size);

	// convert world Y to row index (inverted because screen Y is downwards)
	row = static_cast<int>((grid_start_y - world_y) / tile_draw_size);

	return (row >= 0 && row < TILE_ROWS && col >= 0 && col < TILE_COLS);
}

static void clamp_cam() {
	if (AEInputCheckCurr(AEVK_LEFT)) {
		cam.pos.x -= dt * CAM_SPEED;
	}
	if (AEInputCheckCurr(AEVK_RIGHT)) {
		cam.pos.x += dt * CAM_SPEED;
	}
	if (AEInputCheckCurr(AEVK_UP)) {
		cam.pos.y += dt * CAM_SPEED;
	}
	if (AEInputCheckCurr(AEVK_DOWN)) {
		cam.pos.y -= dt * CAM_SPEED;
	}

	//cam pos represents middle of cam
	//update cam bounds (when zoom is increased, cam can go further)
	//cam is centered on screen origin so:
	//grid_start = campos + view_min / zoom
	f32 cam_min_x = grid_start_x - view_min_x / zoom;
	f32 cam_max_x = grid_start_x + (TILE_COLS * tile_draw_size) - (view_min_x + view_w) / zoom;

	f32 cam_min_y = grid_start_y - (TILE_ROWS * tile_draw_size) - (view_max_y - view_h) / zoom;
	f32 cam_max_y = grid_start_y - view_max_y / zoom;

	cam.pos.x = AEClamp(cam.pos.x, cam_min_x, cam_max_x);
	cam.pos.y = AEClamp(cam.pos.y, cam_min_y, cam_max_y);
}

static void draw_puzzle_edit_interface() {

	edit_panel_x = grid_start_x + edit_panel_w / 2 + 20.0f;
	edit_panel_y = edit_panel_h / 2 - grid_start_y;

	ep_button_x = edit_panel_x + (edit_panel_w / 2) + (edit_panel_h / 4.0f) - 5.0f;

	//Edit interface save button
	render_editor_ui(edit_panel_save, edit_panel_h / 2.0f, edit_panel_h / 2.0f, 1.0f,
		ep_button_x, edit_panel_y + edit_panel_h / 4.0f);

	//Edit interface exit button
	render_editor_ui(edit_panel_exit, edit_panel_h / 2.0f, edit_panel_h / 2.0f, 1.0f,
		ep_button_x, edit_panel_y - edit_panel_h / 4.0f);

	//Edit interface panel (bg)
	render_editor_ui(edit_panel, edit_panel_w, edit_panel_h, 1.0f,
		edit_panel_x, edit_panel_y);


	//Edit interface text
	std::string line1{};
	std::string line2{};

	switch (editing_puzzle->type) {
	case Puzzle::PUSHABLE:
	case Puzzle::PIT:

		switch (editing_puzzle->element) {
		case Puzzle::WATER:
			line1 = "Element: Water";
			break;
		case Puzzle::FIRE:
			line1 = "Element: Fire";
			break;
		default:
			line1 = "Element: None";
			break;
		}

		switch (editing_puzzle->hidden_from) {
		case 1:
			line2 = "Can't see: Knight";
			break;
		case 2:
			line2 = "Can't see: Wizard";
			break;
		default:
			line2 = "Can't see: None";
			break;
		}

		break;
		//Button edit is different, when button is selected it will lock
	case Puzzle::BUTTON_UP:
		line1 = "Place spikes to pair...";

		switch (editing_puzzle->hidden_from) {
		case 1:
			line2 = "Can't see: Knight";
			break;
		case 2:
			line2 = "Can't see: Wizard";
			break;
		default:
			line2 = "Can't see: None";
			break;
		}

		break;

	default:
		break;
	}

	//Print lines
	f32 text_w, text_h;
	char const* msg1 = line1.c_str();
	char const* msg2 = line2.c_str();
	f32 text_scale = 1.15f;
	AEGfxGetPrintSize(font, msg1, text_scale, &text_w, &text_h);

	f32 half_w = AEGfxGetWindowWidth() / 2.0f;
	f32 half_h = AEGfxGetWindowHeight() / 2.0f;

	f32 norm_x = edit_panel_x / half_w;
	f32 norm_y = edit_panel_y / half_h;
	f32 button_size = edit_panel_h / 4.0f;

	//First line (1st param)
	f32 line1_norm_y = norm_y + text_h * 1.5f;

	//Button edit is different not edited using buttons (i v tired)
	if (editing_puzzle->type != Puzzle::BUTTON_UP) {
		//LEFT button
		render_editor_ui(edit_panel_button_left, //tex
			button_size, button_size, 1.0f, //width, height, alpha
			edit_panel_x - edit_panel_w * 0.35f, //posx
			edit_panel_y + edit_panel_h * 0.18f); //posy

		//RIGHT button
		render_editor_ui(edit_panel_button_right,
			button_size, button_size, 1.0f,
			edit_panel_x + edit_panel_w * 0.35f,
			edit_panel_y + edit_panel_h * 0.18f);
	}
	//Actual text showing param to edit
	render_text_center(msg1, font, norm_x, line1_norm_y, text_scale, black);


	//Second line (2nd param)
	f32 line2_norm_y = norm_y - text_h * 1.5f;

	render_editor_ui(edit_panel_button_left,
		button_size, button_size, 1.0f,
		edit_panel_x - edit_panel_w * 0.35f,
		edit_panel_y - edit_panel_h * 0.16f);

	render_editor_ui(edit_panel_button_right,
		button_size, button_size, 1.0f,
		edit_panel_x + edit_panel_w * 0.35f,
		edit_panel_y - edit_panel_h * 0.16f);

	render_text_center(msg2, font, norm_x, line2_norm_y, text_scale, black);
}

static void edit_puzzle() {
	if (AEInputCheckTriggered(AEVK_LBUTTON)) {

		//If save button is clicked
		if (IsButtonClicked(ep_button_x, edit_panel_y + edit_panel_h / 4.0f,
			edit_panel_h / 2.0f, edit_panel_h / 2.0f)) {

			//printf("saved\n");
			editing = false;
			selected_puzzle = Puzzle::PUSHABLE;
			can_paint = false;
		}

		//If exit/cancel button is clicked (won't save)
		if (IsButtonClicked(ep_button_x, edit_panel_y - edit_panel_h / 4.0f,
			edit_panel_h / 2.0f, edit_panel_h / 2.0f)) {

			//printf("exit\n");
			//Restore changes
			*editing_puzzle = old_puzzle;
			editing = false;
			selected_puzzle = Puzzle::PUSHABLE;
			can_paint = false;
		}


		if (editing_puzzle->type != Puzzle::BUTTON_UP) {
			//If left button of 1st line is clicked ("Element:" line)
			if (IsButtonClicked(edit_panel_x - edit_panel_w * 0.35f,
				edit_panel_y + edit_panel_h * 0.18f,
				edit_panel_h / 4.0f, edit_panel_h / 4.0f)) {

				//printf("left clicked\n");
				editing_puzzle->element = previous_element(editing_puzzle->element);
				can_paint = false;
			}

			//If right button of 1st line is clicked ("Element:" line)
			if (IsButtonClicked(edit_panel_x + edit_panel_w * 0.35f,
				edit_panel_y + edit_panel_h * 0.18f,
				edit_panel_h / 4.0f, edit_panel_h / 4.0f)) {

				//printf("right clicked\n");
				editing_puzzle->element = next_element(editing_puzzle->element);
				can_paint = false;
			}
		}

		//If left button of 2nd line is clicked ("Can't see:" line)
		if (IsButtonClicked(edit_panel_x - edit_panel_w * 0.35f,
			edit_panel_y - edit_panel_h * 0.16f,
			edit_panel_h / 4.0f, edit_panel_h / 4.0f)) {

			//decrement hero that puzzle is hidden from
			editing_puzzle->hidden_from = (editing_puzzle->hidden_from + 2) % 3;
			can_paint = false;
			//printf("%d\n", editing_puzzle->hidden_from);
		}

		//If right button of 2nd line is clicked ("Can't see:" line)
		if (IsButtonClicked(edit_panel_x + edit_panel_w * 0.35f,
			edit_panel_y - edit_panel_h * 0.16f,
			edit_panel_h / 4.0f, edit_panel_h / 4.0f)) {

			//decrement hero that puzzle is hidden from
			editing_puzzle->hidden_from = (editing_puzzle->hidden_from + 1) % 3;
			can_paint = false;
		}

	}
}




void level_editor_load()
{
	// Load tile textures once and reuse for palette icons and grid rendering
	load_tile_textures("Assets/Tiles");
	cursor = AEGfxTextureLoad("Assets/Admin/cursorSword_silver.png");
	tomb = AEGfxTextureLoad("Assets/Enemy_spawn_location/tile_0064.png");
	ghost_spawn = AEGfxTextureLoad("Assets/Enemy_spawn_location/tile_0066.png");
	bat_spawn = AEGfxTextureLoad("Assets/Enemy_spawn_location/tile_0126.png");
	font = AEGfxCreateFont("Assets/Kenney Mini Square.ttf", 22);
	erase_tile = AEGfxTextureLoad("Assets/Tiles/tile_0010.png");
	edit_panel = AEGfxTextureLoad("Assets/Admin/edit_interface_panel.png");
	edit_panel_save = AEGfxTextureLoad("Assets/Admin/edit_interface_save.png");
	edit_panel_exit = AEGfxTextureLoad("Assets/Admin/edit_interface_exit.png");
	edit_panel_button_left = AEGfxTextureLoad("Assets/Admin/ep_button_left.png");
	edit_panel_button_right = AEGfxTextureLoad("Assets/Admin/ep_button_right.png");

	Puzzle::load_puzzle_textures();
}

void level_editor_init()
{
	zoom = editor_zoom_min;
	cam_x = cam_y = 0.0f;
	cam = init_cam(&cam_x, &cam_y, 60.0f, 5.0f);

	init_mesh(); // ensure mesh is initialized before any rendering calls
	AEGfxSetBackgroundColor(0.08f, 0.09f, 0.12f);
	AEInputShowCursor(false); // we'll draw our own cursor, so hide the default one

	//Default init editor map (FOR TILES)
	editor_map = new int* [TILE_ROWS];
	for (int r = 0; r < TILE_ROWS; ++r)
	{
		editor_map[r] = new int[TILE_COLS] {};  // {} zero-initializes
	}


	//Default init puzzle editor map (FOR PUZZLES)
	puzzle_editor_map = new Puzzle::PuzzleType * [TILE_ROWS];
	for (int r = 0; r < TILE_ROWS; ++r)
	{
		puzzle_editor_map[r] =
			new Puzzle::PuzzleType[TILE_COLS]{ Puzzle::PUZZLE_TYPE_COUNT };  // {} zero-initializes
	}
	for (int r = 0; r < TILE_ROWS; ++r)
		for (int c = 0; c < TILE_COLS; ++c)
			puzzle_editor_map[r][c] = Puzzle::PUZZLE_TYPE_COUNT;

	puzzles.reserve(100);

	// Tile size to be drawn on the grid
	tile_draw_size = 20.f;

	// Calculate grid starting position to center it in the remaining space
	//f32 grid_w = tile_draw_size * TILE_COLS; // total width of the grid in world units
	//f32 grid_h = tile_draw_size * TILE_ROWS; // total height of the grid in world units

	grid_start_x = -750.f;  // flush to left edge of world
	grid_start_y = 350.f;   // centered vertically

	// --- Outer borders ---
	top_border_height = tile_draw_size * 5.0f;
	bottom_border_height = tile_draw_size * 4.0f;
	side_border_height = AEGfxGetWindowWidth() - top_border_height - bottom_border_height;
	left_border_width = tile_draw_size * 2.5f;
	right_border_width = tile_draw_size * 13.5f;

	// --- Camera limits ---
	view_min_x = AEGfxGetWinMinX() + left_border_width;
	view_max_x = AEGfxGetWinMaxX() - right_border_width;
	view_min_y = AEGfxGetWinMinY() + bottom_border_height;
	view_max_y = AEGfxGetWinMaxY() - top_border_height;
	view_w = view_max_x - view_min_x;
	view_h = view_max_y - view_min_y;


	selected_tile = 1;
	save_success = false;
	save_msg_timer = 0.f;

	wizard_placed = false;
	knight_placed = false;
	door_placed = false;

	can_paint = false;

	editorState = EDIT_TILES;
	editing = false;
	editing_puzzle = nullptr;
	
	//If editing existing level (from level interface)
    if (!selected_level_path.empty())
    {
        load_map_to_editor(selected_level_path.c_str(), editor_map);

        // Restore placement flags
        for (int r = 0; r < TILE_ROWS; ++r)
            for (int c = 0; c < TILE_COLS; ++c)
            {
                if (editor_map[r][c] == 3) wizard_placed = true;
                if (editor_map[r][c] == 4) knight_placed = true;
                if (editor_map[r][c] == 7) door_placed = true;
            }
    }
}

void level_editor_update()
{
	if (AEInputCheckReleased(AEVK_LBUTTON) || AEInputCheckReleased(AEVK_RBUTTON))
		can_paint = true;

	dt = (f32)AEFrameRateControllerGetFrameTime();
	get_zoom(dt, editor_zoom_min, editor_zoom_max);

	AEInputGetCursorPosition(&mouse_x, &mouse_y);

	
	// --- ESC to exit ---
    if (AEInputCheckTriggered(AEVK_ESCAPE))
        next = GS_LEVELS; // adjust to your menu state

	// --- Tick save message ---
	if (save_msg_timer > 0.f)
		save_msg_timer -= dt;
	if (spawn_error_timer > 0.0f)
		spawn_error_timer -= dt;

    // --- Save with S key ---
	if (AEInputCheckTriggered(AEVK_S))
	{
		if (wizard_placed && knight_placed && door_placed)
		{
			if (!selected_level_path.empty())
			{
				save_success =
					save_map_to_file_overwrite(editor_map, selected_level_path) &&
					overwrite_puzzle_blocks(puzzles, button_triggers, selected_level_path);
				std::string spawn_path = selected_level_path.substr(0, selected_level_path.find_last_of('.')) + "_enemy_spawns.txt";
				enemy_manager_instance.save_spawn_positions_from_map(editor_map, spawn_path);
			}
			else
			{
				std::string new_path;
				save_success = save_map_to_file(editor_map, new_path);

				if (save_success)
				{
					// Capture spawn path from map path BEFORE puzzle blocks modifies new_path
					std::string spawn_path = new_path.substr(0, new_path.find_last_of('.')) + "_enemy_spawns.txt";
					enemy_manager_instance.save_spawn_positions_from_map(editor_map, spawn_path);

					save_success = save_puzzle_blocks(puzzles, button_triggers, new_path);

					if (save_success)
						selected_level_path = new_path;
				}
			}
		}
		else
		{
			save_success = false;
		}
		save_msg_timer = SAVE_MSG_DUR;
	}




	//Switch editing state
	if (!editing) {
		if (AEInputCheckTriggered(AEVK_LBUTTON)) {
			if (IsButtonClicked(PANEL_X + PAL_W / 4, PALETTE_TOP_Y + PAL_H, PAL_W / 2, PAL_H)) {
				editorState = EDIT_PUZZLES;
			}
			if (IsButtonClicked(PANEL_X - PAL_W / 4, PALETTE_TOP_Y + PAL_H, PAL_W / 2, PAL_H)) {
				editorState = EDIT_TILES;
			}
		}
	}
	else {
		edit_puzzle();
	}

	//prevent camera from going out of map grids
	//handles camera movement as well
	clamp_cam();

	if (editorState == EDIT_TILES) {

		// --- Palette hotkeys 1-8 ---
		const u8 hotkeys[8] = { AEVK_1, AEVK_2, AEVK_3, AEVK_4, AEVK_5, AEVK_6, AEVK_7, AEVK_8 };
		for (int i = 0; i < PALETTE_COUNT; ++i)
			if (AEInputCheckTriggered(hotkeys[i]))
				selected_tile = palette_ids[i];

		// --- Palette click ---
		if (AEInputCheckTriggered(AEVK_LBUTTON))
		{
			for (int i = 0; i < PALETTE_COUNT; ++i)
			{
				f32 py = PALETTE_TOP_Y - i * PAL_GAP;
				if (IsButtonClicked(PANEL_X, py, PAL_W, PAL_H))
				{
					selected_tile = palette_ids[i];
					return; // stop here, don't also paint on grid
				}
			}
		}

		// --- Paint / Erase on grid ---
		f32 wx, wy;
		mouse_to_world(wx, wy);
		int row, col;
		bool on_grid = editor_screen_to_tile(wx, wy, row, col);
		// Prevent painting when mouse is over the right panel
		f32 grid_right_world = grid_start_x + (TILE_COLS * tile_draw_size);
		f32 grid_right_screen = grid_right_world + 800.f;
		bool on_panel = ((f32)mouse_x > grid_right_screen);
		// Allow painting only after mouse is released once inside editor
		//if (AEInputCheckReleased(AEVK_LBUTTON))
		//	can_paint = true;

		if (on_grid && !on_panel)
		{
			// LEFT CLICK PAINT
			if (can_paint && AEInputCheckCurr(AEVK_LBUTTON))
			{
				int current = editor_map[row][col];
				int placing = selected_tile;

				// Ghost/bat spawn only allowed on floor
				if ((placing == 5 || placing == 6) && current != 2)
					goto skip_paint;

				if ((placing == 5 ) && row >= (TILE_ROWS - 1))
				{
					spawn_error_timer = SPAWN_ERROR_DUR;
					goto skip_paint;
				}

				{
					bool blocked =
						(placing == 3 && wizard_placed && current != 3) ||
						(placing == 4 && knight_placed && current != 4) ||
						(placing == 7 && door_placed && current != 7);

					if (!blocked)
					{
						if (current == 3) wizard_placed = false;
						if (current == 4) knight_placed = false;
						if (current == 7) door_placed = false;

						// Painting over a spawn marker — clear it first
if (current == 5 || current == 6)
    editor_map[row][col] = 2; // restore floor before overwriting

// If placing a tomb on a tile that is the ghost_spawn row of an existing tomb above,
// clear that tomb (turn it back to floor), so the new tomb takes over
if (placing == 5 && row > 0 && editor_map[row - 1][col] == 5)
    editor_map[row - 1][col] = 2; // remove the tomb above

// If placing a tomb and its ghost_spawn row below already has a tomb,
// clear that tomb below
if (placing == 5 && row < (TILE_ROWS - 1) && editor_map[row + 1][col] == 5)
    editor_map[row + 1][col] = 2; // remove the tomb below

						editor_map[row][col] = placing;

						if (placing == 3) wizard_placed = true;
						if (placing == 4) knight_placed = true;
						if (placing == 7) door_placed = true;
					}
				}
			skip_paint:;
			}

			// RIGHT CLICK ERASE
			if (can_paint && AEInputCheckCurr(AEVK_RBUTTON))
			{
				int current = editor_map[row][col];

				if (current == 3) wizard_placed = false;
				if (current == 4) knight_placed = false;
				if (current == 7) door_placed = false;

				// Erasing spawn marker restores floor, not empty
				if (current == 5 || current == 6)
					editor_map[row][col] = 2;
				else
					editor_map[row][col] = 0;
			}
		}

		// --- Fill entire grid with selected tile (F key) ---
		if (AEInputCheckTriggered(AEVK_F))
		{
			// Block fill entirely for unique tiles
			// Block fill entirely for unique tiles
			if (selected_tile == 3 || selected_tile == 4 || selected_tile == 7)
				return; // do nothing

			for (int r = 0; r < TILE_ROWS; ++r)
				for (int c = 0; c < TILE_COLS; ++c)
					if (editor_map[r][c] == 0)
						editor_map[r][c] = selected_tile;
		}

		// --- Clear entire grid (C key) ---
		if (AEInputCheckTriggered(AEVK_C))
		{
			for (int r = 0; r < TILE_ROWS; ++r)
				for (int c = 0; c < TILE_COLS; ++c)
					editor_map[r][c] = 0;
			wizard_placed = false; // reset flags too
			knight_placed = false;
			door_placed = false;
		}
	}

	// --- EDITING PUZZLES ---
	else if (editorState == EDIT_PUZZLES) {

		if (!editing) {
			// --- Puzzle palette hotkeys 1-3 ---
			const u8 hotkeys[EDITABLE_PUZZLE_COUNT] = { AEVK_1, AEVK_2, AEVK_3, AEVK_4 };
			for (int i = 0; i < EDITABLE_PUZZLE_COUNT; ++i)
				if (AEInputCheckTriggered(hotkeys[i]))
					selected_puzzle = puzzle_palette_ids[i];

			// --- Palette click ---
			if (AEInputCheckTriggered(AEVK_LBUTTON))
			{
				for (int i = 0; i < EDITABLE_PUZZLE_COUNT; ++i)
				{
					f32 py = PALETTE_TOP_Y - i * PAL_GAP;
					if (IsButtonClicked(PANEL_X, py, PAL_W, PAL_H))
					{
						if (puzzle_palette_ids[i] != Puzzle::SPIKE) {
							selected_puzzle = puzzle_palette_ids[i];
							return; // stop here, don't also paint on grid
						}
						else {
							//print "Spike must be placed with buttons"
						}
					}
				}
			}

			// --- Clear entire grid (C key) ---
			if (AEInputCheckTriggered(AEVK_C))
			{
				for (int r = 0; r < TILE_ROWS; ++r)
					for (int c = 0; c < TILE_COLS; ++c)
						puzzle_editor_map[r][c] = Puzzle::PUZZLE_TYPE_COUNT;

				puzzles.clear();
			}
		}

		else if (editing_puzzle->type == Puzzle::BUTTON_UP) {
			//offset from erase
			selected_puzzle = puzzle_palette_ids[Puzzle::SPIKE+1];
		}

		// --- Paint / Erase on grid ---
		f32 wx, wy;
		mouse_to_world(wx, wy);
		int row, col;
		bool on_grid = editor_screen_to_tile(wx, wy, row, col);
		// Prevent painting when mouse is over the right panel
		f32 grid_right_world = grid_start_x + (TILE_COLS * tile_draw_size);
		f32 grid_right_screen = grid_right_world + 800.f;
		bool on_panel = ((f32)mouse_x > grid_right_screen);
		// Allow painting only after mouse is released once inside editor
		//if (AEInputCheckReleased(AEVK_LBUTTON))
		//	can_paint = true;

		if (on_grid && !on_panel)
		{

			//if not editing, or editing puzzle type is button
			if (!editing) {
				// LEFT CLICK PAINT
				if (can_paint && AEInputCheckCurr(AEVK_LBUTTON))
				{
					//int current = puzzle_editor_map[row][col];
					//if current tile is floor and is empty
					if (editor_map[row][col] == 2) {

						//if erasing 
						if (selected_puzzle == Puzzle::PUZZLE_TYPE_COUNT) {
							puzzle_editor_map[row][col] = selected_puzzle;
							remove_puzzle(row, col, puzzles);
						}
						//if adding puzzles
						else if (puzzle_editor_map[row][col] == Puzzle::PUZZLE_TYPE_COUNT) {
							puzzle_editor_map[row][col] = selected_puzzle;
							//printf("%d %d\n", row, col);
							place_puzzle(row, col, selected_puzzle, puzzles);
						}
					}
				}

				// RIGHT CLICK TO EDIT PUZZLE
				if (can_paint && AEInputCheckCurr(AEVK_RBUTTON))
				{
					int current = puzzle_editor_map[row][col];

					if (puzzle_editor_map[row][col] != Puzzle::PUZZLE_TYPE_COUNT) {

						//open interface
						editing = true;
						editing_puzzle = get_editing_puzzle(row, col, puzzles);
						old_puzzle = *get_editing_puzzle(row, col, puzzles);
						can_paint = false;

						if (puzzle_editor_map[row][col] == Puzzle::BUTTON_UP) {

							if (editing_puzzle->button_id == -1) {
								//button id is the number of ID triggers yet
								editing_puzzle->button_id = button_triggers.size();

								//add btnTrigger struct (containing new id and empty coords pairs)
								button_triggers.push_back(Puzzle::buttonTrigger{ editing_puzzle->button_id,{} });
							}
							else {

							}

							//printf("now editing button id: %d\n", editing_puzzle->button_id);
						}
					}

				}
			}

			else if (editing_puzzle->type == Puzzle::BUTTON_UP) {
				if (AEInputCheckCurr(AEVK_LBUTTON)) {

					//PLACE SPIKE DOWN
					if (can_paint) {
						//if theres a floor beneath and is empty
						if (editor_map[row][col] == 2 &&
							puzzle_editor_map[row][col] == Puzzle::PUZZLE_TYPE_COUNT) {

							puzzle_editor_map[row][col] = selected_puzzle;

							//add to puzzles vec
							place_puzzle(row, col, Puzzle::SPIKE, puzzles);

							//pairing spikes placed to id of current editing button
							pair_spike_to_button(row, col, button_triggers[editing_puzzle->button_id]);

							//printf("adding spike %d, %d to button id: %d\n", row, col, editing_puzzle->button_id);
						}
					}

					//PAIR WITH OTHER BUTTON
					if (puzzle_editor_map[row][col] == Puzzle::BUTTON_UP) {
						//if clicked on button with no triggers
						Puzzle::puzzle& curr = *get_editing_puzzle(row, col, puzzles);
						//printf("%d\n", curr.button_id);
						if (curr.button_id == -1) {
							
							curr.button_id = editing_puzzle->button_id;
						}
					}
				}

				else if (can_paint && AEInputCheckCurr(AEVK_RBUTTON)) {

					//if is spike and the spike is paired with the id
					if (puzzle_editor_map[row][col] == Puzzle::SPIKE &&
						spike_is_paired(row, col, button_triggers[editing_puzzle->button_id])) {

						puzzle_editor_map[row][col] = Puzzle::PUZZLE_TYPE_COUNT;

						//remove spike from puzzles vec
						remove_puzzle(row, col, puzzles);

						//remove this spike from trigger list of coords
						remove_spike_button_pair(row, col, button_triggers[editing_puzzle->button_id]);

						//printf("removing spike %d, %d from button id: %d\n", row, col, editing_puzzle->button_id);
						//printf("current coords size: %zu\n", button_triggers[editing_puzzle->button_id].coords.size());
					}
				}
			}

		}
	}
}

void level_editor_draw()
{
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	//f32 grid_right = grid_start_x + TILE_COLS * tile_draw_size;

	// Draw black tile as grid background for ALL cells
	if (erase_tile)
	{
		for (int r = 0; r < TILE_ROWS; ++r)
		{
			for (int c = 0; c < TILE_COLS; ++c)
			{
				f32 wx = grid_start_x + c * tile_draw_size + tile_draw_size * 0.5f;
				f32 wy = grid_start_y - r * tile_draw_size - tile_draw_size * 0.5f;
				render_editor_tiles(erase_tile, tile_draw_size, tile_draw_size, 1.0f, wx, wy);
			}
		}
	}

	// Draw actual map tiles on top
	for (int r = 0; r < TILE_ROWS; ++r)
	{
		for (int c = 0; c < TILE_COLS; ++c)
		{
			f32 wx = grid_start_x + c * tile_draw_size + (tile_draw_size * 0.5f);
			f32 wy = grid_start_y - r * tile_draw_size - (tile_draw_size * 0.5f);
			// to optimize, skip drawing tiles that are outside the visible world bounds (including right panel area)
			f32 grid_left = grid_start_x;
			f32 grid_right = grid_left + TILE_COLS * tile_draw_size;
			f32 grid_top = grid_start_y;
			f32 grid_bot = grid_top - TILE_ROWS * tile_draw_size;

			if (wx < grid_left || wx > grid_right) continue;
			if (wy > grid_top || wy < grid_bot) continue;

			int tile = editor_map[r][c];
			if (tile == 5 || tile == 6)
			{
				if (TileTextures[2])
					render_editor_tiles(TileTextures[2], tile_draw_size, tile_draw_size, 1.0f, wx, wy);
			}
			else if (tile != 0 && TileTextures[tile])
				render_editor_tiles(TileTextures[tile], tile_draw_size, tile_draw_size, 1.0f, wx, wy);

			Puzzle::PuzzleType ptype = puzzle_editor_map[r][c];
			if (ptype != Puzzle::PUZZLE_TYPE_COUNT && Puzzle::puzzle_block_textures[ptype]) {

				//When editing a button:
				//Highlight other buttons and spikes that are paired together
				Puzzle::puzzle* curr = get_editing_puzzle(r, c, puzzles);

				bool highlight = editing &&
					editing_puzzle->type == Puzzle::BUTTON_UP &&
					(ptype == Puzzle::SPIKE || ptype == Puzzle::BUTTON_UP) &&
					(spike_is_paired(r, c, button_triggers[editing_puzzle->button_id]) ||
					(curr && curr->button_id == editing_puzzle->button_id));

				render_editor_tiles(Puzzle::puzzle_block_textures[ptype],
					tile_draw_size, tile_draw_size, 1.0f, wx, wy, highlight);
			}

		}

	}
	for (int r = 0; r < TILE_ROWS; ++r)
	for (int r = 0; r < TILE_ROWS; ++r)
	{
		for (int c = 0; c < TILE_COLS; ++c)
		{
			f32 wx = grid_start_x + c * tile_draw_size + (tile_draw_size * 0.5f);
			f32 wy = grid_start_y - r * tile_draw_size - (tile_draw_size * 0.5f);

			if (wx < grid_start_x || wx > grid_start_x + TILE_COLS * tile_draw_size) continue;
			if (wy > grid_start_y || wy < grid_start_y - TILE_ROWS * tile_draw_size) continue;

			int tile = editor_map[r][c];
			if (tile == 5 && tomb)
			{
				render_editor_tiles(tomb, tile_draw_size, tile_draw_size, 1.0f, wx, wy);
				if (ghost_spawn)
					render_editor_tiles(ghost_spawn, tile_draw_size, tile_draw_size, 1.0f, wx, wy - tile_draw_size);
			}
			else if (tile == 6 && bat_spawn)
				render_editor_tiles(bat_spawn, tile_draw_size, tile_draw_size, 1.0f, wx, wy);
		}
	}


	// Hover highlight
	{
		f32 wx_m, wy_m;
		mouse_to_world(wx_m, wy_m);
		int row, col;
		if (editor_screen_to_tile(wx_m, wy_m, row, col))
		{
			f32 cx = grid_start_x + col * tile_draw_size + tile_draw_size * 0.5f;
			f32 cy = grid_start_y - row * tile_draw_size - tile_draw_size * 0.5f;

			// FIND:
			if (erase_tile && cx >= -800.f && cx <= 560.f && cy >= -450.f && cy <= 450.f)
				render_editor_tiles(erase_tile, tile_draw_size, tile_draw_size, 0.3f, cx, cy);

			// REPLACE WITH:
			if (editorState == EDIT_TILES) {
				if (selected_tile == 0)
				{
					if (erase_tile && cx >= -800.f && cx <= 560.f && cy >= -450.f && cy <= 450.f)
						render_editor_tiles(erase_tile, tile_draw_size, tile_draw_size, 0.3f, cx, cy);
				}
				else if (selected_tile == 5)
				{
					if (tomb && cx >= -800.f && cx <= 560.f && cy >= -450.f && cy <= 450.f)
					{
						render_editor_tiles(tomb, tile_draw_size, tile_draw_size, 0.5f, cx, cy);
						if (ghost_spawn)
							render_editor_tiles(ghost_spawn, tile_draw_size, tile_draw_size, 0.5f, cx, cy - tile_draw_size);
					}
				}
				else if (selected_tile == 6)
				{
					if (bat_spawn && cx >= -800.f && cx <= 560.f && cy >= -450.f && cy <= 450.f)
						render_editor_tiles(bat_spawn, tile_draw_size, tile_draw_size, 0.5f, cx, cy);
				}
				else if (TileTextures[selected_tile] && cx >= -800.f && cx <= 560.f && cy >= -450.f && cy <= 450.f)
				{
					render_editor_tiles(TileTextures[selected_tile], tile_draw_size, tile_draw_size, 0.5f, cx, cy);
				}
			}
			else if (editorState == EDIT_PUZZLES) {

				if (selected_puzzle == Puzzle::PUZZLE_TYPE_COUNT) {

					if (erase_tile && cx >= -800.f && cx <= 560.f && cy >= -450.f && cy <= 450.f)
						render_editor_tiles(erase_tile, tile_draw_size, tile_draw_size, 0.3f, cx, cy);

				}
				else if (Puzzle::puzzle_block_textures[selected_puzzle] &&
					cx >= -800.f && cx <= 560.f && cy >= -450.f && cy <= 450.f) {

					render_editor_tiles(Puzzle::puzzle_block_textures[selected_puzzle],
						tile_draw_size, tile_draw_size, 0.5f, cx, cy);
				}
			}

		}
	}


	if (editing) {
		draw_puzzle_edit_interface();

	}

	// =====================================================
	// Outer border  only draw OUTSIDE the grid area
	// =====================================================
	{

		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToMultiply({ 0.129f,0.129f,0.133f,0.0f });

		draw_rect_center(0.0f, AEGfxGetWinMaxY() - top_border_height / 2,
			static_cast<f32>(AEGfxGetWindowWidth()), top_border_height, false);

		draw_rect_center(0.0f, AEGfxGetWinMinY() + bottom_border_height / 2,
			static_cast<f32>(AEGfxGetWindowWidth()), bottom_border_height, false);

		draw_rect_center(AEGfxGetWinMinX() + left_border_width / 2, -tile_draw_size / 2,
			left_border_width, side_border_height, false);

		draw_rect_center(AEGfxGetWinMaxX() - right_border_width / 2, -tile_draw_size / 2,
			right_border_width, side_border_height, false);
	}

	// =====================================================
	// 5. Palette buttons
	// =====================================================
	if (editorState == EDIT_TILES) {

		for (int i = 0; i < PALETTE_COUNT; ++i)
		{
			f32 py = PALETTE_TOP_Y - i * PAL_GAP;
			bool hovered = IsButtonHovered(PANEL_X, py, PAL_W, PAL_H);
			bool is_selected = (palette_ids[i] == selected_tile);

			bool is_locked = (palette_ids[i] == 3 && wizard_placed) ||
				(palette_ids[i] == 4 && knight_placed) ||
				(palette_ids[i] == 7 && door_placed);

			// Selected glow border
			if (is_selected && TileTextures[2])
				render_editor_ui(TileTextures[2], PAL_W + 8.f, PAL_H + 8.f, 0.5f, PANEL_X, py);


			// Hover highlight
			if (hovered && !is_selected && TileTextures[2])
				render_editor_ui(TileTextures[2], PAL_W, PAL_H, 0.2f, PANEL_X, py);

			// Tile icon dimmed if locked
			f32 icon_alpha = is_locked ? 0.3f : 1.0f;
			if (palette_ids[i] == 0)
			{
				render_editor_ui(erase_tile,
					PAL_H - 8.f, PAL_H - 8.f, icon_alpha,
					PANEL_X - PAL_W * 0.5f + PAL_H * 0.5f, py);
			}
			else
			{
				int icon_id = palette_ids[i];
				AEGfxTexture* icon_tex = nullptr;
				if (icon_id == 5)       icon_tex = tomb;
				else if (icon_id == 6)  icon_tex = bat_spawn;
				else                    icon_tex = TileTextures[icon_id];
				if (icon_tex)
					render_editor_ui(icon_tex,
						PAL_H - 8.f, PAL_H - 8.f, icon_alpha,
						PANEL_X - PAL_W * 0.5f + PAL_H * 0.5f, py);

			}

			if (font != -1)
			{
				f32 norm_y = (py - 10.f) / 450.f;

				// Label � grey if locked, yellow if selected, white otherwise
				f32 r = is_locked ? 0.4f : (is_selected ? 1.0f : 0.85f);
				f32 g = is_locked ? 0.4f : (is_selected ? 0.9f : 0.85f);
				f32 b = is_locked ? 0.4f : (is_selected ? 0.2f : 0.85f);
				f32 text_wx = PANEL_X - PAL_W * 0.5f + PAL_H + 8.f;
				AEGfxPrint(font, palette_labels[i], text_wx / 800.f, norm_y, 1.f, r, g, b, 1.f);

				// Right side � green "1/1" if locked, grey hotkey if not
				if (is_locked)
				{
					f32 hx = (PANEL_X + PAL_W * 0.5f - 45.f) / 800.f;
					AEGfxPrint(font, "1/1", hx, norm_y, 0.8f, 0.3f, 1.f, 0.3f, 1.f);
				}
				else
				{
					char hint[4];
					snprintf(hint, sizeof(hint), "[%d]", i + 1);
					f32 hx = (PANEL_X + PAL_W * 0.5f - 40.f) / 800.f;
					AEGfxPrint(font, hint, hx, norm_y, 1.f, 0.45f, 0.45f, 0.45f, 1.f);
				}
			}
		}
	}
	else if (editorState == EDIT_PUZZLES) {

		for (int i = 0; i < EDITABLE_PUZZLE_COUNT; ++i)
		{
			f32 py = PALETTE_TOP_Y - i * PAL_GAP;
			bool hovered = IsButtonHovered(PANEL_X, py, PAL_W, PAL_H);
			bool is_selected = (puzzle_palette_ids[i] == selected_puzzle);

			// Selected glow border
			if (is_selected && TileTextures[2])
				render_editor_ui(TileTextures[2], PAL_W + 8.f, PAL_H + 8.f, 0.5f, PANEL_X, py);

			// Hover highlight
			if (hovered && !is_selected && TileTextures[2])
				render_editor_ui(TileTextures[2], PAL_W, PAL_H, 0.2f, PANEL_X, py);

			// Tile icon � dimmed if locked
			f32 icon_alpha = 1.0f;

			if (puzzle_palette_ids[i] == Puzzle::PUZZLE_TYPE_COUNT)
			{
				render_editor_ui(erase_tile,
					PAL_H - 8.f, PAL_H - 8.f, icon_alpha,
					PANEL_X - PAL_W * 0.5f + PAL_H * 0.5f, py);
			}
			else {
				Puzzle::PuzzleType icon_id = puzzle_palette_ids[i];
				if (Puzzle::puzzle_block_textures[icon_id])
					render_editor_ui(Puzzle::puzzle_block_textures[icon_id],
						PAL_H - 8.f, PAL_H - 8.f, icon_alpha,
						PANEL_X - PAL_W * 0.5f + PAL_H * 0.5f, py);
			}


			if (font != -1)
			{
				f32 norm_y = (py - 10.f) / 450.f;

				// Label � grey if locked, yellow if selected, white otherwise
				f32 r = (is_selected ? 1.0f : 0.85f);
				f32 g = (is_selected ? 0.9f : 0.85f);
				f32 b = (is_selected ? 0.2f : 0.85f);
				f32 text_wx = PANEL_X - PAL_W * 0.5f + PAL_H + 8.f;
				AEGfxPrint(font, puzzle_labels[i], text_wx / 800.f, norm_y, 1.f, r, g, b, 1.f);


				char hint[4];
				snprintf(hint, sizeof(hint), "[%d]", i + 1);
				f32 hx = (PANEL_X + PAL_W * 0.5f - 40.f) / 800.f;
				AEGfxPrint(font, hint, hx, norm_y, 1.f, 0.45f, 0.45f, 0.45f, 1.f);

			}
		}
	}


	// =====================================================
	// 6. "TILES" header above palette
	// =====================================================
	if (font != -1)
	{
		const char* tiles_header = "|   TILES   || PUZZLES |";

		f32 tw = 0.f, th = 0.f;
		AEGfxGetPrintSize(font, tiles_header, 1.f, &tw, &th);
		f32 norm_x = (PANEL_X / 800.f) - tw * 0.5f;
		f32 norm_y = (PALETTE_TOP_Y + 45.f) / 450.f;
		AEGfxPrint(font, tiles_header, norm_x, norm_y, 1.f, 0.7f, 0.6f, 0.3f, 1.f);

		render_text_center("________________", font, norm_x + tw * 0.5f,
			norm_y - th * 0.5f, 1.0f, { 0.7f,0.6f,0.3f });
	}


	// =====================================================
	// 6. Instructions at bottom of panel
	// =====================================================
	if (font != -1)
	{
		if (font != -1)
		{
			const char* hint = "LMB : Paint  ---  RMB : Erase  ---  F : Fill  ---  C : Clear  ---  S : Save  ---  ESC : Exit  ---  SCROLL : Zoom --- Arrows : Pan";
			f32 norm_x = (grid_start_x) / 800.f;
			f32 norm_y = -420.f / 450.f;
			AEGfxPrint(font, hint, norm_x, norm_y, 1.f, 0.55f, 0.55f, 0.55f, 1.f);
		}
	}

	// =====================================================
	// 7. Cursor (always on top)
	// =====================================================
	if (cursor)
	{
		f32 world_x = (f32)mouse_x - 800.f;
		f32 world_y = -(f32)mouse_y + 450.f;
		render_editor_ui(cursor, 50.f, 50.f, 1.0f, world_x + 25.f, world_y - 25.f);
	}
	// --- Notification System (Save & Spawn Errors) ---
	if (font != -1)
	{
		const char* active_msg = nullptr;
		f32 active_alpha = 0.0f;
		bool is_error = true;

		// Priority 1: Save Feedback
		if (save_msg_timer > 0.f) {
			active_msg = save_success ? "Map Saved!" :
				(!wizard_placed ? "Place Wizard first!" :
					(!knight_placed ? "Place Knight first!" :
						(!door_placed ? "Place Door first!" : "Save FAILED!")));

			active_alpha = save_msg_timer / SAVE_MSG_DUR;
			is_error = !save_success;
		}
		// Priority 2: Spawn Error (only shows if no save message is ticking)
		else if (spawn_error_timer > 0.f) {
			active_msg = "Cannot place ghost spawns on the last row!";
			active_alpha = spawn_error_timer / SPAWN_ERROR_DUR;
			is_error = true;
		}

		// Draw the active message if there is one
		if (active_msg) {
			f32 r = is_error ? 1.0f : 0.3f;
			f32 g = is_error ? 0.3f : 1.0f;
			f32 b = 0.3f;

			f32 tw = 0.f, th = 0.f;
			AEGfxGetPrintSize(font, active_msg, 1.5f, &tw, &th);

			// Exact same placement as your original Save Feedback
			AEGfxPrint(font, active_msg, -tw * 0.5f, 0.8f, 1.5f, r, g, b, active_alpha);
		}
	}
}

void level_editor_free()
{
	free_mesh(); // Free any dynamically allocated memory for meshes

	//free editor_map data
	if (editor_map)
	{
		for (int r = 0; r < TILE_ROWS; ++r)
			delete[] editor_map[r];

		delete[] editor_map;

		editor_map = nullptr;
	}

	//free puzzle_editor_map data
	if (puzzle_editor_map) {
		for (int r = 0; r < TILE_ROWS; ++r)
			delete[] puzzle_editor_map[r];

		delete[] puzzle_editor_map;

		puzzle_editor_map = nullptr;
	}

}

void level_editor_unload()
{
	free_tile_textures(); // Unload tile textures to free memory
	Puzzle::unload_puzzle_textures();
	if (cursor) { AEGfxTextureUnload(cursor); cursor = nullptr; }
	if (ghost_spawn) { AEGfxTextureUnload(ghost_spawn); ghost_spawn = nullptr; }
	if (tomb) { AEGfxTextureUnload(tomb); tomb = nullptr; }
	if (bat_spawn) { AEGfxTextureUnload(bat_spawn);   bat_spawn = nullptr; }
	if (font != -1) { AEGfxDestroyFont(font); font = -1; }
	if (erase_tile) { AEGfxTextureUnload(erase_tile); erase_tile = nullptr; }
	if (edit_panel) { AEGfxTextureUnload(edit_panel); edit_panel = nullptr; }
	if (edit_panel_save) { AEGfxTextureUnload(edit_panel_save); edit_panel_save = nullptr; }
	if (edit_panel_exit) { AEGfxTextureUnload(edit_panel_exit); edit_panel_exit = nullptr; }
	if (edit_panel_button_left) { AEGfxTextureUnload(edit_panel_button_left); edit_panel_button_left = nullptr; }
	if (edit_panel_button_right) { AEGfxTextureUnload(edit_panel_button_right); edit_panel_button_right = nullptr; }
}