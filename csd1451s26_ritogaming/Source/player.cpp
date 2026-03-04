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
**************************************************************************/

#include "collision.hpp"
#include "player.hpp"
#include <crtdbg.h>
#include "camera.hpp"
#include "renderer.hpp"
#include "enemy_manager.hpp"
#include "puzzle.hpp"
#include <cmath>
#include <string>


extern int map[TILE_ROWS][TILE_COLS];
extern std::vector<Wall> g_walls;

#define DAMAGE 20.0f // change this for fireball damage

// Global player instance definition
Player player_instance;

/**************************************************************************/
/*!
  Updates transformation matrices for both characters and the active
  player indicator triangle.
*/
/**************************************************************************/
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
        player_ptr->current_player->y + player_ptr->config.triangle_offset);

    AEMtx33Concat(&player_ptr->triangle_transform, &player_ptr->rotate, &player_ptr->triangle_scale);
    AEMtx33Concat(&player_ptr->triangle_transform, &triangle_translate, &player_ptr->triangle_transform);
}

/**************************************************************************/
/*!
  Initializes the player system using the provided configuration.
*/
/**************************************************************************/
void player_system_init(Player* player_ptr, const player_config* config)
{
    if (!player_ptr)
        return;

    // Copy configuration values if provided
    if (config)
    {
        player_ptr->config = *config;
    }

    // --- Character 1 setup ---
    player_ptr->char1.x = player_ptr->config.char1_start_x;
    player_ptr->char1.y = player_ptr->config.char1_start_y;
    player_ptr->char1.speed = player_ptr->config.move_speed;
    player_ptr->char1.character_id = 1;
    player_ptr->char1.element = WATER;
    player_ptr->char1.texture = TileTextures[4];

    // --- Character 2 setup ---
    player_ptr->char2.x = player_ptr->config.char2_start_x;
    player_ptr->char2.y = player_ptr->config.char2_start_y;
    player_ptr->char2.speed = player_ptr->config.move_speed;
    player_ptr->char2.character_id = 2;
    player_ptr->char2.element = FIRE;
    player_ptr->char2.texture = TileTextures[3];
    player_ptr->fireball_count = 5;
    player_ptr->shoot_cooldown = 0.0f;

    // Default to controlling Character 1
    player_ptr->current_player = &player_ptr->char1;
    player_ptr->current_character = 1;

    // Precompute scale and rotation matrices
    AEMtx33Scale(&player_ptr->character_scale, player_ptr->config.character_size, player_ptr->config.character_size);
    AEMtx33Scale(&player_ptr->triangle_scale, player_ptr->config.triangle_size, player_ptr->config.triangle_size);
    AEMtx33Rot(&player_ptr->rotate, 0);

    // Initialize last direction for shooting
    player_ptr->last_dir_x = 1.0f;
    player_ptr->last_dir_y = 0.0f;

    // Initialize all transforms
    player_system_update_transforms(player_ptr);
}

/**************************************************************************/
/*!
  Handles character switching input.
*/
/**************************************************************************/
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

/**************************************************************************/
/*!
  Handles movement input for the active character.
*/
/**************************************************************************/
static void player_handle_movement(Player* player_ptr)
{
    if (!player_ptr || !player_ptr->current_player)
        return;

    f32* move_x = &player_ptr->current_player->x;
    f32* move_y = &player_ptr->current_player->y;
    Element curr_elem = player_ptr->current_player->element;

    float nextX = *move_x;
    float nextY = *move_y;
    float halfSize = player_ptr->config.character_size / 2.0f;

    // Track if player is dragging a puzzle block
    static bool is_dragging = false;
    static s32 puzzle_index;

    // Movement input
    if (AEInputCheckCurr(AEVK_UP) || AEInputCheckCurr(AEVK_W))
    {
        nextY += is_dragging ? 1.0f : player_ptr->config.move_speed;
        player_ptr->last_dir_x = 0.0f;
        player_ptr->last_dir_y = 1.0f;
    }
    if (AEInputCheckCurr(AEVK_DOWN) || AEInputCheckCurr(AEVK_S))
    {
        nextY -= is_dragging ? 1.0f : player_ptr->config.move_speed;
        player_ptr->last_dir_x = 0.0f;
        player_ptr->last_dir_y = -1.0f;
    }
    if (AEInputCheckCurr(AEVK_LEFT) || AEInputCheckCurr(AEVK_A))
    {
        nextX -= is_dragging ? 1.0f : player_ptr->config.move_speed;
        player_ptr->last_dir_x = -1.0f;
        player_ptr->last_dir_y = 0.0f;
    }
    if (AEInputCheckCurr(AEVK_RIGHT) || AEInputCheckCurr(AEVK_D))
    {
        nextX += is_dragging ? 1.0f : player_ptr->config.move_speed;
        player_ptr->last_dir_x = 1.0f;
        player_ptr->last_dir_y = 0.0f;
    }

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

/**************************************************************************/
/*!
  Handles player input for character switching and movement.
*/
/**************************************************************************/
void player_system_handle_input(Player* player_ptr)
{
    if (!player_ptr)
        return;

    player_handle_character_switch(player_ptr);
    player_handle_movement(player_ptr);
}

/**************************************************************************/
/*!
  Handles shooting logic (CHARACTER 2 ONLY).
*/
/**************************************************************************/
void player_handle_shooting(Player* player_ptr, f32 dt)
{
    if (!player_ptr)
        return;
    // Recharge one fireball every 1 second (up to max 5)
    player_ptr->shoot_cooldown += dt;
    if (player_ptr->shoot_cooldown >= 1.0f && player_ptr->fireball_count < 5)
    {
        player_ptr->fireball_count++;
        player_ptr->shoot_cooldown = 0.0f; // reset timer for next recharge
    }

    // Only character 2 can shoot
    if (player_ptr->current_character == 2 && AEInputCheckTriggered(AEVK_LBUTTON)
        && player_ptr->fireball_count > 0)
    {

        // --- Get mouse position in world space ---
        s32 mouseScreenX, mouseScreenY;
        AEInputGetCursorPosition(&mouseScreenX, &mouseScreenY);

        f32 halfWidth = (f32)AEGfxGetWindowWidth() / 2.0f;
        f32 halfHeight = (f32)AEGfxGetWindowHeight() / 2.0f;

        f32 mouseWorldX = (f32)mouseScreenX - halfWidth + cam.pos.x;
        f32 mouseWorldY = halfHeight - (f32)mouseScreenY + cam.pos.y;

        // --- Calculate direction from player to mouse ---
        f32 dx = mouseWorldX - player_ptr->char2.x;
        f32 dy = mouseWorldY - player_ptr->char2.y;

        // Normalize the direction vector
        f32 length = sqrtf(dx * dx + dy * dy);
        if (length < 0.0001f) return; // safety check: don't shoot if cursor is on player

        f32 dirX = dx / length;
        f32 dirY = dy / length;

        // --- Update last direction (so movement-facing still works) ---
        player_ptr->last_dir_x = dirX;
        player_ptr->last_dir_y = dirY;

        // --- Spawn projectile ---
        Projectile new_fire;
        new_fire.active = true;
        new_fire.radius = 8.0f;

        f32 spawn_offset = 30.0f;
        new_fire.x = player_ptr->char2.x + (dirX * spawn_offset);
        new_fire.y = player_ptr->char2.y + (dirY * spawn_offset);

        f32 shotSpeed = 600.0f;
        new_fire.vx = dirX * shotSpeed;
        new_fire.vy = dirY * shotSpeed;

        player_ptr->projectiles.push_back(new_fire);
        player_ptr->fireball_count--;
    }
}
void player_update_particles(Player* player_ptr, f32 dt) {
    for (size_t i = 0; i < player_ptr->particles.size(); ++i) {
        Particle& p = player_ptr->particles[i];

        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.life -= dt;

        if (p.life <= 0) {
            player_ptr->particles.erase(player_ptr->particles.begin() + i);
            i--;
        }
    }
}

void player_render_particles(Player* player_ptr) {
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    for (const auto& p : player_ptr->particles) {
        // Fade the alpha based on remaining life
        float alpha = p.life / p.maxLife;
        AEGfxSetColorToMultiply(1.0f, 0.3f, 0.0f, alpha); // Red-Orange fire

        // Use your existing draw_circle or draw_rect helper
        draw_circle(circle_mesh, p.x, p.y, p.size * alpha, p.size * alpha);
    }
}
    
/**************************************************************************/
/*!
  Updates projectile positions and handles collisions.
*/
/**************************************************************************/
void player_update_projectiles(Player* player_ptr, f32 dt)
{
    if (!player_ptr)
        return;

    for (size_t i = 0; i < player_ptr->projectiles.size(); ++i)
    {
        Projectile& p = player_ptr->projectiles[i];

        p.x += p.vx * dt;
        p.y += p.vy * dt;

        // --- PARTICLE SPAWNING ---
        // Every frame, create a "trail" particle
        Particle trail;
        trail.x = p.x;
        trail.y = p.y;
        // Give it a tiny bit of random drift
        trail.vx = ((float)rand() / RAND_MAX - 0.5f) * 20.0f;
        trail.vy = ((float)rand() / RAND_MAX - 0.5f) * 20.0f;
        trail.maxLife = 0.5f; // Particles last half a second
        trail.life = trail.maxLife;
        trail.size = 10.0f;
        trail.active = true;

        player_ptr->particles.push_back(trail);

        // Check if fireball hits wall
        if (check_projectile_collision(p.x, p.y, p.radius))
        {
            p.active = false;
        }
        //
        if (p.active && g_enemy_manager.check_projectile_hit(p.x, p.y, p.radius, DAMAGE)) {
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
            draw_circle(circle_mesh, p.x, p.y, p.radius * 2.0f, p.radius * 2.0f);
        }
    }
}
/**************************************************************************/
/*!
  Renders all active projectile counts.
*/
/**************************************************************************/
void player_render_fireball_count(Player* player_ptr)
{
    if (!player_ptr)
        return;

    std::string text = "Fireballs: " + std::to_string(player_ptr->fireball_count) + " / 5";

    render_text_corner(text.c_str(), 1, -0.95f, -0.85f, 1.0f, white);
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
    player_render_particles(player_ptr);
    player_render_fireball_count(player_ptr);

    // Draw Character 1
    render_sprite(rect_mesh_center, player_ptr->char1.x, player_ptr->char1.y,
        player_ptr->config.character_size, player_ptr->config.character_size, TileTextures[4]);

    // Draw Character 2
    render_sprite(rect_mesh_center, player_ptr->char2.x, player_ptr->char2.y,
        player_ptr->config.character_size, player_ptr->config.character_size, TileTextures[3]);

    // Render projectiles
    player_render_projectiles(player_ptr);

    // Position the indicator triangle above the active player
    f32 tri_x = player_ptr->current_player->x;
    f32 tri_y = player_ptr->current_player->y + player_ptr->config.triangle_offset;

    // Green triangle marks the currently controlled character
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
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
void player_system_update(Player* player_ptr)
{
    if (!player_ptr)
        return;

    f32 dt = (f32)AEFrameRateControllerGetFrameTime();

    player_system_handle_input(player_ptr);
    player_handle_shooting(player_ptr, dt);
    player_update_projectiles(player_ptr, dt);
    player_system_update_transforms(player_ptr);
    player_update_particles(player_ptr, dt);
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

   
    player_ptr->char1.texture = nullptr;
    player_ptr->char2.texture = nullptr;

    player_ptr->projectiles.clear(); // Clean up projectiles
    player_ptr->particles.clear(); 
    // Clear active player reference
    player_ptr->current_player = nullptr;
}