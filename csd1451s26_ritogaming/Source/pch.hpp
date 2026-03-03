/*!************************************************************************
\file:          pch.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         Precompiled header – single inclusion point for all shared
                headers across the project.

                Including all headers here in dependency order eliminates
                circular include issues that arise from interdependent game
                systems (e.g. player <-> enemy <-> manager). Every .cpp
                file should include this file instead of individual headers.
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