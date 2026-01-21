#include <crtdbg.h>
#include "AEEngine.h"
#include <cstdio>
#include "map.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "game_state_manager.hpp"
#include "game_state_list.hpp"

#define NUM_TILE_TYPES 7
#define TILE_ROWS 36
#define TILE_COLS 64



int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, nullptr);
    AESysSetWindowTitle("Ahmad's Dungeon");
    gsm_initialize(GS_LEVEL1);

    // Create player
   


    while (current != GS_QUIT)
    {
        if (current != GS_RESTART)
            gsm_update();

        if (fpLoad) fpLoad();
        if (fpInitialize) fpInitialize();

        while (next == current)
        {
            AESysFrameStart(); // start of frame

           
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