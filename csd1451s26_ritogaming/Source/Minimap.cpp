/*!************************************************************************
\file:          Minimap.cpp
\author:        Various
\brief:         Minimap rendering system for the dungeon
**************************************************************************/
#include "Minimap.hpp"
#include "AEEngine.h"
#include "map.hpp"

extern AEGfxVertexList* rect_mesh_center;
static f32 worldW, worldH, mapSize;

// We need these colors from your renderer
extern const color white;
extern const color black;

static AEGfxTexture* pMapParts[4] = { nullptr, nullptr, nullptr, nullptr };

void minimap_init(f32 w, f32 h, f32 size) {
    worldW = w;
    worldH = h;
    mapSize = size;

    // Load the 4 parts
    char filename[128];
    for (int i = 0; i < 4; ++i) {
        sprintf_s(filename, "Assets/Map/maptiles_%d.png", i+1);

        // Safety: only load if the slot is empty
        if (pMapParts[i] == nullptr) {
            pMapParts[i] = AEGfxTextureLoad(filename);
        }
    }
}

// Helper to draw a single "dot" or "block" on the map
void draw_object_on_map(f32 worldX, f32 worldY, f32 size, color c) {
    f32 screenHalfW = AEGfxGetWindowWidth() / 2.0f;
    f32 screenHalfH = AEGfxGetWindowHeight() / 2.0f;
    f32 mapCenterX = screenHalfW - (mapSize / 2.0f) - 20.0f;
    f32 mapCenterY = screenHalfH - (mapSize / 2.0f) - 20.0f;

    f32 scaleFactor = mapSize / worldW;

    f32 finalX = (worldX * scaleFactor) + mapCenterX;
    f32 finalY = (worldY * scaleFactor) + mapCenterY;

    AEMtx33 scale, trans, transform;
    AEMtx33Scale(&scale, size, size);
    AEMtx33Trans(&trans, finalX, finalY);
    AEMtx33Concat(&transform, &trans, &scale);

    AEGfxSetColorToAdd(c.r, c.g, c.b, c.a);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);
}

void minimap_draw_walls(int dungeonMap[TILE_ROWS][TILE_COLS]) {
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // Calculate the size of a single wall block on the minimap
    f32 wallDotSize = mapSize / (f32)TILE_COLS;
    f32 tile_size = 64.0f;

    for (int r = 0; r < TILE_ROWS; ++r) {
        for (int c = 0; c < TILE_COLS; ++c) {
            // Check if the tile is a wall
            if (dungeonMap[r][c] == 1) {
                // Convert array index to world coordinates
                f32 worldX = (c - (TILE_COLS / 2.0f)) * tile_size;
                f32 worldY = ((TILE_ROWS / 2.0f) - r) * tile_size;

                // Draw a dark grey block for the wall
                color wallColor = { 0.3f, 0.3f, 0.3f, 1.0f };
                draw_object_on_map(worldX, worldY, wallDotSize, wallColor);
            }
       }
    }

}

void draw_sprite_on_map_centered(AEGfxTexture* pTex, f32 drawX, f32 drawY, f32 size) {
    if (!pTex) {
        printf("Minimap Error: Player texture is NULL!\n");
        return;
    }
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(pTex, 0, 0);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    AEMtx33 s, t, final;
    AEMtx33Scale(&s, size, size);
    AEMtx33Trans(&t, drawX, drawY);
    AEMtx33Concat(&final, &t, &s);
    AEGfxSetTransform(final.m);
    AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);
}

void minimap_draw_overlay(f32 centerX, f32 centerY) {
    // Each part is half the total map size
    float partSize = mapSize / 1.5f;
    // The center of each corner is shifted by 1/4 of the total map size
    float offset = mapSize / 4.0f;

    // Top Left
    draw_sprite_on_map_centered(pMapParts[0], centerX - offset, centerY + offset, partSize);
    // Top Right
    draw_sprite_on_map_centered(pMapParts[1], centerX + offset, centerY + offset, partSize);
    // Bottom Left
    draw_sprite_on_map_centered(pMapParts[2], centerX - offset, centerY - offset, partSize);
    // Bottom Right
    draw_sprite_on_map_centered(pMapParts[3], centerX + offset, centerY - offset, partSize);
}

void minimap_draw(Player* pPlayer, int dungeonMap[TILE_ROWS][TILE_COLS], AEGfxTexture* tiles[]) {
    if (!pPlayer) return;

    // 1. Setup screen positions
    f32 screenHalfW = AEGfxGetWindowWidth() / 2.0f;
    f32 screenHalfH = AEGfxGetWindowHeight() / 2.0f;
    f32 mapCenterX = screenHalfW - (mapSize / 2.0f) - 20.0f;
    f32 mapCenterY = screenHalfH - (mapSize / 2.0f) - 20.0f;

    // 1b. Draw the Brown Border
    // We make this slightly larger than the mapSize
    //f32 borderThickness = 10.0f;
    //AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    //// Define a Brown Color (R: 0.4, G: 0.2, B: 0.1)
    //AEGfxSetColorToMultiply(0.4f, 0.2f, 0.1f, 1.0f);

    //AEMtx33 sBorder, tBorder, finalBorder;
    //// Scale is the mapSize + the thickness on both sides
    //AEMtx33Scale(&sBorder, mapSize + borderThickness, mapSize + borderThickness);
    //AEMtx33Trans(&tBorder, mapCenterX, mapCenterY);
    //AEMtx33Concat(&finalBorder, &tBorder, &sBorder);

    //AEGfxSetTransform(finalBorder.m);
    //AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);

    // 2. Draw a solid black background for the minimap first
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f); // Solid Black

    AEMtx33 sB, tB, finalB;
    // Use mapSize for both X and Y to keep it a perfect circle
    AEMtx33Scale(&sB, mapSize / 2.0f, mapSize / 2.0f);
    AEMtx33Trans(&tB, mapCenterX, mapCenterY);
    AEMtx33Concat(&finalB, &tB, &sB);

    // Use the circle_mesh from your renderer!
    AEGfxSetTransform(finalB.m);
    AEGfxMeshDraw(circle_mesh, AE_GFX_MDM_TRIANGLES);

    // 3. Setup Zoom and Logic
    float zoomLevel = 0.2f;  // MINIMAP ZOOM
    float miniTileSize = (64.0f * zoomLevel);
    float worldStartX = -1600.f / 2.f;
    float worldStartY = 900.f / 2.f;
    float radius = mapSize / 2.0f;

    // 4. Draw Map Tiles with Math Culling
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    for (int r = 0; r < TILE_ROWS; ++r) {
        for (int c = 0; c < TILE_COLS; ++c) {
            int tileID = dungeonMap[r][c];
            if (tileID > 0 && tileID < NUM_TILE_TYPES && tiles[tileID] != nullptr) {

                float tileWorldX = worldStartX + (c * 64.0f);
                float tileWorldY = worldStartY - (r * 64.0f);

                // Calculate position relative to the player
                float relativeX = (tileWorldX - pPlayer->current_player->x) * zoomLevel;
                float relativeY = (tileWorldY - pPlayer->current_player->y) * zoomLevel;

                // --- MATH CULLING ---
                // Only draw if the tile is inside the mapSize box
                // We use (mapSize/2) because mapCenterX is the center of the square
               /* if (relativeX > -mapSize / 2.0f && relativeX < mapSize / 2.0f &&
                    relativeY > -mapSize / 2.0f && relativeY < mapSize / 2.0f) {

                    float drawX = relativeX + mapCenterX;
                    float drawY = relativeY + mapCenterY;

                    AEMtx33 s, t, final;
                    AEGfxTextureSet(tiles[tileID], 0, 0);
                    AEMtx33Scale(&s, miniTileSize, miniTileSize);
                    AEMtx33Trans(&t, drawX, drawY);
                    AEMtx33Concat(&final, &t, &s);
                    AEGfxSetTransform(final.m);
                    AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);
                }*/
                // --- CIRCULAR MATH CULLING ---
                // Calculate distance squared from the center of the minimap (0,0 in relative space)
                float distanceSquared = (relativeX * relativeX) + (relativeY * relativeY);
                float radius = mapSize / 2.0f;

                // Only draw if the tile is inside the circular radius
                // (Using distanceSquared < radiusSquared is faster than using sqrt)
                if (distanceSquared < (radius * radius)) {
                    float drawX = relativeX + mapCenterX;
                    float drawY = relativeY + mapCenterY;

                    AEMtx33 s, t, final;
                    AEGfxTextureSet(tiles[tileID], 0, 0);
                    AEMtx33Scale(&s, miniTileSize, miniTileSize);
                    AEMtx33Trans(&t, drawX, drawY);
                    AEMtx33Concat(&final, &t, &s);
                    AEGfxSetTransform(final.m);
                    AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);
                }
            }
        }
    }
    // 5. Draw Player (Fixed at the center of the minimap)
    draw_sprite_on_map_centered(pPlayer->current_player->texture, mapCenterX, mapCenterY, 10.0f);

    // 5b. Draw the 4-part HUD Overlay
    float overlaySize = mapSize; // Or adjust to fit your specific textures
    minimap_draw_overlay(mapCenterX, mapCenterY);

    //// 6. Draw Border "Frame" ON TOP of everything to hide bleeding
    //AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    //AEGfxSetColorToMultiply(0.4f, 0.2f, 0.1f, 1.0f); // Brown
    //f32 thickness = 10.0f; // Thickness of your border bar

    //AEMtx33 s, t, final;

    // --- TOP BAR ---
    //AEMtx33Scale(&s, mapSize + (thickness * 2), thickness);
    //AEMtx33Trans(&t, mapCenterX, mapCenterY + (mapSize / 2.0f) + (thickness / 2.0f));
    //AEMtx33Concat(&final, &t, &s);
    //AEGfxSetTransform(final.m);
    //AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);

    //// --- BOTTOM BAR ---
    //AEMtx33Trans(&t, mapCenterX, mapCenterY - (mapSize / 2.0f) - (thickness / 2.0f));
    //AEMtx33Concat(&final, &t, &s);
    //AEGfxSetTransform(final.m);
    //AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);

    //// --- LEFT BAR ---
    //AEMtx33Scale(&s, thickness, mapSize + (thickness * 2));
    //AEMtx33Trans(&t, mapCenterX - (mapSize / 2.0f) - (thickness / 2.0f), mapCenterY);
    //AEMtx33Concat(&final, &t, &s);
    //AEGfxSetTransform(final.m);
    //AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);

    //// --- RIGHT BAR ---
    //AEMtx33Trans(&t, mapCenterX + (mapSize / 2.0f) + (thickness / 2.0f), mapCenterY);
    //AEMtx33Concat(&final, &t, &s);
    //AEGfxSetTransform(final.m);
    //AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);

    //if (!pPlayer) return;

    //// 1. Setup screen positions

    //f32 screenHalfW = AEGfxGetWindowWidth() / 2.0f;
    //f32 screenHalfH = AEGfxGetWindowHeight() / 2.0f;
    //f32 mapCenterX = screenHalfW - (mapSize / 2.0f) - 20.0f;
    //f32 mapCenterY = screenHalfH - (mapSize / 2.0f) - 20.0f;
    //f32 scaleFactor = mapSize / worldW;



    //// 2. Reset State

    //AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    //AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);


    //// 3a. Draw Border

    //f32 borderThickness = 10.0f; // Total thickness in pixels
    //AEMtx33 sB, tB, finalB;
    //AEMtx33 s, t, final;

    //// Scale is mapSize + thickness

    ///*

    //AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);

    //AEMtx33Scale(&sB, mapSize + borderThickness, mapSize + borderThickness);

    //AEMtx33Trans(&tB, mapCenterX, mapCenterY);

    //AEMtx33Concat(&finalB, &tB, &sB);

    //AEGfxSetTransform(finalB.m);

    //AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);



    //// 3b. Draw Background Box

    //AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f); // Almost black

    //AEMtx33 s, t, final;

    //AEMtx33Scale(&s, mapSize, mapSize);

    //AEMtx33Trans(&t, mapCenterX, mapCenterY);

    //AEMtx33Concat(&final, &t, &s);

    //AEGfxSetTransform(final.m);

    //AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);*/



    //// 4. DRAW MAP TILES

    //AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    //float miniTileSize = (64.0f * scaleFactor);



    //// Use the same start logic as your friend's map.cpp
    //float startX = -1600.f / 2.f;
    //float startY = 900.f / 2.f;



    //for (int r = 0; r < TILE_ROWS; ++r) {
    //    for (int c = 0; c < TILE_COLS; ++c) {
    //        int tileID = dungeonMap[r][c];
    //        // ONLY draw if the tile exists (is not 0/EMPTY)
    //        if (tileID > 0 && tileID < NUM_TILE_TYPES && tiles[tileID] != nullptr) {
    //            // Convert world coordinates using map.cpp logic

    //            float worldX = startX + c * 64.0f;
    //            float worldY = startY - r * 64.0f;

    //            // Scale those coordinates for the minimap position

    //            float drawX = (worldX * scaleFactor) + mapCenterX;
    //            float drawY = (worldY * scaleFactor) + mapCenterY;



    //            AEGfxTextureSet(tiles[tileID], 0, 0);

    //            AEMtx33Scale(&s, miniTileSize, miniTileSize);

    //            AEMtx33Trans(&t, drawX, drawY);

    //            AEMtx33Concat(&final, &t, &s);

    //            AEGfxSetTransform(final.m);

    //            AEGfxMeshDraw(rect_mesh_center, AE_GFX_MDM_TRIANGLES);

    //        }

    //    }

    //}



    //// Draw Character 1 - USING CHARACTER STRUCT NOW

    //f32 c1X = (pPlayer->char1.x * scaleFactor) + mapCenterX;

    //f32 c1Y = (pPlayer->char1.y * scaleFactor) + mapCenterY;

    //DrawSpriteOnMap(c1X, c1Y, 12.0f, pPlayer->char1.texture);



    //// Draw Character 2 - USING CHARACTER STRUCT NOW

    //f32 c2X = (pPlayer->char2.x * scaleFactor) + mapCenterX;

    //f32 c2Y = (pPlayer->char2.y * scaleFactor) + mapCenterY;

    //DrawSpriteOnMap(c2X, c2Y, 12.0f, pPlayer->char2.texture);

}

void draw_sprite_on_map(f32 worldX, f32 worldY, f32 spriteScale, AEGfxTexture* pTex) {
    if (pTex == nullptr) return;

    // 1. Calculate screen positions (Same math as before)
    f32 screenHalfW = AEGfxGetWindowWidth() / 2.0f;
    f32 screenHalfH = AEGfxGetWindowHeight() / 2.0f;
    f32 mapCenterX = screenHalfW - (mapSize / 2.0f) - 20.0f;
    f32 mapCenterY = screenHalfH - (mapSize / 2.0f) - 20.0f;
    f32 scaleFactor = mapSize / worldW;

    // Use the same origin as the tiles
    f32 startX = -1600.f / 2.f;
    f32 startY = 900.f / 2.f;

    f32 finalX = ((worldX - startX) * scaleFactor) + mapCenterX;
    f32 finalY = ((worldY - startY) * scaleFactor) + mapCenterY;

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

void minimap_unload() {
    for (int i = 0; i < 4; ++i) {
        if (pMapParts[i] != nullptr) {
            AEGfxTextureUnload(pMapParts[i]);
            pMapParts[i] = nullptr; // Crucial reset
        }
    }
}