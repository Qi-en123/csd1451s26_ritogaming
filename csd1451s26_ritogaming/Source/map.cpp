/*!************************************************************************
\file:          map.cpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         This file handles tile-based map rendering and initialization.
                It creates the tile mesh, draws individual tiles and the 
                full map and defines the dungeon layout including rooms, 
                corridors, players,and boss areas.
**************************************************************************/
#include "AEEngine.h"
#include "renderer.hpp"
#include "map.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>   
#include <windows.h>

// Dummy map
s32 map[TILE_ROWS][TILE_COLS];
s32 boss_map[TILE_ROWS][TILE_COLS];
//f32 tile_size{};
AEGfxTexture* TileTextures[NUM_TILE_TYPES];

// load textures for tile types
bool load_tile_textures(const char* folderPath)
{
    for (int i = 0; i < NUM_TILE_TYPES; ++i)
    {
        // Build file path: folderPath + "tile_i.png"
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/tile_%04d.png", folderPath, i);

        // Load the texture
        if (TileTextures[i]) continue; // already loaded, skip
        TileTextures[i] = AEGfxTextureLoad(filename);
        if (!TileTextures[i])
        {
            std::cerr << "Failed to load tile texture: " << filename << std::endl;
            return false;
        }
    }

    return true;
}

// save map to file, avoid creating new files
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


// free tile textures when done
void free_tile_textures()
{
    for (int i = 0; i < NUM_TILE_TYPES; ++i)
    {
        if (TileTextures[i])
        {
            AEGfxTextureUnload(TileTextures[i]); // unload texture
            TileTextures[i] = nullptr;           // avoid dangling pointer
        }
    }
}

// get door position in world coordinates for enemy spawn
void get_door_world_pos(f32& out_x, f32& out_y)
{
    f32 world_left = -800.0f;
    f32 world_top = 450.0f;

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
// load map to editor to edit
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
// load map from file to game
bool load_map_from_file(const char* filename, s32 map[][TILE_COLS])
{
	std::ifstream file(filename); // Open the map file for reading
	if (!file.is_open()) { // Check if the file was successfully opened
		std::cerr << "Failed to open map file: " << filename << "\n"; // Print an error message if the file cannot be opened
		return false; // Return false to indicate failure
    }

	std::string line; // Variable to hold each line read from the file
	int row = 0; // Row index for the map array

	while (std::getline(file, line) && row < TILE_ROWS) // Read each line of the file
    {
		std::istringstream ss(line); // ss acts as a stream for the current line
		int col = 0; // Column index for the map array
		int tileType; // Variable to hold the tile type read from the line

		while (ss >> tileType && col < TILE_COLS)  // Read each tile type in the line
        {
			map[row][col] = tileType; // Store the tile type in the map array
			++col; // Move to the next column
        }

        // Fill remaining columns with EMPTY if line too short
        while (col < TILE_COLS)
        {
			map[row][col] = EMPTY; // Fill with EMPTY tile type
			++col; // Move to the next column
        }

		++row; // Move to the next row
    }

    // Fill remaining rows with EMPTY if file has fewer rows
	while (row < TILE_ROWS) 
    {
		for (int c = 0; c < TILE_COLS; ++c) // Fill remaining columns with EMPTY
            map[row][c] = EMPTY;
		++row; // Move to the next row
    }

	file.close(); // Close the file after reading
	return true; // Return true to indicate successful loading
}

bool save_map_to_file(s32** map, std::string& out_filename)
{
    // Find highest existing lvl number and go one above it
    s32 highest = 0;
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA("Assets/Levels/lvl*.txt", &find_data);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do {
			std::string fname = find_data.cFileName; // file name
            size_t start = fname.find("lvl") + 3; // position of the number 
            size_t end = fname.find(".txt"); //position of the end of number
            if (start != std::string::npos && end != std::string::npos) // ensure both number and .txt exist
            {
				int n = std::stoi(fname.substr(start, end - start)); // extract the number from the filename
				if (n > highest) highest = n; // update highest if this file's number is greater
            }
        } while (FindNextFileA(hFind, &find_data));
        FindClose(hFind);
    }

    std::string filename = "Assets/Levels/lvl" + std::to_string(highest + 1) + ".txt";
    out_filename = filename; // return the filename used

    // Now open that filename for writing
    std::ofstream file{ filename };

    if (!file.is_open())
    {
        std::cerr << "Failed to open map file for writing: " << filename << "\n";
        return false;
    }

    // Write map data
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


// ------------------------------------------------------------
// Draw the entire map perfectly centered
// ------------------------------------------------------------
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
        f32 y = startY - row * tile_size;
        for (int col = 0; col < TILE_COLS; ++col)
        {
            int tileType = map[row][col];

            // Skip empty tiles
            if (tileType == 0)
                continue;
            
            f32 x = startX + col * tile_size;

            if (tileType == KNIGHT || tileType == WIZARD || tileType == TOMB || tileType == TREE) {
                render_sprite(rect_mesh_center, x, y, tile_size, tile_size, TileTextures[2]);
                map[row][col] = FLOOR; // for minimap to render floor
                continue;
            }
            

            render_sprite(rect_mesh_center, x, y, tile_size, tile_size, TileTextures[tileType]);
        }
    }
}

void draw_boss_map(f32 tile_size)
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
        f32 y = startY - row * tile_size;
        for (int col = 0; col < TILE_COLS; ++col)
        {
            int tileType = boss_map[row][col];

            // Skip drawing if tileType is 0
            if (tileType == 0)
                continue;

            f32 x = startX + col * tile_size;
            render_sprite(rect_mesh_center, x, y, tile_size, tile_size, TileTextures[tileType]);
        }
    }
}


// ------------------------------------------------------------
// Initialize a simplified boss map layout (one big room)
// ------------------------------------------------------------
void initialize_boss_map()
{
    // Clear the whole boss map first
    for (int r = 0; r < TILE_ROWS; ++r)
        for (int c = 0; c < TILE_COLS; ++c)
            boss_map[r][c] = WALL; // start with walls everywhere

    // ============================================================
    // BIG BOSS ROOM
    // ============================================================
     // Clear the whole boss map first
    for (int r = 0; r < TILE_ROWS; ++r)
        for (int c = 0; c < TILE_COLS; ++c)
            boss_map[r][c] = EMPTY; // start with walls everywhere
// Floor
    for (int r = 3; r <= 10; ++r)
        for (int c = 2; c <= 9; ++c)
            boss_map[r][c + 5] = FLOOR;

    // Top and bottom walls
    for (int c = 1; c <= 10; ++c)
    {
        boss_map[2][c + 5] = WALL;  // top wall
        boss_map[11][c + 5] = WALL;  // bottom wall
    }

    // Left wall
    for (int r = 3; r <= 11; ++r)
        boss_map[r][6] = WALL;

    // Right wall
    for (int r = 3; r <= 11; ++r)
        boss_map[r][15] = WALL;

}


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

//preview level layout for level select screen, same layout as default level
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