/*!************************************************************************
\file:                          debug.cpp
\author:                        Ahmad Qusyairi Bin Mohammad Jinna
\par DP email:                  ahmadqusyairi.b@digipen.edu

\brief:                         Implementation of debug drawing utilities.
                                Moved from collision.cpp.
**************************************************************************/

#include <cmath>
#include "debug.hpp"
#include "collision.hpp"
#include "player.hpp"
#include "map.hpp"
#include "renderer.hpp"
#include "puzzle.hpp"
#include "enemy_manager.hpp"

namespace DEBUG {

    // ----------------------------------------------------------------------------
    //
    // Function: draw_all_walls
    // Debug draw for all walls. Shows all wall positions as red rectangles.
    //
    // ---------------------------------------------------------------------------- 
    void draw_all_walls()
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 0.5f);

        for (Wall& wall : g_walls)
            draw_square(wall.x, wall.y, wall.halfW * 2.0f, wall.halfH * 2.0f);
    }

    // ----------------------------------------------------------------------------
    //
    // Function: draw_player_tile
    // Debug draw for player and door tile positions.
    // Draws char1 (blue), char2 (green), and the door tile (yellow).
    //
    // ---------------------------------------------------------------------------- 
    void draw_player_tile()
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);

        f32 coll = player_instance.stats.character_size;

        AEGfxSetColorToMultiply(0.0f, 0.0f, 1.0f, 0.5f);
        draw_square(player_instance.char1.x, player_instance.char1.y, coll, coll);

        AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 0.5f);
        draw_square(player_instance.char2.x, player_instance.char2.y, coll, coll);

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 0.5f);

        f32 door_x, door_y;
        get_door_world_pos(door_x, door_y);
        draw_square(door_x, door_y, tile_size, tile_size);
    }

    // ----------------------------------------------------------------------------
    //
    // Function: draw_puzzle_blocks
    // Debug draw for all puzzle blocks, colored by type.
    // PUSHABLE = orange, BUTTON_UP = yellow, BUTTON_DOWN = green, etc.
    //
    // ---------------------------------------------------------------------------- 
    void draw_puzzle_blocks()
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);

        for (Puzzle::puzzle& p : Puzzle::puzzle_blocks)
        {
            if (!p.active) continue;

            switch (p.type)
            {
            case Puzzle::PUSHABLE:    AEGfxSetColorToMultiply(1.0f, 0.5f, 0.0f, 0.6f); break;
            case Puzzle::BUTTON_UP:   AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 0.6f); break;
            case Puzzle::BUTTON_DOWN: AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 0.6f); break;
            case Puzzle::PIT:         AEGfxSetColorToMultiply(0.0f, 0.0f, 0.5f, 0.6f); break;
            case Puzzle::PIT_FILLED:  AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 0.6f); break;
            case Puzzle::SPIKE:       AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 0.6f); break;
            default: break;
            }

            draw_square(p.posx, p.posy, Puzzle::puzzle_size, Puzzle::puzzle_size);
        }
    }

    // ----------------------------------------------------------------------------
    //
    // Function: draw_puzzle_grab_range
    // Debug draw grab range around all active pushable puzzles.
    // Shows the range within which the player can grab and drag them.
    //
    // ---------------------------------------------------------------------------- 
    void draw_puzzle_grab_range()
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(0.0f, 1.0f, 1.0f, 0.4f);

        f32 puzzle_half = (tile_size - 8.0f) / 2.0f;
        f32 player_half = player_instance.stats.character_size / 2.0f;
        f32 grab_range = (player_half + puzzle_half) + Puzzle::PUZZLE_GRAB_RANGE;

        for (Puzzle::puzzle& p : Puzzle::puzzle_blocks)
        {
            if (!p.active) continue;
            if (p.type != Puzzle::PUSHABLE) continue;

            draw_square(p.posx, p.posy, grab_range, grab_range);
        }
    }

    // ----------------------------------------------------------------------------
    //
    // Function: draw_button_spike_links
    // Debug draw markers on buttons (yellow) and their paired spikes (red).
    //
    // ---------------------------------------------------------------------------- 
    void draw_button_spike_links()
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);

        for (Puzzle::buttonTrigger& bt : Puzzle::btns)
        {
            Puzzle::puzzle* btn_puzzle = nullptr;
            for (Puzzle::puzzle& p : Puzzle::puzzle_blocks)
            {
                if ((p.type == Puzzle::BUTTON_UP || p.type == Puzzle::BUTTON_DOWN)
                    && p.button_id == bt.id)
                {
                    btn_puzzle = &p;
                    break;
                }
            }
            if (!btn_puzzle) continue;

            AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 0.8f);
            draw_square(btn_puzzle->posx, btn_puzzle->posy, 8.0f, 8.0f);

            for (std::pair<s32, s32>& coord : bt.coords)
            {
                f32 spike_x = col_to_world(coord.second);
                f32 spike_y = row_to_world(coord.first);

                AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 0.8f);
                draw_square(spike_x, spike_y, 8.0f, 8.0f);
            }
        }
    }

    // ----------------------------------------------------------------------------
    //
    // Function: draw_enemies
    // Debug draw for all active enemies. State is color coded:
    // chasing = orange, fleeing = white, attacking = red.
    //
    // ---------------------------------------------------------------------------- 
    void draw_enemies()
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);

        for (std::unique_ptr<base_enemy> const& e : enemy_manager_instance.get_enemies())
        {
            if (!e->is_active()) continue;

            f32 x = e->get_x();
            f32 y = e->get_y();
            f32 size = e->get_size();

            switch (e->get_state())
            {
            case chasing:   AEGfxSetColorToMultiply(1.0f, 0.5f, 0.0f, 0.5f); break;
            case fleeing:   AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f); break;
            case attacking: AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 0.5f); break;
            default: break;
            }
            draw_square(x, y, size, size);
        }
    }
}