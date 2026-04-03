/*!************************************************************************
\file:						utils.cpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu

\brief:						Implementation of utility functions for UI
                            interaction, input handling, and level state
                            management.
**************************************************************************/

#include "AEEngine.h"
#include <string>
#include <vector>
#include "renderer.hpp"

// ----------------------------------------------------------------------------
//
// Global Variables (Definitions)
//
// ---------------------------------------------------------------------------- 

std::vector<std::string> level_paths;           // Stores standard level filenames
std::vector<std::string> level_puzzle_paths;    // Stores puzzle level filenames
int current_level_index = 0;                    // Tracker for progression
std::string selected_level_path = "";           // Currently loaded standard path
std::string selected_level_puzzle_path = "";    // Currently loaded puzzle path
f32 dt;                                         // Frame delta time

// ----------------------------------------------------------------------------
//
// Function: IsButtonClicked
// Logic for detecting a left-click within a specific UI bounding box.
// Includes coordinate conversion from world space to screen space.
//
// ---------------------------------------------------------------------------- 
int IsButtonClicked(f32 rectX, f32 rectY, f32 rectW, f32 rectH)
{
    // Get mouse position in screen coordinates
    s32 mouse_x, mouse_y;
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    // Convert button world position to screen coordinates
    // Assumes 1600x900 resolution (800 offset / 450 offset)
    f32 screen_x = rectX + 800.f - rectW / 2.f;  // Map center origin to top-left
    f32 screen_y = -rectY + 450.f - rectH / 2.f; // Flip Y axis for screen space

    // Check if mouse is inside button bounds
    if ((f32)mouse_x >= screen_x && (f32)mouse_x <= screen_x + rectW &&
        (f32)mouse_y >= screen_y && (f32)mouse_y <= screen_y + rectH)
    {
        // Return true only if the left mouse button was triggered this frame
        if (AEInputCheckTriggered(AEVK_LBUTTON))
            return 1;
    }
    return 0;
}

// ----------------------------------------------------------------------------
//
// Function: IsButtonHovered
// Logic for detecting if the mouse cursor is hovering over a bounding box.
// Used for visual feedback (e.g., highlighting buttons).
//
// ---------------------------------------------------------------------------- 
int IsButtonHovered(f32 rectX, f32 rectY, f32 rectW, f32 rectH)
{
    s32 mouse_x, mouse_y;
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    f32 screen_x = rectX + 800.f - rectW / 2.f;
    f32 screen_y = -rectY + 450.f - rectH / 2.f;

    if ((f32)mouse_x >= screen_x && (f32)mouse_x <= screen_x + rectW &&
        (f32)mouse_y >= screen_y && (f32)mouse_y <= screen_y + rectH)
        return 1;

    return 0;
}

// ----------------------------------------------------------------------------
//
// Function: draw_confirm_popup
// Handles the rendering and interaction logic for the confirmation modal.
// Manages background dimming, texture rendering, and text centering.
//
// ---------------------------------------------------------------------------- 
int draw_confirm_popup(AEGfxTexture* panel, AEGfxTexture* btn_yes, AEGfxTexture* btn_no, s8 font, const char* message, f32 font_scale)
{
    // 1. Render visual layers
    draw_black_overlay(0.6f); // Dim the background
    render_editor_ui(panel, 800.f, 400.f, 1.0f, 0.f, 0.f); // Main popup box

    // Render buttons with hover detection feedback
    render_editor_ui(btn_yes, 200.f, 80.f, 1.0f, -100.f, -50.f,
        IsButtonHovered(-100.f, -50.f, 200.f, 80.f));
    render_editor_ui(btn_no, 200.f, 80.f, 1.0f, 150.f, -50.f,
        IsButtonHovered(150.f, -50.f, 200.f, 80.f));

    // 2. Render Text
    if (font != -1)
    {
        f32 tw = 0.f, th = 0.f;

        // Draw the main message centered
        AEGfxGetPrintSize(font, message, font_scale, &tw, &th);
        AEGfxPrint(font, message, -tw * 0.5f, 0.1f, font_scale, 0.f, 0.f, 0.f, 1.f);

        // Calculate normalized positions for button labels
        f32 yes_px = -100.f / 800.f;
        f32 yes_py = -50.f / 450.f;

        AEGfxGetPrintSize(font, "Yes", font_scale, &tw, &th);
        AEGfxPrint(font, "Yes", yes_px - tw * 0.5f, yes_py - th * 0.5f,
            font_scale, 0.f, 0.f, 0.f, 1.f);

        f32 no_px = 150.f / 800.f;
        f32 no_py = -50.f / 450.f;

        AEGfxGetPrintSize(font, "No", font_scale, &tw, &th);
        AEGfxPrint(font, "No", no_px - tw * 0.5f, no_py - th * 0.5f,
            font_scale, 0.f, 0.f, 0.f, 1.f);
    }

    // 3. Process Input Results
    if (IsButtonClicked(-100.f, -50.f, 200.f, 80.f)) return 1;  // Yes clicked
    if (IsButtonClicked(150.f, -50.f, 200.f, 80.f)) return 0;   // No clicked

    return -1; // No selection yet
}