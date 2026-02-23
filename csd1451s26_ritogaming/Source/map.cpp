/*!************************************************************************
\file:          map.cpp
\author:        Goh Qi En
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
s32 boss_map[TILE_ROWS][TILE_COLS];
f32 tile_size{};
AEGfxTexture* TileTextures[NUM_TILE_TYPES];
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
            int tileType = map[row][col];

            // Skip empty tiles
            if (tileType == 0)
                continue;

            f32 x = startX + col * tile_size;
            render_sprite(rect_mesh_center, x, y, tile_size, tile_size, TileTextures[tileType]);
        }
    }
}

void draw_boss_map(f32 tile_size)
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
            int tileType = boss_map[row][col];

            // Skip drawing if tileType is 0
            if (tileType == 0)
                continue;

            f32 x = startX + col * tile_size;
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
            map[r][c] = FLOOR;

    // Top and bottom walls
    for (int c = 1; c <= 10; ++c)
    {
        map[2][c] = WALL;  // top wall
        map[11][c] = WALL; // bottom wall
    }

    // walls inside player room 1
    for (int r = 2; r <= 6; ++r)
    {
        map[r][3] = WALL;  // top wall
        map[r][8] = WALL; // bottom wall
    }



    // Left wall
    for (int r = 3; r <= 11; ++r)
        map[r][1] = WALL;

    // Right wall
    for (int r = 3; r <= 11; ++r)
        map[r][10] = WALL;

    // Bottom door (inside floor)
    for (int c = 4; c <= 7; ++c)
        map[10][c] = FLOOR;

    // Right door
    map[6][10] = FLOOR;

    // ============================================================
    // PLAYER ROOM 2 (Bottom Left)
    // ============================================================

   // Floor area
    for (int r = 18; r < 27; ++r)      
        for (int c = 2; c < 10; ++c)
            map[r][c] = FLOOR; // floor tile

    // Top horizontal wall
    for (int c = 1; c <= 10; ++c)
        map[18][c] = WALL;

    // Bottom horizontal wall
    for (int c = 1; c <= 10; ++c)
        map[26][c] = WALL;                

    // Top opening (4 tiles wide, centered)
    for (int c = 4; c <= 7; ++c)
        map[18][c] = SPECIAL_DOOR;

    // Left vertical wall
    for (int r = 18; r <= 26; ++r)     
        map[r][1] = WALL;

    // Right vertical wall
    for (int r = 18; r <= 26; ++r)     
        map[r][10] = WALL;

    map[21][10] = map[22][10] = FLOOR;    

    // ============================================================
    // VERTICAL CORRIDOR (connecting player rooms)
    // ============================================================

    // Corridor floor (expanded width)
    for (int r = 11; r < 18; ++r)
        for (int c = 4; c < 8; ++c)
            map[r][c] = FLOOR;

    // Left and right corridor walls
    for (int r = 11; r < 18; ++r)
        map[r][3] = map[r][8] = WALL;


    // ============================================================
    // BOSS ROOM 1 (Top Right) 
    // ============================================================

    // Floor area
    for (int r = 3; r < 12; ++r)
        for (int c = 25; c < 31; ++c)
            map[r][c] = FLOOR;

    // Top and bottom horizontal walls
    for (int c = 24; c <= 31; ++c) {
        map[2][c] = WALL;
        map[12][c] = WALL; // bottom wall
    }

    // Left and right vertical walls
    for (int r = 2; r <= 12; ++r)
        map[r][24] = map[r][31] = WALL;


    map[7][28] = OPENED_DOOR;

    // ============================================================
    // Horizontal corridor to Boss Room 1 
    // ============================================================

    // Corridor floor
    for (int c = 9; c < 25; ++c)
            map[7][c] = FLOOR;

    // Top and bottom corridor walls
    for (int c = 10; c < 25; ++c) {
        map[6][c] = WALL;  // top wall
        map[8][c] = WALL;  // bottom wall (moved down)
    }

   
    map[7][10] = SPECIAL_DOOR;

    // ============================================================
    // BOSS ROOM 2 (Bottom Right)
    // ============================================================

    // Floor area
    for (int r = 18; r < 24; ++r)
        for (int c = 25; c < 32; ++c)
            map[r][c] = FLOOR;

    // Top and bottom horizontal walls
    for (int c = 24; c <= 32; ++c) {
        map[17][c] = WALL;
        map[24][c] = WALL;
    }

    // Left and right vertical walls
    for (int r = 17; r <= 24; ++r)
        map[r][24] = map[r][32] = WALL;

    // ============================================================
    // Horizontal corridor to Boss Room 2 
    // ============================================================

    // Corridor floor
    for (int c = 9; c < 25; ++c)
            map[21][c] = FLOOR;

    // Top and bottom corridor walls
    for (int c = 10; c < 25; ++c) {
        map[20][c] = WALL; // top wall
        map[22][c] = WALL; // bottom wall
    }



}


// ------------------------------------------------------------
// Initialize a simplified boss map layout (one big room)
// ------------------------------------------------------------
void initialize_boss_map()
{
    // Clear the whole boss map first
    for (int r = 0; r < TILE_ROWS; ++r)
        for (int c = 0; c < TILE_COLS; ++c)
            boss_map[r][c] = WALL; // start with walls everywhere

    // ============================================================
    // BIG BOSS ROOM
    // ============================================================
     // Clear the whole boss map first
    for (int r = 0; r < TILE_ROWS; ++r)
        for (int c = 0; c < TILE_COLS; ++c)
            boss_map[r][c] = EMPTY; // start with walls everywhere
// Floor
    for (int r = 3; r <= 10; ++r)
        for (int c = 2; c <= 9; ++c)
            boss_map[r][c + 5] = FLOOR;

    // Top and bottom walls
    for (int c = 1; c <= 10; ++c)
    {
        boss_map[2][c + 5] = WALL;  // top wall
        boss_map[11][c + 5] = WALL;  // bottom wall
    }

    // Left wall
    for (int r = 3; r <= 11; ++r)
        boss_map[r][6] = WALL;

    // Right wall
    for (int r = 3; r <= 11; ++r)
        boss_map[r][15] = WALL;

}