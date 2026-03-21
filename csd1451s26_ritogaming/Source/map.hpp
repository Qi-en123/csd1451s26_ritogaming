/*!************************************************************************
\file:          map.hpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         This header declares dungeon map rendering and initialization
                functions.
**************************************************************************/
#pragma once
#include "AEEngine.h"
#include <string>
#define NUM_TILE_TYPES 9
#define TILE_ROWS 36
#define TILE_COLS 64

extern AEGfxTexture* TileTextures[NUM_TILE_TYPES];
extern AEGfxVertexList* TileMesh;
extern s32 map[TILE_ROWS][TILE_COLS];
extern s32 boss_map[TILE_ROWS][TILE_COLS];
constexpr f32 tile_size = 64.0f;
void initialize_boss_map();

void draw_boss_map(f32 tile_size);
void draw_map(f32 tile_size);
bool save_map_to_file(s32** map, std::string& out_filename);
bool load_map_from_file(const char* filename, s32 map[][TILE_COLS]);
bool load_tile_textures(const char* folderPath);
void free_tile_textures();
void get_door_world_pos(f32& out_x, f32& out_y);
bool load_map_to_editor(const char* filename, s32** editor_map);
bool save_map_to_file_overwrite(s32** map, const std::string& filename);
void draw_map_preview(s32 preview[][TILE_COLS], f32 center_x, f32 center_y, f32 preview_w, f32 preview_h);
void preview_default_level(s32 out_map[][TILE_COLS]);

enum TILE_ID
{
    EMPTY = 0,
    WALL,
    FLOOR,
    KNIGHT = 3,
    WIZARD = 4,
    OPENED_DOOR = 7,
    DOOR = 8,
};


