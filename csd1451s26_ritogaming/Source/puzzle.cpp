/*!************************************************************************
\file:			puzzle.cpp
\author:		Jerod
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			All these comments are for me paiseh, how to use is in
				puzzle.hpp (go read that one)
**************************************************************************/
#include "puzzle.hpp"
#include "AEEngine.h"
#include "renderer.hpp"
#include "collision.hpp"
#include "map.hpp"
#include "utils.hpp"
#include <vector>

AEGfxTexture* puzzle_block_textures[PUZZLE_TYPE_COUNT]{};

static f32 puzzle_size;
int num_puzzle_blocks{ 8 }; //make sure ints below add up to this no.
std::vector<int> button_indexes{}, pushable_indexes{}; //to store indexes of btns and pushables (more efficient? idk js trying)

std::vector<puzzle> puzzle_blocks;

//vec of pairs to store btn_id and whether triggered or not
static std::vector<std::pair<int, void(*)(int,int)>> btns{}; //wanna use btn struct instead of pair, to store door row,col

void apple() { map[10][5] = OPENED_DOOR; } //js testing

void load_puzzle_textures() {
	char filename[128];
	for (int i{}; i < PUZZLE_TYPE_COUNT; ++i) {
		//I named by puzzle texture tiles to start with 1, so tile_100x
		sprintf_s(filename, "Assets/Tiles/tile_1%03d.png", i);
		puzzle_block_textures[i] = AEGfxTextureLoad(filename);
		if (!puzzle_block_textures[i])
			printf("Failed to load: %s\n", filename);
	}
}

//in the future, this will read from text file to determine puzzle block inits
//for nao, hard code just to test features
void intialize_puzzle_blocks() { //in the future will have char const* of text file name
	puzzle_size = tile_size - 8.0f;
	puzzle_blocks.reserve(num_puzzle_blocks);

	//Simulating for loop vv
	//--------------- bottom left buttons ---------------//
	puzzle tmp{ BUTTON_UP, {-672.0f, -768.0f}, false, 1 };
	puzzle_blocks.push_back(tmp);
	//this is supposed to be called if(type==button_up || type==button_down)
	button_indexes.push_back(0); //this will be changed to 'i'

	tmp = { BUTTON_UP, {-224.0f, -768.0f}, false, 1};
	puzzle_blocks.push_back(tmp);
	button_indexes.push_back(1);

	//--------------- bottom pushable ---------------//
	tmp = { PUSHABLE, {-100.0f, -895.0f}, true, WATER};
	puzzle_blocks.push_back(tmp);
	pushable_indexes.push_back(2);

	//--------------- top left buttons ---------------//
	tmp = { BUTTON_UP, {-224.0f, 256.0f}, false, 2 };
	puzzle_blocks.push_back(tmp);
	button_indexes.push_back(3);

	tmp = { BUTTON_UP, {-672.0f, 256.0f}, false, 2 };
	puzzle_blocks.push_back(tmp);
	button_indexes.push_back(4);

	tmp = { BUTTON_UP, {-448.0f, 256.0f}, false, 2 };
	puzzle_blocks.push_back(tmp);
	button_indexes.push_back(5);

	//--------------- top pushable ---------------//
	tmp = { PUSHABLE, {-672.0f, 128.0f}, false };
	puzzle_blocks.push_back(tmp);
	pushable_indexes.push_back(6);

	//--------------- LAST pushable ---------------//
	tmp = { PUSHABLE, {704.0f, 2.0f}, true, FIRE };
	puzzle_blocks.push_back(tmp);
	pushable_indexes.push_back(7);

	//Saving button ids for btn manager to use
	//for each btns
	for (int i{}; i < button_indexes.size(); ++i) {
		puzzle& curr = puzzle_blocks[button_indexes[i]];

		//if btns_id exists, dont push_back
		bool exists{ false };
		for (int j{}; j < btns.size(); ++j) {
			if (curr.button_id == btns[j].first) exists = true;
		}

		//if id is unique, add it to vec
		//not sure how to init the different functions tho
		if (!exists) btns.push_back(std::pair<int, void(*)(int, int)>{curr.button_id, open_door});
	}
}

void render_puzzle_blocks() {
	for (int i{}; i < num_puzzle_blocks; ++i) {
		puzzle& curr = puzzle_blocks[i];

		color clr{ white };
		if (curr.is_elemental) {
			switch (curr.element)
			{
			case WATER:
				clr = { 0.5f,0.5f,1.0f,1.0f };
				break;
			case EARTH:
				clr = { 0.5f,0.5f,0.5f,1.0f };
				break;
			case FIRE:
				clr = { 1.0f,0.5f,0.5f,1.0f };
				break;
			case AIR:
				clr = { 2.0f,2.0f,2.0f,1.0f };
				break;
			default:
				break;
			}
		}
		render_sprite(rect_mesh_center, curr.position.x,
			curr.position.y, puzzle_size, puzzle_size, puzzle_block_textures[curr.type], clr);
	}
}

bool player_collide_puzzles(f32 newX, f32 newY, f32 playerHalfSize, s32& puzzle_index)
{
	for (int i{}; i < num_puzzle_blocks; ++i)
	{
		puzzle& curr = puzzle_blocks[i];
		if (ss_collision(newX, newY, playerHalfSize, playerHalfSize,
			curr.position.x, curr.position.y, puzzle_size / 2.0f, puzzle_size / 2.0f))
		{
			if (curr.type == BUTTON_UP || curr.type == BUTTON_DOWN) {
				continue;
			}

			// Debug: print tile coordinates
			//printf("Colliding with puzzle at (%.2f, %.2f)\n", e_block.position.x, e_block.position.y);
			puzzle_index = i;
			return true; // collision detected
		}
	}
	return false; // no collision
}

void check_button_collision(f32 p1_x, f32 p1_y, f32 p2_x, f32 p2_y, f32 playerHalfSize) {
	for (int i{}; i < button_indexes.size(); ++i) {
		puzzle& curr = puzzle_blocks[button_indexes[i]];
		bool pressed{ false };

		//Players collision (both p1 & p2)
		if (ss_collision(p1_x, p1_y, playerHalfSize, playerHalfSize,
			curr.position.x, curr.position.y, puzzle_size / 2.0f, puzzle_size / 2.0f) ||
			ss_collision(p2_x, p2_y, playerHalfSize, playerHalfSize,
				curr.position.x, curr.position.y, puzzle_size / 2.0f, puzzle_size / 2.0f))
		{
			pressed = true;
		}

		//Puzzle collision
		for (int j{}; j < pushable_indexes.size(); ++j) {
			puzzle& pushable = puzzle_blocks[pushable_indexes[j]];

			if (ss_collision(pushable.position.x, pushable.position.y, puzzle_size / 2.0f, puzzle_size / 2.0f,
				curr.position.x, curr.position.y, puzzle_size / 2.0f, puzzle_size / 2.0f))
			{
				curr.type = BUTTON_DOWN;
				printf("supposed to be collided");
				break;
			}
			else {
				curr.type = BUTTON_UP;
			}
		}

		curr.type = pressed ? BUTTON_DOWN : BUTTON_UP;
	}
}

void puzzle_button_manager() {
	//goes through each btn id
	for (int j{}; j < btns.size(); ++j) {
		bool triggered{ true };
		//goes through each btn
		for (int i{}; i < button_indexes.size(); ++i) {
			puzzle& curr = puzzle_blocks[button_indexes[i]];

			//if triggered
			if (curr.button_id == btns[j].first) {
				if (curr.type != BUTTON_DOWN) triggered = false;
			}
		}

		if (triggered) {
			//call trigger function here
			//e.g. open_door() assuming its like void open_door()
			//btns[j].second(); //dis calls the trigger fnc

			if (btns[j].first == 1) { //bottom left
				for (int c = 4; c <= 7; ++c) {
					map[18][c] = FLOOR;
				}
			}

			if (btns[j].first == 2) { //top left
				map[7][10] = FLOOR;
			}
			generate_walls_from_map(map, tile_size);
		}
	}
}

//make this private for nao
bool puzzle_on_puzzle_collision(s32 index, f32 pos_x, f32 pos_y) {
	f32 hts = tile_size / 2.0f;

	for (int i{}; i < puzzle_blocks.size(); ++i) {
		if (i == index) continue; //skips itself

		puzzle& curr = puzzle_blocks[i];

		//button triggers done in another function to prevent confusion
		if (curr.type == BUTTON_DOWN || curr.type == BUTTON_UP) continue; 

		if (ss_collision(curr.position.x, curr.position.y, hts, hts,
			pos_x, pos_y, hts, hts)) {
			return true;
		}
	}
	return false;
}

void move_puzzle(s32 index, Direction direction, Element player_elem) {
	puzzle& puzzle = puzzle_blocks[index];

	if (puzzle.type != PUSHABLE) return;
	if (puzzle.is_elemental && puzzle.element != player_elem) return;

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

	if (!puzzle_to_wall_collision(puzzle.position.x + dx, puzzle.position.y + dy)
		&& !puzzle_on_puzzle_collision(index, puzzle.position.x + dx, puzzle.position.y + dy)) {
		puzzle.position.x += dx;
		puzzle.position.y += dy;
	}
}

//based on player movement speed, should change player to use dt
//calls when "Drag" key is pressed
void drag_puzzle(f32 player_pos_x, f32 player_pos_y, f32 player_half_size,
	bool& dragging, s32 index, Element player_elem) {

	puzzle& puzzle = puzzle_blocks[index];

	if (puzzle.type != PUSHABLE) return;
	if (puzzle.is_elemental && puzzle.element != player_elem) return;

	f32 dx{ player_pos_x - puzzle.position.x };
	f32 dy{ player_pos_y - puzzle.position.y };

	const f32 min_gap{ 3.0f }; //prevent puzzle going inside wall
	const f32 grab_range{ (player_half_size + puzzle_size / 2.0f) + PUZZLE_GRAB_RANGE };

	if ((dx * dx + dy * dy) < (grab_range * grab_range) &&
		puzzle.element == player_elem) {
		//printf("is dragging");
		dragging = true;
		const f32 min_dist = player_half_size + (puzzle_size / 2.0f) + min_gap;
		f32 newX{ puzzle.position.x }, newY{ puzzle.position.y };

		//checks if player is top/bottom or left/right using x/y overlap distance
		if ((player_half_size + puzzle_size / 2.0f) - fabsf(dx) <
			(player_half_size + puzzle_size / 2.0f) - fabsf(dy)) {

			newX = (puzzle.position.x > player_pos_x) ?
				player_pos_x + min_dist : player_pos_x - min_dist;
		}
		else {
			newY = (puzzle.position.y > player_pos_y) ?
				player_pos_y + min_dist : player_pos_y - min_dist;
		}

		if (!puzzle_to_wall_collision(newX, newY) &&
			!puzzle_on_puzzle_collision(index, newX, newY)) {
			puzzle.position.x = newX;
			puzzle.position.y = newY;
		}

	}
	else {
		dragging = false;
	}
}

void unload_puzzle_textures() {
	for (int i{}; i < PUZZLE_TYPE_COUNT; ++i) {
		AEGfxTextureUnload(puzzle_block_textures[i]);
	}
}