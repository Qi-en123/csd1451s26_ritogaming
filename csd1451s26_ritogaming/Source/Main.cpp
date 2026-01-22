#include <crtdbg.h>

#include "AEEngine.h"
#include "map.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "collision.hpp"
#include <vector>


camera cam{};
f32 zoom{ 1.0f };

f32 tile_size = 64.0f;

AEGfxTexture* TileTextures[NUM_TILE_TYPES] = { nullptr };
AEGfxVertexList* TileMesh = nullptr;

extern int map[TILE_ROWS][TILE_COLS];

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, nullptr);
    AESysSetWindowTitle("Ahmad's Dungeon");

    // Determine tile size to fit both width and height
    //f32 tile_size = 64.0f;

    InitializeDungeonMap();            // step 1: setup map
    generate_walls_from_map(map, 64.0f); // step 2: make walls from map
    printf("Walls created: %zu\n", g_walls.size());



    // Create mesh
    init_mesh();

    // Load textures properly
    char filename[128];
    for (s32 i = 0; i < NUM_TILE_TYPES; ++i)
    {
        sprintf_s(filename, "Assets/Tiles/tile_%04d.png", i);
        TileTextures[i] = AEGfxTextureLoad(filename);
        if (!TileTextures[i])
            printf("Failed to load: %s\n", filename);
    }

    // Create player
    player player_instance;
    player_system_init(&player_instance);

	//js to test camera (UPDATE CURR_PLAYER TO CHANGE CAMERA VIEW)
	f32* curr_player_x{ &player_instance.char1_x }, * curr_player_y{ &player_instance.char1_y };

	//Starting position of camera
	cam = init_cam(curr_player_x, curr_player_y, 60.0f, 0.0f);

	//int gGameRunning = 1;
	//while (gGameRunning)
    //camera cam = init_cam(&player_instance.x, &player_instance.y, 60.0f, 5.0f);

    s32 g_game_running = 1;
    while (g_game_running)
    {
        AESysFrameStart();

        //AEGfxSetBackgroundColor(0.6f, 0.2f, 0.1f);

        // Tell the Alpha Engine to get ready to draw something with texture.
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

        // Set blend mode to AE_GFX_BM_BLEND, which will allow transparency.
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);

        f32 dt = (f32)AEFrameRateControllerGetFrameTime();

        DrawMap(tile_size);
        // Debug: draw all walls as semi-transparent red
        //debug_draw_all_walls();

        get_zoom(dt);
        //Bounds anyhow set first
        simulate_camera(&cam, &player_instance.x, &player_instance.y, -6000.0f, 6000.0f, -5000.0f, 5000.0f);

        // Update player system
        player_system_update(&player_instance);

        // Render player system
        player_system_render(&player_instance);

        if (AEInputCheckTriggered(AEVK_ESCAPE) || !AESysDoesWindowExist())
            g_game_running = 0;

        AESysFrameEnd();
    }

    // Cleanup
    for (s32 i = 0; i < NUM_TILE_TYPES; ++i)
        if (TileTextures[i]) AEGfxTextureUnload(TileTextures[i]);
    if (TileMesh) AEGfxMeshFree(TileMesh);
    free_mesh();

    AESysExit();
    return 0;
}