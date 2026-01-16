/*!************************************************************************
\file:			player.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:		    This header declares the Player structure and public
                functions for player initialization, input handling,
                updating, rendering, and cleanup using AEEngine.
**************************************************************************/
#pragma once

#include "AEEngine.h"
#include "renderer.hpp"

typedef struct player_config {
    // Size settings
    float characterSize = 200.f;      // Default character size
    float triangleSize = 80.f;        // Default triangle indicator size
    float triangleOffset = 150.f;     // How far above character the triangle sits

    // Position settings
    float char1StartX = -200.0f;      // Character 1 starting position
    float char1StartY = 0.0f;
    float char2StartX = 200.0f;       // Character 2 starting position  
    float char2StartY = 0.0f;

    // Movement settings
    float moveSpeed = 30.0f;          // Player movement speed

    // Texture paths (with YOUR corrected paths!)
    const char* char1TexturePath = "Assets/kenney_tiny-dungeon/Tiles/tile_0096.png";
    const char* char2TexturePath = "Assets/kenney_tiny-dungeon/Tiles/tile_0084.png";
} PlayerConfig;

typedef struct Player {
    player_config config;

    // Position
    float x, y;

    // Character properties
    int currentCharacter;   // 1 = character 1, 2 = character 2
    float speed;

    // Separate positions for each character
    float char1X, char1Y;
    float char2X, char2Y;

    // Meshes and textures
    AEGfxVertexList* squareMesh;
    AEGfxVertexList* triangleMesh;
    AEGfxTexture* char1Texture;
    AEGfxTexture* char2Texture;

    // Transform matrices
    AEMtx33 transform1;
    AEMtx33 transform2;
    AEMtx33 triangleTransform;

    // Scale and rotation matrices
    AEMtx33 characterScale;
    AEMtx33 triangleScale;
    AEMtx33 rotate;
} Player;

/**************************************************************************
    \brief
    Initializes the player system.

    Sets up initial positions, creates meshes, loads textures,
    initializes transformation matrices, and prepares the player
    for use in the game.

    \param player Pointer to the Player structure to initialize.
**************************************************************************/
void player_system_initialize(Player* player, const player_config *config = nullptr);

/**************************************************************************
    \brief
    Handles player input and character switching.

    Processes keyboard input to move the currently active character
    and allows switching between characters.

    \param player Pointer to the Player structure.
**************************************************************************/
void player_system_handle_input(Player* player);

/**************************************************************************
    \brief
    Updates the player state.

    Updates player input, transformation matrices, and the active
    character position for the current frame.

    \param player Pointer to the Player structure.
**************************************************************************/
void player_system_update(Player* player);

/**************************************************************************
    \brief
    Renders the player characters and active character indicator.

    Draws both characters using textures and displays a visual
    indicator above the currently active character.

    \param player Pointer to the Player structure.
**************************************************************************/
void player_system_render(Player* player);

/**************************************************************************
    \brief
    Frees player resources.

    Releases meshes, unloads textures, and cleans up all resources
    associated with the player system.

    \param player Pointer to the Player structure.
**************************************************************************/
void player_system_free(Player* player);
