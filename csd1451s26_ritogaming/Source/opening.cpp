/*!************************************************************************
\file:						opening.cpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu
\brief:						Defines the load/init/update/draw/free/unload cycle for the
                            opening logo screen, including fade-in and fade-out effects.
                            Timing and display values are driven by opening_config.txt.
**************************************************************************/

#include "opening.hpp"
#include "renderer.hpp"
#include "AEEngine.h"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include "audio_manager.hpp"

// ----------------------------------------------------------------------------
//
// Anonymous namespace for internal configuration and state variables.
//
// ----------------------------------------------------------------------------
namespace
{
    struct OpeningConfig
    {
        std::string logo_path;
        float       logo_width;
        float       logo_height;
        float       fade_in_duration;   // seconds logo fades in
        float       hold_duration;      // seconds logo stays fully visible
        float       fade_out_duration;  // seconds logo fades out
    };

    OpeningConfig   config = {};        // Stores parameters loaded from file
    AEGfxTexture* logo_tex = nullptr; // Texture pointer for the logo image
    float           elapsed = 0.f;      // Timer to track state duration
    float           alpha = 0.f;        // Current transparency of the logo
}

// ----------------------------------------------------------------------------
//
// Function: compute_fade_alpha
// Calculates the [0, 1] alpha value based on the current elapsed time 
// relative to the configured fade-in, hold, and fade-out durations.
//
// ----------------------------------------------------------------------------
static float compute_fade_alpha(float elapsed, const OpeningConfig& cfg)
{
    const float fade_in_end = cfg.fade_in_duration;
    const float hold_end = fade_in_end + cfg.hold_duration;
    const float sequence_end = hold_end + cfg.fade_out_duration;

    // Fade-in Phase
    if (elapsed < fade_in_end)
        return elapsed / cfg.fade_in_duration;

    // Hold Phase
    if (elapsed < hold_end)
        return 1.f;

    // Fade-out Phase
    if (elapsed < sequence_end)
        return 1.f - ((elapsed - hold_end) / cfg.fade_out_duration);

    return 0.f;
}

// ----------------------------------------------------------------------------
//
// Function: total_sequence_duration
// Returns the total time the opening screen should run.
//
// ----------------------------------------------------------------------------
static float total_sequence_duration(const OpeningConfig& cfg)
{
    return cfg.fade_in_duration + cfg.hold_duration + cfg.fade_out_duration;
}

// ----------------------------------------------------------------------------
//
// Function: opening_load
// Reads the configuration file to set up logo path and timing, then 
// loads the texture asset.
//
// ----------------------------------------------------------------------------
void opening_load()
{
    std::ifstream file("Assets/Opening/opening.txt");
    std::string   key;
    std::string   value;

    while (file >> key >> value)
    {
        if (key == "logo_path")              config.logo_path = value;
        else if (key == "logo_width")        config.logo_width = std::stof(value);
        else if (key == "logo_height")       config.logo_height = std::stof(value);
        else if (key == "fade_in_duration")  config.fade_in_duration = std::stof(value);
        else if (key == "hold_duration")     config.hold_duration = std::stof(value);
        else if (key == "fade_out_duration") config.fade_out_duration = std::stof(value);
    }

    logo_tex = AEGfxTextureLoad(config.logo_path.c_str());
}

// ----------------------------------------------------------------------------
//
// Function: opening_init
// Resets timers and camera properties for the start of the state.
//
// ----------------------------------------------------------------------------
void opening_init()
{
    init_mesh();
    AEGfxSetBackgroundColor(0.f, 0.f, 0.f);

    elapsed = 0.f;
    alpha = 0.f;
    cam.pos.x = 0.f;
    cam.pos.y = 0.f;
    zoom = 1.f;

    g_bgm_current_track = &g_audio.bgm_op_n_mainmenu;
    play_bgm(g_audio.bgm_op_n_mainmenu, 0.3f);
}

// ----------------------------------------------------------------------------
//
// Function: opening_update
// Updates the timer, computes alpha, and transitions to the main menu
// once the sequence is complete.
//
// ----------------------------------------------------------------------------
void opening_update()
{
    dt = static_cast<float>(AEFrameRateControllerGetFrameTime());
    elapsed += dt;

    // Clamp alpha between 0 and 1
    alpha = compute_fade_alpha(elapsed, config);
    if (alpha < 0.f) alpha = 0.f;
    else if (alpha > 1.f) alpha = 1.f;

    // Check if sequence is finished
    if (elapsed > total_sequence_duration(config))
        next = MAINMENU;
}

// ----------------------------------------------------------------------------
//
// Function: opening_draw
// Handles the rendering of the logo using the computed alpha value.
//
// ----------------------------------------------------------------------------
void opening_draw()
{
    if (!logo_tex || !rect_mesh_center)
        return;

    render_editor_ui(logo_tex, config.logo_width, config.logo_height, alpha);
}

// ----------------------------------------------------------------------------
//
// Function: opening_free
// Cleans up the mesh created during initialization.
//
// ----------------------------------------------------------------------------
void opening_free()
{
    free_mesh();
}

// ----------------------------------------------------------------------------
//
// Function: opening_unload
// Unloads the texture from memory to free up resources.
//
// ----------------------------------------------------------------------------
void opening_unload()
{
    if (logo_tex)
    {
        AEGfxTextureUnload(logo_tex);
        logo_tex = nullptr;
    }
}