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
extern s32 map[TILE_ROWS][TILE_COLS];

extern f32 tile_size;

void initialize_dungeon_map();
void draw_map(f32 tile_size);

enum TILE_ID
{
    EMPTY = 0,   
    WALL, 
    FLOOR,        
    DOOR,
    SPECIAL_DOOR,
    OPENED_DOOR,
};


