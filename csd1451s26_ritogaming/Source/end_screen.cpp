#include "AEEngine.h"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"

static s8 end_font = -1;
bool is_win = false;
bool is_paused = false;

void end_screen_load()
{
    end_font = AEGfxCreateFont("Assets/Exo2-Regular.ttf", 40);
}

void end_screen_unload()
{
    if (end_font != -1)
    {
        AEGfxDestroyFont(end_font);
        end_font = -1;
    }
}

void end_screen_free() {}
void end_screen_init() {}

void end_screen_update()
{
    if (AEInputCheckTriggered(AEVK_R))
        next = GS_LEVEL1;
    if (AEInputCheckTriggered(AEVK_M))
        next = MAINMENU;
}

void pause_update()
{
    if (AEInputCheckTriggered(AEVK_ESCAPE))
        is_paused = !is_paused;

    if (is_paused)
    {
        if (AEInputCheckTriggered(AEVK_R))
        {
            next = GS_LEVEL1;
            is_paused = false;
        }

        if (AEInputCheckTriggered(AEVK_M))
        {
            next = MAINMENU;
            is_paused = false;
        }
    }
}

void end_screen_draw()
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    if (end_font != -1)
    {
        if (is_win)
            AEGfxPrint(end_font, "YOU WIN!", -0.3f, 0.1f, 2.0f, 0.0f, 1.0f, 0.0f, 1.0f);
        else
            AEGfxPrint(end_font, "YOU LOSE!", -0.3f, 0.1f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f);

        AEGfxPrint(end_font, "R - Restart", -0.3f, -0.1f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(end_font, "M - Main Menu", -0.3f, -0.3f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void pause_draw()
{
    if (!is_paused) return;
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    if (end_font != -1)
    {
        AEGfxPrint(end_font, "PAUSED", -0.2f, 0.2f, 2.0f, 1.0f, 1.0f, 0.0f, 1.0f);
        AEGfxPrint(end_font, "ESC - Resume", -0.3f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(end_font, "R   - Restart", -0.3f, -0.2f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(end_font, "M   - Main Menu", -0.3f, -0.4f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    }
}