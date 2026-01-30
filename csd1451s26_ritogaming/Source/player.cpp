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

extern int map[TILE_ROWS][TILE_COLS];
extern std::vector<Wall> g_walls;

/**************************************************************************/
/*!
  Updates transformation matrices for both characters and the active
  player indicator triangle.
*/
/**************************************************************************/
static void player_system_update_transforms(Player* player_ptr) {
    if (!player_ptr) return;

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
void player_system_init(Player* player_ptr, const player_config* config) {
    if (!player_ptr) return;

    // Copy configuration values if provided
    if (config) {
        player_ptr->config = *config;
    }

    // --- Character 1 setup ---
    player_ptr->char1.x = player_ptr->config.char1_start_x;
    player_ptr->char1.y = player_ptr->config.char1_start_y;
    player_ptr->char1.speed = player_ptr->config.move_speed;
    player_ptr->char1.character_id = 1;
    player_ptr->char1.texture = nullptr;  // Assigned during rendering

    // --- Character 2 setup ---
    player_ptr->char2.x = player_ptr->config.char2_start_x;
    player_ptr->char2.y = player_ptr->config.char2_start_y;
    player_ptr->char2.speed = player_ptr->config.move_speed;
    player_ptr->char2.character_id = 2;
    player_ptr->char2.texture = nullptr;  // Assigned during rendering

    // Default to controlling Character 1
    player_ptr->current_player = &player_ptr->char1;

    // Precompute scale and rotation matrices
    AEMtx33Scale(&player_ptr->character_scale, player_ptr->config.character_size, player_ptr->config.character_size);
    AEMtx33Scale(&player_ptr->triangle_scale, player_ptr->config.triangle_size, player_ptr->config.triangle_size);
    AEMtx33Rot(&player_ptr->rotate, 0);

    // Initialize all transforms
    player_system_update_transforms(player_ptr);
}

/**************************************************************************/
/*!
  Handles player input for character switching and movement, including
  collision detection and sliding.
*/
/**************************************************************************/
void player_system_handle_input(Player* player_ptr) {
    if (!player_ptr) return;

    // --- Character switching ---
    // Update which character is currently controlled
    if (AEInputCheckTriggered(AEVK_1)) {
        player_ptr->current_player = &player_ptr->char1;
    }
    if (AEInputCheckTriggered(AEVK_2)) {
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

    // Half-size used for AABB collision detection
    float halfSize = player_ptr->config.character_size / 2.0f;

    // --- Horizontal movement with collision sliding ---
    if (!check_player_collision(nextX, player_ptr->current_player->y, halfSize)) {
        player_ptr->current_player->x = nextX;
    }
    else {
        // Move one unit at a time until just before collision
        float direction = (nextX > player_ptr->current_player->x) ? 1.0f : -1.0f;
        while (!check_player_collision(player_ptr->current_player->x + direction,
            player_ptr->current_player->y, halfSize)) {
            player_ptr->current_player->x += direction;
        }
    }

    // --- Vertical movement with collision sliding ---
    if (!check_player_collision(player_ptr->current_player->x, nextY, halfSize)) {
        player_ptr->current_player->y = nextY;
    }
    else {
        float direction = (nextY > player_ptr->current_player->y) ? 1.0f : -1.0f;
        while (!check_player_collision(player_ptr->current_player->x,
            player_ptr->current_player->y + direction, halfSize)) {
            player_ptr->current_player->y += direction;
        }
    }
}

/**************************************************************************/
/*!
  Renders both characters and the indicator for the active player.
*/
/**************************************************************************/
void player_system_render(Player* player_ptr) {
    if (!player_ptr) return;

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
void player_system_update(Player* player_ptr) {
    if (!player_ptr) return;

    player_system_handle_input(player_ptr);
    player_system_update_transforms(player_ptr);
}

/**************************************************************************/
/*!
  Frees resources used by the player system.
*/
/**************************************************************************/
void player_system_free(Player* player_ptr) {
    if (!player_ptr) return;

    // Unload character textures if they were loaded individually
    if (player_ptr->char1.texture) {
        AEGfxTextureUnload(player_ptr->char1.texture);
        player_ptr->char1.texture = nullptr;
    }

    if (player_ptr->char2.texture) {
        AEGfxTextureUnload(player_ptr->char2.texture);
        player_ptr->char2.texture = nullptr;
    }

    // Clear active player reference
    player_ptr->current_player = nullptr;
}
