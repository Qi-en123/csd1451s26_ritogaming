/*!************************************************************************
\file:						level1.hpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This header declares Level 1 functions for loading, initializing, updating, drawing, freeing resources, and unloading,
defining the interface for Level 1 game logic and state management.
and Restart, providing a central list of all possible game states.
**************************************************************************/


#pragma once


void level1_load();


void level1_init();


void level1_update();


void level1_draw();


void level1_free();


void level1_unload();

void trigger_level_transition(int target_state);
bool is_transitioning();