/*!************************************************************************
\file:						audio_manager.cpp
\author:					Ahmad Qusyairi Bin Mohammad Jinna
\par DP email:				ahmadqusyairi.b@digipen.edu

\brief:						Implementation of the audio management system 
                            using the Alpha Engine audio API. Handles 
                            loading, grouping, and playback of BGM and SFX

**************************************************************************/
#include "audio_manager.hpp"
#include <cstdio>

AudioManager g_audio;

// ----------------------------------------------------------------------------
//
// Function: audio_init
// Initializes the audio engine, creates BGM/SFX groups, and loads all
// necessary audio assets from the disk into memory.
//
// ---------------------------------------------------------------------------- 
void audio_init()
{
    AEAudioInit();

    // Create groups
    g_audio.bgm_group           = AEAudioCreateGroup();
    g_audio.sfx_group           = AEAudioCreateGroup();

    // Load BGM (loops)
    g_audio.bgm_mainmenu        = AEAudioLoadMusic("Assets/Audio/main_menu_bg.mp3");

    // Load SFX (play once)
    // Attacks
    g_audio.sfx_fireball        = AEAudioLoadSound("Assets/Audio/mage_atk.mp3");
    g_audio.sfx_sword_atk       = AEAudioLoadSound("Assets/Audio/sword_atk.mp3");

    // Damage taken
    g_audio.sfx_mage_hurt       = AEAudioLoadSound("Assets/Audio/mage_hurt.mp3");
    g_audio.sfx_knight_hurt     = AEAudioLoadSound("Assets/Audio/knight_hurt.mp3");

    // Puzzles
    g_audio.sfx_button_click    = AEAudioLoadSound("Assets/Audio/button_click.mp3");
    g_audio.sfx_button_hover    = AEAudioLoadSound("Assets/Audio/button_hover.mp3");
    g_audio.sfx_button_puzzle   = AEAudioLoadSound("Assets/Audio/button_puzzle.mp3");
    g_audio.sfx_drag_pull       = AEAudioLoadSound("Assets/Audio/drag_pull.mp3");
    g_audio.sfx_footsteps       = AEAudioLoadSound("Assets/Audio/footsteps.mp3");

}

// ----------------------------------------------------------------------------
//
// Function: audio_update
// Updates the internal state of the audio engine. Must be called once
// per frame within the main game loop.
//
// ---------------------------------------------------------------------------- 
void audio_update()
{
    AEAudioUpdate(); // must be called every frame
}

// ----------------------------------------------------------------------------
//
// Function: audio_free
// Stops all active audio, unloads all sound assets from memory, 
// destroys audio groups, and shuts down the audio engine.
//
// ---------------------------------------------------------------------------- 
void audio_free()
{
    // Stop all audio first
    AEAudioStopGroup(g_audio.bgm_group);
    AEAudioStopGroup(g_audio.sfx_group);

    //// Unload BGM
    AEAudioUnloadAudio(g_audio.bgm_mainmenu);

    //// Unload SFX
    AEAudioUnloadAudio(g_audio.sfx_fireball);
    AEAudioUnloadAudio(g_audio.sfx_button_click);
    AEAudioUnloadAudio(g_audio.sfx_button_hover);
    AEAudioUnloadAudio(g_audio.sfx_button_puzzle);
    AEAudioUnloadAudio(g_audio.sfx_drag_pull);
    AEAudioUnloadAudio(g_audio.sfx_footsteps);
    AEAudioUnloadAudio(g_audio.sfx_knight_hurt);
    AEAudioUnloadAudio(g_audio.sfx_mage_hurt);
    AEAudioUnloadAudio(g_audio.sfx_sword_atk);

    // Unload groups
    AEAudioUnloadAudioGroup(g_audio.bgm_group);
    AEAudioUnloadAudioGroup(g_audio.sfx_group);

    AEAudioExit();
}

// ----------------------------------------------------------------------------
//
// Function: play_bgm
// Stops any currently playing BGM in the music group and starts the
// specified audio track, looping it indefinitely.
//
// ---------------------------------------------------------------------------- 
void play_bgm(AEAudio audio)
{
    AEAudioStopGroup(g_audio.bgm_group); // stop any current BGM first
    AEAudioPlay(audio, g_audio.bgm_group, 0.07f, 1.0f, -1); // -1 = loop forever
}

// ----------------------------------------------------------------------------
//
// Function: stop_bgm
// Immediately stops all audio currently playing in the BGM group.
//
// ---------------------------------------------------------------------------- 
void stop_bgm()
{
    AEAudioStopGroup(g_audio.bgm_group);
}

// ----------------------------------------------------------------------------
//
// Function: pause_bgm
// Pauses the BGM group. Playback state is preserved for later recovery.
//
// ---------------------------------------------------------------------------- 
void pause_bgm()
{
    AEAudioPauseGroup(g_audio.bgm_group);
}

// ----------------------------------------------------------------------------
//
// Function: resume_bgm
// Resumes playback for any paused audio within the BGM group.
//
// ---------------------------------------------------------------------------- 
void resume_bgm()
{
    AEAudioResumeGroup(g_audio.bgm_group);
}

// ----------------------------------------------------------------------------
//
// Function: play_sfx
// Triggers a sound effect to play in the SFX group with adjustable
// volume, pitch, and loop count.
//
// ---------------------------------------------------------------------------- 
void play_sfx(AEAudio audio, float volume, float pitch, int loop)
{
    AEAudioPlay(audio, g_audio.sfx_group, volume, pitch, loop);
}