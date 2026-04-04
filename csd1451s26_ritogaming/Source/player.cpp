/*!************************************************************************
\file:          player.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\
\co-authors:    Jerod (10% - player puzzle collision ; found in player movement function)
\               Ahmad (10% - projectile for character 1 & 2, audio management for footsteps
\
\brief:         Implements the player system including character switching,
\               movement with collision detection, puzzle interaction,
\               projectile shooting for character 2, and particle effects.
\               Manages two playable characters with different elements
\               (WATER and FIRE) and their health systems.
**************************************************************************/
#include "pch.hpp"
#include "utils.hpp"
#include "audio_manager.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;
static const char* player_config_data = "Assets/Config/player_config.json";

extern int map[TILE_ROWS][TILE_COLS];
extern std::vector<Wall> g_walls;


constexpr s32 WIZARD_DAMAGE = 2; // change this for fireball damage
constexpr s32 MELEE_DAMAGE = 1;  // knight swing damage

static bool is_footstep_playing = false;
static bool footstep_was_active = false;

// Global player instance definition
Player player_instance;

/**************************************************************************/
/*!
   saves player state (positions, health, active character) to a JSON file.
   - Claire's part
*/
/**************************************************************************/
bool player_save(const Player* player, const std::string& filepath)
{
    if (!player) return false;

    json data;

    data["char1"]["x"] = player->char1.x;
    data["char1"]["y"] = player->char1.y;
    data["char1"]["is_dead"] = player->char1.is_dead;

    data["char2"]["x"] = player->char2.x;
    data["char2"]["y"] = player->char2.y;
    data["char2"]["is_dead"] = player->char2.is_dead;

    data["current_character"] = player->current_character;

    data["stats"]["char1_curr_health"] = player->stats.char1_curr_health;
    data["stats"]["char2_curr_health"] = player->stats.char2_curr_health;
    data["stats"]["char1_max_health"] = player->stats.char1_max_health;
    data["stats"]["char2_max_health"] = player->stats.char2_max_health;
    data["stats"]["character_size"] = player->stats.character_size;
    data["stats"]["triangle_size"] = player->stats.triangle_size;
    data["stats"]["triangle_offset"] = player->stats.triangle_offset;
    data["stats"]["char1_start_x"] = player->stats.char1_start_x;
    data["stats"]["char1_start_y"] = player->stats.char1_start_y;
    data["stats"]["char2_start_x"] = player->stats.char2_start_x;
    data["stats"]["char2_start_y"] = player->stats.char2_start_y;
    data["stats"]["char1_move_speed"] = player->stats.char1_move_speed;
    data["stats"]["char2_move_speed"] = player->stats.char2_move_speed;

    data["fireball_count"] = player->fireball_count;

    std::ofstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "[PlayerSave] Failed to open: " << filepath << "\n";
        return false;
    }

    file << data.dump(2);
    return true;
}
/**************************************************************************/
/*!
   loads player state (positions, health, active character) from a JSON file.
   - Claire's part
*/
/**************************************************************************/
bool player_load(Player* player, const std::string& filepath)
{
    if (!player) return false;

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "[PlayerLoad] File not found: " << filepath << "\n";
        return false;
    }

    json data;
    try { file >> data; }
    catch (const json::parse_error& e)
    {
        std::cerr << "[PlayerLoad] Parse error: " << e.what() << "\n";
        return false;
    }

    try {
        // Load positions and death state
        player->char1.x = data["char1"]["x"];
        player->char1.y = data["char1"]["y"];
        player->char1.is_dead = data["char1"]["is_dead"];

        player->char2.x = data["char2"]["x"];
        player->char2.y = data["char2"]["y"];
        player->char2.is_dead = data["char2"]["is_dead"];

        // Load current character
        player->current_character = data["current_character"];
        player->current_player = (player->current_character == 1)
            ? &player->char1 : &player->char2;

        // Load health and stats from nested stats object
        player->stats.char1_curr_health = data["stats"]["char1_curr_health"];
        player->stats.char2_curr_health = data["stats"]["char2_curr_health"];
        player->stats.char1_max_health = data["stats"]["char1_max_health"];
        player->stats.char2_max_health = data["stats"]["char2_max_health"];
        player->stats.character_size = data["stats"]["character_size"];
        player->stats.triangle_size = data["stats"]["triangle_size"];
        player->stats.triangle_offset = data["stats"]["triangle_offset"];
        player->stats.char1_start_x = data["stats"]["char1_start_x"];
        player->stats.char1_start_y = data["stats"]["char1_start_y"];
        player->stats.char2_start_x = data["stats"]["char2_start_x"];
        player->stats.char2_start_y = data["stats"]["char2_start_y"];
        player->stats.char1_move_speed = data["stats"]["char1_move_speed"];
        player->stats.char2_move_speed = data["stats"]["char2_move_speed"];

        // Load fireball count
        player->fireball_count = data["fireball_count"];
    }
    catch (const json::type_error& e)
    {
        std::cerr << "[PlayerLoad] Type error: " << e.what() << "\n";
        return false;
    }

    return true;
}
/**************************************************************************/
/*!
   saves current player state (positions, health, active character) to a JSON file.
   - Claire's part
*/
/**************************************************************************/
bool player_save_config(const Player* player, const std::string& filepath)
{
    if (!player) return false;

    json data;

    data["character_size"] = player->stats.character_size;
    data["triangle_size"] = player->stats.triangle_size;
    data["triangle_offset"] = player->stats.triangle_offset;

    data["char1_start_x"] = player->stats.char1_start_x;
    data["char1_start_y"] = player->stats.char1_start_y;
    data["char2_start_x"] = player->stats.char2_start_x;
    data["char2_start_y"] = player->stats.char2_start_y;

    data["char1_move_speed"] = player->stats.char1_move_speed;
    data["char2_move_speed"] = player->stats.char2_move_speed;

    data["char1_max_health"] = player->stats.char1_max_health;
    data["char2_max_health"] = player->stats.char2_max_health;

    std::ofstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "[PlayerSaveConfig] Failed to open: " << filepath << "\n";
        return false;
    }

    file << data.dump(2);
    return true;
}

/**************************************************************************/
/*!
   loads player configuration (default stats) from a JSON file.
   - Claire's part
*/
/**************************************************************************/
bool player_load_config(Player* player, const std::string& filepath)
{
    if (!player) return false;

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "[PlayerLoadConfig] File not found: " << filepath << "\n";
        return false;
    }

    json data;
    try { file >> data; }
    catch (const json::parse_error& e)
    {
        std::cerr << "[PlayerLoadConfig] Parse error: " << e.what() << "\n";
        return false;
    }

    try {
        // Read from the nested "stats" object
        player->stats.character_size = data["stats"]["character_size"];
        player->stats.triangle_size = data["stats"]["triangle_size"];
        player->stats.triangle_offset = data["stats"]["triangle_offset"];

        player->stats.char1_start_x = data["stats"]["char1_start_x"];
        player->stats.char1_start_y = data["stats"]["char1_start_y"];
        player->stats.char2_start_x = data["stats"]["char2_start_x"];
        player->stats.char2_start_y = data["stats"]["char2_start_y"];

        player->stats.char1_move_speed = data["stats"]["char1_move_speed"];
        player->stats.char2_move_speed = data["stats"]["char2_move_speed"];

        player->stats.char1_max_health = data["stats"]["char1_max_health"];
        player->stats.char2_max_health = data["stats"]["char2_max_health"];
    }
    catch (const json::type_error& e)
    {
        std::cerr << "[PlayerLoadConfig] Type error: " << e.what() << "\n";
        return false;
    }

    return true;
}

/**************************************************************************/
/*!
	audio update function to be called each frame. 
    Manages footstep sound effects 
	- Ahmad's part
* /
/**************************************************************************/
void footstep_audio_update()
{
    if (!footstep_was_active && is_footstep_playing)
    {
        AEAudioStopGroup(g_audio.sfx_group);
        is_footstep_playing = false;
    }
    footstep_was_active = false;
}
void play_footstep(float pitch = 1.0f)
{
    static float last_pitch = 1.0f;

    // Restart if pitch changed (e.g. started or stopped dragging)
    if (is_footstep_playing && pitch != last_pitch)
    {
        AEAudioStopGroup(g_audio.sfx_group);
        is_footstep_playing = false;
    }

    if (!is_footstep_playing)
    {
        play_sfx(g_audio.sfx_footsteps, 3.2f, pitch, 1);
        is_footstep_playing = true;
        last_pitch = pitch;
    }
    footstep_was_active = true;
}

/**************************************************************************/
/*!
	Player system function to update transformation matrices for characters and
	the triangle indicator each frame. Should be called after handling input and
	before rendering. - Claire's part
* /
/**************************************************************************/
static void player_system_update_transforms(Player *player_ptr)
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
/**************************************************************************/
/*!
	player system initialization function to set up character positions, textures,
    health, and transformation matrices.
	 - Claire's part

     element - jerod's part
* /
/**************************************************************************/
void player_system_init(Player *player_ptr, AEGfxTexture *char_1_texture,
                        AEGfxTexture *char_2_texture, AEGfxTexture *dead_texture)
{
    if (!player_ptr)
        return;

    // --- Character 1 setup ---
    player_ptr->char1.x = player_ptr->stats.char1_start_x;
    player_ptr->char1.y = player_ptr->stats.char1_start_y;
    player_ptr->char1.speed = player_ptr->stats.char1_move_speed;
    player_ptr->char1.character_id = 1;
    player_ptr->char1.element = Puzzle::WATER;
    player_ptr->char1.texture = char_1_texture;

    // --- Character 2 setup ---
    player_ptr->char2.x = player_ptr->stats.char2_start_x;
    player_ptr->char2.y = player_ptr->stats.char2_start_y;
    player_ptr->char2.speed = player_ptr->stats.char2_move_speed;
    player_ptr->char2.character_id = 2;
    player_ptr->char2.element = Puzzle::FIRE;
    player_ptr->char2.texture = char_2_texture;

    // Default to controlling Character 1
    player_ptr->current_player = &player_ptr->char1;
    player_ptr->current_character = 1;

    //
    player_ptr->char1.dead_texture = dead_texture;
    player_ptr->char2.dead_texture = dead_texture;
    player_ptr->char1.is_dead = false;
    player_ptr->char2.is_dead = false;

    player_ptr->stats.char1_curr_health = player_ptr->stats.char1_max_health;
    player_ptr->stats.char2_curr_health = player_ptr->stats.char2_max_health;
    player_ptr->fireball_count = 5;
    player_ptr->shoot_cooldown = 0.0f;

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
/**************************************************************************/
/*!
    player handle switching between 2 chara. 
    1 - knight & 2 - mage
    - Claire's part
* /
/**************************************************************************/
static void player_handle_character_switch(Player *player_ptr)
{
    if (!player_ptr)
        return;

    if (AEInputCheckTriggered(AEVK_1) && !player_ptr->char1.is_dead)
    {
        player_ptr->current_player = &player_ptr->char1;
        player_ptr->current_character = 1;
    }
    if (AEInputCheckTriggered(AEVK_2) && !player_ptr->char2.is_dead)
    {
        player_ptr->current_player = &player_ptr->char2;
        player_ptr->current_character = 2;
    }
}
/**************************************************************************/
/*!
	player movement handling                      - Claire's part
	collision detection against walls and puzzles - jerod's part
* /
/**************************************************************************/
static void player_handle_movement(Player *player_ptr)
{
    if (!player_ptr || !player_ptr->current_player)
        return;
    if (player_ptr->current_player->is_dead)
        return;

    bool is_pushing = false;

    f32* move_x = &player_ptr->current_player->x;
    f32* move_y = &player_ptr->current_player->y;
    Puzzle::Element curr_elem = player_ptr->current_player->element;

    float nextX = *move_x;
    float nextY = *move_y;
    float halfSize = player_ptr->stats.character_size / 2.0f;

    // Track if player is dragging a puzzle block
    static bool is_dragging = false;
    static s32 puzzle_index;

    f32 move_speed = player_ptr->current_player->speed;

    // Movement input
    if (AEInputCheckCurr(AEVK_W))
    {
        nextY += is_dragging ? move_speed / 5.0f : move_speed;
        player_ptr->last_dir_x = 0.0f;
        player_ptr->last_dir_y = 1.0f;
    }
    if (AEInputCheckCurr(AEVK_S))
    {
        nextY -= is_dragging ? move_speed / 5.0f : move_speed;
        player_ptr->last_dir_x = 0.0f;
        player_ptr->last_dir_y = -1.0f;
    }
    if (AEInputCheckCurr(AEVK_A))
    {
        nextX -= is_dragging ? move_speed / 5.0f : move_speed;
        player_ptr->last_dir_x = -1.0f;
        player_ptr->last_dir_y = 0.0f;
    }
    if (AEInputCheckCurr(AEVK_D))
    {
        nextX += is_dragging ? move_speed / 5.0f : move_speed;
        player_ptr->last_dir_x = 1.0f;
        player_ptr->last_dir_y = 0.0f;
    }

    // Horizontal Movement with collision
    if (!check_player_collision(nextX, *move_y, halfSize) &&
        !Puzzle::player_collide_puzzles(nextX, *move_y, halfSize, puzzle_index))
    {
        *move_x = nextX;
    }
    else
    {
        // Slide along wall
        float direction = (nextX > *move_x) ? 1.0f : -1.0f;
        if (!check_player_collision(*move_x + direction, *move_y, halfSize) &&
            !Puzzle::player_collide_puzzles(*move_x + direction, *move_y, halfSize, puzzle_index))
        {
            *move_x += direction;
        }
        // Handle puzzle pushing
        if (Puzzle::player_collide_puzzles(nextX, *move_y, halfSize, puzzle_index))
        {
            move_puzzle(puzzle_index, (direction > 0.0f) ? Puzzle::RIGHT : Puzzle::LEFT, curr_elem);
            is_pushing = true;
        }
    }

    // Vertical movement with collision
    if (!check_player_collision(*move_x, nextY, halfSize) &&
        !Puzzle::player_collide_puzzles(*move_x, nextY, halfSize, puzzle_index))
    {
        *move_y = nextY;
    }
    else
    {
        // Slide along wall
        float direction = (nextY > *move_y) ? 1.0f : -1.0f;
        if (!check_player_collision(*move_x, *move_y + direction, halfSize) &&
            !Puzzle::player_collide_puzzles(*move_x, *move_y + direction, halfSize, puzzle_index))
        {
            *move_y += direction;
        }
        // Handle puzzle pushing
        if (Puzzle::player_collide_puzzles(*move_x, nextY, halfSize, puzzle_index))
        {
            move_puzzle(puzzle_index, (direction > 0.0f) ? Puzzle::DOWN : Puzzle::UP, curr_elem);
            is_pushing = true;
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
    if (AEInputCheckCurr(AEVK_W) || AEInputCheckCurr(AEVK_A) ||
        AEInputCheckCurr(AEVK_S) || AEInputCheckCurr(AEVK_D))
    {
        play_footstep((is_dragging || is_pushing) ? 0.5f : 1.0f);
    }
    footstep_audio_update();
}
// ------------------------------------- End of Jerod's part -------------------------------------

/**************************************************************************/
/*!
  Main player system function to be called each frame. Handles input 
  and updates transforms. - Claire's part
* /
/**************************************************************************/
void player_system_handle_input(Player *player_ptr)
{
    if (!player_ptr)
        return;

    player_handle_character_switch(player_ptr);
    player_handle_movement(player_ptr);
}

/**************************************************************************/
/*!
  Handles shooting logic (CHARACTER 2 ONLY). - Ahmad's part
*/
/**************************************************************************/
void player_handle_shooting(Player * player_ptr, f32 dt)
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
    if (player_ptr->current_character == 2 && !player_ptr->char2.is_dead && AEInputCheckTriggered(AEVK_LBUTTON) && player_ptr->fireball_count > 0)
    {

        // --- Get mouse position in world space ---
        s32 mouseScreenX, mouseScreenY;
        AEInputGetCursorPosition(&mouseScreenX, &mouseScreenY);

        f32 halfWidth = (f32)AEGfxGetWindowWidth() / 2.0f;
        f32 halfHeight = (f32)AEGfxGetWindowHeight() / 2.0f;

        f32 mouseWorldX = (f32)mouseScreenX - halfWidth + Camera::cam.pos.x;
        f32 mouseWorldY = halfHeight - (f32)mouseScreenY + Camera::cam.pos.y;

        // --- Calculate direction from player to mouse ---
        f32 dx = mouseWorldX - player_ptr->char2.x;
        f32 dy = mouseWorldY - player_ptr->char2.y;

        // Normalize the direction vector
        f32 length = sqrtf(dx * dx + dy * dy);
        if (length < 0.0001f)
            return; // safety check: don't shoot if cursor is on player

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
        play_sfx(g_audio.sfx_fireball, 0.08f, 0.45f);
    }
}
void player_update_particles(Player * player_ptr, f32 dt)
{
    for (size_t i = 0; i < player_ptr->particles.size(); ++i)
    {
        Particle& p = player_ptr->particles[i];

        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.life -= dt;

        if (p.life <= 0)
        {
            player_ptr->particles.erase(player_ptr->particles.begin() + i);
            i--;
        }
    }
}
void player_render_particles(Player *player_ptr)
{
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    for (const auto& p : player_ptr->particles)
    {
        // Fade the alpha based on remaining life
        float alpha = p.life / p.maxLife;
        AEGfxSetColorToMultiply(1.0f, 0.3f, 0.0f, alpha); // Red-Orange fire

        // Use your existing draw_circle or draw_rect helper
        draw_circle(p.x, p.y, p.size * alpha, p.size * alpha);
    }
}

/**************************************************************************/
/*!
  Updates projectile positions and handles collisions. - Ahmad's part
*/
/**************************************************************************/
void player_update_projectiles(Player * player_ptr, f32 dt)
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

        if (p.active && enemy_manager_instance.check_projectile_hit(p.x, p.y, p.radius, WIZARD_DAMAGE))
        {
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
  Renders all active projectiles. - Ahmad's part
*/
/**************************************************************************/
void player_render_projectiles(Player * player_ptr)
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
  Renders all active projectile counts. - Ahmad's part   
*/
/**************************************************************************/
void player_render_fireball_count(Player * player_ptr)
{
    if (!player_ptr)
        return;

    std::string text = "Fireballs: " + std::to_string(player_ptr->fireball_count) + " / 5";

    // render_text_corner(text.c_str(), 1, -0.95f, -0.85f, 1.0f, white);
}
void player_handle_melee(Player* player_ptr)
{
    if (!player_ptr) return;

    // Only character 1 (knight) swings
    if (player_ptr->current_character != 1) return;
    if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;

    // Get mouse world position (same math you already have)
    s32 mouseScreenX, mouseScreenY;
    AEInputGetCursorPosition(&mouseScreenX, &mouseScreenY);

    f32 halfWidth = (f32)AEGfxGetWindowWidth() / 2.0f;
    f32 halfHeight = (f32)AEGfxGetWindowHeight() / 2.0f;

    f32 mouseWorldX = (f32)mouseScreenX - halfWidth + Camera::cam.pos.x;
    f32 mouseWorldY = halfHeight - (f32)mouseScreenY + Camera::cam.pos.y;

    // Determine left or right of player
    float dx = mouseWorldX - player_ptr->char1.x;

    player_ptr->swing.active = true;
    player_ptr->swing.progress = 0.0f;
    player_ptr->swing.dir = (dx >= 0.0f) ? 1.0f : -1.0f;  // +1 right, -1 left

    play_sfx(g_audio.sfx_sword_atk, 0.1f, 1.1f);
}
void player_update_swing(Player* player_ptr, f32 dt)
{
    if (!player_ptr) return;

    SwingArc& sw = player_ptr->swing;
    if (!sw.active) return;

    sw.progress += dt / sw.duration;

    // only check for hits once per swing, not every frame
    if (!sw.has_hit)
    {
        float cx = player_ptr->char1.x;
        float cy = player_ptr->char1.y;

        float startAngle = PI * 0.5f;
        float endAngle = -PI * 0.5f;
        float currEnd = startAngle + (endAngle - startAngle) * sw.progress;

        int segments = 20;
        for (int i = 0; i <= segments; i++)
        {
            float t = (float)i / segments;
            float angle = startAngle + (currEnd - startAngle) * t;

            float px = cx + cosf(angle) * sw.radius * sw.dir;
            float py = cy + sinf(angle) * sw.radius;

            if (enemy_manager_instance.check_projectile_hit(px, py, 6.0f, MELEE_DAMAGE))
            {
                sw.has_hit = true; // stop checking once an enemy is hit
                break;
            }
        }
    }

    if (sw.progress >= 1.0f)
    {
        sw.progress = 0.0f;
        sw.active = false;
        sw.has_hit = false; // reset for next swing
    }
}
void player_render_swing(Player* player_ptr)
{
    if (!player_ptr) return;

    SwingArc& sw = player_ptr->swing;
    if (!sw.active) return;

    float cx = player_ptr->char1.x;
    float cy = player_ptr->char1.y;

    float startAngle = PI * 0.5f;    // top
    float endAngle = -PI * 0.5f;   // bottom
    float currEnd = startAngle + (endAngle - startAngle) * sw.progress;

    int   segments = 20;
    float fade = 1.0f - sw.progress * 0.5f;

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(0.8f, 0.9f, 1.0f, fade);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = 0; i <= segments; i++)
    {
        float t = (float)i / segments;
        float angle = startAngle + (currEnd - startAngle) * t;

        float px = cx + cosf(angle) * sw.radius * sw.dir;
        float py = cy + sinf(angle) * sw.radius;

        // dot size shrinks toward the tip for a trail effect
        float dot_size = 10.0f * (1.0f - t * 0.5f);

        draw_circle(px, py, dot_size, dot_size, true);
    }
}

/**************************************************************************/
/*!
  Renders the player characters and active character indicator. - Claire's part
*/
/**************************************************************************/
void player_system_render(Player* player_ptr)
{
    if (!player_ptr)
        return;
    player_render_particles(player_ptr);
    player_render_swing(player_ptr);
    player_render_fireball_count(player_ptr);

    // Determine flash amount based on hit timer
    f32 flash1 = player_ptr->char1.hit_flash_timer > 0.f ? 1.f : 0.f;
    f32 flash2 = player_ptr->char2.hit_flash_timer > 0.f ? 1.f : 0.f;

    // Draw Character 1
    AEGfxTexture* tex1 = player_ptr->char1.is_dead ? player_ptr->char1.dead_texture
        : player_ptr->char1.texture;
    render_sprite_flash(square_mesh, player_ptr->char1.x, player_ptr->char1.y,
        player_ptr->stats.character_size,
        player_ptr->stats.character_size, tex1, flash1);

    // Draw Character 2
    AEGfxTexture* tex2 = player_ptr->char2.is_dead ? player_ptr->char2.dead_texture
        : player_ptr->char2.texture;
    render_sprite_flash(square_mesh, player_ptr->char2.x, player_ptr->char2.y,
        player_ptr->stats.character_size,
        player_ptr->stats.character_size, tex2, flash2);

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
  Base : Movement, character switching, and general system updates - Claire

  puzzle interaction - Jerod
  Shooting and projectiles - Ahmad
*/
/**************************************************************************/
void player_system_update(Player * player_ptr)
{
    if (!player_ptr)
        return;

    // Tick down hit flash timer
    if (player_ptr->char1.hit_flash_timer > 0.f)
        player_ptr->char1.hit_flash_timer -= dt;
    if (player_ptr->char2.hit_flash_timer > 0.f)
        player_ptr->char2.hit_flash_timer -= dt;

    player_system_handle_input(player_ptr);
    player_handle_shooting(player_ptr, dt);
    player_update_projectiles(player_ptr, dt);
    player_system_update_transforms(player_ptr);
    player_update_particles(player_ptr, dt);
    player_handle_melee(player_ptr);
    player_update_swing(player_ptr, dt);
}

/**************************************************************************/
/*!
  Frees resources used by the player system. 
  Note: Textures are managed by the main game and should not be freed here.
   - Claire
*/
/**************************************************************************/
void player_system_free(Player * player_ptr)
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
/**************************************************************************/
/*!
  Applies damage to the currently active character and handles death logic. 
  - Claire's part
*/     
/**************************************************************************/
void player_take_damage(Player *player_ptr, int damage)
{
    if (!player_ptr)
        return;

	// Start hit flash effect on the currently active character
    if (player_ptr->current_character == 1)
        player_ptr->char1.hit_flash_timer = 0.15f;
    else
        player_ptr->char2.hit_flash_timer = 0.15f;

    // Play different hurt sound depending on active character
    if (player_ptr->current_character == 1)
        play_sfx(g_audio.sfx_knight_hurt); // Knight hurt sound
    else
        play_sfx(g_audio.sfx_mage_hurt); // Mage hurt sound

    int* curr_hp = (player_ptr->current_character == 1)
        ? &player_ptr->stats.char1_curr_health
        : &player_ptr->stats.char2_curr_health;

    *curr_hp -= damage;
    if (*curr_hp < 0)
        *curr_hp = 0;

    // If selected character is dead, switch to the other one if they're alive
    if (*curr_hp <= 0)
    {
        if (player_ptr->current_character == 1)
            player_ptr->char1.is_dead = true;
        else
            player_ptr->char2.is_dead = true;

        // Auto-switch to the other character if not dead
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
    }
}

/**************************************************************************/
/*!
  Checks if both characters are dead. Returns true if the player has lost. 
  - Claire's part
*/
/**************************************************************************/    
bool player_is_dead(Player *player_ptr)
{
    if (!player_ptr)
        return false;
    return (player_ptr->stats.char1_curr_health <= 0 &&
        player_ptr->stats.char2_curr_health <= 0);
}

/**************************************************************************/
/*!
  Spawns player characters based on their positions in the map array. 
  Converts tile coordinates to world coordinates and assigns them to the player struct.
  - Claire's part
*/
/**************************************************************************/
void spawn_player_from_map(Player *player_ptr, f32 tile_size)
{
	for (int r = 0; r < TILE_ROWS; r++)
	{
		for (int c = 0; c < TILE_COLS; c++)
		{
			// Assign positions
			if (map[r][c] == KNIGHT)
			{
				player_ptr->char2.x = col_to_world(c);
				player_ptr->char2.y = row_to_world(r);
			}

			else if (map[r][c] == WIZARD)
			{
				player_ptr->char1.x = col_to_world(c);
				player_ptr->char1.y = row_to_world(r);
			}
		}
	}
}
