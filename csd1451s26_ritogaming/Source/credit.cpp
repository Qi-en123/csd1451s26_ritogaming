/*!************************************************************************
\file:          credit.cpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         This file provides functions and global variables to manage game states, including
                loading, initializing, updating, drawing, freeing, and unloading states. It also
                implements a credit roll with adjustable speed, perspective scaling, and automatic
                reset when finished
**************************************************************************/

#include "AEEngine.h"
#include "cstdio"
#include "renderer.hpp"
#include "utils.hpp"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "audio_manager.hpp"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>

namespace {

    // ----------------------------------------------------------------------------
    // Config parser
    // Reads a key-value .txt file into a map. Lines starting with # are comments.
    // Values may contain spaces (e.g. font paths like "Kenney Mini Square.ttf")
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

            // Read the rest of the line as the value to support paths with spaces
            std::string value;
            if (std::getline(iss >> std::ws, value))
                kv[key] = value;
        }
        return kv;
    }

    // Helper to safely read float from map with a fallback default
    f32 kv_float(const std::unordered_map<std::string, std::string>& kv,
        const std::string& key, f32 fallback = 0.f)
    {
        auto it = kv.find(key);
        return (it != kv.end()) ? std::stof(it->second) : fallback;
    }

    // Helper to safely read int from map with a fallback default
    int kv_int(const std::unordered_map<std::string, std::string>& kv,
        const std::string& key, int fallback = 0)
    {
        auto it = kv.find(key);
        return (it != kv.end()) ? std::stoi(it->second) : fallback;
    }

    // Helper to safely read string from map with a fallback default
    std::string kv_str(const std::unordered_map<std::string, std::string>& kv,
        const std::string& key, const std::string& fallback = "")
    {
        auto it = kv.find(key);
        return (it != kv.end()) ? it->second : fallback;
    }

    // ----------------------------------------------------------------------------
    // Cursor
    // ----------------------------------------------------------------------------
    AEGfxTexture* cursor = nullptr;
    f32 cursor_w = 0.f;
    f32 cursor_h = 0.f;
    s32 mouse_x = 0;
    s32 mouse_y = 0;
	s32 screen_w = AEGfxGetWindowWidth();
	s32 screen_h = AEGfxGetWindowHeight();

    // ----------------------------------------------------------------------------
    // Back Button
    // ----------------------------------------------------------------------------
    AEGfxTexture* button = nullptr;
    f32 button_w = 0.f;
    f32 button_h = 0.f;
    f32 button_x = 0.f;
    f32 button_y = 0.f;

    // ----------------------------------------------------------------------------
    // Background
    // ----------------------------------------------------------------------------
    AEGfxTexture* wall = nullptr;
    f32 wall_tile_size = 0.f;
    f32 wall_tile_min = 0.f;
    f32 wall_tile_max = 0.f;
    f32 wall_tint = 0.f;
    f32 bg_color_r = 0.f;
    f32 bg_color_g = 0.f;
    f32 bg_color_b = 0.f;

    // ----------------------------------------------------------------------------
    // DigiPen Logo
    // ----------------------------------------------------------------------------
    AEGfxTexture* digipen_logo = nullptr;
    f32 logo_x = 0.f;
    f32 logo_y = 0.f;
    f32 logo_w = 0.f;
    f32 logo_h = 0.f;
    f32 logo_alpha = 0.f;

    // ----------------------------------------------------------------------------
    // Font
    // ----------------------------------------------------------------------------
    s8          credit_font = -1;
    int         font_size = 0;
    std::string font_path;

    // ----------------------------------------------------------------------------
    // Credits Scroll State
    // ----------------------------------------------------------------------------
    std::string              credits_file;
    f32                      scrollY = 0.f;
    f32                      scroll_speed = 0.f;
    f32                      scroll_fast_multiplier = 0.f;
    f32                      scroll_start_y = 0.f;
    f32                      scroll_line_spacing = 0.f;
    f32                      scroll_reset_padding = 0.f;
    f32                      totalHeight = 0.f;
    std::vector<std::string> credit_lines;

    // ----------------------------------------------------------------------------
    // Perspective Settings
    // ----------------------------------------------------------------------------
    f32 perspective_base_line_spacing = 0.f;
    f32 perspective_base_scale = 0.f;
    f32 perspective_min_scale = 0.f;
    f32 perspective_power = 0.f;
    f32 perspective_tilt = 0.f;

    // ----------------------------------------------------------------------------
    // Text Color
    // ----------------------------------------------------------------------------
    f32 text_color_r = 0.f;
    f32 text_color_g = 0.f;
    f32 text_color_b = 0.f;

} // namespace

// ----------------------------------------------------------------------------
//
// Function: credit_load
// Loads all assets required for the credits screen. Every value is read
// from Assets/Credits/credit_config.txt so no magic numbers live in code.
//
// ----------------------------------------------------------------------------
void credit_load()
{
    auto cfg = parse_kv_file("Assets/Credits/credit_config.txt");

    // --- Cursor ---
    cursor_w = kv_float(cfg, "cursor_w");
    cursor_h = kv_float(cfg, "cursor_h");
    cursor = AEGfxTextureLoad(kv_str(cfg, "cursor_path").c_str());

    // --- Back Button ---
    button_w = kv_float(cfg, "button_w");
    button_h = kv_float(cfg, "button_h");
    button_x = kv_float(cfg, "button_x");
    button_y = kv_float(cfg, "button_y");
    button = AEGfxTextureLoad(kv_str(cfg, "button_path").c_str());

    // --- Background ---
    wall_tile_size = kv_float(cfg, "wall_tile_size");
    wall_tile_min = kv_float(cfg, "wall_tile_min");
    wall_tile_max = kv_float(cfg, "wall_tile_max");
    wall_tint = kv_float(cfg, "wall_tint");
    bg_color_r = kv_float(cfg, "bg_color_r");
    bg_color_g = kv_float(cfg, "bg_color_g");
    bg_color_b = kv_float(cfg, "bg_color_b");
    wall = AEGfxTextureLoad(kv_str(cfg, "wall_path").c_str());

    // --- DigiPen Logo ---
    logo_x = kv_float(cfg, "logo_digipen_x");
    logo_y = kv_float(cfg, "logo_digipen_y");
    logo_w = kv_float(cfg, "logo_digipen_w");
    logo_h = kv_float(cfg, "logo_digipen_h");
    logo_alpha = kv_float(cfg, "logo_digipen_alpha");
    digipen_logo = AEGfxTextureLoad(kv_str(cfg, "logo_digipen_path").c_str());

    // --- Font: store path and size only, font is created in credit_init ---
    font_path = kv_str(cfg, "font_path");
    font_size = kv_int(cfg, "font_size");
   

    // --- Scroll Settings ---
    scroll_start_y = kv_float(cfg, "scroll_start_y");
    scroll_speed = kv_float(cfg, "scroll_speed");
    scroll_fast_multiplier = kv_float(cfg, "scroll_fast_multiplier");
    scroll_line_spacing = kv_float(cfg, "scroll_line_spacing");
    scroll_reset_padding = kv_float(cfg, "scroll_reset_padding");

    // --- Perspective Settings ---
    perspective_base_line_spacing = kv_float(cfg, "perspective_base_line_spacing");
    perspective_base_scale = kv_float(cfg, "perspective_base_scale");
    perspective_min_scale = kv_float(cfg, "perspective_min_scale");
    perspective_power = kv_float(cfg, "perspective_power");
    perspective_tilt = kv_float(cfg, "perspective_tilt");

    // --- Text Color ---
    text_color_r = kv_float(cfg, "text_color_r");
    text_color_g = kv_float(cfg, "text_color_g");
    text_color_b = kv_float(cfg, "text_color_b");

    // --- Credit Lines ---
    credits_file = kv_str(cfg, "credits_file");
    credit_lines.clear();
    std::ifstream file(credits_file);
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
            credit_lines.push_back(line);
        file.close();
    }
}

// ----------------------------------------------------------------------------
//
// Function: credit_init
// Creates the font (requires graphics context to be ready), resets scroll
// position, and computes total height of all credit lines
//
// ----------------------------------------------------------------------------
void credit_init()
{
    // Font must be created here, not in load, as it requires the graphics context
    credit_font = AEGfxCreateFont(font_path.c_str(), font_size);

    scrollY = scroll_start_y;

    int lineCount = (int)credit_lines.size();
    totalHeight = lineCount * scroll_line_spacing;

    AEInputShowCursor(false);
    init_mesh();
    AEGfxSetBackgroundColor(bg_color_r, bg_color_g, bg_color_b);

    g_bgm_current_track = &g_audio.bgm_credits;
    play_bgm(g_audio.bgm_credits);
}

// ----------------------------------------------------------------------------
//
// Function: credit_update
// Scrolls the credits upward each frame. Holding space increases speed.
// Resets scroll when all lines have passed the top. Handles back button.
//
// ----------------------------------------------------------------------------
void credit_update()
{
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    f32 speed = scroll_speed;
    if (AEInputCheckCurr(AEVK_SPACE))
        speed *= scroll_fast_multiplier;

    scrollY += speed * (f32)AEFrameRateControllerGetFrameTime();

    if (scrollY > scroll_reset_padding + totalHeight)
        scrollY = scroll_start_y;

    if (IsButtonClicked(button_x, button_y, button_w, button_h))
    {
        play_sfx(g_audio.sfx_button_click, 0.1f);
        next = MAINMENU;
    }
}

// ----------------------------------------------------------------------------
//
// Function: credit_draw
// Renders the tiled background, DigiPen logo, back button, custom cursor,
// and all credit lines with perspective scaling that shrinks text toward the top
//
// ----------------------------------------------------------------------------
void credit_draw()
{
    // Tile the wall texture across the entire background
    for (f32 x = wall_tile_min; x <= wall_tile_max; x += wall_tile_size)
        for (f32 y = wall_tile_min; y < wall_tile_max; y += wall_tile_size)
            if (wall) render_editor_ui(wall, wall_tile_size, wall_tile_size, wall_tint, x, y);

    // Draw DigiPen logo at top-right
    if (digipen_logo)
        render_editor_ui(digipen_logo, logo_w, logo_h, logo_alpha, logo_x, logo_y);

    // Draw back button with hover highlight and sound feedback
    static bool was_hovered = false;
    bool hovered = IsButtonHovered(button_x, button_y, button_w, button_h);
    render_editor_ui(button, button_w, button_h, 1.0f, button_x, button_y, hovered);

    if (hovered && !was_hovered)
        play_sfx(g_audio.sfx_button_hover);

    if (IsButtonClicked(button_x, button_y, button_w, button_h))
        play_sfx(g_audio.sfx_button_click, 0.1f);

    was_hovered = hovered;

    // Draw custom cursor at current mouse world position
    if (cursor)
    {
        f32 world_x = (f32)mouse_x - AEGfxGetWindowWidth()/2.0f;  // convert screen x to world x
        f32 world_y = -(f32)mouse_y + AEGfxGetWindowHeight()/2.0f; // flip y axis for world space
        render_editor_ui(cursor, cursor_w, cursor_h, 1.0f, world_x, world_y);
    }

    // ----------------------------------------------------------------------------
    // Perspective scroll rendering
    // Text starts large at the bottom and shrinks toward the top,
    // simulating a 3D receding perspective like a movie credit roll
    // ----------------------------------------------------------------------------
    f32 y = scrollY;

    for (int i = 0; i < (int)credit_lines.size(); ++i)
    {
        // Normalize t: 0 = bottom of crawl, 1 = top of crawl
        f32 t = (y - scroll_start_y) / (scroll_reset_padding + totalHeight - scroll_start_y);
        if (t > 1.f) t = 1.f;
        if (t < 0.f) t = 0.f;

        // Scale shrinks faster toward the top using a power curve
        f32 scale = perspective_base_scale * powf(1.f - t, perspective_power);
        if (scale < perspective_min_scale) scale = perspective_min_scale;

        // Tilt y offset to simulate the text receding into the distance
        f32 yOffset = y * (1.f - t * perspective_tilt);

        // Measure text width to center it horizontally
        f32 textWidth, textHeight;
        AEGfxGetPrintSize(credit_font, credit_lines[i].c_str(), scale, &textWidth, &textHeight);

        AEGfxPrint(credit_font,
            credit_lines[i].c_str(),
            -textWidth / 2.f,
            yOffset,
            scale,
            text_color_r, text_color_g, text_color_b, 1.f);

        // Advance y by scaled line spacing
        y -= perspective_base_line_spacing * scale;
    }
}

// ----------------------------------------------------------------------------
//
// Function: credit_free
// Frees runtime resources: mesh, BGM, and the font (created in credit_init)
//
// ----------------------------------------------------------------------------
void credit_free()
{
    free_mesh();
    stop_bgm();
    g_bgm_current_track = nullptr;

    // Font is destroyed here because it was created in credit_init
    if (credit_font != -1)
    {
        AEGfxDestroyFont(credit_font);
        credit_font = -1;
    }
}

// ----------------------------------------------------------------------------
//
// Function: credit_unload
// Unloads all textures and clears the credit lines vector
//
// ----------------------------------------------------------------------------
void credit_unload()
{
    if (cursor) { AEGfxTextureUnload(cursor);        cursor = nullptr; }
    if (button) { AEGfxTextureUnload(button);        button = nullptr; }
    if (wall) { AEGfxTextureUnload(wall);          wall = nullptr; }
    if (digipen_logo) { AEGfxTextureUnload(digipen_logo);  digipen_logo = nullptr; }

    credit_lines.clear();
}