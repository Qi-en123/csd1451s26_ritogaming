/*!************************************************************************
\file:          levels.hpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         Declares functions to load, initialize, update, draw, free, and unload
				the game's level editor
**************************************************************************/

#pragma once

void levels_load();
void levels_init();
void levels_update();
void levels_draw();
void levels_free();
void levels_unload();
void ensure_level_paths_loaded();