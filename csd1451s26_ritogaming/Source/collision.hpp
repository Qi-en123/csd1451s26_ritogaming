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

// Forward declaration
struct Player;

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

// Check circle vs circle collision
bool cc_collision(f32 x1, f32 y1, f32 r1, f32 x2, f32 y2, f32 r2);

// Check circle vs rectangle collision
bool cr_collision(f32 cx, f32 cy, f32 cr, f32 rx, f32 ry, f32 rw, f32 rh);

bool puzzle_to_wall_collision(f32 newX, f32 newY);

// Check collision for a player position
bool check_player_collision(f32 newX, f32 newY, f32 playerHalfSize);

// Check collision for puzzle blocks
//bool check_puzzle_collision(f32 newX, f32 newY);

// Check collision for projectiles
bool check_projectile_collision(f32 projX, f32 projY, f32 projRadius);

// Circle to rectangle collision (alternative implementation)
bool circle_to_rectangle(f32 cx, f32 cy, f32 cr, f32 rx, f32 ry, f32 rw, f32 rh);

// Convert world position to tile indices
bool world_to_tile(f32 worldX, f32 worldY, int& outRow, int& outCol);

// Check if player is on a door and handle door opening
void check_and_open_doors(Player* player);

// Debug visualization
void debug_draw_all_walls();