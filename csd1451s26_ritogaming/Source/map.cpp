/*!************************************************************************
\file:          map.cpp
\author:        Qi En
\par DP email:  g.qien@digipen.edu
\brief:         This file handles tile-based map rendering and initialization.
                It creates the tile mesh, draws individual tiles and the 
                full map and defines the dungeon layout including rooms, 
                corridors, players,and boss areas.
**************************************************************************/
#include "AEEngine.h"
#include "renderer.hpp"
#include "map.hpp"
    
// Dummy map
s32 map[TILE_ROWS][TILE_COLS];


// ------------------------------------------------------------
// Draw the entire map perfectly centered
// ------------------------------------------------------------
void draw_map(f32 tile_size)
{
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.f);
    AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
    AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);

    f32 startX = -1600.f / 2.f;
    f32 startY = 900.f / 2.f;

    for (int row = 0; row < TILE_ROWS; ++row)
    {
        f32 y = startY - row * tile_size;
        for (int col = 0; col < TILE_COLS; ++col)
        {
            f32 x = startX + col * tile_size;
            int tileType = map[row][col];
            //DrawTile(TileTextures[tileType], x, y, tile_size);
            render_sprite(rect_mesh_center, x, y, tile_size, tile_size, TileTextures[tileType]);
        }
    }
}

// ------------------------------------------------------------
// Initialize the dungeon map layout
// ------------------------------------------------------------
void initialize_dungeon_map()
{
    // ============================================================
    // PLAYER ROOM 1 (Top Left) 
    // ============================================================

// Floor
    for (int r = 3; r <= 10; ++r)
        for (int c = 2; c <= 9; ++c)
            map[r][c] = 2;

    // Top and bottom walls
    for (int c = 1; c <= 10; ++c)
    {
        map[2][c] = 1;  // top wall
        map[11][c] = 1; // bottom wall
    }

    // Left wall
    for (int r = 3; r <= 11; ++r)
        map[r][1] = 1;

    // Right wall
    for (int r = 3; r <= 11; ++r)
        map[r][10] = 1;

    // Bottom door (inside floor)
    for (int c = 4; c <= 7; ++c)
        map[10][c] = 2;

    // Right door
    map[6][10] = 2;

    // ============================================================
    // PLAYER ROOM 2 (Bottom Left)
    // ============================================================

   // Floor area
    for (int r = 18; r < 27; ++r)      // was < 26
        for (int c = 2; c < 10; ++c)
            map[r][c] = 2; // floor tile

    // Top horizontal wall
    for (int c = 1; c <= 10; ++c)
        map[18][c] = 1;

    // Bottom horizontal wall
    for (int c = 1; c <= 10; ++c)
        map[26][c] = 1;                // was 25

    // Top opening (4 tiles wide, centered)
    for (int c = 4; c <= 7; ++c)
        map[18][c] = 2;

    // Left vertical wall
    for (int r = 18; r <= 26; ++r)     // was <= 25
        map[r][1] = 1;

    // Right vertical wall
    for (int r = 18; r <= 26; ++r)     // was <= 25
        map[r][10] = 1;

    map[21][10] = map[22][10] = 2;     // unchanged

    // ============================================================
    // VERTICAL CORRIDOR (connecting player rooms)
    // ============================================================

    // Corridor floor (expanded width)
    for (int r = 11; r < 18; ++r)
        for (int c = 4; c < 8; ++c)
            map[r][c] = 2;

    // Left and right corridor walls
    for (int r = 11; r < 18; ++r)
        map[r][3] = map[r][8] = 1;

    // ============================================================
    // BOSS ROOM 1 (Top Right) 
    // ============================================================

    // Floor area
    for (int r = 3; r < 12; ++r)
        for (int c = 36; c < 54; ++c)
            map[r][c] = 2;

    // Top and bottom horizontal walls
    for (int c = 35; c <= 54; ++c) {
        map[2][c] = 1;
        map[12][c] = 1; // bottom wall
    }

    // Left and right vertical walls
    for (int r = 2; r <= 12; ++r)
        map[r][35] = map[r][54] = 1;

    // ============================================================
    // Horizontal corridor to Boss Room 1 
    // ============================================================

    // Corridor floor
    for (int c = 9; c < 36; ++c)
        for (int r = 5; r < 9; ++r) // increased vertical height from 5-7 to 5-9
            map[r][c] = 2;

    // Top and bottom corridor walls
    for (int c = 10; c < 36; ++c) {
        map[4][c] = 1;  // top wall
        map[9][c] = 1;  // bottom wall (moved down)
    }

    // ============================================================
    // BOSS ROOM 2 (Bottom Right)
    // ============================================================

    // Floor area
    for (int r = 17; r < 28; ++r)
        for (int c = 36; c < 54; ++c)
            map[r][c] = 2;

    // Top and bottom horizontal walls
    for (int c = 35; c <= 54; ++c) {
        map[16][c] = 1;
        map[28][c] = 1;
    }

    // Left and right vertical walls
    for (int r = 16; r <= 28; ++r)
        map[r][35] = map[r][54] = 1;

    // ============================================================
    // Horizontal corridor to Boss Room 2 
    // ============================================================

    // Corridor floor
    for (int c = 9; c < 36; ++c)
        for (int r = 21; r < 24; ++r) // increased vertical height from 21-23 to 21-24
            map[r][c] = 2;

    // Top and bottom corridor walls
    for (int c = 10; c < 36; ++c) {
        map[20][c] = 1; // top wall
        map[24][c] = 1; // bottom wall
    }
}


