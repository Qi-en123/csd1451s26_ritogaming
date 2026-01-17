/*!************************************************************************
\file:          map.hpp
\author:        Qien
\par DP email:  g.qien@digipen.edu
\brief:         This header declares dungeon map rendering and initialization
                functions.
**************************************************************************/
#include "AEEngine.h"

#define TILE_ROWS 90
#define TILE_COLS 160

void InitializeDungeonMap();
void DrawMap(float tile_size);
void DrawTile(AEGfxTexture* tex, float topLeftX, float topLeftY, float tile_size);

extern int map[90][160];
