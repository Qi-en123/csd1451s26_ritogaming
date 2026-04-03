#pragma once

#include "renderer.hpp"
#include "map.hpp"
#include "AEEngine.h"

constexpr f32 minimap_zoom = 2.0f; //change name, to like minimap_zoom or smth and change to constexpr

extern f32 totalWorldW;
extern f32 totalWorldH;

//fak u whr the GIT format
/**
* @brief Sets the internal world bounds and the desired size of the UI.
* @param worldW Total width of your game map (e.g., TILE_COLS * tile_size).
* @param worldH Total height of your game map (e.g., TILE_ROWS * tile_size).
* @param mapSize The pixel size of the square minimap (e.g., 200.0f).
*/
void minimap_init(f32 worldW, f32 worldH);

/**
* @brief Draws the entire minimap HUD, including the background and player.
* @param playerX The current world X-coordinate of the player.
* @param playerY The current world Y-coordinate of the player.
*/
void minimap_draw(Player* pPlayer, int dungeonMap[TILE_ROWS][TILE_COLS], AEGfxTexture* tiles[]);

void minimap_draw_overlay(f32 centerX, f32 centerY);

void draw_sprite_on_map(f32 worldX, f32 worldY, f32 spriteScale, AEGfxTexture* pTex);

void minimap_unload();