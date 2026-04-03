/*!************************************************************************
\file:			level2.hpp
\author:		Jerod lim
                [add yall name]
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			This file declares functions for the boss fight (level)
                to load assets, initialize level, update game logic,
                render & free resources.
**************************************************************************/
#pragma once

// ------------------------------------------------------------------------
// 
// Loads all required assets/resources for the boss room state
// (e.g., textures, audio)
// 
// ------------------------------------------------------------------------ 
void level2_load();

// ------------------------------------------------------------------------
// 
// Initializes variables and sets up the initial state before execution begins
// 
// ------------------------------------------------------------------------ 
void level2_init();

// ------------------------------------------------------------------------
// 
// Updates game logic every frame (e.g., input handling, state changes)
// 
// ------------------------------------------------------------------------ 
void level2_update();

// ------------------------------------------------------------------------
// 
// Renders all visuals related to the boss room state to the screen
// 
// ------------------------------------------------------------------------ 
void level2_draw();

// ------------------------------------------------------------------------
// 
// Frees dynamically allocated memory and temporary resources used during
// the state
// 
// ------------------------------------------------------------------------ 
void level2_free();

// ------------------------------------------------------------------------
// 
// Unloads assets/resources that were loaded in level2_load()
// 
// ------------------------------------------------------------------------ 
void level2_unload();