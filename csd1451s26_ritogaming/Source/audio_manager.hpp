#pragma once

#include "AEAudio.h"

struct AudioManager
{
    // Groups
    AEAudioGroup bgm_group{};
    AEAudioGroup sfx_group{};

    // BGM
    //AEAudio bgm_level1{};
    AEAudio bgm_mainmenu{};

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

void audio_init();
void audio_update();
void audio_free();

void play_bgm(AEAudio audio);
void stop_bgm();
void pause_bgm();
void resume_bgm();

void play_sfx(AEAudio audio, float volume = 0.01f, float pitch = 1.0f, int loop = 0);