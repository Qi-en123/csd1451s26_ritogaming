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
AEAudio* g_bgm_current_track = nullptr;
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
    g_audio.bgm_group = AEAudioCreateGroup();
    g_audio.sfx_group = AEAudioCreateGroup();
    g_audio.sfx_loop_group = AEAudioCreateGroup();

    // Load BGM (loops)
    g_audio.bgm_op_n_mainmenu = AEAudioLoadMusic("Assets/Audio/opening_n_mainmenu_bg.mp3");
    g_audio.bgm_maingame = AEAudioLoadMusic("Assets/Audio/main_game_bg.mp3");
    g_audio.bgm_bossroom = AEAudioLoadMusic("Assets/Audio/boss_room_bg.mp3");
    g_audio.bgm_credits = AEAudioLoadMusic("Assets/Audio/credits_bg.mp3");

    // Load SFX (play once)
    // Attacks
    g_audio.sfx_fireball = AEAudioLoadSound("Assets/Audio/mage_atk.mp3");
    g_audio.sfx_sword_atk = AEAudioLoadSound("Assets/Audio/sword_atk.mp3");

    // Damage taken
    g_audio.sfx_mage_hurt = AEAudioLoadSound("Assets/Audio/mage_hurt.mp3");
    g_audio.sfx_knight_hurt = AEAudioLoadSound("Assets/Audio/knight_hurt.mp3");

    // Puzzles
    g_audio.sfx_button_click = AEAudioLoadSound("Assets/Audio/button_click.mp3");
    g_audio.sfx_button_hover = AEAudioLoadSound("Assets/Audio/button_hover.mp3");
    g_audio.sfx_button_puzzle = AEAudioLoadSound("Assets/Audio/button_puzzle.mp3");
    g_audio.sfx_drag_pull = AEAudioLoadSound("Assets/Audio/dragging.mp3");
    g_audio.sfx_footsteps = AEAudioLoadSound("Assets/Audio/walking.mp3");

    g_audio.sfx_lose = AEAudioLoadSound("Assets/Audio/lose.mp3");
    g_audio.sfx_win = AEAudioLoadSound("Assets/Audio/win.mp3");
    
	// pre warning before entering boss room
    g_audio.sfx_warning = AEAudioLoadSound("Assets/Audio/warning.mp3");

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
    AEAudioStopGroup(g_audio.sfx_loop_group);

    //// Unload BGM
    AEAudioUnloadAudio(g_audio.bgm_op_n_mainmenu);
    AEAudioUnloadAudio(g_audio.bgm_maingame);
    AEAudioUnloadAudio(g_audio.bgm_bossroom);
    AEAudioUnloadAudio(g_audio.bgm_credits);

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
    AEAudioUnloadAudio(g_audio.sfx_lose);
    AEAudioUnloadAudio(g_audio.sfx_win);
    AEAudioUnloadAudio(g_audio.sfx_warning);

    // Unload groups
    AEAudioUnloadAudioGroup(g_audio.bgm_group);
    AEAudioUnloadAudioGroup(g_audio.sfx_group);
    AEAudioUnloadAudioGroup(g_audio.sfx_loop_group);

    AEAudioExit();
}

// ----------------------------------------------------------------------------
//
// Function: play_bgm
// Stops any currently playing BGM in the music group and starts the
// specified audio track, looping it indefinitely.
//
// ---------------------------------------------------------------------------- 
void play_bgm(AEAudio audio, float volume, float pitch, int loop)
{
    AEAudioStopGroup(g_audio.bgm_group); // stop any current BGM first
    AEAudioPlay(audio, g_audio.bgm_group, volume, pitch, loop); // 0.07, 1.0, -1 = loop forever
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

// ----------------------------------------------------------------------------
//
// Function: stop_sfx
// Immediately stops all audio currently playing in the SFX group.
//
// ----------------------------------------------------------------------------
void stop_sfx()
{
    AEAudioStopGroup(g_audio.sfx_group);
}

void play_sfx_loop(AEAudio audio, float volume, float pitch)
{
    AEAudioPlay(audio, g_audio.sfx_loop_group, volume, pitch, -1);
}

void stop_sfx_loop() 
{ 
    AEAudioStopGroup(g_audio.sfx_loop_group); 
}