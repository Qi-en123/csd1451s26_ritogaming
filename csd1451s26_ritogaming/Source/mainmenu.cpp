/*!************************************************************************
\file:						mainmenu.hpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This file defines the 6 cycle function that
							Loads, initializes, updates, draws, and unloads the mainmenu game state, which features an animated knight and ghoul, interactive buttons, 
                            and a custom cursor, all rendered using AEEngine.
**************************************************************************/

#include "renderer.hpp"
#include "AEEngine.h"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "utils.hpp"



namespace {
	// Textures and Fonts
    AEGfxTexture* ground = nullptr;
    AEGfxTexture* wall = nullptr;
    AEGfxTexture* door = nullptr;
    AEGfxTexture* ghoul = nullptr;
    AEGfxTexture* knight = nullptr;
    AEGfxTexture* button = nullptr;
    AEGfxTexture* cursor = nullptr;
    AEGfxTexture* banner = nullptr;

    static s8  font = -1;
    
	// Animation variables
	f32 knight_scale = 200.f; //  normal size for knight
	f32 knight_x = -700.f; // starting x position for knight
    f32 ghoul_scale = 10.f;  //  start small
    f32 ghoul_x = -400.f;    //  starting x position
    bool knight_returning = false; // track if knight is running back
	f32 ghoul_y = -40.f;//  base y position for ghoul
	f32 ghoul_bounce_timer = 0.f; // timer for sine wave bounce


    // Button positions
    const f32 button_w = 280.f;
    const f32 button_h = 100.f;
    const f32 button_x = 500.f;  // right side of screen
    f32 button_y[5] = { 250.f, 125.f, 0.f, -125.f, -250.f }; // evenly spaced
    const char* button_labels[5] = { "Play", "Levels", "Guide", "Credits", "Quit" };
    s32 mouse_x, mouse_y;
    f32 knight_bob_timer = 0.f;
    struct DustParticle {
        f32 x, y;
        f32 alpha;
        f32 scale;
        f32 lifetime; // how long it lives
        f32 vx, vy;   // velocity
        bool active;
    };
    const int MAX_DUST = 60;
    DustParticle dust_particles[MAX_DUST];

    // --- ghoul Trail ---
    struct ghoulTrail {
        f32 x, y;
        f32 scale;
        f32 alpha;
        bool active;
    };
    const int MAX_TRAIL = 5;
    ghoulTrail ghoul_trail[MAX_TRAIL];
    f32 trail_spawn_timer = 0.f;
    
    bool show_exit_confirm = false;


}
void spawn_dust(f32 x, f32 y)
{
    for (int i = 0; i < MAX_DUST; ++i)
    {
        if (!dust_particles[i].active)
        {
            dust_particles[i].x = x + ((rand() % 40) - 20.f);
            dust_particles[i].y = y;
            dust_particles[i].alpha = 1.0f;                         // opaque
            dust_particles[i].scale = (f32)(rand() % 30 + 20);     //  bigger
            dust_particles[i].lifetime = 0.6f + (rand() % 10) * 0.05f; //   lasts longer
            dust_particles[i].vx = -((rand() % 80) + 20.f); 
            dust_particles[i].vy = (f32)(rand() % 60 + 30);     // 30-90 — floats higher
            dust_particles[i].active = true;
            break;
        }
    }
}

void mainmenu_load()
{
	// Load textures 
    ground = AEGfxTextureLoad("Assets/Tiles/tile_0002.png");
    wall = AEGfxTextureLoad("Assets/Tiles/tile_0001.png");
    door = AEGfxTextureLoad("Assets/Tiles/tile_0007.png");
    ghoul = AEGfxTextureLoad("Assets/Tiles/tile_0006.png");
    knight = AEGfxTextureLoad("Assets/Tiles/tile_0004.png");
    button = AEGfxTextureLoad("Assets/Admin/buttonLong_brown_pressed.png");
    cursor = AEGfxTextureLoad("Assets/Admin/cursorSword_silver.png");
    banner = AEGfxTextureLoad("Assets/Admin/banner_classic_curtain.png");

 
	// Load font from Assets folder
    font = AEGfxCreateFont("Assets/Exo2-Regular.ttf", 48);
    printf("Font value: %d\n", font);
    if (font == -1)
    {
        return;
    }
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

	ghoul_scale = 10.f; // Start ghoul small

	ghoul_x = -400.f; // Reset ghoul to original position and small size

    ghoul_y = -40.f;//  base y position for ghoul

	knight_scale = 200.f; // Set knight to normal size

	knight_x = -700.f; // Reset knight to original position

    AEInputShowCursor(false); // Hide system cursor, we'll draw our own
    // Clear particles and trail on init
    for (int i = 0; i < MAX_DUST; ++i)   dust_particles[i].active = false;
    for (int i = 0; i < MAX_TRAIL; ++i)  ghoul_trail[i].active = false;
    trail_spawn_timer = 0.f;
}


// =========================================================
// MAINMENU Screen Update (runs every frame)
// =========================================================
void mainmenu_update()
{
    dt = (f32)AEFrameRateControllerGetFrameTime();

    // Get mouse position 
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    if (show_exit_confirm)
    {
        // Yes button (left)
        if (IsButtonClicked(-100.f, -50.f, 200.f, 80.f))
        {
            next = GS_QUIT;
        }

        // No button (right)
        if (IsButtonClicked(100.f, -50.f, 200.f, 80.f))
        {
            show_exit_confirm = false;  // close popup
        }

        return; // stop other menu logic while popup is active
    }


    // Check button clicks
    if (IsButtonClicked(button_x, button_y[0], button_w, button_h)) // Play
        next = GS_LEVEL1;
	if (IsButtonClicked(button_x, button_y[1], button_w, button_h)) // Levels
    {
        next = GS_LEVEL_EDITOR;
    }
      
    if (IsButtonClicked(button_x, button_y[2], button_w, button_h)) // Guide
    {
       //next = GS_GUIDE;
    }
    if (IsButtonClicked(button_x, button_y[3], button_w, button_h)) // Credits
    {
        next = CREDIT;
    }
    if (IsButtonClicked(button_x, button_y[4], button_w, button_h))
    {
        show_exit_confirm = true;   // show confirmation instead of quitting
    }




	if (knight_returning) // if knight is currently running back to original position
    {
        //  Knight runs back from right side to original position
        knight_x += 200.f * dt; // run back fast

		if (knight_x >= -700.f) //  Knight reached original position, reset everything
        {
            //  Knight back at original position, reset everything
			knight_x = -700.f; // reset to original position
			knight_returning = false; // stop returning
            ghoul_scale = 10.f;  // ghoul spawns small again
			ghoul_x = -400.f; // reset ghoul position
        }
        return;
    }

    // ghoul grows to full size first
	if (ghoul_scale < 200.f) // grow until full size (200)
    {
		ghoul_scale += 100.f * dt; // grow at 100 units/sec
		return; // wait until ghoul fully grown before moving
    }

    // ghoul runs away, knight chases
	ghoul_x += 200.f * dt; // ghoul runs right at 120 units/sec
	knight_x += 200.f * dt; // knight chases at same speed
	ghoul_bounce_timer += dt; // increase bounce timer for sine wave
    ghoul_y = -50.f + sinf(ghoul_bounce_timer * 5.f) * 20.f; // bounce up and down like sin wave

    //  knight exits screen,  knight starts returning
    if (knight_x > 1000.f)
    {
        knight_returning = true;
        knight_x = -1000.f;     //  knight re-enters from left side
    }

  

    // --- Spawn dust under knight feet while running ---
    static f32 dust_timer = 0.f;
    dust_timer += dt;
    if (!knight_returning && ghoul_scale >= 200.f && dust_timer > 0.1f)
    {
        spawn_dust(knight_x - 40.f, -150.f); // left foot  — was -10.f
        spawn_dust(knight_x - 60.f, -150.f); // right foot — was +10.f
        dust_timer = 0.f;
    }

    // --- Update existing dust particles ---
    for (int i = 0; i < MAX_DUST; ++i)
    {
        if (!dust_particles[i].active) continue;

        dust_particles[i].lifetime -= dt;
        dust_particles[i].x += dust_particles[i].vx * dt;
        dust_particles[i].y += dust_particles[i].vy * dt;
        dust_particles[i].alpha -= dt * 1.2f; // fade out

        if (dust_particles[i].lifetime <= 0.f || dust_particles[i].alpha <= 0.f)
            dust_particles[i].active = false;
    }

    // --- Spawn ghoul trail every 0.08s while ghoul is full size ---
    trail_spawn_timer += dt;
    if (ghoul_scale >= 200.f && trail_spawn_timer > 0.08f)
    {
        // Shift old trail entries back
        for (int i = MAX_TRAIL - 1; i > 0; --i)
            ghoul_trail[i] = ghoul_trail[i - 1];

        // Insert newest position at front
        ghoul_trail[0].x = ghoul_x;
        ghoul_trail[0].y = ghoul_y;
        ghoul_trail[0].scale = ghoul_scale;
        ghoul_trail[0].alpha = 0.35f; // trail starts semi-transparent
        ghoul_trail[0].active = true;

        trail_spawn_timer = 0.f;
    }

    // --- Fade out trail over time ---
    for (int i = 0; i < MAX_TRAIL; ++i)
    {
        if (!ghoul_trail[i].active) continue;
        ghoul_trail[i].alpha -= dt * 0.8f;
        if (ghoul_trail[i].alpha <= 0.f)
            ghoul_trail[i].active = false;
    }
    if (!knight_returning && ghoul_scale >= 200.f)
        knight_bob_timer += dt;
    else
        knight_bob_timer = 0.f;
}

void mainmenu_draw()
{
    if (show_exit_confirm)
{
   

    // Popup box
    render_blending_sprite(button, 600.f, 300.f, 1.0f, 0.f, 0.f);

    // Text
    if (font != -1)
    {
        AEGfxPrint(font, "Are you sure you want to quit?",
            -0.35f, 0.1f,
            1.f,
            0.f, 0.f, 0.f, 1.f);

        AEGfxPrint(font, "Yes",
            -0.18f, -0.1f,
            1.f,
            0.f, 0.f, 0.f, 1.f);

        AEGfxPrint(font, "No",
            0.12f, -0.1f,
            1.f,
            0.f, 0.f, 0.f, 1.f);
    }

    // Yes button
    render_blending_sprite(button, 200.f, 80.f, 1.0f, -100.f, -50.f,
        IsButtonHovered(-100.f, -50.f, 200.f, 80.f));

    // No button
    render_blending_sprite(button, 200.f, 80.f, 1.0f, 100.f, -50.f,
        IsButtonHovered(100.f, -50.f, 200.f, 80.f));
}

	// Draw background, knight, and ghoul with blending for transparency
     // Draw wall tiles across top portion of screen
    for (f32 x = -800.f; x <= 800.f; x += 100.f)
    {
        for (f32 y = -50.f; y < 500.f; y += 100.f)
        {
            if (wall) render_blending_sprite(wall, 100.f, 100.f, 1.0f, x, y);
        }
    }

    // Draw ground tiles across bottom portion
    for (f32 x = -800.f; x <= 800.f; x += 100.f)
    {
        for (f32 y = -450.f; y < -50.f; y += 100.f)
        {
            if (ground) render_blending_sprite(ground, 100.f, 100.f, 1.0f, x, y);
        }
    }

    // Draw door in the middle left area
    if (door) render_blending_sprite(door, 200.f, 200.f, 1.0f, -400.f, 0.f);


    // --- Draw dust particles (after knight) ---
    for (int i = 0; i < MAX_DUST; ++i)
    {
        if (!dust_particles[i].active) continue;
        if (ground) // reuse any small square texture as dust puff
            render_blending_sprite(ground,
                dust_particles[i].scale,
                dust_particles[i].scale,
                dust_particles[i].alpha,
                dust_particles[i].x,
                dust_particles[i].y);
    }


    if (knight)
    {
        // Vertical bob — bounces up and down like footsteps
        f32 knight_y = -50.f + sinf(knight_bob_timer * 10.f) * 8.f;

        // Lean forward slightly — tilt using scale trick (squish X slightly)
        f32 lean_x = knight_scale * 0.92f;

        // Squash and stretch — slightly squish on landing, stretch on lift
        f32 squash = 1.f + sinf(knight_bob_timer * 10.f) * 0.05f;
        f32 draw_w = lean_x * squash;
        f32 draw_h = knight_scale / squash;

        render_blending_sprite(knight, draw_w, draw_h, 1.0f, knight_x, knight_y);
    }
    // --- Draw ghoul trail (before ghoul so it appears behind) ---
    for (int i = MAX_TRAIL - 1; i >= 0; --i)
    {
        if (!ghoul_trail[i].active) continue;
        if (ghoul)
            render_blending_sprite(ghoul,
                ghoul_trail[i].scale,
                ghoul_trail[i].scale,
                ghoul_trail[i].alpha,
                ghoul_trail[i].x,
                ghoul_trail[i].y);
    }




    if (ghoul)
    {
        f32 ghoul_alpha = ghoul_scale / 200.f; //  0.0 to 1.0 based on scale
        render_blending_sprite(ghoul, ghoul_scale, ghoul_scale, ghoul_alpha, ghoul_x, ghoul_y);
    }

	// Draw buttons and labels
    for (int i = 0; i < 5; ++i)
    {
        // Draw button texture
        if (button)
            render_blending_sprite(button, button_w, button_h, 1.0f, button_x, button_y[i],
                IsButtonHovered(button_x, button_y[i], button_w, button_h));

        if (font != -1)
        {
            // Measure text size
            f32 text_width = 0.f, text_height = 0.f;
            AEGfxGetPrintSize(font, button_labels[i], 1.f, &text_width, &text_height);

            //  Center text on button
			f32 norm_x = (button_x / 800.f) - (text_width / 2.f); // Normalize button X to -1 to 1, then shift left by half text width
            f32 norm_y = (button_y[i] - 15.f) / 450.f;

			// print starts from left to right, unlike render_blending_sprite which is centered
            AEGfxPrint(font, button_labels[i],
                norm_x, norm_y,
                1.f,
                1.f, 1.f, 1.f, 1.f);
        }
    }
	

    //  Draw banner on top of door
    if (banner) render_blending_sprite(banner, 450.f, 100.f, 1.0f, -400.f, 200.f);

    //  Draw title text on banner
    if (font != -1)
    {
        f32 text_width = 0.f, text_height = 0.f;
		AEGfxGetPrintSize(font, "Ahmad's Dungeon", 1.f, &text_width, &text_height); // measure text size for centering
        f32 norm_x = (-400.f / 800.f) - (text_width / 2.f); // center on banner
		f32 norm_y = (200.f-10.f) / 450.f; // position on banner
        AEGfxPrint(font, "Ahmad's Dungeon",
            norm_x, norm_y,
            1.f,
            0.f, 0.f, 0.f, 1.f); // black text
    }

    if (show_exit_confirm)
    {


        // Dark overlay using blending rectangle
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetRenderMode(AE_GFX_RM_COLOR); // just color, no texture
        AEGfxSetColorToMultiply(0.f, 0.f, 0.f, 0.5f); // 50% black tint
        draw_square(0.f, 0.f, 1600.f, 900.f); // use your square_mesh

        // Popup box
        render_blending_sprite(button, 800.f, 400.f, 1.0f, 0.f, 0.f);


        // Yes button
        render_blending_sprite(button, 200.f, 80.f, 1.0f, -100.f, -50.f,
            IsButtonHovered(-100.f, -50.f, 200.f, 80.f));

        // No button
        render_blending_sprite(button, 200.f, 80.f, 1.0f, 150.f, -50.f,
            IsButtonHovered(150.f, -50.f, 200.f, 80.f));

        // Text
        if (font != -1)
        {
            AEGfxPrint(font, "Are you sure you want to quit?",
                -0.40f, 0.1f,
                1.f,
                0.f, 0.f, 0.f, 1.f);

            AEGfxPrint(font, "Yes",
                -0.18f, -0.14f,
                1.f,
                0.f, 0.f, 0.f, 1.f);

            AEGfxPrint(font, "No",
                0.12f, -0.14f,
                1.f,
                0.f, 0.f, 0.f, 1.f);
        }
    }

    // Draw custom cursor at mouse position
    if (cursor)
    {
        // Convert mouse screen coords to world coords
        f32 world_x = (f32)mouse_x - 800.f; // center origin
        f32 world_y = -(f32)mouse_y + 450.f; // flip Y axis

        render_blending_sprite(cursor, 50.f, 50.f, 1.0f, world_x, world_y);
    }

}

void mainmenu_free()
{
	free_mesh(); // Free any dynamically allocated memory for meshes
}

void mainmenu_unload()
{
	// Unload textures to free memory
    if (ground)
    {
        AEGfxTextureUnload(ground);
        ground = nullptr;
    }
    if (wall)
    {
        AEGfxTextureUnload(wall);
        wall = nullptr;
    }
    if (door)
    {
        AEGfxTextureUnload(door);
        door = nullptr;
    }
    if (knight)
    {
        AEGfxTextureUnload(knight);
        knight = nullptr;
    }
    if (ghoul)
    {
        AEGfxTextureUnload(ghoul);
        ghoul = nullptr;
    }

    if (button)
    {
        AEGfxTextureUnload(button);
        button = nullptr;
    }

    if (cursor)
    {
        AEGfxTextureUnload(cursor);
        cursor = nullptr;
    }

    if (banner)
    {
        AEGfxTextureUnload(banner);
        banner = nullptr;
    }

    if (font != -1)
    {
        AEGfxDestroyFont(font);
        font = -1;
    }

}