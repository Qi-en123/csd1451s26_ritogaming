/*!************************************************************************
\file:						opening.hpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This file defines the 6 cycle function that
                            Loads, initializes, updates, draws, and unloads the opening logo with fade-in/out effects.
**************************************************************************/

#include "renderer.hpp"
#include "AEEngine.h"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "utils.hpp"

namespace {
     AEGfxTexture* logo = nullptr;
     float         elapsed = 0.f;
     float         alpha = 0.f;
     bool          done = false;
}
void opening_load()
{
    logo = AEGfxTextureLoad("Assets/Admin/DigiPen_BLACK.png");
 
}

// =========================================================
// Opening Screen Initialization
// =========================================================
void opening_init()
{
    init_mesh();  // Create/load mesh needed for the opening screen

    AEGfxSetBackgroundColor(0.f, 0.f, 0.f);
    // Set background color to black (RGB: 0,0,0)

    elapsed = 0.f;  // Reset total elapsed time for this scene
    alpha = 0.f;    // Start fully transparent (for fade-in effect)

    cam.pos.x = 0.f; // Reset camera horizontal position to center (no offset on X axis)
    cam.pos.y = 0.f; // Reset camera vertical position to center (no offset on Y axis)
    zoom = 1.0f; // Set camera zoom to normal scale (1.0 = no zoom in/out)

}


// =========================================================
// Opening Screen Update (runs every frame)
// =========================================================
void opening_update()
{
    // Get frame time (delta time) from engine
    dt = static_cast<float>(AEFrameRateControllerGetFrameTime());

    // Accumulate total time since game state started
    elapsed += dt;

    // ---------------------------
    // Fade Logic (Total 6 seconds)
    // ---------------------------

    if (elapsed < 2.f)
        alpha = elapsed / 2.f;
    // Fade in over first 2 seconds
    // alpha goes from 0 ->c1

    else if (elapsed > 4.f)
        alpha = 1.f - ((elapsed - 4.f) / 2.f);
    // Fade out from 4s -> 6s
    // alpha goes from 1 -> 0

    else
        alpha = 1.f;
    // Fully visible between 2s and 4s


// Clamp alpha between 0 and 1 (safety check)
    if (alpha < 0.f) alpha = 0.f;
    if (alpha > 1.f) alpha = 1.f;


    // After 6 seconds, switch to CREDIT state
    if (elapsed > 6.f)
    {
        next = CREDIT;
    }
}

void opening_draw()
{
    if (!logo || !rect_mesh_center)
        return;

    
    render_editor_ui(logo, 1200.f, 700.f, alpha);
}

void opening_free()
{
    free_mesh();
}

void opening_unload()
{
    if (logo)
    {
        AEGfxTextureUnload(logo);
        logo = nullptr;
    }
}