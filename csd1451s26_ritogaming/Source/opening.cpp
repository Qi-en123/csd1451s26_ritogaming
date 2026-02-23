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

static AEGfxTexture* logo = nullptr;
static float         elapsed = 0.f;
static float         alpha = 0.f;
static bool          done = false;

void opening_load()
{
    logo = AEGfxTextureLoad("Assets/DigiPen_BLACK.png");
 
}

void opening_init()
{
    init_mesh();  
    AEGfxSetBackgroundColor(0.f, 0.f, 0.f);
    elapsed = 0.f;
    alpha = 0.f;
    done = false;
    cam.pos.x = 0.f;
    cam.pos.y = 0.f;
    zoom = 1.0f;
}

void opening_update()
{
    dt = static_cast<float>(AEFrameRateControllerGetFrameTime());
    elapsed += dt;

    if (elapsed < 2.f)                          // fade in over 2s
        alpha = elapsed / 2.f;
    else if (elapsed > 4.f)                     // fade out over 2s (4s to 6s)
        alpha = 1.f - ((elapsed - 4.f) / 2.f);
    else
        alpha = 1.f;                            // fully visible from 2s to 4s

	// to clamp the alpha value between 0 and 1, just in case
    if (alpha < 0.f) alpha = 0.f;
    if (alpha > 1.f) alpha = 1.f;

    if (elapsed > 6.f)
    {
        done = true;
        next = CREDIT;
    }
}

void opening_draw()
{
    if (!logo || !rect_mesh_center)
        return;

    
    render_blending_sprite(logo, 1200.f, 700.f, alpha);
}

void opening_free()
{
    // nothing to free per frame
}

void opening_unload()
{
    if (logo)
    {
        AEGfxTextureUnload(logo);
        logo = nullptr;
    }
}