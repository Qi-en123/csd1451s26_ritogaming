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
    AEGfxTexture* background = nullptr;
    AEGfxTexture* ghost = nullptr;
    AEGfxTexture* knight = nullptr;
	f32 knight_scale = 200.f; //  normal size for knight
	f32 knight_x = -700.f; // starting x position for knight
    f32 ghost_scale = 10.f;  //  start small
    f32 ghost_x = -400.f;    //  starting x position
    bool knight_returning = false; // track if knight is running back
	f32 ghost_y = -100.f;//  base y position for ghost
	f32 ghost_bounce_timer = 0.f; // timer for sine wave bounce
}
void mainmenu_load()
{
	// Load textures for background, ghost, and knight
    background = AEGfxTextureLoad("Assets/mainmenu_bg.png");
    ghost = AEGfxTextureLoad("Assets/Tiles/tile_0006.png");
    knight = AEGfxTextureLoad("Assets/Tiles/tile_0004.png");
 

}

// =========================================================
// MAINMENU Screen Initialization
// =========================================================
void mainmenu_init()
{
    init_mesh();  

    cam.pos.x = 0.f;  // Reset camera horizontal position to center (no offset on X axis)

    cam.pos.y = 0.f;  // Reset camera vertical position to center (no offset on Y axis)

    zoom = 1.0f;      // Set camera zoom to normal scale (1.0 = no zoom in/out)

	ghost_scale = 10.f; // Start ghost small

	ghost_x = -400.f; // Reset ghost to original position and small size

	knight_scale = 200.f; // Set knight to normal size

	knight_x = -700.f; // Reset knight to original position

}


// =========================================================
// MAINMENU Screen Update (runs every frame)
// =========================================================
void mainmenu_update()
{
    f32 dt = (f32)AEFrameRateControllerGetFrameTime();

	if (knight_returning) // if knight is currently running back to original position
    {
        //  Knight runs back from right side to original position
        knight_x += 200.f * dt; // run back fast

		if (knight_x >= -700.f) //  Knight reached original position, reset everything
        {
            //  Knight back at original position, reset everything
			knight_x = -700.f; // reset to original position
			knight_returning = false; // stop returning
            ghost_scale = 10.f;  // ghost spawns small again
			ghost_x = -400.f; // reset ghost position
        }
        return;
    }

    // Ghost grows to full size first
	if (ghost_scale < 200.f) // grow until full size (200)
    {
		ghost_scale += 100.f * dt; // grow at 100 units/sec
		return; // wait until ghost fully grown before moving
    }

    // Ghost runs away, knight chases
	ghost_x += 200.f * dt; // ghost runs right at 120 units/sec
	knight_x += 200.f * dt; // knight chases at same speed
	ghost_bounce_timer += dt; // increase bounce timer for sine wave
    ghost_y = -100.f + sinf(ghost_bounce_timer * 5.f) * 20.f; // bounce up and down like sin wave

    //  knight exits screen,  knight starts returning
    if (knight_x > 1000.f)
    {
        knight_returning = true;
        knight_x = -1000.f;     //  knight re-enters from left side
    }

    if (!AESysDoesWindowExist())
        next = GS_QUIT;
}

void mainmenu_draw()
{

	// Draw background, knight, and ghost with blending for transparency
    if(background) 
        render_blending_sprite(background, 1600.f, 900.f, 1.0f);

    if (knight)
        render_blending_sprite(knight, knight_scale, knight_scale, 1.0f, knight_x, -100.f);

    if (ghost)
    {
        f32 ghost_alpha = ghost_scale / 200.f; //  0.0 to 1.0 based on scale
        render_blending_sprite(ghost, ghost_scale, ghost_scale, ghost_alpha, ghost_x, ghost_y);
    }
    
}

void mainmenu_free()
{
    free_mesh(); // or whatever the cleanup function is called
}

void mainmenu_unload()
{
	// Unload textures to free memory
    if (background)
    {
        AEGfxTextureUnload(background);
        background = nullptr;
    }
    if (knight)
    {
        AEGfxTextureUnload(knight);
        knight = nullptr;
    }
    if (ghost)
    {
        AEGfxTextureUnload(ghost);
        ghost = nullptr;
    }
}