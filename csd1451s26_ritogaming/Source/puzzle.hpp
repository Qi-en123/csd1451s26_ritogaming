/*!************************************************************************
\file:			puzzle.hpp
\author:		Jerod
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			awd
**************************************************************************/
#pragma once
#include "AEEngine.h"
#include <vector>

constexpr f32 PUZZLE_BLOCK_SPEED = 40.0f;
constexpr f32 PUZZLE_GRAB_RANGE = 15.0f;
constexpr f32 WITHIN_PIT_THRESHOLD = 0.15f;

extern AEGfxTexture* puzzle_block_textures[];

enum PuzzleType {
	PUSHABLE, //0
	BUTTON_UP,
	PIT,
	SPIKE,
	BUTTON_DOWN,
	PIT_FILLED,
	PUZZLE_TYPE_COUNT
};

enum Element {
	WATER, //0
	FIRE,
	NO_ELEMENT
};

enum Direction {
	LEFT,
	RIGHT,
	UP,
	DOWN
};

//store vec of pairs containing:
//button_id
//vector of spike/s coordinates to de/activate
struct buttonTrigger {
	s32 id;
	std::vector<std::pair<int, int>> coords;
};

//type, pos, is_elem, element
struct puzzle {
	PuzzleType type{}; //push, elemen, 
	f32 posx{}, posy{};	
	s32 hidden_from{}; //0,1,2
	bool active{};

	//might put inside union
	union {
		Element element; //used for non buttons/non pits
		s32 button_id; //used for BUTTON_UP||BUTTON_DOWN puzzles
	};

	//constructor for if not elem or btn
	explicit puzzle(PuzzleType t, f32 x, f32 y, Element elem, s32 chara) :
		type{ t }, posx{ x }, posy{ y }, element{ elem }, hidden_from{ chara }, active{ true } {
	}

	explicit puzzle(PuzzleType t, f32 x, f32 y, s32 btn_id, s32 chara) :
		type{ t }, posx{ x }, posy{ y }, button_id{ btn_id }, hidden_from{ chara }, active{ true } {
	}
};

/**************************************************************************
	\brief
	Loads puzzle textures

	Loads and stores textures into "puzzle_block_textures", based on how 
	many types there are "PUZZLE_TYPE_COUNT". Puzzle tiles are named:
	tile_1xxx, from tile_1000 onwards.
**************************************************************************/
void load_puzzle_textures();

void initialize_puzzle_blocks(char const* filename);

void place_puzzle(s32 row, s32 col, PuzzleType type, std::vector<puzzle>& puzzles);

void remove_puzzle(s32 row, s32 col, std::vector<puzzle>& puzzles);

puzzle* get_editing_puzzle(s32 row, s32 col, std::vector<puzzle>& puzzles);

void pair_spike_to_button(s32 row, s32 col, buttonTrigger& bt);

void remove_spike_button_pair(s32 row, s32 col, buttonTrigger& bt);

bool spike_is_paired(s32 row, s32 col, buttonTrigger const& bt);

bool save_puzzle_blocks(std::vector<puzzle> const& puzzles,
	std::vector<buttonTrigger> const& bt);

void render_puzzle_blocks();

//void check_puzzle_collision(player* player);
bool player_collide_puzzles(f32 newX, f32 newY, f32 playerHalfSize, s32& puzzle_index);

//alot of collisions everywhere sry (dk how else to do it :p)
//void check_button_collision(f32 p1_x, f32 p1_y, f32 p2_x, f32 p2_y, f32 playerHalfSize);

void puzzle_button_manager(); //open to rename suggestions (txt me)

void move_puzzle(s32 index, Direction direction, Element player_elem);

void drag_puzzle(f32 player_pos_x, f32 player_pos_y, f32 player_half_size,
	bool& dragging, s32 index, Element player_elem);

void unload_puzzle_textures();

Element previous_element(Element elem);
Element next_element(Element elem);