/*!************************************************************************
\file:			guide.hpp
\author:		Jerod lim
\par DP email:	jerodyaowei.lim@digipen.edu

\brief:			This source file declares the Game State functions, Load,
				Init, Update, Draw, Free & Unload. For use when transitioning
				between game states.
**************************************************************************/

#pragma once

// ------------------------------------------------------------------------
// 
// Loads all required assets/resources for the guide state
// (e.g., textures, audio)
// 
// ------------------------------------------------------------------------ 
void guide_load();

// ------------------------------------------------------------------------
// 
// Initializes variables and sets up the initial state before execution begins
// 
// ------------------------------------------------------------------------ 
void guide_init();

// ------------------------------------------------------------------------
// 
// Updates game logic every frame (e.g., input handling, state changes)
// 
// ------------------------------------------------------------------------ 
void guide_update();

// ------------------------------------------------------------------------
// 
// Renders all visuals related to the guide state to the screen
// 
// ------------------------------------------------------------------------ 
void guide_draw();

// ------------------------------------------------------------------------
// 
// Frees dynamically allocated memory and temporary resources used during
// the state
// 
// ------------------------------------------------------------------------ 
void guide_free();

// ------------------------------------------------------------------------
// 
// Unloads assets/resources that were loaded in guide_load()
// 
// ------------------------------------------------------------------------ 
void guide_unload();