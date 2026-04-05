/*!************************************************************************
\file:          enemy_manager.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu

\brief:         EnemyManager implementation. All previously hardcoded
                magic numbers (filepaths, timings, sizes, particle params)
                are now read directly from the e_ globals defined in
                enemy.cpp and declared extern in enemy.hpp, populated
                once at startup by dave_Data().
**************************************************************************/

#include "pch.hpp"
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include "map.hpp"

EnemyManager enemy_manager_instance;

// ============================================================================
// MAP HELPER  (uses e_tile_size, e_map_start_x/y from enemy.hpp)
// ============================================================================

static AEVec2 tile_spot(s32 row, s32 col)
{
    AEVec2 p;
    p.x = world_left + col * tile_size + tile_size * 0.5f;
    p.y = world_top - row * tile_size - tile_size * 0.5f;
    return p;
}

// ============================================================================
// LIFECYCLE
// ============================================================================

EnemyManager::EnemyManager() :
    ghost_slots(nullptr), bat_slots(nullptr),
    ghost_spawn_positions(nullptr), bat_spawn_positions(nullptr), cross_positions(nullptr),
    ghost_count(0), bat_count(0), cross_count(0)
{
}

EnemyManager::~EnemyManager()
{
    clear_all();
    unload_textures();
}

// ============================================================================
// LOAD / UNLOAD TEXTURES  (paths from e_ globals)
// ============================================================================

void EnemyManager::load_textures()
{
    if (!ghost_food_tex)   ghost_food_tex = AEGfxTextureLoad(e_ghost_food_tex.c_str());
    if (!bat_food_tex)     bat_food_tex = AEGfxTextureLoad(e_bat_food_tex.c_str());
    if (!cross_tex)        cross_tex = AEGfxTextureLoad(e_cross_tex.c_str());
    if (!ghost_marker_tex) ghost_marker_tex = AEGfxTextureLoad(e_ghost_marker_tex.c_str());
    if (!bat_marker_tex)   bat_marker_tex = AEGfxTextureLoad(e_bat_marker_tex.c_str());
    if (!ghost_tex)        ghost_tex = AEGfxTextureLoad(e_ghost_tex.c_str());
    if (!bat_tex)          bat_tex = AEGfxTextureLoad(e_bat_tex.c_str());
}

void EnemyManager::unload_textures()
{
    auto safe_unload = [](AEGfxTexture*& tex) {
        if (tex) { AEGfxTextureUnload(tex); tex = nullptr; }
        };
    safe_unload(ghost_food_tex); safe_unload(bat_food_tex);   safe_unload(dragon_food_tex);
    safe_unload(cross_tex);      safe_unload(ghost_marker_tex); safe_unload(bat_marker_tex);
    safe_unload(ghost_tex);      safe_unload(bat_tex);          safe_unload(dragon_tex);
}

// ============================================================================
// SPAWN POSITIONS
// ============================================================================

void EnemyManager::load_spawn_positions(const char* filename)
{
    save_Data("Assets/Config/enemy_config.txt");
    clear_all();

    std::ifstream file(filename);
    if (!file.is_open())
    {
        return;
    }

    std::string type;
    int r, c;

    while (file >> type >> r >> c)
    {
        if (type == "GHOST")      ghost_count++;
        else if (type == "BAT")   bat_count++;
        else if (type == "CROSS") cross_count++;
    }

    file.clear();
    file.seekg(0, std::ios::beg);

    if (ghost_count > 0) {
        ghost_slots = new enemy_slot[ghost_count];
        ghost_spawn_positions = new AEVec2[ghost_count];
    }
    if (bat_count > 0) {
        bat_slots = new enemy_slot[bat_count];
        bat_spawn_positions = new AEVec2[bat_count];
    }
    if (cross_count > 0) {
        cross_positions = new AEVec2[cross_count];
    }

    int gc = 0, bc = 0, cc = 0;
    while (file >> type >> r >> c)
    {
        AEVec2 pos = tile_spot(r, c);
        if (type == "GHOST") {
            ghost_spawn_positions[gc] = pos;
            ghost_slots[gc++] = { nullptr, 0.0f };
        }
        else if (type == "BAT") {
            bat_spawn_positions[bc] = pos;
            bat_slots[bc++] = { nullptr, 0.0f };
        }
        else if (type == "CROSS") {
            cross_positions[cc++] = pos;
        }
    }

    file.close();
    spawn_positions_loaded = true;
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void EnemyManager::initialize_tutorial_area()
{
    load_textures();
    for (int i = 0; i < ghost_count; ++i) { ghost_slots[i] = { nullptr, 0.0f }; spawn_ghost_at_slot(i); }
    for (int i = 0; i < bat_count; ++i) { bat_slots[i] = { nullptr, 0.0f }; spawn_bat_at_slot(i); }
}

void EnemyManager::initialize_combat_area()
{
    load_textures();
    for (int i = 0; i < bat_count; ++i) { bat_slots[i] = { nullptr, 0.0f }; spawn_bat_at_slot(i); }
}

// ============================================================================
// UPDATE
// ============================================================================

void EnemyManager::update(f32 dt, Player* player, Camera::camera* p_cam)
{
    for (auto& enemy : enemies) {
        if (!enemy) continue;
        if (enemy->is_active() || enemy->get_state() == dead)
            enemy->update(dt, player, p_cam);
    }

    update_food(dt, player);
    update_respawn_particles(dt);

    if (current_zone == tutorial_area || current_zone == combat_area)
    {
        for (int i = 0; i < ghost_count; ++i)
            if (ghost_slots[i].enemy == nullptr && ghost_slots[i].respawn_timer > 0.0f)
                ghost_slots[i].respawn_timer -= dt;

        for (int i = 0; i < bat_count; ++i)
            if (bat_slots[i].enemy == nullptr && bat_slots[i].respawn_timer > 0.0f)
                bat_slots[i].respawn_timer -= dt;

        spawn_check_timer += dt;
        if (spawn_check_timer >= e_spawn_check_interval)
        {
            maintain_ghost_population();
            maintain_bat_population();
            spawn_check_timer = 0.0f;
        }
    }
    cleanup_dead_enemies();
}

// ============================================================================
// RENDER
// ============================================================================

void EnemyManager::render()
{
    render_spawn_markers();
    for (auto& enemy : enemies) {
        if (enemy->is_active() || enemy->get_state() == dead)
            render_enemy(enemy.get());
    }
    render_food();
    render_respawn_particles();
}

void EnemyManager::render_spawn_markers()
{
    if (current_zone == boss_room) return;
    if (!spawn_positions_loaded)   return;

    if (ghost_marker_tex && ghost_spawn_positions)
        for (int i = 0; i < ghost_count; ++i)
            render_sprite(square_mesh, ghost_spawn_positions[i].x, ghost_spawn_positions[i].y,
                e_tile_size, e_tile_size, ghost_marker_tex);

    if (bat_marker_tex && bat_spawn_positions)
        for (int i = 0; i < bat_count; ++i)
            render_sprite(square_mesh, bat_spawn_positions[i].x, bat_spawn_positions[i].y,
                e_tile_size, e_tile_size, bat_marker_tex);

    if (cross_tex && cross_positions)
        for (int i = 0; i < cross_count; ++i)
            render_sprite(square_mesh, cross_positions[i].x, cross_positions[i].y,
                e_tile_size, e_tile_size, cross_tex);
}

void EnemyManager::render_enemy(base_enemy* other_enemy)
{
    bool flashing = other_enemy->get_hit_flash_timer() > 0.f;
    AEGfxSetColorToAdd(flashing ? 1.f : 0.f, flashing ? 1.f : 0.f, flashing ? 1.f : 0.f, 0.f);

    dragon* d = dynamic_cast<dragon*>(other_enemy);
    if (d) {
        d->render_animated(other_enemy->get_size());
        AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
        return;
    }

    if (other_enemy->get_texture())
        render_sprite(square_mesh,
            other_enemy->get_x(), other_enemy->get_y(),
            other_enemy->get_size(), other_enemy->get_size(),
            other_enemy->get_texture());

    AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
}

// ============================================================================
// SPAWNING
// ============================================================================

void EnemyManager::spawn_ghost_at_slot(int idx)
{
    if (ghost_slots[idx].enemy != nullptr) return;
    AEVec2 pos = ghost_spawn_positions[idx];
    auto g = std::make_unique<ghost>(pos.x, pos.y);
    g->set_enemy_texture(ghost_tex);
    g->set_food_texture(ghost_food_tex);
    ghost_slots[idx].enemy = g.get();
    enemies.push_back(std::move(g));
}

void EnemyManager::spawn_bat_at_slot(int idx)
{
    if (bat_slots[idx].enemy != nullptr) return;
    AEVec2 pos = bat_spawn_positions[idx];
    auto b = std::make_unique<bat>(pos.x, pos.y);
    b->set_enemy_texture(bat_tex);
    b->set_food_texture(bat_food_tex);
    bat_slots[idx].enemy = b.get();
    enemies.push_back(std::move(b));
}

void EnemyManager::spawn_dragon(f32 x, f32 y, f32 r_left, f32 r_right, f32 r_top, f32 r_bottom)
{
    for (auto& enemy : enemies)
        if (enemy && enemy->get_state() != dead && dynamic_cast<dragon*>(enemy.get())) return;

    auto d = std::make_unique<dragon>(x, y, r_left, r_right, r_top, r_bottom);
    d->load_anim_textures();
    d->set_enemy_texture(dragon_tex);
    d->set_food_texture(dragon_food_tex);
    enemies.push_back(std::move(d));
}

// ============================================================================
// RESPAWN PARTICLES  (params from e_ globals)
// ============================================================================

void EnemyManager::spawn_respawn_particles(f32 x, f32 y)
{
    for (int i = 0; i < e_rp_burst_count; ++i)
    {
        Particle p;
        p.x = x; p.y = y;
        f32 angle = AERandFloat() * 2.0f * 3.14159f;
        f32 speed = e_rp_speed_min + AERandFloat() * (e_rp_speed_max - e_rp_speed_min);
        p.vx = AECos(angle) * speed;
        p.vy = AESin(angle) * speed + e_rp_rise_boost;
        p.maxLife = e_rp_life_min + AERandFloat() * (e_rp_life_max - e_rp_life_min);
        p.life = p.maxLife;
        p.size = e_rp_size_min + AERandFloat() * (e_rp_size_max - e_rp_size_min);
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
        p.vy -= e_rp_gravity * dt;
        p.life -= dt;
        if (p.life <= 0.0f) { respawn_particles.erase(respawn_particles.begin() + i); --i; }
    }
}

void EnemyManager::render_respawn_particles()
{
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    for (const auto& p : respawn_particles)
    {
        f32 alpha = p.life / p.maxLife;
        AEGfxSetColorToMultiply(0.6f + 0.4f * alpha, 0.3f, 1.0f, alpha);
        draw_circle(p.x, p.y, p.size * alpha, p.size * alpha);
    }
}

// ============================================================================
// FOOD  (lifetime + size from e_ globals)
// ============================================================================

void EnemyManager::spawn_food(f32 x, f32 y, AEGfxTexture* texture)
{
    food_drop food{};
    food.x = x;
    food.y = y;
    food.size = e_food_size;
    food.lifetime = e_food_lifetime;
    food.active = true;
    food.texture = texture;
    food_drops.push_back(food);
}

void EnemyManager::update_food(f32 dt, Player* player)
{
    for (auto& food : food_drops)
    {
        if (!food.active) continue;
        food.lifetime -= dt;
        if (food.lifetime <= 0.0f) { food.active = false; continue; }

        f32 dx = food.x - player->current_player->x;
        f32 dy = food.y - player->current_player->y;
        f32 dist = sqrtf(dx * dx + dy * dy);

        if (dist < (food.size + player->stats.character_size / 2.0f))
        {
            if (player->current_character == 1)
                player->stats.char1_curr_health = (int)AEMin(
                    (f32)player->stats.char1_curr_health + 1, (f32)player->stats.char1_max_health);
            else
                player->stats.char2_curr_health = (int)AEMin(
                    (f32)player->stats.char2_curr_health + 1, (f32)player->stats.char2_max_health);
            food.active = false;
        }
    }
    food_drops.erase(
        std::remove_if(food_drops.begin(), food_drops.end(),
            [](const food_drop& f) { return !f.active; }),
        food_drops.end());
}

void EnemyManager::render_food()
{
    for (const auto& food : food_drops)
        if (food.active && food.texture)
            render_sprite(square_mesh, food.x, food.y, food.size, food.size, food.texture);
}

// ============================================================================
// COMBAT
// ============================================================================

void EnemyManager::check_player_attacks(Player* player, f32 attack_range, f32 damage)
{
    for (auto& enemy : enemies)
    {
        if (!enemy || !enemy->is_active()) continue;
        if (enemy->distance_to_player(player) <= attack_range)
            enemy->take_damage(static_cast<int>(damage));
    }
}

bool EnemyManager::check_projectile_hit(f32 px, f32 py, f32 radius, f32 damage)
{
    for (auto& enemy : enemies)
    {
        if (!enemy || !enemy->is_active()) continue;
        if (cr_collision(px, py, radius, enemy->get_x(), enemy->get_y(),
            enemy->get_size(), enemy->get_size()))
        {
            enemy->take_damage(static_cast<int>(damage));
            return true;
        }
    }
    return false;
}

// ============================================================================
// CLEANUP
// ============================================================================

void EnemyManager::cleanup_dead_enemies()
{
    for (auto& enemy : enemies)
    {
        if (!enemy || enemy->is_active())  continue;
        if (enemy->get_state() != dead)    continue;

        if (!enemy->has_food_spawned())
        {
            enemy->set_food_spawned(true);
            if (enemy->is_ghost()) {
                ghost* g = static_cast<ghost*>(enemy.get());
                if (g->get_hits_from_player() > 0)
                    spawn_food(enemy->get_x(), enemy->get_y(), enemy->get_food_texture());
            }
            else
                spawn_food(enemy->get_x(), enemy->get_y(), enemy->get_food_texture());
        }

        if (enemy->is_ghost())
        {
            for (int i = 0; i < ghost_count; ++i)
                if (ghost_slots[i].enemy == enemy.get()) {
                    ghost_slots[i].enemy = nullptr;
                    ghost_slots[i].respawn_timer = e_respawn_delay;
                    break;
                }
        }
        else if (dynamic_cast<bat*>(enemy.get()))
        {
            for (int i = 0; i < bat_count; ++i)
                if (bat_slots[i].enemy == enemy.get()) {
                    bat_slots[i].enemy = nullptr;
                    bat_slots[i].respawn_timer = e_respawn_delay;
                    break;
                }
        }
    }

    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const auto& e) { return !e->is_active() && e->get_state() == dead; }),
        enemies.end());
}

// ============================================================================
// POPULATION MAINTENANCE
// ============================================================================

void EnemyManager::maintain_ghost_population()
{
    for (int i = 0; i < ghost_count; ++i)
        if (ghost_slots[i].enemy == nullptr && ghost_slots[i].respawn_timer <= 0.f) {
            spawn_respawn_particles(ghost_spawn_positions[i].x, ghost_spawn_positions[i].y);
            spawn_ghost_at_slot(i);
            return;
        }
}

void EnemyManager::maintain_bat_population()
{
    for (int i = 0; i < bat_count; ++i)
        if (bat_slots[i].enemy == nullptr && bat_slots[i].respawn_timer <= 0.f) {
            spawn_respawn_particles(bat_spawn_positions[i].x, bat_spawn_positions[i].y);
            spawn_bat_at_slot(i);
            return;
        }
}

// ============================================================================
// GETTERS / UTILITIES
// ============================================================================

size_t EnemyManager::get_active_enemy_count() const
{
    return std::count_if(enemies.begin(), enemies.end(),
        [](const std::unique_ptr<base_enemy>& e) { return e && e->is_active(); });
}

void EnemyManager::clear_all()
{
    enemies.clear();
    food_drops.clear();
    respawn_particles.clear();

    delete[] ghost_slots;            ghost_slots = nullptr;
    delete[] bat_slots;              bat_slots = nullptr;
    delete[] ghost_spawn_positions;  ghost_spawn_positions = nullptr;
    delete[] bat_spawn_positions;    bat_spawn_positions = nullptr;
    delete[] cross_positions;        cross_positions = nullptr;

    ghost_count = bat_count = cross_count = 0;
    spawn_check_timer = 0.0f;
    spawn_positions_loaded = false;
}

bool EnemyManager::save_spawn_positions_from_map(s32** map_data, const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    for (int r = 0; r < TILE_ROWS; ++r)
        for (int c = 0; c < TILE_COLS; ++c)
        {
            if (map_data[r][c] == 5) {
                file << "GHOST  " << r + 1 << " " << c << "\n";
                file << "CROSS  " << r << " " << c << "\n";
            }
            else if (map_data[r][c] == 6)
                file << "BAT  " << r << " " << c << "\n";
        }

    file.close();
    return true;
}

void entity_system_update(Player* player_ptr, EnemyManager* enem)
{
    if (!player_ptr) return;
    (void)enem;
    f32 dt = (f32)AEFrameRateControllerGetFrameTime();
    player_update_projectiles(player_ptr, dt);
}