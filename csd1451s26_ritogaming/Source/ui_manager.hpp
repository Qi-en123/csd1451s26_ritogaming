/*!************************************************************************
\file:          ui_manager.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\
\brief:         Declares UIManager structure and UI rendering functions
\               for enemy health indicators and player heads-up display.
\               Manages loading and unloading of UI texture assets.
**************************************************************************/

#pragma once
#include "AEEngine.h"

#include "Minimap.hpp"
#include "enemy_manager.hpp"

constexpr float ENTITY_HP_SCALE = 0.5f;
constexpr float HUD_HEART_SIZE = 64.f;

struct UIManager
{
	 AEGfxTexture* hp_empty_texture;
	 AEGfxTexture* hp_half_texture;
	 AEGfxTexture* hp_full_texture;
	 AEGfxTexture* primary_bg_texture;
	 AEGfxTexture* primary_counter_texture;
	 AEGfxTexture* cursor_knight;        
	 AEGfxTexture* cursor_wizard;        
	 AEGfxTexture* cursor_wizard_active; 
	 s8 font;
};

extern UIManager ui_manager;

void load_ui_textures(UIManager* ui_manager);
void render_enemy_hp(UIManager* ui_manager, EnemyManager* enemy_manager);

void render_player_hud(UIManager* ui_manager, Player* player);

void render_character_ui(UIManager* ui_manager, Player* player);

void render_cursor(UIManager* ui_manager, Player* player, s32 mouse_x, s32 mouse_y, bool cursor_active);

void free_ui_textures(UIManager* ui_manager);