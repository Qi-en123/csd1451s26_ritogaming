/*!************************************************************************
\file:          enemy.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         This header declares the enemy structures and public
                functions for enemy initialization, updating, rendering,
                and cleanup using AEEngine.
**************************************************************************/

#pragma once

#include <string>
#include <vector>
#include "AEEngine.h"
#include "player.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "map.hpp"

namespace EnemySystem {

    // Enum inside namespace
    enum class state {
        IDLE,
        ACTIVE,
        AGGRO,
        FLEEING,
        ATTACKING,
        DYING,
        DEAD
    };

    // Configuration struct
    struct config {
        // Basic properties
        std::string name;
        int type_id = 0;

        // Visual
        std::vector<std::string> texture_paths;
        int default_texture_index = 0;
        float size = 64.0f;

        // Stats
        int max_health = 1;
        int damage = 10;
        float attack_range = 50.0f;
        float attack_cooldown = 1.0f;

        // Movement
        float move_speed = 50.0f;
        float aggro_range = 200.0f;
        float deaggro_range = 300.0f;

        // Spawn
        float min_spawn_distance = 100.0f;
        float max_spawn_distance = 500.0f;
        float view_threshold = 0.3f;

        // Drops
        std::string drop_texture = "Assets/Items/food.png";
        float drop_size = 32.0f;
        int drop_value = 20;
        float drop_chance = 0.5f;
    };

    // Base enemy class
    class enemy_base {
    protected:
        // Common properties
        config config_;
        state state_;
        float x, y;
        float vel_x, vel_y;
        int health_;
        float state_timer_;
        float attack_timer_;
        bool on_screen_;

        // Visual
        std::vector<AEGfxTexture*> textures_;
        float scale_;
        float animation_timer_;
        int current_frame_;

        // Collision
        float collision_radius;

    public:
        // Constructor/destructor
        enemy_base(const config& config, float x, float y);
        virtual ~enemy_base();

        // Getters
        float getX() const { return x; }
        float getY() const { return y; }
        state getState() const { return state_; }
        const config& getConfig() const { return config_; }

        // Core functions - virtual for polymorphism
        virtual void update(float dt, const player* player_ptr);
        virtual void render();
        virtual void onHit(int damage);
        virtual void onDeath();
        virtual bool checkCollision(float x, float y, float radius);

        // Helper functions
        bool isOnScreen() const;
        bool checkWallCollision(float x, float y, float radius) const;

    protected:
        // Helper functions for derived classes
        void loadTextures();
        void unloadTextures();

        // Internal update helpers
        virtual void updateMovement(float dt, const player* player_ptr);
        virtual void updateAnimation(float dt);
        virtual void updateStateMachine(float dt, const player* player_ptr);
    };

    // Helper functions in namespace
    bool isValidPosition(float x, float y, float min_distance);
    void createDrop(float x, float y, const config& config);

}
