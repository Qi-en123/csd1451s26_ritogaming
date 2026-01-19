#include "collision.hpp"

// Check collision with tile map
// Convert world coordinates to tile indices
static bool WorldToTile(float worldX, float worldY, int& outCol, int& outRow)
{
    outCol = static_cast<int>((worldX - MAP_START_X) / TILE_SIZE);
    outRow = static_cast<int>((MAP_START_Y - worldY) / TILE_SIZE);

    // Check if outside map
    if (outCol < 0 || outCol >= TILE_COLS || outRow < 0 || outRow >= TILE_ROWS)
        return false; // outside map
    return true;      // inside map
}

// Check collision with tile map
bool check_player_collision(const Player* player, float newX, float newY, int map[TILE_ROWS][TILE_COLS])
{
    if (!player) return true; // treat null player as collision

    float half = player->config.characterSize * 0.5f;

    // Four corners of player hitbox
    const float corners[4][2] = {
        { newX - half, newY + half }, // top-left
        { newX + half, newY + half }, // top-right
        { newX - half, newY - half }, // bottom-left
        { newX + half, newY - half }  // bottom-right
    };

    // Check each corner for collision
    for (int i = 0; i < 4; ++i)
    {
        int col, row;
        if (!WorldToTile(corners[i][0], corners[i][1], col, row))
            return true; // treat out-of-map as wall

        if (map[row][col] == 1) // wall tile
            return true; // collision detected
    }

    return false; // no collision
}