/*!************************************************************************
\file:			puzzle.hpp
\author:		Jerod
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			awd
**************************************************************************/
#pragma once
#include "AEEngine.h"
#include "player.hpp"

//first puzzle "elements", if element block is hit, open door

extern AEGfxTexture* element_block_textures[];

enum Element {
	WATER, //0
	EARTH,
	FIRE,
	AIR,
	ELEMENT_COUNT
};

enum Direction {
	LEFT,
	RIGHT,
	UP,
	DOWN
};

//default this to the get hit one first
struct element_block {
	Element element{};
	AEGfxTexture* texture{};
	AEVec2 position{};

	element_block(Element e, f32 pos_x, f32 pos_y, AEGfxTexture* tex)
		: element{ e }, position{ pos_x,pos_y }, 
		texture{ tex } {}
};

void load_puzzle_textures();

void intialize_puzzle_blocks();

void render_puzzle_blocks();

//void check_puzzle_collision(player* player);
bool player_collide_puzzles(f32 newX, f32 newY, f32 playerHalfSize, s32& puzzle_index);

void move_puzzle(s32 index, Direction direction);
void drag_puzzle(f32 player_pos_x, f32 player_pos_y, f32 player_half_size, bool& dragging);

void unload_puzzle_textures();