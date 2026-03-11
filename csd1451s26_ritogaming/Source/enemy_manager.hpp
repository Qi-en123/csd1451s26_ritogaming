/*!************************************************************************
\file:          enemy_manager.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\co-author:     Jerod (5% - projectile function)

\brief
        EnemyManager handles enemy-related systems including spawning, updating,
        rendering, food drops, respawning, and combat interactions. The manager
        controls enemy population based on the current game zone and ensures
        enemies respawn at their assigned spawn locations when defeated.

        Zone layout (based on map.cpp):

        tutorial_area
            Player Room 2 (rows 18–26, cols 2–9)
            + vertical corridor (rows 11–17, cols 4–7)
            → 8 ghosts that respawn after RESPAWN_DELAY

        combat_area
            Player Room 1 (rows 3–10, cols 2–9)
            + horizontal corridor (row 7, cols 9–24)
            → 5 bats that respawn after RESPAWN_DELAY

        boss_room
            Boss room (level 2)
            → 1 dragon boss, does not respawn
**************************************************************************/

#pragma once

#include <algorithm>
#include <random>
#include <memory>
#include <vector>
#include "AEEngine.h"

struct camera;

constexpr s32 NUM_GHOST_SLOTS = 3;
constexpr s32 NUM_BAT_SLOTS = 3;
constexpr s32 NUM_CROSS_MARKERS = 3;

constexpr f32 SPAWN_CHECK_INTERVAL = 3.f;
constexpr f32 RESPAWN_DELAY = 5.f;

constexpr f32 FOOD_LIFTIME = 9.f;
constexpr f32 FOOD_TIMER_SCALE = 0.5f;
constexpr f32 FOOD_HEAL_PERCENTAGE = 0.50f;

enum game_zone {
    tutorial_area,
    combat_area,
    boss_room
};

struct food_drop {
    f32 x, y;
    f32 size;
    f32 heal_percentage;
    f32 lifetime;
    bool active;
    AEGfxTexture* texture;
};

struct enemy_slot {
    base_enemy* enemy;  // Pointer to the ghost occupying this slot (nullptr if empty)
    f32 respawn_timer;  // Timer before this slot can respawn
};

class EnemyManager {
private:
    std::vector<std::unique_ptr<base_enemy>> enemies;
    std::vector<food_drop>                   food_drops;

    enemy_slot ghost_slots[NUM_GHOST_SLOTS];
    enemy_slot bat_slots[NUM_BAT_SLOTS];

    // Fixed spawn positions loaded from file
    AEVec2 ghost_spawn_positions[NUM_GHOST_SLOTS];
    AEVec2 bat_spawn_positions[NUM_BAT_SLOTS];

    // Cross spawn markers (visual indicators at spawn points)
    AEVec2        cross_positions[NUM_CROSS_MARKERS];
    AEGfxTexture* cross_tex{};   // tile_0064.png - cross marker at CROSS positions
    AEGfxTexture* ghost_marker_tex{};  // tile_0066.png - ghost icon at GHOST spawn positions
    AEGfxTexture* bat_marker_tex{};  // tile_0126.png - tree icon at BAT spawn positions
    bool          spawn_positions_loaded{ false };

    // Respawn particle effects
    std::vector<Particle> respawn_particles;

    // Food textures
    AEGfxTexture* ghost_food_tex{};
    AEGfxTexture* bat_food_tex{};
    AEGfxTexture* dragon_food_tex{};

    f32 spawn_check_timer{};

    game_zone current_zone{ tutorial_area };
    bool      boss_room_entered{ false };
    int       enemies_defeated{ 0 };

public:
    EnemyManager();
    ~EnemyManager();

    void update(f32 dt, Player* player, camera* cam);
    void render();

    void      set_zone(game_zone zone) { current_zone = zone; }
    game_zone get_zone()         const { return current_zone; }

    void initialize_tutorial_area(); // 8 ghosts � bottom room + vertical corridor
    void initialize_combat_area();   // 5 bats   � upper room + horizontal corridor
    void load_spawn_positions(const char* filename);

    void spawn_ghost_at_slot(int slot_index);
    void spawn_bat_at_slot(int slot_index);
    void spawn_dragon(f32 x, f32 y, f32 r_left, f32 r_right, f32 r_top, f32 r_bottom);

    void spawn_food(f32 x, f32 y, AEGfxTexture* texture);
    void spawn_respawn_particles(f32 x, f32 y); // burst of particles at spawn point

    void check_player_attacks(Player* player, f32 attack_range, f32 damage);
    bool check_projectile_hit(f32 px, f32 py, f32 radius, f32 damage);

    void cleanup_dead_enemies();
    void clear_all();

    void maintain_ghost_population();
    void maintain_bat_population();


    size_t get_active_enemy_count() const;
    size_t get_ghost_count() const;
    size_t get_bat_count()   const;
    bool   is_boss_room_entered()   const { return boss_room_entered; }
    int    get_enemies_defeated()   const { return enemies_defeated; }

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