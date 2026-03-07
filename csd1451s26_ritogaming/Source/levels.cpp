/*!************************************************************************
\file:						levels.cpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This file defines the 6 cycle function that
                            Loads, initializes, updates, draws, and unloads the levels interface
                            which features a 3x3 scrollable grid of levels (default + saved files).
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

namespace {
    AEGfxTexture* cursor = nullptr;
    AEGfxTexture* button = nullptr;
    AEGfxTexture* bg_wall = nullptr;
    AEGfxTexture* scroll_bar = nullptr;
    static s8 font = -1;
    s32 mouse_x, mouse_y;

    // --- Grid layout ---
    const int COLS = 3;
    const int ROWS_VISIBLE = 3;
    const f32 CELL_W = 280.f;
    const f32 CELL_H = 150.f;
    const f32 CELL_GAP_X = 40.f;
    const f32 CELL_GAP_Y = 30.f;
    const f32 GRID_START_X = -360.f;
    const f32 GRID_START_Y = 230.f;

    // --- Scrollbar constants ---
    const f32 BAR_X = 750.f;
    const f32 BAR_TOP = 300.f;
    const f32 BAR_BOT = -300.f;
    const f32 BAR_H = BAR_TOP - BAR_BOT;
    const f32 TRACK_W = 20.f;
    const f32 THUMB_W = 20.f;
    f32 scroll_row_f = 0.f;  // smooth float version
    f32 scroll_target = 0.f; // target to lerp towards
    // --- Scroll state ---
    int scroll_row = 0;

    // --- Level entries ---
    struct LevelEntry {
        std::string name;
        std::string path; // empty = default game level
    };
	std::vector<LevelEntry> levels; // vector of all levels to display (default + saved)

    // Default built-in levels
    const char* default_levels[] = { "Level 1" };
    const int   default_count = 1;
}

// -------------------------------------------------------
// Helpers
// -------------------------------------------------------
static int total_rows() // Calculate total rows needed based on number of levels and columns
{
	return ((int)levels.size() + COLS - 1) / COLS; //number of rows needed to fit all levels
}

static int max_scroll() // Calculate maximum scroll row based on total rows and visible rows
{
    int m = total_rows() - ROWS_VISIBLE;
    return m < 0 ? 0 : m;
}

// -------------------------------------------------------
// Scan Assets/Levels/lvl*.txt + add default levels
// -------------------------------------------------------
static void build_level_list()
{
    levels.clear();

    // Add default levels first
    for (int i = 0; i < default_count; ++i)
    {
        LevelEntry e;
        e.name = default_levels[i];
        e.path = "";
        levels.push_back(e);
    }

    // Scan Assets/Levels/ for lvl*.txt using Win32 (C++14 compatible)
    WIN32_FIND_DATAA find_data;
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

    // Sort alphabetically so lvl_01 < lvl_02 etc.
    std::sort(saved_levels.begin(), saved_levels.end(),
        [](const LevelEntry& a, const LevelEntry& b) {
            auto get_num = [](const std::string& s) {
                size_t start = s.find("lvl") + 3;
                size_t end = s.find(".txt");
                return std::stoi(s.substr(start, end - start));
                };
            return get_num(a.path) < get_num(b.path);
        });

    // Assign display names as Level 2, 3, 4...
    int level_num = default_count + 1;
    for (auto& e : saved_levels)
    {
        e.name = "Level " + std::to_string(level_num++);
        levels.push_back(e);
    }
}

// -------------------------------------------------------
// Load
// -------------------------------------------------------
void levels_load()
{
    cursor = AEGfxTextureLoad("Assets/Admin/cursorSword_silver.png");
    button = AEGfxTextureLoad("Assets/Admin/buttonLong_brown_pressed.png");
    bg_wall = AEGfxTextureLoad("Assets/Tiles/tile_0001.png");
    scroll_bar = AEGfxTextureLoad("Assets/Admin/scrollbar_future_grey.png");
    font = AEGfxCreateFont("Assets/Exo2-Regular.ttf", 32);
}

// -------------------------------------------------------
// Init
// -------------------------------------------------------
void levels_init()
{
    init_mesh();
    AEGfxSetBackgroundColor(0.08f, 0.09f, 0.12f);
    AEInputShowCursor(false);
    scroll_row = 0;
    build_level_list();

    level_paths.clear();
    for (auto& e : levels)
        level_paths.push_back(e.path);
}

// -------------------------------------------------------
// Update
// -------------------------------------------------------
void levels_update()
{
    dt = (f32)AEFrameRateControllerGetFrameTime();
    AEInputGetCursorPosition(&mouse_x, &mouse_y);


    // ESC — back to main menu
    if (AEInputCheckTriggered(AEVK_ESCAPE))
        next = MAINMENU;

    // Back button
    if (IsButtonClicked(-600.f, -380.f, 200.f, 70.f))
        next = MAINMENU;

    // Level Editor button
    if (IsButtonClicked(600.f, -380.f, 200.f, 70.f))
        next = GS_LEVEL_EDITOR;

    // Mouse wheel scroll
    s32 wheel = 0;
    AEInputMouseWheelDelta(&wheel);
    if (wheel > 0) scroll_target = scroll_target > 0.f ? scroll_target - 1.f : 0.f;
    if (wheel < 0) scroll_target = scroll_target < (f32)max_scroll() ? scroll_target + 1.f : (f32)max_scroll();

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

    scroll_row_f += (scroll_target - scroll_row_f) * 10.f * dt;
    scroll_row = (int)(scroll_row_f + 0.5f);

    // Level cell click
    if (AEInputCheckTriggered(AEVK_LBUTTON))
    {
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
                    printf("clicked display idx: %d path: %s\n", idx,
                        levels[idx].path.empty() ? "DEFAULT" : levels[idx].path.c_str());
                    selected_level_path = levels[idx].path;
                    next = GS_LEVEL1;
                }
            }
        }
    }
}

// -------------------------------------------------------
// Draw
// -------------------------------------------------------
void levels_draw()
{

    // Background
    if (bg_wall)
    {
        for (f32 x = -800.f; x <= 800.f; x += 100.f)
            for (f32 y = -450.f; y <= 450.f; y += 100.f)
                render_blending_sprite(bg_wall, 100.f, 100.f, 0.3f, x, y);
    }

    // Title
    if (font != -1)
    {
        const char* title = "-- SELECT LEVEL --";
        f32 tw = 0.f, th = 0.f;
        AEGfxGetPrintSize(font, title, 1.2f, &tw, &th);
        AEGfxPrint(font, title, -tw * 0.5f, 0.9f, 1.2f, 0.9f, 0.8f, 0.3f, 1.f);
    }

    // Level grid
    for (int row = 0; row < ROWS_VISIBLE; ++row)
    {
        for (int col = 0; col < COLS; ++col)
        {
            int idx = (scroll_row + row) * COLS + col;
            f32 cx = GRID_START_X + col * (CELL_W + CELL_GAP_X);
            f32 cy = GRID_START_Y - row * (CELL_H + CELL_GAP_Y);

            if (idx < (int)levels.size())
            {
                bool hovered = IsButtonHovered(cx, cy, CELL_W, CELL_H);
                if (button)
                    render_blending_sprite(button, CELL_W, CELL_H, 1.0f, cx, cy, hovered);

                if (font != -1)
                {
                    const char* label = levels[idx].name.c_str();
                    f32 tw = 0.f, th = 0.f;
                    AEGfxGetPrintSize(font, label, 1.f, &tw, &th);
                    AEGfxPrint(font, label,
                        (cx / 800.f) - tw * 0.5f,
                        (cy - 10.f) / 450.f,
                        1.f, 1.f, 1.f, 1.f, 1.f);

                    // Custom tag for saved levels
                    if (!levels[idx].path.empty())
                    {
                        f32 tw2 = 0.f, th2 = 0.f;
                        AEGfxGetPrintSize(font, "[Custom]", 0.7f, &tw2, &th2);
                        AEGfxPrint(font, "[Custom]",
                            (cx / 800.f) - tw2 * 0.5f,
                            (cy - 40.f) / 450.f,
                            0.7f, 0.5f, 0.9f, 0.5f, 1.f);
                    }
                }
            }
            else
            {
                // Empty slot
                if (button)
                    render_blending_sprite(button, CELL_W, CELL_H, 0.2f, cx, cy);
            }
        }
    }

    // Scrollbar (only shown when needed)
    int tr = total_rows();
    if (tr > ROWS_VISIBLE)
    {
        if (scroll_bar)
            render_blending_sprite(scroll_bar, TRACK_W, BAR_H, 0.4f, BAR_X, (BAR_TOP + BAR_BOT) * 0.5f);

        // Thumb (the scrollable thing)
        int ms = max_scroll();
        f32 thumb_h = BAR_H * ((f32)ROWS_VISIBLE / (f32)tr);
        f32 thumb_ratio = (ms > 0) ? scroll_row_f / (f32)ms : 0.f;
        f32 thumb_cy = BAR_TOP - thumb_h * 0.5f - thumb_ratio * (BAR_H - thumb_h);

        if (scroll_bar)
            render_blending_sprite(scroll_bar, THUMB_W, thumb_h, 1.0f, BAR_X, thumb_cy);

        // Scroll hint text
        if (font != -1)
        {
            if (scroll_row > 0)
                AEGfxPrint(font, "^ scroll up ^", -0.14f, 0.75f, 0.8f, 1.f, 1.f, 0.4f, 1.f);
            if (scroll_row < ms)
                AEGfxPrint(font, "v scroll down v", -0.16f, -0.75f, 0.8f, 1.f, 1.f, 0.4f, 1.f);
        }
    }

    // Back button (bottom left)
    if (button)
        render_blending_sprite(button, 200.f, 70.f, 1.0f, -600.f, -380.f,
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

    // Level Editor button (bottom right)
    if (button)
        render_blending_sprite(button, 200.f, 70.f, 1.0f, 600.f, -380.f,
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

    // Cursor
    if (cursor)
    {
        f32 wx = (f32)mouse_x - 800.f;
        f32 wy = -(f32)mouse_y + 450.f;
        render_blending_sprite(cursor, 50.f, 50.f, 1.0f, wx + 25.f, wy - 25.f);
    }
}

// -------------------------------------------------------
// Free / Unload
// -------------------------------------------------------
void levels_free()
{
    free_mesh();
    levels.clear();
}

void levels_unload()
{
    if (cursor) { AEGfxTextureUnload(cursor);  cursor = nullptr; }
    if (button) { AEGfxTextureUnload(button);  button = nullptr; }
    if (bg_wall) { AEGfxTextureUnload(bg_wall); bg_wall = nullptr; }
    if (scroll_bar) { AEGfxTextureUnload(scroll_bar);    scroll_bar = nullptr; }
    if (font != -1) { AEGfxDestroyFont(font); font = -1; }
}

void ensure_level_paths_loaded()
{
    if (!level_paths.empty()) return; // already loaded from levels screen, don't rebuild

    // Only runs if coming from outside levels screen (e.g. direct GS_LEVEL1 start)
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA("Assets/Levels/lvl*.txt", &find_data);

    std::vector<std::string> paths;
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                paths.push_back(std::string("Assets/Levels/") + find_data.cFileName);
            }
        } while (FindNextFileA(hFind, &find_data));
        FindClose(hFind);
    }

    std::sort(paths.begin(), paths.end(), [](const std::string& a, const std::string& b) {
        auto get_num = [](const std::string& s) {
            size_t start = s.find("lvl") + 3;
            size_t end = s.find(".txt");
            return std::stoi(s.substr(start, end - start));
            };
        return get_num(a) < get_num(b);
        });

    level_paths.clear();
    level_paths.push_back(""); // index 0 = default Level 1
    for (auto& p : paths)
        level_paths.push_back(p);
}
