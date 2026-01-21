/*!************************************************************************
\file:						level1.cpp
\author:					Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This file implements Level 1 functions to load the assests from a file, initialize the level, 
update game logic, render, free resources,
**************************************************************************/
#include "AEEngine.h"
#include <cstdio>
#include "map.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "player.hpp"

#define NUM_TILE_TYPES 7
#define TILE_ROWS 32
#define TILE_COLS 64

static Player player;
static Camera cam;
static float tile_size;
 f32 zoom;
 AEGfxTexture* TileTextures[NUM_TILE_TYPES];
 AEGfxVertexList* TileMesh = nullptr;
static f32 dt;
void level1_load()
{
    
    

   


    // Tell the Alpha Engine to set the background to black.
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
    

    for (int i = 0; i < NUM_TILE_TYPES; i++)
        TileTextures[i] = nullptr;
     TileMesh = nullptr;

    // Load textures properly
    char filename[128];
    for (int i = 0; i < NUM_TILE_TYPES; ++i)
    {
        sprintf_s(filename, "Assets/Tiles/tile_%04d.png", i);
        TileTextures[i] = AEGfxTextureLoad(filename);
        if (!TileTextures[i])
            printf("Failed to load: %s\n", filename);
    }
}

void level1_initialize()
{
    zoom = 1.0f ;
    player_system_initialize(&player);
     cam = initCam(&player.x, &player.y, 60.0f, 5.0f);
    extern int map[TILE_ROWS][TILE_COLS];
    initMesh();

    // Determine tile size to fit both width and height
     tile_size = 25.f;


    // Tell the Alpha Engine to get ready to draw something with texture.
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

    // Set blend mode to AE_GFX_BM_BLEND, which will allow transparency.
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    InitializeDungeonMap();
}

void level1_update()
{
  

   

     dt = (f32)AEFrameRateControllerGetFrameTime();

    

    getZoom(dt);
    //Bounds anyhow set first
    simulateCamera(&cam, &player.x, &player.y, -6000.0f, 6000.0f, -5000.0f, 5000.0f);

    // Update player system
    player_system_update(&player);



}

void level1_draw()
{
    
    DrawMap(tile_size);
    // Render player system
    player_system_render(&player);
}

void level1_free()
{
    // Cleanup

    freeMesh();
}

void level1_unload()
{
    for (int i = 0; i < NUM_TILE_TYPES; ++i)
        if (TileTextures[i]) AEGfxTextureUnload(TileTextures[i]);
    if (TileMesh) AEGfxMeshFree(TileMesh);
}