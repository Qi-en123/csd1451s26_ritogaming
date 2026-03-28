    #pragma once

    #ifndef MINIMAP_H
    #define MINIMAP_H

    #include "renderer.hpp"
    #include "map.hpp"
    #include "AEEngine.h"

    #define ZOOM_LEVEL 2.0f

    extern f32 totalWorldW;
    extern f32 totalWorldH;

    /**
    * @brief Sets the internal world bounds and the desired size of the UI.
    * @param worldW Total width of your game map (e.g., TILE_COLS * tile_size).
    * @param worldH Total height of your game map (e.g., TILE_ROWS * tile_size).
    * @param mapSize The pixel size of the square minimap (e.g., 200.0f).
    */
    void minimap_init(f32 worldW, f32 worldH, f32 mapSize);

    /**
    * @brief Draws the entire minimap HUD, including the background and player.
    * @param playerX The current world X-coordinate of the player.
    * @param playerY The current world Y-coordinate of the player.
    */
    void minimap_draw(Player* pPlayer, int dungeonMap[TILE_ROWS][TILE_COLS], AEGfxTexture* tiles[]);

    void minimap_draw_overlay(f32 centerX, f32 centerY);

    void draw_sprite_on_map(f32 worldX, f32 worldY, f32 spriteScale, AEGfxTexture* pTex);

    // New function to draw the layout
    void minimap_draw_walls(int dungeonMap[TILE_ROWS][TILE_COLS]);

    void minimap_unload();
