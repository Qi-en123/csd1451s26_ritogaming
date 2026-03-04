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

namespace {
    // cursor properties
    AEGfxTexture* cursor = nullptr;
    s32 mouse_x, mouse_y;

	// button properties
    AEGfxTexture* button = nullptr;
    const f32 button_w = 200;
    const f32 button_h = 100;
    const f32 button_x = -650.f;  // left side of screen
	f32 button_y = 350.f; // top side of screen

	// background properties
    AEGfxTexture* wall = nullptr;

	// credit properties
     s8  credit_font = -1;
     f32 scrollY = -1.3f;
     bool finishedScrolling = false;
     f32 scrollSpeed = 0.25f;          // normal speed
     f32 fastScrollMultiplier = 3;     // how much faster when holding space
     f32 totalHeight = 0.f;
    static const char* credit_lines[] =
    {
        "Team Rito Gaming",
        "Team Members:",
        "DIRECTOR and PROGRAMMER",
        "Ahmad Qusyairi",
        "PRODUCER and PROGRAMMER",
        "Claire Cheong",
        "PRODUCER, DESIGN LEAD and PROGRAMMER",
        "Goh Qi En",
        "AUDIO LEAD, COMPOSER, and PROGRAMMER",
        "Jerod Lim",
        " ",
        "Faculty and Advisors",
        "Lab management and Instructor",
        "MR Gerald Wong",
        "MR Tommy Tan",
        "DR Soroor",
        " ",
        "Created at",
        "DigiPen Institute of Technology Singapore",
        "PRESIDENT",
        "Claude COMAIR",
        "EXECUTIVES",
        "Claude COMAIR CHU Jason Yeu Tat Michael GATS",
        "TAN Chek Ming Prasanna Kumar GHALI",
        "Mandy WONG Johnny DEEK",
        " ",
        "Copyrights for software, tools and libraries",
        "(C) Kenney Assets",
        "(C) MetaDigger",
        NULL
    };
}
void credit_load()
{
    // Load textures from Assets folder
    credit_font = AEGfxCreateFont("Assets/Exo2-Regular.ttf", 32);
    cursor = AEGfxTextureLoad("Assets/Admin/cursorSword_silver.png");
    printf("Font value: %d\n", credit_font);
    if (credit_font == -1)
    {
        return;
    }
    button = AEGfxTextureLoad("Assets/Admin/button_brown_close.png");
    wall = AEGfxTextureLoad("Assets/Tiles/tile_0001.png");
}

void credit_init()
{
    scrollY = -1.3f;  // start below screen

    f32 lineSpacing = 0.12f;

    int lineCount = 0;
    while (credit_lines[lineCount] != NULL) lineCount++;
    printf("Total lines: %d\n", lineCount);

    totalHeight = lineCount * lineSpacing;

    AEInputShowCursor(false); // Hide system cursor, we'll draw our own

	init_mesh(); // initialize any meshes needed for drawing (if applicable)

	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.05f); // Set a dark background color for the credits
}

void credit_update()
{
    // Get mouse position 
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    // Default speed
    f32 speed = 0.25f;

    // Fast scroll when space is held
    if (AEInputCheckCurr(AEVK_SPACE))
        speed *= 3.f;

    scrollY += speed * AEFrameRateControllerGetFrameTime();

    // Restart after the last line has fully moved above the top
    if (scrollY > 1.f + totalHeight)  // 1.f is top of normalized coordinates
        scrollY = -1.3f;              // reset below screen

    if (IsButtonClicked(button_x, button_y, button_w, button_h)) // Guide
    {
        next = MAINMENU;
    }

}

void credit_draw()
{
    // Draw wall tiles across top portion of screen
    for (f32 x = -800.f; x <= 800.f; x += 100.f)
    {
        for (f32 y = -800.f; y < 800.f; y += 100.f)
        {
            if (wall) render_blending_sprite(wall, 100.f, 100.f, 0.3f, x, y);
        }
    }


	// draw return button at top left corner
    render_blending_sprite(button, button_w, button_h, 1.0f, button_x, button_y,
        IsButtonHovered(button_x, button_y, button_w, button_h));

    // Draw custom cursor at mouse position
    if (cursor)
    {
        // Convert mouse screen coords to world coords
        f32 world_x = (f32)mouse_x - 800.f; // center origin
        f32 world_y = -(f32)mouse_y + 450.f; // flip Y axis

        render_blending_sprite(cursor, 50.f, 50.f, 1.0f, world_x, world_y);
    }


    f32 y = scrollY;
    const f32 baseLineSpacing = 0.18f;
    const f32 baseScale = 2.0f;
    const f32 minScale = 0.05f;

    // perspective factor (how fast text shrinks toward top)
    const f32 perspective = 1.6f;

    for (int i = 0; credit_lines[i] != NULL; ++i)
    {
        float r = 1.f, g = 0.85f, b = 0.3f; // all yellow

        // Normalize t from 0 (bottom) to 1 (top of crawl)
        f32 t = (y + 1.3f) / (1.5f + totalHeight);
        if (t > 1.f) t = 1.f;
        if (t < 0.f) t = 0.f;

        // scale shrinks faster toward top
        f32 scale = baseScale * powf(1.f - t, perspective);
        if (scale < minScale) scale = minScale;

        // Perspective tilt: y gets smaller upward to simulate receding
        f32 yOffset = y * (1.f - t * 0.5f); // tweak 0.5 for tilt angle

        f32 textWidth, textHeight;
        AEGfxGetPrintSize(credit_font, credit_lines[i], scale, &textWidth, &textHeight);

        // Draw centered horizontally
        AEGfxPrint(credit_font,
            credit_lines[i],
            -textWidth / 2,
            yOffset,
            scale,
            r, g, b, 1.f);

        y -= baseLineSpacing * scale; // line spacing shrinks as scale shrinks
    }


}

void credit_free()
{
    free_mesh(); // Free any dynamically allocated memory for meshes
}

void credit_unload()
{
	// Unload textures and fonts
    if (credit_font != -1)
    {
        AEGfxDestroyFont(credit_font);
        credit_font = -1;
    }

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

}