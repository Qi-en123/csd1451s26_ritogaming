/*!************************************************************************
\file:          enemy_manager.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         Enemy manager implementation - simplified slot-based spawning
**************************************************************************/

#include "enemy_manager.hpp"
#include "map.hpp"

extern s32 map[TILE_ROWS][TILE_COLS];

enemy_manager g_enemy_manager;

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================
enemy_manager::enemy_manager() {
    ghost_food_tex = AEGfxTextureLoad("Assets/kenney_pixel-platformer_food_expansion/cream.png");
    bat_food_tex = AEGfxTextureLoad("Assets/kenney_pixel-platformer_food_expansion/burger.png");
    dragon_food_tex = AEGfxTextureLoad("Assets/kenney_pixel-platformer_food_expansion/sushi.png");
}

enemy_manager::~enemy_manager() {
    clear_all();
    if (ghost_food_tex)  AEGfxTextureUnload(ghost_food_tex);
    if (bat_food_tex)    AEGfxTextureUnload(bat_food_tex);
    if (dragon_food_tex) AEGfxTextureUnload(dragon_food_tex);
}

// ============================================================================
// GHOST SLOT INITIALIZATION
// ============================================================================
void enemy_manager::initialize_ghost_slots() {
    // Based on your map layout:
    // Tile size: 64.0f
    // Map origin: X=-800, Y=450
    // Player Room 1 (Top Left): rows 3-10, cols 2-9

    const f32 TILE_SIZE = 64.0f;
    const f32 MAP_START_X = -1600.0f / 2.0f;  // -800
    const f32 MAP_START_Y = 900.0f / 2.0f;     // 450

    // Helper lambda to convert tile to world position
    auto tile_to_world = [MAP_START_X, MAP_START_Y, TILE_SIZE](s32 row, s32 col) -> AEVec2 {
        AEVec2 p;
        p.x = MAP_START_X + col * TILE_SIZE;
        p.y = MAP_START_Y - row * TILE_SIZE;
        return p;
        };

    // GHOSTS in TOP LEFT ROOM (Player Room 1): rows 3-10, cols 2-9

    // 4 ghosts in Top Left Room (Player Room 1) - spread out
    AEVec2 p0 = tile_to_world(5, 4);   // Top area
    AEVec2 p1 = tile_to_world(7, 6);   // Center
    AEVec2 p2 = tile_to_world(9, 4);   // Bottom-left
    AEVec2 p3 = tile_to_world(9, 8);   // Bottom-right

    // 3 ghosts near the right door area
    AEVec2 p4 = tile_to_world(6, 9);   // Near right door
    AEVec2 p5 = tile_to_world(7, 9);   // Near right door
    AEVec2 p6 = tile_to_world(8, 9);   // Near right door

    // 3 ghosts in the top area
    AEVec2 p7 = tile_to_world(4, 5);   // Top-center left
    AEVec2 p8 = tile_to_world(4, 7);   // Top-center right
    AEVec2 p9 = tile_to_world(6, 3);   // Left side

    ghost_slots[0] = { p0.x, p0.y, nullptr, 0.0f };
    ghost_slots[1] = { p1.x, p1.y, nullptr, 0.0f };
    ghost_slots[2] = { p2.x, p2.y, nullptr, 0.0f };
    ghost_slots[3] = { p3.x, p3.y, nullptr, 0.0f };
    ghost_slots[4] = { p4.x, p4.y, nullptr, 0.0f };
    ghost_slots[5] = { p5.x, p5.y, nullptr, 0.0f };
    ghost_slots[6] = { p6.x, p6.y, nullptr, 0.0f };
    ghost_slots[7] = { p7.x, p7.y, nullptr, 0.0f };
    ghost_slots[8] = { p8.x, p8.y, nullptr, 0.0f };
    ghost_slots[9] = { p9.x, p9.y, nullptr, 0.0f };

    // Spawn all ghosts initially
    for (int i = 0; i < NUM_GHOST_SLOTS; ++i) {
        spawn_ghost_at_slot(i);
    }
}
// ============================================================================
// UPDATE AND RENDER
// ============================================================================
//void enemy_manager::update(f32 dt, Player* player, camera* cam) {
//    // Update all enemies
//    for (auto& e : enemies) {
//        if (e->is_active()) {
//            e->update(dt, player, cam);
//        }
//    }
//
//    // Update food drops
//    update_food(dt, player);
//
//    // Update respawn timers for empty slots
//    for (int i = 0; i < NUM_GHOST_SLOTS; ++i) {
//        if (ghost_slots[i].ghost == nullptr && ghost_slots[i].respawn_timer > 0.0f) {
//            ghost_slots[i].respawn_timer -= dt;
//        }
//    }
//
//    // Clean up dead enemies and maintain population
//    cleanup_dead_enemies();
//
//    // Periodically check if we need to respawn ghosts
//    spawn_check_timer += dt;
//    if (spawn_check_timer >= SPAWN_CHECK_INTERVAL) {
//        maintain_ghost_population();
//        spawn_check_timer = 0.0f;
//    }
//}

void enemy_manager::update(f32 dt, Player* player, camera* cam) {
    // Only maintain ghost population in Level 1
    if (is_in_level1) {
        // Update all enemies
        for (auto& e : enemies) {
            if (e->is_active()) {
                e->update(dt, player, cam);
            }
        }

    for (int i = 0; i < NUM_GHOST_SLOTS; ++i) {
        if (ghost_slots[i].ghost == nullptr && ghost_slots[i].respawn_timer > 0.0f) {
            ghost_slots[i].respawn_timer -= dt;
        }
    }

    // Clean up dead enemies and maintain population
    cleanup_dead_enemies();

        // Periodically check if we need to respawn ghosts
        spawn_check_timer += dt;
        if (spawn_check_timer >= SPAWN_CHECK_INTERVAL) {
            maintain_ghost_population();
            spawn_check_timer = 0.0f;
        }
    }
    else {
        // Level 2/Boss Room - only update existing enemies (dragon)
        for (auto& e : enemies) {
            if (e->is_active()) {
                e->update(dt, player, cam);
            }
        }
        update_food(dt, player);
    }

    // Clean up dead enemies
    cleanup_dead_enemies();
}

void enemy_manager::render() {
    for (auto& e : enemies) {
        if (e->is_active()) {
            render_enemy(e.get());
        }
    }
    render_food();
}

//void enemy_manager::render_enemy(base_enemy* e) {
//    const char* path = e->get_texture_path();
//    AEGfxTexture* tex = AEGfxTextureLoad(path);
//    render_sprite(rect_mesh_center, e->get_x(), e->get_y(),
//        e->get_size(), e->get_size(), tex);
//    AEGfxTextureUnload(tex);
//}

void enemy_manager::render_enemy(base_enemy* e) {
    const char* path = e->get_texture_path();
    AEGfxTexture* tex = AEGfxTextureLoad(path);

    if (tex) {
        render_sprite(rect_mesh_center, e->get_x(), e->get_y(),
            e->get_size(), e->get_size(), tex);
        AEGfxTextureUnload(tex);
    }
    else {
        // Texture failed to load, but don't crash
        printf("[WARNING] Failed to load texture: %s\n", path);
    }
}

// ============================================================================
// SPAWNING FUNCTIONS
// ============================================================================
void enemy_manager::spawn_ghost_at_slot(int slot_index) {
    if (slot_index < 0 || slot_index >= NUM_GHOST_SLOTS) {
        printf("[Enemy Manager] Invalid slot index: %d\n", slot_index);
        return;
    }

    // Check if slot is already occupied
    if (ghost_slots[slot_index].ghost != nullptr) {
        return; // Slot already has a ghost
    }

    // Create a new ghost
    f32 x = ghost_slots[slot_index].spawn_x;
    f32 y = ghost_slots[slot_index].spawn_y;

    enemies.push_back(std::make_unique<ghost_boy>(x, y));
    ghost_slots[slot_index].ghost = enemies.back().get();

    printf("[Enemy Manager] Spawned ghost at slot %d: (%.1f, %.1f)\n", slot_index, x, y);
}

void enemy_manager::spawn_dragon(f32 x, f32 y, f32 r_left, f32 r_right, f32 r_top, f32 r_bottom) {
    // Check if a dragon already exists
    for (auto& enemy : enemies) {
        if (enemy && enemy->get_type() == enemy_type::dragon && enemy->is_active()) {
            printf("[Enemy Manager] Dragon already exists, not spawning another.\n");
            return;
        }
    }

    // Create a new dragon
    enemies.push_back(std::make_unique<dragon>(x, y, r_left, r_right, r_top, r_bottom));
    printf("[Enemy Manager] Spawned dragon at (%.1f, %.1f)\n", x, y);
}
// ============================================================================
// FOOD SYSTEM
// ============================================================================
void enemy_manager::spawn_food(f32 x, f32 y, f32 heal_percentage, const char* texture_path) {
    food_drop food;
    food.x = x;
    food.y = y;
    food.heal_percentage = heal_percentage;
    food.size = 20.0f;
    food.lifetime = 30.0f;
    food.active = true;
    food.texture = nullptr;

    // Use pre-loaded texture based on enemy type
    if (std::strcmp(texture_path, "Assets/kenney_pixel-platformer_food_expansion/cream.png") == 0) {
        food.texture = ghost_food_tex;
    }
    else if (std::strcmp(texture_path, "Assets/kenney_pixel-platformer_food_expansion/burger.png") == 0) {
        food.texture = bat_food_tex;
    }
    else if (std::strcmp(texture_path, "Assets/kenney_pixel-platformer_food_expansion/sushi.png") == 0) {
        food.texture = dragon_food_tex;
    }
    else {
        food.texture = AEGfxTextureLoad(texture_path);
    }

    food_drops.push_back(food);
}

void enemy_manager::update_food(f32 dt, Player* player) {
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

        if (dist < (food.size + player->config.character_size / 2.0f)) {
            printf("[Food] Player picked up food! Healing %.0f%%\n", food.heal_percentage * 100.0f);
            // TODO: Actually heal the player here
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

void enemy_manager::render_food() {
    for (const auto& food : food_drops) {
        if (food.active && food.texture) {
            render_sprite(rect_mesh_center, food.x, food.y,
                food.size, food.size, food.texture);
        }
    }
}

// ============================================================================
// COMBAT
// ============================================================================
void enemy_manager::check_player_attacks(Player* player, f32 attack_range, f32 damage) {
    for (auto& enemy : enemies) {
        if (!enemy || !enemy->is_active()) continue;

        f32 dist = enemy->distance_to_player(player);
        if (dist <= attack_range) {
            enemy->take_damage(damage);
            printf("[Combat] Enemy hit! HP: %.0f/%.0f\n",
                enemy->get_current_health(), enemy->get_max_health());
        }
    }
}

// ============================================================================
// CLEANUP AND MAINTENANCE
// ============================================================================
void enemy_manager::cleanup_dead_enemies() {
    // Find which ghosts died and mark their slots as empty with respawn timer
    for (int i = 0; i < NUM_GHOST_SLOTS; ++i) {
        if (ghost_slots[i].ghost != nullptr) {
            if (ghost_slots[i].ghost->is_dead()) {
                printf("[Enemy Manager] Ghost in slot %d died, respawning in %.1fs\n",
                    i, GHOST_RESPAWN_DELAY);

                // Spawn food where the ghost died
                spawn_food(ghost_slots[i].ghost->get_x(),
                    ghost_slots[i].ghost->get_y(),
                    ghost_slots[i].ghost->get_food_heal_percentage(),
                    ghost_slots[i].ghost->get_food_texture_path());

                // Clear the slot and set respawn timer
                ghost_slots[i].ghost = nullptr;
                ghost_slots[i].respawn_timer = GHOST_RESPAWN_DELAY;
            }
        }
    }

    // Remove dead enemies from the vector
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const auto& e) { return e->is_dead(); }),
        enemies.end()
    );
}

void enemy_manager::maintain_ghost_population() {
    // Only respawn ONE ghost at a time whose timer has expired
    for (int i = 0; i < NUM_GHOST_SLOTS; ++i) {
        if (ghost_slots[i].ghost == nullptr && ghost_slots[i].respawn_timer <= 0.0f) {
            spawn_ghost_at_slot(i);
            return; // Only spawn one ghost per check, creating staggered respawns
        }
    }
}

// ============================================================================
// UTILITY
// ============================================================================
size_t enemy_manager::get_active_enemy_count() const {
    return std::count_if(enemies.begin(), enemies.end(),
        [](const std::unique_ptr<base_enemy>& e) {
            return e && e->is_active();
        });
}

size_t enemy_manager::get_ghost_count() const {
    return std::count_if(enemies.begin(), enemies.end(),
        [](const auto& e) {
            return e->is_active() && e->get_type() == enemy_type::ghost_boy;
        });
}

void enemy_manager::clear_all() {
    enemies.clear();
    food_drops.clear();

    // Clear all slot references and timers
    for (int i = 0; i < NUM_GHOST_SLOTS; ++i) {
        ghost_slots[i].ghost = nullptr;
        ghost_slots[i].respawn_timer = 0.0f;
    }
}

// ============================================================================
// PROJECTILE TO ENEMY
// ============================================================================v
bool enemy_manager::check_projectile_hit(f32 px, f32 py, f32 radius, f32 damage) {
    // This is the SAME loop logic you posted!
    for (auto& enemy : enemies) {
        if (!enemy || !enemy->is_active()) continue;

        // This is the SAME collision check!
        if (cr_collision(px, py, radius,
            enemy->get_x(), enemy->get_y(),
            enemy->get_size(), enemy->get_size()))
        {
            enemy->take_damage(damage); // SAME damage logic!
            return true;
        }
    }
    return false;
}