/*!************************************************************************
\file:          player.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         This file implements the player system using AEEngine.
                It manages two switchable characters, handles player input,
                updates transformations, renders player visuals and indicators,
                and handles resource initialization and cleanup.
**************************************************************************/
#include "collision.hpp"
#include "player.hpp"
#include <crtdbg.h>
#include "camera.hpp"
#include "renderer.hpp"
#include "puzzle.hpp"

extern int map[TILE_ROWS][TILE_COLS];
extern std::vector<Wall> g_walls;

static void player_system_update_transforms(player* player_ptr) {
    if (!player_ptr) return;

    // Update character 1 transform
    AEMtx33 translate1;
    AEMtx33Trans(&translate1, player_ptr->char1_x, player_ptr->char1_y);
    AEMtx33Concat(&player_ptr->transform1, &player_ptr->rotate, &player_ptr->character_scale);
    AEMtx33Concat(&player_ptr->transform1, &translate1, &player_ptr->transform1);

    // Update character 2 transform
    AEMtx33 translate2;
    AEMtx33Trans(&translate2, player_ptr->char2_x, player_ptr->char2_y);
    AEMtx33Concat(&player_ptr->transform2, &player_ptr->rotate, &player_ptr->character_scale);
    AEMtx33Concat(&player_ptr->transform2, &translate2, &player_ptr->transform2);

    // Update triangle position (just above the character)
    AEMtx33 triangle_translate;
    if (player_ptr->current_character == 1) {
        // Position triangle just above character 1 - USING CONFIG OFFSET
        AEMtx33Trans(&triangle_translate, player_ptr->char1_x, player_ptr->char1_y + player_ptr->config.triangle_offset);
    }
    else {
        // Position triangle just above character 2 - USING CONFIG OFFSET
        AEMtx33Trans(&triangle_translate, player_ptr->char2_x, player_ptr->char2_y + player_ptr->config.triangle_offset);
    }

    AEMtx33Concat(&player_ptr->triangle_transform, &player_ptr->rotate, &player_ptr->triangle_scale);
    AEMtx33Concat(&player_ptr->triangle_transform, &triangle_translate, &player_ptr->triangle_transform);
}

void player_system_init(player* player_ptr, const player_config* config) {
    if (!player_ptr) return;

    // Store configuration 
    if (config) {
        player_ptr->config = *config;
    }

    player_ptr->char1_x = player_ptr->config.char1_start_x;
    player_ptr->char1_y = player_ptr->config.char1_start_y;
    player_ptr->char2_x = player_ptr->config.char2_start_x;
    player_ptr->char2_y = player_ptr->config.char2_start_y;

    // Current position (active character)
    player_ptr->x = player_ptr->char1_x;
    player_ptr->y = player_ptr->char1_y;

    // Player properties FROM CONFIG
    player_ptr->current_character = 1;
    player_ptr->speed = player_ptr->config.move_speed;

    // Load textures FROM CONFIG PATHS
    //player_ptr->char1_texture = AEGfxTextureLoad(player_ptr->config.char1_texture_path);
    //player_ptr->char2_texture = AEGfxTextureLoad(player_ptr->config.char2_texture_path);

    // Initialize scale matrices FROM CONFIG
    AEMtx33Scale(&player_ptr->character_scale, player_ptr->config.character_size, player_ptr->config.character_size);
    AEMtx33Scale(&player_ptr->triangle_scale, player_ptr->config.triangle_size, player_ptr->config.triangle_size);
    AEMtx33Rot(&player_ptr->rotate, 0);

    player_ptr->char1_element = WATER;
    player_ptr->char2_element = FIRE;

    // Initialize transform matrices
    player_system_update_transforms(player_ptr);
}

void player_system_handle_input(player* player_ptr) {
    if (!player_ptr) return;

    // Character swapping
    if (AEInputCheckTriggered(AEVK_1)) {
        player_ptr->current_character = 1;
    }
    if (AEInputCheckTriggered(AEVK_2)) {
        player_ptr->current_character = 2;
    }

    // Movement - only affect active character
    f32* move_x = NULL;
    f32* move_y = NULL;

    if (player_ptr->current_character == 1) {
        move_x = &player_ptr->char1_x;
        move_y = &player_ptr->char1_y;
    }
    else {
        move_x = &player_ptr->char2_x;
        move_y = &player_ptr->char2_y;
    }   

    Element curr_elem = (player_ptr->current_character == 1) ?
        player_ptr->char1_element : player_ptr->char2_element;

    if (move_x && move_y) {
        float nextX = *move_x;
        float nextY = *move_y;

        //to check if player is dragging a block
        static bool is_dragging;
        
        if (AEInputCheckCurr(AEVK_UP) || AEInputCheckCurr(AEVK_W)) {
            nextY += is_dragging ? 1.0f : player_ptr->speed;
            player_ptr->last_dir_x = 0.0f; player_ptr->last_dir_y = 1.0f;
        }
        if (AEInputCheckCurr(AEVK_DOWN) || AEInputCheckCurr(AEVK_S)) {
            nextY -= is_dragging ? 1.0f : player_ptr->speed;
            player_ptr->last_dir_x = 0.0f; player_ptr->last_dir_y = -1.0f;
        }
        if (AEInputCheckCurr(AEVK_LEFT) || AEInputCheckCurr(AEVK_A)) {
            nextX -= is_dragging ? 1.0f : player_ptr->speed;
            player_ptr->last_dir_x = -1.0f; player_ptr->last_dir_y = 0.0f;
        }
        if (AEInputCheckCurr(AEVK_RIGHT) || AEInputCheckCurr(AEVK_D)) {
            nextX += is_dragging ? 1.0f : player_ptr->speed;
            player_ptr->last_dir_x = 1.0f; player_ptr->last_dir_y = 0.0f;
        }

        // Half-size for AABB collision
        float halfSize = player_ptr->config.character_size / 2.0f;
        //store index of current puzzle being interacted with
        static s32 puzzle_index;

        // Update the check to use separate Width and Height half-sizes
       //  Horizontal Movement
        if (!check_player_collision(nextX, *move_y, halfSize) &&
            !player_collide_puzzles(nextX, *move_y, halfSize, puzzle_index)) {
            // If the path is totally clear, just move there.
            *move_x = nextX;
        }
        else {
            // Path is blocked! Move 1 pixel at a time until player is touching the wall.
            // if nextX is greater than current pos, +1 if lesser -1
            float direction = (nextX > *move_x) ? 1.0f : -1.0f;

            // While the NEXT pixel is clear, keep moving
            //player collision against walls
            if (!check_player_collision(*move_x + direction, *move_y, halfSize) &&
                !player_collide_puzzles(*move_x + direction, *move_y, halfSize, puzzle_index)) {
                *move_x += direction;                
            }
            //player collision against puzzles
            if (player_collide_puzzles(nextX, *move_y, halfSize, puzzle_index)) {
                move_puzzle(puzzle_index, (direction > 0.0f) ? RIGHT : LEFT, curr_elem);
            }
        }

        // Vertical movement
        if (!check_player_collision(*move_x, nextY, halfSize) &&
            !player_collide_puzzles(*move_x, nextY, halfSize, puzzle_index)) {
            // Path is clear
            *move_y = nextY;
        }
        else {
            // Path is blocked! Move 1 pixel at a time
            float direction = (nextY > *move_y) ? 1.0f : -1.0f;

            if (!check_player_collision(*move_x, *move_y + direction, halfSize) &&
                !player_collide_puzzles(*move_x, *move_y + direction, halfSize, puzzle_index)) {
                *move_y += direction;
            }
            if (player_collide_puzzles(*move_x, nextY, halfSize, puzzle_index)) {
                move_puzzle(puzzle_index, (direction > 0.0f) ? DOWN : UP, curr_elem);
            }
        }

        //DRAGGING OBJECT TEST
        if (AEInputCheckCurr(AEVK_SPACE)) {
            drag_puzzle(*move_x, *move_y, player_ptr->config.character_size / 2.0f, is_dragging, puzzle_index, curr_elem);
        }
        else {
            is_dragging = false;
        }
    }
    // --- SHOOTING LOGIC (CHARACTER 2 ONLY) ---
    if (player_ptr->current_character == 2 && AEInputCheckTriggered(AEVK_F)) {
        Projectile new_fire;
        new_fire.active = true;
        new_fire.radius = 8.0f;

        f32 spawn_offset = 30.0f;
        new_fire.x = player_ptr->char2_x + (player_ptr->last_dir_x * spawn_offset);
        new_fire.y = player_ptr->char2_y + (player_ptr->last_dir_y * spawn_offset);

        f32 shotSpeed = 600.0f;
        new_fire.vx = player_ptr->last_dir_x * shotSpeed;
        new_fire.vy = player_ptr->last_dir_y * shotSpeed;

        player_ptr->projectiles.push_back(new_fire);
    }

}

void player_system_render(player* player_ptr) {
    if (!player_ptr) return;

    //// Setup for textured rendering (characters)
    //AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    //AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    //AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    //AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    //AEGfxSetTransparency(1.0f);

    // Draw character 1
    render_sprite(rect_mesh_center, player_ptr->char1_x, player_ptr->char1_y,
        player_ptr->config.character_size, player_ptr->config.character_size, TileTextures[4]);
    
    // Draw character 2 
    render_sprite(rect_mesh_center, player_ptr->char2_x, player_ptr->char2_y,
        player_ptr->config.character_size, player_ptr->config.character_size, TileTextures[3]);

    // Calculate triangle position above active character
    f32 tri_x, tri_y;
    if (player_ptr->current_character == 1) {
        tri_x = player_ptr->char1_x;
        tri_y = player_ptr->char1_y + player_ptr->config.triangle_offset;
    }
    else {
        tri_x = player_ptr->char2_x;
        tri_y = player_ptr->char2_y + player_ptr->config.triangle_offset;
    }

    // Setup for colored rendering (triangle indicator)
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    // 2. Draw Fireballs (Explicitly set color here!)
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(1.0f, 0.5f, 0.0f, 1.0f); // Bright Orange
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    for (const auto& p : player_ptr->projectiles) {
        if (p.active) {
            draw_circle(circle_mesh, p.x, p.y, p.radius * 2.0f, p.radius * 2.0f);
        }
    }

    // Draw green triangle pointing DOWN above active character
    AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    draw_triangle(tri_x, tri_y,
        player_ptr->config.triangle_size,
        player_ptr->config.triangle_size);
}

void player_system_update(player* player_ptr) {
    if (!player_ptr) return;
    f32 dt = (f32)AEFrameRateControllerGetFrameTime();
    ///
    //
    player_system_handle_input(player_ptr);
    // Get frame time for smooth movement
    
    for (size_t i = 0; i < player_ptr->projectiles.size(); ++i) {
        Projectile& p = player_ptr->projectiles[i];

        p.x += p.vx * dt;
        p.y += p.vy * dt;
        // Check if fireball hits wall
        if (check_projectile_collision(p.x, p.y, p.radius)) {
            p.active = false;
            //debug
            printf("Projectile Reset: Hit Wall at (%.2f, %.2f)\n", p.x, p.y);
        }
        //---------------------------------------------
        // ADD COLLISION FOR ENEMY HERE!!!!!!!!!!!!!!
        //---------------------------------------------

        // Clean up inactive projectiles to save memory
        if (!p.active) {
            player_ptr->projectiles.erase(player_ptr->projectiles.begin() + i);
            i--; // Adjust index after erase
        }
    }
    player_system_update_transforms(player_ptr);

    // Update current position for reference
    if (player_ptr->current_character == 1) {
        player_ptr->x = player_ptr->char1_x;
        player_ptr->y = player_ptr->char1_y;
    }
    else {
        player_ptr->x = player_ptr->char2_x;
        player_ptr->y = player_ptr->char2_y;
    }
}

void player_system_free(player* player_ptr) {
    if (!player_ptr) return;

    // REMOVED mesh freeing (meshes are global in render.cpp)
    // ONLY free textures

    // Unload textures
    if (player_ptr->char1_texture) {
        AEGfxTextureUnload(player_ptr->char1_texture);
        player_ptr->char1_texture = NULL;
    }

    if (player_ptr->char2_texture) {
        AEGfxTextureUnload(player_ptr->char2_texture);
        player_ptr->char2_texture = NULL;
    }
}