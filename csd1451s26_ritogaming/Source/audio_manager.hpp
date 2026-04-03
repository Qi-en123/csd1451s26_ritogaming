#pragma once

/*!************************************************************************
\file:						audio_manager.hpp
\author:					Ahmad Qusyairi Bin Mohammad Jinna
\par DP email:				ahmadqusyairi.b@digipen.edu

\brief:						Header file containing the AudioManager
                            structure and function declarations for
                            global audio control.
**************************************************************************/

#include "AEAudio.h"



// ----------------------------------------------------------------------------
//
// Struct: AudioManager
// A container for all audio handles and groups used within the game.
// Ensures centralized access to BGM and SFX assets.
//
// ---------------------------------------------------------------------------- 
struct AudioManager
{
    // Groups
    AEAudioGroup bgm_group{};
    AEAudioGroup sfx_group{};

    // BGM
    AEAudio bgm_op_n_mainmenu{};
    AEAudio bgm_maingame{};
    AEAudio bgm_bossroom{};
    AEAudio bgm_credits{};

    // SFX
    AEAudio sfx_fireball{};
    AEAudio sfx_sword_atk{};
    AEAudio sfx_mage_hurt{};
    AEAudio sfx_knight_hurt{};
    AEAudio sfx_button_click{};
    AEAudio sfx_button_hover{};
    AEAudio sfx_button_puzzle{};
    AEAudio sfx_drag_pull{};
    AEAudio sfx_footsteps{};

};

extern AudioManager g_audio;
extern AEAudio* g_bgm_current_track;
// ----------------------------------------------------------------------------
//
// Function: audio_init
// Sets up the Alpha Engine audio system and loads initial assets.
//
// ---------------------------------------------------------------------------- 
void audio_init();

// ----------------------------------------------------------------------------
//
// Function: audio_update
// Processes audio callbacks. Should be called in the main loop.
//
// ---------------------------------------------------------------------------- 
void audio_update();

// ----------------------------------------------------------------------------
//
// Function: audio_free
// Cleans up all audio resources and shuts down the engine.
//
// ---------------------------------------------------------------------------- 
void audio_free();

// ----------------------------------------------------------------------------
//
// Function: play_bgm
// Plays a specific music track on the background music group.
//
// ---------------------------------------------------------------------------- 
void play_bgm(AEAudio audio, float volume = 0.07f, float pitch = 1.0f, int loop = -1);

// ----------------------------------------------------------------------------
//
// Function: stop_bgm
// Stops the background music group entirely.
//
// ---------------------------------------------------------------------------- 
void stop_bgm();

// ----------------------------------------------------------------------------
//
// Function: pause_bgm
// Pauses the current background music group.
//
// ---------------------------------------------------------------------------- 
void pause_bgm();

// ----------------------------------------------------------------------------
//
// Function: resume_bgm
// Resumes the background music group from a paused state.
//
// ---------------------------------------------------------------------------- 
void resume_bgm();

// ----------------------------------------------------------------------------
//
// Function: play_sfx
// Plays a sound effect with optional volume, pitch, and loop parameters.
//
// ---------------------------------------------------------------------------- 
void play_sfx(AEAudio audio, float volume = 0.1f, float pitch = 1.0f, int loop = 0);


void stop_sfx();
