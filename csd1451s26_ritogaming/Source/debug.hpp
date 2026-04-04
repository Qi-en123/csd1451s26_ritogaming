/*!************************************************************************
\file:          debug.hpp
\author:        Ahmad Qusyairi Bin Mohammad Jinna
\par DP email:  ahmadqusyairi.b@digipen.edu

\brief:         Debug drawing utilities for walls and player
                tiles. Provides visual feedback for collision,
                puzzle logic, and enemy states.
**************************************************************************/

#pragma once
#include "AETypes.h"

namespace DEBUG {
    extern s32 DEBUGGING;
    extern s32 DRAW_WALLS;
    extern s32 DRAW_PLAYER_TILE;
    extern s32 DRAW_PUZZLE_GRAB_RANGE;
    extern s32 DRAW_PUZZLE_BLOCKS;
    extern s32 DRAW_BUTTON_SPIKE_LINKS;
    extern s32 DRAW_ENEMIES;

    // ----------------------------------------------------------------------------
    //
    // Function: draw_all_walls
    // Iterates through the global wall vector and renders their bounding boxes.
    //
    // ---------------------------------------------------------------------------- 
    void draw_all_walls();

    // ----------------------------------------------------------------------------
    //
    // Function: draw_player_tile
    // Visualizes the specific grid tile and collision box for the player.
    //
    // ---------------------------------------------------------------------------- 
    void draw_player_tile();

    // ----------------------------------------------------------------------------
    //
    // Function: draw_puzzle_blocks
    // Draws bounding boxes for puzzle elements, colored by their specific type.
    //
    // ---------------------------------------------------------------------------- 
    void draw_puzzle_blocks();

    // ----------------------------------------------------------------------------
    //
    // Function: draw_puzzle_grab_range
    // Draws the interaction grab range around pushable puzzle objects.
    //
    // ---------------------------------------------------------------------------- 
    void draw_puzzle_grab_range();

    // ----------------------------------------------------------------------------
    //
    // Function: draw_button_spike_links
    // Draws lines from buttons to their paired spike traps for logical verification.
    //
    // ---------------------------------------------------------------------------- 
    void draw_button_spike_links();

    // ----------------------------------------------------------------------------
    //
    // Function: draw_enemies
    // Renders debug information and state-colored boxes for enemy NPCs.
    //
    // ---------------------------------------------------------------------------- 
    void draw_enemies();
}