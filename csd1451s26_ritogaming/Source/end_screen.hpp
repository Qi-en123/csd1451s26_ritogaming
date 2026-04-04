#pragma once

extern bool is_win;
extern bool is_paused;
extern int restart_level_state;

// ----------------------------------------------------------------------------
//
// Function: end_screen_load
// Loads assets required for the end screen, such as textures and fonts.
//
// ---------------------------------------------------------------------------- 
void end_screen_load();

// ----------------------------------------------------------------------------
//
// Function: end_screen_unload
// Unloads assets to free memory when exiting the end screen state.
//
// ---------------------------------------------------------------------------- 
void end_screen_unload();

// ----------------------------------------------------------------------------
//
// Function: end_screen_free
// Cleans up any dynamic memory allocated specifically for the end screen.
//
// ---------------------------------------------------------------------------- 
void end_screen_free();

// ----------------------------------------------------------------------------
//
// Function: end_screen_init
// Initializes variables, button states, and positions for the end screen.
//
// ---------------------------------------------------------------------------- 
void end_screen_init();

// ----------------------------------------------------------------------------
//
// Function: end_screen_update
// Processes user input and UI logic for the win or game over screens.
//
// ---------------------------------------------------------------------------- 
void end_screen_update();

// ----------------------------------------------------------------------------
//
// Function: end_screen_draw
// Renders the end screen UI, including results and navigation buttons.
//
// ---------------------------------------------------------------------------- 
void end_screen_draw();

// ----------------------------------------------------------------------------
//
// Function: pause_update
// Handles input detection and menu navigation while the game is paused.
//
// ---------------------------------------------------------------------------- 
void pause_update();

// ----------------------------------------------------------------------------
//
// Function: pause_draw
// Renders the pause menu overlay and associated UI elements.
//
// ---------------------------------------------------------------------------- 
void pause_draw();