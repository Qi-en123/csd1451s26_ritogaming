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
    player_ptr->char1_texture = AEGfxTextureLoad(player_ptr->config.char1_texture_path);
    player_ptr->char2_texture = AEGfxTextureLoad(player_ptr->config.char2_texture_path);

    // Initialize scale matrices FROM CONFIG
    AEMtx33Scale(&player_ptr->character_scale, player_ptr->config.character_size, player_ptr->config.character_size);
    AEMtx33Scale(&player_ptr->triangle_scale, player_ptr->config.triangle_size, player_ptr->config.triangle_size);
    AEMtx33Rot(&player_ptr->rotate, 0);

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
   
    if (move_x && move_y) {
        float nextX = *move_x;
        float nextY = *move_y;

        if (AEInputCheckCurr(AEVK_UP) || AEInputCheckCurr(AEVK_W))
            nextY += player_ptr->speed;
        if (AEInputCheckCurr(AEVK_DOWN) || AEInputCheckCurr(AEVK_S))
            nextY -= player_ptr->speed;
        if (AEInputCheckCurr(AEVK_LEFT) || AEInputCheckCurr(AEVK_A))
            nextX -= player_ptr->speed;
        if (AEInputCheckCurr(AEVK_RIGHT) || AEInputCheckCurr(AEVK_D))
            nextX += player_ptr->speed;

        // Half-size for AABB collision
        float halfSize = player_ptr->config.character_size / 2.0f;

        // Check collision separately for X and Y
        if (!check_player_collision(*move_x, nextY, halfSize))
            *move_y = nextY;
        if (!check_player_collision(nextX, *move_y, halfSize))
            *move_x = nextX;
    }



}

void player_system_render(player* player_ptr) {
    if (!player_ptr) return;

    // Setup for textured rendering (characters)
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    // Draw Character 1 - SIMPLER APPROACH
    // Camera is already set by AEEngine, so just draw at world coordinates
    AEMtx33 transform1;
    AEMtx33Identity(&transform1);

    AEMtx33 translate1;
    AEMtx33Trans(&translate1, player_ptr->char1_x, player_ptr->char1_y);

    AEMtx33 scale1;
    AEMtx33Scale(&scale1, player_ptr->config.character_size, player_ptr->config.character_size);

    // Apply transformations: Translate THEN Scale
    AEMtx33Concat(&transform1, &translate1, &scale1);

    AEGfxSetTransform(transform1.m);
    AEGfxTextureSet(player_ptr->char1_texture, 0, 0);
    AEGfxMeshDraw(square_mesh, AE_GFX_MDM_TRIANGLES);

    // Draw Character 2
    AEMtx33 transform2;
    AEMtx33Identity(&transform2);

    AEMtx33 translate2;
    AEMtx33Trans(&translate2, player_ptr->char2_x, player_ptr->char2_y);

    AEMtx33 scale2;
    AEMtx33Scale(&scale2, player_ptr->config.character_size, player_ptr->config.character_size);

    AEMtx33Concat(&transform2, &translate2, &scale2);

    AEGfxSetTransform(transform2.m);
    AEGfxTextureSet(player_ptr->char2_texture, 0, 0);
    AEGfxMeshDraw(square_mesh, AE_GFX_MDM_TRIANGLES);

    // Setup for colored rendering (triangle indicator)
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

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

    // Draw triangle
    AEMtx33 tri_transform;
    AEMtx33Identity(&tri_transform);

    AEMtx33 tri_translate;
    AEMtx33Trans(&tri_translate, tri_x, tri_y);

    AEMtx33 tri_scale;
    AEMtx33Scale(&tri_scale, player_ptr->config.triangle_size, player_ptr->config.triangle_size);

    AEMtx33Concat(&tri_transform, &tri_translate, &tri_scale);

    AEGfxSetTransform(tri_transform.m);
    AEGfxMeshDraw(triangle_mesh, AE_GFX_MDM_TRIANGLES);
}


void player_system_update(player* player_ptr) {
    if (!player_ptr) return;

    player_system_handle_input(player_ptr);
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