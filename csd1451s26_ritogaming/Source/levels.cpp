/*!************************************************************************
\file:          levels.cpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         This file defines the 6 cycle functions that load, initialize,
                update, draw, free, and unload the level select interface.
                Features a 3x3 scrollable grid of levels (default + saved files),
                with play, edit, and delete buttons per custom level cell.
**************************************************************************/

#include "renderer.hpp"
#include "AEEngine.h"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "utils.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <windows.h>
#include "map.hpp"  
#include "audio_manager.hpp"

namespace {

    // --- Textures and font ---
    AEGfxTexture* cursor = nullptr;
    AEGfxTexture* button = nullptr;
    AEGfxTexture* bg_wall = nullptr;
    AEGfxTexture* scroll_bar = nullptr;
    static s8 font = -1;

    // --- Mouse position (screen space) ---
    s32 mouse_x, mouse_y;

    // --- Grid layout constants ---
    const int  COLS = 3;
    const int  ROWS_VISIBLE = 3;
    const f32  CELL_W = 280.f;
    const f32  CELL_H = 150.f;
    const f32  CELL_GAP_X = 40.f;
    const f32  CELL_GAP_Y = 30.f;
    const f32  GRID_START_X = -360.f;
    const f32  GRID_START_Y = 230.f;

    // --- Scrollbar constants ---
    const f32 BAR_X = 750.f;
    const f32 BAR_TOP = 300.f;
    const f32 BAR_BOT = -300.f;
    const f32 BAR_H = BAR_TOP - BAR_BOT;
    const f32 TRACK_W = 20.f;
    const f32 THUMB_W = 20.f;

    // --- Smooth scroll state ---
    f32 scroll_row_f = 0.f;  // float version for smooth lerp
    f32 scroll_target = 0.f;  // target row to lerp towards
    int scroll_row = 0;    // snapped integer row for grid indexing

    // --- Level entry definition ---
    struct LevelEntry {
        std::string name; // display name shown in the cell
        std::string path; // file path � empty means default built-in level
    };

    // All levels to display (default + scanned saved files)
    std::vector<LevelEntry> levels;
    std::vector<LevelEntry> puzzles;

    // Default built-in level shown at index 0
    const char* default_levels[] = { "Level 1" };
    const int   default_count = 1;

	//context pop up
    int context_menu_idx = -1; // which cell has context menu open, -1 = none
    f32 delete_msg_timer = 0.f;

    // --- Map preview ---
    s32 preview_map[TILE_ROWS][TILE_COLS]{};
    int preview_idx = -1;



} // namespace

// -------------------------------------------------------
// Helper: total rows needed to display all levels
// -------------------------------------------------------
static int total_rows()
{
    return ((int)levels.size() + COLS - 1) / COLS;
}

// -------------------------------------------------------
// Helper: maximum scroll row before hitting the bottom
// -------------------------------------------------------
static int max_scroll()
{
    int m = total_rows() - ROWS_VISIBLE;
    return m < 0 ? 0 : m;
}

// -------------------------------------------------------
// Scan Assets/Levels/lvl*.txt and populate levels vector
// -------------------------------------------------------
static void build_level_list()
{
    levels.clear();
    puzzles.clear();

    // Always add default levels at the top
    for (int i = 0; i < default_count; ++i)
    {
        LevelEntry e;
        e.name = default_levels[i];
        e.path = "Assets/Levels/default.txt";
        levels.push_back(e);

        LevelEntry pe;
        pe.name = default_levels[i];
        pe.path = "Assets/Levels/puzzle.txt";
        puzzles.push_back(pe);
    }

    // Win32 directory scan for saved custom level files
	WIN32_FIND_DATAA find_data; // windows struct to hold file info
    // file search handle that stores the state of the search, initialized to the first found file matching the pattern
	HANDLE hFind = FindFirstFileA("Assets/Levels/lvl*.txt", &find_data); 

    std::vector<LevelEntry> saved_levels;
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                std::string filename = find_data.cFileName;
                LevelEntry e;
                e.name = filename.substr(0, filename.find_last_of('.'));
                e.path = std::string("Assets/Levels/") + filename;
                saved_levels.push_back(e);
            }
        } while (FindNextFileA(hFind, &find_data));

        FindClose(hFind);
    }

    // Sort numerically (lvl1 < lvl2 < ... < lvl10 < lvl11)
    std::sort(saved_levels.begin(), saved_levels.end(),
        [](const LevelEntry& a, const LevelEntry& b)
        {
            auto get_num = [](const std::string& s)
                {
                    size_t start = s.find("lvl") + 3;
                    size_t end = s.find(".txt");
                    return std::stoi(s.substr(start, end - start));
                };
            return get_num(a.path) < get_num(b.path);
        });

    // Assign user-friendly display names starting at "Level 1"
    int level_num = 2;
    for (auto& e : saved_levels)
    {
        e.name = "Level " + std::to_string(level_num++);
        levels.push_back(e);
    }






    // Win32 directory scan for saved custom level files
    // file search handle that stores the state of the search, initialized to the first found file matching the pattern
    hFind = FindFirstFileA("Assets/Levels/l*_puzzles.txt", &find_data);

    std::vector<LevelEntry> saved_puzzles;
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                std::string filename = find_data.cFileName;
                LevelEntry e;
                e.name = filename.substr(0, filename.find_last_of('.'));
                e.path = std::string("Assets/Levels/") + filename;
                saved_puzzles.push_back(e);
            }
        } while (FindNextFileA(hFind, &find_data));

        FindClose(hFind);
    }

    // Sort numerically (lvl1 < lvl2 < ... < lvl10 < lvl11)
    std::sort(saved_puzzles.begin(), saved_puzzles.end(),
        [](const LevelEntry& a, const LevelEntry& b)
        {
            auto get_num = [](const std::string& s)
                {
                    //size_t start = s.find("l") + 1;
                    //size_t end = s.find("_puzzles.txt");
                    //return std::stoi(s.substr(start, end - start));
                    size_t end = s.find("_puzzles.txt");
                    size_t start = s.rfind("l", end) + 1;    // <-- FIXED
                    return std::stoi(s.substr(start, end - start));
                };
            return get_num(a.path) < get_num(b.path);
        });

    // Assign user-friendly display names starting at "Level 1"
    level_num = 1;
    for (auto& e : saved_puzzles)
    {
        e.name = "Puzzle " + std::to_string(level_num++);
        puzzles.push_back(e);
    }
}

// -------------------------------------------------------
// Load � called once when entering this game state
// -------------------------------------------------------
void levels_load()
{
    cursor = AEGfxTextureLoad("Assets/Admin/cursorSword_silver.png");
    button = AEGfxTextureLoad("Assets/Admin/buttonLong_brown_pressed.png");
    bg_wall = AEGfxTextureLoad("Assets/Tiles/tile_0001.png");
    scroll_bar = AEGfxTextureLoad("Assets/Admin/scrollbar_future_grey.png");
    font = AEGfxCreateFont("Assets/Exo2-Regular.ttf", 32);
    load_tile_textures("Assets/Tiles");
}

// -------------------------------------------------------
// Init � called every time we enter the levels screen
// -------------------------------------------------------
void levels_init()
{
    init_mesh();
    AEGfxSetBackgroundColor(0.08f, 0.09f, 0.12f);
    AEInputShowCursor(false);

    // Reset scroll position to top
    scroll_row = 0;
    scroll_row_f = 0.f;
    scroll_target = 0.f;

    // Rebuild level list from disk
    build_level_list();

    // Sync global level_paths so check_and_open_doors can navigate levels
    level_paths.clear();
    for (auto& e : levels)
        level_paths.push_back(e.path);

    level_puzzle_paths.clear();
    for (auto& e : puzzles)
        level_puzzle_paths.push_back(e.path);
}

// -------------------------------------------------------
// Update � input and scroll logic each frame
// -------------------------------------------------------
void levels_update()
{
    dt = (f32)AEFrameRateControllerGetFrameTime();
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    static int last_hovered = -1;
    int current_hovered = -1;

    if (IsButtonHovered(-600.f, -380.f, 200.f, 70.f)) current_hovered = 0;
    else if (IsButtonHovered(600.f, -380.f, 200.f, 70.f)) current_hovered = 1;
    else {
        for (int row = 0; row < ROWS_VISIBLE; ++row)
            for (int col = 0; col < COLS; ++col) {
                int idx = (scroll_row + row) * COLS + col;
                if (idx >= (int)levels.size()) continue; // skip empty cells
                f32 cx = GRID_START_X + col * (CELL_W + CELL_GAP_X);
                f32 cy = GRID_START_Y - row * (CELL_H + CELL_GAP_Y);
                if (IsButtonHovered(cx, cy, CELL_W, CELL_H)) { current_hovered = 2 + idx; break; }
            }
    }

    if (current_hovered != last_hovered && current_hovered != -1)
        play_sfx(g_audio.sfx_button_hover);
    last_hovered = current_hovered;
    // --- ESC / Back button � return to main menu ---
    if (AEInputCheckTriggered(AEVK_ESCAPE))
        next = MAINMENU;

    if (IsButtonClicked(-600.f, -380.f, 200.f, 70.f)) {
        play_sfx(g_audio.sfx_button_click);
        next = MAINMENU;
    }

    // --- Level Editor button � open blank editor for new level ---
    if (IsButtonClicked(600.f, -380.f, 200.f, 70.f))
    {
        play_sfx(g_audio.sfx_button_click);
        selected_level_path = ""; // blank map
        selected_level_puzzle_path = "";
        next = GS_LEVEL_EDITOR;
    }

    // --- Mouse wheel scroll ---
    s32 wheel = 0;
    AEInputMouseWheelDelta(&wheel);
    if (wheel > 0) scroll_target = scroll_target > 0.f ? scroll_target - 1.f : 0.f;
    if (wheel < 0) scroll_target = scroll_target < (f32)max_scroll() ? scroll_target + 1.f : (f32)max_scroll();

    // --- Scrollbar drag ---
    f32 sx = (f32)mouse_x - 800.f;
    f32 sy = -(f32)mouse_y + 450.f;
    bool on_bar = (sx >= BAR_X - TRACK_W * 0.5f &&
        sx <= BAR_X + TRACK_W * 0.5f &&
        sy <= BAR_TOP && sy >= BAR_BOT);

    if (on_bar && AEInputCheckCurr(AEVK_LBUTTON))
    {
        int ms = max_scroll();
        if (ms > 0)
        {
            f32 ratio = (BAR_TOP - sy) / BAR_H;
            scroll_target = ratio * (f32)ms;
            scroll_target = scroll_target < 0.f ? 0.f : scroll_target;
            scroll_target = scroll_target > (f32)ms ? (f32)ms : scroll_target;
        }
    }

    // --- Smooth lerp scroll towards target ---
    scroll_row_f += (scroll_target - scroll_row_f) * 10.f * dt;
    scroll_row = (int)(scroll_row_f + 0.5f); // snap to nearest integer row
    if (delete_msg_timer > 0.f) delete_msg_timer -= dt;

    // Right click � open/close context menu for custom levels
    if (AEInputCheckTriggered(AEVK_RBUTTON))
    {
        f32 mx = (f32)mouse_x - 800.f;
        f32 my = -(f32)mouse_y + 450.f;

        bool clicked_any = false;
        for (int row = 0; row < ROWS_VISIBLE; ++row)
        {
            for (int col = 0; col < COLS; ++col)
            {
                int idx = (scroll_row + row) * COLS + col;
                if (idx >= (int)levels.size()) continue;
                if (idx == 0) continue; // default level can't be edited/deleted

                f32 cx = GRID_START_X + col * (CELL_W + CELL_GAP_X);
                f32 cy = GRID_START_Y - row * (CELL_H + CELL_GAP_Y);

                bool in_cell = (mx >= cx - CELL_W * 0.5f && mx <= cx + CELL_W * 0.5f &&
                    my >= cy - CELL_H * 0.5f && my <= cy + CELL_H * 0.5f);
                if (in_cell)
                {
                    context_menu_idx = (context_menu_idx == idx) ? -1 : idx;
                    clicked_any = true;
                }
            }
        }
        if (!clicked_any) context_menu_idx = -1; // clicked outside � close menu
    }

    // Left click � context menu actions or play
    if (AEInputCheckTriggered(AEVK_LBUTTON))
    {
        // Context menu is open � check its buttons first
        if (context_menu_idx >= 0 && context_menu_idx < (int)levels.size())
        {
            int idx = context_menu_idx;
            int col = idx % COLS;
            int row = (idx / COLS) - scroll_row;
            f32 cx = GRID_START_X + col * (CELL_W + CELL_GAP_X);
            f32 cy = GRID_START_Y - row * (CELL_H + CELL_GAP_Y);

            f32 edit_x = cx;
            f32 edit_y = cy + CELL_H * 0.15f;
            if (IsButtonClicked(edit_x, edit_y, CELL_W - 40.f, 50.f))
            {
                selected_level_path = levels[idx].path;
                selected_level_puzzle_path = puzzles[idx].path;
                context_menu_idx = -1;
                next = GS_LEVEL_EDITOR;
                return;
            }

            f32 del_x = cx;
            f32 del_y = cy - CELL_H * 0.25f;
            if (IsButtonClicked(del_x, del_y, CELL_W - 40.f, 50.f))
            {
                remove(levels[idx].path.c_str());
                remove(puzzles[idx].path.c_str());
                build_level_list();
                level_paths.clear();
                for (auto& e : levels)
                    level_paths.push_back(e.path);
                for (auto& e : puzzles)
                    level_puzzle_paths.push_back(e.path);
                scroll_row = 0; scroll_row_f = 0.f; scroll_target = 0.f;
                context_menu_idx = -1;
                delete_msg_timer = 2.f;
                return;
            }

            // Clicked outside context menu � close it
            context_menu_idx = -1;
        }

        // Normal play click
        for (int row = 0; row < ROWS_VISIBLE; ++row)
        {
            for (int col = 0; col < COLS; ++col)
            {
                int idx = (scroll_row + row) * COLS + col;
                if (idx >= (int)levels.size()) continue;

                f32 cx = GRID_START_X + col * (CELL_W + CELL_GAP_X);
                f32 cy = GRID_START_Y - row * (CELL_H + CELL_GAP_Y);

                if (IsButtonClicked(cx, cy, CELL_W, CELL_H))
                {
                    play_sfx(g_audio.sfx_button_click);
                    selected_level_path = levels[idx].path;
                    selected_level_puzzle_path = puzzles[idx].path;
                    next = GS_LEVEL1;
                }
            }
        }
    }
}

// -------------------------------------------------------
// Draw � render grid, scrollbar, buttons, cursor
// -------------------------------------------------------
void levels_draw()
{
    // --- Tiled wall background ---
    if (bg_wall)
    {
        for (f32 x = -800.f; x <= 800.f; x += 100.f)
            for (f32 y = -450.f; y <= 450.f; y += 100.f)
                render_editor_ui(bg_wall, 100.f, 100.f, 0.3f, x, y);
    }

    // --- Screen title ---
    if (font != -1)
    {
        const char* title = "-- SELECT LEVEL --";
        f32 tw = 0.f, th = 0.f;
        AEGfxGetPrintSize(font, title, 1.2f, &tw, &th);
        AEGfxPrint(font, title, -tw * 0.5f, 0.9f, 1.2f, 0.9f, 0.8f, 0.3f, 1.f);
    }

    // --- Level grid cells ---
    for (int row = 0; row < ROWS_VISIBLE; ++row)
    {
        for (int col = 0; col < COLS; ++col)
        {
            int idx = (scroll_row + row) * COLS + col;
            f32 cx = GRID_START_X + col * (CELL_W + CELL_GAP_X);
            f32 cy = GRID_START_Y - row * (CELL_H + CELL_GAP_Y);

            if (idx < (int)levels.size())
            {
                bool is_flipped = (context_menu_idx == idx);

                if (!is_flipped)
                {
                    // Normal front face
                    bool hovered = IsButtonHovered(cx, cy, CELL_W, CELL_H);
                    if (button)
                        render_editor_ui(button, CELL_W, CELL_H, 1.0f, cx, cy, hovered);

                    if (font != -1)
                    {
                        const char* label = levels[idx].name.c_str();
                        f32 tw = 0.f, th = 0.f;
                        AEGfxGetPrintSize(font, label, 1.f, &tw, &th);
                        AEGfxPrint(font, label,
                            (cx / 800.f) - tw * 0.5f,
                            (cy - 10.f) / 450.f,
                            1.f, 1.f, 1.f, 1.f, 1.f);

                        if (idx > 0) // index 0 is always the default level
                        {
                            f32 tw2 = 0.f, th2 = 0.f;
                            AEGfxGetPrintSize(font, "[Custom]", 0.7f, &tw2, &th2);
                            AEGfxPrint(font, "[Custom]",
                                (cx / 800.f) - tw2 * 0.5f,
                                (cy - 40.f) / 450.f,
                                0.7f, 0.5f, 0.9f, 0.5f, 1.f);

                            f32 tw3 = 0.f, th3 = 0.f;
                            AEGfxGetPrintSize(font, "[RMB]", 0.5f, &tw3, &th3);
                            AEGfxPrint(font, "[RMB]",
                                (cx / 800.f) - tw3 * 0.5f,
                                (cy - 58.f) / 450.f,
                                0.5f, 0.6f, 0.6f, 0.6f, 1.f);
                        }
                    }
                }
                else
                {
                    // Flipped back face � show Edit and Delete
                    if (button)
                        render_editor_ui(button, CELL_W, CELL_H, 0.85f, cx, cy);

                    if (font != -1)
                    {
                        // Edit button (top half of cell)
                        f32 edit_x = cx;
                        f32 edit_y = cy + CELL_H * 0.15f;
                        bool edit_h = IsButtonHovered(edit_x, edit_y, CELL_W - 40.f, 50.f);
                        if (button)
                            render_editor_ui(button, CELL_W - 40.f, 50.f,
                                edit_h ? 1.f : 0.7f, edit_x, edit_y);
                        f32 etw = 0.f, eth = 0.f;
                        AEGfxGetPrintSize(font, "Edit", 0.9f, &etw, &eth);
                        AEGfxPrint(font, "Edit",
                            (edit_x / 800.f) - etw * 0.5f,
                            (edit_y - 10.f) / 450.f,
                            0.9f, 1.f, 1.f, 1.f, 1.f);

                        // Delete button (bottom half of cell)
                        f32 del_x = cx;
                        f32 del_y = cy - CELL_H * 0.25f;
                        bool del_h = IsButtonHovered(del_x, del_y, CELL_W - 40.f, 50.f);
                        if (button)
                            render_editor_ui(button, CELL_W - 40.f, 50.f,
                                del_h ? 1.f : 0.7f, del_x, del_y);
                        f32 dtw = 0.f, dth = 0.f;
                        AEGfxGetPrintSize(font, "Delete", 0.9f, &dtw, &dth);
                        AEGfxPrint(font, "Delete",
                            (del_x / 800.f) - dtw * 0.5f,
                            (del_y - 10.f) / 450.f,
                            0.9f, 1.f, 0.3f, 0.3f, 1.f);
                    }
                }
            }
            else
            {
                if (button)
                    render_editor_ui(button, CELL_W, CELL_H, 0.2f, cx, cy);
            }
        }
    }

    // --- Scrollbar (only shown when there are more rows than visible) ---
    int tr = total_rows();
    if (tr > ROWS_VISIBLE)
    {
        if (scroll_bar)
            render_editor_ui(scroll_bar, TRACK_W, BAR_H, 0.4f,
                BAR_X, (BAR_TOP + BAR_BOT) * 0.5f);

        int ms = max_scroll();
        f32 thumb_h = BAR_H * ((f32)ROWS_VISIBLE / (f32)tr);
        f32 thumb_ratio = (ms > 0) ? scroll_row_f / (f32)ms : 0.f;
        f32 thumb_cy = BAR_TOP - thumb_h * 0.5f - thumb_ratio * (BAR_H - thumb_h);

        if (scroll_bar)
            render_editor_ui(scroll_bar, THUMB_W, thumb_h, 1.0f, BAR_X, thumb_cy);

        if (font != -1)
        {
            if (scroll_row > 0)
                AEGfxPrint(font, "^ scroll up ^", -0.14f, 0.75f, 0.8f, 1.f, 1.f, 0.4f, 1.f);
            if (scroll_row < ms)
                AEGfxPrint(font, "v scroll down v", -0.16f, -0.75f, 0.8f, 1.f, 1.f, 0.4f, 1.f);
        }
    }

    // --- Back button (bottom left) ---
    if (button)
        render_editor_ui(button, 200.f, 70.f, 1.0f, -600.f, -380.f,
            IsButtonHovered(-600.f, -380.f, 200.f, 70.f));
    if (font != -1)
    {
        f32 tw = 0.f, th = 0.f;
        AEGfxGetPrintSize(font, "Back", 1.f, &tw, &th);
        AEGfxPrint(font, "Back",
            (-600.f / 800.f) - tw * 0.5f,
            (-380.f - 10.f) / 450.f,
            1.f, 1.f, 1.f, 1.f, 1.f);
    }

    // --- Level Editor button (bottom right) ---
    if (button)
        render_editor_ui(button, 200.f, 70.f, 1.0f, 600.f, -380.f,
            IsButtonHovered(600.f, -380.f, 200.f, 70.f));
    if (font != -1)
    {
        f32 tw = 0.f, th = 0.f;
        AEGfxGetPrintSize(font, "Level Editor", 0.8f, &tw, &th);
        AEGfxPrint(font, "Level Editor",
            (600.f / 800.f) - tw * 0.5f,
            (-380.f - 10.f) / 450.f,
            0.8f, 0.9f, 0.7f, 0.3f, 1.f);
    }

    // --- Delete confirmation toast ---
    if (delete_msg_timer > 0.f && font != -1)
    {
        f32 alpha = delete_msg_timer > 0.5f ? 1.f : delete_msg_timer / 0.5f;
        const char* msg = "Level deleted!";
        f32 tw = 0.f, th = 0.f;
        AEGfxGetPrintSize(font, msg, 1.2f, &tw, &th);
        // Draw dark background box first
        render_editor_ui(button, 400.f, 80.f, 0.9f, 0.f, 0.f);
        AEGfxPrint(font, msg, -tw * 0.5f, -0.05f, 1.2f, 1.f, 0.3f, 0.3f, alpha);
    }

    // --- Map preview on hover ---
    for (int row = 0; row < ROWS_VISIBLE; ++row)
    {
        for (int col = 0; col < COLS; ++col)
        {
            int idx = (scroll_row + row) * COLS + col;
            if (idx >= (int)levels.size()) continue;

            f32 cx = GRID_START_X + col * (CELL_W + CELL_GAP_X);
            f32 cy = GRID_START_Y - row * (CELL_H + CELL_GAP_Y);

            if (!IsButtonHovered(cx, cy, CELL_W, CELL_H)) continue;
            if (context_menu_idx == idx) break; // don't show preview when flipped

            // Load preview map if hovering a new cell
            if (preview_idx != idx)
            {
                preview_idx = idx;
                for (int r = 0; r < TILE_ROWS; ++r)
                    for (int c = 0; c < TILE_COLS; ++c)
                        preview_map[r][c] = 0;

                if (levels[idx].path.empty())
                    load_map_from_file("Assets/Levels/default.txt", preview_map);
                else
                    load_map_from_file(levels[idx].path.c_str(), preview_map);
            }

            // Draw preview background box
            const f32 PREV_W = 320.f;
            const f32 PREV_H = 180.f;
            f32 wx = (f32)mouse_x - 800.f;
            f32 wy = -(f32)mouse_y + 450.f;
            const f32 PREV_X = wx - PREV_W * 0.5f - 10.f;
            const f32 PREV_Y = wy - PREV_H * 0.5f - 10.f;

            if (button)
                render_editor_ui(button, PREV_W + 16.f, PREV_H + 16.f, 0.95f, PREV_X, PREV_Y);

            draw_map_preview(preview_map, PREV_X, PREV_Y, PREV_W, PREV_H);

            // Preview label
            if (font != -1)
            {
                const char* lbl = levels[idx].name.c_str();
                f32 tw = 0.f, th = 0.f;
                AEGfxGetPrintSize(font, lbl, 0.8f, &tw, &th);
                AEGfxPrint(font, lbl,
                    (PREV_X / 800.f) - tw * 0.5f,
                    (PREV_Y + PREV_H * 0.5f + 25.f) / 450.f,
                    0.8f, 1.f, 1.f, 1.f, 1.f);
            }
            break;
        }
    }



    // --- Custom cursor (always rendered last / on top) ---
    if (cursor)
    {
        f32 wx = (f32)mouse_x - 800.f;
        f32 wy = -(f32)mouse_y + 450.f;
        render_editor_ui(cursor, 50.f, 50.f, 1.0f, wx + 25.f, wy - 25.f);
    }
}

// -------------------------------------------------------
// Free � called when leaving this state
// -------------------------------------------------------
void levels_free()
{
    free_mesh();
    levels.clear();
    puzzles.clear();
}

// -------------------------------------------------------
// Unload � called when fully unloading this state's assets
// -------------------------------------------------------
void levels_unload()
{
    if (cursor) { AEGfxTextureUnload(cursor);     cursor = nullptr; }
    if (button) { AEGfxTextureUnload(button);     button = nullptr; }
    if (bg_wall) { AEGfxTextureUnload(bg_wall);    bg_wall = nullptr; }
    if (scroll_bar) { AEGfxTextureUnload(scroll_bar); scroll_bar = nullptr; }
    if (font != -1) { AEGfxDestroyFont(font);         font = -1; }
    free_tile_textures();
}

// -------------------------------------------------------
// ensure_level_paths_loaded
// Populates the global level_paths vector if it is empty.
// Called from level1_init() so that door navigation works
// even when the player enters a level without going through
// the level select screen first.
// -------------------------------------------------------
void ensure_level_paths_loaded()
{
    if (!level_paths.empty()) return; // already populated, skip rebuild

    // Scan saved level files from disk
	WIN32_FIND_DATAA find_data; // windows struct to hold file info
    HANDLE hFind = FindFirstFileA("Assets/Levels/lvl*.txt", &find_data); 

	std::vector<std::string> paths; // vectors to hold file paths for custom levels
	if (hFind != INVALID_HANDLE_VALUE) // if there are any matching files found 
    {
        do
        {
            // Contains flags describing the item found (file, directory, hidden, etc.).
			// We only want to add files, not directories, to the level paths list.
			if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
				paths.push_back(std::string("Assets/Levels/") + find_data.cFileName); // construct the full path and add to the list
		} while (FindNextFileA(hFind, &find_data)); // continue finding the next file until there are no more matches

		FindClose(hFind); // close the search handle to free system resources
    }

    // Sort numerically to match build_level_list() ordering
    std::sort(paths.begin(), paths.end(),
        [](const std::string& a, const std::string& b)
        {
			auto get_num = [](const std::string& s) // Extracts the numeric part of the filename (e.g., "lvl3.txt" -> 3)
                {
					size_t start = s.find("lvl") + 3; // position right after "lvl"
					size_t end = s.find(".txt"); // position of ".txt"
					return std::stoi(s.substr(start, end - start)); // convert the extracted substring to an integer
                };
			return get_num(a) < get_num(b); // sort in ascending order based on the extracted numeric value
        });

    // Index 0 = default built-in level (empty path = initialize_dungeon_map)
    level_paths.clear();
    level_paths.push_back("Assets/Levels/default.txt");
    for (auto& p : paths)
        level_paths.push_back(p);






    if (!level_puzzle_paths.empty()) return; // already populated, skip rebuild

    // Scan saved level files from disk
    hFind = FindFirstFileA("Assets/Levels/l*_puzzles.txt", &find_data);

    paths.clear(); // vectors to hold file paths for custom levels
    if (hFind != INVALID_HANDLE_VALUE) // if there are any matching files found 
    {
        do
        {
            // Contains flags describing the item found (file, directory, hidden, etc.).
            // We only want to add files, not directories, to the level paths list.
            if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                paths.push_back(std::string("Assets/Levels/") + find_data.cFileName); // construct the full path and add to the list
        } while (FindNextFileA(hFind, &find_data)); // continue finding the next file until there are no more matches

        FindClose(hFind); // close the search handle to free system resources
    }

    // Sort numerically to match build_level_list() ordering
    std::sort(paths.begin(), paths.end(),
        [](const std::string& a, const std::string& b)
        {
            auto get_num = [](const std::string& s) // Extracts the numeric part of the filename (e.g., "lvl3.txt" -> 3)
                {
                    //size_t start = s.find("l") + 1; // position right after "lvl"
                    //size_t end = s.find("_puzzles.txt"); // position of ".txt"
                    //return std::stoi(s.substr(start, end - start)); // convert the extracted substring to an integer
                    size_t end = s.find("_puzzles.txt");
                    size_t start = s.rfind("l", end) + 1; // search backwards to find the last "l" before "_puzzles"
                    return std::stoi(s.substr(start, end - start));
                };
            return get_num(a) < get_num(b); // sort in ascending order based on the extracted numeric value
        });

    // Index 0 = default built-in level (empty path = initialize_dungeon_map)
    level_puzzle_paths.clear();
    level_puzzle_paths.push_back("Assets/Levels/puzzle.txt");
    for (auto& p : paths)
        level_puzzle_paths.push_back(p);
}