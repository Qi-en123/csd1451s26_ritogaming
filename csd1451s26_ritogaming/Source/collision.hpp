#pragma once

#include "player.hpp"
#include "map.hpp"

// Check if a player’s tentative position collides with walls
// Returns true if collision occurs
bool check_player_collision(const Player* player, float newX, float newY,
    int map[TILE_ROWS][TILE_COLS], float tile_size, float startX, float startY);