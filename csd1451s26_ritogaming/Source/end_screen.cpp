/*!************************************************************************
\file:                      end_screen.cpp
\author:                    Goh Qi En
\par DP email:              g.qien@digipen.edu
\co-author:					Claire
\par co-author DP email:	c.cheong@digipen.edu
\brief:                     Win / Lose full-screen end state.
                            GS_WIN / GS_LOSE are standalone states (not overlays).
                            Draws a background, the win/lose asset, two buttons and
                            a custom cursor using render_editor_ui - same system as
                            the main menu so hover highlight and coords all match.

                            Pause is a top-level button panel: dark overlay + three
                            centred buttons (Resume / Restart / Main Menu) drawn with
                            the same render_editor_ui / IsButtonHovered system.
**************************************************************************/

#include "AEEngine.h"
#include "renderer.hpp"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "audio_manager.hpp"
#include "utils.hpp"
#include "level1.hpp"

bool is_win = false;
bool is_paused = false;
int restart_level_state = GS_LEVEL1;

namespace
{
    AEGfxTexture* tex_win = nullptr;
    AEGfxTexture* tex_lose = nullptr;
    AEGfxTexture* tex_button = nullptr;
    AEGfxTexture* tex_cursor = nullptr;
    AEGfxTexture* buttons = nullptr;
    AEGfxTexture* pause_panel = nullptr;
    AEGfxTexture* c_panel = nullptr;

    static s8 end_font = -1;

    //Pause panel
    constexpr f32 PANEL_W = 700.f;
    constexpr f32 PANEL_H = 640.f;
    constexpr f32 PANEL_Y = 0.0f;

    //  End-screen button layout 
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

    //confirmation panel
    constexpr f32 c_panel_w = 800.0f;
    constexpr f32 c_panel_h = 400.0f;
    constexpr f32 c_btn_w = c_panel_w / 4.0f;
    constexpr f32 c_btn_h = c_panel_h / 5.0f;
    constexpr f32 c_btn_x = c_panel_w / 5.0f;
    constexpr f32 c_btn_y = -c_panel_h / 8.0f;

    //  Pause button layout (three stacked, centred) 
    constexpr f32 P_BTN_W = 280.f;
    constexpr f32 P_BTN_H = 90.f;
    constexpr f32 P_BTN_X = 0.f;
    constexpr f32 P_BTN_Y = 160.f;   // Paused
    constexpr f32 P_BTN_Y0 = 80.f;   // Resume
    constexpr f32 P_BTN_Y1 = -40.f;   // Restart
    constexpr f32 P_BTN_Y2 = -160.f;  // Main Menu

    s32 mouse_x = 0, mouse_y = 0;
    int last_hovered = -1;
    int pause_last_hovered = -1;
    static int confirm_last_hovered = -1;

    // confirm popup
    enum ConfirmTarget { CONFIRM_NONE, CONFIRM_RESTART, CONFIRM_MAINMENU };
    ConfirmTarget confirm_target = CONFIRM_NONE;
    ConfirmTarget pause_confirm_target = CONFIRM_NONE;

    void draw_confirmation() {
        draw_black_overlay(0.6f);
        render_editor_ui(c_panel,
            c_panel_w, c_panel_h,
            1.0f, 0.f, 0.f);

        render_editor_ui(buttons,
            c_btn_w, c_btn_h,
            1.0f, -c_btn_x, c_btn_y,
            IsButtonHovered(-c_btn_x, c_btn_y, c_btn_w, c_btn_h));

        render_editor_ui(buttons,
            c_btn_w, c_btn_h,
            1.0f, c_btn_x, c_btn_y, //pos
            IsButtonHovered(c_btn_x, c_btn_y, c_btn_w, c_btn_h));

        if (end_font != -1)
        {
            const char* msg = (pause_confirm_target == CONFIRM_RESTART)
                ? "Restart level?" : "Return to Main Menu?";

            render_text_center(msg, end_font,
                0.0f,
                (c_panel_h / 5.0f) / (AEGfxGetWindowHeight() / 2.0f),
                1.0f, black);

            render_text_center("Yes", end_font,
                -(c_btn_x / (AEGfxGetWindowWidth() / 2.0f)),
                c_btn_y / (AEGfxGetWindowHeight() / 2.0f),
                1.0f, black);

            render_text_center("No", end_font,
                (c_btn_x / (AEGfxGetWindowWidth() / 2.0f)),
                c_btn_y / (AEGfxGetWindowHeight() / 2.0f),
                1.0f, black);
        }
    }
}

// -------------------------------------------------------------------------
// Shared helper   centres a label on a button
// -------------------------------------------------------------------------
static void draw_button_label(const char* label, f32 btn_x, f32 btn_y,
    f32 scale = 1.f,
    f32 r = 0.f, f32 g = 0.f, f32 b = 0.f)
{
    if (end_font == -1) return;

    f32 tw = 0.f, th = 0.f;
    AEGfxGetPrintSize(end_font, label, scale, &tw, &th);
    // render_editor_ui is centred on world coords; AEGfxPrint uses -1..1 normalised
    f32 norm_x = (btn_x / (AEGfxGetWindowWidth() / 2.0f)) - (tw / 2.f);
    f32 norm_y = (btn_y - 15.f) / 450.f;
    AEGfxPrint(end_font, label, norm_x, norm_y, scale, 0.f, 0.f, 0.f, 1.f);
}

// -------------------------------------------------------------------------
// Load / Unload / Init / Free
// -------------------------------------------------------------------------
void end_screen_load()
{
    tex_win = AEGfxTextureLoad("Assets/Sprites/Win N Lose/win.png");
    tex_lose = AEGfxTextureLoad("Assets/Sprites/Win N Lose/lose.png");
    tex_button = AEGfxTextureLoad("Assets/Sprites/Admin/main_menu_button.png");
    tex_cursor = AEGfxTextureLoad("Assets/Sprites/Admin/cursorSword_silver.png");
    buttons = AEGfxTextureLoad("Assets/Sprites/Admin/main_menu_button5.png");
    pause_panel = AEGfxTextureLoad("Assets/Sprites/Admin/pause_panel.png");
    c_panel = AEGfxTextureLoad("Assets/Sprites/Admin/end_screen_panel.png");

    if (end_font == -1)
        end_font = AEGfxCreateFont("Assets/Fonts/Kenney Mini Square.ttf", 36);
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
    safe_unload(buttons);
    safe_unload(pause_panel);
    safe_unload(c_panel);
    if (end_font != -1)
    {
        AEGfxDestroyFont(end_font);
        end_font = -1;
    }
}

void end_screen_free()
{
    free_mesh();
}

void end_screen_init()
{
    init_mesh();
    last_hovered = -1;
    pause_last_hovered = -1;
    confirm_target = CONFIRM_NONE;
    pause_confirm_target = CONFIRM_NONE;
    AEInputShowCursor(false);
}

// -------------------------------------------------------------------------
// End-screen update
// -------------------------------------------------------------------------
void end_screen_update()
{
    audio_update();
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    // Hover SFX   fires once per newly-entered button
    int current_hovered = -1;
    if (IsButtonHovered(BTN_X, BTN_Y0, BTN_W, BTN_H)) current_hovered = 0;
    else if (IsButtonHovered(BTN_X, BTN_Y1, BTN_W, BTN_H)) current_hovered = 1;

    if (current_hovered != last_hovered && current_hovered != -1)
        play_sfx(g_audio.sfx_button_hover);
    last_hovered = current_hovered;

    if (confirm_target != CONFIRM_NONE)
    {
        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            if (IsButtonClicked(-100.f, -50.f, 200.f, 80.f)) // Yes
            {
                play_sfx(g_audio.sfx_button_click, 0.1f);
                if (confirm_target == CONFIRM_RESTART)
                {
                    selected_level_path = "Assets/Levels/default.txt";
                    selected_level_puzzle_path = "Assets/Levels/puzzle.txt";
                    next = restart_level_state;
                }
                else
                    next = MAINMENU;
                confirm_target = CONFIRM_NONE;
            }
            else if (IsButtonClicked(150.f, -50.f, 200.f, 80.f)) // No
            {
                confirm_target = CONFIRM_NONE;
            }
        }
        return; // block all other input while popup is open
    }

    if (IsButtonClicked(BTN_X, BTN_Y0, BTN_W, BTN_H) || AEInputCheckTriggered(AEVK_R))
        confirm_target = CONFIRM_RESTART;

    if (IsButtonClicked(BTN_X, BTN_Y1, BTN_W, BTN_H) || AEInputCheckTriggered(AEVK_M))
        confirm_target = CONFIRM_MAINMENU;
}

// -------------------------------------------------------------------------
// End-screen draw
// -------------------------------------------------------------------------
void end_screen_draw()
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

    // Win / lose flag asset
    AEGfxTexture* flag = is_win ? tex_win : tex_lose;
    render_editor_ui(flag, flag_w, flag_h, 1.f, 0.f, 0.f);

    // Centred win / lose text
    if (end_font != -1)
    {
        const char* msg = is_win ? "YOU WIN!" : "YOU LOSE!";
        f32 msg_offset_y = 0.44f;
        render_text_center(msg, end_font, 0.0f, msg_offset_y, 2.5f, { 1.0f, 0.8f, 0.f, 1.f });
    }

    // Buttons
    if (tex_button)
    {
        render_editor_ui(tex_button, BTN_W, BTN_H, 1.f, BTN_X, BTN_Y0,
            IsButtonHovered(BTN_X, BTN_Y0, BTN_W, BTN_H));
        draw_button_label("Restart", BTN_X, BTN_Y0, 0.9f);

        render_editor_ui(tex_button, BTN_W, BTN_H, 1.f, BTN_X, BTN_Y1,
            IsButtonHovered(BTN_X, BTN_Y1, BTN_W, BTN_H));
        draw_button_label("Main Menu", BTN_X, BTN_Y1, 0.9f);
    }

    if (confirm_target != CONFIRM_NONE)
    {
        draw_confirmation();
    }

    // Custom cursor
    if (tex_cursor)
    {
        f32 world_x = (f32)mouse_x - (AEGfxGetWindowWidth()/2.0f);
        f32 world_y = -(f32)mouse_y + (AEGfxGetWindowHeight()/2.0f);
        render_editor_ui(tex_cursor, CURSOR_SIZE, CURSOR_SIZE, 1.f, world_x, world_y);
    }
}

// -------------------------------------------------------------------------
// Pause update  (call every frame from your active game state's update)
// -------------------------------------------------------------------------
void pause_update()
{
    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        if (pause_confirm_target != CONFIRM_NONE)
            pause_confirm_target = CONFIRM_NONE; // ESC closes popup first
        else
        {
            is_paused = !is_paused;
            pause_last_hovered = -1;
            play_sfx(g_audio.sfx_button_click, 0.1f);
        }
    }

    if (!is_paused) return;

    // --- Confirm popup blocks everything ---
    if (pause_confirm_target != CONFIRM_NONE)
    {
        //--- Hover in confirmation---
        int cur_conf = -1;
        if (IsButtonHovered(-c_btn_x, c_btn_y, c_btn_w, c_btn_h)) cur_conf = 0;      // Yes
        else if (IsButtonHovered(c_btn_x, c_btn_y, c_btn_w, c_btn_h)) cur_conf = 1; // No

        if (cur_conf != confirm_last_hovered && cur_conf != -1)
        {
            play_sfx(g_audio.sfx_button_hover);
        }
        confirm_last_hovered = cur_conf;

        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            if (IsButtonClicked(-100.f, -50.f, 200.f, 80.f)) // Yes
            {
                play_sfx(g_audio.sfx_button_click, 0.1f);
                if (pause_confirm_target == CONFIRM_RESTART)
                {
                    next = GS_RESTART;
                }
                else
                    next = MAINMENU;

                is_paused = false;
                pause_confirm_target = CONFIRM_NONE;
            }
            else if (IsButtonClicked(150.f, -50.f, 200.f, 80.f)) // No
            {
                play_sfx(g_audio.sfx_button_click, 0.1f);
                pause_confirm_target = CONFIRM_NONE;
            }
            confirm_last_hovered = -1;
        }
        return;
    }

    // Hover SFX
    int cur = -1;
    if (IsButtonHovered(P_BTN_X, P_BTN_Y0, P_BTN_W, P_BTN_H)) cur = 0;
    else if (IsButtonHovered(P_BTN_X, P_BTN_Y1, P_BTN_W, P_BTN_H)) cur = 1;
    else if (IsButtonHovered(P_BTN_X, P_BTN_Y2, P_BTN_W, P_BTN_H)) cur = 2;
    if (cur != pause_last_hovered && cur != -1)
        play_sfx(g_audio.sfx_button_hover);
    pause_last_hovered = cur;

    // Resume
    if (IsButtonClicked(P_BTN_X, P_BTN_Y0, P_BTN_W, P_BTN_H))
    {
        play_sfx(g_audio.sfx_button_click, 0.1f);
        is_paused = false;
        pause_last_hovered = -1;
    }

    // Restart — show confirm instead
    if (IsButtonClicked(P_BTN_X, P_BTN_Y1, P_BTN_W, P_BTN_H) || AEInputCheckTriggered(AEVK_R))
    {
        play_sfx(g_audio.sfx_button_click, 0.1f);
        pause_confirm_target = CONFIRM_RESTART;
    }

    // Main Menu — show confirm instead
    if (IsButtonClicked(P_BTN_X, P_BTN_Y2, P_BTN_W, P_BTN_H) || AEInputCheckTriggered(AEVK_M))
    {
        play_sfx(g_audio.sfx_button_click, 0.1f);
        pause_confirm_target = CONFIRM_MAINMENU;
    }
}

// -------------------------------------------------------------------------
// Pause draw  (call every frame AFTER your active game state's draw)
// -------------------------------------------------------------------------
void pause_draw()
{
    if (!is_paused) return;

    AEGfxSetColorToMultiply(0.1f, 0.05f, 0.0f, 0.92f);   // dark brown, nearly opaque
    render_editor_ui(pause_panel,
        PANEL_W, PANEL_H,
        1.f, P_BTN_X, PANEL_Y);

    //  3. Reset to textured mode before buttons / text 
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);

    //  4. "PAUSED" title   centred, yellow 
    if (end_font != -1)
    {
        draw_button_label("Paused", P_BTN_X, P_BTN_Y, 2.f, 0.f, 0.f, 0.f);
    }

    //  5. Buttons (drawn on top of panel) 
    if (buttons)
    {
        render_editor_ui(buttons, P_BTN_W, P_BTN_H, 1.f,
            P_BTN_X, P_BTN_Y0,
            IsButtonHovered(P_BTN_X, P_BTN_Y0, P_BTN_W, P_BTN_H));
        draw_button_label("Resume", P_BTN_X, P_BTN_Y0, 1.f, 0.f, 0.f, 0.f);

        render_editor_ui(buttons, P_BTN_W, P_BTN_H, 1.f,
            P_BTN_X, P_BTN_Y1,
            IsButtonHovered(P_BTN_X, P_BTN_Y1, P_BTN_W, P_BTN_H));
        draw_button_label("Restart", P_BTN_X, P_BTN_Y1, 1.f, 0.f, 0.f, 0.f);

        render_editor_ui(buttons, P_BTN_W, P_BTN_H, 1.f,
            P_BTN_X, P_BTN_Y2,
            IsButtonHovered(P_BTN_X, P_BTN_Y2, P_BTN_W, P_BTN_H));
        draw_button_label("Main Menu", P_BTN_X, P_BTN_Y2, 1.f, 0.f, 0.f, 0.f);
    }

    if (pause_confirm_target != CONFIRM_NONE)
    {
        draw_confirmation();
    }
}