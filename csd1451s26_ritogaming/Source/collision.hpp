/*!************************************************************************
\file:          collision.hpp
\author:        Ahmad Qusyairi Bin Mohammad Jinna
\par DP email:  ahmadqusyairi.b@digipen.edu

\brief:         This file consists of declarations of functions
                for map-based wall generation and various
                collision detection algorithms (AABB, Circle).
**************************************************************************/

#pragma once
#include "player.hpp"
#include "map.hpp"
#include <vector>

// ----------------------------------------------------------------------------
//
// Struct: Wall
// Simple axis-aligned rectangle (AABB) for walls.
//
// ---------------------------------------------------------------------------- 
struct Wall
{
    f32 x;      // center x
    f32 y;      // center y
    f32 halfW;  // half width
    f32 halfH;  // half height
};

// Global walls vector
extern std::vector<Wall> g_walls;
extern bool door_triggered;

// ----------------------------------------------------------------------------
//
// Function: generate_walls_from_map
// Generate walls from actual dungeon map.
//
// ---------------------------------------------------------------------------- 
void generate_walls_from_map(s32 tile_grid[TILE_ROWS][TILE_COLS]);

// ----------------------------------------------------------------------------
//
// Function: ss_collision
// Check AABB vs AABB collision.
//
// ---------------------------------------------------------------------------- 
bool ss_collision(
    f32 ax, f32 ay, f32 ahw, f32 ahh,
    f32 bx, f32 by, f32 bhw, f32 bhh
);

// ----------------------------------------------------------------------------
//
// Function: cc_collision
// Check circle vs circle collision.
//
// ---------------------------------------------------------------------------- 
bool cc_collision(f32 x1, f32 y1, f32 r1, f32 x2, f32 y2, f32 r2);

// ----------------------------------------------------------------------------
//
// Function: cr_collision
// Check circle vs rectangle collision.
//
// ---------------------------------------------------------------------------- 
bool cr_collision(f32 cx, f32 cy, f32 cr, f32 rx, f32 ry, f32 rw, f32 rh);

// ----------------------------------------------------------------------------
//
// Function: puzzle_to_wall_collision
// Check puzzle collision against walls.
//
// ---------------------------------------------------------------------------- 
bool puzzle_to_wall_collision(f32 newX, f32 newY);

// ----------------------------------------------------------------------------
//
// Function: puzzle_to_inactive_collision
// Check puzzle collision against inactive player.
//
// ---------------------------------------------------------------------------- 
bool puzzle_to_inactive_collision(f32 newX, f32 newY);

// ----------------------------------------------------------------------------
//
// Function: check_player_collision
// Check collision for a player position.
//
// ---------------------------------------------------------------------------- 
bool check_player_collision(f32 newX, f32 newY, f32 playerHalfSize);

// ----------------------------------------------------------------------------
//
// Function: check_projectile_collision
// Check collision for projectiles.
//
// ---------------------------------------------------------------------------- 
bool check_projectile_collision(f32 projX, f32 projY, f32 projRadius);

// ----------------------------------------------------------------------------
//
// Function: circle_to_rectangle
// Circle to rectangle collision (alternative implementation).
//
// ---------------------------------------------------------------------------- 
bool circle_to_rectangle(f32 cx, f32 cy, f32 cr, f32 rx, f32 ry, f32 rw, f32 rh);

// ----------------------------------------------------------------------------
//
// Function: check_and_open_doors
// Check if player is on a door and handle door opening.
//
// ---------------------------------------------------------------------------- 
void check_and_open_doors(Player* player);