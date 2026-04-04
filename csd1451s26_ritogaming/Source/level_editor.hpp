/*!************************************************************************
\file:          level_editor.hpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         Declares functions to load, initialize, update, draw, free, and unload
                the game's level editor
**************************************************************************/

#pragma once

// ----------------------------------------------------------------------------
//
// Enum: EditorState
// Defines the current mode of the level editor, allowing the user to switch
// between placing environment tiles and configuring interactive puzzle elements.
//
// ----------------------------------------------------------------------------
enum EditorState {
    EDIT_TILES,   // Mode for painting walls, floors, and basic geometry
    EDIT_PUZZLES  // Mode for placing/configuring graves, buttons, pits, and spikes
};

// ----------------------------------------------------------------------------
//
// Function: level_editor_load
// Assets and non-volatile data loading (textures, sounds, config files).
//
// ----------------------------------------------------------------------------
void level_editor_load();

// ----------------------------------------------------------------------------
//
// Function: level_editor_init
// Initializes state variables, resets camera, and prepares the grid for editing.
//
// ----------------------------------------------------------------------------
void level_editor_init();

// ----------------------------------------------------------------------------
//
// Function: level_editor_update
// Handles input processing, camera movement, tile placement, and UI logic.
//
// ----------------------------------------------------------------------------
void level_editor_update();

// ----------------------------------------------------------------------------
//
// Function: level_editor_draw
// Renders the grid, current map data, palette UI, and editor feedback messages.
//
// ----------------------------------------------------------------------------
void level_editor_draw();

// ----------------------------------------------------------------------------
//
// Function: level_editor_free
// Cleans up dynamic memory and temporary state used during the session.
//
// ----------------------------------------------------------------------------
void level_editor_free();

// ----------------------------------------------------------------------------
//
// Function: level_editor_unload
// Unloads assets from memory to free up resources when exiting the editor.
//
// ----------------------------------------------------------------------------
void level_editor_unload();