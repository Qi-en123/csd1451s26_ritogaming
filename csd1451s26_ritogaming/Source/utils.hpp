/*!************************************************************************
\file:			utils.hpp
\author:		Goh Qi En
\par DP email:	g.qien@digipen.edu

\brief:			Header file containing utility functions for
				UI interaction, popup rendering, and global
				level management state.
**************************************************************************/

#pragma once
#include "AEEngine.h"
#include <string>
#include <vector>

// ----------------------------------------------------------------------------
//
// Global Variables (Externs)
// Shared state for timing and level progression used across systems.
//
// ---------------------------------------------------------------------------- 
extern f32 dt;                                  // Global delta time
extern std::vector<std::string> level_paths;     // List of standard level file paths
extern std::vector<std::string> level_puzzle_paths; // List of puzzle level file paths

extern int current_level_index;                 // The index of the active level
extern std::string selected_level_path;         // The file path of the chosen standard level
extern std::string selected_level_puzzle_path;  // The file path of the chosen puzzle level

// ----------------------------------------------------------------------------
//
// Function: IsButtonClicked
// Checks if the mouse is currently clicking within the bounds of a rectangle.
//
// ---------------------------------------------------------------------------- 
int IsButtonClicked(f32 rectX, f32 rectY, f32 rectW, f32 rectH);

// ----------------------------------------------------------------------------
//
// Function: IsButtonHovered
// Checks if the mouse cursor is currently positioned over a rectangle.
//
// ---------------------------------------------------------------------------- 
int IsButtonHovered(f32 rectX, f32 rectY, f32 rectW, f32 rectH);

// ----------------------------------------------------------------------------
//
// Function: draw_confirm_popup
// Renders a UI confirmation dialog with "Yes" and "No" interactive buttons.
// Returns 1 for "Yes", 0 for "No", and -1 if no selection is made.
//
// ---------------------------------------------------------------------------- 
int draw_confirm_popup(AEGfxTexture* panel,
	AEGfxTexture* btn_yes, AEGfxTexture* btn_no,
	s8 font, const char* message, f32 font_scale = 1.f);