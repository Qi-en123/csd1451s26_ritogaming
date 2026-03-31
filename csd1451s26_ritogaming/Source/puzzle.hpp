/*!************************************************************************
\file:			puzzle.hpp
\author:		Jerod
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			This file contains all puzzle related functions. E.g load
				puzzle data from text file, render puzzles, puzzle collision,
				move/drag puzzle etc.
**************************************************************************/
#pragma once
#include "AEEngine.h"
#include <vector>
#include <string>

namespace Puzzle {
	// ------------------------------------------------------------------------
	// Puzzle constants
	// ------------------------------------------------------------------------ 
	constexpr f32 PUZZLE_BLOCK_SPEED = 60.0f;
	constexpr f32 PUZZLE_GRAB_RANGE = 15.0f;
	constexpr f32 WITHIN_PIT_THRESHOLD = 0.15f;

	// ------------------------------------------------------------------------
	// Array to store puzzle textures
	// ------------------------------------------------------------------------ 
	extern AEGfxTexture* puzzle_block_textures[];
	

#pragma region PUZZLE DATA STRUCTURES
	// ------------------------------------------------------------------------
	// enum for puzzle types
	// ------------------------------------------------------------------------ 
	enum PuzzleType {
		PUSHABLE, //0
		BUTTON_UP,
		PIT,
		SPIKE,
		BUTTON_DOWN,
		PIT_FILLED,
		PUZZLE_TYPE_COUNT
	};

	// ------------------------------------------------------------------------
	// enum for elements
	// ------------------------------------------------------------------------ 
	enum Element {
		WATER, //0
		FIRE,
		NO_ELEMENT
	};

	// ------------------------------------------------------------------------
	// enum for movement direction, used for Dragging
	// ------------------------------------------------------------------------ 
	enum Direction {
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	// ----------------------------------------------------------------------------
	//
	// struct to store vector of pairs containing: Button ID, List of coordinates
	// of all spikes paired to the Button ID.
	//
	// ---------------------------------------------------------------------------- 
	struct buttonTrigger {
		s32 id;
		std::vector<std::pair<s32, s32>> coords;
	};

	// ----------------------------------------------------------------------------
	//
	// puzzle stuct. Stores puzzle type, world space position, which character its
	// hidden from as well as: Element OR Button ID depending on its type.
	//
	// ---------------------------------------------------------------------------- 
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

		// ----------------------------------------------------------------------------
		//
		// Default constructor
		//
		// ---------------------------------------------------------------------------- 
		puzzle() : type{ PUZZLE_TYPE_COUNT },
			posx{ 0 }, posy{ 0 },
			hidden_from{ 0 },
			active{ true },
			element{ Puzzle::NO_ELEMENT } {}

		// ----------------------------------------------------------------------------
		//
		// Constructor 1: Constructs puzzles of non-button types (Everything else
		// except button)
		//
		// ---------------------------------------------------------------------------- 
		explicit puzzle(PuzzleType t, f32 x, f32 y, Element elem, s32 chara) :
			type{ t },
			posx{ x }, posy{ y },
			element{ elem },
			hidden_from{ chara },
			active{ true } {
		}

		// ----------------------------------------------------------------------------
		//
		// Constructor 2: Constructs button type puzzles.
		//
		// ---------------------------------------------------------------------------- 
		explicit puzzle(PuzzleType t, f32 x, f32 y, s32 btn_id, s32 chara) :
			type{ t },
			posx{ x }, posy{ y },
			button_id{ btn_id },
			hidden_from{ chara },
			active{ true } {
		}
	}; //end struct puzzle
#pragma endregion
	// ------------------------------------------------------------------------
	// Global puzzle data (shared across files)
	// ------------------------------------------------------------------------
		extern std::vector<puzzle> puzzle_blocks;
		extern std::vector<buttonTrigger> btns;

#pragma region LEVEL EDITOR HELPER FUNCTIONS
	// ----------------------------------------------------------------------------
	//
	// "Places" a puzzle on the maps grid by adding it to "puzzles" vec, initializes
	// with row, col based on grid, while other data members are defaulted based on
	// its constructors.
	//
	// ---------------------------------------------------------------------------- 
	void place_puzzle(s32 row, s32 col, PuzzleType type, std::vector<puzzle>& puzzles);

	// ----------------------------------------------------------------------------
	//
	// "Erases" a puzzle of the level editor map, removing it from "puzzles vec.
	//
	// ---------------------------------------------------------------------------- 
	void remove_puzzle(s32 row, s32 col, std::vector<puzzle>& puzzles);

	// ----------------------------------------------------------------------------
	//
	// Retrieves the current puzzle from the vec for its data members to be edited.
	//
	// ---------------------------------------------------------------------------- 
	puzzle* get_editing_puzzle(s32 row, s32 col, std::vector<puzzle>& puzzles);

	// ----------------------------------------------------------------------------
	//
	// "Pairs" spike to Button ID by adding its coordinates to the vec of coords
	// tied to the Button ID.
	//
	// ---------------------------------------------------------------------------- 
	void pair_spike_to_button(s32 row, s32 col, buttonTrigger& bt);

	// ----------------------------------------------------------------------------
	//
	// "Erases" spike tied to Button ID by removing its coordinates from the vec of
	// coords tied to the Button ID. (NEEDS FIXING)
	//
	// ---------------------------------------------------------------------------- 
	void remove_spike_button_pair(s32 row, s32 col, buttonTrigger& bt);

	// ----------------------------------------------------------------------------
	//
	// Returns true if spike if paired to current editing button. False otherwise.
	// This function is used to prevent erasing of spikes that are not paired to
	// this BUTTON ID.
	//
	// ---------------------------------------------------------------------------- 
	bool spike_is_paired(s32 row, s32 col, buttonTrigger const& bt);

	// ----------------------------------------------------------------------------
	//
	// Overwrites the text file for the current levels puzzle data file, with the
	// one in the editor.
	//
	// ---------------------------------------------------------------------------- 
	bool overwrite_puzzle_blocks(std::vector<puzzle> const& puzzles,
		std::vector<buttonTrigger>& bt,
		const std::string& filename);

	// ----------------------------------------------------------------------------
	//
	// Saves a new text file containing the levels puzzle data for a new level. The
	// new level will be added to the back, being the h
	//
	// ---------------------------------------------------------------------------- 
	bool save_puzzle_blocks(std::vector<puzzle> const& puzzles,
		std::vector<buttonTrigger>& bt,
		std::string& out_filename);

	// ----------------------------------------------------------------------------
	//
	// Loads puzzle data from an existing level's puzzle text file into the editor.
	// This is only used in the level editor.
	//
	// ---------------------------------------------------------------------------- 
	bool load_puzzle_blocks_to_editor(char const* filename, PuzzleType** puzzle_editor_map,
		std::vector<puzzle>& puzzles, std::vector<buttonTrigger>& bt);

	// ----------------------------------------------------------------------------
	//
	// Gets the next or previous elements in the Elements enum order for editing.
	//
	// ---------------------------------------------------------------------------- 
	Element previous_element(Element elem);
	Element next_element(Element elem);
#pragma endregion

#pragma region SETUP FUNCTIONS
	// ----------------------------------------------------------------------------
	//
	// Loads and stores textures into "puzzle_block_textures", based on how 
	// many types there are. Puzzle tiles are named: tile_1xxx,
	// from tile_1000 onwards.
	//
	// ---------------------------------------------------------------------------- 
	void load_puzzle_textures();

	// ----------------------------------------------------------------------------
	//
	// Unloads all the puzzle textures in "puzzle_block_textures" array.
	//
	// ---------------------------------------------------------------------------- 
	void unload_puzzle_textures();

	// ----------------------------------------------------------------------------
	//
	// Reads puzzle data from a text file, intializes puzzle objects and stores
	// button-trigger pairings.
	//
	// ---------------------------------------------------------------------------- 
	void load_puzzle_data(char const* filename);

	// ----------------------------------------------------------------------------
	//
	// Renders all the puzzle blocks, including tints for elemental blocks
	//
	// ---------------------------------------------------------------------------- 
	void render_puzzle_blocks();

#pragma endregion

#pragma region GAMEPLAY FUNCTIONS
	// ----------------------------------------------------------------------------
	//
	// Collision check: Player against puzzles. Used to check collision with active
	// character. (Knight or Wizard)
	//
	// ---------------------------------------------------------------------------- 
	bool player_collide_puzzles(f32 newX, f32 newY, f32 playerHalfSize, s32& puzzle_index);

	// ----------------------------------------------------------------------------
	//
	// Handles everything button related. Checks whether button is pressed or not,
	// changes its state (UP | DOWN), checks if all buttons of its corresponding
	// IDS are down, and also activates it triggers (the spikes) if so.
	//
	// ---------------------------------------------------------------------------- 
	void puzzle_button_manager();

	// ----------------------------------------------------------------------------
	//
	// Plays the audio for puzzle interactions. e.g button press, pushing, pulling
	// of graves etc.
	//
	// ---------------------------------------------------------------------------- 
	void puzzle_audio_update();

	// ----------------------------------------------------------------------------
	//
	// Moves the puzzle in the direction the player is walking (the side of the
	// puzzle that the player is collided with)
	//
	// ---------------------------------------------------------------------------- 
	void move_puzzle(s32 index, Direction direction, Element player_elem);

	// ----------------------------------------------------------------------------
	//
	// Allows player to "grab" puzzle and drag it, walking while dragging will
	// move the puzzle in any direction the player is moving.
	//
	// ---------------------------------------------------------------------------- 
	void drag_puzzle(f32 player_pos_x, f32 player_pos_y, f32 player_half_size,
		bool& dragging, s32 index, Element player_elem);
#pragma endregion
}