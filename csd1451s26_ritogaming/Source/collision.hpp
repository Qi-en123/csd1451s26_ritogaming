#pragma once

#include "player.hpp"
#include "map.hpp"

#define TILE_ROWS 90
#define TILE_COLS 160

// Must match DrawMap
constexpr float TILE_SIZE = 20.0f;          // size of one tile
constexpr float MAP_START_X = -1600.0f / 2;   // X of top-left map tile
constexpr float MAP_START_Y = 900.0f / 2 - TILE_SIZE; // Y of top-left map tile

// Check if a player’s tentative position collides with walls
// Returns true if collision occurs
bool check_player_collision(const Player* player, float newX, float newY, int map[TILE_ROWS][TILE_COLS]);
