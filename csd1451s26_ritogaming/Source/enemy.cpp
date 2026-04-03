/*!************************************************************************
\file:          enemy.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\
\brief:         Implements enemy movement behaviors and enemy type classes.
\               Contains pathfinding helper functions, base_enemy class,
\               and derived enemy types (ghost, bat, dragon) with unique
\               AI behaviors.
**************************************************************************/

#include "pch.hpp"

extern s32 map[TILE_ROWS][TILE_COLS];
extern std::vector<Wall> g_walls;

// ============================================================================
// BOSS PROJECTILE MANAGER  implementation
// ============================================================================

BossProjectileManager boss_proj_manager;

static constexpr float BP_DEG2RAD = 3.14159265f / 180.0f;

static void bp_rotate_vec(float x, float y, float angle_rad,
                           float& out_x, float& out_y)
{
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    out_x = x * c - y * s;
    out_y = x * s + y * c;
}

BossProjectileManager::BossProjectileManager() {}

void BossProjectileManager::init(int pool_size)
{
    m_pool.clear();
    m_pool.resize(static_cast<size_t>(pool_size));
    m_wave_toggle = false;
}

void BossProjectileManager::clear()
{
    for (auto& p : m_pool)
        p.in_use = false;
}

void BossProjectileManager::update(float dt, const camera* p_cam)
{
    if (!p_cam) return;

    extern float zoom;
    const float hw = (AEGfxGetWindowWidth()  * 0.5f) / zoom;
    const float hh = (AEGfxGetWindowHeight() * 0.5f) / zoom;
    const float margin = BossProjCfg::RING_RADIUS * 4.0f;

    for (auto& p : m_pool)
    {
        if (!p.in_use) continue;

        p.age      += dt;
        p.lifetime -= dt;

        // Sinusoidal perpendicular offset applied each frame ? crescent path
        const float sine_val = sinf(p.age * BossProjCfg::WAVE_FREQUENCY + p.phase)
                             * BossProjCfg::WAVE_AMPLITUDE;

        p.x += (p.vx + p.perp_x * sine_val) * dt;
        p.y += (p.vy + p.perp_y * sine_val) * dt;

        const bool expired    = (p.lifetime <= 0.0f);
        const bool off_screen = (p.x < p_cam->pos.x - hw - margin) ||
                                (p.x > p_cam->pos.x + hw + margin) ||
                                (p.y < p_cam->pos.y - hh - margin) ||
                                (p.y > p_cam->pos.y + hh + margin);
        if (expired || off_screen)
            p.in_use = false;
    }
}

int BossProjectileManager::check_player_collision(Player* player)
{
    if (!player || !player->current_player) return 0;

    const float px    = player->current_player->x;
    const float py    = player->current_player->y;
    const float p_rad = player->stats.character_size * 0.5f;

    int hits = 0;
    for (auto& proj : m_pool)
    {
        if (!proj.in_use) continue;
        if (cc_collision(proj.x, proj.y, BossProjCfg::RING_RADIUS, px, py, p_rad))
        {
            proj.in_use = false;
            player_take_damage(player, BossProjCfg::PROJECTILE_DAMAGE);
            ++hits;
        }
    }
    return hits;
}

void BossProjectileManager::trigger_wave(float boss_x, float boss_y,
                                          float target_x, float target_y)
{
    // Build normalised forward direction (boss ? player)
    float dx = target_x - boss_x;
    float dy = target_y - boss_y;
    const float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 1.0f) { dx = 1.0f; dy = 0.0f; }
    else             { dx /= dist; dy /= dist; }

    // Perpendicular (90° CCW rotation)
    const float perp_x = -dy;
    const float perp_y =  dx;

    // 9-slot fan; Wave A ? even indices, Wave B ? odd indices
    const int   total_slots = BossProjCfg::POSITIONS_WAVE_A + BossProjCfg::POSITIONS_WAVE_B;
    const float spread_rad  = BossProjCfg::ANGLE_SPREAD_DEG * BP_DEG2RAD;
    const float slot_step   = spread_rad / static_cast<float>(total_slots - 1);

    const bool is_wave_b = m_wave_toggle;
    const int  count     = is_wave_b ? BossProjCfg::POSITIONS_WAVE_B
                                     : BossProjCfg::POSITIONS_WAVE_A;

    for (int i = 0; i < count; ++i)
    {
        const int   slot         = is_wave_b ? (i * 2 + 1) : (i * 2);
        const float angle_offset = -spread_rad * 0.5f + static_cast<float>(slot) * slot_step;

        float shot_dx, shot_dy, shot_px, shot_py;
        bp_rotate_vec(dx,     dy,     angle_offset, shot_dx, shot_dy);
        bp_rotate_vec(perp_x, perp_y, angle_offset, shot_px, shot_py);

        // Stagger phase so rings interleave when both waves are in-flight
        const float base_phase        = static_cast<float>(slot)
                                      * (2.0f * 3.14159265f / static_cast<float>(total_slots));
        const float wave_phase_offset = is_wave_b
                                      ? (3.14159265f / static_cast<float>(total_slots))
                                      : 0.0f;

        spawn_one(boss_x, boss_y, shot_dx, shot_dy, shot_px, shot_py,
                  base_phase + wave_phase_offset);
    }

    m_wave_toggle = !m_wave_toggle;
}

BossProjectile* BossProjectileManager::acquire_slot()
{
    for (auto& p : m_pool)
        if (!p.in_use) return &p;

    m_pool.emplace_back();
    return &m_pool.back();
}

void BossProjectileManager::spawn_one(float bx, float by,
                                       float dir_x, float dir_y,
                                       float perp_x, float perp_y,
                                       float phase)
{
    BossProjectile* p = acquire_slot();
    *p = BossProjectile{};
    p->x      = bx;
    p->y      = by;
    p->vx     = dir_x * BossProjCfg::BASE_SPEED;
    p->vy     = dir_y * BossProjCfg::BASE_SPEED;
    p->perp_x = perp_x;
    p->perp_y = perp_y;
    p->phase  = phase;
    p->lifetime = BossProjCfg::MAX_LIFETIME;
    p->in_use = true;
}

int BossProjectileManager::get_active_count() const
{
    int n = 0;
    for (const auto& p : m_pool)
        if (p.in_use) ++n;
    return n;
}

//------------------------------- Pathfinding --------------------------------
void enemy_move_towards(f32& x, f32& y, f32 speed, f32 size, f32 target_x, f32 target_y, f32 dt)
{
    f32 dx = target_x - x;
    f32 dy = target_y - y;
    f32 dist = sqrtf(dx * dx + dy * dy);
    if (dist <= 1.0f)
        return;

    dx /= dist;
    dy /= dist;
    f32 nx = x + dx * speed * dt;
    f32 ny = y + dy * speed * dt;
    if (!check_player_collision(nx, y, size / 2.0f))
        x = nx;
    if (!check_player_collision(x, ny, size / 2.0f))
        y = ny;
}

void enemy_move_away_from(f32& x, f32& y, f32 speed, f32 size, f32 from_x, f32 from_y, f32 dt)
{
    f32 dx = x - from_x;
    f32 dy = y - from_y;
    f32 dist = sqrtf(dx * dx + dy * dy);
    if (dist <= 1.0f)
        return;

    dx /= dist;
    dy /= dist;
    f32 nx = x + dx * speed * dt;
    f32 ny = y + dy * speed * dt;
    if (!check_player_collision(nx, y, size / 2.0f))
        x = nx;
    if (!check_player_collision(x, ny, size / 2.0f))
        y = ny;
}

void enemy_flee_to_wall(f32 x, f32 y, f32 speed, f32 player_x, f32 player_y, f32 dt, f32& next_x, f32& next_y)
{
    f32 dx = x - player_x;
    f32 dy = y - player_y;
    f32 dist = sqrtf(dx * dx + dy * dy);
    if (dist < 1.0f)
    {
        dx = 1.0f;
        dy = 0.0f;
    }
    else
    {
        dx /= dist;
        dy /= dist;
    }

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
    texture(nullptr), food_texture(nullptr)
{
}

void base_enemy::take_damage(int amount)
{
    if (!active)
        return;

    current_health -= amount;

    if (current_health <= 0)
    {
        current_health = 0;
        state = enemy_state::dead;
        active = false;
    }
}

bool base_enemy::is_in_camera_view(camera* p_cam) const
{
    if (!p_cam)
        return false;

    extern f32 zoom;
    f32 hw = (AEGfxGetWindowWidth() / 2.0f) / zoom;
    f32 hh = (AEGfxGetWindowHeight() / 2.0f) / zoom;

    return (x > p_cam->pos.x - hw - size && x < p_cam->pos.x + hw + size &&
        y > p_cam->pos.y - hh - size && y < p_cam->pos.y + hh + size);
}

bool base_enemy::collides_with_player(Player* player) const
{
    if (!player || !player->current_player)
        return false;

    f32 dx = x - player->current_player->x;
    f32 dy = y - player->current_player->y;
    f32 dist = sqrtf(dx * dx + dy * dy);

    return dist < (collision_radius + player->stats.character_size / 2.0f);
}

f32 base_enemy::distance_to_player(Player* player) const
{
    if (!player || !player->current_player)
        return 10000.0f;

    f32 dx = x - player->current_player->x;
    f32 dy = y - player->current_player->y;
    return sqrtf(dx * dx + dy * dy);
}

// ============================================================================
// GHOST IMPLEMENTATION
// ============================================================================
ghost::ghost(f32 start_x, f32 start_y)
    : base_enemy(start_x, start_y), hits_from_player(0)
{
    speed = 80.0f;
    current_health = 2;
    max_health = 2;
    size = 50.0f;
    sight_range = 400.0f;
}
void ghost::take_damage(int amount)
{
    if (!active)
        return;
    hits_from_player++;
    base_enemy::take_damage(amount);
}

void ghost::update(f32 dt, Player* player, camera* p_cam)
{
    if (!active)
        return;

    visible_to_player = is_in_camera_view(p_cam);

    if (visible_to_player)
    {
        state = enemy_state::fleeing;
        f32 next_x{}, next_y{};

        enemy_flee_to_wall(x, y, speed, player->current_player->x, player->current_player->y,
            dt, next_x, next_y);

        // flee to wall resolution
        if (!check_player_collision(next_x, y, size / 2.0f))
        {
            x = next_x;
        }
        else
        {
            // Hit a wall while fleeing - ghost dies, active=false so cleanup works
            state = enemy_state::dead;
            active = false;
        }

        if (!check_player_collision(x, next_y, size / 2.0f))
        {
            y = next_y;
        }
        else
        {
            state = enemy_state::dead;
            active = false;
        }
    }
    else
    {
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
}

void bat::update(f32 dt, Player* player, camera* p_cam)
{
    if (!active)
        return;

    visible_to_player = is_in_camera_view(p_cam);

    if (visible_to_player)
    {
        state = enemy_state::chasing;
        enemy_move_towards(x, y, speed, size,
            player->current_player->x,
            player->current_player->y, dt);

        if (collides_with_player(player))
        {
            state = enemy_state::attacking;
            attack_timer += dt;
            if (attack_timer >= attack_cooldown) {
                player_take_damage(player, 3); // 1 unit = half a heart
                attack_timer = 0.0f;
            }
        }
    }
    else
    {
        state = enemy_state::idle;
    }
}

// ============================================================================
// DRAGON IMPLEMENTATION
// ============================================================================

dragon::dragon(f32 start_x, f32 start_y, f32 r_left, f32 r_right, f32 r_top, f32 r_bottom)
    : base_enemy(start_x, start_y), room_left(r_left), room_right(r_right),
    room_top(r_top), room_bottom(r_bottom),
    hits_received(0), is_boss(true)
{
    speed = 40.0f;
    current_health = 10;
    max_health = 10;
    size = 300.0f;
}

void dragon::load_anim_textures()
{
    tex_idle = AEGfxTextureLoad("Assets/Enemy/Boss/dragon_lord_idle_basic_74x74.png");
    tex_walk = AEGfxTextureLoad("Assets/Enemy/Boss/dragon_lord_walk_basic_74x74.png");
    tex_attack = AEGfxTextureLoad("Assets/Enemy/Boss/dragon_lord_attack_arms_90x70.png");
    tex_hurt = AEGfxTextureLoad("Assets/Enemy/Boss/dragon_lord_hurt_basic_130x130.png");
    tex_death = AEGfxTextureLoad("Assets/Enemy/Boss/dragon_lord_death_160x160.png");

    anim.set(tex_idle, 4, 74, 74, 296, 74, 6.f, true);
}

void dragon::tick_attack_pattern(float dt, float tgt_x, float tgt_y)
{
    // Simple repeating timer: fires one alternating wave every WAVE_INTERVAL seconds.
    m_wave_timer -= dt;
    if (m_wave_timer <= 1.f)
    {
        boss_proj_manager.trigger_wave(x, y, tgt_x, tgt_y);
        m_wave_timer = BossProjCfg::WAVE_INTERVAL;
    }
}

void dragon::update(f32 dt, Player* player, camera* p_cam)
{
    if (!active) return;

    // --- Dead: play death anim once then truly deactivate ---
    if (state == dead) {
        if (anim.update(dt))
            active = false;
        return;
    }

    visible_to_player = is_in_camera_view(p_cam);

    bool moving  = false;
    bool hitting = false;

    if (visible_to_player)
    {
        state = enemy_state::chasing;
        f32 target_x = AEClamp(player->current_player->x, room_left, room_right);
        f32 target_y = AEClamp(player->current_player->y, room_bottom, room_top);

        f32 dx = target_x - x, dy = target_y - y;
        moving = (sqrtf(dx * dx + dy * dy) > 4.f);

        enemy_move_towards(x, y, speed, size, target_x, target_y, dt);
        x = AEClamp(x, room_left, room_right);
        y = AEClamp(y, room_bottom, room_top);

        tick_attack_pattern(dt,
                            player->current_player->x,
                            player->current_player->y);

        if (collides_with_player(player))
        {
            state = enemy_state::attacking;
            hitting = true;
            attack_timer += dt;
            if (attack_timer >= 1.f) {
                player_take_damage(player, 1);
                attack_timer = 0.f;
            }
        }
        else attack_timer = 0.f;
    }
    else
    {
        state = enemy_state::idle;
    }

    // --- Animation state machine (unchanged) ---
    if (current_anim == DragonAnim::HURT) {
        if (anim.update(dt)) switch_anim(moving ? DragonAnim::WALK : DragonAnim::IDLE);
    }
    else if (current_anim == DragonAnim::ATTACK) {
        if (anim.update(dt)) switch_anim(moving ? DragonAnim::WALK : DragonAnim::IDLE);
    }
    else {
        anim.update(dt);
        if (hitting)     switch_anim(DragonAnim::ATTACK);
        else if (moving) switch_anim(DragonAnim::WALK);
        else             switch_anim(DragonAnim::IDLE);
    }
}

void dragon::switch_anim(DragonAnim next)
{   
    /*
	*  anim.set(texture, frame_count, frame_w, frame_h, sheet_w, sheet_h, fps, looping)
    */
    if (current_anim == next) return;
    current_anim = next;
    switch (next) {
    case DragonAnim::IDLE:
        anim.set(tex_idle, 4, 74, 74, 296, 74, 6.f, true);  break;
    case DragonAnim::WALK:
        anim.set(tex_walk, 8, 74, 74, 592, 74, 10.f, true);  break;
    case DragonAnim::ATTACK:
        anim.set(tex_attack, 16, 90, 70, 1440, 70, 14.f, false); break;
    case DragonAnim::HURT:
        anim.set(tex_hurt, 5, 130, 130, 650, 130, 10.f, false); break;
    case DragonAnim::DEATH:
        anim.set(tex_death, 20, 10, 50, 360, 50, 12.f, false); break;
    }
}

void dragon::render_animated(f32 draw_size)
{
    f32 w, h;
    switch (current_anim) {
    case DragonAnim::ATTACK:
        w = draw_size * (90.f / 70.f);
        h = draw_size;
        break;
    case DragonAnim::DEATH:
        w = draw_size * (160.f / 55.f);
        h = draw_size;
        break;
    default: // IDLE, WALK, HURT are all square
        w = draw_size;
        h = draw_size;
        break;
    }
    render_anim_sprite(x, y, w, h, anim);
}

void dragon::take_damage(int amount)
{
    if (!active)
        return;

    hits_received++;
    current_health -= amount;

    if (current_health <= 0)
    {
        current_health = 0;
        state = enemy_state::dead;
        switch_anim(DragonAnim::DEATH);
    }
    else switch_anim(DragonAnim::HURT);
}