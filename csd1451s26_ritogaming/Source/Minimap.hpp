#pragma once

#ifndef MINIMAP_H
#define MINIMAP_H

#include "renderer.hpp" // Keep this to access your 'color' struct and meshes
#include "map.hpp"

/**
 * @brief Sets the internal world bounds and the desired size of the UI.
 * @param worldW Total width of your game map (e.g., TILE_COLS * tile_size).
 * @param worldH Total height of your game map (e.g., TILE_ROWS * tile_size).
 * @param mapSize The pixel size of the square minimap (e.g., 200.0f).
 */
void Minimap_Init(float worldW, float worldH, float mapSize);

/**
 * @brief Draws the entire minimap HUD, including the background and player.
 * @param playerX The current world X-coordinate of the player.
 * @param playerY The current world Y-coordinate of the player.
 */
void Minimap_Draw(player* pPlayer, int dungeonMap[TILE_ROWS][TILE_COLS], AEGfxTexture* tiles[]);

void DrawSpriteOnMap(float worldX, float worldY, float spriteScale, AEGfxTexture* pTex);

// New function to draw the layout
void Minimap_DrawWalls(int dungeonMap[TILE_ROWS][TILE_COLS]);

#endif