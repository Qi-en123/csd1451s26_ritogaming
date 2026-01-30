/*!************************************************************************
\file:			player.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:		    This header declares the player structure and public
                functions for player initialization, input handling,
                updating, rendering, and cleanup using AEEngine.

                REFACTORED: Now uses individual Character structs with
                a pointer to the current active character for easier
                reference and cleaner code.
**************************************************************************/
#pragma once

#include "AEEngine.h"
#include "renderer.hpp"
#include "map.hpp"

typedef struct player_config {
    // Size settings
    f32 character_size = 25.f;       // Default character size
    f32 triangle_size = 20.f;        // Default triangle indicator size
    f32 triangle_offset = 40.f;      // How far above character the triangle sits

    f32 char1_start_x = 2100.f;
    f32 char1_start_y = -950.f;

    f32 char2_start_x = 2300.f;      // char1_start_x + 200.f
    f32 char2_start_y = -950.f;      // char1_start_y

    // Movement settings
    f32 move_speed = 20.f;           // Player movement speed

    // Texture paths (commented out - currently using tile textures)
    //const char* char1_texture_path = "Assets/kenney_tiny-dungeon/Tiles/tile_0004.png";
    //const char* char2_texture_path = "Assets/kenney_tiny-dungeon/Tiles/tile_0003.png";
} player_config;

// Individual character structure
typedef struct Character {
    f32 x{}, y{};                    // Position
    f32 speed{};                     // Movement speed

    AEGfxTexture* texture{};         // Character texture
    AEMtx33 transform{};             // Transform matrix

    s32 character_id{};              // 1 or 2 for identification
} Character;

// Player system structure
typedef struct Player {
    player_config config{};

    // Individual characters
    Character char1{};
    Character char2{};

    // Pointer to currently active character - EASY REFERENCE!
    Character* current_player{};

    // Shared resources
    AEGfxVertexList* square_mesh{};
    AEGfxVertexList* triangle_mesh{};

    // Shared transformation matrices
    AEMtx33 character_scale{};
    AEMtx33 triangle_scale{};
    AEMtx33 triangle_transform{};
    AEMtx33 rotate{};
} Player;

/**************************************************************************
    \brief
    Initializes the player system.

    Sets up initial positions for both characters, creates meshes, loads
    textures, initializes transformation matrices, and sets the current
    player pointer to character 1.

    \param player_ptr Pointer to the player structure to initialize.
    \param config Optional configuration override (uses defaults if nullptr).
**************************************************************************/
void player_system_init(Player* player_ptr, const player_config* config = nullptr);

/**************************************************************************
    \brief
    Handles player input and character switching.

    Processes keyboard input to move the currently active character
    and allows switching between characters using keys 1 and 2.
    Updates the current_player pointer when switching.

    \param player_ptr Pointer to the player structure.
**************************************************************************/
void player_system_handle_input(Player* player_ptr);

/**************************************************************************
    \brief
    Updates the player state.

    Updates player input, transformation matrices for both characters,
    and the triangle indicator position for the current frame.

    \param player_ptr Pointer to the player structure.
**************************************************************************/
void player_system_update(Player* player_ptr);

/**************************************************************************
    \brief
    Renders the player characters and active character indicator.

    Draws both characters using textures and displays a green triangle
    indicator above the currently active character.

    \param player_ptr Pointer to the player structure.
**************************************************************************/
void player_system_render(Player* player_ptr);

/**************************************************************************
    \brief
    Frees player resources.

    Releases meshes, unloads textures, and cleans up all resources
    associated with the player system.

    \param player_ptr Pointer to the player structure.
**************************************************************************/
void player_system_free(Player* player_ptr);