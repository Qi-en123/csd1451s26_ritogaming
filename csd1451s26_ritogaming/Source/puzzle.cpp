/*!************************************************************************
\file:			puzzle.cpp
\author:		Jerod
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			awd
**************************************************************************/
#include "puzzle.hpp"
#include "AEEngine.h"
#include "renderer.hpp"
#include "collision.hpp"
#include "map.hpp"
#include "utils.hpp"
#include <vector>

AEGfxTexture* element_block_textures[ELEMENT_COUNT]{};

int num_puzzle_blocks{ 2 };
std::vector<element_block> puzzle_blocks;

void load_puzzle_textures() {
	for (int i{}; i < ELEMENT_COUNT; ++i) {
		//rn is all this tex, will change
		element_block_textures[i] = AEGfxTextureLoad("Assets/Tiles/tile_0006.png");
	}
}

void intialize_puzzle_blocks() {
	puzzle_blocks.reserve(num_puzzle_blocks);

	for (int i{}; i < num_puzzle_blocks; ++i) {
		puzzle_blocks.emplace_back(WATER, tile_size * i, tile_size * i, element_block_textures[WATER]);
	}
}

void render_puzzle_blocks() {
	for (int i{}; i < num_puzzle_blocks; ++i) {
		element_block* curr = &puzzle_blocks[i];

		render_sprite(rect_mesh_center, curr->position.x,
			curr->position.y, tile_size, tile_size, curr->texture);
	}
}

bool player_collide_puzzles(f32 newX, f32 newY, f32 playerHalfSize, s32& puzzle_index)
{
	for (int i{}; i < num_puzzle_blocks; ++i)
	{
		if (ss_collision(newX, newY, playerHalfSize, playerHalfSize,
			puzzle_blocks[i].position.x, puzzle_blocks[i].position.y, tile_size / 2.0f, tile_size / 2.0f))
		{
			// Debug: print tile coordinates
			//printf("Colliding with puzzle at (%.2f, %.2f)\n", e_block.position.x, e_block.position.y);
			puzzle_index = i;
			return true; // collision detected
		}
	}
	return false; // no collision
}

void move_puzzle(s32 index, Direction direction, Element player_elem) {
	element_block& puzzle = puzzle_blocks[index];

	if (puzzle.element != player_elem) return;

	f32 dx{}, dy{};

	switch (direction)
	{
	case LEFT:
		dx = -PUZZLE_BLOCK_SPEED * dt;
		break;
	case RIGHT:
		dx = PUZZLE_BLOCK_SPEED * dt;
		break;
	case UP:
		dy = -PUZZLE_BLOCK_SPEED * dt;
		break;
	case DOWN:
		dy = PUZZLE_BLOCK_SPEED * dt;
		break;
	default:
		break;
	}

	if (!check_puzzle_collision(puzzle.position.x + dx, puzzle.position.y + dy)) {
		puzzle.position.x += dx;
		puzzle.position.y += dy;
	}
}

//based on player movement speed, should change player to use dt
//calls when "Drag" key is pressed
void drag_puzzle(f32 player_pos_x, f32 player_pos_y, f32 player_half_size,
	bool& dragging, s32 index, Element player_elem) {

	element_block& puzzle = puzzle_blocks[index];

	f32 dx{ player_pos_x - puzzle.position.x };
	f32 dy{ player_pos_y - puzzle.position.y };

	const f32 min_gap{ 3.0f }; //prevent puzzle going inside wall
	const f32 grab_range{ (player_half_size + tile_size / 2.0f) + PUZZLE_GRAB_RANGE };

	if ((dx * dx + dy * dy) < (grab_range * grab_range) &&
		puzzle.element == player_elem) {
		//printf("is dragging");
		dragging = true;
		const f32 min_dist = player_half_size + (tile_size / 2.0f) + min_gap;
		f32 newX{ puzzle.position.x }, newY{ puzzle.position.y };

		//checks if player is top/bottom or left/right using x/y overlap distance
		if ((player_half_size + tile_size / 2.0f) - fabsf(dx) <
			(player_half_size + tile_size / 2.0f) - fabsf(dy)) {

			newX = (puzzle.position.x > player_pos_x) ?
				player_pos_x + min_dist : player_pos_x - min_dist;
		}
		else {
			newY = (puzzle.position.y > player_pos_y) ?
				player_pos_y + min_dist : player_pos_y - min_dist;
		}

		if (!check_puzzle_collision(newX, newY)) {
			puzzle.position.x = newX;
			puzzle.position.y = newY;
		}

	}
	else {
		dragging = false;
	}
}

void unload_puzzle_textures() {
	for (int i{}; i < ELEMENT_COUNT; ++i) {
		AEGfxTextureUnload(element_block_textures[i]);
	}
}