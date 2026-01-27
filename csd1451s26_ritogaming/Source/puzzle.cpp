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
		puzzle_blocks.emplace_back(WATER, tile_size*i, tile_size*i, element_block_textures[WATER]);
	}
}

void render_puzzle_blocks() {
	for (int i{}; i < num_puzzle_blocks; ++i) {
		element_block* curr = &puzzle_blocks[i];

		render_sprite(rect_mesh_center, curr->position.x,
			curr->position.y, tile_size, tile_size, curr->texture);
	}
}

//useless
//void check_puzzle_collision(player* player) {
//	for (int i{}; i < num_puzzle_blocks; ++i) {
//		element_block* curr = &puzzle_blocks[i];
//
//		f32 hts = tile_size / 2.0f;
//		f32 hps = player->config.character_size / 2.0f;
//
//		if (ss_collision(player->char1_x, player->char1_y, hps, hps,
//			curr[i].position.x, curr[i].position.y, hts, hts)) {
//
//			if (fabsf(player->char1_x - curr[i].position.x) <
//				fabsf(player->char1_y - curr[i].position.y)) {
//				player->char1_x -= player->config.move_speed;
//			}
//			else {
//				player->char1_y -= player->config.move_speed;
//			}
//		}
//	}
//}

bool player_collide_puzzles(f32 newX, f32 newY, f32 playerHalfSize, s32& puzzle_index)
{
	for (int i{}; i<num_puzzle_blocks; ++i)
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

//
//bool puzzle_collide_wall() {
//
//}

void move_puzzle(s32 index, Direction direction) {
	switch (direction)
	{
	case LEFT:
		puzzle_blocks[index].position.x -= 1.0f;
		break;
	case RIGHT:
		puzzle_blocks[index].position.x += 1.0f;
		break;
	case UP:
		puzzle_blocks[index].position.y -= 1.0f;
		break;
	case DOWN:
		puzzle_blocks[index].position.y += 1.0f;
		break;
	default:
		break;
	}
}

void drag_puzzle(f32 player_pos_x, f32 player_pos_y, f32 player_half_size, bool& dragging) {
	for (int i{}; i < num_puzzle_blocks; ++i) {
		element_block* curr = &puzzle_blocks[i];

		f32 dx = player_pos_x - curr->position.x;
		f32 dy = player_pos_y - curr->position.y;
		f32 dist = sqrtf(dx * dx + dy * dy);

		//printf("%f\n", dist);

		if (dist < (player_half_size + tile_size / 2.0f) + 15.0f) {
			//printf("is dragging");
			dragging = true;
			//checks if player is top/bottom or left/right using x/y overlap distance
			if ((player_half_size + tile_size / 2.0f) - fabsf(dx) <
				(player_half_size + tile_size / 2.0f) - fabsf(dy)) {
				if (curr->position.x > player_pos_x) { //block on the right
					curr->position.x = player_pos_x + player_half_size + (tile_size / 2.0f) + 3.0f;
				}
				if (curr->position.x < player_pos_x) {
					curr->position.x = player_pos_x - player_half_size - (tile_size / 2.0f) - 3.0f;
				}
			}
			else {
				if (curr->position.y > player_pos_y) { //block on the top
					curr->position.y = player_pos_y + player_half_size + (tile_size / 2.0f) + 3.0f;
				}				   			  
				if (curr->position.y < player_pos_y) {
					curr->position.y = player_pos_y - player_half_size - (tile_size / 2.0f) - 3.0f;
				}
			}

		}
		//else {
		//	dragging = false;
		//}
	}
}

void unload_puzzle_textures() {
	for (int i{}; i < ELEMENT_COUNT; ++i) {
		AEGfxTextureUnload(element_block_textures[i]);
	}
}