/*!************************************************************************
\file:          pch.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu

\brief:         Precompiled header that includes all core engine headers
                and game system headers (enemy, player, camera, map,
                collision, renderer, puzzle, ui_manager) for faster
                compilation across the project.
**************************************************************************/

#pragma once

// Core engine and third-party libraries
#include "AEEngine.h"
#include "json.hpp"

// Core game systems
#include "map.hpp"
#include "collision.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "puzzle.hpp"

// Gameplay systems (grouped here to reduce compile times across files)
#include "enemy.hpp"
#include "enemy_manager.hpp"
#include "player.hpp"
#include "ui_manager.hpp"