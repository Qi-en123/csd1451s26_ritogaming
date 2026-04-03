/*!************************************************************************
\file:          debug.hpp
\brief:         Debug drawing utilities for walls and player tiles.
**************************************************************************/

#pragma once

namespace DEBUG {
    static int DEBUGGING = 0;
    static int DRAW_WALLS = 0;
    static int DRAW_PLAYER_TILE = 0;
    static int DRAW_PUZZLE_GRAB_RANGE = 0;
    static int DRAW_PUZZLE_BLOCKS = 0;
    static int DRAW_BUTTON_SPIKE_LINKS = 0;
    static int DRAW_ENEMIES = 0;

    // Collision
    void draw_all_walls();
    void draw_player_tile();

    // Puzzle
    void draw_puzzle_blocks();      // draws bounding boxes colored by type
    void draw_puzzle_grab_range();  // draws grab range around pushables
    void draw_button_spike_links(); // draws lines from buttons to their paired spikes

    void draw_enemies();
}