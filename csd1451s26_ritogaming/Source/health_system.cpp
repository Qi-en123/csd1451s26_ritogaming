/*!************************************************************************
\file:          health_system.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         HP bar for player AND enemies
**************************************************************************/

#include "health_system.hpp"

void draw_health_bar(f32 x, f32 y, f32 width, f32 height,
    f32 current_health, f32 max_health,
    bool is_player) {

    if (max_health <= 0 || current_health <= 0) return;

    f32 health_percent = current_health / max_health;
    if (health_percent > 1.0f) health_percent = 1.0f;
    if (health_percent < 0.0f) health_percent = 0.0f;

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    // BACKGROUND (empty health)
    if (is_player) {
        AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 0.5f);
    }
    else {
        AEGfxSetColorToMultiply(0.5f, 0.0f, 0.0f, 0.5f);
    }
    draw_square(x, y, width, height);

    // FOREGROUND (current health)
    if (health_percent > 0.0f) {
        if (is_player) {
            AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 0.8f);  // Green for player
        }
        else {
            AEGfxSetColorToMultiply(1.0f, 0.5f, 0.0f, 0.8f);  // Orange for enemy
        }

        f32 health_width = width * health_percent;
        f32 health_x = x - (width - health_width) / 2.0f;
        draw_square(health_x, y, health_width, height);
    }

    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
}