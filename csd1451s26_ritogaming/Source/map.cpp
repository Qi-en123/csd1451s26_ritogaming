/*!************************************************************************
\file:          map.cpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu

\brief:         This file handles tile-based map rendering and initialization.
                It creates the tile mesh, draws individual tiles and the
                full map and defines the dungeon layout including rooms,
                corridors, players, and boss areas.
**************************************************************************/

#include "AEEngine.h"
#include "renderer.hpp"
#include "map.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>   
#include <windows.h>

// ----------------------------------------------------------------------------
// Global Map Data
// ----------------------------------------------------------------------------
s32 map[TILE_ROWS][TILE_COLS];
AEGfxTexture* TileTextures[NUM_TILE_TYPES];

// ----------------------------------------------------------------------------
//
// Function: load_tile_textures
// Iterates through the Assets folder to load textures for each tile type index.
//
// ---------------------------------------------------------------------------- 
bool load_tile_textures(const char* folderPath)
{
    for (int i = 0; i < NUM_TILE_TYPES; ++i)
    {
        // Build file path: folderPath + "tile_i.png" (e.g., tile_0001.png)
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/tile_%04d.png", folderPath, i);

        // Load the texture if not already in memory
        if (TileTextures[i]) continue;
        TileTextures[i] = AEGfxTextureLoad(filename);
        if (!TileTextures[i])
        {
            std::cerr << "Failed to load tile texture: " << filename << std::endl;
            return false;
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
//
// Function: save_map_to_file_overwrite
// Saves the current state of a dynamic map to an existing file, overwriting data.
//
// ---------------------------------------------------------------------------- 
bool save_map_to_file_overwrite(s32** map, const std::string& filename)
{
    std::ofstream file{ filename };

    if (!file.is_open())
    {
        std::cerr << "Failed to open map file for overwrite: " << filename << "\n";
        return false;
    }

    for (int row = 0; row < TILE_ROWS; ++row)
    {
        for (int col = 0; col < TILE_COLS; ++col)
        {
            file << map[row][col];
            if (col < TILE_COLS - 1)
                file << " ";
        }
        file << "\n";
    }

    file.close();
    std::cout << "Overwritten: " << filename << "\n";
    return true;
}

// ----------------------------------------------------------------------------
//
// Function: free_tile_textures
// Unloads all tile textures from memory to prevent leaks.
//
// ---------------------------------------------------------------------------- 
void free_tile_textures()
{
    for (int i = 0; i < NUM_TILE_TYPES; ++i)
    {
        if (TileTextures[i])
        {
            AEGfxTextureUnload(TileTextures[i]);
            TileTextures[i] = nullptr;
        }
    }
}

// ----------------------------------------------------------------------------
//
// Function: get_door_world_pos
// Scans the map to find a door tile and returns its center world coordinates.
//
// ---------------------------------------------------------------------------- 
void get_door_world_pos(f32& out_x, f32& out_y)
{
    for (int r = 0; r < TILE_ROWS; ++r)
    {
        for (int c = 0; c < TILE_COLS; ++c)
        {
            if (map[r][c] == DOOR || map[r][c] == OPENED_DOOR)
            {
                out_x = world_left + c * tile_size + tile_size * 0.5f;
                out_y = world_top - r * tile_size - tile_size * 0.5f;
                return;
            }
        }
    }

    // fallback if no door found
    out_x = 0.f;
    out_y = 0.f;
}

// ----------------------------------------------------------------------------
//
// Function: load_map_to_editor
// Loads a map file into a double-pointer map array used by the Level Editor.
//
// ---------------------------------------------------------------------------- 
bool load_map_to_editor(const char* filename, s32** editor_map)
{
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    int row = 0;

    while (std::getline(file, line) && row < TILE_ROWS)
    {
        std::istringstream ss(line);
        int col = 0, tileType;

        while (ss >> tileType && col < TILE_COLS)
        {
            editor_map[row][col] = tileType;
            ++col;
        }
        while (col < TILE_COLS)
            editor_map[row][col++] = EMPTY;

        ++row;
    }
    while (row < TILE_ROWS)
        for (int c = 0; c < TILE_COLS; ++c)
            editor_map[row++][c] = EMPTY;

    file.close();
    return true;
}

// ----------------------------------------------------------------------------
//
// Function: load_map_from_file
// Loads a map file into the standard 2D array used by the game engine.
//
// ---------------------------------------------------------------------------- 
bool load_map_from_file(const char* filename, s32 map[][TILE_COLS])
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open map file: " << filename << "\n";
        return false;
    }

    std::string line;
    int row = 0;

    while (std::getline(file, line) && row < TILE_ROWS)
    {
        std::istringstream ss(line);
        int col = 0;
        int tileType;

        while (ss >> tileType && col < TILE_COLS)
        {
            map[row][col] = tileType;
            ++col;
        }

        // Fill remaining columns with EMPTY if line is shorter than TILE_COLS
        while (col < TILE_COLS)
        {
            map[row][col] = EMPTY;
            ++col;
        }

        ++row;
    }

    // Fill remaining rows with EMPTY if file has fewer than TILE_ROWS
    while (row < TILE_ROWS)
    {
        for (int c = 0; c < TILE_COLS; ++c)
            map[row][c] = EMPTY;
        ++row;
    }

    file.close();
    return true;
}

// ----------------------------------------------------------------------------
//
// Function: save_map_to_file
// Searches existing level files to create a new unique level filename (e.g., lvl5.txt).
//
// ---------------------------------------------------------------------------- 
bool save_map_to_file(s32** map, std::string& out_filename)
{
    s32 highest = 0;
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA("Assets/Levels/lvl*.txt", &find_data);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do {
            std::string fname = find_data.cFileName;
            size_t start = fname.find("lvl") + 3;
            size_t end = fname.find(".txt");
            if (start != std::string::npos && end != std::string::npos)
            {
                int n = std::stoi(fname.substr(start, end - start));
                if (n > highest) highest = n;
            }
        } while (FindNextFileA(hFind, &find_data));
        FindClose(hFind);
    }

    std::string filename = "Assets/Levels/lvl" + std::to_string(highest + 1) + ".txt";
    out_filename = filename;

    std::ofstream file{ filename };

    if (!file.is_open())
    {
        std::cerr << "Failed to open map file for writing: " << filename << "\n";
        return false;
    }

    for (int row = 0; row < TILE_ROWS; ++row)
    {
        for (int col = 0; col < TILE_COLS; ++col)
        {
            file << map[row][col];

            if (col < TILE_COLS - 1)
                file << " ";
        }
        file << "\n";
    }

    file.close();
    std::cout << "Saved to: " << filename << "\n";
    return true;
}

// ----------------------------------------------------------------------------
//
// Function: draw_map
// Renders the entire tile map centered on the screen.
//
// ---------------------------------------------------------------------------- 
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
        f32 y = startY - row * tile_size - tile_size * 0.5f;
        for (int col = 0; col < TILE_COLS; ++col)
        {
            int tileType = map[row][col];

            if (tileType == 0)
                continue;

            f32 x = startX + col * tile_size + tile_size * 0.5f;

            // Handle special entities that occupy a floor tile
            if (tileType == KNIGHT || tileType == WIZARD || tileType == TOMB || tileType == TREE) {
                render_sprite(square_mesh, x, y, tile_size, tile_size, TileTextures[2]);
                map[row][col] = FLOOR; // Reset to floor for internal map logic
                continue;
            }

            render_sprite(square_mesh, x, y, tile_size, tile_size, TileTextures[tileType]);
        }
    }
}

// ----------------------------------------------------------------------------
//
// Function: draw_map_preview
// Renders a miniature version of the map for UI selection screens.
//
// ---------------------------------------------------------------------------- 
void draw_map_preview(s32 preview[][TILE_COLS], f32 center_x, f32 center_y, f32 preview_w, f32 preview_h)
{
    f32 dot_w = preview_w / (f32)TILE_COLS;
    f32 dot_h = preview_h / (f32)TILE_ROWS;

    f32 start_x = center_x - preview_w * 0.5f;
    f32 start_y = center_y + preview_h * 0.5f;

    for (int r = 0; r < TILE_ROWS; ++r)
    {
        for (int c = 0; c < TILE_COLS; ++c)
        {
            int tile = preview[r][c];
            if (tile == 0) continue;

            f32 tx = start_x + c * dot_w + dot_w * 0.5f;
            f32 ty = start_y - r * dot_h - dot_h * 0.5f;

            if (TileTextures[tile])
                render_editor_ui(TileTextures[tile], dot_w, dot_h, 1.f, tx, ty);
        }
    }
}

// ----------------------------------------------------------------------------
//
// Function: preview_default_level
// Hardcodes the layout for the default level preview.
//
// ---------------------------------------------------------------------------- 
void preview_default_level(s32 out_map[][TILE_COLS])
{
    for (int r = 0; r < TILE_ROWS; ++r)
        for (int c = 0; c < TILE_COLS; ++c)
            out_map[r][c] = EMPTY;

    // PLAYER ROOM 1
    for (int r = 3; r <= 10; ++r)
        for (int c = 2; c <= 9; ++c)
            out_map[r][c] = FLOOR;
    for (int c = 1; c <= 10; ++c) { out_map[2][c] = WALL; out_map[11][c] = WALL; }
    for (int r = 2; r <= 6; ++r) { out_map[r][3] = WALL; out_map[r][8] = WALL; }
    for (int r = 3; r <= 11; ++r) { out_map[r][1] = WALL; out_map[r][10] = WALL; }
    for (int c = 4; c <= 7; ++c)    out_map[10][c] = FLOOR;
    out_map[6][10] = FLOOR;

    // PLAYER ROOM 2
    for (int r = 18; r < 27; ++r)
        for (int c = 2; c < 10; ++c)
            out_map[r][c] = FLOOR;
    for (int c = 1; c <= 10; ++c) { out_map[18][c] = WALL; out_map[26][c] = WALL; }
    for (int r = 18; r <= 26; ++r) { out_map[r][1] = WALL; out_map[r][10] = WALL; }
    out_map[21][10] = out_map[22][10] = FLOOR;

    // VERTICAL CORRIDOR
    for (int r = 11; r < 18; ++r)
        for (int c = 4; c < 8; ++c)
            out_map[r][c] = FLOOR;
    for (int r = 11; r < 18; ++r)
        out_map[r][3] = out_map[r][8] = WALL;

    // BOSS ROOM 1
    for (int r = 3; r < 12; ++r)
        for (int c = 25; c < 31; ++c)
            out_map[r][c] = FLOOR;
    for (int c = 24; c <= 31; ++c) { out_map[2][c] = WALL; out_map[12][c] = WALL; }
    for (int r = 2; r <= 12; ++r)   out_map[r][24] = out_map[r][31] = WALL;
    out_map[7][28] = OPENED_DOOR;

    // HORIZONTAL CORRIDOR TO BOSS ROOM 1
    for (int c = 9; c < 25; ++c)  out_map[7][c] = FLOOR;
    for (int c = 10; c < 25; ++c) { out_map[6][c] = WALL; out_map[8][c] = WALL; }

    // BOSS ROOM 2
    for (int r = 18; r < 24; ++r)
        for (int c = 25; c < 32; ++c)
            out_map[r][c] = FLOOR;
    for (int c = 24; c <= 32; ++c) { out_map[17][c] = WALL; out_map[24][c] = WALL; }
    for (int r = 17; r <= 24; ++r)  out_map[r][24] = out_map[r][32] = WALL;

    // HORIZONTAL CORRIDOR TO BOSS ROOM 2
    for (int c = 9; c < 25; ++c)  out_map[21][c] = FLOOR;
    for (int c = 10; c < 25; ++c) { out_map[20][c] = WALL; out_map[22][c] = WALL; }
}

// ----------------------------------------------------------------------------
// Coordinate Helper Functions
// ----------------------------------------------------------------------------

s32 convert_to_col(f32 x) {
    return static_cast<s32>((x - world_left) / tile_size);
}

s32 convert_to_row(f32 y) {
    return static_cast<s32>((world_top - y) / tile_size);
}

f32 row_to_world(s32 row) {
    return static_cast<f32>(world_top - row * tile_size - tile_size * 0.5);
}

f32 col_to_world(s32 col) {
    return static_cast<f32>(col * tile_size + world_left + tile_size * 0.5);
}

// ----------------------------------------------------------------------------
//
// Function: world_to_tile
// Converts a world-space position (x, y) into tile map indices (row, col)
// Returns true if the position lies within the map bounds, false otherwise.
//
// ---------------------------------------------------------------------------- 
bool world_to_tile(f32 worldX, f32 worldY, int& outRow, int& outCol)
{
    outCol = convert_to_col(worldX);
    outRow = convert_to_row(worldY);

    if (outRow < 0 || outRow >= TILE_ROWS ||
        outCol < 0 || outCol >= TILE_COLS)
    {
        return false;
    }

    return true;
}