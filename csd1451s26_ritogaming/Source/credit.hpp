/*!************************************************************************
\file:          credit.hpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         Declares functions to load, initialize, update, draw, free,
                and unload the game's credits. Handles scrolling text with
                adjustable speed, perspective scaling, and automatic reset
                when the credits finish.
**************************************************************************/
#pragma once

// ----------------------------------------------------------------------------
//
// Function: credit_load
// Loads all assets required for the credits screen.
//
// ----------------------------------------------------------------------------
void credit_load();

// ----------------------------------------------------------------------------
//
// Function: credit_init
// Initializes scroll position, speed, and state for the credits screen.
//
// ----------------------------------------------------------------------------
void credit_init();

// ----------------------------------------------------------------------------
//
// Function: credit_update
// Updates scroll position each frame and handles input and reset logic.
//
// ----------------------------------------------------------------------------
void credit_update();

// ----------------------------------------------------------------------------
//
// Function: credit_draw
// Renders scrolling credit text with perspective scaling each frame.
//
// ----------------------------------------------------------------------------
void credit_draw();

// ----------------------------------------------------------------------------
//
// Function: credit_free
// Frees runtime resources used during the credits screen.
//
// ----------------------------------------------------------------------------
void credit_free();

// ----------------------------------------------------------------------------
//
// Function: credit_unload
// Unloads all assets when leaving the credits state.
//
// ----------------------------------------------------------------------------
void credit_unload();