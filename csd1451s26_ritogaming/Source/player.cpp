/*!************************************************************************
\file:          player.cpp
\author:        Claire 
\par DP email:  c.cheong@digipen.edu
\
\co-authors:    Jerod (10% - player puzzle collision ; found in player movement function)
\               Ahmad (10% - projectile for player 2 ; seprated out by claire into different functions)
\
\brief:         Implements the player system using AEEngine.

                Responsibilities of this system:
                - Manages two switchable player characters
                - Processes player input and movement
                - Updates transformation matrices
                - Renders characters and the active-player indicator
                - Handles initialization and cleanup of player resources
**************************************************************************/

#include "pch.hpp"
#include <crtdbg.h>

extern int map[TILE_ROWS][TILE_COLS];
extern std::vector<Wall> g_walls;

#define DAMAGE 1 // change this for fireball damage

// Global player instance definition
Player player_instance;


static void player_system_update_transforms(Player* player_ptr)
{
    if (!player_ptr)
        return;

    // --- Character 1 transform ---
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
    AEMtx33 triangle_translate;
    AEMtx33Trans(&triangle_translate,
        player_ptr->current_player->x,
        player_ptr->current_player->y + player_ptr->stats.triangle_offset);

    AEMtx33Concat(&player_ptr->triangle_transform, &player_ptr->rotate, &player_ptr->triangle_scale);
    AEMtx33Concat(&player_ptr->triangle_transform, &triangle_translate, &player_ptr->triangle_transform);
}


void player_system_init(Player* player_ptr, AEGfxTexture* char_1_texture, AEGfxTexture* char_2_texture)
{
    if (!player_ptr)
        return;

    // --- Character 1 setup ---
    player_ptr->char1.x = player_ptr->stats.char1_start_x;
    player_ptr->char1.y = player_ptr->stats.char1_start_y;
    player_ptr->char1.speed = player_ptr->stats.move_speed;
    player_ptr->char1.character_id = 1;
    player_ptr->char1.element = WATER;
    player_ptr->char1.texture = char_1_texture;

    // --- Character 2 setup ---
    player_ptr->char2.x = player_ptr->stats.char2_start_x;
    player_ptr->char2.y = player_ptr->stats.char2_start_y;
    player_ptr->char2.speed = player_ptr->stats.move_speed;
    player_ptr->char2.character_id = 2;
    player_ptr->char2.element = FIRE;
    player_ptr->char2.texture = char_2_texture;

    // Default to controlling Character 1
    player_ptr->current_player = &player_ptr->char1;
    player_ptr->current_character = 1;

    // Precompute scale and rotation matrices
    AEMtx33Scale(&player_ptr->character_scale, player_ptr->stats.character_size, player_ptr->stats.character_size);
    AEMtx33Scale(&player_ptr->triangle_scale, player_ptr->stats.triangle_size, player_ptr->stats.triangle_size);
    AEMtx33Rot(&player_ptr->rotate, 0);

    // Initialize last direction for shooting
    player_ptr->last_dir_x = 1.0f;
    player_ptr->last_dir_y = 0.0f;

    // Initialize all transforms
    player_system_update_transforms(player_ptr);
}


static void player_handle_character_switch(Player* player_ptr)
{
    if (!player_ptr)
        return;

    if (AEInputCheckTriggered(AEVK_1))
    {
        player_ptr->current_player = &player_ptr->char1;
        player_ptr->current_character = 1;
    }
    if (AEInputCheckTriggered(AEVK_2))
    {
        player_ptr->current_player = &player_ptr->char2;
        player_ptr->current_character = 2;
    }
}


static void player_handle_movement(Player* player_ptr)
{
    if (!player_ptr || !player_ptr->current_player)
        return;

    f32* move_x = &player_ptr->current_player->x;
    f32* move_y = &player_ptr->current_player->y;
    Element curr_elem = player_ptr->current_player->element;

    float nextX = *move_x;
    float nextY = *move_y;
    float halfSize = player_ptr->stats.character_size / 2.0f;

    // Track if player is dragging a puzzle block
    static bool is_dragging = false;
    static s32 puzzle_index;

    // Movement input
    if (AEInputCheckCurr(AEVK_W))
    {
        nextY += is_dragging ? 1.0f : player_ptr->stats.move_speed;
        player_ptr->last_dir_x = 0.0f;
        player_ptr->last_dir_y = 1.0f;
    }
    if (AEInputCheckCurr(AEVK_S))
    {
        nextY -= is_dragging ? 1.0f : player_ptr->stats.move_speed;
        player_ptr->last_dir_x = 0.0f;
        player_ptr->last_dir_y = -1.0f;
    }
    if (AEInputCheckCurr(AEVK_A))
    {
        nextX -= is_dragging ? 1.0f : player_ptr->stats.move_speed;
        player_ptr->last_dir_x = -1.0f;
        player_ptr->last_dir_y = 0.0f;
    }
    if (AEInputCheckCurr(AEVK_D))
    {
        nextX += is_dragging ? 1.0f : player_ptr->stats.move_speed;
        player_ptr->last_dir_x = 1.0f;
        player_ptr->last_dir_y = 0.0f;
    }
    // ------------------------------------- Jerods part -------------------------------------
    // Horizontal Movement with collision
    if (!check_player_collision(nextX, *move_y, halfSize) &&
        !player_collide_puzzles(nextX, *move_y, halfSize, puzzle_index))
    {
        *move_x = nextX;
    }
    else
    {
        // Slide along wall
        float direction = (nextX > *move_x) ? 1.0f : -1.0f;
        if (!check_player_collision(*move_x + direction, *move_y, halfSize) &&
            !player_collide_puzzles(*move_x + direction, *move_y, halfSize, puzzle_index))
        {
            *move_x += direction;
        }
        // Handle puzzle pushing
        if (player_collide_puzzles(nextX, *move_y, halfSize, puzzle_index))
        {
            move_puzzle(puzzle_index, (direction > 0.0f) ? RIGHT : LEFT, curr_elem);
        }
    }

    // Vertical movement with collision
    if (!check_player_collision(*move_x, nextY, halfSize) &&
        !player_collide_puzzles(*move_x, nextY, halfSize, puzzle_index))
    {
        *move_y = nextY;
    }
    else
    {
        // Slide along wall
        float direction = (nextY > *move_y) ? 1.0f : -1.0f;
        if (!check_player_collision(*move_x, *move_y + direction, halfSize) &&
            !player_collide_puzzles(*move_x, *move_y + direction, halfSize, puzzle_index))
        {
            *move_y += direction;
        }
        // Handle puzzle pushing
        if (player_collide_puzzles(*move_x, nextY, halfSize, puzzle_index))
        {
            move_puzzle(puzzle_index, (direction > 0.0f) ? DOWN : UP, curr_elem);
        }
    }

    // Handle puzzle dragging
    if (AEInputCheckCurr(AEVK_SPACE))
    {
        drag_puzzle(*move_x, *move_y, halfSize, is_dragging, puzzle_index, curr_elem);
    }
    else
    {
        is_dragging = false;
    }
}


void player_system_handle_input(Player* player_ptr)
{
    if (!player_ptr)
        return;

    player_handle_character_switch(player_ptr);
    player_handle_movement(player_ptr);
}

//------------------------------------------ Ahmad's functions ------------------------------------------------------
void player_handle_shooting(Player* player_ptr)
{
    if (!player_ptr)
        return;

    // Only character 2 can shoot
    if (player_ptr->current_character == 2 && AEInputCheckTriggered(AEVK_F))
    {
        Projectile new_fire;
        new_fire.active = true;
        new_fire.radius = 8.0f;

        f32 spawn_offset = 30.0f;
        new_fire.x = player_ptr->char2.x + (player_ptr->last_dir_x * spawn_offset);
        new_fire.y = player_ptr->char2.y + (player_ptr->last_dir_y * spawn_offset);

        f32 shotSpeed = 600.0f;
        new_fire.vx = player_ptr->last_dir_x * shotSpeed;
        new_fire.vy = player_ptr->last_dir_y * shotSpeed;

        player_ptr->projectiles.push_back(new_fire);
    }
}


void update_player_projectiles(Player* player_ptr, f32 dt)
{
    if (!player_ptr)
        return;

    for (size_t i = 0; i < player_ptr->projectiles.size(); ++i)
    {
        Projectile& p = player_ptr->projectiles[i];

        p.x += p.vx * dt;
        p.y += p.vy * dt;

        // Check if fireball hits wall
        if (check_projectile_collision(p.x, p.y, p.radius))
        {
            p.active = false;
        }
        //
        if (p.active && enemy_manager_instance.check_projectile_hit(p.x, p.y, p.radius, DAMAGE)) {
            p.active = false; // Destroy fireball on hit
        }

        // Clean up inactive projectiles
        if (!p.active)
        {
            player_ptr->projectiles.erase(player_ptr->projectiles.begin() + i);
            i--;
        }
    }
}

/**************************************************************************/
/*!
  Renders all active projectiles.
*/
/**************************************************************************/
void player_render_projectiles(Player* player_ptr)
{
    if (!player_ptr)
        return;

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(1.0f, 0.5f, 0.0f, 1.0f); // Bright Orange
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    for (const auto& p : player_ptr->projectiles)
    {
        if (p.active)
        {
            draw_circle(p.x, p.y, p.radius * 2.0f, p.radius * 2.0f);
        }
    }
}

/**************************************************************************/
/*!
  Renders the player characters and active character indicator.
*/
/**************************************************************************/
void player_system_render(Player* player_ptr)
{
    if (!player_ptr)
        return;

    // Draw Character 1
    render_sprite(rect_mesh_center, player_ptr->char1.x, player_ptr->char1.y,
        player_ptr->stats.character_size, player_ptr->stats.character_size, player_ptr->char1.texture);

    // Draw Character 2
    render_sprite(rect_mesh_center, player_ptr->char2.x, player_ptr->char2.y,
        player_ptr->stats.character_size, player_ptr->stats.character_size, player_ptr->char2.texture);

    // Render projectiles
    player_render_projectiles(player_ptr);

    // Position the indicator triangle above the active player
    f32 tri_x = player_ptr->current_player->x;
    f32 tri_y = player_ptr->current_player->y + player_ptr->stats.triangle_offset;

    // Green triangle marks the currently controlled character
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    draw_triangle(tri_x, tri_y,
        player_ptr->stats.triangle_size,
        player_ptr->stats.triangle_size);
}

/**************************************************************************/
/*!
  Updates the player system each frame.
*/
/**************************************************************************/
void player_system_update(Player* player_ptr)
{
    if (!player_ptr)
        return;
    
    player_system_handle_input(player_ptr);
    player_handle_shooting(player_ptr);
    player_system_update_transforms(player_ptr);
}

/**************************************************************************/
/*!
  Frees resources used by the player system.
*/
/**************************************************************************/
void player_system_free(Player* player_ptr)
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

void player_take_damage(Player* player_ptr, int damage)
{
    if (!player_ptr) return;

    int* curr_hp = (player_ptr->current_character == 1)
        ? &player_ptr->stats.char1_curr_health
        : &player_ptr->stats.char2_curr_health;

    *curr_hp -= damage;
    if (*curr_hp < 0) *curr_hp = 0;

    // If selected character is dead, switch to the other one if they're alive
    if (*curr_hp <= 0)
    {
        if (player_ptr->current_character == 1 && player_ptr->stats.char2_curr_health > 0)
        {
            player_ptr->current_character = 2;
            player_ptr->current_player = &player_ptr->char2;
        }
        else if (player_ptr->current_character == 2 && player_ptr->stats.char1_curr_health > 0)
        {
            player_ptr->current_character = 1;
            player_ptr->current_player = &player_ptr->char1;
        }
        // both are 0 ? player_is_dead() will return true, handle game over elsewhere
    }
}

bool player_is_dead(Player* player_ptr)
{
    if (!player_ptr) return false;
    return (player_ptr->stats.char1_curr_health <= 0 &&
        player_ptr->stats.char2_curr_health <= 0);
}