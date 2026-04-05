/*!************************************************************************
\file:          enemy.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu

\brief:
    Core enemy system definitions.

    Includes:
    - Base enemy class (shared data + behavior)
    - Enemy types: ghost, bat, dragon (boss)
    - Boss projectile system
    - Movement helper utilities
    - Global config variables loaded from enemy_data.txt via dave_Data()

    Use this file as the main reference for:
    - Enemy tuning (speed, damage, detection, etc.)
    - Boss attack configuration
    - Extending new enemy types
**************************************************************************/

#pragma once

#include <cmath>
#include <vector>
#include <string>
#include <cstring>
#include "AEEngine.h"
#include "camera.hpp"


// ----------------------------------------------------------------------------
// ENEMY DATA  —  loaded from Assets/Config/enemy_data.txt by dave_Data()
// All values below are populated once at startup.
// enemy_manager.cpp (and any other file) can use them directly via extern.
// Do NOT reassign these anywhere except inside dave_Data().
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Texture filepaths — file paths for all enemy and UI sprite textures
// ----------------------------------------------------------------------------
extern std::string e_ghost_tex;         // Ghost enemy sprite
extern std::string e_bat_tex;           // Bat enemy sprite
extern std::string e_ghost_food_tex;    // Food drop sprite for ghost
extern std::string e_bat_food_tex;      // Food drop sprite for bat
extern std::string e_cross_tex;         // Cross / marker UI sprite
extern std::string e_ghost_marker_tex;  // Off-screen indicator for ghost
extern std::string e_bat_marker_tex;    // Off-screen indicator for bat
extern std::string e_dragon_tex_idle;   // Dragon idle animation sheet
extern std::string e_dragon_tex_walk;   // Dragon walk animation sheet
extern std::string e_dragon_tex_attack; // Dragon attack animation sheet
extern std::string e_dragon_tex_hurt;   // Dragon hurt animation sheet
extern std::string e_dragon_tex_death;  // Dragon death animation sheet

// ----------------------------------------------------------------------------
// Ghost stats — tuning values for the ghost enemy type
// ----------------------------------------------------------------------------
extern float e_ghost_speed;       // Movement speed (units/sec)
extern int   e_ghost_max_health;  // Maximum hit points
extern float e_ghost_size;        // Render / collision radius (world units)
extern float e_ghost_sight_range; // Detection radius; ghost aggros within this

// ----------------------------------------------------------------------------
// Bat stats — tuning values for the bat enemy type
// ----------------------------------------------------------------------------
extern float e_bat_speed;           // Movement speed (units/sec)
extern int   e_bat_max_health;      // Maximum hit points
extern float e_bat_size;            // Render / collision radius (world units)
extern float e_bat_sight_range;     // Detection radius; bat aggros within this
extern float e_bat_attack_cooldown; // Seconds between consecutive bat attacks
extern int   e_bat_attack_damage;   // Damage dealt per bat attack hit

// ----------------------------------------------------------------------------
// Dragon stats — tuning values for the dragon boss
// ----------------------------------------------------------------------------
extern float e_dragon_speed;           // Movement speed (units/sec)
extern int   e_dragon_max_health;      // Maximum hit points
extern float e_dragon_size;            // Render / collision radius (world units)
extern float e_dragon_attack_cooldown; // Seconds between consecutive boss attacks
extern int   e_dragon_attack_damage;   // Damage dealt per boss melee hit

// ----------------------------------------------------------------------------
// Boss projectile config — controls the wave-fire pattern of the dragon's shots
// ----------------------------------------------------------------------------
extern float e_bp_ring_radius;      // Visual ring radius drawn around projectile
extern float e_bp_ring_thickness;   // Thickness of the visual ring
extern float e_bp_base_speed;       // Base travel speed of each projectile
extern float e_bp_wave_amplitude;   // Side-to-side wave amplitude (world units)
extern float e_bp_wave_frequency;   // Oscillation frequency of the wave motion
extern float e_bp_max_lifetime;     // Seconds before a projectile auto-expires
extern int   e_bp_positions_wave_a; // Number of projectiles in wave pattern A
extern int   e_bp_positions_wave_b; // Number of projectiles in wave pattern B
extern float e_bp_angle_spread_deg; // Total arc (degrees) over which shots spread
extern float e_bp_wave_interval;    // Seconds between consecutive wave triggers
extern int   e_bp_projectile_damage;// Damage dealt to player on projectile hit
extern int   e_bp_pool_size;        // Max simultaneous projectiles in the pool

// ----------------------------------------------------------------------------
// Manager / world config — global values used by the enemy manager and map
// ----------------------------------------------------------------------------
extern float e_spawn_check_interval; // How often (sec) the manager checks spawns
extern float e_respawn_delay;        // Delay (sec) before a dead enemy respawns
extern float e_food_lifetime;        // Seconds a food drop persists before fading
extern float e_food_size;            // Render size of food drops (world units)
extern float e_map_start_x;         // World-space X origin of the tile map
extern float e_map_start_y;         // World-space Y origin of the tile map
extern float e_tile_size;           // Width/height of a single tile (world units)

// ----------------------------------------------------------------------------
// Respawn particles — controls the burst effect when an enemy reappears
// ----------------------------------------------------------------------------
extern int   e_rp_burst_count; // Number of particles spawned per respawn burst
extern float e_rp_speed_min;   // Minimum launch speed of a particle
extern float e_rp_speed_max;   // Maximum launch speed of a particle
extern float e_rp_life_min;    // Minimum particle lifetime (seconds)
extern float e_rp_life_max;    // Maximum particle lifetime (seconds)
extern float e_rp_size_min;    // Minimum particle render size
extern float e_rp_size_max;    // Maximum particle render size
extern float e_rp_gravity;     // Downward gravity applied to each particle
extern float e_rp_rise_boost;  // Initial upward impulse added on spawn

// ----------------------------------------------------------------------------
// save_Data()
// Reads enemy_data.txt and assigns every token to its matching global
// variable. Uses ifstream + getline + istringstream for tokenization.
// Call once at startup before any enemy or EnemyManager is constructed.
//
// \param filepath  Path to the config file
//                  e.g. "Assets/Config/enemy_data.txt"
// \return          true  — file opened and all values parsed successfully
//                  false — file could not be opened; globals stay at defaults
// ----------------------------------------------------------------------------
bool save_Data(const std::string& filepath);


// ============================================================================
// BossProjectile — single projectile fired by the dragon boss
// ============================================================================

// ----------------------------------------------------------------------------
// Represents one live boss projectile.
// Pooled inside BossProjectileManager; only slots where in_use == true
// are updated and rendered each frame.
// ----------------------------------------------------------------------------
struct BossProjectile
{
    float x{}, y{};           // Current world-space position
    float vx{}, vy{};         // Base velocity along the fire direction
    float perp_x{}, perp_y{}; // Perpendicular axis used for wave oscillation
    float phase{};            // Phase offset so each projectile waves differently
    float age{};              // Seconds since this projectile was spawned
    float lifetime{};         // Seconds until auto-expiry (copied from e_bp_max_lifetime)
    bool  in_use{ false };    // True while this slot is active in the pool
};

struct Player;

// ----------------------------------------------------------------------------
// BossProjectileManager
// Owns and drives a fixed-size pool of BossProjectile objects.
// Responsible for spawning wave bursts, ticking movement, and hit detection.
// ----------------------------------------------------------------------------
class BossProjectileManager
{
public:
    BossProjectileManager();

    // ------------------------------------------------------------------------
    // init()
    // Allocates the projectile pool. Call once after save_Data() returns.
    //
    // \param pool_size  Number of slots to reserve.
    //                   Pass 0 to use e_bp_pool_size from the data file.
    // ------------------------------------------------------------------------
    void init(int pool_size = 0);

    // ------------------------------------------------------------------------
    // clear()
    // Marks every pool slot as inactive. Safe to call mid-level on reset.
    // ------------------------------------------------------------------------
    void clear();

    // ------------------------------------------------------------------------
    // update()
    // Advances all active projectiles by dt seconds: applies base velocity,
    // adds sinusoidal wave motion, and culls expired or off-screen shots.
    //
    // \param dt   Delta time in seconds for this frame
    // \param cam  Active camera used for off-screen culling
    // ------------------------------------------------------------------------
    void update(float dt, const Camera::camera* cam);

    // ------------------------------------------------------------------------
    // check_player_collision()
    // Tests every active projectile against the player's hit box.
    // Deactivates any projectile that connects.
    //
    // \param player  Pointer to the player object to test against
    // \return        Total damage dealt this frame (hits * e_bp_projectile_damage)
    // ------------------------------------------------------------------------
    int  check_player_collision(Player* player);

    // ------------------------------------------------------------------------
    // trigger_wave()
    // Fires a new burst of projectiles aimed from the boss toward the target.
    // Alternates between wave pattern A and B on successive calls.
    //
    // \param boss_x    World-space X position of the dragon (fire origin)
    // \param boss_y    World-space Y position of the dragon (fire origin)
    // \param target_x  World-space X position to aim at (usually the player)
    // \param target_y  World-space Y position to aim at (usually the player)
    // ------------------------------------------------------------------------
    void trigger_wave(float boss_x, float boss_y,
        float target_x, float target_y);

    // ------------------------------------------------------------------------
    // get_projectiles()
    // Returns a const ref to the internal pool for rendering purposes.
    // Do not modify the pool through this reference.
    // ------------------------------------------------------------------------
    const std::vector<BossProjectile>& get_projectiles() const { return m_pool; }

    // ------------------------------------------------------------------------
    // get_active_count()
    // Returns the number of pool slots currently marked in_use.
    // Useful for debug overlays or performance monitoring.
    // ------------------------------------------------------------------------
    int  get_active_count() const;

private:
    // ------------------------------------------------------------------------
    // acquire_slot()
    // Finds the first inactive slot in the pool and returns a pointer to it.
    // Returns nullptr if the pool is full.
    // ------------------------------------------------------------------------
    BossProjectile* acquire_slot();

    // ------------------------------------------------------------------------
    // spawn_one()
    // Initialises a single projectile slot with the given direction/phase data.
    //
    // \param bx      Fire origin X
    // \param by      Fire origin Y
    // \param dir_x   Normalised X component of the fire direction
    // \param dir_y   Normalised Y component of the fire direction
    // \param perp_x  Normalised X component perpendicular to fire direction
    // \param perp_y  Normalised Y component perpendicular to fire direction
    // \param phase   Wave phase offset for this specific projectile
    // ------------------------------------------------------------------------
    void spawn_one(float bx, float by,
        float dir_x, float dir_y,
        float perp_x, float perp_y,
        float phase);

    std::vector<BossProjectile> m_pool;    // Fixed-capacity projectile pool
    bool m_wave_toggle{ false };           // Tracks which wave pattern fires next
};

// ----------------------------------------------------------------------------
// Global boss projectile manager — shared across enemy_manager and renderer
// ----------------------------------------------------------------------------
extern BossProjectileManager boss_proj_manager;


// ============================================================================
// Gameplay constants
// ============================================================================

// ----------------------------------------------------------------------------
// Damage dealt to the player (in HP) on direct enemy body contact
// ----------------------------------------------------------------------------
constexpr int player_heart_damage{ 50 };

// ----------------------------------------------------------------------------
// Fraction of the enemy's max HP that a food drop restores to the player
// ----------------------------------------------------------------------------
constexpr f32 food_health_percent{ 0.50f };

// ----------------------------------------------------------------------------
// Seconds a food pickup remains in the world before disappearing
// ----------------------------------------------------------------------------
constexpr f32 food_lifetime{ 10.f };


// ============================================================================
// Movement helpers
// ============================================================================

// ----------------------------------------------------------------------------
// enemy_move_towards()
// Moves an enemy one step toward a target position this frame.
// Applies collision-safe stepping using the enemy's size as a buffer.
//
// \param x        Enemy world-space X (modified in place)
// \param y        Enemy world-space Y (modified in place)
// \param speed    Movement speed in world units per second
// \param size     Collision radius used for wall sliding / separation
// \param target_x World-space X of the position to approach
// \param target_y World-space Y of the position to approach
// \param dt       Delta time in seconds
// ----------------------------------------------------------------------------
void enemy_move_towards(f32& x, f32& y, f32 speed, f32 size,
    f32 target_x, f32 target_y, f32 dt);

// ----------------------------------------------------------------------------
// enemy_move_away_from()
// Moves an enemy one step directly away from a source position this frame.
// Used when an enemy is in the fleeing state.
//
// \param x       Enemy world-space X (modified in place)
// \param y       Enemy world-space Y (modified in place)
// \param speed   Movement speed in world units per second
// \param size    Collision radius used for wall sliding / separation
// \param from_x  World-space X of the position to flee from (usually player)
// \param from_y  World-space Y of the position to flee from (usually player)
// \param dt      Delta time in seconds
// ----------------------------------------------------------------------------
void enemy_move_away_from(f32& x, f32& y, f32 speed, f32 size,
    f32 from_x, f32 from_y, f32 dt);

// ----------------------------------------------------------------------------
// enemy_flee_to_wall()
// Computes a flee destination toward the nearest map wall rather than
// directly away, giving fleeing enemies more natural wall-hugging behavior.
//
// \param x        Enemy current world-space X
// \param y        Enemy current world-space Y
// \param speed    Movement speed in world units per second
// \param player_x Player world-space X (used to pick opposite wall)
// \param player_y Player world-space Y (used to pick opposite wall)
// \param dt       Delta time in seconds
// \param next_x   Output: computed destination X for this frame
// \param next_y   Output: computed destination Y for this frame
// ----------------------------------------------------------------------------
void enemy_flee_to_wall(f32 x, f32 y, f32 speed,
    f32 player_x, f32 player_y, f32 dt,
    f32& next_x, f32& next_y);


// ============================================================================
// Enemy state machine
// ============================================================================

// ----------------------------------------------------------------------------
// enemy_state — the set of behavioral states shared by all enemy types.
//
//   idle      — no threat detected; enemy stands still or patrols
//   chasing   — player spotted; enemy moves toward the player
//   fleeing   — enemy is retreating away from the player
//   attacking — enemy is executing its attack action
//   dead      — enemy HP reached 0; pending food drop and respawn timer
// ----------------------------------------------------------------------------
enum enemy_state { idle, chasing, fleeing, attacking, dead };


// ============================================================================
// base_enemy — abstract base class shared by all enemy types
// ============================================================================

// ----------------------------------------------------------------------------
// base_enemy
// Holds the common data and non-type-specific logic for every enemy.
// Subclasses must implement update() to define their unique AI behavior.
// ----------------------------------------------------------------------------
class base_enemy
{
public:
    // ------------------------------------------------------------------------
    // Fallback stat constants used if the data file is missing entries
    // ------------------------------------------------------------------------
    static constexpr int enemy_max_hp{ 100 };         // Default maximum HP
    static constexpr int enemy_damage_per_hit{ 50 };  // Default damage per hit

protected:
    f32 x, y;                   // Current world-space position
    f32 spawn_x, spawn_y;       // Original spawn point used for respawning
    f32 velocity_x, velocity_y; // Current frame velocity (set by movement helpers)
    f32 speed;                  // Base movement speed (units/sec)
    f32 size;                   // Render and collision radius (world units)
    int current_health;         // Hit points remaining
    int max_health;             // Maximum hit points for this instance
    f32 attack_cooldown;        // Seconds required between attacks
    f32 attack_timer;           // Counts down to 0 before the next attack fires
    enemy_state state;          // Current behavioral state
    bool active;                // False when dead/despawned; skips update + render
    bool visible_to_player;     // True when inside the camera frustum
    f32 collision_radius;       // Radius used for player overlap tests
    f32 sight_range;            // Distance at which this enemy detects the player
    AEGfxTexture* texture;      // Primary sprite texture
    AEGfxTexture* food_texture; // Texture for the food drop this enemy leaves
    bool food_spawned{ false };              // True once the food drop has been placed
    f32 hit_flash_timer = 0.f;              // Counts down while the hurt flash is visible
    static constexpr f32 HIT_FLASH_DUR = 0.15f; // Duration (sec) of the hit flash effect

public:
    // ------------------------------------------------------------------------
    // base_enemy()
    // Sets up shared member defaults and records the spawn position.
    //
    // \param start_x  World-space X position where this enemy spawns
    // \param start_y  World-space Y position where this enemy spawns
    // ------------------------------------------------------------------------
    base_enemy(f32 start_x, f32 start_y);

    virtual ~base_enemy() {}

    // ------------------------------------------------------------------------
    // update() — pure virtual
    // Drives the enemy AI and animation for one frame.
    // Each subclass defines its own state transitions and movement logic.
    //
    // \param dt      Delta time in seconds
    // \param player  Pointer to the player object for targeting and collision
    // \param cam     Active camera for frustum/visibility checks
    // ------------------------------------------------------------------------
    virtual void update(f32 dt, struct Player* player, Camera::camera* cam) = 0;

    // ------------------------------------------------------------------------
    // take_damage()
    // Reduces current_health by amount and triggers the hit flash.
    // Sets state to dead if HP drops to 0 or below.
    //
    // \param amount  Positive integer amount of damage to apply
    // ------------------------------------------------------------------------
    virtual void take_damage(int amount);

    // ------------------------------------------------------------------------
    // is_ghost() — returns true only for ghost instances (used for type checks)
    // ------------------------------------------------------------------------
    virtual bool is_ghost() const { return false; }

    // ------------------------------------------------------------------------
    // get_max_health() — returns the max HP configured for this enemy instance
    // ------------------------------------------------------------------------
    virtual int  get_max_health() const { return max_health; }

    // ------------------------------------------------------------------------
    // is_in_camera_view()
    // Returns true if this enemy's position falls within the camera's viewport.
    //
    // \param cam  Active camera to test against
    // ------------------------------------------------------------------------
    bool is_in_camera_view(Camera::camera* cam) const;

    // ------------------------------------------------------------------------
    // collides_with_player()
    // Returns true if this enemy's collision radius overlaps the player's.
    //
    // \param player  Player to test overlap against
    // ------------------------------------------------------------------------
    bool collides_with_player(struct Player* player) const;

    // ------------------------------------------------------------------------
    // distance_to_player()
    // Returns the straight-line world-space distance to the player.
    //
    // \param player  Player to measure distance to
    // ------------------------------------------------------------------------
    f32  distance_to_player(struct Player* player) const;

    // ------------------------------------------------------------------------
    // Accessors — food drop state
    // ------------------------------------------------------------------------
    bool has_food_spawned() const { return food_spawned; }
    void set_food_spawned(bool val) { food_spawned = val; }

    // ------------------------------------------------------------------------
    // Accessors — read-only state queries
    // ------------------------------------------------------------------------
    f32           get_x()              const { return x; }
    f32           get_y()              const { return y; }
    f32           get_size()           const { return size; }
    int           get_current_health() const { return current_health; }
    enemy_state   get_state()          const { return state; }
    bool          is_active()          const { return active; }
    f32           get_hit_flash_timer()const { return hit_flash_timer; }
    AEGfxTexture* get_texture()        const { return texture; }
    AEGfxTexture* get_food_texture()   const { return food_texture; }

    // ------------------------------------------------------------------------
    // Accessors — setters used by the enemy manager and renderer
    // ------------------------------------------------------------------------
    void set_active(bool val) { active = val; }
    void set_position(f32 nx, f32 ny) { x = nx; y = ny; }
    void set_enemy_texture(AEGfxTexture* tex) { texture = tex; }
    void set_food_texture(AEGfxTexture* tex) { food_texture = tex; }
};


// ============================================================================
// Ghost — enemy subclass that flees when hit and returns to idle at range
// ============================================================================

// ----------------------------------------------------------------------------
// ghost
// A timid enemy that chases the player at close range, but flees after
// taking enough hits and eventually resets back toward its spawn point.
// ----------------------------------------------------------------------------
class ghost : public base_enemy
{
    int hits_from_player; // Running count of hits received; used to trigger flee
public:
    // ------------------------------------------------------------------------
    // ghost()
    // Initialises ghost-specific stats from the global config variables.
    //
    // \param start_x  World-space X spawn position
    // \param start_y  World-space Y spawn position
    // ------------------------------------------------------------------------
    ghost(f32 start_x, f32 start_y);

    ~ghost() override = default;

    // ------------------------------------------------------------------------
    // update()
    // Runs ghost AI: idle -> chasing -> fleeing state transitions,
    // movement toward / away from the player, and hit flash decay.
    //
    // \param dt      Delta time in seconds
    // \param player  Player reference for position and collision queries
    // \param cam     Camera reference for visibility culling
    // ------------------------------------------------------------------------
    void update(f32 dt, struct Player* player, Camera::camera* cam) override;

    // ------------------------------------------------------------------------
    // take_damage()
    // Increments hits_from_player in addition to the base HP reduction,
    // so the ghost can track when to switch into the fleeing state.
    //
    // \param amount  Positive damage amount to apply
    // ------------------------------------------------------------------------
    void take_damage(int amount) override;

    // ------------------------------------------------------------------------
    // is_ghost() — always true for ghost instances
    // ------------------------------------------------------------------------
    bool is_ghost() const override { return true; }

    // ------------------------------------------------------------------------
    // get_hits_from_player() — returns the accumulated hit count
    // ------------------------------------------------------------------------
    int  get_hits_from_player() const { return hits_from_player; }
};


// ============================================================================
// Bat — enemy subclass that dives at the player and retreats to attack again
// ============================================================================

// ----------------------------------------------------------------------------
// bat
// An aggressive enemy that actively chases the player and deals damage
// on contact. Uses a cooldown to pace repeated attacks.
// ----------------------------------------------------------------------------
class bat : public base_enemy
{
public:
    // ------------------------------------------------------------------------
    // bat()
    // Initialises bat-specific stats from the global config variables.
    //
    // \param start_x  World-space X spawn position
    // \param start_y  World-space Y spawn position
    // ------------------------------------------------------------------------
    bat(f32 start_x, f32 start_y);

    ~bat() override = default;

    // ------------------------------------------------------------------------
    // update()
    // Runs bat AI: chases the player when in sight range, applies attack
    // damage on contact respecting the cooldown timer.
    //
    // \param dt      Delta time in seconds
    // \param player  Player reference for position and collision queries
    // \param cam     Camera reference for visibility culling
    // ------------------------------------------------------------------------
    void update(f32 dt, struct Player* player, Camera::camera* cam) override;
};


// ============================================================================
// Animator — frame-based sprite sheet animation controller
// ============================================================================

// ----------------------------------------------------------------------------
// Animator
// Drives a single animation strip from a sprite sheet.
// Owns a cached AEGfxVertexList built for the current frame to avoid
// rebuilding the mesh every frame unless the frame index changes.
// ----------------------------------------------------------------------------
struct Animator {
    AEGfxTexture* sheet = nullptr; // Source sprite sheet texture
    AEGfxVertexList* cached_mesh = nullptr; // Cached mesh for the current frame
    int  last_built_frame = -1;             // Frame index the cached mesh was built for
    int  frame_count = 1;             // Total number of frames in the strip
    int  current_frame = 0;             // Index of the frame currently displayed
    float frame_w = 1.f, frame_h = 1.f; // Pixel dimensions of a single frame
    float sheet_w = 1.f, sheet_h = 1.f; // Pixel dimensions of the full sheet
    float fps = 8.f;                  // Playback speed in frames per second
    float timer = 0.f;                  // Accumulates dt; advances frame when >= 1/fps
    bool  looping = true;                 // If true, wraps back to frame 0 at the end
    bool  finished = false;                // Set true on the last frame of a non-looping clip

    // ------------------------------------------------------------------------
    // free_mesh()
    // Releases the cached vertex list and resets last_built_frame.
    // Call before switching sheets or on cleanup.
    // ------------------------------------------------------------------------
    void free_mesh()
    {
        if (cached_mesh) { AEGfxMeshFree(cached_mesh); cached_mesh = nullptr; }
        last_built_frame = -1;
    }

    // ------------------------------------------------------------------------
    // update()
    // Advances the animation timer by dt and increments current_frame when
    // the frame duration elapses.
    // For non-looping clips, clamps to the last frame and sets finished=true.
    //
    // \param dt    Delta time in seconds for this frame
    // \return      true on the exact frame a non-looping clip reaches its end
    // ------------------------------------------------------------------------
    bool update(float dt)
    {
        if (finished) return false;
        timer += dt;
        if (timer >= 1.f / fps) {
            timer -= 1.f / fps;
            current_frame++;
            if (current_frame >= frame_count) {
                if (looping) current_frame = 0;
                else { current_frame = frame_count - 1; finished = true; return true; }
            }
        }
        return false;
    }

    // ------------------------------------------------------------------------
    // set()
    // Configures the animator for a new clip and resets playback state.
    //
    // \param tex           Sprite sheet texture to sample from
    // \param frames        Total frame count in the strip
    // \param fw            Width of a single frame in pixels
    // \param fh            Height of a single frame in pixels
    // \param sw            Full sheet width in pixels
    // \param sh            Full sheet height in pixels
    // \param playback_fps  Desired playback rate in frames per second
    // \param loop          True to loop continuously; false to play once and stop
    // ------------------------------------------------------------------------
    void set(AEGfxTexture* tex, int frames,
        float fw, float fh, float sw, float sh,
        float playback_fps, bool loop)
    {
        sheet = tex; frame_count = frames;
        frame_w = fw; frame_h = fh; sheet_w = sw; sheet_h = sh;
        fps = playback_fps; looping = loop;
        current_frame = 0; timer = 0.f; finished = false;
    }

    // ------------------------------------------------------------------------
    // UV helpers — compute normalised texture coordinates for current_frame.
    // Use u0/u1 for horizontal extent and v0/v1 for vertical extent.
    // ------------------------------------------------------------------------
    float u0() const { return (current_frame * frame_w) / sheet_w; }
    float u1() const { return u0() + frame_w / sheet_w; }
    float v0() const { return 0.f; }
    float v1() const { return frame_h / sheet_h; }
};


// ============================================================================
// Dragon — boss enemy with phased attacks and animated sprite sheets
// ============================================================================

// ----------------------------------------------------------------------------
// dragon
// The main boss enemy. Roams within a defined room boundary, fires
// projectile waves via BossProjectileManager, and plays multi-clip
// sprite animations for each behavioral state.
// ----------------------------------------------------------------------------
class dragon : public base_enemy {
private:
    // ------------------------------------------------------------------------
    // Room boundary — dragon is confined to this AABB during the boss fight
    // ------------------------------------------------------------------------
    f32 room_left, room_right, room_top, room_bottom;

    s32 hits_received;      // Total hits taken; used to escalate attack patterns
    s32 hits_until_attack;  // Threshold; triggers a projectile wave when reached
    bool is_boss;           // True when this instance is the real boss (not a dummy)

    // ------------------------------------------------------------------------
    // Per-state animation sheets — loaded via load_anim_textures()
    // ------------------------------------------------------------------------
    AEGfxTexture* tex_idle = nullptr;
    AEGfxTexture* tex_walk = nullptr;
    AEGfxTexture* tex_attack = nullptr;
    AEGfxTexture* tex_hurt = nullptr;
    AEGfxTexture* tex_death = nullptr;

    Animator anim; // Active animator; reconfigured on every state transition

    // ------------------------------------------------------------------------
    // DragonAnim — internal enum mapping states to animation clips
    // ------------------------------------------------------------------------
    enum class DragonAnim { IDLE, WALK, ATTACK, HURT, DEATH };
    DragonAnim current_anim = DragonAnim::IDLE;

    float m_wave_timer{ 3.f }; // Countdown (sec) to the next projectile wave

    // ------------------------------------------------------------------------
    // switch_anim()
    // Transitions to a new animation clip if it differs from the current one.
    // Reconfigures the Animator and resets playback from frame 0.
    //
    // \param next_anim  The DragonAnim clip to switch to
    // ------------------------------------------------------------------------
    void switch_anim(DragonAnim next_anim);

    // ------------------------------------------------------------------------
    // tick_attack_pattern()
    // Decrements m_wave_timer and fires a projectile wave via
    // boss_proj_manager.trigger_wave() when the timer expires.
    //
    // \param dt       Delta time in seconds
    // \param tgt_x   Target world-space X to aim the wave at (player X)
    // \param tgt_y   Target world-space Y to aim the wave at (player Y)
    // ------------------------------------------------------------------------
    void tick_attack_pattern(float dt, float tgt_x, float tgt_y);

public:
    // ------------------------------------------------------------------------
    // dragon()
    // Initialises the boss with stats from global config and stores the
    // room boundary for movement clamping.
    //
    // \param start_x   World-space X spawn position
    // \param start_y   World-space Y spawn position
    // \param r_left    Left edge of the boss room (world units)
    // \param r_right   Right edge of the boss room (world units)
    // \param r_top     Top edge of the boss room (world units)
    // \param r_bottom  Bottom edge of the boss room (world units)
    // ------------------------------------------------------------------------
    dragon(f32 start_x, f32 start_y,
        f32 r_left, f32 r_right, f32 r_top, f32 r_bottom);

    ~dragon() override;

    // ------------------------------------------------------------------------
    // update()
    // Runs dragon AI: moves within the room, ticks the wave timer,
    // switches animation clips, and handles hit flash decay.
    //
    // \param dt      Delta time in seconds
    // \param player  Player reference for targeting and collision
    // \param cam     Camera reference for visibility culling
    // ------------------------------------------------------------------------
    void update(f32 dt, struct Player* player, Camera::camera* cam) override;

    // ------------------------------------------------------------------------
    // take_damage()
    // Applies damage, increments hits_received, triggers the hurt animation,
    // and checks whether hits_until_attack threshold has been crossed.
    //
    // \param amount  Positive damage amount to apply
    // ------------------------------------------------------------------------
    void take_damage(int amount) override;

    // ------------------------------------------------------------------------
    // has_dropped_sister()
    // Returns true when the boss is dead — used by the game manager to
    // trigger the sister rescue cutscene / unlock sequence.
    // ------------------------------------------------------------------------
    bool has_dropped_sister() const { return is_boss && state == dead; }

    // ------------------------------------------------------------------------
    // load_anim_textures()
    // Loads all five dragon sprite sheets from the paths in the global config
    // (e_dragon_tex_idle … e_dragon_tex_death) and stores them in tex_*.
    // Call once after save_Data() before the first update().
    // ------------------------------------------------------------------------
    void load_anim_textures();

    // ------------------------------------------------------------------------
    // render_animated()
    // Draws the dragon at its current world position using the active
    // Animator frame. Builds a new mesh only when current_frame changes.
    //
    // \param draw_size  Uniform scale applied to the sprite (world units)
    // ------------------------------------------------------------------------
    void render_animated(f32 draw_size);
};