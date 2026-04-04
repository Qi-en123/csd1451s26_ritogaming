/*!************************************************************************
\file:			opening.hpp
\author:		Goh Qi En
\par DP email:	g.qien@digipen.edu

\brief:			This file declares the 6 cycle functions that
				load, initialize, update, draw, and unload
				the opening logo with fade-in/out effects.
**************************************************************************/

#pragma once

// ----------------------------------------------------------------------------
//
// Function: opening_load
// Loads the necessary assets (textures, etc.) for the opening sequence.
//
// ----------------------------------------------------------------------------
void opening_load();

// ----------------------------------------------------------------------------
//
// Function: opening_init
// Initializes variables, timers, and alpha values for the fade effects.
//
// ----------------------------------------------------------------------------
void opening_init();

// ----------------------------------------------------------------------------
//
// Function: opening_update
// Handles the logic for the logo sequence, including fade timers and 
// state transitions.
//
// ----------------------------------------------------------------------------
void opening_update();

// ----------------------------------------------------------------------------
//
// Function: opening_draw
// Renders the logo to the screen using the calculated alpha transparency.
//
// ----------------------------------------------------------------------------
void opening_draw();

// ----------------------------------------------------------------------------
//
// Function: opening_free
// Frees any temporary memory allocated during the initialization phase.
//
// ----------------------------------------------------------------------------
void opening_free();

// ----------------------------------------------------------------------------
//
// Function: opening_unload
// Unloads assets from memory to prepare for the next game state.
//
// ----------------------------------------------------------------------------
void opening_unload();