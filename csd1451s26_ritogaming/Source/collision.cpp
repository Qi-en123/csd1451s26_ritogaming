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
#include "game_state_manager.hpp"
#include "game_state_list.hpp"

// Global vector storing all wall objects generated from the map
std::vector<Wall> g_walls;

/*!************************************************************************
\brief Generate wall objects from a 2D tile map.

\param map The 2D array representing the map tiles.
\param tile_size The width and height of each square tile in world units.

This function iterates through the 2D tile map and generates Wall
objects for all tiles marked as solid (e.g., tileID == 1). The walls
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

            if (map[row][col] == WALL || map[row][col] == SPECIAL_DOOR)
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
    return !(ax + ahw < bx - bhw || // A's right < B's left - no collision
        ax - ahw > bx + bhw || // A's left > B's right - no collision
        ay + ahh < by - bhh || // A's top < B's bottom - no collision
        ay - ahh > by + bhh);  // A's bottom > B's top - no collision
}

/*!************************************************************************
\brief Check for collision between two circular objects.

\param x1  X-coordinate of the first circle's center.
\param y1  Y-coordinate of the first circle's center.
\param r1  Radius of the first circle.
\param x2  X-coordinate of the second circle's center.
\param y2  Y-coordinate of the second circle's center.
\param r2  Radius of the second circle.

\return True if the circles overlap or touch, false otherwise.

This function performs a collision check by calculating the squared
distance between the centers of two circles and comparing it to the
squared sum of their radii. This approach avoids the computational
expense of a square root operation.
**************************************************************************/
bool cc_collision(f32 x1, f32 y1, f32 r1, f32 x2, f32 y2, f32 r2) {
    f32 distX = x1 - x2;
    f32 distY = y1 - y2;
    f32 distanceSq = (distX * distX) + (distY * distY);
    f32 radiiSum = r1 + r2;

    return distanceSq <= (radiiSum * radiiSum);
}

/**
 * @brief Checks for collision between a circular projectile and a square boss.
 * @param cx Circle center X
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
        if (ss_collision(newX, newY, playerHalfSize - 2.0f, playerHalfSize - 2.0f,
            wall.x, wall.y, wall.halfW, wall.halfH))
        {
            return true; // collision detected
        }
    }
    return false; // no collision
}

//this is for puzzle against walls
bool puzzle_to_wall_collision(f32 newX, f32 newY) {
    for (Wall& wall : g_walls)
    {
        if (ss_collision(newX, newY, (tile_size / 2.0f) - 2.0f, (tile_size / 2.0f)-2.0f,
            wall.x, wall.y, wall.halfW, wall.halfH))
        {
            return true; // collision detected
        }
    }
    return false; // no collision
}


/*!************************************************************************
\brief Debug draw for all walls
Shows all wall positions as red rectangles
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

/*!************************************************************************
\brief Check if a circular projectile collides with any wall in the map.

\param projX      X-coordinate of the projectile's center.
\param projY      Y-coordinate of the projectile's center.
\param projRadius The radius of the circular projectile.

\return True if a collision is detected with any wall, false otherwise.

This function iterates through the global vector of wall objects (g_walls).
It uses Circle-to-Rectangle collision detection (cr_collision) by
converting each wall's half-dimensions into full width and height to
determine if the projectile has impacted a solid tile.
**************************************************************************/
bool check_projectile_collision(f32 projX, f32 projY, f32 projRadius) {
    for (Wall& wall : g_walls) {
        if (cr_collision(projX, projY, projRadius, wall.x, wall.y, wall.halfW * 2.0f, wall.halfH * 2.0f)) {
            return true;
        }
    }
    return false;
}

bool circle_to_rectangle(f32 cx, f32 cy, f32 cr, f32 rx, f32 ry, f32 rw, f32 rh) {
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

// Converts a world-space position (x, y) into tile map indices (row, col)
// Returns true if the position lies within the map bounds, false otherwise
bool world_to_tile(f32 worldX, f32 worldY, int& outRow, int& outCol)
{
    // Map's top-left origin in world coordinates (must match draw_map())
    f32 startX = -1600.f / 2.f; // left edge
    f32 startY = 900.f / 2.f;   // top edge

    // Convert world X to column index
    outCol = (int)((worldX - startX) / tile_size);

    // Convert world Y to row index (Y increases downward in tiles)
    outRow = (int)((startY - worldY) / tile_size);

    // Check bounds
    if (outRow < 0 || outRow >= TILE_ROWS ||
        outCol < 0 || outCol >= TILE_COLS)
    {
        return false; // outside map
    }

    return true; // valid tile indices
}

/*!************************************************************************
\brief Opens the door tile if the player is standing exactly on it

\param player Pointer to the player structure

This function converts the player's world position to tile indices and
checks if they're standing on an OPENED_DOOR tile. If so, it triggers
the transition to the boss room.
**************************************************************************/
void check_and_open_doors(Player* player)
{
    if (!player || !player->current_player)
        return;

    int playerRow, playerCol;

    // Convert player's world position to tile indices
    if (!world_to_tile(player->current_player->x, player->current_player->y, playerRow, playerCol))
        return; // player outside map

    // Check if the player is exactly on a door tile
    if (map[playerRow][playerCol] == OPENED_DOOR)
    {
        next = GS_BOSS_ROOM;
    }
}