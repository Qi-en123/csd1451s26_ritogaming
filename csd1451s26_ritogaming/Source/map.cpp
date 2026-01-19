/*!************************************************************************
\file:          map.cpp
\author:        Qien
\par DP email:  g.qien@digipen.edu
\brief:         This file handles tile-based map rendering and initialization.
                It creates the tile mesh, draws individual tiles and the 
                full map and defines the dungeon layout including rooms, 
                corridors, players,and boss areas.
**************************************************************************/
#include "AEEngine.h"
#include "renderer.hpp"

#define NUM_TILE_TYPES 7
#define TILE_ROWS 90
#define TILE_COLS 160

extern AEGfxTexture* TileTextures[NUM_TILE_TYPES];
extern AEGfxVertexList* TileMesh;


extern int map[TILE_ROWS][TILE_COLS];

// ------------------------------------------------------------
// Create a square tile mesh
// ------------------------------------------------------------
void CreateTileMesh(AEGfxVertexList** mesh)
{
    AEGfxMeshStart();

    // top-left = (0,0), bottom-right = (1,1)
    AEGfxTriAdd(
        0.f, 0.f, 0xFFFFFFFF, 0.0f, 0.0f,   // top-left
        1.f, 0.f, 0xFFFFFFFF, 1.0f, 0.0f,   // top-right
        0.f, 1.f, 0xFFFFFFFF, 0.0f, 1.0f);  // bottom-left

    AEGfxTriAdd(
        1.f, 0.f, 0xFFFFFFFF, 1.0f, 0.0f,   // top-right
        1.f, 1.f, 0xFFFFFFFF, 1.0f, 1.0f,   // bottom-right
        0.f, 1.f, 0xFFFFFFFF, 0.0f, 1.0f);  // bottom-left

    *mesh = AEGfxMeshEnd();
}

// ------------------------------------------------------------
// Draw a single tile using top-left coordinates
// ------------------------------------------------------------
void DrawTile(AEGfxTexture* tex, float topLeftX, float topLeftY, float tile_size)
{
    if (!tex || !TileMesh) return;

    AEMtx33 Scale, Rotate, Translate, Transform;

    AEMtx33Scale(&Scale, tile_size, -tile_size);
    AEMtx33Rot(&Rotate, 0.0f);
    AEMtx33Trans(&Translate, topLeftX, topLeftY);

    AEMtx33Concat(&Transform, &Rotate, &Scale);
    AEMtx33Concat(&Transform, &Translate, &Transform);

    AEGfxSetTransform(Transform.m);

    AEGfxTextureSet(tex, 0, 0);
    AEGfxMeshDraw(TileMesh, AE_GFX_MDM_TRIANGLES);
}

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
    float startY = 900.f / 2.f - tile_size;

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
            map[r][c] = 0; // background/empty tile

    // ============================================================
    // PLAYER ROOM 1 (Top Left) - Expanded & Centered
    // ============================================================
    for (int r = 20; r < 32; ++r)      // floor area rows
        for (int c = 20; c < 32; ++c)  // floor area columns
            map[r][c] = 2;             // floor tile

    for (int c = 19; c < 32; ++c) {
        map[19][c] = 1;  // top horizontal line of walls
        map[32][c] = 2;  // bottom horizontal line of floor tiles
    }
    for (int r = 19; r < 33; ++r)
        map[r][19] = map[r][32] = 1;  // left and right vertical lines of walls

    //map[26][26] = 3; // player 1 spawn position


    // ============================================================
    // PLAYER ROOM 2 (Bottom Left) - Expanded & Centered
    // ============================================================
    for (int r = 58; r < 70; ++r)      // floor area rows
        for (int c = 20; c < 32; ++c)  // floor area columns
            map[r][c] = 2;             // floor tile

    for (int c = 19; c < 33; ++c)
        map[57][c] = map[70][c] = 1; // top and bottom horizontal lines of walls
    for (int r = 57; r < 71; ++r)
        map[r][19] = map[r][32] = 1; // left and right vertical lines of walls

    //map[64][26] = 4; // player 2 spawn position

    // ============================================================
    // VERTICAL CORRIDOR - Centered & Wider
    // ============================================================
    for (int r = 33; r < 58; ++r)      // floor area rows
        for (int c = 20; c < 32; ++c)  // floor area columns
            map[r][c] = 2;             // corridor floor tile

    for (int r = 33; r < 58; ++r)
        map[r][19] = map[r][32] = 1; // left and right vertical lines of corridor walls
        map[23][32] = map[24][32] = map[25][32] = map[26][32] = 2; // opening of top pathway
        map[64][32] = map[65][32] = map[66][32] = map[67][32] = 2; // opening of btm pathway
    // ============================================================
    // BOSS ROOM 1 (Top Right) - Centered horizontally
    // ============================================================
    for (int r = 15; r < 35; ++r)       // floor area rows
        for (int c = 60; c < 90; ++c) // floor area columns
            map[r][c] = 2;              // floor tile

    for (int c = 59; c < 91; ++c)
        map[14][c] = map[35][c] = 1; // top and bottom horizontal lines of walls
    for (int r = 14; r < 36; ++r)
        map[r][59] = map[r][90] = 1; // left and right vertical lines of walls

    //map[25][80] = 5; // boss 1 position
    // ============================================================
    // Horizontal corridor to Boss Room 1 - Shortened & synced
    // ============================================================
    for (int c = 33; c < 60; ++c)
        for (int r = 23; r < 27; ++r)
            map[r][c] = 2; // floor tiles for horizontal corridor

    for (int c = 33; c < 60; ++c)
        map[22][c] = map[27][c] = 1; // top and bottom horizontal lines of corridor walls


    // ============================================================
    // BOSS ROOM 2 (Bottom Right � Bigger) - Centered horizontally
    // ============================================================
    for (int r = 55; r < 85; ++r)       // floor area rows
        for (int c = 60; c < 100; ++c) // floor area columns
            map[r][c] = 2;              // floor tile

    for (int c = 59; c < 100; ++c)
        map[54][c] = map[85][c] = 1; // top and bottom horizontal lines of walls
    for (int r = 54; r < 86; ++r)
        map[r][59] = map[r][100] = 1; // left and right vertical lines of walls

    //map[70][80] = 6; // boss 2 position
    // ============================================================
    // Horizontal corridor to Boss Room 2 - Shortened & synced
    // ============================================================
    for (int c = 33; c < 60; ++c)
        for (int r = 64; r < 68; ++r)
            map[r][c] = 2; // floor tiles for horizontal corridor

    for (int c = 33; c < 60; ++c)
        map[63][c] = map[68][c] = 1; // top and bottom horizontal lines of corridor walls
}