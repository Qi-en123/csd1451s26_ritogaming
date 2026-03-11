/*!************************************************************************
\file:          enemy_manager.cpp
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

#include "pch.hpp"
#include <cstdlib>// rand()
#include <cmath>
#include <string>

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
// LOAD SPAWN POSITIONS FROM FILE
// ============================================================================
void EnemyManager::load_spawn_positions(const char* filename)
{
    FILE* fp = nullptr;
    fopen_s(&fp, filename, "r");
    if (!fp)
    {
        printf("[EnemyManager] Failed to open spawn file: %s\n", filename);
        return;
    }

    int ghost_count = 0;
    int bat_count = 0;
    int cross_count = 0;
    char token[32];

    while (fscanf_s(fp, "%31s", token, (unsigned)sizeof(token)) == 1)
    {
        if (strcmp(token, "GHOST") == 0)
        {
            int row, col;
            if (fscanf_s(fp, "%d %d", &row, &col) == 2 && ghost_count < NUM_GHOST_SLOTS)
            {
                ghost_spawn_positions[ghost_count] = tile_spot(row, col);
                ++ghost_count;
            }
        }
        else if (strcmp(token, "BAT") == 0)
        {
            int row, col;
            if (fscanf_s(fp, "%d %d", &row, &col) == 2 && bat_count < NUM_BAT_SLOTS)
            {
                bat_spawn_positions[bat_count] = tile_spot(row, col);
                ++bat_count;
            }
        }
        else if (strcmp(token, "CROSS") == 0)
        {
            int row, col;
            if (fscanf_s(fp, "%d %d", &row, &col) == 2 && cross_count < NUM_CROSS_MARKERS)
            {
                cross_positions[cross_count] = tile_spot(row, col);
                ++cross_count;
            }
        }
        // Skip section headers: "Spawns:", "Cross:", "Bats:" and their counts
    }

    fclose(fp);
    printf("[EnemyManager] Loaded %d ghost, %d bat, %d cross positions from %s\n",
        ghost_count, bat_count, cross_count, filename);
    spawn_positions_loaded = true;
}

/**************************************************************************/
/*!
  Updates projectiles each frame (called from level update).
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
EnemyManager::EnemyManager() :ghost_slots{}, bat_slots{},
ghost_spawn_positions{}, bat_spawn_positions{}, cross_positions{}
{}

EnemyManager::~EnemyManager() {
    clear_all();
    if (ghost_food_tex)   AEGfxTextureUnload(ghost_food_tex);
    if (bat_food_tex)     AEGfxTextureUnload(bat_food_tex);
    if (dragon_food_tex)  AEGfxTextureUnload(dragon_food_tex);
    if (cross_tex)        AEGfxTextureUnload(cross_tex);
    if (ghost_marker_tex) AEGfxTextureUnload(ghost_marker_tex);
    if (bat_marker_tex)   AEGfxTextureUnload(bat_marker_tex);
}

// ============================================================================
// ZONE INITIALISATION
// ============================================================================
void EnemyManager::initialize_tutorial_area()
{
    // Load food textures (AEEngine is ready at this point)
    if (!ghost_food_tex)  ghost_food_tex = AEGfxTextureLoad("Assets/Enemy_food/cream.png");
    if (!bat_food_tex)    bat_food_tex = AEGfxTextureLoad("Assets/Enemy_food/burger.png");
    if (!dragon_food_tex) dragon_food_tex = AEGfxTextureLoad("Assets/Enemy_food/sushi.png");

    // Load spawn marker textures
    if (!cross_tex)        cross_tex = AEGfxTextureLoad("Assets/Enemy_spawn_location/tile_0064.png");
    if (!ghost_marker_tex) ghost_marker_tex = AEGfxTextureLoad("Assets/Enemy_spawn_location/tile_0066.png");
    if (!bat_marker_tex)   bat_marker_tex = AEGfxTextureLoad("Assets/Enemy_spawn_location/tile_0126.png");

    // Load all spawn positions (ghosts, bats, crosses) from file
    load_spawn_positions("Assets/Enemy_spawn_location/Enemy_spawning_coordinates.txt");

    // Spawn ghosts at their fixed positions
    for (int i = 0; i < NUM_GHOST_SLOTS; ++i)
    {
        ghost_slots[i] = { nullptr, 0.0f };
        spawn_ghost_at_slot(i);
    }

    // Spawn bats at their fixed positions
    for (int i = 0; i < NUM_BAT_SLOTS; ++i)
    {
        bat_slots[i] = { nullptr, 0.0f };
        spawn_bat_at_slot(i);
    }
}

void EnemyManager::initialize_combat_area()
{
    // Load food and marker textures if not already loaded
    if (!ghost_food_tex)  ghost_food_tex = AEGfxTextureLoad("Assets/Enemy_food/cream.png");
    if (!bat_food_tex)    bat_food_tex = AEGfxTextureLoad("Assets/Enemy_food/burger.png");
    if (!dragon_food_tex) dragon_food_tex = AEGfxTextureLoad("Assets/Enemy_food/sushi.png");
    if (!cross_tex)        cross_tex = AEGfxTextureLoad("Assets/Enemy_spawn_location/tile_0064.png");
    if (!ghost_marker_tex) ghost_marker_tex = AEGfxTextureLoad("Assets/Enemy_spawn_location/tile_0066.png");
    if (!bat_marker_tex)   bat_marker_tex = AEGfxTextureLoad("Assets/Enemy_spawn_location/tile_0126.png");

    load_spawn_positions("Assets/Enemy_spawn_location/Enemy_spawning_coordinates.txt");

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
    update_respawn_particles(dt);

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
    render_spawn_markers();  // Always draw cross markers under enemies

    for (auto& enemy : enemies)
        if (enemy->is_active())
            render_enemy(enemy.get());

    render_food();
    render_respawn_particles();
}

void EnemyManager::render_spawn_markers()
{
    if (!spawn_positions_loaded) return;

    // Draw ghost icon at each GHOST spawn position
    if (ghost_marker_tex)
    {
        for (int i = 0; i < NUM_GHOST_SLOTS; ++i)
            render_sprite(rect_mesh_center,
                ghost_spawn_positions[i].x, ghost_spawn_positions[i].y,
                64.0f, 64.0f, ghost_marker_tex);
    }

    // Draw cross marker at each CROSS position
    if (cross_tex)
    {
        for (int i = 0; i < NUM_CROSS_MARKERS; ++i)
            render_sprite(rect_mesh_center,
                cross_positions[i].x, cross_positions[i].y,
                64.0f, 64.0f, cross_tex);
    }

    // Draw tree icon at each BAT spawn position
    if (bat_marker_tex)
    {
        for (int i = 0; i < NUM_BAT_SLOTS; ++i)
            render_sprite(rect_mesh_center,
                bat_spawn_positions[i].x, bat_spawn_positions[i].y,
                64.0f, 64.0f, bat_marker_tex);
    }
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

    AEVec2 pos = ghost_spawn_positions[slot_index];

    auto g = std::make_unique<ghost>(pos.x, pos.y);
    g->load_textures();
    g->set_food_texture(ghost_food_tex); // use manager-owned texture to avoid dangling ptr on death
    ghost_slots[slot_index].enemy = g.get();
    enemies.push_back(std::move(g));
}

void EnemyManager::spawn_bat_at_slot(int slot_index) {
    if (bat_slots[slot_index].enemy != nullptr) return;

    AEVec2 pos = bat_spawn_positions[slot_index];

    auto b = std::make_unique<bat>(pos.x, pos.y);
    b->load_textures();
    b->set_food_texture(bat_food_tex); // use manager-owned texture to avoid dangling ptr on death
    bat_slots[slot_index].enemy = b.get();
    enemies.push_back(std::move(b));
}

void EnemyManager::spawn_dragon(f32 x, f32 y, f32 r_left, f32 r_right, f32 r_top, f32 r_bottom) {
    for (auto& enemy : enemies)
        if (enemy && enemy->get_state() != dead && dynamic_cast<dragon*>(enemy.get()) != nullptr)
            return;

    auto d = std::make_unique<dragon>(x, y, r_left, r_right, r_top, r_bottom);
    d->load_textures();
    d->set_food_texture(dragon_food_tex); // use manager-owned texture to avoid dangling ptr on death
    enemies.push_back(std::move(d));
}
// ============================================================================
// RESPAWN PARTICLE EFFECTS
//   Burst of particles at the spawn point just before an enemy reappears.
//   Reuses the same Particle struct from player.hpp — no new types needed.
// ============================================================================
void EnemyManager::spawn_respawn_particles(f32 x, f32 y)
{
    constexpr int BURST = 20;  // number of particles per respawn

    for (int i = 0; i < BURST; ++i)
    {
        Particle p;
        p.x = x;
        p.y = y;

        // Random outward burst using AERandFloat() (same helper player uses)
        f32 angle = AERandFloat() * 2.0f * 3.14159f;
        f32 speed = 30.0f + AERandFloat() * 80.0f;
        p.vx = AECos(angle) * speed;
        p.vy = AESin(angle) * speed + 40.0f; // slight upward bias

        p.maxLife = 0.4f + AERandFloat() * 0.4f; // 0.4 – 0.8 s
        p.life = p.maxLife;
        p.size = 4.0f + AERandFloat() * 6.0f;
        p.active = true;

        respawn_particles.push_back(p);
    }
}

void EnemyManager::update_respawn_particles(f32 dt)
{
    for (size_t i = 0; i < respawn_particles.size(); ++i)
    {
        Particle& p = respawn_particles[i];
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.vy -= 120.0f * dt; // gravity pulls particles down
        p.life -= dt;

        if (p.life <= 0.0f)
        {
            respawn_particles.erase(respawn_particles.begin() + i);
            --i;
        }
    }
}

void EnemyManager::render_respawn_particles()
{
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    for (const auto& p : respawn_particles)
    {
        f32 alpha = p.life / p.maxLife;  // fade out over lifetime

        // Purple / blue-white swirl to signal a respawn
        AEGfxSetColorToMultiply(0.6f + 0.4f * alpha, 0.3f, 1.0f, alpha);
        draw_circle(p.x, p.y, p.size * alpha, p.size * alpha);
    }
}

// ============================================================================
// FOOD SYSTEM
// ============================================================================
void EnemyManager::spawn_food(f32 x, f32 y, AEGfxTexture* texture) {
    food_drop food{};
    food.x = x;
    food.y = y;
    food.size = 50.0f;
    food.lifetime = FOOD_LIFTIME;
    food.active = true;
    food.texture = texture;
    food_drops.push_back(food);
}

void EnemyManager::update_food(f32 dt, Player* player) {
    for (auto& food : food_drops) {
        if (!food.active) continue;

        printf("Food active at (%.2f, %.2f), lifetime: %.2f\n",
            food.x, food.y, food.lifetime);

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

    for (const auto& food : food_drops) {
        if (!food.active || !food.texture) {
            printf("Food inactive or no texture\n");
            continue;
        }

        // Draw food
        render_sprite(rect_mesh_center, food.x, food.y, food.size, food.size, food.texture);
    }
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
    for (auto& enemy : enemies) {
        if (!enemy || enemy->get_state() != dead) continue;

        if (enemy->is_ghost()) {
            ghost* g = static_cast<ghost*>(enemy.get());
            if (g->get_hits_from_player() > 0)
                spawn_food(enemy->get_x(), enemy->get_y(), enemy->get_food_texture());
        }
        else
            spawn_food(enemy->get_x(), enemy->get_y(), enemy->get_food_texture());

        // Release ghost slot → respawns at a random pool position after delay
        if (enemy->is_ghost()) {
            for (int i = 0; i < NUM_GHOST_SLOTS; ++i) {
                if (ghost_slots[i].enemy == enemy.get()) {
                    ghost_slots[i].enemy = nullptr;
                    ghost_slots[i].respawn_timer = RESPAWN_DELAY;
                    break;
                }
            }
        }

        // Release bat slot → respawns at a random pool position after delay
        if (dynamic_cast<bat*>(enemy.get())) {
            for (int i = 0; i < NUM_BAT_SLOTS; ++i) {
                if (bat_slots[i].enemy == enemy.get()) {
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
    for (int i = 0; i < NUM_GHOST_SLOTS; ++i) {
        if (ghost_slots[i].enemy == nullptr && ghost_slots[i].respawn_timer <= 0.f) {
            spawn_respawn_particles(ghost_spawn_positions[i].x, ghost_spawn_positions[i].y);
            spawn_ghost_at_slot(i);
            return;
        }
    }
}

void EnemyManager::maintain_bat_population() {
    for (int i = 0; i < NUM_BAT_SLOTS; ++i) {
        if (bat_slots[i].enemy == nullptr && bat_slots[i].respawn_timer <= 0.f) {
            spawn_respawn_particles(bat_spawn_positions[i].x, bat_spawn_positions[i].y);
            spawn_bat_at_slot(i);
            return;
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
    respawn_particles.clear();

    for (int i = 0; i < NUM_GHOST_SLOTS; ++i)
        ghost_slots[i] = { nullptr, 0.0f };

    for (int i = 0; i < NUM_BAT_SLOTS; ++i)
        bat_slots[i] = { nullptr, 0.0f };

    spawn_check_timer = 0.0f;
    enemies_defeated = 0;
    boss_room_entered = false;
    spawn_positions_loaded = false;
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