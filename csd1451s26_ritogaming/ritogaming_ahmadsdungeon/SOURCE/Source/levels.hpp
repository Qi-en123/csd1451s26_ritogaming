/*!************************************************************************
\file:          levels.hpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu

\brief:         Declares functions to load, initialize, update, draw, free,
                and unload the game's level selection or editor state.
**************************************************************************/

#pragma once

// ----------------------------------------------------------------------------
//
// Function: levels_load
// Loads assets and resources required for the level selection/editor menu.
//
// ---------------------------------------------------------------------------- 
void levels_load();

// ----------------------------------------------------------------------------
//
// Function: levels_init
// Initializes selection variables, UI positions, and default level paths.
//
// ---------------------------------------------------------------------------- 
void levels_init();

// ----------------------------------------------------------------------------
//
// Function: levels_update
// Processes user input for level selection and handles menu logic.
//
// ---------------------------------------------------------------------------- 
void levels_update();

// ----------------------------------------------------------------------------
//
// Function: levels_draw
// Renders the level selection UI and any associated level previews.
//
// ---------------------------------------------------------------------------- 
void levels_draw();

// ----------------------------------------------------------------------------
//
// Function: levels_free
// Frees temporary memory allocated during the initialization of this state.
//
// ---------------------------------------------------------------------------- 
void levels_free();

// ----------------------------------------------------------------------------
//
// Function: levels_unload
// Unloads textures and UI elements to free memory for the next state.
//
// ---------------------------------------------------------------------------- 
void levels_unload();

// ----------------------------------------------------------------------------
//
// Function: ensure_level_paths_loaded
// Verifies and populates the list of available level files from the Assets folder.
//
// ---------------------------------------------------------------------------- 
void ensure_level_paths_loaded();