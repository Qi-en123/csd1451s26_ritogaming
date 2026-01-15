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

#define NUM_TILE_TYPES 131
#define TILE_ROWS 90
#define TILE_COLS 160

extern AEGfxTexture* TileTextures[NUM_TILE_TYPES];
extern AEGfxVertexList* TileMesh;

// Dummy map
extern int map[TILE_ROWS][TILE_COLS];



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
    // Clear map
    for (int r = 0; r < TILE_ROWS; ++r)
        for (int c = 0; c < TILE_COLS; ++c)
            map[r][c] = 0; //clear room

    // ============================================================
    // PLAYER ROOM 1 (Top Left)
    // ============================================================
    for (int r = 10; r < 18; ++r)
        for (int c = 10; c < 18; ++c)
            map[r][c] = 2; 

    for (int c = 9; c < 19; ++c)
        map[9][c] = map[18][c] = 1; // wall tile
    for (int r = 9; r < 19; ++r)
        map[r][9] = map[r][18] = 1; // wall tile

    map[14][14] = 3; //player 1


    // ============================================================
    // PLAYER ROOM 2 (Bottom Left)
    // ============================================================
    for (int r = 60; r < 68; ++r)
        for (int c = 10; c < 18; ++c)
            map[r][c] = 2; // floor tile

    for (int c = 9; c < 19; ++c)
        map[59][c] = map[68][c] = 1; // wall tile
    for (int r = 59; r < 69; ++r)
        map[r][9] = map[r][18] = 1; // wall tile

    map[64][14] = 4; //player 2


    // ============================================================
    // VERTICAL CORRIDOR
    // ============================================================
    for (int r = 18; r < 68; ++r)
        for (int c = 9; c < 19; ++c)
            map[r][c] = 2; // floor tile

    for (int r = 18; r < 68; ++r) {
        map[r][9] = 1;  // wall tile
        map[r][18] = 1; // wall tile
    }


    // ============================================================
    // BOSS ROOM 1 (Top Right)
    // ============================================================
    for (int r = 6; r < 26; ++r)
        for (int c = 60; c < 90; ++c)
            map[r][c] = 2; // floor tile

    for (int c = 59; c < 91; ++c)
        map[5][c] = map[26][c] = 1; // // wall tile
    for (int r = 5; r < 27; ++r)
        map[r][59] = map[r][90] = 1; // // wall tile

    map[16][75] = 5; // boss 1


    // Corridor to Boss Room 1
    for (int c = 18; c < 60; ++c)
        for (int r = 13; r < 15; ++r)
            map[r][c] = 2; // floor tile

    for (int c = 18; c < 60; ++c) {
        map[12][c] = 1; // wall tile
        map[15][c] = 1; // wall tile
    }


    // ============================================================
    // BOSS ROOM 2 (Bottom Right – Bigger)
    // ============================================================
    for (int r = 48; r < 82; ++r)
        for (int c = 60; c < 100; ++c)
            map[r][c] = 2; // floor tile

    for (int c = 59; c < 101; ++c)
        map[47][c] = map[82][c] = 1; // wall tile
    for (int r = 47; r < 83; ++r)
        map[r][59] = map[r][100] = 1; // wall tile

    map[65][80] = 6; // boss 2


    // Corridor to Boss Room 2
    for (int c = 18; c < 60; ++c)
        for (int r = 63; r < 65; ++r)
            map[r][c] = 2; // floor tile

    for (int c = 18; c < 60; ++c) {
        map[62][c] = 1; // wall tile
        map[65][c] = 1; // wall tile
    }
}