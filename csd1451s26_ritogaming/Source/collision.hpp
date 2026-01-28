/*!************************************************************************
\file:          collision.hpp
\author:        Ahmad Qusyairi
\par DP email:  ahmadqusyairi.b@digipen.edu
\brief:         This file consist of declarations of functions
                GenerateWallsFromMap, AABBvsAABB and CheckPlayerCollision
**************************************************************************/
#pragma once

#include "player.hpp"
#include "map.hpp"
#include <vector>

// Simple axis-aligned rectangle (AABB) for walls
struct Wall
{
    f32 x;      // center x
    f32 y;      // center y
    f32 halfW;  // half width
    f32 halfH;  // half height
};

// Global walls vector
extern std::vector<Wall> g_walls;

// Generate walls from actual dungeon map
void generate_walls_from_map(s32 map[TILE_ROWS][TILE_COLS], f32 tile_size);

// Check AABB vs AABB collision
bool ss_collision(
    f32 ax, f32 ay, f32 ahw, f32 ahh,
    f32 bx, f32 by, f32 bhw, f32 bhh
);

// Check collision for a player position
bool check_player_collision(f32 newX, f32 newY, f32 playerHalfSize);

void debug_draw_all_walls();
bool circle_to_rectangle(f32 cx, f32 cy, f32 cr, f32 rx, f32 ry, f32 rw, f32 rh);
