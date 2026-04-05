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

// ------------------------------------------------------------------------
// Scaling factor for enemy HP bar rendering
// ------------------------------------------------------------------------
constexpr float ENTITY_HP_SCALE = 0.5f;

// ------------------------------------------------------------------------
// Size of HUD heart icons
// ------------------------------------------------------------------------
constexpr float HUD_HEART_SIZE = 64.f;

// ------------------------------------------------------------------------
//
// Stores all UI-related textures and font data
//
// ------------------------------------------------------------------------
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

// ------------------------------------------------------------------------
// Global UI manager instance
// ------------------------------------------------------------------------
extern UIManager ui_manager;

// ------------------------------------------------------------------------
//
// Loads all UI textures into the UIManager
// ui_manager - pointer to UIManager to initialize
//
// ------------------------------------------------------------------------
void load_ui_textures(UIManager* ui_manager);

// ------------------------------------------------------------------------
//
// Renders enemy health bars above enemies
// ui_manager    - UI resources
// enemy_manager - contains enemy data
//
// ------------------------------------------------------------------------
void render_enemy_hp(UIManager* ui_manager, EnemyManager* enemy_manager);

// ------------------------------------------------------------------------
//
// Renders the player's HUD (health, abilities, etc.)
// ui_manager - UI resources
// player     - player data
//
// ------------------------------------------------------------------------
void render_player_hud(UIManager* ui_manager, Player* player);

// ------------------------------------------------------------------------
//
// Renders character-specific UI elements
// ui_manager - UI resources
// player     - player data
//
// ------------------------------------------------------------------------
void render_character_ui(UIManager* ui_manager, Player* player);

// ------------------------------------------------------------------------
//
// Renders the cursor on screen
// ui_manager    - UI resources
// player        - player data (for cursor type)
// mouse_x       - mouse x position
// mouse_y       - mouse y position
// cursor_active - whether active cursor state is used
//
// ------------------------------------------------------------------------
void render_cursor(UIManager* ui_manager, Player* player, s32 mouse_x, s32 mouse_y, bool cursor_active);

// ------------------------------------------------------------------------
//
// Frees all UI textures from memory
// ui_manager - pointer to UIManager to clean up
//
// ------------------------------------------------------------------------
void free_ui_textures(UIManager* ui_manager);