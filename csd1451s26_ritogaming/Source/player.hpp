/*!************************************************************************
\file:			player.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\
\co-authors:    Jerod (10% - player puzzle collision ; found in player movement function)
\               Ahmad (10% - projectile for player 2 ; seprated out by claire into different functions)
\
\brief:		    This header declares the player structure and public
                functions for player initialization, input handling,
                updating, rendering, and cleanup using AEEngine.
**************************************************************************/
#pragma once

#include <vector>
#include "AEEngine.h"
#include "puzzle.hpp"

struct Projectile
{
    f32 x, y;
    f32 vx, vy;
    f32 radius;
    bool active;
};


struct Particle {
    f32 x, y;
    f32 vx, vy;
    f32 life;     // Current life (seconds)
    f32 maxLife;  // Total duration
    f32 size;
    bool active;
};

struct PlayerStats
{
    // Size settings
    f32 character_size = 25.f;  // Default character size
    f32 triangle_size = 20.f;   // Default triangle indicator size
    f32 triangle_offset = 40.f; // How far above character the triangle sits

    // Position settings
    f32 char1_start_x = -400.0f;      // Character 1 starting position
    f32 char1_start_y = 0.0f;

    f32 char2_start_x = 1000.0f;       // Character 2 starting position  
    f32 char2_start_y = -950.0f;

    // Movement settings
    f32 move_speed = 13.f; // Player movement speed

    // This is for hp
    int char1_max_health{ 6 };
    int char1_curr_health{6};

    int char2_max_health{ 6 };
    int char2_curr_health{6};
};

// Individual character structure
struct Character
{
    f32 x{}, y{}; // Position
    f32 speed{};  // Movement speed

    AEGfxTexture* texture{};      // Character texture
	AEGfxTexture* dead_texture{}; // Texture to show when character is "dead" (0 health)
    AEMtx33 transform{};          // Transform matrix

    s32 character_id{}; // 1 or 2 for identification
    Element element{};  // Character's element (WATER, FIRE, etc.)
	bool is_dead{};     // Flag to indicate if character is dead
};


// Player system structure
struct Player
{
	PlayerStats stats{};

	// Individual characters
	Character char1{};
	Character char2{};

	Character* current_player{};
	s32 current_character{ 1 }; // 1 or 2

	// Shared resources
	AEGfxVertexList* square_mesh{};
	AEGfxVertexList* triangle_mesh{};

	// Shared transformation matrices
	AEMtx33 character_scale{};
	AEMtx33 triangle_scale{};
	AEMtx33 triangle_transform{};
	AEMtx33 rotate{};

    // Projectile system
    std::vector<Projectile> projectiles;
    std::vector<Particle> particles;

    int fireball_count{ 5 };
    f32 shoot_cooldown{ 0.0f };

	// Last movement direction for shooting
	f32 last_dir_x{}, last_dir_y{};
};

// Global player instance
extern Player player_instance;

/**************************************************************************
    \brief
    Initializes the player system.

    Sets up initial positions for both characters, creates meshes, loads
    textures, initializes transformation matrices, and sets the current
    player pointer to character 1.

    \param player_ptr Pointer to the player structure to initialize.
    \param config Optional configuration override (uses defaults if nullptr).
**************************************************************************/
void player_system_init(Player* player_ptr, AEGfxTexture* char_1_texture, 
                        AEGfxTexture* char_2_texture, AEGfxTexture* dead_texture);

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

/**************************************************************************
    \brief
    Updates projectile positions and handles collisions.

    \param player_ptr Pointer to the player structure.
    \param dt Delta time for frame-independent movement.
**************************************************************************/
void player_update_projectiles(Player* player_ptr, f32 dt);

/**************************************************************************
    \brief
    Handles shooting logic for character 2.

    \param player_ptr Pointer to the player structure.
**************************************************************************/
void player_handle_shooting(Player* player_ptr, f32 dt);

/**************************************************************************
    \brief
    Renders all active projectiles.

    \param player_ptr Pointer to the player structure.
**************************************************************************/
void player_render_projectiles(Player* player_ptr);

/**************************************************************************
    \brief
    Applies damage to the currently active character following the
    half-heart system. Damage carries over across hearts.

    \param player_ptr Pointer to the player structure.
    \param damage     Amount of damage (1 = half a heart).
**************************************************************************/
void player_take_damage(Player* player_ptr, int damage);

/**************************************************************************
    \brief
    Returns true if both characters have no health remaining.
**************************************************************************/
bool player_is_dead(Player* player_ptr);