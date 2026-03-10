#include "audio_manager.hpp"
#include <cstdio>

AudioManager g_audio;

void audio_init()
{
    AEAudioInit();

    // Create groups
    g_audio.bgm_group = AEAudioCreateGroup();
    g_audio.sfx_group = AEAudioCreateGroup();

    // Load BGM (loops)
    //g_audio.bgm_level1 = AEAudioLoadMusic("Assets/Audio/....");
    g_audio.bgm_mainmenu = AEAudioLoadMusic("Assets/Audio/main_menu_bg.mp3");

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

    if (!AEAudioIsValidAudio(g_audio.sfx_fireball))    printf("ERROR: sfx_fireball failed to load!\n");
    if (!AEAudioIsValidAudio(g_audio.sfx_knight_hurt)) printf("ERROR: sfx_player_hurt1 failed to load!\n");
    if (!AEAudioIsValidAudio(g_audio.sfx_mage_hurt)) printf("ERROR: sfx_player_hurt2 failed to load!\n");
    if (!AEAudioIsValidAudio(g_audio.bgm_mainmenu))    printf("ERROR: bgm_mainmenu failed to load!\n");

}

void audio_update()
{
    AEAudioUpdate(); // must be called every frame
}

void audio_free()
{
    // Stop all audio first
    AEAudioStopGroup(g_audio.bgm_group);
    AEAudioStopGroup(g_audio.sfx_group);

    //// Unload BGM
    //AEAudioUnloadAudio(g_audio.bgm_level1);
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

void play_bgm(AEAudio audio)
{
    AEAudioStopGroup(g_audio.bgm_group); // stop any current BGM first
    AEAudioPlay(audio, g_audio.bgm_group, 1.0f, 1.0f, -1); // -1 = loop forever
}

void stop_bgm()
{
    AEAudioStopGroup(g_audio.bgm_group);
}

void pause_bgm()
{
    AEAudioPauseGroup(g_audio.bgm_group);
}

void resume_bgm()
{
    AEAudioResumeGroup(g_audio.bgm_group);
}

void play_sfx(AEAudio audio)
{
    AEAudioPlay(audio, g_audio.sfx_group, 1.0f, 1.0f, 0); // 0 = play once
}