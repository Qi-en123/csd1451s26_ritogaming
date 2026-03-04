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
};

extern UIManager ui_manager;

void load_ui_textures(UIManager* ui_manager);
void render_enemy_hp(UIManager* ui_manager, EnemyManager* enemy_manager);

void render_player_hud(UIManager* ui_manager, Player* player);

void free_ui_textures(UIManager* ui_manager);