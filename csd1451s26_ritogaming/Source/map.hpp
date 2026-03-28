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

constexpr s32 NUM_TILE_TYPES = 9;
constexpr s32 TILE_ROWS = 36;
constexpr s32 TILE_COLS = 64;

constexpr f32 world_left = -800.0f;
constexpr f32 world_top = 450.0f;

enum TILE_ID
{
    EMPTY,
    WALL,
    FLOOR,
    KNIGHT,
    WIZARD,
    TOMB,
    TREE,
    OPENED_DOOR,
    DOOR,
};

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
s32 convert_to_col(f32 x);
s32 convert_to_row(f32 y);
f32 row_to_world(s32 row);
f32 col_to_world(s32 col);

// Convert world position to tile indices
bool world_to_tile(f32 worldX, f32 worldY, int& outRow, int& outCol);
