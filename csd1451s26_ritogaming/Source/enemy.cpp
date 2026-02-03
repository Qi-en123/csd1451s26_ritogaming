/*!************************************************************************
\file:          enemy.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:
**************************************************************************/

#include "enemy.hpp"
#include "map.hpp"

extern s32 map[TILE_ROWS][TILE_COLS];
extern std::vector<Wall> g_walls;

// ============================================================================
// BASE ENEMY IMPLEMENTATION
// ============================================================================
base_enemy::base_enemy(enemy_type enemy_type, f32 start_x, f32 start_y)
    : x(start_x), y(start_y), spawn_x(start_x), spawn_y(start_y),
    velocity_x(0), velocity_y(0), speed(50.0f), size(25.0f),
    max_health(100.0f), current_health(100.0f), damage(10.0f),
    attack_cooldown(1.0f), attack_timer(0.0f),
    type(enemy_type), state(enemy_state::idle), active(true),
    visible_to_player(false), collision_radius(15.0f),
    sight_range(300.0f), reset_range(500.0f),
    texture(nullptr), animation_timer(0.0f),
    food_heal_percentage(0.35f), food_texture_path(nullptr)
{
}

void base_enemy::take_damage(f32 amount) {
    if (!active) return;

    current_health -= amount;

    if (current_health <= 0) {
        current_health = 0;
        state = enemy_state::dead;
        active = false;
    }
}

void base_enemy::move_towards(f32 target_x, f32 target_y, f32 dt) {
    f32 dx = target_x - x;
    f32 dy = target_y - y;
    f32 dist = sqrtf(dx * dx + dy * dy);

    if (dist > 1.0f) {
        dx /= dist;
        dy /= dist;

        f32 next_x = x + dx * speed * dt;
        f32 next_y = y + dy * speed * dt;

        if (!check_player_collision(next_x, y, size / 2.0f)) {
            x = next_x;
        }
        if (!check_player_collision(x, next_y, size / 2.0f)) {
            y = next_y;
        }
    }
}

void base_enemy::move_away_from(f32 target_x, f32 target_y, f32 dt) {
    f32 dx = x - target_x;
    f32 dy = y - target_y;
    f32 dist = sqrtf(dx * dx + dy * dy);

    if (dist > 1.0f) {
        dx /= dist;
        dy /= dist;

        f32 next_x = x + dx * speed * dt;
        f32 next_y = y + dy * speed * dt;

        if (!check_player_collision(next_x, y, size / 2.0f)) {
            x = next_x;
        }
        if (!check_player_collision(x, next_y, size / 2.0f)) {
            y = next_y;
        }
    }
}

void base_enemy::flee_to_wall(f32 player_x, f32 player_y, f32 dt) {
    f32 dx = x - player_x;
    f32 dy = y - player_y;
    f32 dist = sqrtf(dx * dx + dy * dy);

    if (dist < 1.0f) {
        dx = 1.0f;
        dy = 0.0f;
    }
    else {
        dx /= dist;
        dy /= dist;
    }

    f32 next_x = x + dx * speed * dt;
    f32 next_y = y + dy * speed * dt;

    if (!check_player_collision(next_x, y, size / 2.0f)) {
        x = next_x;
    }
    else {
        active = false;
    }

    if (!check_player_collision(x, next_y, size / 2.0f)) {
        y = next_y;
    }
    else {
        active = false;
    }
}

bool base_enemy::is_in_camera_view(camera* cam) const {
    if (!cam) return false;

    extern f32 zoom;
    f32 half_screen_w = (AEGfxGetWindowWidth() / 2.0f) / zoom;
    f32 half_screen_h = (AEGfxGetWindowHeight() / 2.0f) / zoom;

    return (x > cam->pos.x - half_screen_w - size &&
        x < cam->pos.x + half_screen_w + size &&
        y > cam->pos.y - half_screen_h - size &&
        y < cam->pos.y + half_screen_h + size);
}

bool base_enemy::collides_with_player(Player* player) const {
    if (!player || !player->current_player) return false;

    f32 dx = x - player->current_player->x;
    f32 dy = y - player->current_player->y;
    f32 dist = sqrtf(dx * dx + dy * dy);

    return dist < (collision_radius + player->config.character_size / 2.0f);
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
ghost_boy::ghost_boy(f32 start_x, f32 start_y)
    : base_enemy(enemy_type::ghost_boy, start_x, start_y),
    flee_timer(0.0f), has_reached_wall(false)
{
    speed = 80.0f;
    max_health = 50.0f;
    current_health = 50.0f;
    size = 50.0f;
    food_heal_percentage = 0.35f;  // 35% heal
    sight_range = 400.0f;

    texture = "Assets/Tiles/tile_0006.png";
    food_texture_path = "Assets/kenney_pixel-platformer_food_expansion/cream.png";
}

void ghost_boy::update(f32 dt, Player* player, camera* cam) {
    if (!active) return;

    visible_to_player = is_in_camera_view(cam);

    if (visible_to_player) {
        state = enemy_state::fleeing;
        flee_to_wall(player->current_player->x, player->current_player->y, dt);
    }
    else {
        state = enemy_state::idle;
    }
}

// ============================================================================
// BAT IMPLEMENTATION
// ============================================================================

bat::bat(f32 start_x, f32 start_y)
    : base_enemy(enemy_type::bat, start_x, start_y), aggro_timer(0.0f)
{
    speed = 60.0f;
    max_health = 80.0f;
    current_health = 80.0f;
    size = 60.0f;
    food_heal_percentage = 0.50f;  // 50% heal
    sight_range = 350.0f;

    texture = "Assets/kenney_pixel-platformer/tile_0025.png";
    food_texture_path = "Assets/kenney_pixel-platformer_food_expansion/burger.png";
}

void bat::update(f32 dt, Player* player, camera* cam) {
    if (!active) return;

    visible_to_player = is_in_camera_view(cam);

    if (visible_to_player) {
        state = enemy_state::chasing;
        move_towards(player->current_player->x, player->current_player->y, dt);

        if (collides_with_player(player)) {
            state = enemy_state::attacking;
            attack_timer += dt;
            if (attack_timer >= attack_cooldown) {
                // player_take_damage or use HUD system:
                // player_take_damage(player, damage);
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
    : base_enemy(enemy_type::dragon, start_x, start_y),
    room_left(r_left), room_right(r_right),
    room_top(r_top), room_bottom(r_bottom),
    hits_received(0), hits_until_attack(7), is_boss(true)
{
    speed = 40.0f;
    max_health = 300.0f;
    current_health = 300.0f;
    damage = 50.0f;
    size = 150.0f;
    food_heal_percentage = 1.0f;

    texture = "Assets/Tiles/tile_0005.png";
    food_texture_path = "Assets/kenney_pixel-platformer_food_expansion/sushi.png";
}

void dragon::update(f32 dt, Player* player, camera* cam) {
    if (!active) return;

    visible_to_player = is_in_camera_view(cam);

    if (visible_to_player) {
        state = enemy_state::chasing;

        f32 target_x = player->current_player->x;
        f32 target_y = player->current_player->y;

        target_x = AEClamp(target_x, room_left, room_right);
        target_y = AEClamp(target_y, room_bottom, room_top);

        move_towards(target_x, target_y, dt);

        x = AEClamp(x, room_left, room_right);
        y = AEClamp(y, room_bottom, room_top);

        if (hits_received >= hits_until_attack && collides_with_player(player)) {
            state = enemy_state::attacking;
            attack_timer += dt;
            if (attack_timer >= attack_cooldown) {
                // player_take_damage or use HUD system:
                // player_take_damage(player, damage);
                hits_received = 0;
                attack_timer = 0.0f;
            }
        }
    }
    else {
        state = enemy_state::idle;
    }
}

void dragon::take_damage(f32 amount) {
    if (!active) return;

    hits_received++;
    base_enemy::take_damage(amount);
}