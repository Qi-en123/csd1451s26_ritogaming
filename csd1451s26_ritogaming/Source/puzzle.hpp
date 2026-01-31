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

#define PUZZLE_BLOCK_SPEED 40.0f
#define PUZZLE_GRAB_RANGE 15.0f

extern AEGfxTexture* puzzle_block_textures[];

//MORE ENUMSS I LOVE ENUMS, GIVE ME MORE
enum PuzzleType {
	PUSHABLE, //0
	BUTTON_UP,
	BUTTON_DOWN,
	//ELEMENT_BOARD, //this one is hit with element to trigger (dk wat to call this)
	PUZZLE_TYPE_COUNT
};

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
//struct element_block {
//	Element element{};
//	AEGfxTexture* texture{};
//	AEVec2 position{};
//
//	element_block(Element e, f32 pos_x, f32 pos_y, AEGfxTexture* tex)
//		: element{ e }, position{ pos_x,pos_y }, 
//		texture{ tex } {}
//};

//type, pos, is_elem, element
struct puzzle {
	PuzzleType type{}; //push, elemen, 
	AEVec2 position{};
	bool is_elemental{};

	//might put inside union
	Element element{};
};

void load_puzzle_textures();

void intialize_puzzle_blocks();

void render_puzzle_blocks();

//void check_puzzle_collision(player* player);
bool player_collide_puzzles(f32 newX, f32 newY, f32 playerHalfSize, s32& puzzle_index);

//alot of collisions everywhere sry (dk how else to do it :p)
void check_button_collision(f32 pos_x, f32 pos_y, f32 playerHalfSize);

void move_puzzle(s32 index, Direction direction, Element player_elem);

void drag_puzzle(f32 player_pos_x, f32 player_pos_y, f32 player_half_size,
	bool& dragging, s32 index, Element player_elem);

void unload_puzzle_textures();