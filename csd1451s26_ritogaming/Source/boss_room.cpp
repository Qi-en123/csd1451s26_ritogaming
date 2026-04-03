/*!************************************************************************
\file:			boss_room.cpp
\author:		Jerod lim
                [add yall name]
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			This file implements functions for the boss fight (level)
                to load assets, initialize level, update game logic,
                render & free resources.
**************************************************************************/

#include "AEEngine.h"
#include <cstdio>
#include "map.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "collision.hpp"
#include "game_state_manager.hpp"
#include "puzzle.hpp"
#include "utils.hpp"
#include "minimap.hpp"
#include "enemy_manager.hpp"
#include "ui_manager.hpp"
#include "game_state_list.hpp"
#include "end_screen.hpp"
#include "debug.hpp"
#include "audio_manager.hpp"
#include "boss_room.hpp"

namespace {
    bool enemy_spawned = false;    
    AEGfxTexture* skeleton_texture = nullptr;
    bool cursor_active = false;
    s32 mouse_x, mouse_y;
}

void level2_load()
{
    // Load textures properly
    if (!load_tile_textures("Assets/Sprites/Tiles"))
        next = GS_QUIT; // If tile textures fail to load, quit the game
    load_ui_textures(&ui_manager);
    skeleton_texture = AEGfxTextureLoad("Assets/Sprites/Tiles/dead_last_location.png");
    end_screen_load();
}

void level2_init()
{
    //Disable cursor
    AEInputShowCursor(false);

    //Init default game state vals
    is_win = false;

    //Intialize default game state graphics
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    Camera::zoom = 0.9f;
    init_mesh();

    //Load boos room
    load_map_from_file("Assets/Levels/boss_room.txt", map);
    generate_walls_from_map(map, tile_size);

    enemy_spawned = false;
    
    //Initialize & spawn player/characters
    player_instance.stats.character_size = tile_size - 8.0f;
    player_system_init(&player_instance,
        TileTextures[4], TileTextures[3],
        skeleton_texture);    
    spawn_player_from_map(&player_instance, tile_size);

    // Camera follows the active character via the pointer
    Camera::cam = Camera::init_cam(
        &player_instance.current_player->x,
        &player_instance.current_player->y);

    //totalWorldW = ((f32)TILE_COLS * tile_size) / minimap_zoom;
    //totalWorldH = ((f32)TILE_ROWS * tile_size) / minimap_zoom;
    //minimap_init(totalWorldW, totalWorldH, 200.0f);

    // Clear any existing enemies then lock zone so ghost/bat respawn never fires here
    boss_proj_manager.clear();
    enemy_manager_instance.clear_all();
    enemy_manager_instance.set_zone(boss_room);

    // Boss room coordinates from initialize_boss_map(): rows 3-10, cols 7-15
    // Center of boss room
    f32 dragon_x = world_left + (13 * tile_size);  // Column 11 (center of cols 7-15)
    f32 dragon_y = world_top - (13 * tile_size); // Row 6.5 (center of rows 3-10)

    // Room bounds: approximate boundaries of the boss room
    // Adjusted for the boss room layout in initialize_boss_map()
    f32 room_left = world_left;
    f32 room_right = world_left + (27 * tile_size);
    f32 room_top = world_top;
    f32 room_bottom = world_top - (27 * tile_size);

    enemy_manager_instance.spawn_dragon(dragon_x, dragon_y, room_left, room_right, room_top, room_bottom);

    g_bgm_current_track = &g_audio.bgm_bossroom;
    play_bgm(g_audio.bgm_bossroom, 1.0f);
}

void level2_update()
{
    //Get mouse position
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    //Check if game is paused
    pause_update();
    if (is_paused)
        return;

    //Update delta time
    dt = (f32)AEFrameRateControllerGetFrameTime();

    //Check if player left-clicked
    cursor_active = AEInputCheckCurr(AEVK_LBUTTON);

    //Update zoom levels
    get_zoom(dt);

    // Bounds for boss room
    Camera::simulate_camera(&Camera::cam,
        &player_instance.current_player->x,
        &player_instance.current_player->y);

    player_system_update(&player_instance);
    entity_system_update(&player_instance, &enemy_manager_instance);


    // Update enemy manager (for the dragon)
    enemy_manager_instance.update(dt, &player_instance, &Camera::cam);
    boss_proj_manager.update(dt, &Camera::cam);
    boss_proj_manager.check_player_collision(&player_instance);

    if (enemy_manager_instance.get_active_enemy_count() > 0)
        enemy_spawned = true;

    // Lose condition
    if (player_is_dead(&player_instance))
    {
        is_win = false;
        restart_level_state = GS_BOSS_ROOM;
        next = GS_LOSE;
    }

    // Win condition (level 2 only)
    if (enemy_spawned && enemy_manager_instance.get_active_enemy_count() == 0)
    {
        is_win = true;
        restart_level_state = GS_BOSS_ROOM;
        next = GS_WIN;
    }
}

void level2_draw()
{
    AEGfxReset();
    //draw_boss_map(tile_size);
    draw_map(tile_size);

    if (DEBUG::DEBUGGING) {
        // Debug: draw all walls as semi-transparent red
        DEBUG::draw_all_walls();
        DEBUG::draw_player_tile();
        DEBUG::draw_puzzle_grab_range();
        DEBUG::draw_puzzle_blocks();
        DEBUG::draw_button_spike_links();
        DEBUG::draw_enemies();
    }

	enemy_manager_instance.render();
    render_enemy_hp(&ui_manager, &enemy_manager_instance);
	render_boss_projectiles(boss_proj_manager);   

    player_system_render(&player_instance);    
    
    //minimap_draw(&player_instance, map, TileTextures);
    render_player_hud(&ui_manager, &player_instance);
    render_character_ui(&ui_manager, &player_instance);
    pause_draw();

    //Renders the custom cursor
    render_cursor(&ui_manager, &player_instance, mouse_x, mouse_y, cursor_active); //iihih
}

void level2_free()
{
    // Cleanup enemies
    free_ui_textures(&ui_manager);
    boss_proj_manager.clear();
    enemy_manager_instance.clear_all();
    player_system_free(&player_instance);
    free_mesh();

    stop_bgm();
    stop_sfx();
    g_bgm_current_track = nullptr;
}

void level2_unload()
{
    free_tile_textures();
    //free_mesh();
    player_system_free(&player_instance);

    if (skeleton_texture)
    {
        AEGfxTextureUnload(skeleton_texture);
        skeleton_texture = nullptr;
    }
    end_screen_unload();
    minimap_unload();
}