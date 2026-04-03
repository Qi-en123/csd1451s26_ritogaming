/*!************************************************************************
\file:						main.cpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu

\brief:						The entry point for "Ahmad's Dungeon".
                            Handles the core game loop, state transitions
                            via the Game State Manager (GSM), and memory
                            leak detection.
**************************************************************************/

#include <crtdbg.h>

#include "AEEngine.h"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "enemy_manager.hpp"
#include "audio_manager.hpp"

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
    // ------------------------------------------------------------------------
    // 1. Debugging & Memory Leak Detection Setup
    // ------------------------------------------------------------------------
    int oldFlag = _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    // _CrtSetBreakAlloc(2647); // Uncomment to break on a specific memory allocation ID

    // ------------------------------------------------------------------------
    // 2. Systems Initialization
    // ------------------------------------------------------------------------
    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, nullptr);
    AESysSetWindowTitle("Ahmad's Dungeon");

    // Initialize Global Systems
    gsm_initialize(MAINMENU);
    audio_init();

    // ------------------------------------------------------------------------
    // 3. Main Game Loop (GSM Pattern)
    // ------------------------------------------------------------------------
    while (current != GS_QUIT)
    {
        // Handle logic for restarting or switching states
        if (current != GS_RESTART)
            gsm_update();

        // Load and Initialize resources for the current state
        if (fpLoad)
            fpLoad();
        if (fpInitialize)
            fpInitialize();

        // The Inner Loop: Runs once per frame while in the same state
        while (next == current)
        {
            AESysFrameStart(); // Start of frame timing/input

            // Ensure window remains active
            if (!AESysDoesWindowExist())
                next = GS_QUIT;

            // State-specific Update and Draw calls
            if (fpUpdate)
                fpUpdate();
            if (fpDraw)
                fpDraw();

            // Update global audio system
            audio_update();

            AESysFrameEnd(); // End of frame buffers swap
        }

        // --------------------------------------------------------------------
        // 4. State Cleanup
        // --------------------------------------------------------------------
        if (fpFree)
            fpFree();   // Free resources used in the state loop
        if (fpUnload)
            fpUnload(); // Unload assets from memory

        // Manage State Transitions
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

    // Explicitly clean up global singleton/manager instances before exit
    enemy_manager_instance.~EnemyManager();

    // Disable automatic leak check at exit to avoid false positives with globals
    _CrtSetDbgFlag(oldFlag & ~_CRTDBG_LEAK_CHECK_DF);

    audio_free();
    AESysExit();

    return 0;
}