/*!************************************************************************
\file:          mainmenu.cpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         This file defines the 6 cycle functions that load,
                initialize, update, draw, free, and unload the main menu
                game state. Features an animated knight and ghoul,
                interactive buttons, dust particles, a ghoul trail effect,
                and a custom cursor, all rendered using AEEngine.
**************************************************************************/

#include "renderer.hpp"
#include "AEEngine.h"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "utils.hpp"
#include "audio_manager.hpp"
#include <fstream>
#include <sstream>
#include <string>

namespace
{
    // ----------------------------------------------------------------------------
    // Textures and Font
    // ----------------------------------------------------------------------------
    AEGfxTexture* ground = nullptr;             // ground tile used for dust particles
    AEGfxTexture* wall = nullptr;               // wall tile (unused in draw, kept for future use)
    AEGfxTexture* door = nullptr;               // door tile (unused in draw, kept for future use)
    AEGfxTexture* ghoul = nullptr;              // ghoul character texture
    AEGfxTexture* knight = nullptr;             // knight character texture
    AEGfxTexture* buttons[6] = { nullptr };     // button textures indexed 0-5
    AEGfxTexture* cursor = nullptr;             // custom sword cursor texture
    AEGfxTexture* banner = nullptr;             // title banner texture
    AEGfxTexture* confirmation_panel = nullptr; // exit confirmation popup panel
    AEGfxTexture* bg = nullptr;                 // main menu background texture
    static s8 font = -1;                        // font handle for button labels and title

    // ----------------------------------------------------------------------------
    // Character Animation Constants (loaded from Assets/Mainmenu/mainmenu.txt)
    // ----------------------------------------------------------------------------
    f32 cfg_knight_start_x = -700.0f;  // knight initial x position
    f32 cfg_ghoul_start_x = -310.0f;  // ghoul initial x position
    f32 cfg_ghoul_start_y = -40.0f;   // ghoul initial y position
    f32 cfg_ghoul_grow_rate = 100.0f;   // units per second the ghoul grows
    f32 cfg_character_full_scale = 150.0f; // max size for both characters
    f32 cfg_character_min_scale = 10.0f;  // starting size for ghoul
    f32 cfg_character_speed = 200.0f;   // movement speed for both characters

    // ----------------------------------------------------------------------------
    // Button Layout (loaded from Assets/Mainmenu/mainmenu.txt)
    // ----------------------------------------------------------------------------
    f32 cfg_button_w = 280.f;                              // button width
    f32 cfg_button_h = 100.f;                              // button height
    f32 cfg_button_x = 500.f;                              // button x (right side)
    f32 cfg_button_y[5] = { 250.f, 125.f, 0.f, -125.f, -250.f }; // button y positions

    // ----------------------------------------------------------------------------
    // Title and Button Labels (loaded from Assets/Mainmenu/mainmenu.txt)
    // ----------------------------------------------------------------------------
    std::string cfg_title = "Ahmad's Dungeon";             // game title on banner
    std::string cfg_button_labels[5] = { "Play", "Levels", "Guide", "Credits", "Quit" };

    // ----------------------------------------------------------------------------
    // Character Animation State
    // ----------------------------------------------------------------------------
    f32  knight_scale;       // current rendered size of the knight
    f32  knight_x;           // current x position of the knight
    f32  ghoul_scale;        // current rendered size of the ghoul
    f32  ghoul_x;            // current x position of the ghoul
    f32  ghoul_y;            // current y position of the ghoul (bounces via sine)
    f32  ghoul_bounce_timer; // elapsed time used to drive ghoul sine wave bounce
    bool knight_returning;   // true while knight is running back to start position
    f32  knight_bob_timer;   // elapsed time used to drive knight vertical bob

    // ----------------------------------------------------------------------------
    // Mouse Input
    // ----------------------------------------------------------------------------
    s32 mouse_x, mouse_y; // current mouse position in screen space
    constexpr f32 CURSOR_SIZE = 50.f;

    // ----------------------------------------------------------------------------
    // Dust Particle System
    // ----------------------------------------------------------------------------
    struct DustParticle
    {
        f32  x, y;      // world position
        f32  alpha;     // current opacity (fades to 0 over lifetime)
        f32  scale;     // rendered size
        f32  lifetime;  // remaining time before deactivation
        f32  vx, vy;    // velocity per second
        bool active;    // true if this slot is in use
    };
    const int MAX_DUST = 60;               // maximum simultaneous dust particles
    DustParticle dust_particles[MAX_DUST]; // fixed-size pool of dust particles

    // ----------------------------------------------------------------------------
    // Ghoul Trail Effect
    // ----------------------------------------------------------------------------
    struct GhoulTrail
    {
        f32  x, y;   // world position of this trail snapshot
        f32  scale;  // size of this trail snapshot
        f32  alpha;  // current opacity (fades out over time)
        bool active; // true if this slot is in use
    };
    const int MAX_TRAIL = 5;            // number of trail snapshots kept at once
    GhoulTrail ghoul_trail[MAX_TRAIL];  // ring buffer of recent ghoul positions
    f32 trail_spawn_timer = 0.f;        // time since the last trail snapshot was taken

    // ----------------------------------------------------------------------------
    // Exit Confirmation Popup
    // ----------------------------------------------------------------------------
    bool show_exit_confirm = false; // true while the quit confirmation is visible

    constexpr f32 c_panel_w = 800.0f;          // confirmation panel width
    constexpr f32 c_panel_h = 400.0f;          // confirmation panel height
    constexpr f32 c_btn_w = c_panel_w / 4.0f; // yes/no button width
    constexpr f32 c_btn_h = c_panel_h / 5.0f; // yes/no button height
    constexpr f32 c_btn_x = c_panel_w / 5.0f; // horizontal offset for yes/no buttons
    constexpr f32 c_btn_y = -c_panel_h / 8.0f; // vertical offset for yes/no buttons
}

// ----------------------------------------------------------------------------
//
// Function: load_mainmenu_config
// Reads character animation constants, button layout, title text, and
// button labels from Assets/Mainmenu/mainmenu.txt into cfg_ variables
//
// ----------------------------------------------------------------------------
static void load_mainmenu_config()
{
    std::ifstream file("Assets/Mainmenu/mainmenu.txt");
    if (!file.is_open())
    {
        printf("Warning: could not open Assets/Mainmenu/mainmenu.txt, using defaults\n");
        return;
    }

    std::string key;
    while (file >> key)
    {
        if (key == "knight_start_x")      file >> cfg_knight_start_x;
        else if (key == "ghoul_start_x")       file >> cfg_ghoul_start_x;
        else if (key == "ghoul_start_y")       file >> cfg_ghoul_start_y;
        else if (key == "ghoul_grow_rate")     file >> cfg_ghoul_grow_rate;
        else if (key == "character_full_scale") file >> cfg_character_full_scale;
        else if (key == "character_min_scale") file >> cfg_character_min_scale;
        else if (key == "character_speed")     file >> cfg_character_speed;
        else if (key == "button_w")            file >> cfg_button_w;
        else if (key == "button_h")            file >> cfg_button_h;
        else if (key == "button_x")            file >> cfg_button_x;
        else if (key == "button_y0")           file >> cfg_button_y[0];
        else if (key == "button_y1")           file >> cfg_button_y[1];
        else if (key == "button_y2")           file >> cfg_button_y[2];
        else if (key == "button_y3")           file >> cfg_button_y[3];
        else if (key == "button_y4")           file >> cfg_button_y[4];
        else if (key == "title")               std::getline(file, cfg_title);
        else if (key == "label0")              std::getline(file, cfg_button_labels[0]);
        else if (key == "label1")              std::getline(file, cfg_button_labels[1]);
        else if (key == "label2")              std::getline(file, cfg_button_labels[2]);
        else if (key == "label3")              std::getline(file, cfg_button_labels[3]);
        else if (key == "label4")              std::getline(file, cfg_button_labels[4]);
    }

    // Trim leading space from getline values (caused by space after key)
    for (int i = 0; i < 5; ++i)
    {
        if (!cfg_button_labels[i].empty() && cfg_button_labels[i][0] == ' ')
            cfg_button_labels[i] = cfg_button_labels[i].substr(1);
    }
    if (!cfg_title.empty() && cfg_title[0] == ' ')
        cfg_title = cfg_title.substr(1);

    file.close();
}

// ----------------------------------------------------------------------------
//
// Function: spawn_dust
// Activates the first available dust particle slot at the given position
// with randomized velocity, scale, and lifetime for a footstep puff effect
//
// ----------------------------------------------------------------------------
static void spawn_dust(f32 x, f32 y)
{
    for (int i = 0; i < MAX_DUST; ++i)
    {
        if (!dust_particles[i].active)
        {
            dust_particles[i].x = x + ((rand() % 40) - 20.f); // random horizontal scatter
            dust_particles[i].y = y;
            dust_particles[i].alpha = 1.0f;                          // start fully opaque
            dust_particles[i].scale = (f32)(rand() % 30 + 20);       // random size 20-50
            dust_particles[i].lifetime = 0.6f + (rand() % 10) * 0.05f; // random lifetime
            dust_particles[i].vx = -((rand() % 80) + 20.f);       // drift left
            dust_particles[i].vy = (f32)(rand() % 60 + 30);       // float upward 30-90
            dust_particles[i].active = true;
            break; // only spawn one particle per call
        }
    }
}

// ----------------------------------------------------------------------------
//
// Function: mainmenu_load
// Loads config from file, then loads all textures and the font required
// for the main menu screen
//
// ----------------------------------------------------------------------------
void mainmenu_load()
{
    // Read tweakable values from data file before anything else
    load_mainmenu_config();

    // Load character and environment textures
    ground = AEGfxTextureLoad("Assets/Sprites/Tiles/tile_0002.png");
    wall = AEGfxTextureLoad("Assets/Sprites/Tiles/tile_0001.png");
    door = AEGfxTextureLoad("Assets/Sprites/Tiles/tile_0007.png");
    ghoul = AEGfxTextureLoad("Assets/Sprites/Enemy/tile_0006.png");
    knight = AEGfxTextureLoad("Assets/Sprites/Tiles/tile_0004.png");

    // Load UI textures
    cursor = AEGfxTextureLoad("Assets/Sprites/Admin/cursorSword_silver.png");
    banner = AEGfxTextureLoad("Assets/Sprites/Admin/title_banner.png");
    confirmation_panel = AEGfxTextureLoad("Assets/Sprites/Admin/end_screen_panel.png");
    bg = AEGfxTextureLoad("Assets/Sprites/Admin/main_menu_bg.png");

    // Load button textures (index 0 unused, 1-5 map to the 5 menu buttons)
    buttons[0] = AEGfxTextureLoad("Assets/Sprites/Admin/main_menu_button.png");
    buttons[1] = AEGfxTextureLoad("Assets/Sprites/Admin/main_menu_button2.png");
    buttons[2] = AEGfxTextureLoad("Assets/Sprites/Admin/main_menu_button3.png");
    buttons[3] = AEGfxTextureLoad("Assets/Sprites/Admin/main_menu_button4.png");
    buttons[4] = AEGfxTextureLoad("Assets/Sprites/Admin/main_menu_button5.png");
    buttons[5] = AEGfxTextureLoad("Assets/Sprites/Admin/main_menu_button6.png");

    // Load font for button labels and title text
    font = AEGfxCreateFont("Assets/Fonts/Kenney Mini Square.ttf", 40);
    printf("Font value: %d\n", font);
    if (font == -1)
        return;
}

// ----------------------------------------------------------------------------
//
// Function: mainmenu_init
// Resets character positions using config values, clears particles and
// trail, and starts the main menu BGM
//
// ----------------------------------------------------------------------------
void mainmenu_init()
{
    init_mesh();

    // Reset ghoul to small size at config starting position
    ghoul_scale = cfg_character_min_scale;
    ghoul_x = cfg_ghoul_start_x;
    ghoul_y = cfg_ghoul_start_y;
    ghoul_bounce_timer = 0.0f;

    // Reset knight to full size at config starting position
    knight_scale = cfg_character_full_scale;
    knight_x = cfg_knight_start_x;
    knight_returning = false;
    knight_bob_timer = 0.f;

    AEInputShowCursor(false); // hide system cursor, custom one is drawn instead

    // Deactivate all dust particles and trail snapshots from previous visits
    for (int i = 0; i < MAX_DUST; ++i) dust_particles[i].active = false;
    for (int i = 0; i < MAX_TRAIL; ++i) ghoul_trail[i].active = false;
    trail_spawn_timer = 0.0f;

    if (g_bgm_current_track != &g_audio.bgm_op_n_mainmenu)
    {
        g_bgm_current_track = &g_audio.bgm_op_n_mainmenu;
        play_bgm(g_audio.bgm_op_n_mainmenu, 1.0f);
    }

    printf("Supposed to ply udio");
}

// ----------------------------------------------------------------------------
//
// Function: mainmenu_update
// Handles button input, exit confirmation popup, character animation,
// dust particle spawning and update, and ghoul trail management each frame
//
// ----------------------------------------------------------------------------
void mainmenu_update()
{
    audio_update();
    dt = (f32)AEFrameRateControllerGetFrameTime();
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    // --- Button hover sound (fires once per newly-entered button) ---
    static int last_hovered = -1;
    int current_hovered = -1;
    for (int i = 0; i < 5; ++i)
    {
        if (IsButtonHovered(cfg_button_x, cfg_button_y[i], cfg_button_w, cfg_button_h))
        {
            current_hovered = i;
            break;
        }
    }
    if (current_hovered != last_hovered && current_hovered != -1)
        play_sfx(g_audio.sfx_button_hover);
    last_hovered = current_hovered;

    // --- Exit confirmation popup blocks all other input while visible ---
    if (show_exit_confirm)
    {
        if (IsButtonClicked(-100.f, -50.f, 200.f, 80.f)) // Yes — quit
            next = GS_QUIT;
        if (IsButtonClicked(100.f, -50.f, 200.f, 80.f))  // No — close popup
            show_exit_confirm = false;
        return;
    }

    // --- Main button clicks ---
    if (IsButtonClicked(cfg_button_x, cfg_button_y[0], cfg_button_w, cfg_button_h)) // Play
    {
        play_sfx(g_audio.sfx_button_click, 0.1f);
        selected_level_path = "Assets/Levels/default.txt";
        selected_level_puzzle_path = "Assets/Levels/puzzle.txt";
        next = GS_LEVEL1;
    }
    if (IsButtonClicked(cfg_button_x, cfg_button_y[1], cfg_button_w, cfg_button_h)) // Levels
    {
        play_sfx(g_audio.sfx_button_click, 0.1f);
        next = GS_LEVELS;
    }
    if (IsButtonClicked(cfg_button_x, cfg_button_y[2], cfg_button_w, cfg_button_h)) // Guide
    {
        play_sfx(g_audio.sfx_button_click, 0.1f);
        next = GS_GUIDE;
    }
    if (IsButtonClicked(cfg_button_x, cfg_button_y[3], cfg_button_w, cfg_button_h)) // Credits
    {
        play_sfx(g_audio.sfx_button_click, 0.1f);
        next = CREDIT;
    }
    if (IsButtonClicked(cfg_button_x, cfg_button_y[4], cfg_button_w, cfg_button_h)) // Quit
    {
        play_sfx(g_audio.sfx_button_click, 0.1f);
        show_exit_confirm = true; // show confirmation before quitting
    }

    // --- Knight return animation (runs back to start after exiting screen) ---
    if (knight_returning)
    {
        knight_x += cfg_character_speed * dt; // move knight rightward back to start

        if (knight_x >= cfg_knight_start_x) // reached start position — reset everything
        {
            knight_x = cfg_knight_start_x;
            knight_returning = false;
            ghoul_scale = cfg_character_min_scale; // ghoul resets to small
            ghoul_x = cfg_ghoul_start_x;
        }
        return; // skip all other animation while returning
    }

    // --- Ghoul grows to full size before the chase begins ---
    if (ghoul_scale < cfg_character_full_scale)
    {
        ghoul_scale += cfg_ghoul_grow_rate * dt; // grow at constant rate
        return; // wait until fully grown before starting movement
    }

    // --- Chase animation: ghoul flees, knight pursues ---
    ghoul_x += cfg_character_speed * dt;
    knight_x += cfg_character_speed * dt;
    ghoul_bounce_timer += dt;
    ghoul_y = -50.f + sinf(ghoul_bounce_timer * 5.f) * 20.f; // sine wave bounce

    // When knight exits the right edge, re-enter from the left
    if (knight_x > AEGfxGetWinMaxX() + 100.0f)
    {
        knight_returning = true;
        knight_x = AEGfxGetWinMinX() - 100.0f;
    }

    // --- Dust particles spawn under knight feet while chasing ---
    static f32 dust_timer = 0.f;
    dust_timer += dt;
    if (!knight_returning && ghoul_scale >= cfg_character_full_scale && dust_timer > 0.1f)
    {
        spawn_dust(knight_x - 40.f, -150.f); // left foot position
        spawn_dust(knight_x - 60.f, -150.f); // right foot position
        dust_timer = 0.f;
    }

    // --- Update active dust particles (move, fade, deactivate) ---
    for (int i = 0; i < MAX_DUST; ++i)
    {
        if (!dust_particles[i].active) continue;

        dust_particles[i].lifetime -= dt;
        dust_particles[i].x += dust_particles[i].vx * dt;
        dust_particles[i].y += dust_particles[i].vy * dt;
        dust_particles[i].alpha -= dt * 1.2f; // fade out over time

        if (dust_particles[i].lifetime <= 0.f || dust_particles[i].alpha <= 0.f)
            dust_particles[i].active = false;
    }

    // --- Ghoul trail: snapshot ghoul position every 0.08s ---
    trail_spawn_timer += dt;
    if (ghoul_scale >= cfg_character_full_scale && trail_spawn_timer > 0.08f)
    {
        // Shift existing trail entries back to make room for the newest
        for (int i = MAX_TRAIL - 1; i > 0; --i)
            ghoul_trail[i] = ghoul_trail[i - 1];

        // Store current ghoul state as the newest trail entry
        ghoul_trail[0].x = ghoul_x;
        ghoul_trail[0].y = ghoul_y;
        ghoul_trail[0].scale = ghoul_scale;
        ghoul_trail[0].alpha = 0.35f; // trail starts semi-transparent
        ghoul_trail[0].active = true;
        trail_spawn_timer = 0.f;
    }

    // --- Fade out existing trail entries ---
    for (int i = 0; i < MAX_TRAIL; ++i)
    {
        if (!ghoul_trail[i].active) continue;
        ghoul_trail[i].alpha -= dt * 0.8f;
        if (ghoul_trail[i].alpha <= 0.f)
            ghoul_trail[i].active = false;
    }

    // Advance knight bob timer only while the chase is active
    if (!knight_returning && ghoul_scale >= cfg_character_full_scale)
        knight_bob_timer += dt;
    else
        knight_bob_timer = 0.f;
}

// ----------------------------------------------------------------------------
//
// Function: mainmenu_draw
// Renders the background, dust particles, knight, ghoul trail, ghoul,
// menu buttons, title banner, exit confirmation popup, and custom cursor
//
// ----------------------------------------------------------------------------
void mainmenu_draw()
{
    // Draw full-screen background image
    render_editor_ui(bg, AEGfxGetWindowWidth(), AEGfxGetWindowHeight(), 1, 0, 0);

    // --- Draw dust particles using the ground tile as a puff sprite ---
    for (int i = 0; i < MAX_DUST; ++i)
    {
        if (!dust_particles[i].active) continue;
        if (ground)
            render_editor_ui(ground,
                dust_particles[i].scale,
                dust_particles[i].scale,
                dust_particles[i].alpha,
                dust_particles[i].x,
                dust_particles[i].y);
    }

    // --- Draw knight with vertical bob, forward lean, and squash-and-stretch ---
    if (knight)
    {
        f32 knight_y = -50.f + sinf(knight_bob_timer * 10.f) * 8.f;  // vertical bob
        f32 lean_x = knight_scale * 0.92f;                          // lean forward by squishing x
        f32 squash = 1.f + sinf(knight_bob_timer * 10.f) * 0.05f;  // squash on landing
        f32 draw_w = lean_x * squash;
        f32 draw_h = knight_scale / squash;
        render_editor_ui(knight, draw_w, draw_h, 1.0f, knight_x, knight_y);
    }

    // --- Draw ghoul trail behind the ghoul (oldest entry first) ---
    for (int i = MAX_TRAIL - 1; i >= 0; --i)
    {
        if (!ghoul_trail[i].active) continue;
        if (ghoul)
            render_editor_ui(ghoul,
                ghoul_trail[i].scale,
                ghoul_trail[i].scale,
                ghoul_trail[i].alpha,
                ghoul_trail[i].x,
                ghoul_trail[i].y);
    }

    // --- Draw ghoul with alpha based on its current scale ---
    if (ghoul)
    {
        f32 ghoul_alpha = ghoul_scale / 200.f; // fade in as ghoul grows
        render_editor_ui(ghoul, ghoul_scale, ghoul_scale, ghoul_alpha, ghoul_x, ghoul_y);
    }

    // --- Draw menu buttons and centered text labels ---
    for (int i = 0; i < 5; ++i)
    {
        // 1. Draw the Button Sprite (Keep your existing code)
        if (buttons[i + 1])
            render_editor_ui(buttons[i + 1], cfg_button_w, cfg_button_h, 1.0f,
                cfg_button_x, cfg_button_y[i],
                IsButtonHovered(cfg_button_x, cfg_button_y[i], cfg_button_w, cfg_button_h));

        // 2. Draw the Text Label
        if (font != -1)
        {
            f32 text_width = 0.f, text_height = 0.f;
            AEGfxGetPrintSize(font, cfg_button_labels[i].c_str(), 1.0f, &text_width, &text_height);

            // Convert World X/Y to NDC [-1, 1]
            // Formula: (WorldPos / (WindowSize / 2))
            f32 ndc_x = (cfg_button_x / (AEGfxGetWindowWidth() / 2.0f));
            f32 ndc_y = (cfg_button_y[i] / (AEGfxGetWindowHeight() / 2.0f));

            // Center the text: Subtract half of the normalized text width
            // We also apply a small Y offset (-0.02f) to visually center it vertically on the sign
            AEGfxPrint(font, cfg_button_labels[i].c_str(),
                ndc_x - (text_width / 2.0f),
                ndc_y - 0.02f,
                1.0f, 0.f, 0.f, 0.f, 1.0f);
        }
    }

    // --- Draw title banner and game title text ---
    if (banner)
        render_editor_ui(banner, 840.f, 260.f, 1.0f, cfg_ghoul_start_x, 200.f);

    if (font != -1)
    {
        f32 text_width = 0.f, text_height = 0.f;
        AEGfxGetPrintSize(font, cfg_title.c_str(), 1.f, &text_width, &text_height);
        f32 norm_x = (cfg_ghoul_start_x / 800.f) - (text_width / 2.f); // center on banner
        f32 norm_y = (200.f + 300.0f / 8.0f) / 450.f;
        AEGfxPrint(font, cfg_title.c_str(), norm_x, norm_y, 1.f, 0.f, 0.f, 0.f, 1.f);
    }

    // --- Draw exit confirmation popup when active ---
    if (show_exit_confirm)
    {
        draw_black_overlay(0.6f); // darken the background

        // Popup panel
        render_editor_ui(confirmation_panel, c_panel_w, c_panel_h, 1.0f, 0.f, 0.f);

        // Yes button (left side)
        render_editor_ui(buttons[2],
            c_btn_w, c_btn_h,
            1.0f, -c_btn_x, c_btn_y,
            IsButtonHovered(-c_btn_x, c_btn_y, c_btn_w, c_btn_h));

        // No button (right side)
        render_editor_ui(buttons[3],
            c_btn_w, c_btn_h,
            1.0f, c_btn_x, c_btn_y, //pos
            IsButtonHovered(c_btn_x, c_btn_y, c_btn_w, c_btn_h));

        if (font != -1)
        {
            // Confirmation message centered on panel
            render_text_center("Are you sure you want to quit?", font,
                0.0f,
                (c_panel_h / 5.0f) / (AEGfxGetWindowHeight() / 2.0f),
                1.0f, black);

            // Yes/No labels centered on their respective buttons
            render_text_center("Yes", font,
                -(c_btn_x / (AEGfxGetWindowWidth() / 2.0f)),
                c_btn_y / (AEGfxGetWindowHeight() / 2.0f),
                1.0f, black);

            render_text_center("No", font,
                (c_btn_x / (AEGfxGetWindowWidth() / 2.0f)),
                c_btn_y / (AEGfxGetWindowHeight() / 2.0f),
                1.0f, black);
        }
    }

    // --- Draw custom cursor at current mouse world position ---
    if (cursor)
    {
        // Convert mouse screen coords to world coords
        f32 world_x = (f32)mouse_x - AEGfxGetWindowWidth()/2.0f; // center origin
        f32 world_y = -(f32)mouse_y + AEGfxGetWindowHeight()/2.0f; // flip Y axis

        render_editor_ui(cursor, CURSOR_SIZE, CURSOR_SIZE, 1.0f, world_x, world_y);
    }
}

// ----------------------------------------------------------------------------
//
// Function: mainmenu_free
// Frees runtime mesh resources and stops the background music
//
// ----------------------------------------------------------------------------
void mainmenu_free()
{
    free_mesh(); // release vertex list memory created in init_mesh
    stop_bgm();
}

// ----------------------------------------------------------------------------
//
// Function: mainmenu_unload
// Unloads all textures and destroys the font when leaving the main menu
//
// ----------------------------------------------------------------------------
void mainmenu_unload()
{
    // Unload environment and character textures
    if (ground) { AEGfxTextureUnload(ground);  ground = nullptr; }
    if (wall) { AEGfxTextureUnload(wall);    wall = nullptr; }
    if (door) { AEGfxTextureUnload(door);    door = nullptr; }
    if (knight) { AEGfxTextureUnload(knight);  knight = nullptr; }
    if (ghoul) { AEGfxTextureUnload(ghoul);   ghoul = nullptr; }

    // Unload all button textures
    for (AEGfxTexture* btn : buttons)
        if (btn) AEGfxTextureUnload(btn);

    // Unload UI textures
    if (cursor) { AEGfxTextureUnload(cursor);             cursor = nullptr; }
    if (banner) { AEGfxTextureUnload(banner);             banner = nullptr; }
    if (confirmation_panel) { AEGfxTextureUnload(confirmation_panel); confirmation_panel = nullptr; }
    if (bg) { AEGfxTextureUnload(bg);                 bg = nullptr; }

    // Destroy font handle
    if (font != -1) { AEGfxDestroyFont(font); font = -1; }
}