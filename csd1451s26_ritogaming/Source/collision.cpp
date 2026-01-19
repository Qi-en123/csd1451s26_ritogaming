/*!************************************************************************
\file:          collision.cpp
\author:        Ahmad Qusyairi
\par DP email:  ahmadqusyairi.b@digipen.edu
\brief:         Implementation of collision detection for tile-based maps.
                Includes wall generation from a map and AABB collision 
                checks.

This file defines functions to:
1. Generate wall objects from a 2D map array.
2. Perform Axis-Aligned Bounding Box (AABB) collision checks.
3. Check collisions between a player and map walls.
**************************************************************************/

#include "collision.hpp"
#include <vector>

// Global vector storing all wall objects generated from the map
std::vector<Wall> g_walls;

/*!************************************************************************
\brief Generate wall objects from a 2D tile map.

\param map The 2D array representing the map tiles.
\param tile_size The width and height of each square tile in world units.

This function iterates through the 2D tile map and generates Wall
objects for all tiles marked as solid (e.g., tileID == 14). The walls
are centered in their respective tile positions and stored in the
global g_walls vector.
**************************************************************************/
void generate_walls_from_map(int map[TILE_ROWS][TILE_COLS], float tile_size)
{
    g_walls.clear();

    float startX = -1600.f / 2.f; // same as in DrawMap
    float startY = 900.f / 2.f - tile_size;

    for (int row = 0; row < TILE_ROWS; ++row)
    {
        float y = startY - row * tile_size + tile_size / 2.0f; // center of tile
        for (int col = 0; col < TILE_COLS; ++col)
        {
            float x = startX + col * tile_size + tile_size / 2.0f; // center of tile

            int tileID = map[row][col];

            // Solid tiles: example, tileID 14 = wall
            if (tileID == 14)
            {
                Wall w;
                w.x = x;
                w.y = y;
                w.halfW = tile_size / 2.0f;
                w.halfH = tile_size / 2.0f;

                g_walls.push_back(w);
            }
        }
    }
}

// AABB collision check
/*!************************************************************************
\brief Check collision between two Axis-Aligned Bounding Boxes (AABB).

\param ax X-coordinate of the first box center
\param ay Y-coordinate of the first box center
\param ahw Half-width of the first box
\param ahh Half-height of the first box
\param bx X-coordinate of the second box center
\param by Y-coordinate of the second box center
\param bhw Half-width of the second box
\param bhh Half-height of the second box
\return True if the boxes overlap, false otherwise

   A : The Player
   B : Wall etc.
**************************************************************************/
bool aabb_vs_aabb(float ax, float ay, float ahw, float ahh,
                  float bx, float by, float bhw, float bhh)
{
    return !(ax + ahw < bx - bhw || // A’s right < B’s left - no collision
             ax - ahw > bx + bhw || // A’s left > B’s right - no collision
             ay + ahh < by - bhh || // A’s top < B’s bottom - no collision
             ay - ahh > by + bhh);  // A’s bottom > B’s top - no collision
}

/*!************************************************************************
\brief Check if a player collides with any wall in the map.

\param newX X-coordinate of the player's center
\param newY Y-coordinate of the player's center
\param playerHalfSize Half-size of the player's collision box
\return True if collision detected, false otherwise

Iterates through all walls in g_walls and uses AABBvsAABB to check
for overlap with the player's bounding box.
**************************************************************************/
bool check_player_collision(float newX, float newY, float playerHalfSize)
{
    for (Wall& wall : g_walls)
    {
        if (aabb_vs_aabb(newX, newY, playerHalfSize, playerHalfSize,
            wall.x, wall.y, wall.halfW, wall.halfH))
        {
            return true; // collision detected
        }
    }

    return false; // no collision
}
