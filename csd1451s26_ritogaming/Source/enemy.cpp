/*!************************************************************************
\file:          enemy.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:
**************************************************************************/

#include "pch.hpp"

extern s32 map[TILE_ROWS][TILE_COLS];
extern std::vector<Wall> g_walls;

//------------------------------- Pathfinding --------------------------------
void enemy_move_towards(f32& x, f32& y, f32 speed, f32 size,
    f32 target_x, f32 target_y, f32 dt) {
    f32 dx = target_x - x;
    f32 dy = target_y - y;
    f32 dist = sqrtf(dx * dx + dy * dy);
    if (dist <= 1.0f) return;

    dx /= dist; dy /= dist;
    f32 nx = x + dx * speed * dt;
    f32 ny = y + dy * speed * dt;
    if (!check_player_collision(nx, y, size / 2.0f)) x = nx;
    if (!check_player_collision(x, ny, size / 2.0f)) y = ny;
}

void enemy_move_away_from(f32& x, f32& y, f32 speed, f32 size,
    f32 from_x, f32 from_y, f32 dt) {
    f32 dx = x - from_x;
    f32 dy = y - from_y;
    f32 dist = sqrtf(dx * dx + dy * dy);
    if (dist <= 1.0f) return;

    dx /= dist; dy /= dist;
    f32 nx = x + dx * speed * dt;
    f32 ny = y + dy * speed * dt;
    if (!check_player_collision(nx, y, size / 2.0f)) x = nx;
    if (!check_player_collision(x, ny, size / 2.0f)) y = ny;
}

void enemy_flee_to_wall(f32 x, f32 y, f32 speed,
    f32 player_x, f32 player_y, f32 dt,
    f32& next_x, f32& next_y) {
    f32 dx = x - player_x;
    f32 dy = y - player_y;
    f32 dist = sqrtf(dx * dx + dy * dy);
    if (dist < 1.0f) { dx = 1.0f; dy = 0.0f; }
    else { dx /= dist; dy /= dist; }

    next_x = x + dx * speed * dt;
    next_y = y + dy * speed * dt;
}

// ============================================================================
// BASE ENEMY IMPLEMENTATION
// ============================================================================
base_enemy::base_enemy(f32 start_x, f32 start_y)
    : x(start_x), y(start_y), spawn_x(start_x), spawn_y(start_y),
    velocity_x(0), velocity_y(0), speed(50.0f), size(25.0f),
    current_health(2), max_health(2), attack_cooldown(1.0f), attack_timer(0.0f),
    state(enemy_state::idle), active(true),
    visible_to_player(false), collision_radius(15.0f),
    sight_range(300.0f),
    texture_path(nullptr), food_texture_path(nullptr),
    texture(nullptr), food_texture(nullptr)
{
}

void base_enemy::take_damage(int amount) {
    if (!active) return;

    current_health -= amount;

    if (current_health <= 0) {
        current_health = 0;
        state = enemy_state::dead;
        active = false;
    }
}

bool base_enemy::is_in_camera_view(camera* cam) const {
    if (!cam) return false;

    extern f32 zoom;
    f32 hw = (AEGfxGetWindowWidth() / 2.0f) / zoom;
    f32 hh = (AEGfxGetWindowHeight() / 2.0f) / zoom;

    return (x > cam->pos.x - hw - size && x < cam->pos.x + hw + size &&
        y > cam->pos.y - hh - size && y < cam->pos.y + hh + size);
}

bool base_enemy::collides_with_player(Player* player) const {
    if (!player || !player->current_player) return false;

    f32 dx = x - player->current_player->x;
    f32 dy = y - player->current_player->y;
    f32 dist = sqrtf(dx * dx + dy * dy);

    return dist < (collision_radius + player->stats.character_size / 2.0f);
}

f32 base_enemy::distance_to_player(Player* player) const {
    if (!player || !player->current_player) return 10000.0f;

    f32 dx = x - player->current_player->x;
    f32 dy = y - player->current_player->y;
    return sqrtf(dx * dx + dy * dy);
}

// ============================================================================
// GHOST BOY IMPLEMENTATION
// ============================================================================
ghost::ghost(f32 start_x, f32 start_y)
    : base_enemy(start_x, start_y)
{
    speed = 80.0f;
    current_health = 2;
    max_health = 2;
    size = 50.0f;
    sight_range = 400.0f;
    texture_path = "Assets/Tiles/tile_0006.png";
    food_texture_path = "Assets/kenney_pixel-platformer_food_expansion/cream.png";
}

void ghost::update(f32 dt, Player* player, camera* p_cam) {
    if (!active) return;

    visible_to_player = is_in_camera_view(p_cam);

    if (visible_to_player) {
        state = enemy_state::fleeing;
        f32 next_x{}, next_y{};

        enemy_flee_to_wall(x, y, speed, player->current_player->x, player->current_player->y,
            dt, next_x, next_y);

        // flee to wall resolution
        if (!check_player_collision(next_x, y, size / 2.0f)) {
            x = next_x;
        }
        else {
            state = enemy_state::dead;
        }

        if (!check_player_collision(x, next_y, size / 2.0f)) {
            y = next_y;
        }
        else {
            state = enemy_state::dead;
        }
    }
    else {
        state = enemy_state::idle;
    }
}

// ============================================================================
// BAT IMPLEMENTATION
// ============================================================================

bat::bat(f32 start_x, f32 start_y)
    : base_enemy(start_x, start_y)
{
    speed = 80.0f;
    current_health = 4;
    max_health = 4;
    size = 70.0f;
    sight_range = 350.0f;
    texture_path = "Assets/kenney_pixel-platformer/tile_0025.png";
    food_texture_path = "Assets/kenney_pixel-platformer_food_expansion/burger.png";
}

void bat::update(f32 dt, Player* player, camera* p_cam) {
    if (!active) return;

    visible_to_player = is_in_camera_view(p_cam);

    if (visible_to_player) {
        state = enemy_state::chasing;
        enemy_move_towards(x, y, speed, size,
            player->current_player->x,
            player->current_player->y, dt);

        if (collides_with_player(player)) {
            state = enemy_state::attacking;
            attack_timer += dt;
            if (attack_timer >= attack_cooldown) {
                player_take_damage(player, 3); // 1 unit = half a heart
                attack_timer = 0.0f;
            }
        }
    }
    else {
        state = enemy_state::idle;
    }
}

// ============================================================================
// DRAGON IMPLEMENTATION
// ============================================================================

dragon::dragon(f32 start_x, f32 start_y, f32 r_left, f32 r_right, f32 r_top, f32 r_bottom)
    : base_enemy(start_x, start_y), room_left(r_left), room_right(r_right),
    room_top(r_top), room_bottom(r_bottom),
    hits_received(0), hits_until_attack(7), is_boss(true)
{
    speed = 40.0f;
    current_health = 2;
    max_health = 2;
    size = 150.0f;
    texture_path = "Assets/Tiles/tile_0005.png";
    food_texture_path = "Assets/kenney_pixel-platformer_food_expansion/sushi.png";
}

void dragon::update(f32 dt, Player* player, camera* p_cam) {
    if (!active) return;

    visible_to_player = is_in_camera_view(p_cam);

    if (visible_to_player) {
        state = enemy_state::chasing;

        f32 target_x = player->current_player->x;
        f32 target_y = player->current_player->y;

        target_x = AEClamp(target_x, room_left, room_right);
        target_y = AEClamp(target_y, room_bottom, room_top);

        enemy_move_towards(x, y, speed, size, target_x, target_y, dt);

        x = AEClamp(x, room_left, room_right);
        y = AEClamp(y, room_bottom, room_top);

        if (hits_received >= hits_until_attack && collides_with_player(player)) {
            state = enemy_state::attacking;
            attack_timer += dt;
            if (attack_timer >= attack_cooldown) {
                player_take_damage(player, 1);
                hits_received = 0;
                attack_timer = 0.0f;
            }
        }
    }
    else {
        state = enemy_state::idle;
    }
}

void dragon::take_damage(int amount) {
    if (!active) return;

    hits_received++;
    base_enemy::take_damage(amount);
}

