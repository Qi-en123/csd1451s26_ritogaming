/*!************************************************************************
\file:						main.cpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu
\brief:						The entry point for "Ahmad's Dungeon".
                            Handles the core game loop, global screen
                            transitions via the Game State Manager (GSM),
                            and memory leak detection.
**************************************************************************/

#include <crtdbg.h>
#include "AEEngine.h"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "enemy_manager.hpp"
#include "audio_manager.hpp"
#include "renderer.hpp" 
#include "level1.hpp"

// ----------------------------------------------------------------------------
// Global Transition Variables
// ----------------------------------------------------------------------------
f32 g_transition_alpha = 0.0f;
s32 g_pending_next_state = -1;
enum TransitionState { T_NONE, T_FADE_OUT, T_FADE_IN };
TransitionState g_transition_state = T_NONE;
const f32 TRANSITION_SPEED = 1.0f;

// ----------------------------------------------------------------------------
//
// Function: wWinMain
// The Windows-specific entry point that initializes the engine, manages 
// the main loop, and ensures proper cleanup on exit.
//
// ---------------------------------------------------------------------------- 
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(hPrevInstance);
    // ------------------------------------------------------------------------
    // 1. Debugging & Memory Leak Detection Setup
    // ------------------------------------------------------------------------
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // ------------------------------------------------------------------------
    // 2. Systems Initialization
    // ------------------------------------------------------------------------
    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, nullptr);
    AESysSetWindowTitle("Ahmad's Dungeon");

    // Initialize Global Systems
    gsm_initialize(OPENING);
    audio_init();

    // ------------------------------------------------------------------------
    // 3. Main Game Loop (GSM Pattern)
    // ------------------------------------------------------------------------
    while (current != GS_QUIT)
    {
        // Handle logic for switching states
        if (current != GS_RESTART)
            gsm_update();

        // Load and Initialize resources for the current state
        if (fpLoad)
            fpLoad();
        if (fpInitialize)
            fpInitialize();

        // Reset transition to Fade In whenever a new state starts
        g_transition_state = T_FADE_IN;
        g_transition_alpha = 1.0f;

        // The Inner Loop: Runs once per frame while in the same state
        while (next == current)
        {
            AESysFrameStart();
            f32 dt = (float)AEFrameRateControllerGetFrameTime();

            if (!AESysDoesWindowExist())
                next = GS_QUIT;

            // ----------------------------------------------------------------
            // Global Transition Logic
            // ----------------------------------------------------------------
            if (g_transition_state == T_FADE_OUT)
            {
                g_transition_alpha += dt * TRANSITION_SPEED;
                if (g_transition_alpha >= 1.0f)
                {
                    g_transition_alpha = 1.0f;
                    next = g_pending_next_state; // Break inner loop to switch state
                }
            }
            else if (g_transition_state == T_FADE_IN)
            {
                g_transition_alpha -= dt * TRANSITION_SPEED;
                if (g_transition_alpha <= 0.0f)
                {
                    g_transition_alpha = 0.0f;
                    g_transition_state = T_NONE;
                }
            }

            // ----------------------------------------------------------------
            // State Updates & Drawing
            // ----------------------------------------------------------------
            if (fpUpdate)
                fpUpdate();

            if (fpDraw)
                fpDraw();

            // Always draw the transition overlay LAST so it covers UI/Menu/Game
            if (g_transition_alpha > 0.0f)
            {
                draw_black_overlay(g_transition_alpha);
            }

            audio_update();
            AESysFrameEnd();
        }

        // --------------------------------------------------------------------
        // 4. State Cleanup
        // --------------------------------------------------------------------
        if (fpFree)
            fpFree();
        if (fpUnload)
            fpUnload();

        previous = current;
        current = next;

        // Logic for restarting the current level
        if (current == GS_RESTART)
        {
            current = previous;
            next = previous;
        }
    }

    // ------------------------------------------------------------------------
    // 5. Final Shutdown & Cleanup
    // ------------------------------------------------------------------------

    audio_free();
    AESysExit();



    return 0;
}