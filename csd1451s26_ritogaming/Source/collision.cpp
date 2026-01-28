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
#include "map.hpp"

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
void generate_walls_from_map(s32 map[TILE_ROWS][TILE_COLS], f32 tile_size)
{
    g_walls.clear();

    // MATCHING map.cpp: Use the exact same start positions
    f32 startX = -1600.f / 2.f;
    f32 startY = 900.f / 2.f;

    for (s32 row = 0; row < TILE_ROWS; ++row)
    {
        // MATCHING map.cpp: Use the exact same Y calculation
        f32 y = startY - row * tile_size;

        for (s32 col = 0; col < TILE_COLS; ++col)
        {
            // MATCHING map.cpp: Use the exact same X calculation
            f32 x = startX + col * tile_size;

            if (map[row][col] == 1)
            {
                Wall w;
                w.x = x; // Now matches the visual center used in DrawMap
                w.y = y; // Now matches the visual center used in DrawMap
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
bool ss_collision(
    f32 ax, f32 ay, f32 ahw, f32 ahh,
    f32 bx, f32 by, f32 bhw, f32 bhh
)
{
    return !(ax + ahw < bx - bhw || // A’s right < B’s left - no collision
        ax - ahw > bx + bhw || // A’s left > B’s right - no collision
        ay + ahh < by - bhh || // A’s top < B’s bottom - no collision
        ay - ahh > by + bhh);  // A’s bottom > B’s top - no collision
}

/**
 * @brief Checks for collision between a circular projectile and a square boss.
 * * @param cx Circle center X
 * \param cy Circle center Y
 * \param cr Circle radius
 * \param rx Rectangle center X
 * \param ry Rectangle center Y
 * \param rw Rectangle width (or size if square)
 * \param rh Rectangle height (or size if square)
 * \return true if colliding, false otherwise
 */
bool cr_collision(f32 cx, f32 cy, f32 cr, f32 rx, f32 ry, f32 rw, f32 rh) {
    // 1. Find the rectangle's edges
    f32 rLeft = rx - (rw / 2.0f);
    f32 rRight = rx + (rw / 2.0f);
    f32 rTop = ry + (rh / 2.0f);
    f32 rBottom = ry - (rh / 2.0f);

    // 2. Find the closest point on the rectangle to the circle center
    f32 closestX = cx;
    f32 closestY = cy;

    // Clamp closestX to the rectangle's range
    if (closestX < rLeft)  closestX = rLeft;
    else if (closestX > rRight) closestX = rRight;

    // Clamp closestY to the rectangle's range
    if (closestY < rBottom) closestY = rBottom;
    else if (closestY > rTop)    closestY = rTop;

    // 3. calculate the distance from the circle center to this closest point
    f32 distX = cx - closestX;
    f32 distY = cy - closestY;

    // 4. compare squared distance to squared radius (performance optimization)
    f32 distanceSq = (distX * distX) + (distY * distY);

    return distanceSq <= (cr * cr);
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
bool check_player_collision(f32 newX, f32 newY, f32 playerHalfSize)
{
    for (Wall& wall : g_walls)
    {
        if (ss_collision(newX, newY, playerHalfSize, playerHalfSize,
            wall.x, wall.y, wall.halfW, wall.halfH))
        {
            // Debug: print tile coordinates
            printf("Colliding with wall at (%.2f, %.2f)\n", wall.x, wall.y);

            return true; // collision detected
        }
    }
    return false; // no collision
}

/*!************************************************************************
\brief Debug draw for all walls
Shows all wall positions as red rectangles
// Go to main.cpp and uncomment the debug_draw_all_walls(); after
// drawmap()
**************************************************************************/
void debug_draw_all_walls()
{
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 0.5f); // red, semi-transparent

    for (Wall& wall : g_walls)
    {
        // draw_square expects full width/height
        draw_square(wall.x, wall.y, wall.halfW * 2.0f, wall.halfH * 2.0f);
    }
}
