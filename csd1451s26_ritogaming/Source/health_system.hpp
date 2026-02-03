/*!************************************************************************
\file:          health_system.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         HP bar drawing for player AND enemies
**************************************************************************/

#pragma once
#include "AEEngine.h"
#include "renderer.hpp"

// Draw health bar for ANY entity (player or enemy)
void draw_health_bar(f32 x, f32 y, f32 width, f32 height,
    f32 current_health, f32 max_health,
    bool is_player = true);

// Health math formulas for BOTH player and enemies
namespace health_math {
    // When ANYONE takes damage (player or enemy)
    inline f32 apply_damage(f32 current_hp, f32 damage) {
        f32 new_hp = current_hp - damage;
        if (new_hp < 0) new_hp = 0;
        return new_hp;
    }

    // When ANYONE heals (player or enemy)
    inline f32 apply_heal(f32 current_hp, f32 heal_amount, f32 max_hp) {
        f32 new_hp = current_hp + heal_amount;
        if (new_hp > max_hp) new_hp = max_hp;
        return new_hp;
    }

    // When player heals by percentage (from food)
    inline f32 apply_percentage_heal(f32 current_hp, f32 heal_percent, f32 max_hp) {
        f32 heal_amount = max_hp * heal_percent;
        return apply_heal(current_hp, heal_amount, max_hp);
    }
}