/*!************************************************************************
\file:          enemy.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         Enemy base class and derived enemy types
**************************************************************************/

#pragma once

#include "AEEngine.h"
#include "player.hpp"
#include "camera.hpp"
#include "collision.hpp"
#include "map.hpp"
#include <cmath>
#include <vector>
#include <cstring> 

enum class enemy_type {
    ghost_boy,
    bat,
    dragon
};

enum class enemy_state {
    idle,
    chasing,
    fleeing,
    attacking,
    dead
};

class base_enemy {
protected:
    // Position & Movement
    f32 x, y;
    f32 spawn_x, spawn_y;
    f32 velocity_x, velocity_y;
    f32 speed;
    f32 size;

    // Health & Combat
    f32 max_health;
    f32 current_health;
    f32 damage;
    f32 attack_cooldown;
    f32 attack_timer;

    // State
    enemy_type type;
    enemy_state state;
    bool active;
    bool visible_to_player;

    // Collision & Detection
    f32 collision_radius;
    f32 sight_range;
    f32 reset_range;

    // Graphics
    const char* texture;
    f32 animation_timer;

    // Food Drop
    f32 food_heal_percentage;
    const char* food_texture_path;

public:
    base_enemy(enemy_type enemy_type, f32 start_x, f32 start_y);
    virtual ~base_enemy() = default;

    // Pure virtual - must be overridden
    virtual void update(f32 dt, struct Player* player, camera* cam) = 0;

    // Common functions
    virtual void take_damage(f32 amount);
    virtual bool is_dead() const { return current_health <= 0 || !active; }
    virtual bool should_despawn() const { return !active; }

    // Movement helpers
    void move_towards(f32 target_x, f32 target_y, f32 dt);
    void move_away_from(f32 target_x, f32 target_y, f32 dt);
    void flee_to_wall(f32 player_x, f32 player_y, f32 dt);

    // Collision & Detection
    bool is_in_camera_view(camera* cam) const;
    bool collides_with_player(struct Player* player) const;
    f32 distance_to_player(struct Player* player) const;

    // Getters
    f32 get_x() const { return x; }
    f32 get_y() const { return y; }
    f32 get_size() const { return size; }
    f32 get_current_health() const { return current_health; }
    f32 get_max_health() const { return max_health; }
    enemy_type get_type() const { return type; }
    enemy_state get_state() const { return state; }
    bool is_active() const { return active; }
    f32 get_food_heal_percentage() const { return food_heal_percentage; }
    const char* get_texture_path() const { return texture; }
    const char* get_food_texture_path() const { return food_texture_path; }

    // Setters
    void set_active(bool val) { active = val; }
    void set_position(f32 new_x, f32 new_y) { x = new_x; y = new_y; }
};

// ============================================================================
// GHOST BOY ENEMY
// ============================================================================
class ghost_boy : public base_enemy {
private:
    f32 flee_timer;
    bool has_reached_wall;

public:
    ghost_boy(f32 start_x, f32 start_y);
    ~ghost_boy() override = default;

    void update(f32 dt, struct Player* player, camera* cam) override;
};

// ============================================================================
// BAT ENEMY
// ============================================================================
class bat : public base_enemy {
private:
    f32 aggro_timer;

public:
    bat(f32 start_x, f32 start_y);
    ~bat() override = default;

    void update(f32 dt, struct Player* player, camera* cam) override;
};

// ============================================================================
// DRAGON BOSS ENEMY
// ============================================================================
class dragon : public base_enemy {
private:
    f32 room_left, room_right, room_top, room_bottom;
    s32 hits_received;
    s32 hits_until_attack;
    bool is_boss;

public:
    dragon(f32 start_x, f32 start_y, f32 r_left, f32 r_right, f32 r_top, f32 r_bottom);
    ~dragon() override = default;

    void update(f32 dt, struct Player* player, camera* cam) override;
    void take_damage(f32 amount) override;

    bool has_sister_drop() const { return is_boss && is_dead(); }
};