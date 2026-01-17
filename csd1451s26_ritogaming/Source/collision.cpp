#include "collision.hpp"

// Check collision with tile map
bool check_player_collision(const Player* player, float newX, float newY, int map[TILE_ROWS][TILE_COLS], float tile_size, float startX, float startY)
{
    if (!player) return true; // treat null player as collision

    // Compute the corners of the player's hitbox
    int tileLeft = (int)((newX - player->config.characterSize / 2 - (startX - tile_size / 2)) / tile_size);
    int tileRight = (int)((newX + player->config.characterSize / 2 - (startX - tile_size / 2)) / tile_size);
    int tileTop = (int)(((startY + tile_size / 2) - (newY - player->config.characterSize / 2)) / tile_size);
    int tileBottom = (int)(((startY + tile_size / 2) - (newY + player->config.characterSize / 2)) / tile_size);


    // Clamp tile indices to map bounds
    tileLeft = (tileLeft < 0) ? 0 : (tileLeft >= TILE_COLS) ? TILE_COLS - 1 : tileLeft;
    tileRight = (tileRight < 0) ? 0 : (tileRight >= TILE_COLS) ? TILE_COLS - 1 : tileRight;
    tileTop = (tileTop < 0) ? 0 : (tileTop >= TILE_ROWS) ? TILE_ROWS - 1 : tileTop;
    tileBottom = (tileBottom < 0) ? 0 : (tileBottom >= TILE_ROWS) ? TILE_ROWS - 1 : tileBottom;

    // Check all corners for wall tiles (value == 1)
    if (map[tileTop][tileLeft] == 1 || map[tileTop][tileRight] == 1 ||
        map[tileBottom][tileLeft] == 1 || map[tileBottom][tileRight] == 1)
    {
        return true; // collision detected
    }

    return false; // no collision
}
