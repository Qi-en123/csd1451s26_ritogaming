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
    float x;      // center x
    float y;      // center y
    float halfW;  // half width
    float halfH;  // half height
};

// Global walls vector
extern std::vector<Wall> g_walls;

// Generate walls from actual dungeon map
void generate_walls_from_map(int map[TILE_ROWS][TILE_COLS], float tile_size);

// Check AABB vs AABB collision
bool aabb_vs_aabb(
    float ax, float ay, float ahw, float ahh,
    float bx, float by, float bhw, float bhh
);

// Check collision for a player position
bool check_player_collision(float newX, float newY, float playerHalfSize);

void debug_draw_all_walls();
