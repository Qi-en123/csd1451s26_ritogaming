/*!************************************************************************
\file:          enemy.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         Enemy base class and derived enemy types
**************************************************************************/

#pragma once

#include <cmath>
#include <vector>
#include <cstring> 
#include "AEEngine.h"
#include "camera.hpp"

constexpr int  player_heart_damage{ 50 };
constexpr f32  food_health_percent{ 0.50f };
constexpr f32  food_lifetime{ 10.f };

//------------------------------- Pathfinding --------------------------------
// Move position (x, y) toward a target, wall-checking along each axis
void enemy_move_towards(f32& x, f32& y, f32 speed, f32 size, f32 target_x, f32 target_y, f32 dt);

// Move position (x, y) away from a source, wall-checking along each axis
void enemy_move_away_from(f32& x, f32& y, f32 speed, f32 size, f32 from_x, f32 from_y, f32 dt);

// Compute the next flee position (caller decides whether to apply it)
void enemy_flee_to_wall(f32 x, f32 y, f32 speed, f32 player_x, f32 player_y, f32 dt,
    f32& next_x, f32& next_y);

//------------------------------- Enemy State --------------------------------
enum enemy_state {
    idle,
    chasing,
    fleeing,
    attacking,
    dead
};

//------------------------------- Enemy Type --------------------------------
class base_enemy {
public:
    static constexpr int  enemy_max_hp{ 100 };
    static constexpr int  enemy_damage_per_hit{ 50 };

protected:
    // Position & Movement
    f32 x, y;
    f32 spawn_x, spawn_y;
    f32 velocity_x, velocity_y;
    f32 speed;
    f32 size;

    int current_health;
    int max_health;

    f32 attack_cooldown;
    f32 attack_timer;

    // State
    enemy_state state;
    bool active;
    bool visible_to_player;

    // Collision & Detection
    f32 collision_radius;
    f32 sight_range;

    // Graphics
    const char* texture_path;
    AEGfxTexture* texture;

    // Food Drop
    const char* food_texture_path;
    AEGfxTexture* food_texture;

public:
    base_enemy(f32 start_x, f32 start_y);
    virtual ~base_enemy() { unload_textures(); }

    // Pure virtual
    virtual void load_textures() {
        if (texture_path && !texture) texture = AEGfxTextureLoad(texture_path);
        // food_texture is intentionally not loaded here — owned by EnemyManager
    }

    virtual void unload_textures() {
        if (texture) { AEGfxTextureUnload(texture); texture = nullptr; }
        // food_texture is NOT unloaded here — it is owned and managed by EnemyManager
        food_texture = nullptr;
    }
    virtual void update(f32 dt, struct Player* player, camera* cam) = 0;
    virtual void take_damage(int amount);

    // Type identification without an enum � override in derived classes
    virtual bool is_ghost() const { return false; }

    virtual int get_max_health() const { return max_health; }

    // Collision & Detection
    bool is_in_camera_view(camera* cam)               const;
    bool collides_with_player(struct Player* player)  const;
    f32  distance_to_player(struct Player* player)    const;

    // Getters
    f32         get_x()              const { return x; }
    f32         get_y()              const { return y; }
    f32         get_size()           const { return size; }
    int         get_current_health() const { return current_health; }
    enemy_state get_state()          const { return state; }
    bool        is_active()          const { return active; }

    const char* get_texture_path()      const { return texture_path; }
    const char* get_food_texture_path() const { return food_texture_path; }
    AEGfxTexture* get_texture()           const { return texture; }
    AEGfxTexture* get_food_texture()      const { return food_texture; }

    void set_active(bool val) { active = val; }
    void set_position(f32 new_x, f32 new_y) { x = new_x; y = new_y; }
    void set_food_texture(AEGfxTexture* tex) { food_texture = tex; } // point to manager-owned texture
};

// ============================================================================
// GHOST BOY    flees the moment it enters the player's camera view
// ============================================================================
class ghost : public base_enemy
{
    int hits_from_player;
public:
    ghost(f32 start_x, f32 start_y);
    ~ghost() override = default;

    void update(f32 dt, struct Player* player, camera* cam) override;
    void take_damage(int amount) override;
    bool is_ghost() const override { return true; }
    int get_hits_from_player() const { return hits_from_player; }
};

// ============================================================================
// BAT    chases and attacks the player; optional to defeat
// ============================================================================
class bat : public base_enemy
{
public:
    bat(f32 start_x, f32 start_y);
    ~bat() override = default;

    void update(f32 dt, struct Player* player, camera* cam) override;
};

// ============================================================================
// DRAGON (Boss)   chases the player within fixed room boundaries
// ============================================================================
class dragon : public base_enemy
{
private:
    f32  room_left, room_right, room_top, room_bottom;
    s32  hits_received;
    s32  hits_until_attack;
    bool is_boss;

public:
    dragon(f32 start_x, f32 start_y,
        f32 r_left, f32 r_right, f32 r_top, f32 r_bottom);
    ~dragon() override = default;

    void update(f32 dt, struct Player* player, camera* cam) override;
    void take_damage(int amount) override;

    bool has_dropped_sister() const { return is_boss && state == dead; }
};