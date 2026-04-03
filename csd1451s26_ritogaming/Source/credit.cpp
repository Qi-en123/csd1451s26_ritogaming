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
#include <vector>
#include <string>

namespace {
    // ----------------------------------------------------------------------------
    // Cursor
    // ----------------------------------------------------------------------------
    AEGfxTexture* cursor = nullptr;  // custom sword cursor texture
    s32 mouse_x, mouse_y;            // current mouse position in screen space

    // ----------------------------------------------------------------------------
    // Back Button
    // ----------------------------------------------------------------------------
    AEGfxTexture* button = nullptr;   // button texture for the back/exit button
    const f32 button_w = 200;         // button width in world units
    const f32 button_h = 100;         // button height in world units
    const f32 button_x = -650.f;      // button x position (left side of screen)
    f32 button_y = 350.f;             // button y position (top side of screen)

    // ----------------------------------------------------------------------------
    // Background
    // ----------------------------------------------------------------------------
    AEGfxTexture* wall = nullptr;     // tiled wall texture for the background

    // ----------------------------------------------------------------------------
    // Credits Scroll State
    // ----------------------------------------------------------------------------
    s8  credit_font = -1;             // font handle for rendering credit text
    f32 scrollY = -1.3f;              // current vertical scroll position (normalized)
    bool finishedScrolling = false;   // true when credits have scrolled off screen
    f32 scrollSpeed = 0.25f;          // base scroll speed per second
    f32 fastScrollMultiplier = 3;     // multiplier applied when space is held
    f32 totalHeight = 0.f;            // total height of all credit lines combined
    std::vector<std::string> credit_lines; // lines of text loaded from credits file
}

// ----------------------------------------------------------------------------
//
// Function: credit_load
// Loads all assets required for the credits screen, including the font,
// textures, and credit lines read from Assets/Credits/credit.txt
//
// ----------------------------------------------------------------------------
void credit_load()
{
    // Load font for rendering credit text
    credit_font = AEGfxCreateFont("Assets/Fonts/Kenney Mini Square.ttf", 40);
    cursor = AEGfxTextureLoad("Assets/Sprites/Admin/cursorSword_silver.png");
    printf("Font value: %d\n", credit_font);
    if (credit_font == -1)
        return;

    // Load button and background textures
    button = AEGfxTextureLoad("Assets/Sprites/Admin/button_brown_close.png");
    wall = AEGfxTextureLoad("Assets/Sprites/Tiles/tile_0001.png");

    // Read credit lines from external file so non-programmers can edit them
    credit_lines.clear();
    std::ifstream file("Assets/Credits/credit.txt");
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
            credit_lines.push_back(line);  // store each line for rendering
        file.close();
    }
}

// ----------------------------------------------------------------------------
//
// Function: credit_init
// Resets scroll position to below the screen and computes the total
// height of all credit lines for scroll bounds detection
//
// ----------------------------------------------------------------------------
void credit_init()
{
    scrollY = -1.3f;  // start below visible screen area

    // Each line occupies this much normalized screen space
    f32 lineSpacing = 0.12f;
    int lineCount = (int)credit_lines.size();
    printf("Total lines: %d\n", lineCount);

    // Total height determines when to reset the scroll
    totalHeight = lineCount * lineSpacing;

    AEInputShowCursor(false);  // hide system cursor, draw custom one instead
    init_mesh();                // initialize meshes needed for rendering
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.05f);  // dark navy background
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
    // Track mouse position for cursor and button interaction
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    // Base scroll speed, tripled when space is held
    f32 speed = 0.25f;
    if (AEInputCheckCurr(AEVK_SPACE))
        speed *= 3.f;

    // Advance scroll position upward each frame
    scrollY += speed * AEFrameRateControllerGetFrameTime();

    // Reset scroll once all lines have moved above the top of the screen
    if (scrollY > 1.f + totalHeight)
        scrollY = -1.3f;

    // Back button returns to main menu
    if (IsButtonClicked(button_x, button_y, button_w, button_h))
    {
        play_sfx(g_audio.sfx_button_click);
        next = MAINMENU;
    }
}

// ----------------------------------------------------------------------------
//
// Function: credit_draw
// Renders the tiled background, back button, custom cursor, and all
// credit lines with perspective scaling that shrinks text toward the top
//
// ----------------------------------------------------------------------------
void credit_draw()
{
    // Tile the wall texture across the entire background
    for (f32 x = -800.f; x <= 800.f; x += 100.f)
        for (f32 y = -800.f; y < 800.f; y += 100.f)
            if (wall) render_editor_ui(wall, 100.f, 100.f, 0.3f, x, y);

    // Draw back button with hover highlight and sound feedback
    static bool was_hovered = false;
    bool hovered = IsButtonHovered(button_x, button_y, button_w, button_h);
    render_editor_ui(button, button_w, button_h, 1.0f, button_x, button_y, hovered);

    // Play hover sound once when cursor first enters the button
    if (hovered && !was_hovered)
        play_sfx(g_audio.sfx_button_hover);

    // Play click sound when button is pressed
    if (IsButtonClicked(button_x, button_y, button_w, button_h))
        play_sfx(g_audio.sfx_button_click);

    was_hovered = hovered;  // store hover state for next frame comparison

    // Draw custom cursor at current mouse world position
    if (cursor)
    {
        f32 world_x = (f32)mouse_x - 800.f;   // convert screen x to world x
        f32 world_y = -(f32)mouse_y + 450.f;  // flip y axis for world space
        render_editor_ui(cursor, 50.f, 50.f, 1.0f, world_x, world_y);
    }

    // ----------------------------------------------------------------------------
    // Perspective scroll rendering
    // Text starts large at the bottom and shrinks toward the top,
    // simulating a 3D receding perspective like a movie credit roll
    // ----------------------------------------------------------------------------
    f32 y = scrollY;                   // current y position for the first line
    const f32 baseLineSpacing = 0.18f; // spacing between lines at full scale
    const f32 baseScale = 2.0f;        // maximum text scale at bottom of screen
    const f32 minScale = 0.05f;        // minimum text scale before text disappears
    const f32 perspective = 1.6f;      // controls how fast text shrinks toward top

    for (int i = 0; i < (int)credit_lines.size(); ++i)
    {
        // All credit lines use a warm yellow colour
        float r = 1.f, g = 0.85f, b = 0.3f;

        // Normalize t: 0 = bottom of crawl, 1 = top of crawl
        f32 t = (y + 1.3f) / (1.5f + totalHeight);
        if (t > 1.f) t = 1.f;
        if (t < 0.f) t = 0.f;

        // Scale shrinks faster toward the top using a power curve
        f32 scale = baseScale * powf(1.f - t, perspective);
        if (scale < minScale) scale = minScale;  // clamp to minimum visible size

        // Tilt y offset to simulate the text receding into the distance
        f32 yOffset = y * (1.f - t * 0.5f);

        // Measure text width to center it horizontally
        f32 textWidth, textHeight;
        AEGfxGetPrintSize(credit_font, credit_lines[i].c_str(), scale, &textWidth, &textHeight);

        // Render the credit line centered on screen
        AEGfxPrint(credit_font,
            credit_lines[i].c_str(),
            -textWidth / 2,   // center horizontally
            yOffset,
            scale,
            r, g, b, 1.f);

        // Advance y position by scaled line spacing so lines stay evenly spaced
        y -= baseLineSpacing * scale;
    }
}

// ----------------------------------------------------------------------------
//
// Function: credit_free
// Frees runtime mesh resources used during the credits screen
//
// ----------------------------------------------------------------------------
void credit_free()
{
    free_mesh();  // release vertex list memory created in init_mesh
}

// ----------------------------------------------------------------------------
//
// Function: credit_unload
// Unloads all textures, destroys the font, and clears the credit lines
// vector when leaving the credits state
//
// ----------------------------------------------------------------------------
void credit_unload()
{
    // Destroy font handle if it was successfully created
    if (credit_font != -1)
    {
        AEGfxDestroyFont(credit_font);
        credit_font = -1;
    }

    // Unload textures and null their pointers to prevent dangling references
    if (cursor)
    {
        AEGfxTextureUnload(cursor);
        cursor = nullptr;
    }
    if (button)
    {
        AEGfxTextureUnload(button);
        button = nullptr;
    }
    if (wall)
    {
        AEGfxTextureUnload(wall);
        wall = nullptr;
    }

    // Clear credit lines so they are reloaded fresh on next entry
    credit_lines.clear();
}