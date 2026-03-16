/*!************************************************************************
\file:          enemy_manager.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         EnemyManager – zone-based spawning, food drops, combat helpers
\
\               Zone layout (based on map.cpp):
\
\               tutorial_area  – Player Room 2 (rows 18-26, cols 2-9)
\                   + vertical corridor (rows 11-17, cols 4-7)
\                   ? 8 ghost boys, respawn after RESPAWN_DELAY
\
\               combat_area    – Player Room 1 (rows 3-10, cols 2-9)
\                   + horizontal corridor (row 7, cols 9-24)
\                   ? 5 bats, respawn after RESPAWN_DELAY
\
\                boss_room      – Boss room (level 2)
\                   ? 1 dragon, no respawn
**************************************************************************/

#pragma once

#include <algorithm>
#include <random>
#include <memory>
#include <vector>
#include "AEEngine.h"

struct camera;

constexpr s32 NUM_GHOST_SLOTS = 10;
constexpr s32 NUM_BAT_SLOTS = 5;

constexpr f32 SPAWN_CHECK_INTERVAL = 3.0f;
constexpr f32 RESPAWN_DELAY = 5.0f;  

constexpr f32 FOOD_LIFTIME = 10.0f;
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
	enemy_slot bat_slots  [NUM_BAT_SLOTS];

    // Food textures
    AEGfxTexture* ghost_food_tex {};
    AEGfxTexture* bat_food_tex   {};
    AEGfxTexture* dragon_food_tex{};

    f32 spawn_check_timer{};

    game_zone current_zone     { tutorial_area };
    bool      boss_room_entered{ false };
    int       enemies_defeated { 0 };

public:
    EnemyManager ();
    ~EnemyManager();

    void load_textures();
    void unload_textures();

    void update(f32 dt, Player* player, camera* cam);
    void render();

    void      set_zone(game_zone zone) { current_zone = zone; }
    game_zone get_zone()         const { return current_zone; }

    void initialize_tutorial_area(); // 8 ghosts – bottom room + vertical corridor
    void initialize_combat_area();   // 5 bats   – upper room + horizontal corridor

    void spawn_ghost_at_slot(int slot_index);
    void spawn_bat_at_slot  (int slot_index);
    void spawn_dragon       (f32 x, f32 y, f32 r_left, f32 r_right, f32 r_top, f32 r_bottom);

    void spawn_food(f32 x, f32 y, AEGfxTexture* texture);

    void check_player_attacks(Player* player, f32 attack_range, f32 damage);
    bool check_projectile_hit(f32 px, f32 py, f32 radius, f32 damage);

    void cleanup_dead_enemies();
    void clear_all();

    void maintain_ghost_population();
    void maintain_bat_population  ();


    size_t get_active_enemy_count() const;
    size_t get_ghost_count() const;
	size_t get_bat_count()   const;
    bool   is_boss_room_entered()   const { return boss_room_entered; }
    int    get_enemies_defeated()   const { return enemies_defeated; }

	std::vector<std::unique_ptr<base_enemy>> const & get_enemies() const { return enemies; }

private:
    void update_food(f32 dt, Player* player);
    void render_food();
    void render_enemy(base_enemy* enemy);
};
extern EnemyManager enemy_manager_instance;

void entity_system_update(Player* player_ptr, EnemyManager* enem);


