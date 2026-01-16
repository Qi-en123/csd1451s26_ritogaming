#include <crtdbg.h>
#include "AEEngine.h"
#include "map.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "player.hpp"

// #include "puzzle.hpp"

#define NUM_TILE_TYPES 131
#define TILE_ROWS 90
#define TILE_COLS 160

Camera cam{};
f32 zoom{1.0f};
Player player;

AEGfxTexture *TileTextures[NUM_TILE_TYPES] = {nullptr};
AEGfxVertexList *TileMesh = nullptr;
float tile_size = 64.0f;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, nullptr);
    AESysSetWindowTitle("Ahmad's Dungeon");

    // Determine tile size to fit both width and height
    // float tile_size = 64.0f;

    InitializeDungeonMap();

    // Create mesh
    // CreateTileMesh(&TileMesh);
    initMesh();

    ////js to test camera (UPDATE CURR_PLAYER TO CHANGE CAMERA VIEW)
    // f32 curr_player_x{}, curr_player_y{};

    ////Starting position of camera
    // Camera cam = initCam(&curr_player_x, &curr_player_y, 60.0f, 5.0f);

    // Load textures properly
    char filename[128];
    for (int i = 0; i < NUM_TILE_TYPES; ++i)
    {
        sprintf_s(filename, "Assets/kenney_tiny-dungeon/Tiles/tile_%04d.png", i);
        TileTextures[i] = AEGfxTextureLoad(filename);
        if (!TileTextures[i])
            printf("Failed to load: %s\n", filename);
    }

    // Create player
    player_system_initialize(&player);

    Camera cam = initCam(&player.x, &player.y, 60.0f, 5.0f);

    int gGameRunning = 1;
    while (gGameRunning)
    {
        AESysFrameStart();

        // Tell the Alpha Engine to get ready to draw something with texture.
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

        // Set blend mode to AE_GFX_BM_BLEND, which will allow transparency.
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);

        f32 dt = (f32)AEFrameRateControllerGetFrameTime();

        DrawMap(tile_size);

        getZoom(dt);
        // Bounds anyhow set first
        simulateCamera(&cam, &player.x, &player.y, -6000.0f, 6000.0f, -5000.0f, 5000.0f);

        // Bounds anyhow set first
        simulateCamera(&cam, curr_player_x, curr_player_y, -6000.0f, 6000.0f, -5000.0f, 5000.0f);

        // Update player system
        player_system_update(&player);

        // Render player system
        player_system_render(&player);

        if (AEInputCheckTriggered(AEVK_ESCAPE) || !AESysDoesWindowExist())
            gGameRunning = 0;
        AEGfxSetBackgroundColor(0, 0, 0);
        AESysFrameEnd();
    }

    // Cleanup
    for (int i = 0; i < NUM_TILE_TYPES; ++i)
        if (TileTextures[i])
            AEGfxTextureUnload(TileTextures[i]);
    if (TileMesh)
        AEGfxMeshFree(TileMesh);
    freeMesh();

    AESysExit();
    return 0;
}