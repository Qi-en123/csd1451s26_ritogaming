/*!************************************************************************
\file:          Minimap.cpp
\author:        Various
\brief:         Minimap rendering system for the dungeon
**************************************************************************/
#include "Minimap.hpp"
#include "AEEngine.h"
#include "map.hpp"

extern AEGfxVertexList* rect_mesh_center;
static float worldW, worldH, mapSize;

// We need these colors from your renderer
extern const color white;
extern const color black;

void Minimap_Init(float w, float h, float size) {
    worldW = w;
    worldH = h;
    mapSize = size;
}

// Helper to draw a single "dot" or "block" on the map
void DrawObjectOnMap(float worldX, float worldY, float size, color c) {
    float screenHalfW = AEGfxGetWindowWidth() / 2.0f;
    float screenHalfH = AEGfxGetWindowHeight() / 2.0f;
    float mapCenterX = screenHalfW - (mapSize / 2.0f) - 20.0f;
    float mapCenterY = screenHalfH - (mapSize / 2.0f) - 20.0f;

    float scaleFactor = mapSize / worldW;

    float finalX = (worldX * scaleFactor) + mapCenterX;
    float finalY = (worldY * scaleFactor) + mapCenterY;

    AEMtx33 scale, trans, transform;
    AEMtx33Scale(&scale, size, size);
    AEMtx33Trans(&trans, finalX, finalY);
    AEMtx33Concat(&transform, &trans, &scale);

    AEGfxSetColorToAdd(c.r, c.g, c.b, c.a);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);
}

void Minimap_DrawWalls(int dungeonMap[TILE_ROWS][TILE_COLS]) {
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // Calculate the size of a single wall block on the minimap
    float wallDotSize = mapSize / (float)TILE_COLS;
    float tile_size = 64.0f; // Matches your main.cpp

    for (int r = 0; r < TILE_ROWS; ++r) {
        for (int c = 0; c < TILE_COLS; ++c) {
            // Check if the tile is a wall (usually 1, but check your map.hpp)
            if (dungeonMap[r][c] == 1) {
                // Convert array index to world coordinates
                // Assuming (0,0) is center:
                float worldX = (c - (TILE_COLS / 2.0f)) * tile_size;
                float worldY = ((TILE_ROWS / 2.0f) - r) * tile_size;

                // Draw a dark grey block for the wall
                color wallColor = { 0.3f, 0.3f, 0.3f, 1.0f };
                DrawObjectOnMap(worldX, worldY, wallDotSize, wallColor);
            }
        }
    }
}

void Minimap_Draw(Player* pPlayer, int dungeonMap[TILE_ROWS][TILE_COLS], AEGfxTexture* tiles[]) {
    if (!pPlayer) return;

    // 1. Setup screen positions
    float screenHalfW = AEGfxGetWindowWidth() / 2.0f;
    float screenHalfH = AEGfxGetWindowHeight() / 2.0f;
    float mapCenterX = screenHalfW - (mapSize / 2.0f) - 20.0f;
    float mapCenterY = screenHalfH - (mapSize / 2.0f) - 20.0f;
    float scaleFactor = mapSize / worldW;

    // 2. Reset State
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    // 3a. Draw Border
    float borderThickness = 50.0f; // Total thickness in pixels
    AEMtx33 sB, tB, finalB;

    // Scale is mapSize + thickness
    AEMtx33Scale(&sB, mapSize + borderThickness + 150, mapSize + borderThickness);
    AEMtx33Trans(&tB, mapCenterX, mapCenterY);
    AEMtx33Concat(&finalB, &tB, &sB);
    AEGfxSetTransform(finalB.m);
    AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);

    // 3b. Draw Background Box
    AEGfxSetColorToMultiply(0.05f, 0.05f, 0.05f, 0.9f); // Almost black
    AEMtx33 s, t, final;
    AEMtx33Scale(&s, mapSize, mapSize);
    AEMtx33Trans(&t, mapCenterX, mapCenterY);
    AEMtx33Concat(&final, &t, &s);
    AEGfxSetTransform(final.m);
    AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);

    // 4. DRAW MAP TILES
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

    // Calculate how big one tile is on the minimap
    float miniTileSize = (64.0f * scaleFactor) + 0.5f;

    for (int r = 0; r < TILE_ROWS; ++r) {
        for (int c = 0; c < TILE_COLS; ++c) {
            int tileID = dungeonMap[r][c];

            // Only draw if it's a valid tile and NOT an empty floor (optional)
            if (tileID >= 0 && tileID < NUM_TILE_TYPES && tiles[tileID] != nullptr) {

                // Math to match your world drawing logic
                float worldX = (c - (TILE_COLS / 2.0f)) * 64.0f;
                float worldY = ((TILE_ROWS / 2.0f) - r) * 64.0f;

                float drawX = (worldX * scaleFactor) + mapCenterX;
                float drawY = (worldY * scaleFactor) + mapCenterY;

                AEGfxTextureSet(tiles[tileID], 0, 0);
                AEMtx33Scale(&s, miniTileSize, miniTileSize);
                AEMtx33Trans(&t, drawX, drawY);
                AEMtx33Concat(&final, &t, &s);
                AEGfxSetTransform(final.m);
                AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);
            }
        }
    }

    // Draw Character 1 - USING CHARACTER STRUCT NOW
    float c1X = (pPlayer->char1.x * scaleFactor) + mapCenterX;
    float c1Y = (pPlayer->char1.y * scaleFactor) + mapCenterY;
    DrawSpriteOnMap(c1X, c1Y, 12.0f, pPlayer->char1.texture);

    // Draw Character 2 - USING CHARACTER STRUCT NOW
    float c2X = (pPlayer->char2.x * scaleFactor) + mapCenterX;
    float c2Y = (pPlayer->char2.y * scaleFactor) + mapCenterY;
    DrawSpriteOnMap(c2X, c2Y, 12.0f, pPlayer->char2.texture);
}

void DrawSpriteOnMap(float worldX, float worldY, float spriteScale, AEGfxTexture* pTex) {
    if (pTex == nullptr) return;

    // 1. Calculate screen positions (Same math as before)
    float screenHalfW = AEGfxGetWindowWidth() / 2.0f;
    float screenHalfH = AEGfxGetWindowHeight() / 2.0f;
    float mapCenterX = screenHalfW - (mapSize / 2.0f) - 20.0f;
    float mapCenterY = screenHalfH - (mapSize / 2.0f) - 20.0f;
    float scaleFactor = mapSize / worldW;

    float finalX = (worldX * scaleFactor) + mapCenterX;
    float finalY = (worldY * scaleFactor) + mapCenterY;

    // 2. Switch to Texture Mode
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pTex, 0, 0);

    // Reset tint to white so the sprite isn't green/black
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    // 3. Transform (We scale the sprite based on the minimap ratio)
    AEMtx33 s, t, final;
    // We make the sprite a bit larger than a dot so it's visible (e.g., 20px)
    AEMtx33Scale(&s, spriteScale, spriteScale);
    AEMtx33Trans(&t, finalX, finalY);
    AEMtx33Concat(&final, &t, &s);

    AEGfxSetTransform(final.m);
    AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);

    // Cleanup: switch back to color mode for other UI elements
    AEGfxTextureSet(NULL, 0, 0);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
}