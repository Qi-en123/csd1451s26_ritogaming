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

#define NUM_TILE_TYPES 7
#define TILE_ROWS 36
#define TILE_COLS 64

extern AEGfxTexture* TileTextures[NUM_TILE_TYPES];
extern AEGfxVertexList* TileMesh;


int map[TILE_ROWS][TILE_COLS];



// ------------------------------------------------------------
// Draw the entire map perfectly centered
// ------------------------------------------------------------
void DrawMap(float tile_size)
{
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.f);
    AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
    AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);

    float startX = -1600.f / 2.f;
    float startY = 900.f / 2.f;

    for (int row = 0; row < TILE_ROWS; ++row)
    {
        float y = startY - row * tile_size;
        for (int col = 0; col < TILE_COLS; ++col)
        {
            float x = startX + col * tile_size;
            int tileType = map[row][col];
            //DrawTile(TileTextures[tileType], x, y, tile_size);
            renderSprite(rectMeshCenter, x, y, tile_size, tile_size, TileTextures[tileType]);
        }
    }
}

// ------------------------------------------------------------
// Initialize the dungeon map layout
// ------------------------------------------------------------
void InitializeDungeonMap()
{
    // ============================================================
    // Fill entire map with empty tiles first (background)
    // ============================================================
    for (int r = 0; r < TILE_ROWS; ++r)
        for (int c = 0; c < TILE_COLS; ++c)
            map[r][c] = 0; // empty tile

    // ============================================================
    // PLAYER ROOM 1 (Top Left)
    // ============================================================

    // Floor area
    for (int r = 4; r < 8; ++r)
        for (int c = 4; c < 8; ++c)
            map[r][c] = 2; // floor tile

    // Top horizontal wall
    for (int c = 3; c <= 8; ++c)
        map[3][c] = 1;

    // Bottom horizontal wall
    for (int c = 3; c <= 8; ++c)
        map[8][c] = 1;
        map[8][5] = map[8][6] = 2; // opening to bottom left room
    // Left vertical wall
    for (int r = 3; r <= 8; ++r)
        map[r][3] = 1;

    // Right vertical wall
    for (int r = 3; r <= 8; ++r)
        map[r][8] = map[r][8] = 1;
        map[5][8] = map[6][8] = 2; //opening to top right room 

// ============================================================
// PLAYER ROOM 2 (Bottom Left) 
// ============================================================

// Floor area
for (int r = 18; r < 24; ++r)   
    for (int c = 4; c < 8; ++c)
        map[r][c] = 2; // floor tile

// Top horizontal wall
for (int c = 3; c <= 8; ++c)
    map[19][c] = 1;  

// Bottom horizontal wall
for (int c = 3; c <= 8; ++c)
    map[24][c] = 1;   

// Left vertical wall
for (int r = 19; r <= 24; ++r)
    map[r][3] = 1;   

// Right vertical wall
for (int r = 19; r <= 24; ++r)
    map[r][8] = 1;   
map[21][8] = map[22][8] = 2; //opening to bottom right room 
// ============================================================
// VERTICAL CORRIDOR (connecting player rooms)
// ============================================================

// Corridor floor
for (int r = 9; r < 20; ++r)    
    for (int c = 5; c < 7; ++c)
        map[r][c] = 2;

// Left and right corridor walls
for (int r = 9; r < 20; ++r)    
    map[r][4] = map[r][7] = 1;




    // ============================================================
    // BOSS ROOM 1 (Top Right)
    // ============================================================

    // Floor area
    for (int r = 3; r < 9; ++r)
        for (int c = 36; c < 54; ++c)
            map[r][c] = 2;

    // Top and bottom horizontal walls
    for (int c = 35; c <= 54; ++c) {
        map[2][c] = 1;
        map[9][c] = 1;
    }

    // Left and right vertical walls
    for (int r = 2; r <= 9; ++r)
        map[r][35] = map[r][54] = 1;

    // ============================================================
    // Horizontal corridor to Boss Room 1
    // ============================================================

    // Corridor floor
    for (int c = 9; c < 36; ++c)
        for (int r = 5; r < 7; ++r)
            map[r][c] = 2;

    // Top and bottom corridor walls
    for (int c = 9; c < 36; ++c) {
        map[4][c] = 1; // top wall
        map[7][c] = 1; // bottom wall
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
        for (int r = 21; r < 23; ++r)
            map[r][c] = 2;

    // Top and bottom corridor walls
    for (int c = 9; c < 36; ++c) {
        map[20][c] = 1; // top wall
        map[23][c] = 1; // bottom wall
    }
}

void ShiftMapRight()
{
    for (int r = 0; r < TILE_ROWS; ++r)
    {
        for (int c = TILE_COLS - 1; c > 0; --c)
        {
            map[r][c] = map[r][c - 2]; // shift everything to the right
        }
        map[r][0] = 0; // first column becomes empty after shift
    }
}