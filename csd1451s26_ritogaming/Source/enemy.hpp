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
    - Global config variables loaded from enemy_data.txt via dave_Data()

    Use this file as the main reference for:
    - Enemy tuning (speed, damage, detection, etc.)
    - Boss attack configuration
    - Extending new enemy types
**************************************************************************/

#pragma once

#include <cmath>
#include <vector>
#include <string>
#include <cstring>
#include "AEEngine.h"
#include "camera.hpp"


// ============================================================================
// ENEMY DATA  —  loaded from Assets/Config/enemy_data.txt by dave_Data()
// ----------------------------------------------------------------------------
// All values below are populated once at startup.
// enemy_manager.cpp (and any other file) can use them directly via extern.
// Do NOT reassign these anywhere except inside dave_Data().
// ============================================================================

// --- Texture filepaths ---
extern std::string e_ghost_tex;
extern std::string e_bat_tex;
extern std::string e_ghost_food_tex;
extern std::string e_bat_food_tex;
extern std::string e_cross_tex;
extern std::string e_ghost_marker_tex;
extern std::string e_bat_marker_tex;
extern std::string e_dragon_tex_idle;
extern std::string e_dragon_tex_walk;
extern std::string e_dragon_tex_attack;
extern std::string e_dragon_tex_hurt;
extern std::string e_dragon_tex_death;

// --- Ghost stats ---
extern float e_ghost_speed;
extern int   e_ghost_max_health;
extern float e_ghost_size;
extern float e_ghost_sight_range;

// --- Bat stats ---
extern float e_bat_speed;
extern int   e_bat_max_health;
extern float e_bat_size;
extern float e_bat_sight_range;
extern float e_bat_attack_cooldown;
extern int   e_bat_attack_damage;

// --- Dragon stats ---
extern float e_dragon_speed;
extern int   e_dragon_max_health;
extern float e_dragon_size;
extern float e_dragon_attack_cooldown;
extern int   e_dragon_attack_damage;

// --- Boss projectile ---
extern float e_bp_ring_radius;
extern float e_bp_ring_thickness;
extern float e_bp_base_speed;
extern float e_bp_wave_amplitude;
extern float e_bp_wave_frequency;
extern float e_bp_max_lifetime;
extern int   e_bp_positions_wave_a;
extern int   e_bp_positions_wave_b;
extern float e_bp_angle_spread_deg;
extern float e_bp_wave_interval;
extern int   e_bp_projectile_damage;
extern int   e_bp_pool_size;

// --- Manager / world ---
extern float e_spawn_check_interval;
extern float e_respawn_delay;
extern float e_food_lifetime;
extern float e_food_size;
extern float e_map_start_x;
extern float e_map_start_y;
extern float e_tile_size;

// --- Respawn particles ---
extern int   e_rp_burst_count;
extern float e_rp_speed_min;
extern float e_rp_speed_max;
extern float e_rp_life_min;
extern float e_rp_life_max;
extern float e_rp_size_min;
extern float e_rp_size_max;
extern float e_rp_gravity;
extern float e_rp_rise_boost;

/**************************************************************************
    \brief
    Reads enemy_data.txt and assigns every token to its matching global
    variable. Uses ifstream + getline + istringstream for tokenization.
    Call once at startup before any enemy or EnemyManager is constructed.

    \param filepath  Path to the data file
                     (e.g. "Assets/Config/enemy_data.txt").
    \return True if the file was opened and fully parsed,
            false if the file could not be opened.
**************************************************************************/
bool save_Data(const std::string& filepath);


// ============================================================================
// BossProjectile  (unchanged)
// ============================================================================
struct BossProjectile
{
    float x{}, y{};
    float vx{}, vy{};
    float perp_x{}, perp_y{};
    float phase{};
    float age{};
    float lifetime{};
    bool  in_use{ false };
};

struct Player;

class BossProjectileManager
{
public:
    BossProjectileManager();

    void init(int pool_size = 0);   // 0 = use e_bp_pool_size from data file
    void clear();
    void update(float dt, const Camera::camera* cam);
    int  check_player_collision(Player* player);
    void trigger_wave(float boss_x, float boss_y,
        float target_x, float target_y);

    const std::vector<BossProjectile>& get_projectiles() const { return m_pool; }
    int  get_active_count() const;

private:
    BossProjectile* acquire_slot();
    void spawn_one(float bx, float by,
        float dir_x, float dir_y,
        float perp_x, float perp_y,
        float phase);

    std::vector<BossProjectile> m_pool;
    bool m_wave_toggle{ false };
};

extern BossProjectileManager boss_proj_manager;


// ============================================================================
// Gameplay constants  (unchanged)
// ============================================================================
constexpr int player_heart_damage{ 50 };
constexpr f32 food_health_percent{ 0.50f };
constexpr f32 food_lifetime{ 10.f };


// ============================================================================
// Movement helpers  (unchanged)
// ============================================================================
void enemy_move_towards(f32& x, f32& y, f32 speed, f32 size,
    f32 target_x, f32 target_y, f32 dt);
void enemy_move_away_from(f32& x, f32& y, f32 speed, f32 size,
    f32 from_x, f32 from_y, f32 dt);
void enemy_flee_to_wall(f32 x, f32 y, f32 speed,
    f32 player_x, f32 player_y, f32 dt,
    f32& next_x, f32& next_y);


// ============================================================================
// Enemy state machine  (unchanged)
// ============================================================================
enum enemy_state { idle, chasing, fleeing, attacking, dead };


// ============================================================================
// Base enemy class  (unchanged)
// ============================================================================
class base_enemy
{
public:
    static constexpr int enemy_max_hp{ 100 };
    static constexpr int enemy_damage_per_hit{ 50 };

protected:
    f32 x, y;
    f32 spawn_x, spawn_y;
    f32 velocity_x, velocity_y;
    f32 speed;
    f32 size;
    int current_health;
    int max_health;
    f32 attack_cooldown;
    f32 attack_timer;
    enemy_state state;
    bool active;
    bool visible_to_player;
    f32 collision_radius;
    f32 sight_range;
    AEGfxTexture* texture;
    AEGfxTexture* food_texture;
    bool food_spawned{ false };
    f32 hit_flash_timer = 0.f;
    static constexpr f32 HIT_FLASH_DUR = 0.15f;

public:
    base_enemy(f32 start_x, f32 start_y);
    virtual ~base_enemy() {}

    virtual void update(f32 dt, struct Player* player, Camera::camera* cam) = 0;
    virtual void take_damage(int amount);
    virtual bool is_ghost() const { return false; }
    virtual int  get_max_health() const { return max_health; }

    bool is_in_camera_view(Camera::camera* cam) const;
    bool collides_with_player(struct Player* player) const;
    f32  distance_to_player(struct Player* player) const;

    bool has_food_spawned() const { return food_spawned; }
    void set_food_spawned(bool val) { food_spawned = val; }

    f32           get_x()              const { return x; }
    f32           get_y()              const { return y; }
    f32           get_size()           const { return size; }
    int           get_current_health() const { return current_health; }
    enemy_state   get_state()          const { return state; }
    bool          is_active()          const { return active; }
    f32           get_hit_flash_timer()const { return hit_flash_timer; }
    AEGfxTexture* get_texture()        const { return texture; }
    AEGfxTexture* get_food_texture()   const { return food_texture; }

    void set_active(bool val) { active = val; }
    void set_position(f32 nx, f32 ny) { x = nx; y = ny; }
    void set_enemy_texture(AEGfxTexture* tex) { texture = tex; }
    void set_food_texture(AEGfxTexture* tex) { food_texture = tex; }
};


// ============================================================================
// Ghost  (unchanged interface)
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
    int  get_hits_from_player() const { return hits_from_player; }
};


// ============================================================================
// Bat  (unchanged interface)
// ============================================================================
class bat : public base_enemy
{
public:
    bat(f32 start_x, f32 start_y);
    ~bat() override = default;
    void update(f32 dt, struct Player* player, Camera::camera* cam) override;
};


// ============================================================================
// Animator  (unchanged)
// ============================================================================
struct Animator {
    AEGfxTexture* sheet = nullptr;
    AEGfxVertexList* cached_mesh = nullptr;
    int  last_built_frame = -1;
    int  frame_count = 1;
    int  current_frame = 0;
    float frame_w = 1.f, frame_h = 1.f;
    float sheet_w = 1.f, sheet_h = 1.f;
    float fps = 8.f;
    float timer = 0.f;
    bool  looping = true;
    bool  finished = false;

    void free_mesh()
    {
        if (cached_mesh) { AEGfxMeshFree(cached_mesh); cached_mesh = nullptr; }
        last_built_frame = -1;
    }

    bool update(float dt)
    {
        if (finished) return false;
        timer += dt;
        if (timer >= 1.f / fps) {
            timer -= 1.f / fps;
            current_frame++;
            if (current_frame >= frame_count) {
                if (looping) current_frame = 0;
                else { current_frame = frame_count - 1; finished = true; return true; }
            }
        }
        return false;
    }

    void set(AEGfxTexture* tex, int frames,
        float fw, float fh, float sw, float sh,
        float playback_fps, bool loop)
    {
        sheet = tex; frame_count = frames;
        frame_w = fw; frame_h = fh; sheet_w = sw; sheet_h = sh;
        fps = playback_fps; looping = loop;
        current_frame = 0; timer = 0.f; finished = false;
    }

    float u0() const { return (current_frame * frame_w) / sheet_w; }
    float u1() const { return u0() + frame_w / sheet_w; }
    float v0() const { return 0.f; }
    float v1() const { return frame_h / sheet_h; }
};


// ============================================================================
// Dragon (boss)  (unchanged interface)
// ============================================================================
class dragon : public base_enemy {
private:
    f32 room_left, room_right, room_top, room_bottom;
    s32 hits_received;
    s32 hits_until_attack;
    bool is_boss;

    AEGfxTexture* tex_idle = nullptr;
    AEGfxTexture* tex_walk = nullptr;
    AEGfxTexture* tex_attack = nullptr;
    AEGfxTexture* tex_hurt = nullptr;
    AEGfxTexture* tex_death = nullptr;

    Animator anim;

    enum class DragonAnim { IDLE, WALK, ATTACK, HURT, DEATH };
    DragonAnim current_anim = DragonAnim::IDLE;

    float m_wave_timer{ 3.f };

    void switch_anim(DragonAnim next_anim);
    void tick_attack_pattern(float dt, float tgt_x, float tgt_y);

public:
    dragon(f32 start_x, f32 start_y,
        f32 r_left, f32 r_right, f32 r_top, f32 r_bottom);
    ~dragon() override = default;

    void update(f32 dt, struct Player* player, Camera::camera* cam) override;
    void take_damage(int amount) override;
    bool has_dropped_sister() const { return is_boss && state == dead; }
    void load_anim_textures();
    void render_animated(f32 draw_size);
};