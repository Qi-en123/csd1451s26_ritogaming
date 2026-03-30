/*!************************************************************************
\file:          Minimap.cpp
\author:        Ahmad Qusyairi
\brief:         Minimap rendering system for the dungeon
**************************************************************************/

#include "Minimap.hpp"
#include "AEEngine.h"
#include "map.hpp"      // for map tiles
#include "renderer.hpp" // for render_ui_sprite

// =========================================================================
// INTERNAL (file-private)
// =========================================================================
namespace
{
    constexpr f32 MINIMAP_PADDING = 0.0f;
    constexpr f32 HALF = 2.0f;
    constexpr f32 TILE_SIZE = 64.0f;
    constexpr f32 MINIMAP_ZOOM = 0.2f;
    constexpr f32 WORLD_WIDTH = 1600.0f;
    constexpr f32 WORLD_HEIGHT = 900.0f;
    constexpr f32 OVERLAY_SCALE = 1.5f;
    constexpr f32 OVERLAY_OFFSET_DIV = 4.0f;
    constexpr f32 PLAYER_DOT_SIZE = 10.0f;

    AEGfxTexture* pMapParts[4] = { nullptr, nullptr, nullptr, nullptr };

    // ---------------------------------------------------------------------

    void get_world_start(f32& x, f32& y)
    {
        x = -(WORLD_WIDTH / HALF);
        y = (WORLD_HEIGHT / HALF);
    }

    void get_map_center(f32& outX, f32& outY, f32 mapSize)
    {
        outX = (AEGfxGetWindowWidth() / HALF) - (mapSize / HALF) - MINIMAP_PADDING;
        outY = (AEGfxGetWindowHeight() / HALF) - (mapSize / HALF) - MINIMAP_PADDING;
    }

    void tile_to_world_center(int r, int c, f32& x, f32& y)
    {
        f32 startX, startY;
        get_world_start(startX, startY);

        x = startX + (c * TILE_SIZE) + (TILE_SIZE / 2.0f);
        y = startY - (r * TILE_SIZE) - (TILE_SIZE / 2.0f);
    }
}

// =========================================================================
// GLOBAL STATE
// =========================================================================

f32 totalWorldW;
f32 totalWorldH;
f32 mapSize;

// =========================================================================

void minimap_init(f32 w, f32 h, f32 size)
{
    totalWorldW = w;
    totalWorldH = h;
    mapSize = size;

    char filename[128];
    for (int i = 0; i < 4; ++i)
    {
        sprintf_s(filename, "Assets/Map/maptiles_%d.png", i + 1);
        if (!pMapParts[i])
            pMapParts[i] = AEGfxTextureLoad(filename);
    }
}

// =========================================================================

void minimap_draw_overlay(f32 centerX, f32 centerY)
{
    f32 partSize = mapSize / OVERLAY_SCALE;
    f32 cullPadding = (TILE_SIZE * MINIMAP_ZOOM) / HALF;
    f32 offset = mapSize / OVERLAY_OFFSET_DIV + (cullPadding / HALF);

    render_ui_sprite(rect_mesh_center, centerX - offset, centerY + offset, partSize, partSize, pMapParts[0], white);
    render_ui_sprite(rect_mesh_center, centerX + offset, centerY + offset, partSize, partSize, pMapParts[1], white);
    render_ui_sprite(rect_mesh_center, centerX - offset, centerY - offset, partSize, partSize, pMapParts[2], white);
    render_ui_sprite(rect_mesh_center, centerX + offset, centerY - offset, partSize, partSize, pMapParts[3], white);
}

// =========================================================================

void minimap_draw(Player* pPlayer, int dungeonMap[TILE_ROWS][TILE_COLS], AEGfxTexture* tiles[])
{
    if (!pPlayer) return;

    f32 mapCenterX, mapCenterY;
    get_map_center(mapCenterX, mapCenterY, mapSize);

    // Background circle
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(0, 0, 0, 1);

    AEMtx33 sB, tB, finalB;
    AEMtx33Scale(&sB, mapSize / HALF, mapSize / HALF);
    AEMtx33Trans(&tB, mapCenterX, mapCenterY);
    AEMtx33Concat(&finalB, &tB, &sB);

    AEGfxSetTransform(finalB.m);
    AEGfxMeshDraw(circle_mesh, AE_GFX_MDM_TRIANGLES);

    f32 miniTileSize = TILE_SIZE * MINIMAP_ZOOM;
    f32 radius = mapSize / HALF;
    f32 cullPadding = miniTileSize / HALF;

    // Bounds
    f32 mapMinX = FLT_MAX, mapMaxX = -FLT_MAX;
    f32 mapMinY = -FLT_MAX, mapMaxY = FLT_MAX;

    for (int r = 0; r < TILE_ROWS; ++r)
    {
        for (int c = 0; c < TILE_COLS; ++c)
        {
            if (dungeonMap[r][c] > 0)
            {
                f32 tx, ty;
                tile_to_world_center(r, c, tx, ty);

                if (tx < mapMinX) mapMinX = tx;
                if (tx > mapMaxX) mapMaxX = tx;
                if (ty > mapMinY) mapMinY = ty;
                if (ty < mapMaxY) mapMaxY = ty;
            }
        }
    }

    f32 viewRadius = radius / MINIMAP_ZOOM;

    f32 clampedX = pPlayer->current_player->x;
    f32 clampedY = pPlayer->current_player->y;

    if (clampedX < mapMinX + viewRadius) clampedX = mapMinX + viewRadius;
    if (clampedX > mapMaxX - viewRadius) clampedX = mapMaxX - viewRadius;
    if (clampedY > mapMinY - viewRadius) clampedY = mapMinY - viewRadius;
    if (clampedY < mapMaxY + viewRadius) clampedY = mapMaxY + viewRadius;

    // Draw tiles
    for (int r = 0; r < TILE_ROWS; ++r)
    {
        for (int c = 0; c < TILE_COLS; ++c)
        {
            int tileID = dungeonMap[r][c];

            if (tileID > 0 && tileID < NUM_TILE_TYPES && tiles[tileID])
            {
                f32 tileX, tileY;
                tile_to_world_center(r, c, tileX, tileY);

                f32 relX = (tileX - clampedX) * MINIMAP_ZOOM;
                f32 relY = (tileY - clampedY) * MINIMAP_ZOOM;

                f32 distSq = relX * relX + relY * relY;

                if (distSq < (radius + cullPadding) * (radius + cullPadding))
                {
                    render_ui_sprite(
                        rect_mesh_center,
                        mapCenterX + relX,
                        mapCenterY + relY,
                        miniTileSize,
                        miniTileSize,
                        tiles[tileID],
                        white
                    );
                }
            }
        }
    }

    // Player
    f32 playerX = (pPlayer->current_player->x - clampedX) * MINIMAP_ZOOM;
    f32 playerY = (pPlayer->current_player->y - clampedY) * MINIMAP_ZOOM;

    render_ui_sprite(
        rect_mesh_center,
        mapCenterX + playerX,
        mapCenterY + playerY,
        PLAYER_DOT_SIZE,
        PLAYER_DOT_SIZE,
        pPlayer->current_player->texture,
        white
    );

    minimap_draw_overlay(mapCenterX, mapCenterY);
}

// =========================================================================

void draw_sprite_on_map(f32 worldX, f32 worldY, f32 spriteScale, AEGfxTexture* pTex)
{
    if (!pTex) return;

    f32 mapCenterX, mapCenterY;
    get_map_center(mapCenterX, mapCenterY, mapSize);

    f32 startX, startY;
    get_world_start(startX, startY);

    f32 scaleFactor = mapSize / totalWorldW;

    f32 finalX = ((worldX - startX) * scaleFactor) + mapCenterX;
    f32 finalY = ((worldY - startY) * scaleFactor) + mapCenterY;

    render_ui_sprite(rect_mesh_center, finalX, finalY, spriteScale, spriteScale, pTex, white);
}

// =========================================================================

void minimap_unload()
{
    for (int i = 0; i < 4; ++i)
    {
        if (pMapParts[i])
        {
            AEGfxTextureUnload(pMapParts[i]);
            pMapParts[i] = nullptr;
        }
    }
}