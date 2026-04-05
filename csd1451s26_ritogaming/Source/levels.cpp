/*!************************************************************************
\file           levels.cpp
\author         Goh Qi En
\par DP email:  g.qien@digipen.edu

\brief          Defines the load/init/update/draw/free/unload cycle for
                the level select screen. Features a scrollable grid of
                levels (default + saved files), with play, edit, and
                delete actions per custom level cell. All asset paths and
                layout values are driven by levels_config.txt.
**************************************************************************/

#include "levels.hpp"
#include "renderer.hpp"
#include "AEEngine.h"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "utils.hpp"
#include "map.hpp"
#include "audio_manager.hpp"

#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <windows.h>

// ---------------------------------------------------------------------------
// Configuration — loaded from levels_config.txt
// ---------------------------------------------------------------------------
namespace
{
    struct LevelsConfig
    {
        // Asset paths
        std::string tex_cursor;
        std::string tex_button;
        std::string tex_back_button;
        std::string tex_editor_button;
        std::string tex_button_edit;
        std::string tex_bg_wall;
        std::string tex_scroll_bar;
        std::string tex_panel;
        std::string tex_confirm_yes;
        std::string tex_confirm_no;
        std::string font_path;
        s32         font_size;
        std::string tile_sprites_dir;

        // Grid layout
        s32   grid_cols;
        s32   grid_rows_visible;
        f32 cell_width;
        f32 cell_height;
        f32 cell_gap_x;
        f32 cell_gap_y;
        f32 grid_start_y;

        // Scrollbar
        f32 scrollbar_x;
        f32 scrollbar_top;
        f32 scrollbar_bot;
        f32 scrollbar_track_w;

        // Background
        f32 bg_color_r;
        f32 bg_color_g;
        f32 bg_color_b;
        f32 bg_tile_step;

        // Navigation buttons
        f32 back_btn_x, back_btn_y, back_btn_w, back_btn_h;
        f32 editor_btn_x, editor_btn_y, editor_btn_w, editor_btn_h;

        // Map preview tooltip
        f32 preview_w;
        f32 preview_h;

        // Scroll
        f32 scroll_lerp_speed;

        // Misc
        s32 default_level_count;

        // Derived — computed after loading
        f32 grid_start_x;
        f32 scrollbar_height;
    };

    // ---------------------------------------------------------------------------
    // Level entry — one cell in the grid
    // ---------------------------------------------------------------------------
    struct LevelEntry
    {
        std::string name;   // display name shown in the cell
        std::string path;   // file path to the level tile data
    };

    // ---------------------------------------------------------------------------
    // State
    // ---------------------------------------------------------------------------
    LevelsConfig    cfg = {};
    AEGfxTexture* tex_cursor = nullptr;
    AEGfxTexture* tex_button = nullptr;
    AEGfxTexture* tex_back_button = nullptr;
    AEGfxTexture* tex_editor_btn = nullptr;
    AEGfxTexture* tex_button_edit = nullptr;
    AEGfxTexture* tex_bg_wall = nullptr;
    AEGfxTexture* tex_scroll_bar = nullptr;
    AEGfxTexture* tex_panel = nullptr;
    AEGfxTexture* tex_confirm_yes = nullptr;
    AEGfxTexture* tex_confirm_no = nullptr;
    s8              font = -1;

    s32 mouse_x = 0, mouse_y = 0;

    std::vector<LevelEntry> levels;
    std::vector<LevelEntry> puzzles;

    s32   context_menu_idx = -1;
    f32 delete_msg_timer = 0.f;
    bool  show_delete_confirm = false;
    s32   pending_delete_idx = -1;

    f32 scroll_row_f = 0.f;
    f32 scroll_target = 0.f;
    s32   scroll_row = 0;

    s32 preview_map[TILE_ROWS][TILE_COLS]{};
    s32 preview_idx = -1;

    // screen 
    f32 half_screen_w;
    f32 half_screen_h;

} // namespace

// ----------------------------------------------------------------------------
//
// Function: load_config
// Reads levels_config.txt line by line and populates the LevelsConfig struct.
// Also computes derived fields (grid_start_x, scrollbar_height) after parsing.
//
// ----------------------------------------------------------------------------
static void load_config()
{
    std::ifstream file("Assets/Config/levels_config.txt");
    if (!file.is_open())
    {
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string key;
        if (!(iss >> key)) continue;

        // Read remainder of line as value — handles paths containing spaces
        std::string value;
        std::getline(iss, value);
        size_t start = value.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        value = value.substr(start);

        if (key == "tex_cursor")          cfg.tex_cursor = value;
        else if (key == "tex_button")          cfg.tex_button = value;
        else if (key == "tex_back_button")     cfg.tex_back_button = value;
        else if (key == "tex_editor_button")   cfg.tex_editor_button = value;
        else if (key == "tex_button_edit")     cfg.tex_button_edit = value;
        else if (key == "tex_bg_wall")         cfg.tex_bg_wall = value;
        else if (key == "tex_scroll_bar")      cfg.tex_scroll_bar = value;
        else if (key == "tex_panel")           cfg.tex_panel = value;
        else if (key == "tex_confirm_yes")     cfg.tex_confirm_yes = value;
        else if (key == "tex_confirm_no")      cfg.tex_confirm_no = value;
        else if (key == "font_path")           cfg.font_path = value;
        else if (key == "font_size")           cfg.font_size = std::stoi(value);
        else if (key == "tile_sprites_dir")    cfg.tile_sprites_dir = value;
        else if (key == "grid_cols")           cfg.grid_cols = std::stoi(value);
        else if (key == "grid_rows_visible")   cfg.grid_rows_visible = std::stoi(value);
        else if (key == "cell_width")          cfg.cell_width = std::stof(value);
        else if (key == "cell_height")         cfg.cell_height = std::stof(value);
        else if (key == "cell_gap_x")          cfg.cell_gap_x = std::stof(value);
        else if (key == "cell_gap_y")          cfg.cell_gap_y = std::stof(value);
        else if (key == "grid_start_y")        cfg.grid_start_y = std::stof(value);
        else if (key == "scrollbar_x")         cfg.scrollbar_x = std::stof(value);
        else if (key == "scrollbar_top")       cfg.scrollbar_top = std::stof(value);
        else if (key == "scrollbar_bot")       cfg.scrollbar_bot = std::stof(value);
        else if (key == "scrollbar_track_w")   cfg.scrollbar_track_w = std::stof(value);
        else if (key == "default_level_count") cfg.default_level_count = std::stoi(value);
        else if (key == "bg_color_r")          cfg.bg_color_r = std::stof(value);
        else if (key == "bg_color_g")          cfg.bg_color_g = std::stof(value);
        else if (key == "bg_color_b")          cfg.bg_color_b = std::stof(value);
        else if (key == "bg_tile_step")        cfg.bg_tile_step = std::stof(value);
        else if (key == "back_btn_x")          cfg.back_btn_x = std::stof(value);
        else if (key == "back_btn_y")          cfg.back_btn_y = std::stof(value);
        else if (key == "back_btn_w")          cfg.back_btn_w = std::stof(value);
        else if (key == "back_btn_h")          cfg.back_btn_h = std::stof(value);
        else if (key == "editor_btn_x")        cfg.editor_btn_x = std::stof(value);
        else if (key == "editor_btn_y")        cfg.editor_btn_y = std::stof(value);
        else if (key == "editor_btn_w")        cfg.editor_btn_w = std::stof(value);
        else if (key == "editor_btn_h")        cfg.editor_btn_h = std::stof(value);
        else if (key == "preview_w")           cfg.preview_w = std::stof(value);
        else if (key == "preview_h")           cfg.preview_h = std::stof(value);
        else if (key == "scroll_lerp_speed")   cfg.scroll_lerp_speed = std::stof(value);
    }

    // grid_start_x centers the grid horizontally based on column count and cell size
    cfg.grid_start_x = -(cfg.grid_cols * cfg.cell_width - (cfg.grid_cols + 1) * cfg.cell_gap_x) / 2.f;
    cfg.scrollbar_height = cfg.scrollbar_top - cfg.scrollbar_bot;
}

// ----------------------------------------------------------------------------
//
// Function: scan_level_files
// Searches a directory for files matching a Win32 wildcard pattern.
// Skips any file whose name contains a substring in skip_if_contains.
//
// ----------------------------------------------------------------------------
static std::vector<std::string> scan_level_files(const char* pattern,
    const std::vector<std::string>& skip_if_contains)
{
    std::vector<std::string> result;
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return result;

    do
    {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

        std::string name = fd.cFileName;
        bool        skip = false;
        for (const auto& token : skip_if_contains)
        {
            if (name.find(token) != std::string::npos) { skip = true; break; }
        }
        if (!skip)
            result.push_back(std::string("Assets/Levels/") + name);
    } while (FindNextFileA(h, &fd));

    FindClose(h);
    return result;
}

// ----------------------------------------------------------------------------
//
// Function: extract_level_num
// Parses the numeric ID embedded in a tile level path (e.g. "lvl3.txt" -> 3).
// Used as the sort key when ordering the level list numerically.
//
// ----------------------------------------------------------------------------
static s32 extract_level_num(const std::string& path)
{
    size_t start = path.find("lvl") + 3;
    size_t end = path.find(".txt");
    return std::stoi(path.substr(start, end - start));
}

// ----------------------------------------------------------------------------
//
// Function: extract_puzzle_num
// Parses the numeric ID embedded in a puzzle level path (e.g. "lvl2_puzzles.txt" -> 2).
// Uses rfind to locate the last 'l' before "_puzzles" to handle prefixes correctly.
//
// ----------------------------------------------------------------------------
static s32 extract_puzzle_num(const std::string& path)
{
    size_t end = path.find("_puzzles.txt");
    size_t start = path.rfind("l", end) + 1; // search backwards to avoid matching earlier 'l' characters
    return std::stoi(path.substr(start, end - start));
}

// ----------------------------------------------------------------------------
//
// Function: total_rows
// Returns the number of grid rows needed to display all levels,
// rounding up so the last partial row is still counted.
//
// ----------------------------------------------------------------------------
static s32 total_rows()
{
    return ((int)levels.size() + cfg.grid_cols - 1) / cfg.grid_cols;
}

// ----------------------------------------------------------------------------
//
// Function: max_scroll
// Returns the highest valid scroll row index so the grid never scrolls
// past its last row of content.
//
// ----------------------------------------------------------------------------
static s32 max_scroll()
{
    s32 m = total_rows() - cfg.grid_rows_visible;
    return m < 0 ? 0 : m;
}

// ----------------------------------------------------------------------------
//
// Function: sync_global_paths
// Copies the local levels and puzzles path data into the global vectors
// so other game states (e.g. level1) can navigate between levels via doors.
//
// ----------------------------------------------------------------------------
static void sync_global_paths()
{
    level_paths.clear();
    for (const auto& e : levels) level_paths.push_back(e.path);

    level_puzzle_paths.clear();
    for (const auto& e : puzzles) level_puzzle_paths.push_back(e.path);
}

// ----------------------------------------------------------------------------
//
// Function: build_level_list
// Clears and repopulates the levels and puzzles vectors by scanning the
// Assets/Levels directory. Entries are sorted numerically and assigned
// user-facing display names ("Level 2", "Level 3", etc.).
//
// ----------------------------------------------------------------------------
static void build_level_list()
{
    levels.clear();
    puzzles.clear();

    // Index 0 is always the built-in default level
    levels.push_back({ "Level 1",  "Assets/Levels/default.txt" });
    puzzles.push_back({ "Level 1", "Assets/Levels/puzzle.txt" });

    // --- Tile levels ---
    auto level_paths_raw = scan_level_files("Assets/Levels/lvl*.txt", { "_puzzles", "_enemy_spawns" });
    std::sort(level_paths_raw.begin(), level_paths_raw.end(),
        [](const std::string& a, const std::string& b)
        { return extract_level_num(a) < extract_level_num(b); });

    s32 level_num = 2;
    for (const auto& path : level_paths_raw)
        levels.push_back({ "Level " + std::to_string(level_num++), path });

    // --- Puzzle levels ---
    auto puzzle_paths_raw = scan_level_files("Assets/Levels/l*_puzzles.txt", {});
    std::sort(puzzle_paths_raw.begin(), puzzle_paths_raw.end(),
        [](const std::string& a, const std::string& b)
        { return extract_puzzle_num(a) < extract_puzzle_num(b); });

    s32 puzzle_num = 1;
    for (const auto& path : puzzle_paths_raw)
        puzzles.push_back({ "Puzzle " + std::to_string(puzzle_num++), path });
}

// ----------------------------------------------------------------------------
//
// Function: levels_load
// Reads levels_config.txt, then loads all textures, the font, and tile
// sprites referenced by the config into GPU memory.
//
// ----------------------------------------------------------------------------
void levels_load()
{
    load_config();

    tex_cursor = AEGfxTextureLoad(cfg.tex_cursor.c_str());
    tex_button = AEGfxTextureLoad(cfg.tex_button.c_str());
    tex_back_button = AEGfxTextureLoad(cfg.tex_back_button.c_str());
    tex_editor_btn = AEGfxTextureLoad(cfg.tex_editor_button.c_str());
    tex_button_edit = AEGfxTextureLoad(cfg.tex_button_edit.c_str());
    tex_bg_wall = AEGfxTextureLoad(cfg.tex_bg_wall.c_str());
    tex_scroll_bar = AEGfxTextureLoad(cfg.tex_scroll_bar.c_str());
    tex_panel = AEGfxTextureLoad(cfg.tex_panel.c_str());
    tex_confirm_yes = AEGfxTextureLoad(cfg.tex_confirm_yes.c_str());
    tex_confirm_no = AEGfxTextureLoad(cfg.tex_confirm_no.c_str());
    SILENT_CALL(font = AEGfxCreateFont(cfg.font_path.c_str(), cfg.font_size));

    load_tile_textures(cfg.tile_sprites_dir.c_str());
}

// ----------------------------------------------------------------------------
//
// Function: levels_init
// Resets scroll position and UI flags, then rebuilds the level list from
// disk and syncs the global path vectors for door navigation.
//
// ----------------------------------------------------------------------------
void levels_init()
{
    init_mesh();
    AEGfxSetBackgroundColor(cfg.bg_color_r, cfg.bg_color_g, cfg.bg_color_b);
    AEInputShowCursor(false);

    scroll_row = 0;
    scroll_row_f = 0.f;
    scroll_target = 0.f;

    show_delete_confirm = false;
    pending_delete_idx = -1;

    half_screen_w = AEGfxGetWindowWidth() / 2.0f;
    half_screen_h = AEGfxGetWindowHeight() / 2.0f;

    build_level_list();
    sync_global_paths();
}

// ----------------------------------------------------------------------------
//
// Function: levels_update
// Handles all per-frame input: delete modal, navigation buttons, left-click
// play/context-menu, mouse wheel scroll, scrollbar drag, and right-click
// context menu toggle.
//
// ----------------------------------------------------------------------------
void levels_update()
{
    dt = (f32)AEFrameRateControllerGetFrameTime();
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    // --- Button hover sound ---
    static s32 last_hovered = -1;
    s32 current_hovered = -1;
    for (s32 row = 0; row < cfg.grid_rows_visible; ++row)
    {
        for (s32 col = 0; col < cfg.grid_cols; ++col)
        {
            s32   idx = (scroll_row + row) * cfg.grid_cols + col;
            if (idx >= (int)levels.size()) continue;
            f32 cx = cfg.grid_start_x + col * (cfg.cell_width + cfg.cell_gap_x);
            f32 cy = cfg.grid_start_y - row * (cfg.cell_height + cfg.cell_gap_y);
            if (IsButtonHovered(cx, cy, cfg.cell_width, cfg.cell_height))
            {
                current_hovered = idx;
                break;
            }
        }
    }
    if (IsButtonHovered(cfg.back_btn_x, cfg.back_btn_y, cfg.back_btn_w, cfg.back_btn_h))
        current_hovered = 9998;
    if (IsButtonHovered(cfg.editor_btn_x, cfg.editor_btn_y, cfg.editor_btn_w, cfg.editor_btn_h))
        current_hovered = 9999;

    if (current_hovered != last_hovered && current_hovered != -1)
        play_sfx(g_audio.sfx_button_hover);
    last_hovered = current_hovered;


    // Modal popup takes priority — all other input is blocked while it is open
    if (show_delete_confirm)
    {
        s32 result = draw_confirm_popup(tex_panel, tex_confirm_yes, tex_confirm_no,
            font, "Delete this level?", 0.9f);

        if (result == 1 && pending_delete_idx >= 0)
        {
            s32 idx = pending_delete_idx;
            remove(levels[idx].path.c_str());
            remove(puzzles[idx].path.c_str());

            // Also remove the companion enemy spawn file if it exists
            std::string spawn_path = levels[idx].path.substr(0, levels[idx].path.find_last_of('.'))
                + "_enemy_spawns.txt";
            remove(spawn_path.c_str());

            build_level_list();
            sync_global_paths();

            scroll_row = 0; scroll_row_f = 0.f; scroll_target = 0.f;
            delete_msg_timer = 0.8f;
            pending_delete_idx = -1;
            show_delete_confirm = false;
        }
        else if (result == 0)
        {
            show_delete_confirm = false;
            pending_delete_idx = -1;
        }
        return;
    }

    // --- Navigation ---
    if (AEInputCheckTriggered(AEVK_ESCAPE))
        next = MAINMENU;

    if (IsButtonClicked(cfg.back_btn_x, cfg.back_btn_y, cfg.back_btn_w, cfg.back_btn_h))
    {
        play_sfx(g_audio.sfx_button_click, 0.1f);
        next = MAINMENU;
    }
    if (IsButtonClicked(cfg.editor_btn_x, cfg.editor_btn_y, cfg.editor_btn_w, cfg.editor_btn_h))
    {
        play_sfx(g_audio.sfx_button_click, 0.1f);
        selected_level_path = "";
        selected_level_puzzle_path = "";
        next = GS_LEVEL_EDITOR;
    }

    // --- Left click: context menu actions or play ---
    if (AEInputCheckTriggered(AEVK_LBUTTON))
    {
        if (context_menu_idx >= 0 && context_menu_idx < (int)levels.size())
        {
            s32   idx = context_menu_idx;
            s32   col = idx % cfg.grid_cols;
            s32   row = (idx / cfg.grid_cols) - scroll_row;
            f32 cx = cfg.grid_start_x + col * (cfg.cell_width + cfg.cell_gap_x);
            f32 cy = cfg.grid_start_y - row * (cfg.cell_height + cfg.cell_gap_y);

            f32 edit_x = cx, edit_y = cy + cfg.cell_height * 0.15f;
            if (IsButtonClicked(edit_x, edit_y, cfg.cell_width - 40.f, 50.f))
            {
                play_sfx(g_audio.sfx_button_click, 0.1f);
                selected_level_path = levels[idx].path;
                selected_level_puzzle_path = puzzles[idx].path;
                context_menu_idx = -1;
                next = GS_LEVEL_EDITOR;
                return;
            }

            f32 del_x = cx, del_y = cy - cfg.cell_height * 0.25f;
            if (IsButtonClicked(del_x, del_y, cfg.cell_width - 40.f, 50.f))
            {
                play_sfx(g_audio.sfx_button_click, 0.1f);
                pending_delete_idx = idx;
                show_delete_confirm = true;
                context_menu_idx = -1;
                return;
            }

            context_menu_idx = -1; // clicked outside context menu — close it
        }

        for (s32 row = 0; row < cfg.grid_rows_visible; ++row)
        {
            for (s32 col = 0; col < cfg.grid_cols; ++col)
            {
                s32   idx = (scroll_row + row) * cfg.grid_cols + col;
                if (idx >= (int)levels.size()) continue;

                f32 cx = cfg.grid_start_x + col * (cfg.cell_width + cfg.cell_gap_x);
                f32 cy = cfg.grid_start_y - row * (cfg.cell_height + cfg.cell_gap_y);

                if (IsButtonClicked(cx, cy, cfg.cell_width, cfg.cell_height))
                {
                    play_sfx(g_audio.sfx_button_click, 0.1f);
                    selected_level_path = levels[idx].path;
                    selected_level_puzzle_path = puzzles[idx].path;
                    next = GS_LEVEL1;
                }
            }
        }
    }

    // --- Mouse wheel scroll ---
    s32 wheel = 0;
    AEInputMouseWheelDelta(&wheel);
    if (wheel > 0) scroll_target = scroll_target > 0.f ? scroll_target - 1.f : 0.f;
    if (wheel < 0) scroll_target = scroll_target < (f32)max_scroll() ? scroll_target + 1.f : (f32)max_scroll();

    // --- Scrollbar drag ---
    f32 sx = (float)mouse_x - half_screen_w;
    f32 sy = -(float)mouse_y + half_screen_h;
    bool on_bar = (sx >= cfg.scrollbar_x - cfg.scrollbar_track_w * 0.5f &&
        sx <= cfg.scrollbar_x + cfg.scrollbar_track_w * 0.5f &&
        sy <= cfg.scrollbar_top && sy >= cfg.scrollbar_bot);

    if (on_bar && AEInputCheckCurr(AEVK_LBUTTON))
    {
        s32 ms = max_scroll();
        if (ms > 0)
        {
            // Map the mouse Y position within the bar track to a scroll row target
            f32 ratio = (cfg.scrollbar_top - sy) / cfg.scrollbar_height;
            f32 raw_target = ratio * (float)ms;
            if (raw_target < 0.f)       scroll_target = 0.f;
            else if (raw_target > (float)ms) scroll_target = (float)ms;
            else                             scroll_target = raw_target;
        }
    }

    // Lerp towards scroll target each frame, then snap to nearest integer row for grid indexing
    scroll_row_f += (scroll_target - scroll_row_f) * cfg.scroll_lerp_speed * dt;
    scroll_row = (int)(scroll_row_f + 0.5f);

    if (delete_msg_timer > 0.f) delete_msg_timer -= dt;

    // --- Right click: open/close context menu for custom levels ---
    if (AEInputCheckTriggered(AEVK_RBUTTON))
    {
        f32 mx = (float)mouse_x - half_screen_w;
        f32 my = -(float)mouse_y + half_screen_h;
        bool  clicked_any = false;

        for (s32 row = 0; row < cfg.grid_rows_visible; ++row)
        {
            for (s32 col = 0; col < cfg.grid_cols; ++col)
            {
                s32 idx = (scroll_row + row) * cfg.grid_cols + col;
                if (idx >= (int)levels.size())     continue;
                if (idx < cfg.default_level_count) continue; // built-in levels cannot be edited/deleted

                f32 cx = cfg.grid_start_x + col * (cfg.cell_width + cfg.cell_gap_x);
                f32 cy = cfg.grid_start_y - row * (cfg.cell_height + cfg.cell_gap_y);

                bool in_cell = (mx >= cx - cfg.cell_width * 0.5f && mx <= cx + cfg.cell_width * 0.5f &&
                    my >= cy - cfg.cell_height * 0.5f && my <= cy + cfg.cell_height * 0.5f);
                if (in_cell)
                {
                    // Toggle: right-clicking an already-open menu closes it
                    context_menu_idx = (context_menu_idx == idx) ? -1 : idx;
                    clicked_any = true;
                }
            }
        }
        if (!clicked_any) context_menu_idx = -1;
    }
}

// ----------------------------------------------------------------------------
//
// Function: levels_draw
// Renders all UI elements in depth order: background, grid cells, scrollbar,
// navigation buttons, delete toast, map preview tooltip, and delete modal.
//
// ----------------------------------------------------------------------------
void levels_draw()
{
    // --- Tiled background ---
    if (tex_bg_wall)
    {
        for (f32 x = -half_screen_w; x <= half_screen_w; x += cfg.bg_tile_step)
            for (f32 y = -450.f; y <= 450.f; y += cfg.bg_tile_step)
                render_editor_ui(tex_bg_wall, cfg.bg_tile_step, cfg.bg_tile_step, 0.3f, x, y);
    }

    // --- Title ---
    if (font != -1)
    {
        const char* title = "-- SELECT LEVEL --";
        f32 tw = 0.f, th = 0.f;
        AEGfxGetPrintSize(font, title, 1.2f, &tw, &th);
        AEGfxPrint(font, title, -tw * 0.5f, 0.9f, 1.2f, 0.9f, 0.8f, 0.3f, 1.f);
    }

    // --- Level grid ---
    for (s32 row = 0; row < cfg.grid_rows_visible; ++row)
    {
        for (s32 col = 0; col < cfg.grid_cols; ++col)
        {
            s32   idx = (scroll_row + row) * cfg.grid_cols + col;
            f32 cx = cfg.grid_start_x + col * (cfg.cell_width + cfg.cell_gap_x);
            f32 cy = cfg.grid_start_y - row * (cfg.cell_height + cfg.cell_gap_y);

            if (idx >= (int)levels.size())
            {
                // Empty placeholder cell
                if (tex_button)
                    render_editor_ui(tex_button, cfg.cell_width, cfg.cell_height, 0.2f, cx, cy);
                continue;
            }

            bool context_open = (context_menu_idx == idx);

            if (!context_open)
            {
                bool hovered = !show_delete_confirm && IsButtonHovered(cx, cy, cfg.cell_width, cfg.cell_height);
                if (tex_button)
                    render_editor_ui(tex_button, cfg.cell_width, cfg.cell_height, 1.f, cx, cy, hovered);

                if (font != -1)
                {
                    f32 name_tw = 0.f, name_th = 0.f;
                    AEGfxGetPrintSize(font, levels[idx].name.c_str(), 1.f, &name_tw, &name_th);
                    AEGfxPrint(font, levels[idx].name.c_str(),
                        (cx / half_screen_w) - name_tw * 0.5f, (cy - 10.f) / half_screen_h,
                        1.f, 0.f, 0.f, 0.f, 1.f);

                    if (idx >= cfg.default_level_count)
                    {
                        // Custom level badge and right-click hint
                        f32 custom_tw = 0.f, custom_th = 0.f;
                        AEGfxGetPrintSize(font, "[Custom]", 0.7f, &custom_tw, &custom_th);
                        AEGfxPrint(font, "[Custom]",
                            (cx / half_screen_w) - custom_tw * 0.5f, (cy - 40.f) / half_screen_h,
                            0.7f, 0.5f, 0.9f, 0.5f, 1.f);

                        f32 hint_tw = 0.f, hint_th = 0.f;
                        AEGfxGetPrintSize(font, "[RMB]", 0.5f, &hint_tw, &hint_th);
                        AEGfxPrint(font, "[RMB]",
                            (cx / half_screen_w) - hint_tw * 0.5f, (cy - 58.f) / half_screen_h,
                            0.5f, 0.4f, 0.4f, 0.4f, 1.f);
                    }
                }
            }
            else
            {
                // Context menu face — shows Edit and Delete actions
                if (tex_button)
                    render_editor_ui(tex_button, cfg.cell_width, cfg.cell_height, 0.75f, cx, cy);

                if (font != -1)
                {
                    f32 edit_x = cx, edit_y = cy + cfg.cell_height * 0.15f;
                    bool  edit_h = !show_delete_confirm &&
                        IsButtonHovered(edit_x, edit_y, cfg.cell_width * 0.8f, cfg.cell_height * 0.3f);
                    if (tex_button_edit)
                        render_editor_ui(tex_button_edit, cfg.cell_width * 0.8f, cfg.cell_height * 0.3f,
                            edit_h ? 1.f : 0.7f, edit_x, edit_y);

                    f32 edit_tw = 0.f, edit_th = 0.f;
                    AEGfxGetPrintSize(font, "Edit", 0.9f, &edit_tw, &edit_th);
                    AEGfxPrint(font, "Edit",
                        (edit_x / half_screen_w) - edit_tw * 0.5f, (edit_y - 10.f) / half_screen_h,
                        0.9f, 0.f, 0.f, 0.f, 1.f);

                    f32 del_x = cx, del_y = cy - cfg.cell_height * 0.20f;
                    bool  del_h = !show_delete_confirm &&
                        IsButtonHovered(del_x, del_y, cfg.cell_width * 0.8f, cfg.cell_height * 0.3f);
                    if (tex_button_edit)
                        render_editor_ui(tex_button_edit, cfg.cell_width * 0.8f, cfg.cell_height * 0.3f,
                            del_h ? 1.f : 0.7f, del_x, del_y);

                    f32 del_tw = 0.f, del_th = 0.f;
                    AEGfxGetPrintSize(font, "Delete", 0.9f, &del_tw, &del_th);
                    AEGfxPrint(font, "Delete",
                        (del_x / half_screen_w) - del_tw * 0.5f, (del_y - 10.f) / half_screen_h,
                        0.9f, 1.f, 0.3f, 0.3f, 1.f);
                }
            }
        }
    }

    // --- Scrollbar ---
    if (total_rows() > cfg.grid_rows_visible)
    {
        f32 bar_center_y = (cfg.scrollbar_top + cfg.scrollbar_bot) * 0.5f;
        if (tex_scroll_bar)
            render_editor_ui(tex_scroll_bar, cfg.scrollbar_track_w, cfg.scrollbar_height,
                0.4f, cfg.scrollbar_x, bar_center_y);

        // Thumb height is proportional to visible rows vs total rows
        s32   ms = max_scroll();
        f32 thumb_h = cfg.scrollbar_height * ((float)cfg.grid_rows_visible / (float)total_rows());
        f32 thumb_ratio = (ms > 0) ? scroll_row_f / (float)ms : 0.f;
        f32 thumb_cy = cfg.scrollbar_top - thumb_h * 0.5f - thumb_ratio * (cfg.scrollbar_height - thumb_h);

        if (tex_scroll_bar)
            render_editor_ui(tex_scroll_bar, cfg.scrollbar_track_w, thumb_h, 1.f, cfg.scrollbar_x, thumb_cy);
    }

    // --- Back button ---
    bool back_hovered = !show_delete_confirm &&
        IsButtonHovered(cfg.back_btn_x, cfg.back_btn_y, cfg.back_btn_w, cfg.back_btn_h);
    if (tex_back_button)
        render_editor_ui(tex_back_button, cfg.back_btn_w, cfg.back_btn_h, 1.f,
            cfg.back_btn_x, cfg.back_btn_y, back_hovered);

    if (font != -1)
    {
        f32 back_tw = 0.f, back_th = 0.f;
        AEGfxGetPrintSize(font, "Back", 0.8f, &back_tw, &back_th);
        AEGfxPrint(font, "Back",
            (cfg.back_btn_x / half_screen_w) - back_tw * 0.5f, (cfg.back_btn_y - 10.f) / half_screen_h,
            0.8f, 0.f, 0.f, 0.f, 1.f);
    }

    // --- Editor button ---
    bool editor_hovered = !show_delete_confirm &&
        IsButtonHovered(cfg.editor_btn_x, cfg.editor_btn_y, cfg.editor_btn_w, cfg.editor_btn_h);
    if (tex_editor_btn)
        render_editor_ui(tex_editor_btn, cfg.editor_btn_w, cfg.editor_btn_h, 1.f,
            cfg.editor_btn_x, cfg.editor_btn_y, editor_hovered);

    if (font != -1)
    {
        f32 editor_tw = 0.f, editor_th = 0.f;
        AEGfxGetPrintSize(font, "Editor", 0.8f, &editor_tw, &editor_th);
        AEGfxPrint(font, "Editor",
            (cfg.editor_btn_x / half_screen_w) - editor_tw * 0.5f, (cfg.editor_btn_y - 10.f) / half_screen_h,
            0.8f, 0.9f, 0.7f, 0.3f, 1.f);
    }

    // --- Delete toast notification ---
    if (delete_msg_timer > 0.f && font != -1)
    {
        // Fade out during the last 0.5 seconds of the timer
        f32 toast_alpha = delete_msg_timer > 0.5f ? 1.f : delete_msg_timer / 0.5f;
        const char* msg = "Level deleted!";
        f32 msg_tw = 0.f, msg_th = 0.f;
        AEGfxGetPrintSize(font, msg, 1.f, &msg_tw, &msg_th);
        render_editor_ui(tex_button, 400.f, 80.f, 0.9f, 0.f, 0.f);
        AEGfxPrint(font, msg, -msg_tw * 0.5f, -0.05f, 1.f, 1.f, 0.3f, 0.3f, toast_alpha);
    }

    // --- Map preview tooltip on hover ---
    if (!show_delete_confirm)
    {
        for (s32 row = 0; row < cfg.grid_rows_visible; ++row)
        {
            for (s32 col = 0; col < cfg.grid_cols; ++col)
            {
                s32   idx = (scroll_row + row) * cfg.grid_cols + col;
                if (idx >= (int)levels.size()) continue;

                f32 cx = cfg.grid_start_x + col * (cfg.cell_width + cfg.cell_gap_x);
                f32 cy = cfg.grid_start_y - row * (cfg.cell_height + cfg.cell_gap_y);

                if (!IsButtonHovered(cx, cy, cfg.cell_width, cfg.cell_height)) continue;
                if (context_menu_idx == idx) break;

                // Only reload the preview map when hovering over a different cell
                if (preview_idx != idx)
                {
                    preview_idx = idx;
                    for (s32 r = 0; r < TILE_ROWS; ++r)
                        for (s32 c = 0; c < TILE_COLS; ++c)
                            preview_map[r][c] = 0;

                    load_map_from_file(levels[idx].path.c_str(), preview_map);
                }

                f32 wx = (float)mouse_x - half_screen_w, wy = -(float)mouse_y + half_screen_h;
                f32 px = wx - cfg.preview_w * 0.5f - 10.f;
                f32 py = wy - cfg.preview_h * 0.5f - 10.f;

                if (tex_button)
                    render_editor_ui(tex_button, cfg.preview_w + 16.f, cfg.preview_h + 16.f, 0.95f, px, py);

                draw_map_preview(preview_map, px, py, cfg.preview_w, cfg.preview_h);

                if (font != -1)
                {
                    f32 label_tw = 0.f, label_th = 0.f;
                    AEGfxGetPrintSize(font, levels[idx].name.c_str(), 0.8f, &label_tw, &label_th);
                    AEGfxPrint(font, levels[idx].name.c_str(),
                        (px / half_screen_w) - label_tw * 0.5f, (py + cfg.preview_h * 0.5f + 25.f) / half_screen_h,
                        0.8f, 1.f, 1.f, 1.f, 1.f);
                }
                break;
            }
        }
    }

    // --- Delete confirmation modal (drawn last, on top of everything) ---
    if (show_delete_confirm)
        draw_confirm_popup(tex_panel, tex_confirm_yes, tex_confirm_no, font, "Delete this level?", 0.9f);

    // --- Custom cursor ---
    if (tex_cursor)
    {
        f32 wx = (float)mouse_x - half_screen_w, wy = -(float)mouse_y + half_screen_h;
        render_editor_ui(tex_cursor, 50.f, 50.f, 1.f, wx + 25.f, wy - 25.f);
    }
}

// ----------------------------------------------------------------------------
//
// Function: levels_free
// Releases the vertex mesh and clears the level and puzzle entry vectors.
//
// ----------------------------------------------------------------------------
void levels_free()
{
    free_mesh();
    levels.clear();
    puzzles.clear();
}

// ----------------------------------------------------------------------------
//
// Function: levels_unload
// Unloads all textures and the font from GPU memory and nullifies their
// pointers to prevent use-after-free on re-entry.
//
// ----------------------------------------------------------------------------
void levels_unload()
{
    if (tex_cursor) { AEGfxTextureUnload(tex_cursor);      tex_cursor = nullptr; }
    if (tex_button) { AEGfxTextureUnload(tex_button);      tex_button = nullptr; }
    if (tex_back_button) { AEGfxTextureUnload(tex_back_button); tex_back_button = nullptr; }
    if (tex_editor_btn) { AEGfxTextureUnload(tex_editor_btn);  tex_editor_btn = nullptr; }
    if (tex_button_edit) { AEGfxTextureUnload(tex_button_edit); tex_button_edit = nullptr; }
    if (tex_bg_wall) { AEGfxTextureUnload(tex_bg_wall);     tex_bg_wall = nullptr; }
    if (tex_scroll_bar) { AEGfxTextureUnload(tex_scroll_bar);  tex_scroll_bar = nullptr; }
    if (tex_panel) { AEGfxTextureUnload(tex_panel);       tex_panel = nullptr; }
    if (tex_confirm_yes) { AEGfxTextureUnload(tex_confirm_yes); tex_confirm_yes = nullptr; }
    if (tex_confirm_no) { AEGfxTextureUnload(tex_confirm_no);  tex_confirm_no = nullptr; }
    if (font != -1) { AEGfxDestroyFont(font);              font = -1; }
    free_tile_textures();
}

// ----------------------------------------------------------------------------
//
// Function: ensure_level_paths_loaded
// Populates global level_paths/level_puzzle_paths if they are empty.
// Called from level1_init() so door navigation works even if the player
// enters a level directly without visiting the level select screen.
//
// ----------------------------------------------------------------------------
void ensure_level_paths_loaded()
{
    if (!level_paths.empty()) return;

    build_level_list();
    sync_global_paths();
}