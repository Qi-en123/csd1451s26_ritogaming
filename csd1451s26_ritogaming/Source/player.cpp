/*!************************************************************************
\file:          player.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         This file implements the player system using AEEngine.
                It manages two switchable characters, handles player input,
                updates transformations, renders player visuals and indicators,
                and handles resource initialization and cleanup.
**************************************************************************/
#include "player.hpp"
#include <crtdbg.h>
#include "camera.hpp"
#include "renderer.hpp"


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
        if (AEInputCheckCurr(AEVK_W)) {
            *move_y += player_ptr->speed;
        }
        if (AEInputCheckCurr(AEVK_S)) {
            *move_y -= player_ptr->speed;
        }
        if (AEInputCheckCurr(AEVK_A)) {
            *move_x -= player_ptr->speed;
        }
        if (AEInputCheckCurr(AEVK_D)) {
            *move_x += player_ptr->speed;
        }
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

    //// Draw Character 1
    //AEGfxSetTransform(player->transform1.m);
    //AEGfxTextureSet(player->char1Texture, 0, 0);
    //AEGfxMeshDraw(squareMesh, AE_GFX_MDM_TRIANGLES);

    renderSprite(rectMeshCenter, player->char1X, player->char1Y,
        player->config.characterSize, player->config.characterSize, player->char1Texture);

    //// Draw Character 2
    //AEGfxSetTransform(player->transform2.m);
    //AEGfxTextureSet(player->char2Texture, 0, 0);
    //AEGfxMeshDraw(squareMesh, AE_GFX_MDM_TRIANGLES);

    renderSprite(rectMeshCenter, player->char2X, player->char2Y,
        player->config.characterSize, player->config.characterSize, player->char2Texture);

    // Setup for colored rendering (triangle indicator)
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    // Draw green triangle pointing DOWN above active character
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