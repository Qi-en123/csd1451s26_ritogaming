/*!************************************************************************
\file:          enemy.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu

\brief:
    Core enemy system definitions.

    Includes:
    - Base enemy class (shared data + behavior)
    - Enemy types: ghost, bat, dragon (boss)
    - Boss projectile system
    - Movement helper utilities

    Use this file as the main reference for:
    - Enemy tuning (speed, damage, detection, etc.)
    - Boss attack configuration
    - Extending new enemy types
**************************************************************************/

#pragma once

#include <cmath>
#include <vector>
#include <cstring>
#include "AEEngine.h"
#include "camera.hpp"


// ============================================================================
// BOSS PROJECTILE CONFIG
// ----------------------------------------------------------------------------
// Centralized tuning values for boss attacks.
// Safe to modify for gameplay balancing (difficulty, pacing, feel).
// ============================================================================
namespace BossProjCfg
{
    constexpr float RING_RADIUS = 10.f;        // Hitbox + visual size
    constexpr float RING_THICKNESS = 3.f;      // Visual only

    constexpr float BASE_SPEED = 220.f;        // Forward velocity
    constexpr float WAVE_AMPLITUDE = 80.f;     // Sideways wave distance
    constexpr float WAVE_FREQUENCY = 10.f;     // Wave oscillation speed

    constexpr float MAX_LIFETIME = 5.f;        // Seconds before despawn

    // Pattern layout (alternating waves)
    constexpr int   POSITIONS_WAVE_A = 5;      // Odd slots
    constexpr int   POSITIONS_WAVE_B = 4;      // Even slots
    constexpr float ANGLE_SPREAD_DEG = 120.0f; // Total fan spread

    constexpr float WAVE_INTERVAL = 0.45f;     // Delay between waves

    constexpr int   PROJECTILE_DAMAGE = 1;     // 1 = half-heart
}


// ============================================================================
// BossProjectile
// ----------------------------------------------------------------------------
// Represents a single projectile instance.
// Motion = forward direction + sine offset along perpendicular axis.
// ============================================================================
struct BossProjectile
{
    float x{}, y{};            // World position

    float vx{}, vy{};          // Forward velocity
    float perp_x{}, perp_y{};  // Perpendicular axis (for wave motion)

    float phase{};             // Unique offset (prevents uniform motion)
    float age{};               // Time alive
    float lifetime{ BossProjCfg::MAX_LIFETIME };

    bool  in_use{ false };     // Object pool flag
};


// ============================================================================
// BossProjectileManager
// ----------------------------------------------------------------------------
// Responsibilities:
// - Manages projectile pool (no runtime allocations)
// - Spawns attack patterns
// - Updates movement and lifetime
// - Handles player collision
// ============================================================================
struct Player;

class BossProjectileManager
{
public:
    BossProjectileManager();

    /// Initialize projectile pool (call once on boss setup)
    void init(int pool_size = 128);

    /// Reset all projectiles (call when boss resets/despawns)
    void clear();

    /// Update movement, lifetime, and camera culling
    void update(float dt, const Camera::camera* cam);

    /// Check collision against player; returns number of hits this frame
    int  check_player_collision(Player* player);

    /// Spawn a projectile wave aimed at target position
    void trigger_wave(float boss_x, float boss_y,
        float target_x, float target_y);

    const std::vector<BossProjectile>& get_projectiles() const { return m_pool; }
    int  get_active_count() const;

private:
    /// Get an unused projectile from pool
    BossProjectile* acquire_slot();

    /// Initialize a projectile instance with movement parameters
    void spawn_one(float bx, float by,
        float dir_x, float dir_y,
        float perp_x, float perp_y,
        float phase);

    std::vector<BossProjectile> m_pool;

    bool m_wave_toggle{ false }; // Alternates between wave patterns
};


// Global projectile system (shared by boss logic)
extern BossProjectileManager boss_proj_manager;


// ============================================================================
// Gameplay Constants
// ============================================================================
constexpr int player_heart_damage{ 50 };     // Damage applied to player
constexpr f32 food_health_percent{ 0.50f }; // % heal from food pickup
constexpr f32 food_lifetime{ 10.f };        // Seconds before despawn


// ============================================================================
// Movement Helpers
// ----------------------------------------------------------------------------
// Wall-aware movement utilities.
// Use these instead of directly modifying position to avoid clipping.
// ============================================================================

/// Move toward target while respecting collision boundaries
void enemy_move_towards(f32& x, f32& y, f32 speed, f32 size,
    f32 target_x, f32 target_y, f32 dt);

/// Move away from source while respecting collision boundaries
void enemy_move_away_from(f32& x, f32& y, f32 speed, f32 size,
    f32 from_x, f32 from_y, f32 dt);

/// Compute flee position (does NOT apply movement)
void enemy_flee_to_wall(f32 x, f32 y, f32 speed,
    f32 player_x, f32 player_y, f32 dt,
    f32& next_x, f32& next_y);


// ============================================================================
// Enemy State Machine
// ============================================================================
enum enemy_state
{
    idle,
    chasing,
    fleeing,
    attacking,
    dead
};


// ============================================================================
// Base Enemy Class
// ----------------------------------------------------------------------------
// Shared data and behavior for all enemies.
// Derive from this class to implement custom enemy logic.
// ============================================================================
class base_enemy
{
public:
    static constexpr int enemy_max_hp{ 100 };
    static constexpr int enemy_damage_per_hit{ 50 };

protected:
    // --- Transform ---
    f32 x, y;
    f32 spawn_x, spawn_y;

    // --- Movement ---
    f32 velocity_x, velocity_y;
    f32 speed;
    f32 size;

    // --- Health ---
    int current_health;
    int max_health;

    // --- Combat ---
    f32 attack_cooldown;
    f32 attack_timer;

    // --- State ---
    enemy_state state;
    bool active;
    bool visible_to_player;

    // --- Detection ---
    f32 collision_radius;
    f32 sight_range;

    // --- Rendering ---
    AEGfxTexture* texture;

    // --- Drops ---
    AEGfxTexture* food_texture;
    bool food_spawned{ false };

public:
    base_enemy(f32 start_x, f32 start_y);
    virtual ~base_enemy() {}

    /// Per-frame update (must be implemented by derived classes)
    virtual void update(f32 dt, struct Player* player, Camera::camera* cam) = 0;

    /// Apply damage and handle death state
    virtual void take_damage(int amount);

    virtual bool is_ghost() const { return false; }
    virtual int get_max_health() const { return max_health; }

    // --- Utility ---
    bool is_in_camera_view(Camera::camera* cam) const;
    bool collides_with_player(struct Player* player) const;
    f32 distance_to_player(struct Player* player) const;

    // --- Drops ---
    bool has_food_spawned() const { return food_spawned; }
    void set_food_spawned(bool val) { food_spawned = val; }

    // --- Getters ---
    f32 get_x() const { return x; }
    f32 get_y() const { return y; }
    f32 get_size() const { return size; }
    int get_current_health() const { return current_health; }
    enemy_state get_state() const { return state; }
    bool is_active() const { return active; }

    AEGfxTexture* get_texture() const { return texture; }
    AEGfxTexture* get_food_texture() const { return food_texture; }

    void set_active(bool val) { active = val; }

    void set_position(f32 new_x, f32 new_y)
    {
        x = new_x;
        y = new_y;
    }

    void set_enemy_texture(AEGfxTexture* tex) { texture = tex; }
    void set_food_texture(AEGfxTexture* tex) { food_texture = tex; }
};


// ============================================================================
// GHOST
// ----------------------------------------------------------------------------
// Behavior:
// - Flees immediately when visible to player
// - Tracks number of hits taken from player
// ============================================================================
class ghost : public base_enemy
{
    int hits_from_player;

public:
    ghost(f32 start_x, f32 start_y);
    ~ghost() override = default;

    void update(f32 dt, struct Player* player, Camera::camera* cam) override;
    void take_damage(int amount) override;

    bool is_ghost() const override { return true; }
    int get_hits_from_player() const { return hits_from_player; }
};


// ============================================================================
// BAT
// ----------------------------------------------------------------------------
// Behavior:
// - Actively chases player
// - Can attack but is not required to defeat
// ============================================================================
class bat : public base_enemy
{
public:
    bat(f32 start_x, f32 start_y);
    ~bat() override = default;

    void update(f32 dt, struct Player* player, Camera::camera* cam) override;
};


// ============================================================================
// Animator
// ----------------------------------------------------------------------------
// Handles sprite sheet animation playback.
// Supports looping and one-shot animations.
// ============================================================================
struct Animator {
    AEGfxTexture* sheet = nullptr;

    int   frame_count = 1;
    int   current_frame = 0;

    float frame_w = 1.f;
    float frame_h = 1.f;
    float sheet_w = 1.f;
    float sheet_h = 1.f;

    float fps = 8.f;
    float timer = 0.f;

    bool  looping = true;
    bool  finished = false; // true when non-looping animation completes

    void set(AEGfxTexture* tex,
        int frames, float fw, float fh, float sw, float sh,
        float playback_fps, bool loop)
    {
        sheet = tex;
        frame_count = frames;
        frame_w = fw;  frame_h = fh;
        sheet_w = sw;  sheet_h = sh;
        fps = playback_fps;
        looping = loop;

        current_frame = 0;
        timer = 0.f;
        finished = false;
    }

    /// Advance animation; returns true if a one-shot animation just finished
    bool update(float dt)
    {
        if (finished) return false;

        timer += dt;
        if (timer >= 1.f / fps) {
            timer -= 1.f / fps;
            current_frame++;

            if (current_frame >= frame_count) {
                if (looping) current_frame = 0;
                else {
                    current_frame = frame_count - 1;
                    finished = true;
                    return true;
                }
            }
        }
        return false;
    }

    // UV coordinates (0–1 range)
    float u0() const { return (current_frame * frame_w) / sheet_w; }
    float u1() const { return u0() + frame_w / sheet_w; }
    float v0() const { return 0.f; }
    float v1() const { return frame_h / sheet_h; }
};


// ============================================================================
// DRAGON (Boss)
// ----------------------------------------------------------------------------
// Behavior:
// - Confined within room bounds
// - Uses animation state machine
// - Fires projectile waves at player
// ============================================================================
class dragon : public base_enemy {
private:
    // --- Room bounds (movement limits) ---
    f32 room_left, room_right, room_top, room_bottom;

    // --- Combat tracking ---
    s32 hits_received;
    s32 hits_until_attack;
    bool is_boss;

    // --- Animation textures ---
    AEGfxTexture* tex_idle = nullptr;
    AEGfxTexture* tex_walk = nullptr;
    AEGfxTexture* tex_attack = nullptr;
    AEGfxTexture* tex_hurt = nullptr;
    AEGfxTexture* tex_death = nullptr;

    Animator anim;

    enum class DragonAnim { IDLE, WALK, ATTACK, HURT, DEATH };
    DragonAnim current_anim = DragonAnim::IDLE;

    // --- Attack timing ---
    float m_wave_timer{ 3.f }; // Controls projectile wave intervals

    void switch_anim(DragonAnim next_anim);
    void tick_attack_pattern(float dt, float tgt_x, float tgt_y);

public:
    dragon(f32 start_x, f32 start_y,
        f32 r_left, f32 r_right, f32 r_top, f32 r_bottom);
    ~dragon() override = default;

    void update(f32 dt, struct Player* player, Camera::camera* cam) override;
    void take_damage(int amount) override;

    /// Returns true when boss is defeated and reward should spawn
    bool has_dropped_sister() const { return is_boss && state == dead; }

    void load_anim_textures();
    void render_animated(f32 draw_size);
};