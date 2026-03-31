/*!************************************************************************
\file:          debug.hpp
\brief:         Debug drawing utilities for walls and player tiles.
**************************************************************************/

#pragma once

namespace DEBUG {
    constexpr int DEBUGGING = 1;

    // Collision
    void draw_all_walls();
    void draw_player_tile();

    // Puzzle
    void draw_puzzle_blocks();      // draws bounding boxes colored by type
    void draw_puzzle_grab_range();  // draws grab range around pushables
    void draw_button_spike_links(); // draws lines from buttons to their paired spikes

    void draw_enemies();
}