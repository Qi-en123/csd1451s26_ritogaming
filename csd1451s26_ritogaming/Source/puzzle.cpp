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
#include "audio_manager.hpp"

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <Windows.h>

//textures for all puzzle types (elements are tints)
AEGfxTexture* puzzle_block_textures[PUZZLE_TYPE_COUNT]{};

//size of puzzle, based off tile size
static f32 puzzle_size;

//to store indexes of btns and pushables
std::vector<int> button_indexes{}, pushable_indexes{};

//to store all puzzle blocks
std::vector<puzzle> puzzle_blocks;

std::vector<buttonTrigger> btns{};

static bool is_drag_playing = false;
static bool drag_was_active = false;
extern AudioManager g_audio;

#pragma region LEVEL EDITOR HELPER FUNCTIONS

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

void initialize_puzzle_blocks(char const* filename) {
	printf("%f\n", tile_size);
	puzzle_size = tile_size - 8.0f;

	std::ifstream ifs{ filename };
	if (!ifs.is_open()) {
		printf("Failed to open: %s\n", filename);
		//terminate
	}

	//Reads first line, gets number of puzzle blocks in level
	std::string line;
	s32 num_puzzle_blocks;
	ifs >> line >> num_puzzle_blocks;

	//idk y nid but the map uses
	f32 startX = -1600.f / 2.f;
	f32 startY = 900.f / 2.f;

	puzzle_blocks.clear();
	button_indexes.clear();
	pushable_indexes.clear();

	//Reads and stores values to init puzzle blocks
	for (int i{}; i < num_puzzle_blocks; ++i) {
		std::string type;
		s32 row, col, chara;
		ifs >> type >> row >> col >> chara;

		f32 posx = startX + col * tile_size;
		f32 posy = startY - row * tile_size;
		printf("type:%s row:%d col:%d -> posx:%f posy:%f\n", type.c_str(), row, col, posx, posy);
		//buttons have no elems (by design)
		if (type == "BUTTON") {
			s32 id;
			ifs >> id;

			puzzle tmp{ BUTTON_UP, posx, posy, id, chara };
			puzzle_blocks.push_back(tmp);
			button_indexes.push_back(i);

			std::cout << type << ' ' << row << ' ' << col << ' ' << id << '\n';
		}

		//every other puzzle type may have elems
		else {
			s32 elem;
			ifs >> elem;

			Element e{ NO_ELEMENT };
			if (elem >= 0 && elem <= NO_ELEMENT) {
				e = static_cast<Element>(elem);
			}
		
			if (type == "PUSHABLE") {

				puzzle tmp{ PUSHABLE, posx, posy, e, chara };
				puzzle_blocks.push_back(tmp);
				pushable_indexes.push_back(i);

				std::cout << type << ' ' << row << ' ' << col << ' ' << elem << '\n';
			}

			else if (type == "PIT"){

				puzzle tmp{ PIT,posx,posy,e,chara };
				puzzle_blocks.push_back(tmp);
			}

			else if (type == "SPIKE") {
				
				puzzle tmp{ SPIKE,posx,posy,e,chara };
				puzzle_blocks.push_back(tmp);
			}
		}
	}

	//Now reading button triggers
	s32 triggers;
	ifs >> line >> triggers;
	//std::cout << line << triggers << '\n';
	//skip nl
	ifs.ignore();

	//NOT SAFE 
	//number of triggers
	btns.clear();
	for (int i{}; i < triggers; ++i) {
		int id, row, col;
		char ch;

		std::getline(ifs, line);
		std::stringstream ss(line);
		
		//get button id
		ss >> id;
		btns.push_back({ id,{} });
		buttonTrigger& back = btns.back();

		while (ss >> ch) {
			if (ch == '(') {
				ss >> row >> ch >> col >> ch;
				back.coords.push_back({row, col});
				//printf("%d, %d\n", row, col);
			}
		}
	}

	//printf("%d %d", btns.size(), button_indexes.size());
	ifs.close();
}

//u gotta reserve the vec
//cuz if i click n drag
//its gon reallocate so many times n lag
void place_puzzle(s32 row, s32 col, PuzzleType type, std::vector<puzzle>& puzzles) {
	if (type == PUZZLE_TYPE_COUNT) return;

	f32 startX = -1600.f / 2.f;
	f32 startY = 900.f / 2.f;

	f32 posx = startX + col * tile_size;
	f32 posy = startY - row * tile_size;

	switch (type) {
	case PUSHABLE:
		puzzles.emplace_back(PUSHABLE, posx, posy, NO_ELEMENT, 0);
		break;

	case SPIKE:
		puzzles.emplace_back(SPIKE, posx, posy, NO_ELEMENT, 0);
		break;

	case BUTTON_UP:
	case BUTTON_DOWN:
		puzzles.emplace_back(BUTTON_UP, posx, posy, -1, 0);
		break;

	case PIT:
	case PIT_FILLED:
		puzzles.emplace_back(PIT, posx, posy, NO_ELEMENT, 0);
		break;

	default:
		break;
	}
}

void remove_puzzle(s32 row, s32 col, std::vector<puzzle>& puzzles) {
	if (puzzles.empty()) return;

	f32 startX = -1600.f / 2.f;
	f32 startY = 900.f / 2.f;

	f32 posx = startX + col * tile_size;
	f32 posy = startY - row * tile_size;

	for (s32 i{}; i < puzzles.size(); ++i) {

		if (posx == puzzles[i].posx && posy == puzzles[i].posy) {

			puzzles[i] = puzzles.back();
			puzzles.pop_back();
			return;
		}

	}
}

puzzle* get_editing_puzzle(s32 row, s32 col, std::vector<puzzle>& puzzles) {

	//printf("%d, %d\n", row, col);

	f32 startX = -1600.f / 2.f;
	f32 startY = 900.f / 2.f;
	
	f32 posx = startX + col * tile_size;
	f32 posy = startY - row * tile_size;

	//printf("Current: %f, %f\n", posx, posy);

	for (puzzle& p : puzzles) {
		//printf("Check with: %f, %f\n", p.posx, p.posy);

		if (posx == p.posx && posy == p.posy) {
			return &p;
		}
	}

	return nullptr;
}

void pair_spike_to_button(s32 row, s32 col, buttonTrigger& bt) {

	bt.coords.push_back({ row,col });
}

void remove_spike_button_pair(s32 row, s32 col, buttonTrigger& bt) {

	for (std::pair<int, int>& c : bt.coords) {
		if (c.first == row && c.second == col) {
			c = bt.coords.back();
			bt.coords.pop_back();
			return;
		}
	}
}

bool spike_is_paired(s32 row, s32 col, buttonTrigger const& bt) {

	for (std::pair<int, int> const& c : bt.coords) {
		if (c.first == row && c.second == col) {
			return true;
		}
	}
	return false;

}

static s32 convert_to_col(f32 x) {
	f32 startX = -800.0f;
	return static_cast<s32>((x - startX) / tile_size);
}

static s32 convert_to_row(f32 y) {
	f32 startY = 450.0f;
	return static_cast<s32>((startY - y) / tile_size);
}

bool overwrite_puzzle_blocks(std::vector<puzzle> const& puzzles,
	std::vector<buttonTrigger> const& bt,
	const std::string& filename) {

	std::ofstream file{ filename };

	if (!file.is_open())
	{
		std::cerr << "Failed to open map file for overwrite: " << filename << "\n";
		return false;
	}

	file << "Puzzles: " << puzzles.size() << "\n\n";

	//Write buttons to text
	//abit inefficient but am looping thrice to print in order
	//so i can render in order
	for (const puzzle& p : puzzles) {
		if (p.type == BUTTON_UP || p.type == BUTTON_DOWN) {
			//I need a world to grid helper fnc to conver pos.x to rows etc.
			file << "BUTTON\t "
				<< convert_to_row(p.posy) << ' '
				<< convert_to_col(p.posx) << ' '
				<< p.hidden_from << ' ' << p.button_id << '\n';
		}
	}
	//Write Pit 2nd last
	for (const puzzle& p : puzzles) {
		if (p.type == PIT || p.type == PIT_FILLED) {
			file << "PIT\t "
				<< convert_to_row(p.posy) << ' '
				<< convert_to_col(p.posx) << ' '
				<< p.hidden_from << ' ' << p.element << '\n';
		}
	}

	for (const puzzle& p : puzzles) {
		if (p.type == SPIKE) {
			file << "SPIKE\t"
				<< convert_to_row(p.posy) << ' '
				<< convert_to_col(p.posx) << ' '
				<< p.hidden_from << ' ' << p.element << '\n';
		}
	}

	//Save pushable last
	for (const puzzle& p : puzzles) {
		if (p.type == PUSHABLE) {
			file << "PUSHABLE "
				<< convert_to_row(p.posy) << ' '
				<< convert_to_col(p.posx) << ' '
				<< p.hidden_from << ' ' << p.element << '\n';
		}
	}


	//save triggers here
	file << "\nTriggers: " << bt.size();
	for (buttonTrigger const& b : bt) {
		//print id
		file << '\n' << b.id << ": ";

		//print coords
		for (std::pair<int, int> const& c : b.coords) {
			file << '(' << c.first << ',' << c.second << ')';
		}
	}

	file.close();
	return true;
}

bool save_puzzle_blocks(std::vector<puzzle> const& puzzles,
	std::vector<buttonTrigger> const& bt,
	std::string& out_filename) {
	printf("tile_size at save: %f\n", tile_size);
	// Find highest existing lvl number and go one above it
	s32 highest = 0;
	WIN32_FIND_DATAA find_data;
	HANDLE hFind = FindFirstFileA("Assets/Levels/l*_puzzles.txt", &find_data);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			std::string fname = find_data.cFileName; // file name
			size_t start = fname.find("l") + 1; // position of the number 
			size_t end = fname.find("_puzzles.txt"); //position of the end of number
			if (start != std::string::npos && end != std::string::npos) // ensure both number and .txt exist
			{
				int n = std::stoi(fname.substr(start, end - start)); // extract the number from the filename
				if (n > highest) highest = n; // update highest if this file's number is greater
			}
		} while (FindNextFileA(hFind, &find_data));
		FindClose(hFind);
	}

	std::string filename = "Assets/Levels/l" + std::to_string(highest + 1) + "_puzzles.txt";
	out_filename = filename; // return the filename used

	//filename now represents next available level
	//std::cout << filename << '\n';
	std::ofstream ofs{ filename };
	if (!ofs)
	{
		std::cerr << "Failed to open puzzle map file for writing: " << filename << "\n";
		return false;
	}

	ofs << "Puzzles: " << puzzles.size() << "\n\n";

	//Write buttons to text
	//abit inefficient but am looping thrice to print in order
	//so i can render in order
	for (const puzzle& p : puzzles) {
		if (p.type == BUTTON_UP || p.type == BUTTON_DOWN) {
			//I need a world to grid helper fnc to conver pos.x to rows etc.
			ofs << "BUTTON\t " 
				<< convert_to_row(p.posy) << ' '
				<< convert_to_col(p.posx) << ' '
				<< p.hidden_from << ' ' << p.button_id << '\n';
		}
	}
	//Write Pit 2nd last
	for (const puzzle& p : puzzles) {
		if (p.type == PIT || p.type == PIT_FILLED) {
			ofs << "PIT\t "
				<< convert_to_row(p.posy) << ' '
				<< convert_to_col(p.posx) << ' '
				<< p.hidden_from << ' ' << p.element << '\n';
		}
	}

	for (const puzzle& p : puzzles) {
		if (p.type == SPIKE) {
			ofs << "SPIKE\t"
				<< convert_to_row(p.posy) << ' '
				<< convert_to_col(p.posx) << ' '
				<< p.hidden_from << ' ' << p.element << '\n';
		}
	}

	//Save pushable last
	for (const puzzle& p : puzzles) {
		if (p.type == PUSHABLE) {
			ofs << "PUSHABLE "
				<< convert_to_row(p.posy) << ' '
				<< convert_to_col(p.posx) << ' '
				<< p.hidden_from << ' ' << p.element << '\n';
		}
	}


	//save triggers here
	ofs << "\nTriggers: " << bt.size();
	for (buttonTrigger const& b : bt) {
		//print id
		ofs << '\n' << b.id << ": ";

		//print coords
		for (std::pair<int, int> const& c : b.coords) {
			ofs << '(' << c.first << ',' << c.second << ')';
		}
	}

	ofs.close();
	return true;
}

#pragma endregion


void render_puzzle_blocks() {
	for (size_t i{}; i < puzzle_blocks.size(); ++i) {
		puzzle& curr = puzzle_blocks[i];

		//skips inactive puzzles
		if (!curr.active) continue;

		//only renders for characters based on who this is "hidden_from"
		if (curr.hidden_from == player_instance.current_character) continue;
		
		color clr{ white };

		//for now, only pushables have elements
		if (curr.type == PUSHABLE) {
			switch (curr.element)
			{
			case WATER:
				clr = { 0.0f,0.0f,3.0f,1.0f };
				break;
			//case EARTH:
			//	clr = { 0.0f,0.0f,0.0f,1.0f };
			//	break;
			case FIRE:
				clr = { 3.0f,0.0f,0.0f,1.0f };
				break;
			//case AIR:
			//	clr = { 3.0f,3.0f,3.0f,1.0f };
			//	break;
			default:
				break;
			}
		}

		render_sprite(rect_mesh_center, curr.posx,
			curr.posy, puzzle_size, puzzle_size, puzzle_block_textures[curr.type], clr);
	}
}

bool player_collide_puzzles(f32 newX, f32 newY, f32 playerHalfSize, s32& puzzle_index)
{
	for (size_t i{}; i < puzzle_blocks.size(); ++i)
	{
		puzzle& curr = puzzle_blocks[i];
		//skips inactive blocks
		if (!curr.active) continue;

		if (ss_collision(newX, newY, playerHalfSize, playerHalfSize,
			curr.posx, curr.posy, puzzle_size / 2.0f, puzzle_size / 2.0f))
		{
			//no collision against these types
			if (curr.type == BUTTON_UP ||
				curr.type == BUTTON_DOWN ||
				curr.type == PIT_FILLED) {

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

void check_button_pressed() {
	f32& p1_x = player_instance.char1.x;
	f32& p1_y = player_instance.char1.y;
	f32& p2_x = player_instance.char2.x;
	f32& p2_y = player_instance.char2.y;
	f32 phs = player_instance.stats.character_size / 2.0f;

	for (int i{}; i < button_indexes.size(); ++i) {
		puzzle& curr = puzzle_blocks[button_indexes[i]];
		
		//skips inactive buttons (although all buttons shld be always active)
		if (!curr.active) continue;
		
		//don't allow button to activate for certain characters
		if (curr.hidden_from == player_instance.current_character) continue;

		bool pressed{ false };

		//Check if either players are standing on button (both p1 & p2)
		if (ss_collision(p1_x, p1_y, phs, phs,
			curr.posx, curr.posy, puzzle_size / 2.0f, puzzle_size / 2.0f) ||
			ss_collision(p2_x, p2_y, phs, phs,
				curr.posx, curr.posy, puzzle_size / 2.0f, puzzle_size / 2.0f))
		{
			pressed = true;
		}

		//Check if any pushables are on top of button
		for (int j{}; j < pushable_indexes.size(); ++j) {
			puzzle& pushable = puzzle_blocks[pushable_indexes[j]];
			//skips inactive pushables
			if (!pushable.active) continue;

			if (ss_collision(pushable.posx, pushable.posy, puzzle_size / 2.0f, puzzle_size / 2.0f,
				curr.posx, curr.posy, puzzle_size / 2.0f, puzzle_size / 2.0f))
			{
				pressed = true;
				break;
			}
		}

		//Set button to down if pressed by anything
		curr.type = pressed ? BUTTON_DOWN : BUTTON_UP;
	}
}

void puzzle_button_manager() {
	check_button_pressed();

	//goes through each btn id
	for (int j{}; j < btns.size(); ++j) {
		bool triggered{ true };

		//goes through each btn
		for (int i{}; i < button_indexes.size(); ++i) {
			puzzle& curr = puzzle_blocks[button_indexes[i]];

			//skips inactive buttons
			if (!curr.active) continue;

			//if triggered
			if (curr.button_id == btns[j].id) {
				if (curr.type != BUTTON_DOWN) triggered = false;
			}
		}

		if (triggered) {
			//goes through all coords
			for (std::pair<int,int> pos : btns[j].coords) {

				//printf("%d, %d\n", pos.first, pos.second);

				puzzle* spike = get_editing_puzzle(pos.first, pos.second, puzzle_blocks);
				spike->active = false;
			}
		}
	}
}

//make this private for nao
bool puzzle_collide_puzzles(s32 index, f32 pos_x, f32 pos_y) {
	f32 hts = tile_size / 2.0f;

	for (int i{}; i < puzzle_blocks.size(); ++i) {
		if (i == index) continue; //skips itself

		puzzle& curr = puzzle_blocks[i];

		//skips inactive puzzles
		if (!curr.active) continue;

		//button triggers done in another function to prevent confusion
		switch (curr.type) {

		case BUTTON_DOWN:
		case BUTTON_UP:
		case PIT_FILLED:
			continue;
			break;

		case PIT:
			//check if pos.x and pos.y is inside the pit enough
			//threshold represents how overlapped both axis must be to be considered "in"
			if (fabsf(curr.posx - pos_x) / (puzzle_size + puzzle_size) < WITHIN_PIT_THRESHOLD &&
				fabsf(curr.posy - pos_y) / (puzzle_size + puzzle_size) < WITHIN_PIT_THRESHOLD) {

				//"remove" pushable, its now in the pit :(
				puzzle_blocks[index].active = false;
				//change type to filled
				curr.type = PIT_FILLED;
			}

			break;

		case PUSHABLE:
		case SPIKE:
			//check collision against other puzzles
			if (ss_collision(curr.posx, curr.posy, hts, hts,
				pos_x, pos_y, hts, hts)) {
				return true;
			}
			break;

		default:
			break;
		}
	}
	return false;
}

void puzzle_audio_update() {
	if (!drag_was_active && is_drag_playing) {
		// Neither move nor drag fired this frame — stop the sound
		AEAudioStopGroup(g_audio.sfx_group);
		is_drag_playing = false;
	}
	// Reset for next frame
	drag_was_active = false;
}


void move_puzzle(s32 index, Direction direction, Element player_elem) {
	puzzle& curr = puzzle_blocks[index];

	//printf("%d", index);
	if (curr.hidden_from == player_instance.current_character) return;
	if (curr.type != PUSHABLE) return;
	if (curr.element != NO_ELEMENT && curr.element != player_elem) return;

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

	//Check if puzzle if blocked by: Wall | Puzzle | Player
	if (!puzzle_to_wall_collision(curr.posx + dx, curr.posy + dy) &&
		!puzzle_collide_puzzles(index, curr.posx + dx, curr.posy + dy) &&
		!puzzle_to_inactive_collision(curr.posx + dx, curr.posy + dy)) {
		curr.posx += dx;
		curr.posy += dy;

		if (!is_drag_playing) {
			play_sfx(g_audio.sfx_drag_pull, 0.03f, 1.0f, -1);
			is_drag_playing = true;
		}
		drag_was_active = true;
	}
}

//based on player movement speed, should change player to use dt
//calls when "Drag" key is pressed
void drag_puzzle(f32 player_pos_x, f32 player_pos_y, f32 player_half_size,
	bool& dragging, s32 index, Element player_elem) {

	puzzle& puzzle = puzzle_blocks[index];

	if (puzzle.type != PUSHABLE) return;
	if (puzzle.element != NO_ELEMENT && puzzle.element != player_elem) return;

	//x & y axis distances between player and puzzle
	f32 dx{ player_pos_x - puzzle.posx };
	f32 dy{ player_pos_y - puzzle.posy };

	//prevent puzzle going inside wall
	const f32 min_gap{ 3.0f };
	const f32 grab_range{ (player_half_size + puzzle_size / 2.0f) + PUZZLE_GRAB_RANGE };

	//printf("%f, %f\n", (dx * dx + dy * dy),(grab_range * grab_range));
	//printf("%d, %d\n", puzzle.element, player_elem);

	if ((dx * dx + dy * dy) < (grab_range * grab_range)) {
		//printf("is dragging");
		dragging = true;
		const f32 min_dist = player_half_size + (puzzle_size / 2.0f) + min_gap;
		f32 newX{ puzzle.posx }, newY{ puzzle.posy };

		//checks if player is top/bottom or left/right using x/y overlap distance
		if ((player_half_size + puzzle_size / 2.0f) - fabsf(dx) <
			(player_half_size + puzzle_size / 2.0f) - fabsf(dy)) {

			//comes here if x overlap is greater (player is left/right)
			newX = (puzzle.posx > player_pos_x) ?
				player_pos_x + min_dist : player_pos_x - min_dist;
		}
		else {
			newY = (puzzle.posy > player_pos_y) ?
				player_pos_y + min_dist : player_pos_y - min_dist;
		}

		if (!puzzle_to_wall_collision(newX, newY) &&
			!puzzle_collide_puzzles(index, newX, newY)) {
			puzzle.posx = newX;
			puzzle.posy = newY;
			if (!is_drag_playing) {
				play_sfx(g_audio.sfx_drag_pull, 0.03f, 1.0f, -1);
				is_drag_playing = true;
			}
			drag_was_active = true;
		}
	}
}

void unload_puzzle_textures() {
	for (int i{}; i < PUZZLE_TYPE_COUNT; ++i) {
		AEGfxTextureUnload(puzzle_block_textures[i]);
	}
}

//water(0), fire(1)
Element previous_element(Element elem) {
	switch (elem) {
	case WATER:
		return NO_ELEMENT;
	case FIRE:
		return WATER;
	case NO_ELEMENT:
		return FIRE;
	}
}

Element next_element(Element elem) {
	switch (elem) {
	case WATER:
		return FIRE;
	case FIRE:
		return NO_ELEMENT;
	case NO_ELEMENT:
		return WATER;
	}
}