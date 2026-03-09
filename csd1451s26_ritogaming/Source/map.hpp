/*!************************************************************************
\file:          map.hpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         This header declares dungeon map rendering and initialization
                functions.
**************************************************************************/
#pragma once
#include "AEEngine.h"
#define NUM_TILE_TYPES 10
#define TILE_ROWS 36
#define TILE_COLS 64

extern AEGfxTexture* TileTextures[NUM_TILE_TYPES];
extern AEGfxVertexList* TileMesh;
extern s32 map[TILE_ROWS][TILE_COLS];
extern s32 boss_map[TILE_ROWS][TILE_COLS];
//extern f32 tile_size;
constexpr f32 tile_size = 64.0f;
void initialize_boss_map();
void initialize_dungeon_map();

void draw_boss_map(f32 tile_size);
void draw_map(f32 tile_size);
bool save_map_to_file(s32** map);
bool load_map_from_file(const char* filename, s32** map);
bool load_tile_textures(const char* folderPath);
void free_tile_textures();



enum TILE_ID
{
    EMPTY = 0,   
    WALL, 
    FLOOR,        
    OPENED_DOOR = 7,
    DOOR = 8,
    SPECIAL_DOOR=9
};


