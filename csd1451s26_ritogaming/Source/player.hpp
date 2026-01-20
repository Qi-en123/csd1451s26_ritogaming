/*!************************************************************************
\file:			player.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:		    This header declares the player structure and public
                functions for player initialization, input handling,
                updating, rendering, and cleanup using AEEngine.
**************************************************************************/
#pragma once

#include "AEEngine.h"
#include "renderer.hpp"

typedef struct player_config {
    // Size settings
    f32 character_size = 64.f;       // Default character size
    f32 triangle_size = 20.f;        // Default triangle indicator size
    f32 triangle_offset = 40.f;     // How far above character the triangle sits

    // Position settings
    f32 char1_start_x = 4320.f;      // Character 1 starting position
    f32 char1_start_y = -3374.f;

    f32 char2_start_x = 4120.f;       // Character 2 starting position  
    f32 char2_start_y = -3374.f;

    // Movement settings
    f32 move_speed = 25.f;          // Player movement speed
    
    // Texture paths
    const char* char1_texture_path = "Assets/kenney_tiny-dungeon/Tiles/tile_0096.png";
    const char* char2_texture_path = "Assets/kenney_tiny-dungeon/Tiles/tile_0084.png";
} player_config;

typedef struct player {
    player_config config;

    f32 x, y;

    s32 current_character;
    f32 speed;

    f32 char1_x, char1_y;
    f32 char2_x, char2_y;

    AEGfxVertexList* square_mesh;
    AEGfxVertexList* triangle_mesh;
    AEGfxTexture* char1_texture;
    AEGfxTexture* char2_texture;

    AEMtx33 transform1;
    AEMtx33 transform2;
    AEMtx33 triangle_transform;

    AEMtx33 character_scale;
    AEMtx33 triangle_scale;
    AEMtx33 rotate;
} player;

/**************************************************************************
    \brief
    Initializes the player system.

    Sets up initial positions, creates meshes, loads textures,
    initializes transformation matrices, and prepares the player
    for use in the game.

    \param player_ptr Pointer to the player structure to initialize.
**************************************************************************/
void player_system_init(player* player_ptr, const player_config* config = nullptr);

/**************************************************************************
    \brief
    Handles player input and character switching.

    Processes keyboard input to move the currently active character
    and allows switching between characters.

    \param player_ptr Pointer to the player structure.
**************************************************************************/
void player_system_handle_input(player* player_ptr);

/**************************************************************************
    \brief
    Updates the player state.

    Updates player input, transformation matrices, and the active
    character position for the current frame.

    \param player_ptr Pointer to the player structure.
**************************************************************************/
void player_system_update(player* player_ptr);

/**************************************************************************
    \brief
    Renders the player characters and active character indicator.

    Draws both characters using textures and displays a visual
    indicator above the currently active character.

    \param player_ptr Pointer to the player structure.
**************************************************************************/
void player_system_render(player* player_ptr);

/**************************************************************************
    \brief
    Frees player resources.

    Releases meshes, unloads textures, and cleans up all resources
    associated with the player system.

    \param player_ptr Pointer to the player structure.
**************************************************************************/
void player_system_free(player* player_ptr);