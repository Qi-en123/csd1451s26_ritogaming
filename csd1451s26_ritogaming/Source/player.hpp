/*!************************************************************************
\file:			player.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\
\co-authors:    Jerod (10% - player puzzle collision ; found in player movement function)
\               Ahmad (10% - projectile for character 1 & 2 ; separated out by Claire into different functions)
\
\brief:		    Declares the player system structures and functions.
\               Defines PlayerStats, Character, Projectile, and Particle
\               structures. Provides interface for player initialization,
\               input handling, updating, rendering, and resource cleanup.
\               Supports two characters with different elements, health
\               tracking, projectile shooting, and particle effects.
**************************************************************************/
#pragma once

#include <vector>
#include <string>
#include "AEEngine.h"
#include "puzzle.hpp"

// ------------------------------------------------------------------------
//
// Represents a projectile (fireball) shot by the player.
// Stores position, velocity, size, and active state.
//
// ------------------------------------------------------------------------
struct Projectile
{
    f32 x, y;
    f32 vx, vy;
    f32 radius;
    bool active;
};

// ------------------------------------------------------------------------
//
// Represents a melee swing arc for the knight.
// Handles swing progress, direction, hit detection, and duration.
//
// ------------------------------------------------------------------------
struct SwingArc {
    bool active = false;
    bool has_hit = false;
    f32 progress =   0.0f;
    f32 dir = 1.0f;
    f32 radius = 45.0f;
    f32 duration = 0.10f;
};

// ------------------------------------------------------------------------
//
// Represents a particle used for visual effects (e.g. fire trails).
// Contains movement, lifetime, and size data.
//
// ------------------------------------------------------------------------
struct Particle {
    f32 x, y;
    f32 vx, vy;
    f32 life;     // Current life (seconds)
    f32 maxLife;  // Total duration
    f32 size;
    bool active;
};

// ------------------------------------------------------------------------
//
// Stores all configurable player stats such as size, position,
// movement speed, and health values for both characters.
//
// ------------------------------------------------------------------------
struct PlayerStats
{
    // Size settings
    f32 character_size;
    f32 triangle_size;
    f32 triangle_offset;

    // Position settings
    f32 char1_start_x;
    f32 char1_start_y;
    f32 char2_start_x;
    f32 char2_start_y;

    // Movement settings
    f32 char1_move_speed = 10.f;  // knight - slightly slower
    f32 char2_move_speed = 13.f;  // mage   - faster
    //f32 move_speed = 4.f; // Player movement speed (4.f) makes it look more in sync with audio.

    // HP settings
    s32 char1_max_health;
    s32 char1_curr_health;
    s32 char2_max_health;
    s32 char2_curr_health;
};

// ------------------------------------------------------------------------
//
// Represents an individual player character.
// Stores position, movement, rendering data, and state.
//
// ------------------------------------------------------------------------
struct Character
{
    f32 x{}, y{}; // Position
    f32 speed{};  // Movement speed

    AEGfxTexture* texture{};      // Character texture
	AEGfxTexture* dead_texture{}; // Texture to show when character is "dead" (0 health)
    AEMtx33 transform{};          // Transform matrix

    s32 character_id{}; // 1 or 2 for identification
    Puzzle::Element element{};  // Character's element (WATER, FIRE, etc.)
	bool is_dead{};     // Flag to indicate if character is dead

	f32 hit_flash_timer = 0.f; // Timer for damage flash effect
};

// ------------------------------------------------------------------------
//
// Main player system container.
// Holds both characters, shared resources, projectiles,
// particles, and gameplay-related state.
//
// ------------------------------------------------------------------------
struct Player
{
    PlayerStats stats{};
    SwingArc swing;

    // Individual characters
    Character char1{};
    Character char2{};

    // Pointer to currently active character - EASY REFERENCE!
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

    s32 fireball_count{ 5 };
    f32 shoot_cooldown{ 0.0f };

    // Last movement direction for shooting
    f32 last_dir_x{}, last_dir_y{};

	// Damage flash timer for player hit feedback
    static constexpr f32 HIT_FLASH_DUR = 0.15f;
};

// Global player instance
extern Player player_instance;

/**************************************************************************
    \brief
    Saves player configuration (default stats) to a JSON file.

    \param player_ptr Pointer to the player structure.
    \param filepath Path to the JSON file.
    \return True if successful, false otherwise.
**************************************************************************/
bool player_save(const Player* player, const std::string& filepath);

/**************************************************************************
    \brief
    Loads player configuration (default stats) from a JSON file.

    \param player_ptr Pointer to the player structure.
    \param filepath Path to the JSON file.
    \return True if successful, false otherwise.
**************************************************************************/
bool player_load(Player* player, const std::string& filepath);

/**************************************************************************
    \brief
    Saves current player state (positions, health, active character) 
    to a JSON file.

    \param player_ptr Pointer to the player structure.
    \param filepath Path to the JSON file.
	\return True if successful, false otherwise.    
**************************************************************************/
bool player_save_config(const Player* player, const std::string& filepath);

/**************************************************************************
    \brief
    Loads player state (positions, health, active character) from a JSON file.

    \param player_ptr Pointer to the player structure.
    \param filepath Path to the JSON file.
    \return True if successful, false otherwise.
**************************************************************************/
bool player_load_config(Player* player, const std::string& filepath);

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
void player_update_projectiles(Player* player_ptr);

/**************************************************************************
    \brief
    Handles shooting logic for character 2.

    \param player_ptr Pointer to the player structure.
**************************************************************************/
void player_handle_shooting(Player* player_ptr);

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
void player_take_damage(Player* player_ptr, s32 damage);

/**************************************************************************
    \brief
    Returns true if both characters have no health remaining.
**************************************************************************/
bool player_is_dead(Player* player_ptr);

// ------------------------------------------------------------------------
//
// Spawns player characters based on map tile data.
// Converts tile coordinates into world positions.
//
// player_ptr - player to initialize positions for
//
// ------------------------------------------------------------------------
void spawn_player_from_map(Player* player_ptr);

// ------------------------------------------------------------------------
//
// Handles melee attack input for character 1 (knight).
//
// ------------------------------------------------------------------------
void player_handle_melee(Player* player_ptr); 

// ------------------------------------------------------------------------
//
// Updates melee swing animation and hit detection.
//
// ------------------------------------------------------------------------
void player_update_swing(Player* player_ptr);

// ------------------------------------------------------------------------
//
// Renders melee swing visual effect.
//
// ------------------------------------------------------------------------
void player_render_swing(Player* player_ptr);