/*!************************************************************************
\file:          minimap.hpp
\author:        Ahmad Qusyairi Bin Mohammad Jinna
\par DP email:  ahmadqusyairi.b@digipen.edu

\brief:         Declarations for the HUD minimap system. Handles coordinate
                transformation from world space to UI space and renders a
                scaled representation of the dungeon and entities.
**************************************************************************/

#pragma once

#include "renderer.hpp"
#include "map.hpp"
#include "AEEngine.h"

// Minimap configuration constants
constexpr f32 minimap_zoom = 2.0f;

extern f32 totalWorldW;
extern f32 totalWorldH;

// ----------------------------------------------------------------------------
//
// Function: minimap_init
// Sets the internal world bounds and the desired size of the UI.
//
// ---------------------------------------------------------------------------- 
void minimap_init(f32 worldW, f32 worldH);

// ----------------------------------------------------------------------------
//
// Function: minimap_draw
// Draws the entire minimap HUD, including the background and player.
//
// ---------------------------------------------------------------------------- 
void minimap_draw(Player* pPlayer, int dungeonMap[TILE_ROWS][TILE_COLS], AEGfxTexture* tiles[]);

// ----------------------------------------------------------------------------
//
// Function: minimap_draw_overlay
// Renders the static UI frame or border overlay for the minimap.
//
// ---------------------------------------------------------------------------- 
void minimap_draw_overlay(f32 centerX, f32 centerY);

// ----------------------------------------------------------------------------
//
// Function: draw_sprite_on_map
// Translates a world-space coordinate into a minimap-space coordinate 
// and renders a sprite at that location.
//
// ---------------------------------------------------------------------------- 
void draw_sprite_on_map(f32 worldX, f32 worldY, f32 spriteScale, AEGfxTexture* pTex);

// ----------------------------------------------------------------------------
//
// Function: minimap_unload
// Cleans up minimap resources and textures before state transitions.
//
// ---------------------------------------------------------------------------- 
void minimap_unload();