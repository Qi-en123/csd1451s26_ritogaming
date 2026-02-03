/*!************************************************************************
\file:          enemy_manager.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         Manages spawning, updating, and rendering enemies and food drops
**************************************************************************/

#pragma once

#include "AEEngine.h"
#include "collision.hpp"
#include "renderer.hpp"
#include "enemy.hpp"
#include <algorithm>
#include <random>
#include <memory>
#include <vector>

struct Player;
struct camera;
class base_enemy;
class ghost_boy;
class bat;
class dragon;

constexpr s32 NUM_GHOST_SLOTS = 10;
constexpr f32 SPAWN_CHECK_INTERVAL = 2.0f;
constexpr f32 GHOST_RESPAWN_DELAY = 5.0f;  // Delay between each ghost respawn

struct food_drop {
    f32 x, y;
    f32 size;
    f32 heal_percentage;
    f32 lifetime;
    bool active;
    AEGfxTexture* texture;
};

struct GhostSlot {
    f32 spawn_x, spawn_y;
    base_enemy* ghost;  // Pointer to the ghost occupying this slot (nullptr if empty)
    f32 respawn_timer;  // Timer before this slot can respawn
};

class enemy_manager {
private:
    std::vector<std::unique_ptr<base_enemy>> enemies;
    std::vector<food_drop> food_drops;

    // Fixed ghost spawn slots
    GhostSlot ghost_slots[NUM_GHOST_SLOTS];

    // Food textures
    AEGfxTexture* ghost_food_tex{};
    AEGfxTexture* bat_food_tex{};
    AEGfxTexture* dragon_food_tex{};

    f32 spawn_check_timer{};

public:
    enemy_manager();
    ~enemy_manager();

    void update(f32 dt, Player* player, camera* cam);
    void render();

    void spawn_ghost_at_slot(int slot_index);
    void spawn_bat(f32 x, f32 y);
    void spawn_dragon(f32 x, f32 y, f32 r_left, f32 r_right, f32 r_top, f32 r_bottom);

    void spawn_food(f32 x, f32 y, f32 heal_percentage, const char* texture_path);
    void check_player_attacks(Player* player, f32 attack_range, f32 damage);

    bool check_projectile_hit(f32 px, f32 py, f32 radius, f32 damage);

    void cleanup_dead_enemies();
    void clear_all();

    void maintain_ghost_population();
    void initialize_ghost_slots();

    size_t get_active_enemy_count() const;
    size_t get_ghost_count() const;

private:
    void update_food(f32 dt, Player* player);
    void render_food();
    void render_enemy(base_enemy* enemy);
};

extern enemy_manager g_enemy_manager;