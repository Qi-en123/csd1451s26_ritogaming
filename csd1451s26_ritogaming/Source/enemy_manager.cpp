/*!************************************************************************
\file:          enemy_manager.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\co-authour  :  Jerod (5% - projectile function)
\
\brief:         EnemyManager – zone-based spawning, food drops, combat helpers
\               Zone layout (based on map.cpp):
\
\               tutorial_area  – Player Room 2 (rows 18-26, cols 2-9)
\                   + vertical corridor (rows 11-17, cols 4-7)
\                   → 8 ghost boys, respawn after RESPAWN_DELAY
\
\               combat_area    – Player Room 1 (rows 3-10, cols 2-9)
\                   + horizontal corridor (row 7, cols 9-24)
\                   → 5 bats, respawn after RESPAWN_DELAY
\
\                boss_room      – Boss room (level 2)
\                   → 1 dragon, no respawn
**************************************************************************/

#include "pch.hpp"
#include <cstdlib> // rand()

EnemyManager enemy_manager_instance;

extern s32 map[TILE_ROWS][TILE_COLS];

static constexpr f32 MAP_START_X = -800.0f;
static constexpr f32 MAP_START_Y = 450.0f;
static constexpr f32 TILE_SZ = 64.0f;

static AEVec2 tile_spot(s32 row, s32 col)
{
    AEVec2 p;
    p.x = MAP_START_X + col * TILE_SZ;
    p.y = MAP_START_Y - row * TILE_SZ;
    return p;
}

// ============================================================================
// SPAWN POOLS
//   Ghosts – scattered across bottom area + corridors
//   Bats   – scattered across upper room + horizontal corridor
// ============================================================================
static const AEVec2 ghost_spawn_pool[NUM_GHOST_SLOTS] =
{
    tile_spot(23, 25),
    tile_spot(23, 29),
    tile_spot(24, 27),
    tile_spot(25, 25),
    tile_spot(25, 29),
    tile_spot(26, 27),
    tile_spot(27, 25),
    tile_spot(27, 29),
};

static const AEVec2 bat_spawn_pool[NUM_BAT_SLOTS] =
{
    tile_spot(13, 27),
    tile_spot(13, 28),
    tile_spot(13, 27),
    tile_spot(13, 26),
    tile_spot(12, 26),
};

/**************************************************************************/
/*!
  Updates the player system each frame.
*/
/**************************************************************************/
void entity_system_update(Player* player_ptr, EnemyManager* enem)
{
    if (!player_ptr)
        return;
    (void)enem;
    f32 dt = (f32)AEFrameRateControllerGetFrameTime();

    player_update_projectiles(player_ptr, dt);
}


// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================
EnemyManager::EnemyManager() :ghost_slots{}, bat_slots{} {
    ghost_food_tex = AEGfxTextureLoad("Assets/kenney_pixel-platformer_food_expansion/cream.png");
    bat_food_tex = AEGfxTextureLoad("Assets/kenney_pixel-platformer_food_expansion/burger.png");
    dragon_food_tex = AEGfxTextureLoad("Assets/kenney_pixel-platformer_food_expansion/sushi.png");
}

EnemyManager::~EnemyManager() {
    clear_all();
    if (ghost_food_tex)  AEGfxTextureUnload(ghost_food_tex);
    if (bat_food_tex)    AEGfxTextureUnload(bat_food_tex);
    if (dragon_food_tex) AEGfxTextureUnload(dragon_food_tex);
}

// ============================================================================
// ZONE INITIALISATION
// ============================================================================
void EnemyManager::initialize_tutorial_area()
{
    for (int i = 0; i < NUM_GHOST_SLOTS; ++i)
    {
        ghost_slots[i] = { nullptr, 0.0f };
        spawn_ghost_at_slot(i);
    }
}

void EnemyManager::initialize_combat_area()
{
    for (int i = 0; i < NUM_BAT_SLOTS; ++i)
    {
        bat_slots[i] = { nullptr, 0.0f };
        spawn_bat_at_slot(i);
    }
}

// ============================================================================
// UPDATE AND RENDER
// ============================================================================
void EnemyManager::update(f32 dt, Player* player, camera* p_cam) {

    for (auto& enemy : enemies)
        if (enemy && enemy->is_active())
            enemy->update(dt, player, p_cam);

    update_food(dt, player);

    if (current_zone == tutorial_area || current_zone == combat_area)
    {
        for (int i = 0; i < NUM_GHOST_SLOTS; ++i)
            if (ghost_slots[i].enemy == nullptr && ghost_slots[i].respawn_timer > 0.0f)
                ghost_slots[i].respawn_timer -= dt;

        for (int i = 0; i < NUM_BAT_SLOTS; ++i)
            if (bat_slots[i].enemy == nullptr && bat_slots[i].respawn_timer > 0.0f)
                bat_slots[i].respawn_timer -= dt;

        spawn_check_timer += dt;
        if (spawn_check_timer >= SPAWN_CHECK_INTERVAL)
        {
            maintain_ghost_population();
            maintain_bat_population();
            spawn_check_timer = 0.0f;
        }
    }

    cleanup_dead_enemies();
}

void EnemyManager::render() {
    for (auto& enemy : enemies)
        if (enemy->is_active())
            render_enemy(enemy.get());

    render_food();
}

void EnemyManager::render_enemy(base_enemy* e) {
    AEGfxTexture* tex = e->get_texture();

    if (tex)  render_sprite(rect_mesh_center, e->get_x(), e->get_y(), e->get_size(), e->get_size(), tex);
}

// ============================================================================
// SPAWNING FUNCTIONS
// ============================================================================
void EnemyManager::spawn_ghost_at_slot(int slot_index) {

    if (ghost_slots[slot_index].enemy != nullptr)
        return;

    // Create a new ghost
    s32    pool_idx = rand() % NUM_GHOST_SLOTS;
    AEVec2 pos = ghost_spawn_pool[pool_idx];

    auto g = std::make_unique<ghost>(pos.x, pos.y);
    g->load_textures();
    g->set_food_texture(ghost_food_tex); // manager owns this texture
    ghost_slots[slot_index].enemy = g.get();
    enemies.push_back(std::move(g));
}

void EnemyManager::spawn_bat_at_slot(int slot_index) {
    if (bat_slots[slot_index].enemy != nullptr) return;

    s32    pool_idx = rand() % NUM_BAT_SLOTS;
    AEVec2 pos = bat_spawn_pool[pool_idx];

    auto b = std::make_unique<bat>(pos.x, pos.y);
    b->load_textures();
    bat_slots[slot_index].enemy = b.get();
    enemies.push_back(std::move(b));
}

void EnemyManager::spawn_dragon(f32 x, f32 y, f32 r_left, f32 r_right, f32 r_top, f32 r_bottom) {
    // Check if a dragon already exists
    for (auto& enemy : enemies)
        if (enemy && enemy->get_state() != dead && dynamic_cast<dragon*>(enemy.get()) != nullptr)
            return;

    auto d = std::make_unique<dragon>(x, y, r_left, r_right, r_top, r_bottom);
    d->load_textures();
    enemies.push_back(std::move(d));
}
// ============================================================================
// FOOD SYSTEM
// ============================================================================
void EnemyManager::spawn_food(f32 x, f32 y, AEGfxTexture* texture) {
    food_drop food{};
    food.x = x;
    food.y = y;
    food.size = 20.0f;
    food.lifetime = FOOD_LIFTIME;
    food.active = true;
    food.texture = texture;
    food_drops.push_back(food);
}

void EnemyManager::update_food(f32 dt, Player* player) {
    for (auto& food : food_drops) {
        if (!food.active) continue;

        // Decrease lifetime
        food.lifetime -= dt;
        if (food.lifetime <= 0.0f) {
            food.active = false;
            continue;
        }

        // Check collision with player
        f32 dx = food.x - player->current_player->x;
        f32 dy = food.y - player->current_player->y;
        f32 dist = sqrtf(dx * dx + dy * dy);

        if (dist < (food.size + player->stats.character_size / 2.0f)) {
            if (player->current_character == 1)
            {
                player->stats.char1_curr_health = (int)AEMin((f32)player->stats.char1_curr_health + 1, (f32)player->stats.char1_max_health);
            }
            else
                player->stats.char2_curr_health = (int)AEMin((f32)player->stats.char2_curr_health + 1, (f32)player->stats.char2_max_health);

            food.active = false;
        }
    }

    // Remove inactive food
    food_drops.erase(
        std::remove_if(food_drops.begin(), food_drops.end(),
            [](const food_drop& f) { return !f.active; }),
        food_drops.end()
    );
}

void EnemyManager::render_food() {
    for (const auto& food : food_drops)
        if (food.active && food.texture)
            render_sprite(rect_mesh_center, food.x, food.y, food.size, food.size, food.texture);
}

// ============================================================================
// COMBAT
// ============================================================================
void EnemyManager::check_player_attacks(Player* player, f32 attack_range, f32 damage) {
    for (auto& enemy : enemies) {
        if (!enemy || !enemy->is_active()) continue;

        f32 dist = enemy->distance_to_player(player);
        if (dist <= attack_range) {
            enemy->take_damage(static_cast<int>(damage));
            enemy->get_current_health();
        }
    }
}
bool EnemyManager::check_projectile_hit(f32 px, f32 py, f32 radius, f32 damage)
{
    for (auto& enemy : enemies)
    {
        if (!enemy || !enemy->is_active()) continue;
        if (cr_collision(px, py, radius,
            enemy->get_x(), enemy->get_y(),
            enemy->get_size(), enemy->get_size()))
        {
            enemy->take_damage(static_cast<int>(damage));
            return true;
        }
    }
    return false;
}

// ============================================================================
// CLEANUP AND MAINTENANCE
// ============================================================================
void EnemyManager::cleanup_dead_enemies() {
    for (auto& enemy : enemies)
    {
        if (!enemy || enemy->get_state() != dead) continue;

        spawn_food(enemy->get_x(), enemy->get_y(), enemy->get_food_texture());

        // Release ghost slot → respawns at a random pool position after delay
        if (enemy->is_ghost())
        {
            for (int i = 0; i < NUM_GHOST_SLOTS; ++i)
            {
                if (ghost_slots[i].enemy == enemy.get())
                {
                    ghost_slots[i].enemy = nullptr;
                    ghost_slots[i].respawn_timer = RESPAWN_DELAY;
                    break;
                }
            }
        }

        // Release bat slot → respawns at a random pool position after delay
        if (dynamic_cast<bat*>(enemy.get()))
        {
            for (int i = 0; i < NUM_BAT_SLOTS; ++i)
            {
                if (bat_slots[i].enemy == enemy.get())
                {
                    bat_slots[i].enemy = nullptr;
                    bat_slots[i].respawn_timer = RESPAWN_DELAY;
                    break;
                }
            }
        }
    }
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const auto& e) { return e->get_state() == dead; }),
        enemies.end());
}

void EnemyManager::maintain_ghost_population() {
    // Only respawn ONE ghost at a time whose timer has expired
    for (int i = 0; i < NUM_GHOST_SLOTS; ++i) {
        if (ghost_slots[i].enemy == nullptr && ghost_slots[i].respawn_timer <= 0.f) {
            spawn_ghost_at_slot(i);
            return; // Only spawn one ghost per check, creating staggered respawns
        }
    }
}

void EnemyManager::maintain_bat_population() {
    for (int i = 0; i < NUM_BAT_SLOTS; ++i) {
        if (bat_slots[i].enemy == nullptr && bat_slots[i].respawn_timer <= 0.f) {
            spawn_bat_at_slot(i);
            return; // Only spawn one bat per check, creating staggered respawns
        }
    }
}

// ============================================================================
// UTILITY
// ============================================================================
size_t EnemyManager::get_active_enemy_count() const {
    return std::count_if(enemies.begin(), enemies.end(),
        [](const std::unique_ptr<base_enemy>& e) {
            return e && e->is_active();
        });
}

size_t EnemyManager::get_ghost_count() const {
    return std::count_if(enemies.begin(), enemies.end(),
        [](const auto& e) {
            return e->is_active() && dynamic_cast<ghost*>(e.get()) != nullptr;
        });
}

size_t EnemyManager::get_bat_count() const {
    return std::count_if(enemies.begin(), enemies.end(),
        [](const auto& e) {
            return e->is_active() && dynamic_cast<bat*>(e.get()) != nullptr;
        });
}

void EnemyManager::clear_all()
{
    enemies.clear();
    food_drops.clear();

    for (int i = 0; i < NUM_GHOST_SLOTS; ++i)
        ghost_slots[i] = { nullptr, 0.0f };

    for (int i = 0; i < NUM_BAT_SLOTS; ++i)
        bat_slots[i] = { nullptr, 0.0f };

    spawn_check_timer = 0.0f;
    enemies_defeated = 0;
    boss_room_entered = false;
}

//// ============================================================================
//// PROJECTILE TO ENEMY - by Jerod
//// ============================================================================v
//bool EnemyManager::check_projectile_hit(f32 px, f32 py, f32 radius, f32 damage) {
//    // This is the SAME loop logic you posted!
//    for (auto& enemy : enemies) {
//        if (!enemy || !enemy->is_active()) continue;
//
//        // This is the SAME collision check!
//        if (cr_collision(px, py, radius,
//            enemy->get_x(), enemy->get_y(),
//            enemy->get_size(), enemy->get_size()))
//        {
//            enemy->take_damage(static_cast<int>(damage)); // SAME damage logic!
//            return true;
//        }
//    }
//    return false;
//}