/*!************************************************************************
\file:                          level_editor.cpp
\author:                        Goh Qi En
\par DP email:                  g.qien@digipen.edu
\co-author:                     Jerod Lim
\par co-author DP email:        jerodyaowei.lim@digipen.edu

\brief:         This file defines functions to load, initialize, update,
                draw, free, and unload the game's level interface which
                allows users to create custom maps by painting tiles on a
                grid and saving them to a file. The editor features a tile
                palette, camera panning, and visual feedback for tile
                placement and saving status.
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
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "enemy_manager.hpp"
#include "audio_manager.hpp"

namespace {

    // ----------------------------------------------------------------------------
    // Config parser
    // Reads a key-value .txt file into a map. Lines starting with # are comments.
    // Values may contain spaces (e.g. font paths, hint strings)
    // ----------------------------------------------------------------------------
    std::unordered_map<std::string, std::string> parse_kv_file(const std::string& filepath)
    {
        std::unordered_map<std::string, std::string> kv;
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            return kv;
        }
        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream iss(line);
            std::string key;
            if (!(iss >> key)) continue;
            std::string value;
            if (std::getline(iss >> std::ws, value))
                kv[key] = value;
        }
        return kv;
    }
    // Helper to get float from map with fallback
    f32 kv_float(const std::unordered_map<std::string, std::string>& kv,
        const std::string& key, f32 fallback = 0.f)
    {
        auto it = kv.find(key);
        return (it != kv.end()) ? std::stof(it->second) : fallback;
    }
    // Helper to get int from map with fallback
    s32 kv_int(const std::unordered_map<std::string, std::string>& kv,
        const std::string& key, s32 fallback = 0)
    {
        auto it = kv.find(key);
        return (it != kv.end()) ? std::stoi(it->second) : fallback;
    }
    // Helper to get string from map with fallback
    std::string kv_str(const std::unordered_map<std::string, std::string>& kv,
        const std::string& key, const std::string& fallback = "")
    {
        auto it = kv.find(key);
        return (it != kv.end()) ? it->second : fallback;
    }

#pragma region Static variables

    // --- Textures ---
    AEGfxTexture* erase_tile = nullptr;
    AEGfxTexture* cursor = nullptr;
    AEGfxTexture* ghost_spawn = nullptr;
    AEGfxTexture* tomb = nullptr;
    AEGfxTexture* bat_spawn = nullptr;
    AEGfxTexture* confirm_panel = nullptr;
    AEGfxTexture* confirm_btn_yes = nullptr;
    AEGfxTexture* confirm_btn_no = nullptr;
    AEGfxTexture* edit_panel = nullptr;
    AEGfxTexture* edit_panel_save = nullptr;
    AEGfxTexture* edit_panel_exit = nullptr;
    AEGfxTexture* edit_panel_button_left = nullptr;
    AEGfxTexture* edit_panel_button_right = nullptr;

    s32 mouse_x, mouse_y;
    s8  level_editor_font = -1;

    // --- Cursor ---
    f32 cursor_w = 0.f;
    f32 cursor_h = 0.f;
    f32 cursor_offset_x = 0.f;
    f32 cursor_offset_y = 0.f;

    // --- Palette UI ---
    f32 PANEL_X = 0.f;
    f32 PALETTE_TOP_Y = 0.f;
    f32 PAL_W = 0.f;
    f32 PAL_H = 0.f;
    f32 PAL_GAP = 0.f;

    // --- Edit Panel ---
    f32 edit_panel_w = 0.f;
    f32 edit_panel_h = 0.f;
    f32 edit_panel_x = 0.f;
    f32 edit_panel_y = 0.f;
    f32 ep_button_x = 0.f;

    // --- Camera ---
    f32 CAM_SPEED = 0.f;
    f32 editor_zoom_min = 0.f;
    f32 editor_zoom_max = 0.f;
    f32 cam_x = 0.f;
    f32 cam_y = 0.f;

    // --- Grid ---
    f32 tile_draw_size = 0.f;
    f32 grid_start_x = 0.f;
    f32 grid_start_y = 0.f;

    // --- Borders ---
    f32 top_border_tile_mult = 0.f;
    f32 bottom_border_tile_mult = 0.f;
    f32 left_border_tile_mult = 0.f;
    f32 right_border_tile_mult = 0.f;
    f32 top_border_height = 0.f;
    f32 bottom_border_height = 0.f;
    f32 side_border_height = 0.f;
    f32 left_border_width = 0.f;
    f32 right_border_width = 0.f;

    // --- Camera limits ---
    f32 view_min_x = 0.f;
    f32 view_max_x = 0.f;
    f32 view_min_y = 0.f;
    f32 view_max_y = 0.f;
    f32 view_w = 0.f;
    f32 view_h = 0.f;

    // --- Background ---
    f32 bg_color_r = 0.f;
    f32 bg_color_g = 0.f;
    f32 bg_color_b = 0.f;

    // --- Save / Spawn error ---
    f32  SAVE_MSG_DUR = 0.f;
    f32  SPAWN_ERROR_DUR = 0.f;
    bool save_success = false;
    f32  save_msg_timer = 0.f;
    f32  spawn_error_timer = 0.f;

    // --- Hover / Preview ---
    f32 hover_alpha = 0.f;
    f32 ghost_preview_alpha = 0.f;
    f32 palette_icon_padding = 0.f;

    // --- Text ---
    f32 text_scale_edit_panel = 0.f;
    std::string tiles_header_label;
    f32 tiles_header_color_r = 0.f;
    f32 tiles_header_color_g = 0.f;
    f32 tiles_header_color_b = 0.f;

    // --- Hint bar ---
    f32 hint_bar_norm_y = 0.f;
    f32 hint_bar_color_r = 0.f;
    f32 hint_bar_color_g = 0.f;
    f32 hint_bar_color_b = 0.f;
    std::string hint_tiles;
    std::string hint_puzzles;

    // --- Palette label colors ---
    f32 label_selected_r = 0.f, label_selected_g = 0.f, label_selected_b = 0.f;
    f32 label_normal_r = 0.f, label_normal_g = 0.f, label_normal_b = 0.f;
    f32 label_locked_r = 0.f, label_locked_g = 0.f, label_locked_b = 0.f;
    f32 label_hotkey_r = 0.f, label_hotkey_g = 0.f, label_hotkey_b = 0.f;
    f32 label_locked_count_r = 0.f, label_locked_count_g = 0.f, label_locked_count_b = 0.f;

    // --- Confirm popup ---
    std::string confirm_label;
    f32 confirm_text_scale = 0.f;
    f32 confirm_yes_x = 0.f, confirm_yes_y = 0.f, confirm_yes_w = 0.f, confirm_yes_h = 0.f;
    f32 confirm_no_x = 0.f, confirm_no_y = 0.f, confirm_no_w = 0.f, confirm_no_h = 0.f;

    // --- Status message ---
    f32 status_msg_norm_y = 0.f;
    f32 status_msg_scale = 0.f;
    f32 status_success_r = 0.f, status_success_g = 0.f, status_success_b = 0.f;
    f32 status_error_r = 0.f, status_error_g = 0.f, status_error_b = 0.f;

    // --- Grid screen clamp ---
    f32 grid_screen_left = 0.f;
    f32 grid_screen_right = 0.f;
    f32 grid_screen_top = 0.f;
    f32 grid_screen_bottom = 0.f;

    // --- World conversion offsets ---
    f32 world_x_offset = 0.f;
    f32 world_y_offset = 0.f;

    // --- Tile palettes ---
    const s32 PALETTE_COUNT = 8;
    const s32 palette_ids[PALETTE_COUNT] = { 0, 1, 2, 3, 4, 7, 5, 6 };
    const char* palette_labels[PALETTE_COUNT] =
    { "Erase", "Wall", "Floor", "Wizard", "Knight", "Door", "Ghost", "Bat" };
    s32 selected_tile = 1;

    // --- Puzzle palettes ---
    const s32 EDITABLE_PUZZLE_COUNT = 5;
    const Puzzle::PuzzleType puzzle_palette_ids[EDITABLE_PUZZLE_COUNT] =
    {
        Puzzle::PUZZLE_TYPE_COUNT,
        Puzzle::PUSHABLE,
        Puzzle::BUTTON_UP,
        Puzzle::PIT,
        Puzzle::SPIKE
    };
    const char* puzzle_labels[EDITABLE_PUZZLE_COUNT] =
    { "Erase", "Grave", "Button", "Pit", "Spike" };
    Puzzle::PuzzleType selected_puzzle = Puzzle::PUSHABLE;

    // --- One-time placement limits ---
    bool wizard_placed = false;
    bool knight_placed = false;
    bool door_placed = false;

    // --- Notification ---
    const char* active_status_msg = "";
    bool        active_status_error = false;

    // --- Painting control ---
    bool can_paint = false;

    // --- Map data ---
    s32** editor_map = nullptr;
    Puzzle::PuzzleType** puzzle_editor_map = nullptr;
    std::vector<Puzzle::puzzle>          puzzles;
    std::vector<Puzzle::buttonTrigger>   button_triggers{};

    // --- Editing States ---
    EditorState     editorState = EDIT_TILES;
    bool            editing = false;
    Puzzle::puzzle* editing_puzzle = nullptr;
    Puzzle::puzzle  old_puzzle{};

    // --- Exit confirm ---
    bool show_exit_confirm = false;

#pragma endregion

#pragma region Helper functions

    // ----------------------------------------------------------------------------
        //
        // Function: mouse_to_world
        // Converts the current mouse screen position into raw world-space coordinates.
        //
        // ----------------------------------------------------------------------------
    void mouse_to_world(f32& wx, f32& wy)
    {
        // Add screen X to the world's left bound to get world X
        wx = (f32)mouse_x + world_left;

        // Invert screen Y and add the world's top bound to get world Y
        wy = -(f32)mouse_y + world_top;
    }

    // ----------------------------------------------------------------------------
      //
      // Function: editor_screen_to_tile
      // Converts screen-space coordinates to grid-based tile indices (row/column).
      // Returns true if the resulting coordinates fall within the valid map bounds.
      //
      // ----------------------------------------------------------------------------
    bool editor_screen_to_tile(f32 wx, f32 wy, int& row, int& col)
    {
        // Adjust raw input by camera zoom and position to find true world coordinates
        f32 world_x = (wx / Camera::zoom) + Camera::cam.pos.x;
        f32 world_y = (wy / Camera::zoom) + Camera::cam.pos.y;

        // Calculate column and row by finding distance from grid start and dividing by tile size
        col = static_cast<int>((world_x - grid_start_x) / tile_draw_size);
        row = static_cast<int>((grid_start_y - world_y) / tile_draw_size);

        // Validation check against defined map dimensions
        return (row >= 0 && row < TILE_ROWS && col >= 0 && col < TILE_COLS);
    }

    // ----------------------------------------------------------------------------
    //
    // Function: simulate_editor_cam
    // Handles editor camera movement via arrow keys and enforces boundary clamping
    // to keep the view within the playable/editable area.
    //
    // ----------------------------------------------------------------------------
    void simulate_editor_cam()
    {
        // Update camera position based on keyboard input and frame delta time
        if (AEInputCheckCurr(AEVK_LEFT))  Camera::cam.pos.x -= dt * CAM_SPEED;
        if (AEInputCheckCurr(AEVK_RIGHT)) Camera::cam.pos.x += dt * CAM_SPEED;
        if (AEInputCheckCurr(AEVK_UP))    Camera::cam.pos.y += dt * CAM_SPEED;
        if (AEInputCheckCurr(AEVK_DOWN))  Camera::cam.pos.y -= dt * CAM_SPEED;

        // Calculate dynamic camera limits based on grid size and current zoom level
        f32 cam_min_x = grid_start_x - view_min_x / Camera::zoom;
        f32 cam_max_x = grid_start_x + (TILE_COLS * tile_draw_size) - (view_min_x + view_w) / Camera::zoom;
        f32 cam_min_y = grid_start_y - (TILE_ROWS * tile_draw_size) - (view_max_y - view_h) / Camera::zoom;
        f32 cam_max_y = grid_start_y - view_max_y / Camera::zoom;

        // Apply clamping to ensure the camera never wanders out of bounds
        Camera::cam.pos.x = AEClamp(Camera::cam.pos.x, cam_min_x, cam_max_x);
        Camera::cam.pos.y = AEClamp(Camera::cam.pos.y, cam_min_y, cam_max_y);
    }

    // ----------------------------------------------------------------------------
    //
    // Function: draw_puzzle_edit_interface
    // Renders the graphical user interface for modifying puzzle-specific properties
    // like elemental types and character-specific visibility.
    //
    // ----------------------------------------------------------------------------
    void draw_puzzle_edit_interface()
    {
        // Set up anchoring for the UI panel and side buttons
        edit_panel_x = grid_start_x + edit_panel_w / 2.f + 20.f;
        edit_panel_y = edit_panel_h / 2.f - grid_start_y;
        ep_button_x = edit_panel_x + (edit_panel_w / 2.f) + (edit_panel_h / 4.f) - 5.f;

        // Render the primary control buttons (Save and Exit/Cancel)
        render_editor_ui(edit_panel_save, edit_panel_h / 2.f, edit_panel_h / 2.f, 1.f,
            ep_button_x, edit_panel_y + edit_panel_h / 4.f);
        render_editor_ui(edit_panel_exit, edit_panel_h / 2.f, edit_panel_h / 2.f, 1.f,
            ep_button_x, edit_panel_y - edit_panel_h / 4.f);
        render_editor_ui(edit_panel, edit_panel_w, edit_panel_h, 1.f,
            edit_panel_x, edit_panel_y);

        std::string line1{};
        std::string line2{};

        // Contextual logic: change text based on the type of puzzle being edited
        switch (editing_puzzle->type)
        {
        case Puzzle::PUSHABLE:
        case Puzzle::PIT:
        case Puzzle::SPIKE:
            // Identify active element (Water, Fire, etc.)
            switch (editing_puzzle->element)
            {
            case Puzzle::WATER: line1 = "Element: Water"; break;
            case Puzzle::FIRE:  line1 = "Element: Fire";  break;
            default:            line1 = "Element: None";  break;
            }
            // Identify visibility restrictions
            switch (editing_puzzle->hidden_from)
            {
            case 1:  line2 = "Can't see: Knight"; break;
            case 2:  line2 = "Can't see: Wizard"; break;
            default: line2 = "Can't see: None";   break;
            }
            break;

        case Puzzle::BUTTON_UP:
            line1 = "Place spikes to pair...";
            switch (editing_puzzle->hidden_from)
            {
            case 1:  line2 = "Can't see: Knight"; break;
            case 2:  line2 = "Can't see: Wizard"; break;
            default: line2 = "Can't see: None";   break;
            }
            break;

        default: break;
        }

        f32 text_w, text_h;
        char const* msg1 = line1.c_str();
        char const* msg2 = line2.c_str();
        AEGfxGetPrintSize(level_editor_font, msg1, text_scale_edit_panel, &text_w, &text_h);

        // Normalize coordinates for Alpha Engine text rendering
        f32 half_w = AEGfxGetWindowWidth() / 2.f;
        f32 half_h = AEGfxGetWindowHeight() / 2.f;
        f32 norm_x = edit_panel_x / half_w;
        f32 norm_y = edit_panel_y / half_h;
        f32 btn_sz = edit_panel_h / 4.f;

        f32 line1_norm_y = norm_y + text_h * 1.5f;
        f32 line2_norm_y = norm_y - text_h * 1.5f;

        // Render arrow buttons for cycling through elements (if applicable)
        if (editing_puzzle->type != Puzzle::BUTTON_UP)
        {
            render_editor_ui(edit_panel_button_left,
                btn_sz, btn_sz, 1.f,
                edit_panel_x - edit_panel_w * 0.35f,
                edit_panel_y + edit_panel_h * 0.18f);
            render_editor_ui(edit_panel_button_right,
                btn_sz, btn_sz, 1.f,
                edit_panel_x + edit_panel_w * 0.35f,
                edit_panel_y + edit_panel_h * 0.18f);
        }

        render_text_center(msg1, level_editor_font, norm_x, line1_norm_y, text_scale_edit_panel, black);

        // Render arrow buttons for cycling through visibility options
        render_editor_ui(edit_panel_button_left,
            btn_sz, btn_sz, 1.f,
            edit_panel_x - edit_panel_w * 0.35f,
            edit_panel_y - edit_panel_h * 0.16f);
        render_editor_ui(edit_panel_button_right,
            btn_sz, btn_sz, 1.f,
            edit_panel_x + edit_panel_w * 0.35f,
            edit_panel_y - edit_panel_h * 0.16f);

        render_text_center(msg2, level_editor_font, norm_x, line2_norm_y, text_scale_edit_panel, black);
    }

    // ----------------------------------------------------------------------------
    //
    // Function: edit_puzzle
    // Processes user input for the puzzle editing UI, handling button clicks
    // to save changes, cancel edits, or toggle properties.
    //
    // ----------------------------------------------------------------------------
    void edit_puzzle()
    {
        // Only process if a mouse click occurred this frame
        if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;

        // Handle Save Button click: commit changes and close panel
        if (IsButtonClicked(ep_button_x, edit_panel_y + edit_panel_h / 4.f,
            edit_panel_h / 2.f, edit_panel_h / 2.f))
        {
            play_sfx(g_audio.sfx_button_click, 0.1f);
            editing = false;
            selected_puzzle = Puzzle::PUSHABLE;
            can_paint = false;
        }

        // Handle Exit Button click: restore the puzzle to its state prior to editing
        if (IsButtonClicked(ep_button_x, edit_panel_y - edit_panel_h / 4.f,
            edit_panel_h / 2.f, edit_panel_h / 2.f))
        {
            play_sfx(g_audio.sfx_button_click, 0.1f);
            *editing_puzzle = old_puzzle;
            editing = false;
            selected_puzzle = Puzzle::PUSHABLE;
            can_paint = false;
        }

        // Element toggle logic: cycle through available elemental types
        if (editing_puzzle->type != Puzzle::BUTTON_UP)
        {
            if (IsButtonClicked(edit_panel_x - edit_panel_w * 0.35f,
                edit_panel_y + edit_panel_h * 0.18f,
                edit_panel_h / 4.f, edit_panel_h / 4.f))
            {
                play_sfx(g_audio.sfx_button_click, 0.1f);
                editing_puzzle->element = previous_element(editing_puzzle->element);
                can_paint = false;
            }
            if (IsButtonClicked(edit_panel_x + edit_panel_w * 0.35f,
                edit_panel_y + edit_panel_h * 0.18f,
                edit_panel_h / 4.f, edit_panel_h / 4.f))
            {
                play_sfx(g_audio.sfx_button_click, 0.1f);
                editing_puzzle->element = next_element(editing_puzzle->element);
                can_paint = false;
            }
        }

        // Visibility toggle logic: cycle through visibility modes (Both, Knight only, Wizard only)
        if (IsButtonClicked(edit_panel_x - edit_panel_w * 0.35f,
            edit_panel_y - edit_panel_h * 0.16f,
            edit_panel_h / 4.f, edit_panel_h / 4.f))
        {
            play_sfx(g_audio.sfx_button_click, 0.1f);
            editing_puzzle->hidden_from = (editing_puzzle->hidden_from + 2) % 3;
            can_paint = false;
        }

        if (IsButtonClicked(edit_panel_x + edit_panel_w * 0.35f,
            edit_panel_y - edit_panel_h * 0.16f,
            edit_panel_h / 4.f, edit_panel_h / 4.f))
        {
            play_sfx(g_audio.sfx_button_click, 0.1f);
            editing_puzzle->hidden_from = (editing_puzzle->hidden_from + 1) % 3;
            can_paint = false;
        }
    }

    // ----------------------------------------------------------------------------
    //
    // Function: render_status_msg
    // Displays temporary feedback text (e.g., "Map Saved") that fades out 
    // over a specified duration.
    //
    // ----------------------------------------------------------------------------
    void render_status_msg(s8 font, f32 timer, f32 duration)
    {
        // Exit early if there is no message to display
        if (!active_status_msg || active_status_msg[0] == '\0') return;

        // Choose color based on whether the status is an error or a success
        f32 r = active_status_error ? status_error_r : status_success_r;
        f32 g = active_status_error ? status_error_g : status_success_g;
        f32 b = active_status_error ? status_error_b : status_success_b;

        // Calculate alpha for a smooth fade-out effect as the timer runs down
        f32 alpha = timer / duration;

        f32 tw = 0.f, th = 0.f;
        AEGfxGetPrintSize(font, active_status_msg, status_msg_scale, &tw, &th);

        // Render centered message on screen
        AEGfxPrint(font, active_status_msg, -tw * 0.5f, status_msg_norm_y,
            status_msg_scale, r, g, b, alpha);

        // Reset message flags after rendering to prevent persistence
        active_status_msg = "";
        active_status_error = false;
    }

#pragma endregion

} // namespace

// ----------------------------------------------------------------------------
// level_editor_load
// ----------------------------------------------------------------------------
void level_editor_load()
{
    auto cfg = parse_kv_file("Assets/LevelEditor/level_editor_config.txt");

    // --- Textures ---
    cursor = AEGfxTextureLoad(kv_str(cfg, "cursor_path").c_str());
    tomb = AEGfxTextureLoad(kv_str(cfg, "tomb_path").c_str());
    ghost_spawn = AEGfxTextureLoad(kv_str(cfg, "ghost_spawn_path").c_str());
    bat_spawn = AEGfxTextureLoad(kv_str(cfg, "bat_spawn_path").c_str());
    erase_tile = AEGfxTextureLoad(kv_str(cfg, "erase_tile_path").c_str());
    edit_panel = AEGfxTextureLoad(kv_str(cfg, "edit_panel_path").c_str());
    edit_panel_save = AEGfxTextureLoad(kv_str(cfg, "edit_panel_save_path").c_str());
    edit_panel_exit = AEGfxTextureLoad(kv_str(cfg, "edit_panel_exit_path").c_str());
    edit_panel_button_left = AEGfxTextureLoad(kv_str(cfg, "edit_panel_button_left_path").c_str());
    edit_panel_button_right = AEGfxTextureLoad(kv_str(cfg, "edit_panel_button_right_path").c_str());
    confirm_panel = AEGfxTextureLoad(kv_str(cfg, "confirm_panel_path").c_str());
    confirm_btn_yes = AEGfxTextureLoad(kv_str(cfg, "confirm_btn_yes_path").c_str());
    confirm_btn_no = AEGfxTextureLoad(kv_str(cfg, "confirm_btn_no_path").c_str());

    load_tile_textures(kv_str(cfg, "tile_textures_folder").c_str());
    Puzzle::load_puzzle_textures();

    // --- Font (path + size stored; font created in init) ---
    // Store into local statics so init can use them
    static std::string s_font_path;
    static s32         s_font_size = 0;
    s_font_path = kv_str(cfg, "font_path");
    s_font_size = kv_int(cfg, "font_size");

    // --- Cursor ---
    cursor_w = kv_float(cfg, "cursor_w");
    cursor_h = kv_float(cfg, "cursor_h");
    cursor_offset_x = kv_float(cfg, "cursor_offset_x");
    cursor_offset_y = kv_float(cfg, "cursor_offset_y");

    // --- Palette UI ---
    PANEL_X = kv_float(cfg, "panel_x");
    PALETTE_TOP_Y = kv_float(cfg, "palette_top_y");
    PAL_W = kv_float(cfg, "palette_w");
    PAL_H = kv_float(cfg, "palette_h");
    PAL_GAP = kv_float(cfg, "palette_gap");

    // --- Edit Panel ---
    edit_panel_w = kv_float(cfg, "edit_panel_w");
    edit_panel_h = kv_float(cfg, "edit_panel_h");

    // --- Camera ---
    CAM_SPEED = kv_float(cfg, "cam_speed");
    editor_zoom_min = kv_float(cfg, "zoom_min");
    editor_zoom_max = kv_float(cfg, "zoom_max");

    // --- Grid ---
    tile_draw_size = kv_float(cfg, "tile_draw_size");
    grid_start_x = kv_float(cfg, "grid_start_x");
    grid_start_y = kv_float(cfg, "grid_start_y");

    // --- Border multipliers ---
    top_border_tile_mult = kv_float(cfg, "top_border_tile_mult");
    bottom_border_tile_mult = kv_float(cfg, "bottom_border_tile_mult");
    left_border_tile_mult = kv_float(cfg, "left_border_tile_mult");
    right_border_tile_mult = kv_float(cfg, "right_border_tile_mult");

    // --- Background color ---
    bg_color_r = kv_float(cfg, "bg_color_r");
    bg_color_g = kv_float(cfg, "bg_color_g");
    bg_color_b = kv_float(cfg, "bg_color_b");

    // --- Timers ---
    SAVE_MSG_DUR = kv_float(cfg, "save_msg_duration");
    SPAWN_ERROR_DUR = kv_float(cfg, "spawn_error_duration");

    // --- Hover / Preview ---
    hover_alpha = kv_float(cfg, "hover_alpha");
    ghost_preview_alpha = kv_float(cfg, "ghost_preview_alpha");
    palette_icon_padding = kv_float(cfg, "palette_icon_padding");

    // --- Text ---
    text_scale_edit_panel = kv_float(cfg, "text_scale_edit_panel");
    tiles_header_label = kv_str(cfg, "tiles_header_label");
    tiles_header_color_r = kv_float(cfg, "tiles_header_color_r");
    tiles_header_color_g = kv_float(cfg, "tiles_header_color_g");
    tiles_header_color_b = kv_float(cfg, "tiles_header_color_b");

    // --- Hint bar ---
    hint_bar_norm_y = kv_float(cfg, "hint_bar_norm_y");
    hint_bar_color_r = kv_float(cfg, "hint_bar_color_r");
    hint_bar_color_g = kv_float(cfg, "hint_bar_color_g");
    hint_bar_color_b = kv_float(cfg, "hint_bar_color_b");
    hint_tiles = kv_str(cfg, "hint_tiles");
    hint_puzzles = kv_str(cfg, "hint_puzzles");

    // --- Palette label colors ---
    label_selected_r = kv_float(cfg, "label_selected_r");
    label_selected_g = kv_float(cfg, "label_selected_g");
    label_selected_b = kv_float(cfg, "label_selected_b");
    label_normal_r = kv_float(cfg, "label_normal_r");
    label_normal_g = kv_float(cfg, "label_normal_g");
    label_normal_b = kv_float(cfg, "label_normal_b");
    label_locked_r = kv_float(cfg, "label_locked_r");
    label_locked_g = kv_float(cfg, "label_locked_g");
    label_locked_b = kv_float(cfg, "label_locked_b");
    label_hotkey_r = kv_float(cfg, "label_hotkey_r");
    label_hotkey_g = kv_float(cfg, "label_hotkey_g");
    label_hotkey_b = kv_float(cfg, "label_hotkey_b");
    label_locked_count_r = kv_float(cfg, "label_locked_count_r");
    label_locked_count_g = kv_float(cfg, "label_locked_count_g");
    label_locked_count_b = kv_float(cfg, "label_locked_count_b");

    // --- Confirm popup ---
    confirm_label = kv_str(cfg, "confirm_label");
    confirm_text_scale = kv_float(cfg, "confirm_text_scale");
    confirm_yes_x = kv_float(cfg, "confirm_yes_x");
    confirm_yes_y = kv_float(cfg, "confirm_yes_y");
    confirm_yes_w = kv_float(cfg, "confirm_yes_w");
    confirm_yes_h = kv_float(cfg, "confirm_yes_h");
    confirm_no_x = kv_float(cfg, "confirm_no_x");
    confirm_no_y = kv_float(cfg, "confirm_no_y");
    confirm_no_w = kv_float(cfg, "confirm_no_w");
    confirm_no_h = kv_float(cfg, "confirm_no_h");

    // --- Status message ---
    status_msg_norm_y = kv_float(cfg, "status_msg_norm_y");
    status_msg_scale = kv_float(cfg, "status_msg_scale");
    status_success_r = kv_float(cfg, "status_success_r");
    status_success_g = kv_float(cfg, "status_success_g");
    status_success_b = kv_float(cfg, "status_success_b");
    status_error_r = kv_float(cfg, "status_error_r");
    status_error_g = kv_float(cfg, "status_error_g");
    status_error_b = kv_float(cfg, "status_error_b");

    // --- Grid screen clamp ---
    grid_screen_left = kv_float(cfg, "grid_screen_left");
    grid_screen_right = kv_float(cfg, "grid_screen_right");
    grid_screen_top = kv_float(cfg, "grid_screen_top");
    grid_screen_bottom = kv_float(cfg, "grid_screen_bottom");

    // --- World offsets ---
    world_x_offset = kv_float(cfg, "world_x_offset");
    world_y_offset = kv_float(cfg, "world_y_offset");
}

// ----------------------------------------------------------------------------
// level_editor_init
// ----------------------------------------------------------------------------
void level_editor_init()
{
    // Font created here — graphics context is ready
    {
        auto cfg = parse_kv_file("Assets/LevelEditor/level_editor_config.txt");
        std::string fp = kv_str(cfg, "font_path");
        s32         fs = kv_int(cfg, "font_size");
        SILENT_CALL(level_editor_font = AEGfxCreateFont(fp.c_str(), fs));
    }

    Camera::zoom = editor_zoom_min;
    cam_x = cam_y = 0.f;
    Camera::cam = Camera::init_cam(&cam_x, &cam_y);
    show_exit_confirm = false;

    init_mesh();
    AEGfxSetBackgroundColor(bg_color_r, bg_color_g, bg_color_b);
    AEInputShowCursor(false);

    // Allocate tile map
    editor_map = new int* [TILE_ROWS];
    for (s32 r = 0; r < TILE_ROWS; ++r)
        editor_map[r] = new int[TILE_COLS] {};

    // Allocate puzzle map
    puzzle_editor_map = new Puzzle::PuzzleType * [TILE_ROWS];
    for (s32 r = 0; r < TILE_ROWS; ++r)
        puzzle_editor_map[r] = new Puzzle::PuzzleType[TILE_COLS]{ Puzzle::PUZZLE_TYPE_COUNT };
    for (s32 r = 0; r < TILE_ROWS; ++r)
        for (s32 c = 0; c < TILE_COLS; ++c)
            puzzle_editor_map[r][c] = Puzzle::PUZZLE_TYPE_COUNT;

    puzzles.clear();
    button_triggers.clear();
    puzzles.reserve(100);

    // Borders (derived from tile_draw_size and multipliers)
    top_border_height = tile_draw_size * top_border_tile_mult;
    bottom_border_height = tile_draw_size * bottom_border_tile_mult;
    side_border_height = AEGfxGetWindowWidth() - top_border_height - bottom_border_height;
    left_border_width = tile_draw_size * left_border_tile_mult;
    right_border_width = tile_draw_size * right_border_tile_mult;

    // Camera limits
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

    if (!selected_level_path.empty())
    {
        load_map_to_editor(selected_level_path.c_str(), editor_map);
        Puzzle::load_puzzle_blocks_to_editor(
            selected_level_puzzle_path.c_str(),
            puzzle_editor_map, puzzles, button_triggers);

        for (s32 r = 0; r < TILE_ROWS; ++r)
            for (s32 c = 0; c < TILE_COLS; ++c)
            {
                if (editor_map[r][c] == 3) wizard_placed = true;
                if (editor_map[r][c] == 4) knight_placed = true;
                if (editor_map[r][c] == 7) door_placed = true;
            }
    }
}

// ----------------------------------------------------------------------------
// level_editor_update
// ----------------------------------------------------------------------------
void level_editor_update()
{
    if (AEInputCheckReleased(AEVK_LBUTTON) || AEInputCheckReleased(AEVK_RBUTTON))
        can_paint = true;

    dt = (f32)AEFrameRateControllerGetFrameTime();
    get_zoom(dt, editor_zoom_min, editor_zoom_max);
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    // --- Button hover sound ---
    static s32 last_hovered = -1;
    s32 current_hovered = -1;

    if (editorState == EDIT_TILES)
    {
        for (s32 i = 0; i < PALETTE_COUNT; ++i)
        {
            f32 py = PALETTE_TOP_Y - i * PAL_GAP;
            if (IsButtonHovered(PANEL_X, py, PAL_W, PAL_H)) { current_hovered = i; break; }
        }
    }
    else if (editorState == EDIT_PUZZLES)
    {
        for (s32 i = 0; i < EDITABLE_PUZZLE_COUNT; ++i)
        {
            f32 py = PALETTE_TOP_Y - i * PAL_GAP;
            if (IsButtonHovered(PANEL_X, py, PAL_W, PAL_H)) { current_hovered = i + 100; break; }
        }
    }

    if (show_exit_confirm)
    {
        if (IsButtonHovered(confirm_yes_x, confirm_yes_y, confirm_yes_w, confirm_yes_h)) current_hovered = 9998;
        if (IsButtonHovered(confirm_no_x, confirm_no_y, confirm_no_w, confirm_no_h))  current_hovered = 9999;
    }

    if (current_hovered != last_hovered && current_hovered != -1)
        play_sfx(g_audio.sfx_button_hover);
    last_hovered = current_hovered;

    if (AEInputCheckTriggered(AEVK_ESCAPE))
        show_exit_confirm = true;

    if (show_exit_confirm)
    {
        if (IsButtonClicked(confirm_yes_x, confirm_yes_y, confirm_yes_w, confirm_yes_h))
        {
            play_sfx(g_audio.sfx_button_click, 0.1f);
            next = GS_LEVELS;
        }
        if (IsButtonClicked(confirm_no_x, confirm_no_y, confirm_no_w, confirm_no_h))
        {
            play_sfx(g_audio.sfx_button_click, 0.1f);
            show_exit_confirm = false;
            can_paint = false;
        }
        return;
    }

    if (save_msg_timer > 0.f) save_msg_timer -= dt;
    if (spawn_error_timer > 0.f) spawn_error_timer -= dt;

    if (AEInputCheckTriggered(AEVK_S))
    {
        if (wizard_placed && knight_placed && door_placed)
        {
            if (!selected_level_path.empty())
            {
                save_success =
                    save_map_to_file_overwrite(editor_map, selected_level_path) &&
                    overwrite_puzzle_blocks(puzzles, button_triggers, selected_level_puzzle_path);
                std::string spawn_path =
                    selected_level_path.substr(0, selected_level_path.find_last_of('.')) + "_enemy_spawns.txt";
                enemy_manager_instance.save_spawn_positions_from_map(editor_map, spawn_path);
            }
            else
            {
                std::string new_level_path, new_puzzle_path;
                save_success = save_map_to_file(editor_map, new_level_path) &&
                    save_puzzle_blocks(puzzles, button_triggers, new_puzzle_path);
                if (save_success)
                {
                    std::string spawn_path =
                        new_level_path.substr(0, new_level_path.find_last_of('.')) + "_enemy_spawns.txt";
                    enemy_manager_instance.save_spawn_positions_from_map(editor_map, spawn_path);
                    selected_level_path = new_level_path;
                    selected_level_puzzle_path = new_puzzle_path;
                }
            }
        }
        else
        {
            save_success = false;
        }
        save_msg_timer = SAVE_MSG_DUR;
    }

    if (!editing)
    {
        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            if (IsButtonClicked(PANEL_X + PAL_W / 4.f, PALETTE_TOP_Y + PAL_H, PAL_W / 2.f, PAL_H))
            {
                play_sfx(g_audio.sfx_button_click, 0.1f);
                editorState = EDIT_PUZZLES;
            }
            if (IsButtonClicked(PANEL_X - PAL_W / 4.f, PALETTE_TOP_Y + PAL_H, PAL_W / 2.f, PAL_H))
            {
                play_sfx(g_audio.sfx_button_click, 0.1f);
                editorState = EDIT_TILES;
            }
        }
    }
    else
    {
        edit_puzzle();
    }

    simulate_editor_cam();

    if (editorState == EDIT_TILES)
    {
        const u8 hotkeys[8] = { AEVK_1, AEVK_2, AEVK_3, AEVK_4, AEVK_5, AEVK_6, AEVK_7, AEVK_8 };
        for (s32 i = 0; i < PALETTE_COUNT; ++i)
            if (AEInputCheckTriggered(hotkeys[i]))
                selected_tile = palette_ids[i];

        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            for (s32 i = 0; i < PALETTE_COUNT; ++i)
            {
                f32 py = PALETTE_TOP_Y - i * PAL_GAP;
                if (IsButtonClicked(PANEL_X, py, PAL_W, PAL_H))
                {
                    play_sfx(g_audio.sfx_button_click, 0.1f);
                    selected_tile = palette_ids[i];
                    return;
                }
            }
        }

        f32 wx, wy;
        mouse_to_world(wx, wy);
        s32 row, col;
        bool on_grid = editor_screen_to_tile(wx, wy, row, col);
        f32  grid_right_world = grid_start_x + (TILE_COLS * tile_draw_size);
        f32  grid_right_screen = grid_right_world + world_x_offset;
        bool on_panel = ((f32)mouse_x > grid_right_screen);

        if (on_grid && !on_panel)
        {
            if (can_paint && AEInputCheckCurr(AEVK_LBUTTON))
            {
                s32 curr = editor_map[row][col];
                s32 placing = selected_tile;

                if ((placing == 5 || placing == 6) && curr != 2) goto skip_paint;
                if (placing == 5 && row >= (TILE_ROWS - 1)) { spawn_error_timer = SPAWN_ERROR_DUR; goto skip_paint; }

                {
                    bool blocked =
                        (placing == 3 && wizard_placed && curr != 3) ||
                        (placing == 4 && knight_placed && curr != 4) ||
                        (placing == 7 && door_placed && curr != 7) ||
                        (puzzle_editor_map[row][col] != Puzzle::PUZZLE_TYPE_COUNT);

                    if (!blocked)
                    {
                        if (curr == 3) wizard_placed = false;
                        if (curr == 4) knight_placed = false;
                        if (curr == 7) door_placed = false;

                        if (curr == 5 || curr == 6)
                            editor_map[row][col] = 2;

                        if (placing == 5 && row > 0 && editor_map[row - 1][col] == 5)
                            editor_map[row - 1][col] = 2;
                        if (placing == 5 && row < (TILE_ROWS - 1) && editor_map[row + 1][col] == 5)
                            editor_map[row + 1][col] = 2;

                        editor_map[row][col] = placing;
                        if (placing == 3) wizard_placed = true;
                        if (placing == 4) knight_placed = true;
                        if (placing == 7) door_placed = true;
                    }
                }
            skip_paint:;
            }

            if (puzzle_editor_map[row][col] == Puzzle::PUZZLE_TYPE_COUNT &&
                can_paint && AEInputCheckCurr(AEVK_RBUTTON))
            {
                s32 curr = editor_map[row][col];
                if (curr == 3) wizard_placed = false;
                if (curr == 4) knight_placed = false;
                if (curr == 7) door_placed = false;
                editor_map[row][col] = (curr == 5 || curr == 6) ? 2 : 0;
            }
        }

        if (AEInputCheckTriggered(AEVK_F))
        {
            if (selected_tile == 3 || selected_tile == 4 || selected_tile == 7) return;
            for (s32 r = 0; r < TILE_ROWS; ++r)
                for (s32 c = 0; c < TILE_COLS; ++c)
                    if (editor_map[r][c] == 0)
                        editor_map[r][c] = selected_tile;
        }

        if (AEInputCheckTriggered(AEVK_C))
        {
            for (s32 r = 0; r < TILE_ROWS; ++r)
                for (s32 c = 0; c < TILE_COLS; ++c)
                    editor_map[r][c] = 0;
            wizard_placed = knight_placed = door_placed = false;
        }
    }
    else if (editorState == EDIT_PUZZLES)
    {
        if (!editing)
        {
            const u8 hotkeys[EDITABLE_PUZZLE_COUNT] = { AEVK_1, AEVK_2, AEVK_3, AEVK_4 };
            for (s32 i = 0; i < EDITABLE_PUZZLE_COUNT; ++i)
                if (AEInputCheckTriggered(hotkeys[i]))
                    selected_puzzle = puzzle_palette_ids[i];

            if (AEInputCheckTriggered(AEVK_LBUTTON))
            {
                for (s32 i = 0; i < EDITABLE_PUZZLE_COUNT; ++i)
                {
                    f32 py = PALETTE_TOP_Y - i * PAL_GAP;
                    if (IsButtonClicked(PANEL_X, py, PAL_W, PAL_H))
                    {
                        play_sfx(g_audio.sfx_button_click, 0.1f);
                        if (puzzle_palette_ids[i] != Puzzle::SPIKE)
                        {
                            selected_puzzle = puzzle_palette_ids[i];
                            return;
                        }
                    }
                }
            }

            if (AEInputCheckTriggered(AEVK_C))
            {
                for (s32 r = 0; r < TILE_ROWS; ++r)
                    for (s32 c = 0; c < TILE_COLS; ++c)
                        puzzle_editor_map[r][c] = Puzzle::PUZZLE_TYPE_COUNT;
                puzzles.clear();
            }
        }
        else if (editing_puzzle->type == Puzzle::BUTTON_UP)
        {
            selected_puzzle = puzzle_palette_ids[Puzzle::SPIKE + 1];
        }

        f32 wx, wy;
        mouse_to_world(wx, wy);
        s32 row, col;
        bool on_grid = editor_screen_to_tile(wx, wy, row, col);
        f32  grid_right_world = grid_start_x + (TILE_COLS * tile_draw_size);
        f32  grid_right_screen = grid_right_world + world_x_offset;
        bool on_panel = ((f32)mouse_x > grid_right_screen);

        if (!on_grid && on_panel) return;

        if (!editing)
        {
            if (can_paint && AEInputCheckCurr(AEVK_LBUTTON) && editor_map[row][col] == 2)
            {
                if (selected_puzzle == Puzzle::PUZZLE_TYPE_COUNT)
                {
                    if (puzzle_editor_map[row][col] == Puzzle::SPIKE)
                    {
                        for (auto& bt : button_triggers)
                        {
                            if (spike_is_paired(row, col, bt))
                            {
                                remove_spike_button_pair(row, col, bt);
                                break;
                            }
                        }
                    }
                    puzzle_editor_map[row][col] = selected_puzzle;
                    remove_puzzle(row, col, puzzles);
                }
                else if (puzzle_editor_map[row][col] == Puzzle::PUZZLE_TYPE_COUNT)
                {
                    puzzle_editor_map[row][col] = selected_puzzle;
                    place_puzzle(row, col, selected_puzzle, puzzles);
                }
            }

            if (can_paint && AEInputCheckCurr(AEVK_RBUTTON) &&
                puzzle_editor_map[row][col] != Puzzle::PUZZLE_TYPE_COUNT)
            {
                editing = true;
                editing_puzzle = get_editing_puzzle(row, col, puzzles);
                old_puzzle = *editing_puzzle;
                can_paint = false;

                if (puzzle_editor_map[row][col] == Puzzle::BUTTON_UP)
                {
                    if (editing_puzzle->button_id == -1)
                    {
                        editing_puzzle->button_id = (s32)button_triggers.size();
                        button_triggers.push_back(Puzzle::buttonTrigger{ editing_puzzle->button_id, {} });
                    }
                }
            }
        }
        else if (editing_puzzle->type == Puzzle::BUTTON_UP)
        {
            if (AEInputCheckCurr(AEVK_LBUTTON))
            {
                if (can_paint && editor_map[row][col] == 2 &&
                    puzzle_editor_map[row][col] == Puzzle::PUZZLE_TYPE_COUNT)
                {
                    puzzle_editor_map[row][col] = selected_puzzle;
                    place_puzzle(row, col, Puzzle::SPIKE, puzzles);
                    pair_spike_to_button(row, col, button_triggers[editing_puzzle->button_id]);
                }
                if (puzzle_editor_map[row][col] == Puzzle::BUTTON_UP)
                {
                    Puzzle::puzzle& curr = *get_editing_puzzle(row, col, puzzles);
                    if (curr.button_id == -1)
                        curr.button_id = editing_puzzle->button_id;
                }
            }
            else if (can_paint && AEInputCheckCurr(AEVK_RBUTTON))
            {
                if (puzzle_editor_map[row][col] == Puzzle::SPIKE &&
                    spike_is_paired(row, col, button_triggers[editing_puzzle->button_id]))
                {
                    puzzle_editor_map[row][col] = Puzzle::PUZZLE_TYPE_COUNT;
                    remove_puzzle(row, col, puzzles);
                    remove_spike_button_pair(row, col, button_triggers[editing_puzzle->button_id]);
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// level_editor_draw
// ----------------------------------------------------------------------------
void level_editor_draw()
{
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);

    // Grid background
    if (erase_tile)
    {
        for (s32 r = 0; r < TILE_ROWS; ++r)
            for (s32 c = 0; c < TILE_COLS; ++c)
            {
                f32 wx = grid_start_x + c * tile_draw_size + tile_draw_size * 0.5f;
                f32 wy = grid_start_y - r * tile_draw_size - tile_draw_size * 0.5f;
                render_editor_tiles(erase_tile, tile_draw_size, tile_draw_size, 1.f, wx, wy);
            }
    }

    // Map tiles
    for (s32 r = 0; r < TILE_ROWS; ++r)
    {
        for (s32 c = 0; c < TILE_COLS; ++c)
        {
            f32 wx = grid_start_x + c * tile_draw_size + tile_draw_size * 0.5f;
            f32 wy = grid_start_y - r * tile_draw_size - tile_draw_size * 0.5f;

            f32 grid_left = grid_start_x;
            f32 grid_right = grid_left + TILE_COLS * tile_draw_size;
            f32 grid_top = grid_start_y;
            f32 grid_bot = grid_top - TILE_ROWS * tile_draw_size;
            if (wx < grid_left || wx > grid_right) continue;
            if (wy > grid_top || wy < grid_bot)   continue;

            s32 tile = editor_map[r][c];
            if (tile == 5 || tile == 6)
            {
                if (TileTextures[2])
                    render_editor_tiles(TileTextures[2], tile_draw_size, tile_draw_size, 1.f, wx, wy);
            }
            else if (tile != 0 && TileTextures[tile])
                render_editor_tiles(TileTextures[tile], tile_draw_size, tile_draw_size, 1.f, wx, wy);

            Puzzle::PuzzleType ptype = puzzle_editor_map[r][c];
            if (ptype != Puzzle::PUZZLE_TYPE_COUNT && Puzzle::puzzle_block_textures[ptype])
            {
                Puzzle::puzzle* curr = get_editing_puzzle(r, c, puzzles);
                bool highlight = editing &&
                    editing_puzzle->type == Puzzle::BUTTON_UP &&
                    (ptype == Puzzle::SPIKE || ptype == Puzzle::BUTTON_UP) &&
                    (spike_is_paired(r, c, button_triggers[editing_puzzle->button_id]) ||
                        (curr && curr->button_id == editing_puzzle->button_id));

                render_editor_tiles(Puzzle::puzzle_block_textures[ptype],
                    tile_draw_size, tile_draw_size, 1.f, wx, wy, highlight);
            }
        }
    }

    // Spawn overlays
    for (s32 r = 0; r < TILE_ROWS; ++r)
    {
        for (s32 c = 0; c < TILE_COLS; ++c)
        {
            f32 wx = grid_start_x + c * tile_draw_size + tile_draw_size * 0.5f;
            f32 wy = grid_start_y - r * tile_draw_size - tile_draw_size * 0.5f;

            if (wx < grid_start_x || wx > grid_start_x + TILE_COLS * tile_draw_size) continue;
            if (wy > grid_start_y || wy < grid_start_y - TILE_ROWS * tile_draw_size) continue;

            s32 tile = editor_map[r][c];
            if (tile == 5 && tomb)
            {
                render_editor_tiles(tomb, tile_draw_size, tile_draw_size, 1.f, wx, wy);
                if (ghost_spawn)
                    render_editor_tiles(ghost_spawn, tile_draw_size, tile_draw_size, 1.f,
                        wx, wy - tile_draw_size);
            }
            else if (tile == 6 && bat_spawn)
                render_editor_tiles(bat_spawn, tile_draw_size, tile_draw_size, 1.f, wx, wy);
        }
    }

    // Hover highlight
    {
        f32 wx_m, wy_m;
        mouse_to_world(wx_m, wy_m);
        s32 row, col;
        if (editor_screen_to_tile(wx_m, wy_m, row, col))
        {
            f32 cx = grid_start_x + col * tile_draw_size + tile_draw_size * 0.5f;
            f32 cy = grid_start_y - row * tile_draw_size - tile_draw_size * 0.5f;
            bool in_bounds = cx >= grid_screen_left && cx <= grid_screen_right &&
                cy >= grid_screen_bottom && cy <= grid_screen_top;

            if (editorState == EDIT_TILES)
            {
                if (selected_tile == 0 && erase_tile && in_bounds)
                    render_editor_tiles(erase_tile, tile_draw_size, tile_draw_size, hover_alpha, cx, cy);
                else if (selected_tile == 5 && tomb && in_bounds)
                {
                    render_editor_tiles(tomb, tile_draw_size, tile_draw_size, ghost_preview_alpha, cx, cy);
                    if (ghost_spawn)
                        render_editor_tiles(ghost_spawn, tile_draw_size, tile_draw_size,
                            ghost_preview_alpha, cx, cy - tile_draw_size);
                }
                else if (selected_tile == 6 && bat_spawn && in_bounds)
                    render_editor_tiles(bat_spawn, tile_draw_size, tile_draw_size, ghost_preview_alpha, cx, cy);
                else if (TileTextures[selected_tile] && in_bounds)
                    render_editor_tiles(TileTextures[selected_tile], tile_draw_size, tile_draw_size,
                        ghost_preview_alpha, cx, cy);
            }
            else if (editorState == EDIT_PUZZLES)
            {
                if (selected_puzzle == Puzzle::PUZZLE_TYPE_COUNT && erase_tile && in_bounds)
                    render_editor_tiles(erase_tile, tile_draw_size, tile_draw_size, hover_alpha, cx, cy);
                else if (Puzzle::puzzle_block_textures[selected_puzzle] && in_bounds)
                    render_editor_tiles(Puzzle::puzzle_block_textures[selected_puzzle],
                        tile_draw_size, tile_draw_size, ghost_preview_alpha, cx, cy);
            }
        }
    }

    if (editing) draw_puzzle_edit_interface();

    // Outer borders
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToMultiply({ 0.129f, 0.129f, 0.133f, 0.f });

        draw_square(0.f, AEGfxGetWinMaxY() - top_border_height / 2.f,
            static_cast<f32>(AEGfxGetWindowWidth()), top_border_height, false);
        draw_square(0.f, AEGfxGetWinMinY() + bottom_border_height / 2.f,
            static_cast<f32>(AEGfxGetWindowWidth()), bottom_border_height, false);
        draw_square(AEGfxGetWinMinX() + left_border_width / 2.f, -tile_draw_size / 2.f,
            left_border_width, side_border_height, false);
        draw_square(AEGfxGetWinMaxX() - right_border_width / 2.f, -tile_draw_size / 2.f,
            right_border_width, side_border_height, false);
    }

    // Palette
    if (editorState == EDIT_TILES)
    {
        for (s32 i = 0; i < PALETTE_COUNT; ++i)
        {
            f32  py = PALETTE_TOP_Y - i * PAL_GAP;
            bool hovered = IsButtonHovered(PANEL_X, py, PAL_W, PAL_H);
            bool is_selected = (palette_ids[i] == selected_tile);
            bool is_locked = (palette_ids[i] == 3 && wizard_placed) ||
                (palette_ids[i] == 4 && knight_placed) ||
                (palette_ids[i] == 7 && door_placed);

            if (is_selected && TileTextures[2])
                render_editor_ui(TileTextures[2], PAL_W + palette_icon_padding, PAL_H + palette_icon_padding,
                    ghost_preview_alpha, PANEL_X, py);
            if (hovered && !is_selected && TileTextures[2])
                render_editor_ui(TileTextures[2], PAL_W, PAL_H, hover_alpha, PANEL_X, py);

            f32 icon_alpha = is_locked ? hover_alpha : 1.f;
            f32 icon_x = PANEL_X - PAL_W * 0.5f + PAL_H * 0.5f;

            if (palette_ids[i] == 0)
                render_editor_ui(erase_tile, PAL_H - palette_icon_padding, PAL_H - palette_icon_padding,
                    icon_alpha, icon_x, py);
            else
            {
                s32 icon_id = palette_ids[i];
                AEGfxTexture* icon_tex =
                    (icon_id == 5) ? tomb :
                    (icon_id == 6) ? bat_spawn :
                    TileTextures[icon_id];
                if (icon_tex)
                    render_editor_ui(icon_tex, PAL_H - palette_icon_padding, PAL_H - palette_icon_padding,
                        icon_alpha, icon_x, py);
            }

            if (level_editor_font != -1)
            {
                f32 norm_y = (py - 10.f) / world_y_offset;
                f32 lr = is_locked ? label_locked_r : (is_selected ? label_selected_r : label_normal_r);
                f32 lg = is_locked ? label_locked_g : (is_selected ? label_selected_g : label_normal_g);
                f32 lb = is_locked ? label_locked_b : (is_selected ? label_selected_b : label_normal_b);
                f32 text_wx = PANEL_X - PAL_W * 0.5f + PAL_H + palette_icon_padding;
                AEGfxPrint(level_editor_font, palette_labels[i], text_wx / world_x_offset, norm_y,
                    1.f, lr, lg, lb, 1.f);

                if (is_locked)
                {
                    f32 hx = (PANEL_X + PAL_W * 0.5f - 45.f) / world_x_offset;
                    AEGfxPrint(level_editor_font, "1/1", hx, norm_y, 0.8f,
                        label_locked_count_r, label_locked_count_g, label_locked_count_b, 1.f);
                }
                else
                {
                    char hint[4];
                    snprintf(hint, sizeof(hint), "[%d]", i + 1);
                    f32 hx = (PANEL_X + PAL_W * 0.5f - 40.f) / world_x_offset;
                    AEGfxPrint(level_editor_font, hint, hx, norm_y, 1.f,
                        label_hotkey_r, label_hotkey_g, label_hotkey_b, 1.f);
                }
            }
        }
    }
    else if (editorState == EDIT_PUZZLES)
    {
        for (s32 i = 0; i < EDITABLE_PUZZLE_COUNT; ++i)
        {
            f32  py = PALETTE_TOP_Y - i * PAL_GAP;
            bool hovered = IsButtonHovered(PANEL_X, py, PAL_W, PAL_H);
            bool is_selected = (puzzle_palette_ids[i] == selected_puzzle);

            if (is_selected && TileTextures[2])
                render_editor_ui(TileTextures[2], PAL_W + palette_icon_padding, PAL_H + palette_icon_padding,
                    ghost_preview_alpha, PANEL_X, py);
            if (hovered && !is_selected && TileTextures[2])
                render_editor_ui(TileTextures[2], PAL_W, PAL_H, hover_alpha, PANEL_X, py);

            f32 icon_x = PANEL_X - PAL_W * 0.5f + PAL_H * 0.5f;
            if (puzzle_palette_ids[i] == Puzzle::PUZZLE_TYPE_COUNT)
                render_editor_ui(erase_tile, PAL_H - palette_icon_padding, PAL_H - palette_icon_padding,
                    1.f, icon_x, py);
            else if (Puzzle::puzzle_block_textures[puzzle_palette_ids[i]])
                render_editor_ui(Puzzle::puzzle_block_textures[puzzle_palette_ids[i]],
                    PAL_H - palette_icon_padding, PAL_H - palette_icon_padding, 1.f, icon_x, py);

            if (level_editor_font != -1)
            {
                f32 norm_y = (py - 10.f) / world_y_offset;
                f32 lr = is_selected ? label_selected_r : label_normal_r;
                f32 lg = is_selected ? label_selected_g : label_normal_g;
                f32 lb = is_selected ? label_selected_b : label_normal_b;
                f32 text_wx = PANEL_X - PAL_W * 0.5f + PAL_H + palette_icon_padding;
                AEGfxPrint(level_editor_font, puzzle_labels[i], text_wx / world_x_offset, norm_y,
                    1.f, lr, lg, lb, 1.f);

                char hint[4];
                snprintf(hint, sizeof(hint), "[%d]", i + 1);
                f32 hx = (PANEL_X + PAL_W * 0.5f - 40.f) / world_x_offset;
                AEGfxPrint(level_editor_font, hint, hx, norm_y, 1.f,
                    label_hotkey_r, label_hotkey_g, label_hotkey_b, 1.f);
            }
        }
    }

    // Header
    if (level_editor_font != -1)
    {
        f32 tw = 0.f, th = 0.f;
        AEGfxGetPrintSize(level_editor_font, tiles_header_label.c_str(), 1.f, &tw, &th);
        f32 norm_x = (PANEL_X / world_x_offset) - tw * 0.5f;
        f32 norm_y = (PALETTE_TOP_Y + 45.f) / world_y_offset;
        AEGfxPrint(level_editor_font, tiles_header_label.c_str(), norm_x, norm_y, 1.f,
            tiles_header_color_r, tiles_header_color_g, tiles_header_color_b, 1.f);
        render_text_center("________________", level_editor_font, norm_x + tw * 0.5f,
            norm_y - th * 0.5f, 1.f, { tiles_header_color_r, tiles_header_color_g, tiles_header_color_b });
    }

    // Hint bar
    if (level_editor_font != -1)
    {
        const char* hint = (editorState == EDIT_TILES) ? hint_tiles.c_str() : hint_puzzles.c_str();
        f32 norm_x = grid_start_x / world_x_offset;
        f32 norm_y = hint_bar_norm_y / world_y_offset;
        AEGfxPrint(level_editor_font, hint, norm_x, norm_y, 1.f,
            hint_bar_color_r, hint_bar_color_g, hint_bar_color_b, 1.f);
    }

    // Notifications
    if (save_msg_timer > 0.f)
    {
        active_status_msg =
            save_success ? "Map Saved!" :
            !wizard_placed ? "Place Wizard first!" :
            !knight_placed ? "Place Knight first!" :
            !door_placed ? "Place Door first!" : "Save FAILED!";
        active_status_error = !save_success;
        render_status_msg(level_editor_font, save_msg_timer, SAVE_MSG_DUR);
    }
    else if (spawn_error_timer > 0.f)
    {
        active_status_error = true;
        active_status_msg = "Cannot place ghost spawns on the last row!";
        render_status_msg(level_editor_font, spawn_error_timer, SPAWN_ERROR_DUR);
    }

    // Exit confirm popup
    if (show_exit_confirm)
        draw_confirm_popup(confirm_panel, confirm_btn_yes, confirm_btn_no,
            level_editor_font, confirm_label.c_str(), confirm_text_scale);

    // Cursor (always last)
    if (cursor)
    {
        f32 wx = (f32)mouse_x - world_x_offset;
        f32 wy = -(f32)mouse_y + world_y_offset;
        render_editor_ui(cursor, cursor_w, cursor_h, 1.f, wx + cursor_offset_x, wy - cursor_offset_y);
    }
}

// ----------------------------------------------------------------------------
// level_editor_free
// ----------------------------------------------------------------------------
void level_editor_free()
{
    free_mesh();

    if (editor_map)
    {
        for (s32 r = 0; r < TILE_ROWS; ++r) delete[] editor_map[r];
        delete[] editor_map;
        editor_map = nullptr;
    }
    if (puzzle_editor_map)
    {
        for (s32 r = 0; r < TILE_ROWS; ++r) delete[] puzzle_editor_map[r];
        delete[] puzzle_editor_map;
        puzzle_editor_map = nullptr;
    }

    if (level_editor_font != -1)
    {
        AEGfxDestroyFont(level_editor_font);
        level_editor_font = -1;
    }
}

// ----------------------------------------------------------------------------
// level_editor_unload
// ----------------------------------------------------------------------------
void level_editor_unload()
{
    free_tile_textures();
    Puzzle::unload_puzzle_textures();

    if (cursor) { AEGfxTextureUnload(cursor);                cursor = nullptr; }
    if (ghost_spawn) { AEGfxTextureUnload(ghost_spawn);           ghost_spawn = nullptr; }
    if (tomb) { AEGfxTextureUnload(tomb);                  tomb = nullptr; }
    if (bat_spawn) { AEGfxTextureUnload(bat_spawn);             bat_spawn = nullptr; }
    if (erase_tile) { AEGfxTextureUnload(erase_tile);            erase_tile = nullptr; }
    if (edit_panel) { AEGfxTextureUnload(edit_panel);            edit_panel = nullptr; }
    if (edit_panel_save) { AEGfxTextureUnload(edit_panel_save);       edit_panel_save = nullptr; }
    if (edit_panel_exit) { AEGfxTextureUnload(edit_panel_exit);       edit_panel_exit = nullptr; }
    if (edit_panel_button_left) { AEGfxTextureUnload(edit_panel_button_left);  edit_panel_button_left = nullptr; }
    if (edit_panel_button_right) { AEGfxTextureUnload(edit_panel_button_right); edit_panel_button_right = nullptr; }
    if (confirm_panel) { AEGfxTextureUnload(confirm_panel);         confirm_panel = nullptr; }
    if (confirm_btn_yes) { AEGfxTextureUnload(confirm_btn_yes);       confirm_btn_yes = nullptr; }
    if (confirm_btn_no) { AEGfxTextureUnload(confirm_btn_no);        confirm_btn_no = nullptr; }
}