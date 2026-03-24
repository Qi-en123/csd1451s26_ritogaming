/*!************************************************************************
\file:          end_screen.cpp
\author:        
\par DP email:  
\
\Co-author:     Claire (60%- namespace, draw_button_label, loading, drawing, 
\                            designing win &lose)
\
\brief:         Win / Lose full-screen end state.
                GS_WIN / GS_LOSE are standalone states (not overlays).
                Draws a background, the win/lose asset, two buttons and
                a custom cursor using render_editor_ui - same system as
                the main menu so hover highlight and coords all match.
**************************************************************************/

#include "AEEngine.h"
#include "renderer.hpp"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "audio_manager.hpp"
#include "utils.hpp"

//static s8 end_font = -1;
bool is_win = false;
bool is_paused = false;
int restart_level_state = GS_LEVEL1;

namespace
{
    AEGfxTexture* tex_win = nullptr;
    AEGfxTexture* tex_lose = nullptr;
    AEGfxTexture* tex_button = nullptr;
    AEGfxTexture* tex_cursor = nullptr;

    static s8 end_font = -1;

    // Button layout - same coordinate system as render_editor_ui / IsButtonHovered
    constexpr f32 BTN_W = 280.f;
    constexpr f32 BTN_H = 90.f;
    constexpr f32 BTN_X = 0.f;
    constexpr f32 BTN_Y0 = 0.f;  
    constexpr f32 BTN_Y1 = -110.f;  

    constexpr f32 flag_w = 694.f;
    constexpr f32 flag_h = 900.f;
    constexpr f32 TEXT_SCALE = 1.0f;  
    constexpr f32 TEXT_Y = 0.15f;    
    constexpr f32 CURSOR_SIZE = 50.f;

    s32 mouse_x = 0, mouse_y = 0;
    int last_hovered = -1;
}

static void draw_button_label(const char* label, f32 btn_x, f32 btn_y)
{
    if (end_font == -1) return;
    f32 tw = 0.f, th = 0.f;
    AEGfxGetPrintSize(end_font, label, TEXT_SCALE * 0.9f, &tw, &th);
    // render_editor_ui is centred on world coords; AEGfxPrint uses -1..1 normalised
    f32 norm_x = (btn_x / 800.f) - (tw / 2.f);
    f32 norm_y = (btn_y - 15.f) / 450.f;
    AEGfxPrint(end_font, label, norm_x, norm_y, TEXT_SCALE * 0.9f, 0.f, 0.f, 0.f, 1.f);
}

void end_screen_load()
{
    tex_win = AEGfxTextureLoad("Assets/Win  N  Lose/win.png");
    tex_lose = AEGfxTextureLoad("Assets/Win  N  Lose/lose.png");
    tex_button = AEGfxTextureLoad("Assets/Admin/main_menu_button.png");
    tex_cursor = AEGfxTextureLoad("Assets/Admin/cursorSword_silver.png");
    
    if (end_font == -1)
        end_font = AEGfxCreateFont("Assets/Kenney Mini Square.ttf", 36);
}

void end_screen_unload()
{
    auto safe_unload = [](AEGfxTexture*& t)
        {
            if (t) { AEGfxTextureUnload(t); t = nullptr; }
        };
    safe_unload(tex_win);
    safe_unload(tex_lose);
    safe_unload(tex_button);
    safe_unload(tex_cursor);

    if (end_font != -1)
    {
        AEGfxDestroyFont(end_font);
        end_font = -1;
    }
}

void end_screen_free() {
	free_mesh();
}

void end_screen_init() {
    init_mesh();
    last_hovered = -1;
    AEInputShowCursor(false);
}

void end_screen_update()
{
    audio_update();
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    // Hover SFX - fires once per new button entered
    int current_hovered = -1;
    if (IsButtonHovered(BTN_X, BTN_Y0, BTN_W, BTN_H)) current_hovered = 0;
    else if (IsButtonHovered(BTN_X, BTN_Y1, BTN_W, BTN_H)) current_hovered = 1;

    if (current_hovered != last_hovered && current_hovered != -1)
        play_sfx(g_audio.sfx_button_hover, 1.f);
    last_hovered = current_hovered;

    // Restart
    if (IsButtonClicked(BTN_X, BTN_Y0, BTN_W, BTN_H) || AEInputCheckTriggered(AEVK_R))
    {
        play_sfx(g_audio.sfx_button_click, 1.f);
        //next = GS_LEVEL1;
        next = restart_level_state;
    }

    // Main Menu
    if (IsButtonClicked(BTN_X, BTN_Y1, BTN_W, BTN_H) || AEInputCheckTriggered(AEVK_M))
    {
        play_sfx(g_audio.sfx_button_click, 1.f);
        next = MAINMENU;
    }
}


void end_screen_draw()
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

    // Flag asset
    AEGfxTexture* flag = is_win ? tex_win : tex_lose;
    render_editor_ui(flag, flag_w, flag_h, 1.f, 0.f, 0.f);

    // Centered win/lose text
    if (end_font != -1)
    {
        const char* msg = is_win ? "YOU WIN!" : "YOU LOSE!";
        f32 tw = 0.f, th = 0.f;
        AEGfxGetPrintSize(end_font, msg, TEXT_SCALE * 2.5f, &tw, &th);
        AEGfxPrint(end_font, msg, -(tw / 2.f), 0.3f+th/2.0f, TEXT_SCALE * 2.5f, 1.0f, 0.8f, 0.f, 1.f);
       
    }

    // Buttons
    if (tex_button)
    {
        render_editor_ui(tex_button, BTN_W, BTN_H, 1.f, BTN_X, BTN_Y0,
            IsButtonHovered(BTN_X, BTN_Y0, BTN_W, BTN_H));
        draw_button_label("Restart [R]", BTN_X, BTN_Y0);

        render_editor_ui(tex_button, BTN_W, BTN_H, 1.f, BTN_X, BTN_Y1,
            IsButtonHovered(BTN_X, BTN_Y1, BTN_W, BTN_H));
        draw_button_label("Main Menu [M]", BTN_X, BTN_Y1);
    }

    // Cursor
    if (tex_cursor)
    {
        f32 world_x = (f32)mouse_x - 800.f;
        f32 world_y = -(f32)mouse_y + 450.f;
        render_editor_ui(tex_cursor, CURSOR_SIZE, CURSOR_SIZE, 1.f, world_x, world_y);
    }
}

void pause_update()
{
    if (AEInputCheckTriggered(AEVK_ESCAPE))
        is_paused = !is_paused;

    if (is_paused)
    {
        if (AEInputCheckTriggered(AEVK_R))
        {
            next = GS_RESTART;
            is_paused = false;
        }

        if (AEInputCheckTriggered(AEVK_M))
        {
            next = MAINMENU;
            is_paused = false;
        }
    }
}
void pause_draw()
{
    if (!is_paused) return;
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    if (end_font != -1)
    {
        AEGfxPrint(end_font, "PAUSED", -0.2f, 0.2f, TEXT_SCALE * 2.0f, 1.0f, 1.0f, 0.0f, 1.0f);
        AEGfxPrint(end_font, "ESC - Resume", -0.3f, 0.0f, TEXT_SCALE, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(end_font, "R   - Restart", -0.3f, -0.2f, TEXT_SCALE, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(end_font, "M   - Main Menu", -0.3f, -0.4f, TEXT_SCALE, 1.0f, 1.0f, 1.0f, 1.0f);
    }
}