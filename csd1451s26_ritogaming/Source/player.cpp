/*!************************************************************************
\file:          player.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         Implements the player system using AEEngine.

                Responsibilities of this system:
                - Manages two switchable player characters
                - Processes player input and movement
                - Updates transformation matrices
                - Renders characters and the active-player indicator
                - Handles initialization and cleanup of player resources

                Refactor note:
                Each character is stored in its own Character struct, and
                a current_player pointer is used to reference the active one.
                This avoids duplicate logic and keeps the system modular.
**************************************************************************/

#include "collision.hpp"
#include "player.hpp"
#include <crtdbg.h>
#include "camera.hpp"
#include "renderer.hpp"
#include "enemy_manager.hpp"
#include "puzzle.hpp"

extern int map[TILE_ROWS][TILE_COLS];
extern std::vector<Wall> g_walls;
Player player_instance;

/**************************************************************************/
/*!
  Updates transformation matrices for both characters and the active
  player indicator triangle.
*/
/**************************************************************************/
static void player_system_update_transforms(Player *player_ptr)
{
    if (!player_ptr)
        return;

    // --- Character 1 transform ---
    // Apply rotation and scale, then translate to world position
    AEMtx33 translate1;
    AEMtx33Trans(&translate1, player_ptr->char1.x, player_ptr->char1.y);
    AEMtx33Concat(&player_ptr->char1.transform, &player_ptr->rotate, &player_ptr->character_scale);
    AEMtx33Concat(&player_ptr->char1.transform, &translate1, &player_ptr->char1.transform);

    // --- Character 2 transform ---
    AEMtx33 translate2;
    AEMtx33Trans(&translate2, player_ptr->char2.x, player_ptr->char2.y);
    AEMtx33Concat(&player_ptr->char2.transform, &player_ptr->rotate, &player_ptr->character_scale);
    AEMtx33Concat(&player_ptr->char2.transform, &translate2, &player_ptr->char2.transform);

    // --- Active player indicator (triangle) ---
    // Positioned slightly above the currently controlled character
    AEMtx33 triangle_translate;
    AEMtx33Trans(&triangle_translate,
                 player_ptr->current_player->x,
                 player_ptr->current_player->y + player_ptr->config.triangle_offset);

    AEMtx33Concat(&player_ptr->triangle_transform, &player_ptr->rotate, &player_ptr->triangle_scale);
    AEMtx33Concat(&player_ptr->triangle_transform, &triangle_translate, &player_ptr->triangle_transform);
}

/**************************************************************************/
/*!
  Initializes the player system using the provided configuration.
*/
/**************************************************************************/
void player_system_init(Player *player_ptr, const player_config *config)
{
    if (!player_ptr)
        return;

    // Copy configuration values if provided
    if (config)
    {
        player_ptr->config = *config;
    }

    // --- Character 1 setup ---
    player_ptr->char1.x = player_ptr->config.char1_start_x;
    player_ptr->char1.y = player_ptr->config.char1_start_y;
    player_ptr->char1.speed = player_ptr->config.move_speed;
    player_ptr->char1.character_id = 1;
    player_ptr->char1.texture = nullptr; // Assigned during rendering

    // --- Character 2 setup ---
    player_ptr->char2.x = player_ptr->config.char2_start_x;
    player_ptr->char2.y = player_ptr->config.char2_start_y;
    player_ptr->char2.speed = player_ptr->config.move_speed;
    player_ptr->char2.character_id = 2;
    player_ptr->char2.texture = nullptr; // Assigned during rendering

    // Default to controlling Character 1
    player_ptr->current_player = &player_ptr->char1;

    // Precompute scale and rotation matrices
    AEMtx33Scale(&player_ptr->character_scale, player_ptr->config.character_size, player_ptr->config.character_size);
    AEMtx33Scale(&player_ptr->triangle_scale, player_ptr->config.triangle_size, player_ptr->config.triangle_size);
    AEMtx33Rot(&player_ptr->rotate, 0);

    player_ptr->char1_element = WATER;
    player_ptr->char2_element = FIRE;

    player_ptr->char1_element = WATER;
    player_ptr->char2_element = FIRE;

    // Initialize all transforms
    player_system_update_transforms(player_ptr);
}

/**************************************************************************/
/*!
  Handles player input for character switching and movement, including
  collision detection and sliding.
*/
/**************************************************************************/
void player_system_handle_input(Player *player_ptr)
{
    if (!player_ptr)
        return;

    // --- Character switching ---
    // Update which character is currently controlled
    if (AEInputCheckTriggered(AEVK_1))
    {
        player_ptr->current_player = &player_ptr->char1;
    }
    if (AEInputCheckTriggered(AEVK_2))
    {
        player_ptr->current_player = &player_ptr->char2;
    }

    // Target position before collision checks
    float nextX = player_ptr->current_player->x;
    float nextY = player_ptr->current_player->y;

    // Movement input (arrow keys + WASD)
    if (AEInputCheckCurr(AEVK_UP) || AEInputCheckCurr(AEVK_W))
        nextY += player_ptr->current_player->speed;
    if (AEInputCheckCurr(AEVK_DOWN) || AEInputCheckCurr(AEVK_S))
        nextY -= player_ptr->current_player->speed;
    if (AEInputCheckCurr(AEVK_LEFT) || AEInputCheckCurr(AEVK_A))
        nextX -= player_ptr->current_player->speed;
    if (AEInputCheckCurr(AEVK_RIGHT) || AEInputCheckCurr(AEVK_D))
        nextX += player_ptr->current_player->speed;
    // Movement - only affect active character
    f32 *move_x = NULL;
    f32 *move_y = NULL;

    if (player_ptr->current_character == 1)
    {
        move_x = &player_ptr->char1_x;
        move_y = &player_ptr->char1_y;
    }
    else
    {
        move_x = &player_ptr->char2_x;
        move_y = &player_ptr->char2_y;
    }

    Element curr_elem = (player_ptr->current_character == 1) ? player_ptr->char1_element : player_ptr->char2_element;

    if (move_x && move_y)
    {
        float nextX = *move_x;
        float nextY = *move_y;

        // to check if player is dragging a block
        static bool is_dragging;

        if (AEInputCheckCurr(AEVK_UP) || AEInputCheckCurr(AEVK_W))
        {
            nextY += is_dragging ? 1.0f : player_ptr->speed;
            player_ptr->last_dir_x = 0.0f;
            player_ptr->last_dir_y = 1.0f;
        }
        if (AEInputCheckCurr(AEVK_DOWN) || AEInputCheckCurr(AEVK_S))
        {
            nextY -= is_dragging ? 1.0f : player_ptr->speed;
            player_ptr->last_dir_x = 0.0f;
            player_ptr->last_dir_y = -1.0f;
        }
        if (AEInputCheckCurr(AEVK_LEFT) || AEInputCheckCurr(AEVK_A))
        {
            nextX -= is_dragging ? 1.0f : player_ptr->speed;
            player_ptr->last_dir_x = -1.0f;
            player_ptr->last_dir_y = 0.0f;
        }
        if (AEInputCheckCurr(AEVK_RIGHT) || AEInputCheckCurr(AEVK_D))
        {
            nextX += is_dragging ? 1.0f : player_ptr->speed;
            player_ptr->last_dir_x = 1.0f;
            player_ptr->last_dir_y = 0.0f;
        }

        // Half-size for AABB collision
        float halfSize = player_ptr->config.character_size / 2.0f;
        // store index of current puzzle being interacted with
        static s32 puzzle_index;

        // Update the check to use separate Width and Height half-sizes
        //  Horizontal Movement
        if (!check_player_collision(nextX, *move_y, halfSize) &&
            !player_collide_puzzles(nextX, *move_y, halfSize, puzzle_index))
        {
            // If the path is totally clear, just move there.
            *move_x = nextX;
        }
        else
        {
            // Path is blocked! Move 1 pixel at a time until player is touching the wall.
            // if nextX is greater than current pos, +1 if lesser -1
            float direction = (nextX > *move_x) ? 1.0f : -1.0f;

            // While the NEXT pixel is clear, keep moving
            // player collision against walls
            if (!check_player_collision(*move_x + direction, *move_y, halfSize) &&
                !player_collide_puzzles(*move_x + direction, *move_y, halfSize, puzzle_index))
            {
                *move_x += direction;
            }
            // player collision against puzzles
            if (player_collide_puzzles(nextX, *move_y, halfSize, puzzle_index))
            {
                move_puzzle(puzzle_index, (direction > 0.0f) ? RIGHT : LEFT, curr_elem);
            }
        }

        // Vertical movement
        if (!check_player_collision(*move_x, nextY, halfSize) &&
            !player_collide_puzzles(*move_x, nextY, halfSize, puzzle_index))
        {
            // Path is clear
            *move_y = nextY;
        }
        else
        {
            // Path is blocked! Move 1 pixel at a time
            float direction = (nextY > *move_y) ? 1.0f : -1.0f;

            if (!check_player_collision(*move_x, *move_y + direction, halfSize) &&
                !player_collide_puzzles(*move_x, *move_y + direction, halfSize, puzzle_index))
            {
                *move_y += direction;
            }
            if (player_collide_puzzles(*move_x, nextY, halfSize, puzzle_index))
            {
                move_puzzle(puzzle_index, (direction > 0.0f) ? DOWN : UP, curr_elem);
            }
        }

        // DRAGGING OBJECT TEST
        if (AEInputCheckCurr(AEVK_SPACE))
        {
            drag_puzzle(*move_x, *move_y, player_ptr->config.character_size / 2.0f, is_dragging, puzzle_index, curr_elem);
        }
        else
        {
            is_dragging = false;
        }
    }
    // --- SHOOTING LOGIC (CHARACTER 2 ONLY) ---
    if (player_ptr->current_character == 2 && AEInputCheckTriggered(AEVK_F))
    {
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

void player_system_render(player *player_ptr)
{
    if (!player_ptr)
        return;

    // Draw Character 1
    render_sprite(rect_mesh_center, player_ptr->char1.x, player_ptr->char1.y,
                  player_ptr->config.character_size, player_ptr->config.character_size, TileTextures[4]);

    // Draw Character 2
    render_sprite(rect_mesh_center, player_ptr->char2.x, player_ptr->char2.y,
                  player_ptr->config.character_size, player_ptr->config.character_size, TileTextures[3]);

    // Position the indicator triangle above the active player
    f32 tri_x = player_ptr->current_player->x;
    f32 tri_y = player_ptr->current_player->y + player_ptr->config.triangle_offset;

    // Set render state for colored shape drawing
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    // 2. Draw Fireballs (Explicitly set color here!)
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(1.0f, 0.5f, 0.0f, 1.0f); // Bright Orange
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    for (const auto &p : player_ptr->projectiles)
    {
        if (p.active)
        {
            draw_circle(circle_mesh, p.x, p.y, p.radius * 2.0f, p.radius * 2.0f);
        }
    }

    // 2. Draw Fireballs (Explicitly set color here!)
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(1.0f, 0.5f, 0.0f, 1.0f); // Bright Orange
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    for (const auto &p : player_ptr->projectiles)
    {
        if (p.active)
        {
            draw_circle(circle_mesh, p.x, p.y, p.radius * 2.0f, p.radius * 2.0f);
        }
    }

    // Green triangle marks the currently controlled character
    AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    draw_triangle(tri_x, tri_y,
                  player_ptr->config.triangle_size,
                  player_ptr->config.triangle_size);
}

/**************************************************************************/
/*!
  Updates the player system each frame.
*/
/**************************************************************************/
void player_system_update(Player *player_ptr)
{
    if (!player_ptr)
        return;
    f32 dt = (f32)AEFrameRateControllerGetFrameTime();
    ///
    //
    player_system_handle_input(player_ptr);
    // Get frame time for smooth movement

    for (size_t i = 0; i < player_ptr->projectiles.size(); ++i)
    {
        Projectile &p = player_ptr->projectiles[i];

        p.x += p.vx * dt;
        p.y += p.vy * dt;
        // Check if fireball hits wall
        if (check_projectile_collision(p.x, p.y, p.radius))
        {
            p.active = false;
            // debug
            printf("Projectile Reset: Hit Wall at (%.2f, %.2f)\n", p.x, p.y);
        }
        //---------------------------------------------
        // ADD COLLISION FOR ENEMY HERE!!!!!!!!!!!!!!
        //---------------------------------------------

        // Clean up inactive projectiles to save memory
        if (!p.active)
        {
            player_ptr->projectiles.erase(player_ptr->projectiles.begin() + i);
            i--; // Adjust index after erase
        }
    }
    player_system_update_transforms(player_ptr);
}

/**************************************************************************/
/*!
  Frees resources used by the player system.
*/
/**************************************************************************/
void player_system_free(Player *player_ptr)
{
    if (!player_ptr)
        return;

    // Unload character textures if they were loaded individually
    if (player_ptr->char1.texture)
    {
        AEGfxTextureUnload(player_ptr->char1.texture);
        player_ptr->char1.texture = nullptr;
    }

    if (player_ptr->char2.texture)
    {
        AEGfxTextureUnload(player_ptr->char2.texture);
        player_ptr->char2.texture = nullptr;
    }

    // Clear active player reference
    player_ptr->current_player = nullptr;
}
