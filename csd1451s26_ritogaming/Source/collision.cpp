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
extern Player player_instance;
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
    return !(ax + ahw < bx - bhw || // A�s right < B�s left - no collision
        ax - ahw > bx + bhw || // A�s left > B�s right - no collision
        ay + ahh < by - bhh || // A�s top < B�s bottom - no collision
        ay - ahh > by + bhh);  // A�s bottom > B�s top - no collision
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
            //printf("Colliding with wall at (%.2f, %.2f)\n", wall.x, wall.y);

            return true; // collision detected
        }
    }
    return false; // no collision
}

//this is for puzzle against walls
bool puzzle_to_wall_collision(f32 newX, f32 newY) {
    for (Wall& wall : g_walls)
    {
        if (ss_collision(newX, newY, tile_size/2.0f, tile_size/2.0f,
            wall.x, wall.y, wall.halfW, wall.halfH))
        {
            // Debug: print tile coordinates
            //printf("Colliding with wall at (%.2f, %.2f)\n", wall.x, wall.y);

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
        // Uses your cr_collision (Circle vs Rect) math
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
    // These values MUST match the map's drawing origin in draw_map()
    // startX represents the left-most edge of the map in world coordinates
    // startY represents the top-most edge of the map in world coordinates
    f32 startX = -1600.f / 2.f;
    f32 startY = 900.f / 2.f;

    // Convert the world X position to a tile column index
    // 1. Subtract startX to get distance from the left edge of the map
    // 2. Divide by tile_size to determine how many tiles across
    // 3. Cast to int to floor the value and obtain the tile index
    outCol = (int)((worldX - startX) / tile_size);

    // Convert the world Y position to a tile row index
    // Note: Y is inverted because tile rows increase downward
    // 1. Subtract worldY from startY to get distance from the top edge
    // 2. Divide by tile_size to determine how many tiles down
    // 3. Cast to int to floor the value and obtain the tile index
    outRow = (int)((startY - worldY) / tile_size);

    // Bounds check to ensure the calculated tile indices are valid
    // Prevents accessing tiles outside the map array
    if (outRow < 0 || outRow >= TILE_ROWS ||
        outCol < 0 || outCol >= TILE_COLS)
    {
        return false; // World position is outside the map
    }

    // Successfully converted world position to valid tile indices
    return true;
}
// Checks if the player is close to any door tile and opens it
// A door is considered "close" if it is within a 3x3 tile area
// around the player's current tile position
void check_and_open_doors()
{
    // Variables to store the player's current tile row and column
    int playerRow, playerCol;

    // Convert the player's world position into tile map indices
    // If the player is outside the map bounds, exit early
    if (!world_to_tile(player_instance.x, player_instance.y, playerRow, playerCol))
        return;

    // Loop through the surrounding tiles in a 3x3 area
    // This allows the door to open when the player is near,
    // rather than requiring perfect alignment on the door tile
    for (int r = playerRow - 1; r <= playerRow + 1; ++r)
    {
        for (int c = playerCol - 1; c <= playerCol + 1; ++c)
        {
            // Ensure the tile indices are within valid map bounds
            // Prevents accessing memory outside the map array
            if (r < 0 || r >= TILE_ROWS ||
                c < 0 || c >= TILE_COLS)
            {
                continue;
            }

            // Check if the current tile is a closed door
            if (map[r][c] == DOOR)
            {
                // Change the tile to an opened door
                map[r][c] = OPENED_DOOR;

                // Regenerate wall collision data since the door
                // should no longer block player movement
                generate_walls_from_map(map, tile_size);

                // Exit after opening one door to prevent
                // multiple doors opening at the same time
                return;
            }
        }
    }
}