/*!************************************************************************
\file:          collision.cpp
\author:        Ahmad Qusyairi Bin Mohammad Jinna
\par DP email:  ahmadqusyairi.b@digipen.edu

\brief:         Implementation of collision detection fortile-based
                maps. Includes wall generationfrom a map, AABB,
                Circle-to-Circle, and Circle-to-AABB collision checks.
**************************************************************************/

#include "collision.hpp"
#include "player.hpp"
#include <vector>
#include "map.hpp"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "renderer.hpp"
#include "utils.hpp"
#include "debug.hpp"
#include "level1.hpp"

// Global vector storing all wall objects generated from the map
std::vector<Wall> g_walls;
bool door_triggered = false;

// ----------------------------------------------------------------------------
//
// Function: generate_walls_from_map
// Iterates through the 2D tile map and generates Wall objects for all 
// tiles marked as solid (WALL or EMPTY). The walls are centered in their 
// respective tile positions and stored in the global g_walls vector.
//
// ---------------------------------------------------------------------------- 
void generate_walls_from_map(s32 tile_grid[TILE_ROWS][TILE_COLS])
{
    g_walls.clear();

    // MATCHING map.cpp: Use the exact same start positions
    f32 startX = AEGfxGetWinMinX();
    f32 startY = AEGfxGetWinMaxY();

    for (s32 row = 0; row < TILE_ROWS; ++row)
    {
        // MATCHING map.cpp: Use the exact same Y calculation
        f32 y = startY - row * tile_size - tile_size * 0.5f;

        for (s32 col = 0; col < TILE_COLS; ++col)
        {
            // MATCHING map.cpp: Use the exact same X calculation
            f32 x = startX + col * tile_size + tile_size * 0.5f;

            if (tile_grid[row][col] == WALL || tile_grid[row][col] == EMPTY)
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

// ----------------------------------------------------------------------------
//
// Function: ss_collision
// Check collision between two Axis-Aligned Bounding Boxes (AABB).
// Returns true if the boxes overlap, false otherwise.
//
// ---------------------------------------------------------------------------- 
bool ss_collision(
    f32 ax, f32 ay, f32 ahw, f32 ahh,
    f32 bx, f32 by, f32 bhw, f32 bhh
)
{
    return !(ax + ahw < bx - bhw || // A's right < B's left - no collision
        ax - ahw > bx + bhw ||      // A's left > B's right - no collision
        ay + ahh < by - bhh ||      // A's top < B's bottom - no collision
        ay - ahh > by + bhh);       // A's bottom > B's top - no collision
}

// ----------------------------------------------------------------------------
//
// Function: cc_collision
// Performs a collision check between two circles by comparing squared 
// distance to the squared sum of their radii to avoid square root expense.
//
// ---------------------------------------------------------------------------- 
bool cc_collision(f32 x1, f32 y1, f32 r1, f32 x2, f32 y2, f32 r2) {
    f32 distX = x1 - x2;
    f32 distY = y1 - y2;
    f32 distanceSq = (distX * distX) + (distY * distY);
    f32 radiiSum = r1 + r2;

    return distanceSq <= (radiiSum * radiiSum);
}

// ----------------------------------------------------------------------------
//
// Function: cr_collision
// Checks for collision between a circular projectile and a square boss/wall
// by finding the closest point on the rectangle to the circle center.
//
// ---------------------------------------------------------------------------- 
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

// ----------------------------------------------------------------------------
//
// Function: check_player_collision
// Iterates through all walls in g_walls and uses AABBvsAABB to check
// for overlap with the player's bounding box.
//
// ---------------------------------------------------------------------------- 
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

// ----------------------------------------------------------------------------
//
// Function: puzzle_to_wall_collision
// Special collision check for puzzle elements against map walls.
//
// ---------------------------------------------------------------------------- 
bool puzzle_to_wall_collision(f32 newX, f32 newY) {
    f32 puzzle_hw = (tile_size / 2.0f) - 2.0f;

    for (Wall& wall : g_walls)
    {
        if (ss_collision(newX, newY, puzzle_hw, puzzle_hw,
            wall.x, wall.y, wall.halfW, wall.halfH))
        {
            return true; // collision detected
        }
    }
    return false; // no collision
}

// ----------------------------------------------------------------------------
//
// Function: puzzle_to_inactive_collision
// Checks for collision between puzzle elements and the character currently 
// not controlled by the player.
//
// ---------------------------------------------------------------------------- 
bool puzzle_to_inactive_collision(f32 newX, f32 newY) {
    f32 puzzle_hw = (tile_size / 2.0f) - 2.0f;

    // Puzzle to inactive player collision
    Character& ip = (player_instance.current_character == 1)
        ? player_instance.char2 : player_instance.char1;
    f32 phs = player_instance.stats.character_size / 2.0f;

    if (ss_collision(newX, newY, puzzle_hw, puzzle_hw,
        ip.x, ip.y, phs, phs)) {
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
//
// Function: check_projectile_collision
// Uses Circle-to-Rectangle collision detection to determine if a circular 
// projectile has impacted any wall in the map.
//
// ---------------------------------------------------------------------------- 
bool check_projectile_collision(f32 projX, f32 projY, f32 projRadius) {
    for (Wall& wall : g_walls) {
        if (cr_collision(projX, projY, projRadius, wall.x, wall.y, wall.halfW * 2.0f, wall.halfH * 2.0f)) {
            return true;
        }
    }
    return false;
}

// ----------------------------------------------------------------------------
//
// Function: circle_to_rectangle
// Helper function finding the closest point on a rectangle to a circle 
// to detect intersection.
//
// ---------------------------------------------------------------------------- 
bool circle_to_rectangle(f32 cx, f32 cy, f32 cr, f32 rx, f32 ry, f32 rw, f32 rh) {
    f32 rLeft = rx - (rw / 2.0f);
    f32 rRight = rx + (rw / 2.0f);
    f32 rTop = ry + (rh / 2.0f);
    f32 rBottom = ry - (rh / 2.0f);

    f32 closestX = cx;
    f32 closestY = cy;

    if (closestX < rLeft)  closestX = rLeft;
    else if (closestX > rRight) closestX = rRight;

    if (closestY < rBottom) closestY = rBottom;
    else if (closestY > rTop)    closestY = rTop;

    f32 distX = cx - closestX;
    f32 distY = cy - closestY;

    f32 distanceSq = (distX * distX) + (distY * distY);
    return distanceSq <= (cr * cr);
}

// ----------------------------------------------------------------------------
//
// Function: check_and_open_doors
// Converts player world position to tile indices to check for OPENED_DOOR 
// tiles, handling level transitions or boss room warnings.
//
// ---------------------------------------------------------------------------- 
void check_and_open_doors(Player* player)
{
    if (!player || !player->current_player) return;

    f32 px = player->current_player->x + 8.f;
    f32 py = player->current_player->y - 8.f;

    bool near_door = false;

    s32 door_row = -1, door_col = -1;
    if (world_to_tile(px, py, door_row, door_col))
    {
        if (map[door_row][door_col] == OPENED_DOOR)
            near_door = true;
    }

    if (near_door)
    {
        if (!door_triggered)
        {
            door_triggered = true;

            s32 found_index = -1;
            s32 num_levels = (s32)level_paths.size();
            for (s32 i = 0; i < num_levels; ++i)
            {
                if (level_paths[i] == selected_level_path)
                {
                    found_index = i;
                    break;
                }
            }

            s32 next_index = found_index + 1;
            while (next_index < num_levels && level_paths[next_index].empty())
                next_index++;

            if (found_index != -1 && next_index < num_levels)
            {
                selected_level_path = level_paths[next_index];
                if (next_index < (s32)level_puzzle_paths.size())
                    selected_level_puzzle_path = level_puzzle_paths[next_index];
                trigger_level_transition(GS_RESTART);
            }
            else
            {
                trigger_boss_warning();
            }
        }
    }
    else
    {
        door_triggered = false;
    }
}