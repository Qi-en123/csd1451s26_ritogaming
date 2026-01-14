#include "AEEngine.h"
void InitializeDungeonMap();
void DrawMap(float tile_size);
void DrawTile(AEGfxTexture* tex, float topLeftX, float topLeftY, float tile_size);
void CreateTileMesh(AEGfxVertexList** mesh);

int map[90][160];
