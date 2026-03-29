#pragma once

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include "AEEngine.h"
#include "enemy.hpp"
#include "player.hpp"

struct camera;

// Logic Constants
static constexpr f32 SPAWN_CHECK_INTERVAL = 3.f;
static constexpr f32 RESPAWN_DELAY = 5.f;
static constexpr f32 FOOD_LIFTIME = 9.f;

enum game_zone { tutorial_area, combat_area, boss_room };

struct food_drop {
    f32 x, y, size, lifetime;
    bool active;
    AEGfxTexture* texture;
};

struct enemy_slot {
    base_enemy* enemy;
    f32 respawn_timer;
};

class EnemyManager {
private:
    // Core Data Structures
    std::vector<std::unique_ptr<base_enemy>> enemies;
    std::vector<food_drop>                   food_drops;
    std::vector<Particle>                    respawn_particles;

    // Dynamic Arrays (Manual Memory Management)
    enemy_slot* ghost_slots;
    enemy_slot* bat_slots;
    AEVec2* ghost_spawn_positions;
    AEVec2* bat_spawn_positions;
    AEVec2* cross_positions;

    // Dynamic Counts
    int ghost_count;
    int bat_count;
    int cross_count;

    // Textures
    AEGfxTexture* cross_tex{ nullptr }, * ghost_marker_tex{ nullptr }, * bat_marker_tex{ nullptr };
    AEGfxTexture* ghost_tex{ nullptr }, * bat_tex{ nullptr }, * dragon_tex{ nullptr };
    AEGfxTexture* ghost_food_tex{ nullptr }, * bat_food_tex{ nullptr }, * dragon_food_tex{ nullptr };

    // State Variables
    bool      spawn_positions_loaded{ false };
    f32       spawn_check_timer{ 0.0f };
    game_zone current_zone{ tutorial_area };
    bool      boss_room_entered{ false };
    int       enemies_defeated{ 0 };

public:
    EnemyManager();
    ~EnemyManager();

    // Lifecycle
    void load_textures();
    void unload_textures();
    void update(f32 dt, Player* player, camera* cam);
    void render();

    // Setup
    void set_zone(game_zone zone) { current_zone = zone; }
    void initialize_tutorial_area();
    void initialize_combat_area();
    void load_spawn_positions(const char* filename);
    void clear_all();

    // Spawning
    void spawn_ghost_at_slot(int idx);
    void spawn_bat_at_slot(int idx);
    void spawn_dragon(f32 x, f32 y, f32 r_left, f32 r_right, f32 r_top, f32 r_bottom);
    void maintain_ghost_population();
    void maintain_bat_population();

    // VFX & Feedback
    void spawn_respawn_particles(f32 x, f32 y);
    void spawn_food(f32 x, f32 y, AEGfxTexture* texture);

    // Combat
    void check_player_attacks(Player* player, f32 range, f32 damage);
    bool check_projectile_hit(f32 px, f32 py, f32 radius, f32 damage);
    void cleanup_dead_enemies();

    // Getters
    size_t get_active_enemy_count() const;
    size_t get_ghost_count() const;
    size_t get_bat_count() const;
    bool   save_spawn_positions_from_map(s32** map_data, const std::string& filename);
    int get_ghost_count_dynamic() const { return ghost_count; }
    int get_bat_count_dynamic() const { return bat_count; }
    std::vector<std::unique_ptr<base_enemy>> const& get_enemies() const { return enemies; }

private:
    void update_food(f32 dt, Player* player);
    void render_food();
    void render_enemy(base_enemy* enemy);
    void render_spawn_markers();
    void update_respawn_particles(f32 dt);
    void render_respawn_particles();
};

extern EnemyManager enemy_manager_instance;
void entity_system_update(Player* player_ptr, EnemyManager* enem);