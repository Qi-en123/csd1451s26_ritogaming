/*!************************************************************************
\file:          level_editor.cpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         This file defines functions to load, initialize, update, draw, free, and unload
				the game's level editor which saves the current map array to a text file.
**************************************************************************/

#include "AEEngine.h"
#include "renderer.hpp"
#include "utils.hpp"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "map.hpp"
#include "puzzle.hpp"
#include "level_editor.hpp"

namespace {
	/// Assets
	AEGfxTexture* erase_tile = nullptr;
	AEGfxTexture* cursor = nullptr;
	s32 mouse_x, mouse_y;
	static s8 font = -1;

	// --- Tile Palette ---
	const int PALETTE_COUNT = 7;
	const int palette_ids[PALETTE_COUNT] = { 0, 1, 2, 3, 4, 8 ,9 };
	const char* palette_labels[PALETTE_COUNT] = { "Erase", "Wall", "Floor", "Wizard", "Knight", "Door", "Spikes" };
	int selected_tile = 1; // default to wall

	// --- Puzzle Palette ---
	const int EDITABLE_PUZZLE_COUNT = 4;
	const PuzzleType puzzle_palette_ids[EDITABLE_PUZZLE_COUNT] =
	{ PUZZLE_TYPE_COUNT, PUSHABLE, BUTTON_UP, PIT };
	const char* puzzle_labels[EDITABLE_PUZZLE_COUNT] =
	{ "Erase", "Pushable", "Button", "Pit" };
	PuzzleType selected_puzzle = PUSHABLE; //default

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

	// --- Save feedback ---
	bool save_success = false;
	f32  save_msg_timer = 0.f;
	const f32 SAVE_MSG_DUR = 2.f;

	// --- Camera pan ---
	f32 cam_offset_x = 0.f;
	f32 cam_offset_y = 0.f;
	const f32 CAM_SPEED = 300.f;

	/// --- Painting control ---
	static bool can_paint = false;

	s32** editor_map = nullptr; // for creating new map
	PuzzleType** puzzle_editor_map = nullptr;

	EditorState editorState = EDIT_TILES;
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
// ------------------------------------------------
static bool world_to_tile(f32 wx, f32 wy, int& row, int& col)
{
	col = (int)((wx - (grid_start_x + cam_offset_x)) / tile_draw_size); // convert world X to column index
	row = (int)(((grid_start_y + cam_offset_y) - wy) / tile_draw_size); // convert world Y to row index (inverted because screen Y is downwards)
	return (row >= 0 && row < TILE_ROWS && col >= 0 && col < TILE_COLS);
}










void level_editor_load()
{

	// Load tile textures once and reuse for palette icons and grid rendering
	load_tile_textures("Assets/Tiles");
	cursor = AEGfxTextureLoad("Assets/Admin/cursorSword_silver.png");
	font = AEGfxCreateFont("Assets/Exo2-Regular.ttf", 22);
	erase_tile = AEGfxTextureLoad("Assets/Tiles/tile_0010.png");

	load_puzzle_textures();
}

void level_editor_init()
{
	zoom = 1.1f;
	f32 camx{}, camy{};
	cam = init_cam(&camx, &camy, 60.0f, 5.0f);

	init_mesh(); // ensure mesh is initialized before any rendering calls
	AEGfxSetBackgroundColor(0.08f, 0.09f, 0.12f);
	AEInputShowCursor(false); // we'll draw our own cursor, so hide the default one

	//Default init editor map (FOR TILES)
	editor_map = new int* [TILE_ROWS];
	for (int r = 0; r < TILE_ROWS; ++r)
	{
		editor_map[r] = new int[TILE_COLS] {};  // {} zero-initializes
	}

	// set default map data (all 0)
	for (int r = 0; r < TILE_ROWS; ++r)
		for (int c = 0; c < TILE_COLS; ++c)
			editor_map[r][c] = 0;

	//Default init puzzle editor map (FOR PUZZLES)
	puzzle_editor_map = new PuzzleType * [TILE_ROWS];
	for (int r = 0; r < TILE_ROWS; ++r)
	{
		puzzle_editor_map[r] = new PuzzleType[TILE_COLS]{ PUZZLE_TYPE_COUNT };  // {} zero-initializes
	}
	for (int r = 0; r < TILE_ROWS; ++r)
		for (int c = 0; c < TILE_COLS; ++c)
			puzzle_editor_map[r][c] = PUZZLE_TYPE_COUNT;

	// Fit tile size so entire map fits in visible area
	const f32 SCREEN_W = (f32)AEGfxGetWindowWidth();
	const f32 SCREEN_H = (f32)AEGfxGetWindowHeight();
	const f32 PANEL_WIDTH = 240.f; // reserve space on right for palette

	// Tile size to be drawn on the grid
	tile_draw_size = 20.f;

	// Calculate grid starting position to center it in the remaining space
	f32 grid_w = tile_draw_size * TILE_COLS; // total width of the grid in world units
	f32 grid_h = tile_draw_size * TILE_ROWS; // total height of the grid in world units

	grid_start_x = -750.f;  // flush to left edge of world
	grid_start_y = 350.f;   // centered vertically
	cam_offset_x = 0.f;
	cam_offset_y = 0.f;
	selected_tile = 1;
	save_success = false;
	save_msg_timer = 0.f;

	wizard_placed = false;
	knight_placed = false;
	door_placed = false;



	cam_offset_x = 0.f;
	cam_offset_y = 0.f;
	can_paint = false;

	editorState = EDIT_TILES;
}

void level_editor_update()
{
	dt = (f32)AEFrameRateControllerGetFrameTime();
	get_zoom(dt);
	AEInputGetCursorPosition(&mouse_x, &mouse_y);


	// --- ESC to exit ---
	if (AEInputCheckTriggered(AEVK_ESCAPE))
		next = MAINMENU; // adjust to your menu state

	// --- Save with S key ---
	if (AEInputCheckTriggered(AEVK_S))
	{
		if (wizard_placed && knight_placed && door_placed)
		{
			save_success = save_map_to_file(editor_map);
		}
		else
		{
			save_success = false; // show fail message
		}
		save_msg_timer = SAVE_MSG_DUR;
	}


	// --- Tick save message ---
	if (save_msg_timer > 0.f)
		save_msg_timer -= dt;

	//Switch editing state
	if (AEInputCheckTriggered(AEVK_LBUTTON)) {
		if (IsButtonClicked(PANEL_X + PAL_W / 4, PALETTE_TOP_Y + PAL_H, PAL_W / 2, PAL_H)) {
			editorState = EDIT_PUZZLES;
		}
		if (IsButtonClicked(PANEL_X - PAL_W / 4, PALETTE_TOP_Y + PAL_H, PAL_W / 2, PAL_H)) {
			editorState = EDIT_TILES;
		}
	}

	if (editorState == EDIT_TILES) {

		// --- Palette hotkeys 1-8 ---
		const int hotkeys[8] = { AEVK_1, AEVK_2, AEVK_3, AEVK_4, AEVK_5, AEVK_6, AEVK_7, AEVK_8 };
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
		bool on_grid = world_to_tile(wx, wy, row, col);
		// Prevent painting when mouse is over the right panel
		f32 grid_right_world = grid_start_x + cam_offset_x + (TILE_COLS * tile_draw_size);
		f32 grid_right_screen = grid_right_world + 800.f;
		bool on_panel = ((f32)mouse_x > grid_right_screen);
		// Allow painting only after mouse is released once inside editor
		if (AEInputCheckReleased(AEVK_LBUTTON))
			can_paint = true;

		if (on_grid && !on_panel)
		{
			// LEFT CLICK PAINT
			if (can_paint && AEInputCheckCurr(AEVK_LBUTTON))
			{
				int current = editor_map[row][col];
				int placing = selected_tile;

				bool blocked =
					(placing == 3 && wizard_placed && current != 3) ||
					(placing == 4 && knight_placed && current != 4) ||
					(placing == 8 && door_placed && current != 8);

				if (!blocked)
				{
					if (current == 3) wizard_placed = false;
					if (current == 4) knight_placed = false;
					if (current == 8) door_placed = false;

					editor_map[row][col] = placing;

					if (placing == 3) wizard_placed = true;
					if (placing == 4) knight_placed = true;
					if (placing == 8) door_placed = true;
				}
			}

			// RIGHT CLICK ERASE
			if (can_paint && AEInputCheckCurr(AEVK_RBUTTON))
			{
				int current = editor_map[row][col];

				if (current == 3) wizard_placed = false;
				if (current == 4) knight_placed = false;
				if (current == 8) door_placed = false;

				editor_map[row][col] = 0;
			}
		}

		// --- Fill entire grid with selected tile (F key) ---
		if (AEInputCheckTriggered(AEVK_F))
		{
			// Block fill entirely for unique tiles
			// Block fill entirely for unique tiles
			if (selected_tile == 3 || selected_tile == 4 || selected_tile == 8)
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

	else if (editorState == EDIT_PUZZLES) {
		// --- Puzzle palette hotkeys 1-3 ---
		const int hotkeys[EDITABLE_PUZZLE_COUNT] = { AEVK_1, AEVK_2, AEVK_3, AEVK_4 };
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
					selected_puzzle = puzzle_palette_ids[i];
					return; // stop here, don't also paint on grid
				}
			}
		}

		// --- Paint / Erase on grid ---
		f32 wx, wy;
		mouse_to_world(wx, wy);
		int row, col;
		bool on_grid = world_to_tile(wx, wy, row, col);
		// Prevent painting when mouse is over the right panel
		f32 grid_right_world = grid_start_x + cam_offset_x + (TILE_COLS * tile_draw_size);
		f32 grid_right_screen = grid_right_world + 800.f;
		bool on_panel = ((f32)mouse_x > grid_right_screen);
		// Allow painting only after mouse is released once inside editor
		if (AEInputCheckReleased(AEVK_LBUTTON))
			can_paint = true;

		if (on_grid && !on_panel)
		{
			// LEFT CLICK PAINT
			if (can_paint && AEInputCheckCurr(AEVK_LBUTTON))
			{
				int current = puzzle_editor_map[row][col];
				PuzzleType placing = selected_puzzle;

				puzzle_editor_map[row][col] = placing;
			}

			// RIGHT CLICK ERASE
			if (can_paint && AEInputCheckCurr(AEVK_RBUTTON))
			{
				int current = puzzle_editor_map[row][col];

				puzzle_editor_map[row][col] = PUZZLE_TYPE_COUNT;
			}
		}

		// --- Clear entire grid (C key) ---
		if (AEInputCheckTriggered(AEVK_C))
		{
			for (int r = 0; r < TILE_ROWS; ++r)
				for (int c = 0; c < TILE_COLS; ++c)
					puzzle_editor_map[r][c] = PUZZLE_TYPE_COUNT;
		}
	}

}

void level_editor_draw()
{
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	f32 grid_right = grid_start_x + TILE_COLS * tile_draw_size;

	// =====================================================
	// 0. Outer border — only draw OUTSIDE the grid area
	 // =====================================================
	//for (f32 x = -800.f; x <= 800.f; x += tile_draw_size*0.5f)
	//{
	//    for (f32 y = -450.f; y <= 450.f; y += tile_draw_size*0.5f)
	//    {
	//        f32 cx = x + tile_draw_size * 0.5f;
	//        f32 cy = y + tile_draw_size * 0.5f;

	//        bool inside_grid = (cx >= grid_start_x &&
	//            cx <= grid_start_x + TILE_COLS * tile_draw_size &&
	//            cy <= grid_start_y &&
	//            cy >= grid_start_y - TILE_ROWS * tile_draw_size);
	//        if (inside_grid) continue;

	//        if (TileTextures[0])
	//            render_blending_sprite(TileTextures[0], tile_draw_size, tile_draw_size, 1.0f, cx, cy);
	//    }
	//}

	// Draw black tile as grid background for ALL cells
	if (erase_tile)
	{
		for (int r = 0; r < TILE_ROWS; ++r)
		{
			for (int c = 0; c < TILE_COLS; ++c)
			{
				f32 wx = grid_start_x + c * tile_draw_size + tile_draw_size * 0.5f + cam_offset_x;
				f32 wy = grid_start_y - r * tile_draw_size - tile_draw_size * 0.5f + cam_offset_y;
				render_blending_sprite(erase_tile, tile_draw_size, tile_draw_size, 1.0f, wx, wy);
			}
		}
	}

	// Draw actual map tiles on top
	for (int r = 0; r < TILE_ROWS; ++r)
	{
		for (int c = 0; c < TILE_COLS; ++c)
		{
			f32 wx = grid_start_x + c * tile_draw_size + (tile_draw_size * 0.5f) + cam_offset_x;
			f32 wy = grid_start_y - r * tile_draw_size - (tile_draw_size * 0.5f) + cam_offset_y;
			// to optimize, skip drawing tiles that are outside the visible world bounds (including right panel area)
			f32 grid_left = grid_start_x + cam_offset_x;
			f32 grid_right = grid_left + TILE_COLS * tile_draw_size;
			f32 grid_top = grid_start_y + cam_offset_y;
			f32 grid_bot = grid_top - TILE_ROWS * tile_draw_size;

			if (wx < grid_left || wx > grid_right) continue;
			if (wy > grid_top || wy < grid_bot) continue;

			int tile = editor_map[r][c];
			if (tile != 0 && TileTextures[tile])
				render_blending_sprite(TileTextures[tile], tile_draw_size, tile_draw_size, 1.0f, wx, wy);

			PuzzleType puzzle = puzzle_editor_map[r][c];
			if (puzzle != PUZZLE_TYPE_COUNT && puzzle_block_textures[puzzle]) {
				render_blending_sprite(puzzle_block_textures[puzzle], tile_draw_size, tile_draw_size, 1.0f, wx, wy);
			}
		}
	}

	// Hover highlight
	{
		f32 wx_m, wy_m;
		mouse_to_world(wx_m, wy_m);
		int row, col;
		if (world_to_tile(wx_m, wy_m, row, col))
		{
			f32 cx = grid_start_x + col * tile_draw_size + tile_draw_size * 0.5f + cam_offset_x;
			f32 cy = grid_start_y - row * tile_draw_size - tile_draw_size * 0.5f + cam_offset_y;


			// FIND:
			if (erase_tile && cx >= -800.f && cx <= 560.f && cy >= -450.f && cy <= 450.f)
				render_blending_sprite(erase_tile, tile_draw_size, tile_draw_size, 0.3f, cx, cy);

			// REPLACE WITH:
			if (editorState == EDIT_TILES) {
				if (selected_tile == 0)
				{
					// Erase — show red tint using floor tile
					if (erase_tile && cx >= -800.f && cx <= 560.f && cy >= -450.f && cy <= 450.f)
						render_blending_sprite(erase_tile, tile_draw_size, tile_draw_size, 0.3f, cx, cy);
				}
				else if (TileTextures[selected_tile] && cx >= -800.f && cx <= 560.f && cy >= -450.f && cy <= 450.f)
				{
					render_blending_sprite(TileTextures[selected_tile], tile_draw_size, tile_draw_size, 0.5f, cx, cy);
				}
			}
			else if (editorState == EDIT_PUZZLES) {
				if (selected_tile == PUZZLE_TYPE_COUNT) {
					return;
				}
				else if (puzzle_block_textures[selected_puzzle] &&
					cx >= -800.f && cx <= 560.f && cy >= -450.f && cy <= 450.f) {
					render_blending_sprite(puzzle_block_textures[selected_puzzle],
						tile_draw_size, tile_draw_size, 0.5f, cx, cy);
				}
			}

		}
	}



	if (editorState == EDIT_TILES) {
		// =====================================================
		// 5. Palette buttons
		// =====================================================
		for (int i = 0; i < PALETTE_COUNT; ++i)
		{
			f32 py = PALETTE_TOP_Y - i * PAL_GAP;
			bool hovered = IsButtonHovered(PANEL_X, py, PAL_W, PAL_H);
			bool is_selected = (palette_ids[i] == selected_tile);

			bool is_locked = (palette_ids[i] == 3 && wizard_placed) ||
				(palette_ids[i] == 4 && knight_placed) ||
				(palette_ids[i] == 8 && door_placed);

			// Selected glow border
			if (is_selected && TileTextures[2])
				render_blending_sprite(TileTextures[2], PAL_W + 8.f, PAL_H + 8.f, 0.5f, PANEL_X, py);


			// Hover highlight
			if (hovered && !is_selected && TileTextures[2])
				render_blending_sprite(TileTextures[2], PAL_W, PAL_H, 0.2f, PANEL_X, py);

			// Tile icon — dimmed if locked
			f32 icon_alpha = is_locked ? 0.3f : 1.0f;
			if (palette_ids[i] == 0)
			{
				render_blending_sprite(erase_tile,
					PAL_H - 8.f, PAL_H - 8.f, icon_alpha,
					PANEL_X - PAL_W * 0.5f + PAL_H * 0.5f, py);
			}
			else
			{
				int icon_id = palette_ids[i];
				if (TileTextures[icon_id])
					render_blending_sprite(TileTextures[icon_id],
						PAL_H - 8.f, PAL_H - 8.f, icon_alpha,
						PANEL_X - PAL_W * 0.5f + PAL_H * 0.5f, py);
			}

			if (font != -1)
			{
				f32 norm_y = (py - 10.f) / 450.f;

				// Label — grey if locked, yellow if selected, white otherwise
				f32 r = is_locked ? 0.4f : (is_selected ? 1.0f : 0.85f);
				f32 g = is_locked ? 0.4f : (is_selected ? 0.9f : 0.85f);
				f32 b = is_locked ? 0.4f : (is_selected ? 0.2f : 0.85f);
				f32 text_wx = PANEL_X - PAL_W * 0.5f + PAL_H + 8.f;
				AEGfxPrint(font, palette_labels[i], text_wx / 800.f, norm_y, 1.f, r, g, b, 1.f);

				// Right side — green "1/1" if locked, grey hotkey if not
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
		// =====================================================
		// 5. Palette buttons
		// =====================================================
		for (int i = 0; i < EDITABLE_PUZZLE_COUNT; ++i)
		{
			f32 py = PALETTE_TOP_Y - i * PAL_GAP;
			bool hovered = IsButtonHovered(PANEL_X, py, PAL_W, PAL_H);
			bool is_selected = (puzzle_palette_ids[i] == selected_puzzle);

			// Selected glow border
			if (is_selected && TileTextures[2])
				render_blending_sprite(TileTextures[2], PAL_W + 8.f, PAL_H + 8.f, 0.5f, PANEL_X, py);

			// Hover highlight
			if (hovered && !is_selected && TileTextures[2])
				render_blending_sprite(TileTextures[2], PAL_W, PAL_H, 0.2f, PANEL_X, py);

			// Tile icon — dimmed if locked
			f32 icon_alpha = 1.0f;

			if (puzzle_palette_ids[i] == PUZZLE_TYPE_COUNT)
			{
				render_blending_sprite(erase_tile,
					PAL_H - 8.f, PAL_H - 8.f, icon_alpha,
					PANEL_X - PAL_W * 0.5f + PAL_H * 0.5f, py);
			}
			else {
				PuzzleType icon_id = puzzle_palette_ids[i];
				if (puzzle_block_textures[icon_id])
					render_blending_sprite(puzzle_block_textures[icon_id],
						PAL_H - 8.f, PAL_H - 8.f, icon_alpha,
						PANEL_X - PAL_W * 0.5f + PAL_H * 0.5f, py);
			}


			if (font != -1)
			{
				f32 norm_y = (py - 10.f) / 450.f;

				// Label — grey if locked, yellow if selected, white otherwise
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

		render_text_center("__________________", font, norm_x + tw * 0.5f,
			norm_y - th * 0.5f, 1.0f, { 0.7f,0.6f,0.3f });
	}


	// =====================================================
	// 6. Instructions at bottom of panel
	// =====================================================
	if (font != -1)
	{
		const char* instructions[] = {
			"LMB: Paint",
			"RMB: Erase",
			"F: Fill all",
			"C: Clear all",
			"S: Save",
			"ESC: Exit"
		};
		for (int i = 0; i < 6; ++i)
		{
			f32 iy = -195.f - i * 40.f;
			f32 norm_x = (PANEL_X - PAL_W * 0.5f + 5.f) / 800.f;
			f32 norm_y = iy / 450.f;
			AEGfxPrint(font, instructions[i], norm_x, norm_y, 1.f, 0.55f, 0.55f, 0.55f, 1.f);
		}
	}

	// =====================================================
	// 7. Cursor (always on top)
	// =====================================================
	if (cursor)
	{
		f32 world_x = (f32)mouse_x - 800.f;
		f32 world_y = -(f32)mouse_y + 450.f;
		render_blending_sprite(cursor, 50.f, 50.f, 1.0f, world_x + 25.f, world_y - 25.f);
	}
	// Save feedback — centered on screen, fades after 2 seconds
	if (save_msg_timer > 0.f && font != -1)
	{
		const char* msg = save_success ? "Map Saved!" :
			(!wizard_placed ? "Place Wizard first!" :
				(!knight_placed ? "Place Knight first!" :
					(!door_placed ? "Place Door first!" : "Save FAILED!")));
		f32 r = save_success ? 0.3f : 1.0f;
		f32 g = save_success ? 1.0f : 0.3f;
		f32 b = 0.3f;
		f32 alpha = save_msg_timer / SAVE_MSG_DUR; // fades out over time

		f32 tw = 0.f, th = 0.f;
		AEGfxGetPrintSize(font, msg, 1.5f, &tw, &th);
		f32 norm_x = -tw * 0.5f;   // centered horizontally
		f32 norm_y = 0.8f;          // near top of screen

		AEGfxPrint(font, msg, norm_x, norm_y, 1.5f, r, g, b, alpha);
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
	unload_puzzle_textures();
	if (cursor) { AEGfxTextureUnload(cursor); cursor = nullptr; }
	if (font != -1) { AEGfxDestroyFont(font); font = -1; }
	if (erase_tile) { AEGfxTextureUnload(erase_tile); erase_tile = nullptr; }
}