#include <crtdbg.h>

#include "AEEngine.h"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, nullptr);
    AESysSetWindowTitle("Ahmad's Dungeon");
    //gsm_initialize(CREDIT);
    //gsm_initialize(GS_LEVEL1);
    gsm_initialize(GS_LEVEL_EDITOR);

    // Create player
   

	//Starting position of camera
	//cam = init_cam(curr_player_x, curr_player_y, 60.0f, 0.0f);

    while (current != GS_QUIT)
    {
        if (current != GS_RESTART)
            gsm_update();

        if (fpLoad) fpLoad();
        if (fpInitialize) fpInitialize();

        while (next == current)
        {
            AESysFrameStart(); // start of frame

            if (!AESysDoesWindowExist())
                next = GS_QUIT;

            if (fpUpdate) fpUpdate();
            if (fpDraw) fpDraw();

            AESysFrameEnd();   // end of frame
        }

        if (fpFree) fpFree();
        if (next != GS_RESTART && fpUnload)
            fpUnload();

        previous = current;
        current = next;
    }

 

    AESysExit();
    return 0;
}