/*!************************************************************************
\file:          map.hpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu

\brief:         This header declares dungeon map rendering, coordinate
                conversion, and file I/O initialization functions.
**************************************************************************/

#pragma once
#include "AEEngine.h"
#include <string>

// ----------------------------------------------------------------------------
// Grid & World Constants
// ----------------------------------------------------------------------------
constexpr s32 NUM_TILE_TYPES = 9;   // Total types of tiles defined in TILE_ID
constexpr s32 TILE_ROWS = 36;        // Total number of rows in the map grid
constexpr s32 TILE_COLS = 64;        // Total number of columns in the map grid

// World space offsets for the top-left corner of the map grid
constexpr f32 world_left = -800.0f;
constexpr f32 world_top = 450.0f;
constexpr f32 tile_size = 64.0f;    // The width/height of a single square tile

// ----------------------------------------------------------------------------
// Enum for identifying tile types and their corresponding texture indices.
// ----------------------------------------------------------------------------
enum TILE_ID
{
    EMPTY, //0
    WALL,
    FLOOR,
    KNIGHT,
    WIZARD,
    TOMB,
    TREE,
    OPENED_DOOR,
    DOOR, //8
};

// ----------------------------------------------------------------------------
// Global Data Declarations
// ----------------------------------------------------------------------------
extern AEGfxTexture* TileTextures[NUM_TILE_TYPES]; // Global array of tile textures
extern AEGfxVertexList* TileMesh;                  // Shared mesh used to render tiles
extern s32 map[TILE_ROWS][TILE_COLS];              // The primary 2D game map array

// ----------------------------------------------------------------------------
//
// Function: draw_map
// Renders the entire tile map centered based on the tile_size provided.
//
// ---------------------------------------------------------------------------- 
void draw_map(f32 tile_size);

// ----------------------------------------------------------------------------
//
// Function: save_map_to_file
// Saves a dynamically allocated map to a new unique level file (e.g. lvl5.txt).
//
// ---------------------------------------------------------------------------- 
bool save_map_to_file(s32** map, std::string& out_filename);

// ----------------------------------------------------------------------------
//
// Function: load_map_from_file
// Reads map data from a file into the fixed-size game map array.
//
// ---------------------------------------------------------------------------- 
bool load_map_from_file(const char* filename, s32 map[][TILE_COLS]);

// ----------------------------------------------------------------------------
//
// Function: load_tile_textures
// Loads PNG textures for all tile types from a specific folder.
//
// ---------------------------------------------------------------------------- 
bool load_tile_textures(const char* folderPath);

// ----------------------------------------------------------------------------
//
// Function: free_tile_textures
// Unloads all loaded tile textures and cleans up memory.
//
// ---------------------------------------------------------------------------- 
void free_tile_textures();

// ----------------------------------------------------------------------------
//
// Function: get_door_world_pos
// Searches the map for a door and returns its world-space coordinates.
//
// ---------------------------------------------------------------------------- 
void get_door_world_pos(f32& out_x, f32& out_y);

// ----------------------------------------------------------------------------
//
// Function: load_map_to_editor
// Loads a map file into a double-pointer array for use in the level editor.
//
// ---------------------------------------------------------------------------- 
bool load_map_to_editor(const char* filename, s32** editor_map);

// ----------------------------------------------------------------------------
//
// Function: save_map_to_file_overwrite
// Overwrites an existing level file with new map data.
//
// ---------------------------------------------------------------------------- 
bool save_map_to_file_overwrite(s32** map, const std::string& filename);

// ----------------------------------------------------------------------------
//
// Function: draw_map_preview
// Renders a scaled preview of the map for UI or selection screens.
//
// ---------------------------------------------------------------------------- 
void draw_map_preview(s32 preview[][TILE_COLS], f32 center_x, f32 center_y, f32 preview_w, f32 preview_h);

// ----------------------------------------------------------------------------
//
// Function: preview_default_level
// Fills an array with the hardcoded default level layout for previewing.
//
// ---------------------------------------------------------------------------- 
void preview_default_level(s32 out_map[][TILE_COLS]);

// ----------------------------------------------------------------------------
//
// Function: convert_to_col / convert_to_row
// Calculates the grid indices based on a world position.
//
// ---------------------------------------------------------------------------- 
s32 convert_to_col(f32 x);
s32 convert_to_row(f32 y);

// ----------------------------------------------------------------------------
//
// Function: row_to_world / col_to_world
// Calculates the world coordinates of the center of a specific grid cell.
//
// ---------------------------------------------------------------------------- 
f32 row_to_world(s32 row);
f32 col_to_world(s32 col);

// ----------------------------------------------------------------------------
//
// Function: world_to_tile
// Converts a world position (x, y) into map grid indices (row, col).
//
// ---------------------------------------------------------------------------- 
bool world_to_tile(f32 worldX, f32 worldY, int& outRow, int& outCol);