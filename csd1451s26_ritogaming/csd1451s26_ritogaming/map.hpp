#include "AEEngine.h"
void InitializeDungeonMap();
void DrawMap(float tile_size, float screen_w, float screen_h);
void DrawTile(AEGfxTexture* tex, float topLeftX, float topLeftY, float tile_size);
void CreateTileMesh(AEGfxVertexList** mesh);

int map[90][160];
