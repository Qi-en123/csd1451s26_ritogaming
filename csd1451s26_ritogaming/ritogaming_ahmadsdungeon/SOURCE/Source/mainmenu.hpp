/*!************************************************************************
\file:          mainmenu.hpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu

\brief:         This file declares the 6 cycle functions that load,
                initialize, update, draw, free, and unload the main menu
                game state.
**************************************************************************/

#pragma once

// ----------------------------------------------------------------------------
//
// Function: mainmenu_load
// Loads all assets required for the main menu screen.
//
// ----------------------------------------------------------------------------
void mainmenu_load();

// ----------------------------------------------------------------------------
//
// Function: mainmenu_init
// Initializes animation state, particle systems, and input for the main menu.
//
// ----------------------------------------------------------------------------
void mainmenu_init();

// ----------------------------------------------------------------------------
//
// Function: mainmenu_update
// Handles button input, animation updates, and particle logic each frame.
//
// ----------------------------------------------------------------------------
void mainmenu_update();

// ----------------------------------------------------------------------------
//
// Function: mainmenu_draw
// Renders the background, animated characters, buttons, and custom cursor.
//
// ----------------------------------------------------------------------------
void mainmenu_draw();

// ----------------------------------------------------------------------------
//
// Function: mainmenu_free
// Frees runtime mesh resources and stops background music.
//
// ----------------------------------------------------------------------------
void mainmenu_free();

// ----------------------------------------------------------------------------
//
// Function: mainmenu_unload
// Unloads all textures and destroys the font when leaving the main menu.
//
// ----------------------------------------------------------------------------
void mainmenu_unload();