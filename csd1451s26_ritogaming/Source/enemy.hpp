/*!************************************************************************
\file:          enemy.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\
\brief:         Declares enemy classes and movement helper functions.
\               Contains base_enemy abstract class with common enemy
\               properties and behaviors. Defines ghost, bat, and dragon
\               enemy types with their specific movement patterns and
\               attack behaviors.
**************************************************************************/

#pragma once

#include <cmath>
#include <vector>
#include <cstring>
#include "AEEngine.h"
#include "camera.hpp"

constexpr int player_heart_damage{50};
constexpr f32 food_health_percent{0.50f};
constexpr f32 food_lifetime{10.f};

//------------------------------- Pathfinding --------------------------------
// Move position (x, y) toward a target, wall-checking along each axis
void enemy_move_towards(f32 &x, f32 &y, f32 speed, f32 size, f32 target_x, f32 target_y, f32 dt);

// Move position (x, y) away from a source, wall-checking along each axis
void enemy_move_away_from(f32 &x, f32 &y, f32 speed, f32 size, f32 from_x, f32 from_y, f32 dt);

// Compute the next flee position (caller decides whether to apply it)
void enemy_flee_to_wall(f32 x, f32 y, f32 speed, f32 player_x, f32 player_y, f32 dt,
                        f32 &next_x, f32 &next_y);

//------------------------------- Enemy State --------------------------------
enum enemy_state
{
    idle,
    chasing,
    fleeing,
    attacking,
    dead
};

//------------------------------- Enemy Type --------------------------------
class base_enemy
{
public:
    static constexpr int enemy_max_hp{100};
    static constexpr int enemy_damage_per_hit{50};

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
    AEGfxTexture *texture;

    // Food Drop
    AEGfxTexture *food_texture;
    bool food_spawned{false};

public:
    base_enemy(f32 start_x, f32 start_y);

    virtual ~base_enemy() {}

    virtual void update(f32 dt, struct Player *player, camera *cam) = 0;
    virtual void take_damage(int amount);
    virtual bool is_ghost() const { return false; }
    virtual int get_max_health() const { return max_health; }

    // Collision & Detection
    bool is_in_camera_view(camera *cam) const;
    bool collides_with_player(struct Player *player) const;
    f32 distance_to_player(struct Player *player) const;

    // Food drop tracking
    bool has_food_spawned() const { return food_spawned; }
    void set_food_spawned(bool val) { food_spawned = val; }

    // Getters
    f32 get_x() const { return x; }
    f32 get_y() const { return y; }
    f32 get_size() const { return size; }
    int get_current_health() const { return current_health; }
    enemy_state get_state() const { return state; }
    bool is_active() const { return active; }

    AEGfxTexture *get_texture() const { return texture; }
    AEGfxTexture *get_food_texture() const { return food_texture; }

    void set_active(bool val) { active = val; }
    void set_position(f32 new_x, f32 new_y)
    {
        x = new_x;
        y = new_y;
    }
    void set_enemy_texture(AEGfxTexture *tex) { texture = tex; }
    void set_food_texture(AEGfxTexture *tex) { food_texture = tex; }
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

    void update(f32 dt, struct Player *player, camera *cam) override;
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

    void update(f32 dt, struct Player *player, camera *cam) override;
};

// ============================================================================
// DRAGON (Boss)   chases the player within fixed room boundaries
// ============================================================================
struct Animator {
    AEGfxTexture* sheet = nullptr;
    int   frame_count = 1;
    int   current_frame = 0;
    float frame_w = 1.f;   // pixel width of one frame
    float frame_h = 1.f;   // pixel height of one frame
    float sheet_w = 1.f;   // total sheet pixel width
    float sheet_h = 1.f;   // total sheet pixel height
    float fps = 8.f;   // playback speed
    float timer = 0.f;
    bool  looping = true;
    bool  finished = false; // true after last frame of a non-looping anim

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

    // Call every frame. Returns true when a non-looping anim just finished.
    bool update(float dt)
    {
        if (finished) return false;
        timer += dt;
        if (timer >= 1.f / fps) {
            timer = 0.f;
            current_frame++;
            if (current_frame >= frame_count) {
                if (looping) current_frame = 0;
                else { current_frame = frame_count - 1; finished = true; return true; }
            }
        }
        return false;
    }

    // UV coordinates of the current frame (0..1 range)
    float u0() const { return (current_frame * frame_w) / sheet_w; }
    float u1() const { return u0() + frame_w / sheet_w; }
    float v0() const { return 0.f; }
    float v1() const { return frame_h / sheet_h; }
};

class dragon : public base_enemy{
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

        void switch_anim(DragonAnim next_anim);

    public:
        dragon(f32 start_x, f32 start_y,
               f32 r_left, f32 r_right, f32 r_top, f32 r_bottom);
        ~dragon() override = default;

        void update(f32 dt, struct Player *player, camera *cam) override;
        void take_damage(int amount) override;

        bool has_dropped_sister() const { return is_boss && state == dead; }

        void load_anim_textures(); 
        void render_animated(f32 draw_size);
};
