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
		fpInitialize = level1_initialize;
		fpUpdate = level1_update;
		fpDraw = level1_draw;
		fpFree = level1_free;
		fpUnload = level1_unload;
		break;

	case GS_BOSS_ROOM:
		fpLoad = level2_load;
		fpInitialize = level2_initialize;
		fpUpdate = level2_update;
		fpDraw = level2_draw;
		fpFree = level2_free;
		fpUnload = level2_unload;
		break;
	case CREDIT:
		fpLoad = credit_load;
		fpInitialize = credit_initialize;
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
	case GS_RESTART:
		break;

	case GS_QUIT:
		break;

	default:
		break;
	}
}