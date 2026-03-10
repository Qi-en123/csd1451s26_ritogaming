#include "AEEngine.h"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"

static s8 end_font = -1;
bool is_win = false; // set this to true for win, false for lose

void end_screen_load()
{
    end_font = AEGfxCreateFont("Assets/Exo2-Regular.ttf", 40);
}

void end_screen_unload()
{
    if (end_font != -1)
        AEGfxDestroyFont(end_font);
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

void end_screen_draw()
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

    if (end_font != -1)
    {
        if (is_win)
        {
            // Green YOU WIN text
            AEGfxPrint(end_font, "YOU WIN!", -0.3f, 0.1f, 2.0f, 0.0f, 1.0f, 0.0f, 1.0f);
        }
        else
        {
            // Red YOU LOSE text
            AEGfxPrint(end_font, "YOU LOSE!", -0.3f, 0.1f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f);
        }

        AEGfxPrint(end_font, "R - Restart", -0.3f, -0.1f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(end_font, "M - Main Menu", -0.3f, -0.3f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    }
}