/*!************************************************************************
\file:          map.hpp
\author:        Qien
\par DP email:  g.qien@digipen.edu
\brief:         This header declares dungeon map rendering and initialization
                functions.
**************************************************************************/
#include "AEEngine.h"
#define NUM_TILE_TYPES 131
#define TILE_ROWS 90
#define TILE_COLS 160

extern f32 tile_size;
extern AEGfxTexture* TileTextures[];
extern AEGfxVertexList* TileMesh;

void InitializeDungeonMap();
void DrawMap(float tile_size);
void DrawTile(AEGfxTexture* tex, float topLeftX, float topLeftY, float tile_size);

extern int map[90][160];
//int map[90][160];
