/*!************************************************************************
\file:          enemy.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu

\brief:         Implements enemy movement behaviors and enemy type classes.
                Contains dave_Data() which reads enemy_data.txt and
                assigns all tuning values and filepaths to the global
                variables declared in enemy.hpp. Also implements
                BossProjectileManager, base_enemy, ghost, bat, and dragon.
**************************************************************************/

#include "pch.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

extern s32 map[TILE_ROWS][TILE_COLS];
extern std::vector<Wall> g_walls;


// ----------------------------------------------------------------------------
// Global variable definitions — declared extern in enemy.hpp.
// All assigned inside save_Data(); do not modify elsewhere.
// ----------------------------------------------------------------------------

// Texture filepaths
std::string e_ghost_tex;
std::string e_bat_tex;
std::string e_ghost_food_tex;
std::string e_bat_food_tex;
std::string e_cross_tex;
std::string e_ghost_marker_tex;
std::string e_bat_marker_tex;
std::string e_dragon_tex_idle;
std::string e_dragon_tex_walk;
std::string e_dragon_tex_attack;
std::string e_dragon_tex_hurt;
std::string e_dragon_tex_death;

// ----------------------------------------------------------------------------
// Dragon animation config — frame counts, dimensions, and playback rates
// for each of the five dragon clips. All values assigned by save_Data().
// ----------------------------------------------------------------------------
int   e_dragon_idle_frames;
float e_dragon_idle_frame_w;
float e_dragon_idle_frame_h;
float e_dragon_idle_sheet_w;
float e_dragon_idle_sheet_h;
float e_dragon_idle_fps;
int   e_dragon_idle_loop;

int   e_dragon_walk_frames;
float e_dragon_walk_frame_w;
float e_dragon_walk_frame_h;
float e_dragon_walk_sheet_w;
float e_dragon_walk_sheet_h;
float e_dragon_walk_fps;
int   e_dragon_walk_loop;

int   e_dragon_attack_frames;
float e_dragon_attack_frame_w;
float e_dragon_attack_frame_h;
float e_dragon_attack_sheet_w;
float e_dragon_attack_sheet_h;
float e_dragon_attack_fps;
int   e_dragon_attack_loop;

int   e_dragon_hurt_frames;
float e_dragon_hurt_frame_w;
float e_dragon_hurt_frame_h;
float e_dragon_hurt_sheet_w;
float e_dragon_hurt_sheet_h;
float e_dragon_hurt_fps;
int   e_dragon_hurt_loop;

int   e_dragon_death_frames;
float e_dragon_death_frame_w;
float e_dragon_death_frame_h;
float e_dragon_death_sheet_w;
float e_dragon_death_sheet_h;
float e_dragon_death_fps;
int   e_dragon_death_loop;

// Ghost stats
float e_ghost_speed;
int   e_ghost_max_health;
float e_ghost_size;
float e_ghost_sight_range;

// Bat stats
float e_bat_speed;
int   e_bat_max_health;
float e_bat_size;
float e_bat_sight_range;
float e_bat_attack_cooldown;
int   e_bat_attack_damage;

// Dragon stats
float e_dragon_speed;
int   e_dragon_max_health;
float e_dragon_size;
float e_dragon_attack_cooldown;
int   e_dragon_attack_damage;

// Boss projectile config
float e_bp_ring_radius;
float e_bp_ring_thickness;
float e_bp_base_speed;
float e_bp_wave_amplitude;
float e_bp_wave_frequency;
float e_bp_max_lifetime;
int   e_bp_positions_wave_a;
int   e_bp_positions_wave_b;
float e_bp_angle_spread_deg;
float e_bp_wave_interval;
int   e_bp_projectile_damage;
int   e_bp_pool_size;

// Manager / world config
float e_spawn_check_interval;
float e_respawn_delay;
float e_food_lifetime;
float e_food_size;
float e_map_start_x;
float e_map_start_y;
float e_tile_size;

// Respawn particle config
int   e_rp_burst_count;
float e_rp_speed_min;
float e_rp_speed_max;
float e_rp_life_min;
float e_rp_life_max;
float e_rp_size_min;
float e_rp_size_max;
float e_rp_gravity;
float e_rp_rise_boost;


// ----------------------------------------------------------------------------
// save_Data()
// Reads the config file line by line. Each line is "<token> <value>".
// Lines starting with '#' or empty lines are skipped.
// Unknown tokens print a warning to stderr.
//
// \param filepath  Path to the config file
// \return          true on success, false if the file could not be opened
// ----------------------------------------------------------------------------
bool save_Data(const std::string& filepath = "Assets/Config/enemy_config.txt")
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "[dave_Data] Could not open: " << filepath << "\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string token;
        if (!(iss >> token)) continue;

        // Texture filepaths
        if (token == "ghost_tex")          iss >> e_ghost_tex;
        else if (token == "bat_tex")            iss >> e_bat_tex;
        else if (token == "ghost_food_tex")     iss >> e_ghost_food_tex;
        else if (token == "bat_food_tex")       iss >> e_bat_food_tex;
        else if (token == "cross_tex")          iss >> e_cross_tex;
        else if (token == "ghost_marker_tex")   iss >> e_ghost_marker_tex;
        else if (token == "bat_marker_tex")     iss >> e_bat_marker_tex;
        else if (token == "dragon_tex_idle")    iss >> e_dragon_tex_idle;
        else if (token == "dragon_tex_walk")    iss >> e_dragon_tex_walk;
        else if (token == "dragon_tex_attack")  iss >> e_dragon_tex_attack;
        else if (token == "dragon_tex_hurt")    iss >> e_dragon_tex_hurt;
        else if (token == "dragon_tex_death")   iss >> e_dragon_tex_death;

        // Dragon animation config
        else if (token == "dragon_idle_frames")     iss >> e_dragon_idle_frames;
        else if (token == "dragon_idle_frame_w")    iss >> e_dragon_idle_frame_w;
        else if (token == "dragon_idle_frame_h")    iss >> e_dragon_idle_frame_h;
        else if (token == "dragon_idle_sheet_w")    iss >> e_dragon_idle_sheet_w;
        else if (token == "dragon_idle_sheet_h")    iss >> e_dragon_idle_sheet_h;
        else if (token == "dragon_idle_fps")        iss >> e_dragon_idle_fps;
        else if (token == "dragon_idle_loop")       iss >> e_dragon_idle_loop;

        else if (token == "dragon_walk_frames")     iss >> e_dragon_walk_frames;
        else if (token == "dragon_walk_frame_w")    iss >> e_dragon_walk_frame_w;
        else if (token == "dragon_walk_frame_h")    iss >> e_dragon_walk_frame_h;
        else if (token == "dragon_walk_sheet_w")    iss >> e_dragon_walk_sheet_w;
        else if (token == "dragon_walk_sheet_h")    iss >> e_dragon_walk_sheet_h;
        else if (token == "dragon_walk_fps")        iss >> e_dragon_walk_fps;
        else if (token == "dragon_walk_loop")       iss >> e_dragon_walk_loop;

        else if (token == "dragon_attack_frames")   iss >> e_dragon_attack_frames;
        else if (token == "dragon_attack_frame_w")  iss >> e_dragon_attack_frame_w;
        else if (token == "dragon_attack_frame_h")  iss >> e_dragon_attack_frame_h;
        else if (token == "dragon_attack_sheet_w")  iss >> e_dragon_attack_sheet_w;
        else if (token == "dragon_attack_sheet_h")  iss >> e_dragon_attack_sheet_h;
        else if (token == "dragon_attack_fps")      iss >> e_dragon_attack_fps;
        else if (token == "dragon_attack_loop")     iss >> e_dragon_attack_loop;

        else if (token == "dragon_hurt_frames")     iss >> e_dragon_hurt_frames;
        else if (token == "dragon_hurt_frame_w")    iss >> e_dragon_hurt_frame_w;
        else if (token == "dragon_hurt_frame_h")    iss >> e_dragon_hurt_frame_h;
        else if (token == "dragon_hurt_sheet_w")    iss >> e_dragon_hurt_sheet_w;
        else if (token == "dragon_hurt_sheet_h")    iss >> e_dragon_hurt_sheet_h;
        else if (token == "dragon_hurt_fps")        iss >> e_dragon_hurt_fps;
        else if (token == "dragon_hurt_loop")       iss >> e_dragon_hurt_loop;

        else if (token == "dragon_death_frames")    iss >> e_dragon_death_frames;
        else if (token == "dragon_death_frame_w")   iss >> e_dragon_death_frame_w;
        else if (token == "dragon_death_frame_h")   iss >> e_dragon_death_frame_h;
        else if (token == "dragon_death_sheet_w")   iss >> e_dragon_death_sheet_w;
        else if (token == "dragon_death_sheet_h")   iss >> e_dragon_death_sheet_h;
        else if (token == "dragon_death_fps")       iss >> e_dragon_death_fps;
        else if (token == "dragon_death_loop")      iss >> e_dragon_death_loop;

        // Ghost stats
        else if (token == "ghost_speed")        iss >> e_ghost_speed;
        else if (token == "ghost_max_health")   iss >> e_ghost_max_health;
        else if (token == "ghost_size")         iss >> e_ghost_size;
        else if (token == "ghost_sight_range")  iss >> e_ghost_sight_range;

        // Bat stats
        else if (token == "bat_speed")          iss >> e_bat_speed;
        else if (token == "bat_max_health")     iss >> e_bat_max_health;
        else if (token == "bat_size")           iss >> e_bat_size;
        else if (token == "bat_sight_range")    iss >> e_bat_sight_range;
        else if (token == "bat_attack_cooldown")iss >> e_bat_attack_cooldown;
        else if (token == "bat_attack_damage")  iss >> e_bat_attack_damage;

        // Dragon stats
        else if (token == "dragon_speed")           iss >> e_dragon_speed;
        else if (token == "dragon_max_health")      iss >> e_dragon_max_health;
        else if (token == "dragon_size")            iss >> e_dragon_size;
        else if (token == "dragon_attack_cooldown") iss >> e_dragon_attack_cooldown;
        else if (token == "dragon_attack_damage")   iss >> e_dragon_attack_damage;

        // Boss projectile config
        else if (token == "bp_ring_radius")       iss >> e_bp_ring_radius;
        else if (token == "bp_ring_thickness")    iss >> e_bp_ring_thickness;
        else if (token == "bp_base_speed")        iss >> e_bp_base_speed;
        else if (token == "bp_wave_amplitude")    iss >> e_bp_wave_amplitude;
        else if (token == "bp_wave_frequency")    iss >> e_bp_wave_frequency;
        else if (token == "bp_max_lifetime")      iss >> e_bp_max_lifetime;
        else if (token == "bp_positions_wave_a")  iss >> e_bp_positions_wave_a;
        else if (token == "bp_positions_wave_b")  iss >> e_bp_positions_wave_b;
        else if (token == "bp_angle_spread_deg")  iss >> e_bp_angle_spread_deg;
        else if (token == "bp_wave_interval")     iss >> e_bp_wave_interval;
        else if (token == "bp_projectile_damage") iss >> e_bp_projectile_damage;
        else if (token == "bp_pool_size")         iss >> e_bp_pool_size;

        // Manager / world config
        else if (token == "spawn_check_interval") iss >> e_spawn_check_interval;
        else if (token == "respawn_delay")        iss >> e_respawn_delay;
        else if (token == "food_lifetime")        iss >> e_food_lifetime;
        else if (token == "food_size")            iss >> e_food_size;
        else if (token == "map_start_x")          iss >> e_map_start_x;
        else if (token == "map_start_y")          iss >> e_map_start_y;
        else if (token == "tile_size")            iss >> e_tile_size;

        // Respawn particle config
        else if (token == "rp_burst_count")  iss >> e_rp_burst_count;
        else if (token == "rp_speed_min")    iss >> e_rp_speed_min;
        else if (token == "rp_speed_max")    iss >> e_rp_speed_max;
        else if (token == "rp_life_min")     iss >> e_rp_life_min;
        else if (token == "rp_life_max")     iss >> e_rp_life_max;
        else if (token == "rp_size_min")     iss >> e_rp_size_min;
        else if (token == "rp_size_max")     iss >> e_rp_size_max;
        else if (token == "rp_gravity")      iss >> e_rp_gravity;
        else if (token == "rp_rise_boost")   iss >> e_rp_rise_boost;

        else
            std::cerr << "[dave_Data] Unknown token: " << token << "\n";
    }

    return true;
}


// ============================================================================
// BOSS PROJECTILE MANAGER
// ============================================================================

BossProjectileManager boss_proj_manager;

static constexpr float BP_DEG2RAD = 3.14159265f / 180.0f;

// ----------------------------------------------------------------------------
// bp_rotate_vec() — rotates vector (x, y) by angle_rad, writes to out_x/out_y
// ----------------------------------------------------------------------------
static void bp_rotate_vec(float x, float y, float angle_rad,
    float& out_x, float& out_y)
{
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    out_x = x * c - y * s;
    out_y = x * s + y * c;
}

BossProjectileManager::BossProjectileManager() {}

// Allocates the pool; uses e_bp_pool_size if pool_size is 0
void BossProjectileManager::init(int pool_size)
{
    int actual = (pool_size > 0) ? pool_size : e_bp_pool_size;
    m_pool.clear();
    m_pool.resize(static_cast<size_t>(actual));
    m_wave_toggle = false;
}

// Marks every slot inactive without freeing memory
void BossProjectileManager::clear()
{
    for (auto& p : m_pool) p.in_use = false;
}

// ----------------------------------------------------------------------------
// update() — advances all active projectiles, applying base velocity and
// sinusoidal wave offset. Culls expired or off-screen projectiles.
// ----------------------------------------------------------------------------
void BossProjectileManager::update(float dt, const Camera::camera* p_cam)
{
    if (!p_cam) return;

    const float hw = (AEGfxGetWindowWidth() * 0.5f) / Camera::zoom;
    const float hh = (AEGfxGetWindowHeight() * 0.5f) / Camera::zoom;
    const float margin = e_bp_ring_radius * 4.0f;

    for (auto& p : m_pool)
    {
        if (!p.in_use) continue;

        p.age += dt;
        p.lifetime -= dt;

        // Sinusoidal perpendicular offset for wave motion
        const float sine_val = sinf(p.age * e_bp_wave_frequency + p.phase)
            * e_bp_wave_amplitude;

        p.x += (p.vx + p.perp_x * sine_val) * dt;
        p.y += (p.vy + p.perp_y * sine_val) * dt;

        // Cull if expired or outside the camera margin
        const bool expired = (p.lifetime <= 0.0f);
        const bool off_screen = (p.x < p_cam->pos.x - hw - margin) ||
            (p.x > p_cam->pos.x + hw + margin) ||
            (p.y < p_cam->pos.y - hh - margin) ||
            (p.y > p_cam->pos.y + hh + margin);
        if (expired || off_screen) p.in_use = false;
    }
}

// ----------------------------------------------------------------------------
// check_player_collision() — tests all active projectiles against the player.
// Deactivates any that connect and applies damage. Returns total hits.
// ----------------------------------------------------------------------------
int BossProjectileManager::check_player_collision(Player* player)
{
    if (!player || !player->current_player) return 0;

    const float px = player->current_player->x;
    const float py = player->current_player->y;
    const float p_rad = player->stats.character_size * 0.5f;

    int hits = 0;
    for (auto& proj : m_pool)
    {
        if (!proj.in_use) continue;
        if (cc_collision(proj.x, proj.y, e_bp_ring_radius, px, py, p_rad))
        {
            proj.in_use = false;
            player_take_damage(player, e_bp_projectile_damage);
            ++hits;
        }
    }
    return hits;
}

// ----------------------------------------------------------------------------
// trigger_wave() — fires a spread of projectiles aimed at the target.
// Alternates between wave pattern A (even slots) and B (odd slots) each call.
// ----------------------------------------------------------------------------
void BossProjectileManager::trigger_wave(float boss_x, float boss_y,
    float target_x, float target_y)
{
    // Normalise the fire direction
    float dx = target_x - boss_x;
    float dy = target_y - boss_y;
    const float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 1.0f) { dx = 1.0f; dy = 0.0f; }
    else { dx /= dist; dy /= dist; }

    const float perp_x = -dy;
    const float perp_y = dx;

    // Distribute projectiles across the angular spread
    const int   total_slots = e_bp_positions_wave_a + e_bp_positions_wave_b;
    const float spread_rad = e_bp_angle_spread_deg * BP_DEG2RAD;
    const float slot_step = spread_rad / static_cast<float>(total_slots - 1);

    const bool is_wave_b = m_wave_toggle;
    const int  count = is_wave_b ? e_bp_positions_wave_b : e_bp_positions_wave_a;

    for (int i = 0; i < count; ++i)
    {
        // Wave B uses odd slots so the two patterns interleave
        const int   slot = is_wave_b ? (i * 2 + 1) : (i * 2);
        const float angle_offset = -spread_rad * 0.5f + static_cast<float>(slot) * slot_step;

        float shot_dx, shot_dy, shot_px, shot_py;
        bp_rotate_vec(dx, dy, angle_offset, shot_dx, shot_dy);
        bp_rotate_vec(perp_x, perp_y, angle_offset, shot_px, shot_py);

        // Stagger wave phases so projectiles oscillate out of sync
        const float base_phase = static_cast<float>(slot)
            * (2.0f * 3.14159265f / static_cast<float>(total_slots));
        const float wave_phase_offset = is_wave_b
            ? (3.14159265f / static_cast<float>(total_slots))
            : 0.0f;

        spawn_one(boss_x, boss_y, shot_dx, shot_dy, shot_px, shot_py,
            base_phase + wave_phase_offset);
    }

    m_wave_toggle = !m_wave_toggle;
}

// Returns the first inactive pool slot; grows the pool if completely full
BossProjectile* BossProjectileManager::acquire_slot()
{
    for (auto& p : m_pool)
        if (!p.in_use) return &p;
    m_pool.emplace_back();
    return &m_pool.back();
}

// Initialises one projectile slot with the given direction, perp axis, and phase
void BossProjectileManager::spawn_one(float bx, float by,
    float dir_x, float dir_y,
    float perp_x, float perp_y,
    float phase)
{
    BossProjectile* p = acquire_slot();
    *p = BossProjectile{};
    p->x = bx;
    p->y = by;
    p->vx = dir_x * e_bp_base_speed;
    p->vy = dir_y * e_bp_base_speed;
    p->perp_x = perp_x;
    p->perp_y = perp_y;
    p->phase = phase;
    p->lifetime = e_bp_max_lifetime;
    p->in_use = true;
}

// Returns the number of currently active projectiles in the pool
int BossProjectileManager::get_active_count() const
{
    int n = 0;
    for (const auto& p : m_pool) if (p.in_use) ++n;
    return n;
}


// ============================================================================
// PATHFINDING HELPERS
// ============================================================================

bool g_enemy_debug_draw = false;

// ----------------------------------------------------------------------------
// path_is_clear() — samples 10 points along the line from (x,y) to (tx,ty).
// Returns false if any sample point collides with a wall.
// ----------------------------------------------------------------------------
static bool path_is_clear(f32 x, f32 y, f32 tx, f32 ty, f32 half)
{
    for (int i = 1; i <= 10; ++i)
    {
        f32 t = i / 10.f;
        if (check_player_collision(x + (tx - x) * t, y + (ty - y) * t, half))
            return false;
    }
    return true;
}

// ----------------------------------------------------------------------------
// enemy_move_towards() — steps toward (target_x, target_y) this frame.
// Applies X and Y movement independently so the enemy slides along walls.
// ----------------------------------------------------------------------------
void enemy_move_towards(f32& x, f32& y, f32 speed, f32 size,
    f32 target_x, f32 target_y, f32 dt)
{
    f32 dx = target_x - x, dy = target_y - y;
    f32 dist = sqrtf(dx * dx + dy * dy);
    if (dist <= 1.0f) return;

    dx /= dist; dy /= dist;
    const f32 half = size * 0.5f;
    const f32 step = speed * dt;

    // Probe at the leading edge; skip axis if it would enter a wall
    f32 probe_x = x + (dx > 0 ? half : -half);
    f32 probe_y = y + (dy > 0 ? half : -half);

    if (!check_player_collision(probe_x + dx * step, y, 2.f)) x += dx * step;
    if (!check_player_collision(x, probe_y + dy * step, 2.f)) y += dy * step;
}

// ----------------------------------------------------------------------------
// enemy_move_away_from() — steps directly away from (from_x, from_y).
// Applies X and Y independently to allow wall sliding while fleeing.
// ----------------------------------------------------------------------------
void enemy_move_away_from(f32& x, f32& y, f32 speed, f32 size,
    f32 from_x, f32 from_y, f32 dt)
{
    f32 dx = x - from_x, dy = y - from_y;
    f32 dist = sqrtf(dx * dx + dy * dy);
    if (dist <= 1.0f) return;
    dx /= dist; dy /= dist;
    f32 nx = x + dx * speed * dt;
    f32 ny = y + dy * speed * dt;
    if (!check_player_collision(nx, y, size / 2.0f)) x = nx;
    if (!check_player_collision(x, ny, size / 2.0f)) y = ny;
}

// ----------------------------------------------------------------------------
// enemy_flee_to_wall() — computes a flee destination in the direction
// opposite the player. Caller applies wall-collision checks as needed.
// ----------------------------------------------------------------------------
void enemy_flee_to_wall(f32 x, f32 y, f32 speed,
    f32 player_x, f32 player_y, f32 dt,
    f32& next_x, f32& next_y)
{
    f32 dx = x - player_x, dy = y - player_y;
    f32 dist = sqrtf(dx * dx + dy * dy);
    if (dist < 1.0f) { dx = 1.0f; dy = 0.0f; }
    else { dx /= dist; dy /= dist; }
    next_x = x + dx * speed * dt;
    next_y = y + dy * speed * dt;
}


// ============================================================================
// BASE ENEMY
// ============================================================================

// Sets common member defaults; subclasses override speed/size/health after this
base_enemy::base_enemy(f32 start_x, f32 start_y)
    : x(start_x), y(start_y), spawn_x(start_x), spawn_y(start_y),
    velocity_x(0), velocity_y(0), speed(50.f), size(25.f),
    current_health(2), max_health(2), attack_cooldown(1.f), attack_timer(0.f),
    state(enemy_state::idle), active(true), visible_to_player(false),
    collision_radius(15.f), sight_range(300.f),
    texture(nullptr), food_texture(nullptr)
{
}

// Applies damage, triggers hit flash, and sets dead state at 0 HP
void base_enemy::take_damage(int amount)
{
    if (!active) return;
    current_health -= amount;
    hit_flash_timer = HIT_FLASH_DUR;
    if (current_health <= 0)
    {
        current_health = 0;
        state = enemy_state::dead;
        active = false;
    }
}

// Returns true if the enemy's bounding box overlaps the camera viewport
bool base_enemy::is_in_camera_view(Camera::camera* p_cam) const
{
    if (!p_cam) return false;
    f32 hw = (AEGfxGetWindowWidth() / 2.0f) / Camera::zoom;
    f32 hh = (AEGfxGetWindowHeight() / 2.0f) / Camera::zoom;
    return (x > p_cam->pos.x - hw - size && x < p_cam->pos.x + hw + size &&
        y > p_cam->pos.y - hh - size && y < p_cam->pos.y + hh + size);
}

// Returns true if the enemy's collision radius overlaps the player's
bool base_enemy::collides_with_player(Player* player) const
{
    if (!player || !player->current_player) return false;
    f32 dx = x - player->current_player->x;
    f32 dy = y - player->current_player->y;
    return sqrtf(dx * dx + dy * dy) < (collision_radius + player->stats.character_size / 2.f);
}

// Returns straight-line distance to the player; 10000 if player is null
f32 base_enemy::distance_to_player(Player* player) const
{
    if (!player || !player->current_player) return 10000.f;
    f32 dx = x - player->current_player->x;
    f32 dy = y - player->current_player->y;
    return sqrtf(dx * dx + dy * dy);
}


// ============================================================================
// GHOST
// ============================================================================

// Initialises ghost stats from config globals
ghost::ghost(f32 start_x, f32 start_y)
    : base_enemy(start_x, start_y), hits_from_player(0)
{
    speed = e_ghost_speed;
    current_health = e_ghost_max_health;
    max_health = e_ghost_max_health;
    size = e_ghost_size;
    sight_range = e_ghost_sight_range;
}

// Increments hit counter before delegating to base damage logic
void ghost::take_damage(int amount)
{
    if (!active) return;
    hits_from_player++;
    base_enemy::take_damage(amount);
}

// ----------------------------------------------------------------------------
// Ghost AI: flees toward the nearest wall while the player is in camera view;
// returns to idle when off-screen. Dies if it walks into a wall during flee.
// ----------------------------------------------------------------------------
void ghost::update(f32 dt, Player* player, Camera::camera* p_cam)
{
    if (!active) return;
    if (hit_flash_timer > 0.f) hit_flash_timer -= dt;
    visible_to_player = is_in_camera_view(p_cam);

    if (visible_to_player)
    {
        state = enemy_state::fleeing;
        f32 next_x{}, next_y{};
        enemy_flee_to_wall(x, y, speed,
            player->current_player->x, player->current_player->y,
            dt, next_x, next_y);

        // Kill ghost if it collides with a wall while fleeing
        if (!check_player_collision(next_x, y, size / 2.0f)) x = next_x;
        else { state = enemy_state::dead; active = false; }

        if (!check_player_collision(x, next_y, size / 2.0f)) y = next_y;
        else { state = enemy_state::dead; active = false; }
    }
    else state = enemy_state::idle;
}


// ============================================================================
// BAT
// ============================================================================

// Initialises bat stats from config globals
bat::bat(f32 start_x, f32 start_y)
    : base_enemy(start_x, start_y)
{
    speed = e_bat_speed;
    current_health = e_bat_max_health;
    max_health = e_bat_max_health;
    size = e_bat_size;
    sight_range = e_bat_sight_range;
    attack_cooldown = e_bat_attack_cooldown;
}

// ----------------------------------------------------------------------------
// Bat AI: chases the player when in camera view; deals damage on contact
// using the attack cooldown timer to pace repeated hits.
// ----------------------------------------------------------------------------
void bat::update(f32 dt, Player* player, Camera::camera* p_cam)
{
    if (!active) return;
    if (hit_flash_timer > 0.f) hit_flash_timer -= dt;
    visible_to_player = is_in_camera_view(p_cam);

    if (visible_to_player)
    {
        state = enemy_state::chasing;
        enemy_move_towards(x, y, speed, size,
            player->current_player->x, player->current_player->y, dt);

        if (collides_with_player(player))
        {
            state = enemy_state::attacking;
            attack_timer += dt;
            if (attack_timer >= attack_cooldown)
            {
                player_take_damage(player, e_bat_attack_damage);
                attack_timer = 0.0f;
            }
        }
    }
    else state = enemy_state::idle;
}


// ============================================================================
// DRAGON
// ============================================================================

// Initialises boss stats and stores the room boundary for movement clamping
dragon::dragon(f32 start_x, f32 start_y,
    f32 r_left, f32 r_right, f32 r_top, f32 r_bottom)
    : base_enemy(start_x, start_y),
    room_left(r_left), room_right(r_right), room_top(r_top), room_bottom(r_bottom),
    hits_received(0), is_boss(true)
{
    speed = e_dragon_speed;
    current_health = e_dragon_max_health;
    max_health = e_dragon_max_health;
    size = e_dragon_size;
    attack_cooldown = e_dragon_attack_cooldown;
}

// Loads all five animation sheets from config paths and starts the idle clip
void dragon::load_anim_textures()
{
    tex_idle = AEGfxTextureLoad(e_dragon_tex_idle.c_str());
    tex_walk = AEGfxTextureLoad(e_dragon_tex_walk.c_str());
    tex_attack = AEGfxTextureLoad(e_dragon_tex_attack.c_str());
    tex_hurt = AEGfxTextureLoad(e_dragon_tex_hurt.c_str());
    tex_death = AEGfxTextureLoad(e_dragon_tex_death.c_str());

    anim.set(tex_idle,
        e_dragon_idle_frames,
        e_dragon_idle_frame_w, e_dragon_idle_frame_h,
        e_dragon_idle_sheet_w, e_dragon_idle_sheet_h,
        e_dragon_idle_fps,
        e_dragon_idle_loop != 0);
}

// Counts down m_wave_timer and fires a projectile wave when it expires
void dragon::tick_attack_pattern(float dt, float tgt_x, float tgt_y)
{
    m_wave_timer -= dt;
    if (m_wave_timer <= 1.f)
    {
        boss_proj_manager.trigger_wave(x, y, tgt_x, tgt_y);
        m_wave_timer = e_bp_wave_interval;
    }
}

// ----------------------------------------------------------------------------
// Dragon AI: chases the player within the room boundary, fires projectile
// waves on a timer, deals melee damage on contact, and manages animation
// transitions between IDLE, WALK, ATTACK, HURT, and DEATH clips.
// ----------------------------------------------------------------------------
void dragon::update(f32 dt, Player* player, Camera::camera* p_cam)
{
    if (!active) return;
    if (hit_flash_timer > 0.f) hit_flash_timer -= dt;

    // Let the death animation finish before deactivating
    if (state == dead) {
        if (anim.update(dt)) active = false;
        return;
    }

    visible_to_player = is_in_camera_view(p_cam);
    bool moving = false, hitting = false;

    if (visible_to_player)
    {
        state = enemy_state::chasing;

        // Clamp the target so the dragon stays within the boss room
        f32 target_x = AEClamp(player->current_player->x, room_left, room_right);
        f32 target_y = AEClamp(player->current_player->y, room_bottom, room_top);

        f32 dx = target_x - x, dy = target_y - y;
        moving = (sqrtf(dx * dx + dy * dy) > 4.f);

        enemy_move_towards(x, y, speed, size, target_x, target_y, dt);
        x = AEClamp(x, room_left, room_right);
        y = AEClamp(y, room_bottom, room_top);

        tick_attack_pattern(dt, player->current_player->x, player->current_player->y);

        if (collides_with_player(player))
        {
            state = enemy_state::attacking;
            hitting = true;
            attack_timer += dt;
            if (attack_timer >= attack_cooldown)
            {
                player_take_damage(player, e_dragon_attack_damage);
                attack_timer = 0.f;
            }
        }
        else attack_timer = 0.f;
    }
    else state = enemy_state::idle;

    // Animation priority: let HURT/ATTACK finish before switching clips
    if (current_anim == DragonAnim::HURT)
    {
        if (anim.update(dt)) switch_anim(moving ? DragonAnim::WALK : DragonAnim::IDLE);
    }
    else if (current_anim == DragonAnim::ATTACK)
    {
        if (anim.update(dt)) switch_anim(moving ? DragonAnim::WALK : DragonAnim::IDLE);
    }
    else
    {
        anim.update(dt);
        if (hitting)     switch_anim(DragonAnim::ATTACK);
        else if (moving) switch_anim(DragonAnim::WALK);
        else             switch_anim(DragonAnim::IDLE);
    }
}

// Reconfigures the Animator for the requested clip; no-op if already playing it
void dragon::switch_anim(DragonAnim next_anim)
{
    if (current_anim == next_anim) return;
    current_anim = next_anim;

    switch (next_anim) {
    case DragonAnim::IDLE:
        anim.set(tex_idle, e_dragon_idle_frames,
            e_dragon_idle_frame_w, e_dragon_idle_frame_h,
            e_dragon_idle_sheet_w, e_dragon_idle_sheet_h,
            e_dragon_idle_fps, e_dragon_idle_loop != 0);
        break;
    case DragonAnim::WALK:
        anim.set(tex_walk, e_dragon_walk_frames,
            e_dragon_walk_frame_w, e_dragon_walk_frame_h,
            e_dragon_walk_sheet_w, e_dragon_walk_sheet_h,
            e_dragon_walk_fps, e_dragon_walk_loop != 0);
        break;
    case DragonAnim::ATTACK:
        anim.set(tex_attack, e_dragon_attack_frames,
            e_dragon_attack_frame_w, e_dragon_attack_frame_h,
            e_dragon_attack_sheet_w, e_dragon_attack_sheet_h,
            e_dragon_attack_fps, e_dragon_attack_loop != 0);
        break;
    case DragonAnim::HURT:
        anim.set(tex_hurt, e_dragon_hurt_frames,
            e_dragon_hurt_frame_w, e_dragon_hurt_frame_h,
            e_dragon_hurt_sheet_w, e_dragon_hurt_sheet_h,
            e_dragon_hurt_fps, e_dragon_hurt_loop != 0);
        break;
    case DragonAnim::DEATH:
        anim.set(tex_death, e_dragon_death_frames,
            e_dragon_death_frame_w, e_dragon_death_frame_h,
            e_dragon_death_sheet_w, e_dragon_death_sheet_h,
            e_dragon_death_fps, e_dragon_death_loop != 0);
        break;
    }
}

// Draws the dragon sprite at the correct aspect ratio for the current clip
void dragon::render_animated(f32 draw_size)
{
    f32 w, h;
    switch (current_anim) {
    case DragonAnim::ATTACK: w = draw_size * (e_dragon_attack_frame_w / e_dragon_attack_frame_h); h = draw_size; break;
    case DragonAnim::DEATH:  w = draw_size * (e_dragon_death_sheet_w / e_dragon_death_frame_h);  h = draw_size; break;
    default:                 w = draw_size; h = draw_size; break;
    }
    render_anim_sprite(x, y, w, h, anim);
}

// ----------------------------------------------------------------------------
// dragon::take_damage() — applies damage and switches to HURT or DEATH clip.
// Does not use base_enemy::take_damage so the dragon controls its own anim.
// ----------------------------------------------------------------------------
void dragon::take_damage(int amount)
{
    if (!active) return;
    hits_received++;
    current_health -= amount;
    if (current_health <= 0)
    {
        current_health = 0;
        state = enemy_state::dead;
        switch_anim(DragonAnim::DEATH);
    }
    else switch_anim(DragonAnim::HURT);
}