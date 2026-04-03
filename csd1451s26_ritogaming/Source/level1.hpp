/*!************************************************************************
\file:						level1.hpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This header declares Level 1 functions for loading,
                            initializing, updating, drawing, freeing resources,
                            and unloading, defining the interface for Level 1
                            game logic and state management.
**************************************************************************/

#pragma once

// ----------------------------------------------------------------------------
//
// Function: level1_load
// Loads all necessary assets for the level (textures, audio, fonts).
//
// ---------------------------------------------------------------------------- 
void level1_load();

// ----------------------------------------------------------------------------
//
// Function: level1_init
// Initializes level-specific variables, player spawns, and camera settings.
//
// ---------------------------------------------------------------------------- 
void level1_init();

// ----------------------------------------------------------------------------
//
// Function: level1_update
// Processes game logic, physics, and input for the level once per frame.
//
// ---------------------------------------------------------------------------- 
void level1_update();

// ----------------------------------------------------------------------------
//
// Function: level1_draw
// Renders all game objects, tiles, and UI elements to the screen.
//
// ---------------------------------------------------------------------------- 
void level1_draw();

// ----------------------------------------------------------------------------
//
// Function: level1_free
// Frees temporary memory and instances allocated during the init phase.
//
// ---------------------------------------------------------------------------- 
void level1_free();

// ----------------------------------------------------------------------------
//
// Function: level1_unload
// Unloads assets from memory to prepare for a new game state.
//
// ---------------------------------------------------------------------------- 
void level1_unload();

// ----------------------------------------------------------------------------
//
// Function: trigger_level_transition
// Starts a screen fade-out and prepares the state manager for a new level.
//
// ---------------------------------------------------------------------------- 
void trigger_level_transition(int target_state);

// ----------------------------------------------------------------------------
//
// Function: is_transitioning
// Checks if the level is currently in the middle of a fade transition.
//
// ---------------------------------------------------------------------------- 
bool is_transitioning();

// ----------------------------------------------------------------------------
//
// Function: trigger_boss_warning
// Activates the danger UI warning sequence before entering a boss area.
//
// ---------------------------------------------------------------------------- 
void trigger_boss_warning();