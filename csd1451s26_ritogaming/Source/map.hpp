/*!************************************************************************
\file:          map.hpp
\author:        Qi En
\par DP email:  g.qien@digipen.edu
\brief:         This header declares dungeon map rendering and initialization
                functions.
**************************************************************************/
#pragma once
#include "AEEngine.h"
#define NUM_TILE_TYPES 7
#define TILE_ROWS 36
#define TILE_COLS 64

extern AEGfxTexture* TileTextures[NUM_TILE_TYPES];
extern AEGfxVertexList* TileMesh;

extern f32 tile_size;

void InitializeDungeonMap();
void DrawMap(f32 tile_size);

extern int map[TILE_ROWS][TILE_COLS];

void ShiftMapRight(int shift_right_by);


