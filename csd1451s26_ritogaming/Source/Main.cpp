#include <crtdbg.h>

#include "AEEngine.h"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "enemy_manager.hpp"
#include "audio_manager.hpp"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{

    int oldFlag = _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(2647);
    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, nullptr);
    AESysSetWindowTitle("Ahmad's Dungeon");
    // gsm_initialize(CREDIT);
    // gsm_initialize(GS_LEVEL1);
    gsm_initialize(MAINMENU);
    audio_init();

    // Create player

    // Starting position of camera
    // cam = init_cam(curr_player_x, curr_player_y, 60.0f, 0.0f);

    while (current != GS_QUIT)
    {
        if (current != GS_RESTART)
            gsm_update();

        if (fpLoad)
            fpLoad();
        if (fpInitialize)
            fpInitialize();

        while (next == current)
        {
            AESysFrameStart(); // start of frame

            if (!AESysDoesWindowExist())
                next = GS_QUIT;

            if (fpUpdate)
                fpUpdate();
            if (fpDraw)
                fpDraw();

            audio_update();

            AESysFrameEnd(); // end of frame
        }

        if (fpFree)
            fpFree();
        if (fpUnload)
            fpUnload();

        previous = current;
        current = next;
        if (current == GS_RESTART)
        {
            current = previous; // restart = reload same state
            next = previous;
        }
    }

    // Before exiting, manually clean up global objects in correct order
    enemy_manager_instance.~EnemyManager(); // Call destructor explicitly

    // Now disable the automatic leak check at exit
    _CrtSetDbgFlag(oldFlag & ~_CRTDBG_LEAK_CHECK_DF);

    audio_free();
    AESysExit();
    return 0;
}