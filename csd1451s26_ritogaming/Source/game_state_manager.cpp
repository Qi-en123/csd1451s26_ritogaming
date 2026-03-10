/*!************************************************************************
\file:          game_state_manager.cpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         This file implements the Game State Manager (GSM), which controls which game state is active and
				which functions are executed at any time during the game.
**************************************************************************/

#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "level1.hpp"
#include "level2.hpp"
#include "credit.hpp"
#include "opening.hpp"
#include "mainmenu.hpp"
#include "level_editor.hpp"
#include "end_screen.hpp"
#include "levels.hpp"

// ----------------------------------------------------------------------------
// Current, previous, and next game state identifiers
// Used by the Game State Manager (GSM) to track and switch between states
int current = 0;   // The currently active game state
int previous = 0;  // The previously active game state 
int next = 0;      // The next game state to switch to

// ----------------------------------------------------------------------------
// Function pointers for the current game state
// Each pointer corresponds to a function for the active state
// Initialized to nullptr to indicate no function is assigned yet
fp fpLoad = nullptr;        // Function to load assets for the current game state
fp fpInitialize = nullptr;  // Function to initialize the game state after loading
fp fpUpdate = nullptr;      // Function to update game logic per frame
fp fpDraw = nullptr;        // Function to render/draw the current game state per frame
fp fpFree = nullptr;        // Function to free runtime resources when exiting the game state
fp fpUnload = nullptr;      // Function to unload assets after the game state ends

// ----------------------------------------------------------------------------
// Initialize the Game State Manager (GSM)
// startingState: the game state to start with (e.g., GS_LEVEL1)
// Sets current, previous, and next states to the starting state
void gsm_initialize(int startingState)
{
	current = previous = next = startingState;
}

void gsm_update()
{
	// Assign function pointers based on the current state
	switch (current)
	{
	case GS_LEVEL1:
		fpLoad = level1_load;
		fpInitialize = level1_init;
		fpUpdate = level1_update;
		fpDraw = level1_draw;
		fpFree = level1_free;
		fpUnload = level1_unload;
		break;

	case GS_BOSS_ROOM:
		fpLoad = level2_load;
		fpInitialize = level2_init;
		fpUpdate = level2_update;
		fpDraw = level2_draw;
		fpFree = level2_free;
		fpUnload = level2_unload;
		break;
	case CREDIT:
		fpLoad = credit_load;
		fpInitialize = credit_init;
		fpUpdate = credit_update;
		fpDraw = credit_draw;
		fpFree = credit_free;
		fpUnload = credit_unload;
		break;
	case OPENING:
		fpLoad = opening_load;
		fpInitialize = opening_init;
		fpUpdate = opening_update;
		fpDraw = opening_draw;
		fpFree = opening_free;
		fpUnload = opening_unload;
		break;
	case MAINMENU:
		fpLoad = mainmenu_load;
		fpInitialize = mainmenu_init;
		fpUpdate = mainmenu_update;
		fpDraw = mainmenu_draw;
		fpFree = mainmenu_free;
		fpUnload = mainmenu_unload;
		break;
	case GS_LEVEL_EDITOR:
		fpLoad = level_editor_load;
		fpInitialize = level_editor_init;
		fpUpdate = level_editor_update;
		fpDraw = level_editor_draw;
		fpFree = level_editor_free;
		fpUnload = level_editor_unload;
		break;
	case GS_LEVELS:
		fpLoad = levels_load;
		fpInitialize = levels_init;
		fpUpdate = levels_update;
		fpDraw = levels_draw;
		fpFree = levels_free;
		fpUnload = levels_unload;
		break;
	case GS_RESTART:
		break;
	case GS_WIN:
	case GS_LOSE:
		fpLoad = end_screen_load;
		fpInitialize = end_screen_init;
		fpUpdate = end_screen_update;
		fpDraw = end_screen_draw;
		fpFree = end_screen_free;
		fpUnload = end_screen_unload;
		break;
	case GS_QUIT:
		break;

	default:
		break;
	}
}