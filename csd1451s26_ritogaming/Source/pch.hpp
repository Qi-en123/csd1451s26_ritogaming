/*!************************************************************************
\file:          enemy_manager.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
<<<<<<< Updated upstream
\brief:         EnemyManager – zone-based spawning, food drops, combat helpers
\
\               Zone layout (based on map.cpp):
\
\               tutorial_area  – Player Room 2 (rows 18-26, cols 2-9)
\                   + vertical corridor (rows 11-17, cols 4-7)
\                   ? 8 ghost boys, respawn after RESPAWN_DELAY
\
\               combat_area    – Player Room 1 (rows 3-10, cols 2-9)
\                   + horizontal corridor (row 7, cols 9-24)
\                   ? 5 bats, respawn after RESPAWN_DELAY
\
\                boss_room      – Boss room (level 2)
\                   ? 1 dragon, no respawn
=======
\
\brief:         Precompiled header that includes all core engine headers
\               and game system headers (enemy, player, camera, map,
\               collision, renderer, puzzle, ui_manager) for faster
\               compilation across the project.
>>>>>>> Stashed changes
**************************************************************************/

#pragma once

#include "AEEngine.h"
#include "map.hpp"
#include "collision.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "puzzle.hpp"

// Claire's files
#include "enemy.hpp"
#include "enemy_manager.hpp"
#include "player.hpp"
#include "ui_manager.hpp"