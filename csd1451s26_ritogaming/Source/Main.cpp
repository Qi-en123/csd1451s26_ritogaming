#include <crtdbg.h>
#include "AEEngine.h"
#include <cstdio>
#include "map.hpp"

#define NUM_TILE_TYPES 131
#define TILE_ROWS 90
#define TILE_COLS 160

AEGfxTexture* TileTextures[NUM_TILE_TYPES] = { nullptr };
AEGfxVertexList* TileMesh = nullptr;


// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, nullptr);
    AESysSetWindowTitle("Square Tile Map");

    float screen_w = 1600.f;
    float screen_h = 900.f;

    // Determine tile size to fit both width and height

    float tile_size = 10.f;

    InitializeDungeonMap();

    // Create mesh
    CreateTileMesh(&TileMesh);

    // Load textures properly
    char filename[128];
    for (int i = 0; i < NUM_TILE_TYPES; ++i)
    {
        sprintf_s(filename, "Assets/kenney_tiny-dungeon/Tiles/tile_%04d.png", i);
        TileTextures[i] = AEGfxTextureLoad(filename);
        if (!TileTextures[i])
            printf("Failed to load: %s\n", filename);
    }

    int gGameRunning = 1;
    while (gGameRunning)
    {
        AESysFrameStart();

        DrawMap(tile_size, screen_w, screen_h);

        AESysFrameEnd();

        if (AEInputCheckTriggered(AEVK_ESCAPE) || !AESysDoesWindowExist())
            gGameRunning = 0;
    }

    // Cleanup
    for (int i = 0; i < NUM_TILE_TYPES; ++i)
        if (TileTextures[i]) AEGfxTextureUnload(TileTextures[i]);
    if (TileMesh) AEGfxMeshFree(TileMesh);

    AESysExit();
    return 0;
}