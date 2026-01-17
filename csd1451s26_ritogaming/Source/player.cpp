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


static void player_system_update_transforms(Player* player) {
    if (!player) return;

    // Update character 1 transform
    AEMtx33 translate1;
    AEMtx33Trans(&translate1, player->char1X, player->char1Y);
    AEMtx33Concat(&player->transform1, &player->rotate, &player->characterScale);
    AEMtx33Concat(&player->transform1, &translate1, &player->transform1);

    // Update character 2 transform
    AEMtx33 translate2;
    AEMtx33Trans(&translate2, player->char2X, player->char2Y);
    AEMtx33Concat(&player->transform2, &player->rotate, &player->characterScale);
    AEMtx33Concat(&player->transform2, &translate2, &player->transform2);

    // Update triangle position (just above the character)
    AEMtx33 triangleTranslate;
    if (player->currentCharacter == 1) {
        // Position triangle just above character 1 - USING CONFIG OFFSET
        AEMtx33Trans(&triangleTranslate, player->char1X, player->char1Y + player->config.triangleOffset);
    }
    else {
        // Position triangle just above character 2 - USING CONFIG OFFSET
        AEMtx33Trans(&triangleTranslate, player->char2X, player->char2Y + player->config.triangleOffset);
    }

    AEMtx33Concat(&player->triangleTransform, &player->rotate, &player->triangleScale);
    AEMtx33Concat(&player->triangleTransform, &triangleTranslate, &player->triangleTransform);
}

void player_system_initialize(Player* player, const player_config* config) {
    if (!player) return;

    // Store configuration 
    if (config) {
        player->config = *config;
    }

    player->char1X = player->config.char1StartX;
    player->char1Y = player->config.char1StartY;
    player->char2X = player->config.char2StartX;
    player->char2Y = player->config.char2StartY;

    // Current position (active character)
    player->x = player->char1X;
    player->y = player->char1Y;

    // Player properties FROM CONFIG
    player->currentCharacter = 1;
    player->speed = player->config.moveSpeed;

    // Load textures FROM CONFIG PATHS
    player->char1Texture = AEGfxTextureLoad(player->config.char1TexturePath);
    player->char2Texture = AEGfxTextureLoad(player->config.char2TexturePath);

    // Initialize scale matrices FROM CONFIG
    AEMtx33Scale(&player->characterScale, player->config.characterSize, player->config.characterSize);
    AEMtx33Scale(&player->triangleScale, player->config.triangleSize, player->config.triangleSize);
    AEMtx33Rot(&player->rotate, 0);

    // Initialize transform matrices
    player_system_update_transforms(player);
}

void player_system_handle_input(Player* player) {
    if (!player) return;

    // Character swapping
    if (AEInputCheckTriggered(AEVK_1)) {
        player->currentCharacter = 1;
    }
    if (AEInputCheckTriggered(AEVK_2)) {
        player->currentCharacter = 2;
    }

    // Movement - only affect active character
    float* moveX = NULL;
    float* moveY = NULL;

    if (player->currentCharacter == 1) {
        moveX = &player->char1X;
        moveY = &player->char1Y;
    }
    else {
        moveX = &player->char2X;
        moveY = &player->char2Y;
    }

    if (moveX && moveY) {
        if (AEInputCheckCurr(AEVK_UP) || AEInputCheckCurr(AEVK_W)) {
            *moveY += player->speed;
        }
        if (AEInputCheckCurr(AEVK_DOWN) || AEInputCheckCurr(AEVK_S)) {
            *moveY -= player->speed;
        }
        if (AEInputCheckCurr(AEVK_LEFT) || AEInputCheckCurr(AEVK_A)) {
            *moveX -= player->speed;
        }
        if (AEInputCheckCurr(AEVK_RIGHT) || AEInputCheckCurr(AEVK_D)) {
            *moveX += player->speed;
        }
    }
}

void player_system_render(Player* player) {
    if (!player) return;

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

    //// Setup for colored rendering (triangle indicator)
    //AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    //AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    //AEGfxSetTransparency(1.0f);

    //// Draw green triangle pointing DOWN above active character
    //AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 1.0f);
    //AEGfxSetTransform(player->triangleTransform.m);
    //AEGfxMeshDraw(triangleMesh, AE_GFX_MDM_TRIANGLES);
}


void player_system_update(Player* player) {
    if (!player) return;

    player_system_handle_input(player);
    player_system_update_transforms(player);

    // Update current position for reference
    if (player->currentCharacter == 1) {
        player->x = player->char1X;
        player->y = player->char1Y;
    }
    else {
        player->x = player->char2X;
        player->y = player->char2Y;
    }
}

void player_system_free(Player* player) {
    if (!player) return;

    // REMOVED mesh freeing (meshes are global in renderer.cpp)
    // ONLY free textures

    // Unload textures
    if (player->char1Texture) {
        AEGfxTextureUnload(player->char1Texture);
        player->char1Texture = NULL;
    }

    if (player->char2Texture) {
        AEGfxTextureUnload(player->char2Texture);
        player->char2Texture = NULL;
    }
}