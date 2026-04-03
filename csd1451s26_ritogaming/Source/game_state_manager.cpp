/*!************************************************************************
\file:          game_state_manager.cpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         This file implements the Game State Manager (GSM), which
				controls which game state is active and which functions are
				executed at any time during the game.
**************************************************************************/

#include "game_state_manager.hpp"
#include "game_state_list.hpp"
#include "level1.hpp"
#include "boss_room.hpp"
#include "credit.hpp"
#include "opening.hpp"
#include "mainmenu.hpp"
#include "level_editor.hpp"
#include "end_screen.hpp"
#include "levels.hpp"
#include "guide.hpp"

// ----------------------------------------------------------------------------
//
// Global State Identifiers
// Tracks the progression and flow of game states.
//
// ----------------------------------------------------------------------------
int current = 0;   // The currently active game state
int previous = 0;  // The previously active game state 
int next = 0;      // The next game state to switch to

// ----------------------------------------------------------------------------
//
// Function Pointers (Lifecycle Hooks)
// These pointers are reassigned based on the active state to execute 
// the correct logic for loading, updating, drawing, and cleaning up.
//
// ----------------------------------------------------------------------------
fp fpLoad = nullptr;        // Function to load assets for the current game state
fp fpInitialize = nullptr;  // Function to initialize the game state after loading
fp fpUpdate = nullptr;      // Function to update game logic per frame
fp fpDraw = nullptr;        // Function to render/draw the current game state per frame
fp fpFree = nullptr;        // Function to free runtime resources when exiting the game state
fp fpUnload = nullptr;      // Function to unload assets after the game state ends

// ----------------------------------------------------------------------------
//
// Function: gsm_initialize
// Sets the initial state of the game manager. Usually called once at 
// the start of the application.
//
// ----------------------------------------------------------------------------
void gsm_initialize(int startingState)
{
	current = previous = next = startingState;
}

// ----------------------------------------------------------------------------
//
// Function: gsm_update
// Reassigns the global function pointers based on the 'current' state 
// identifier. This ensures the main game loop calls the correct 
// functions for whichever level or menu is active.
//
// ----------------------------------------------------------------------------
void gsm_update()
{
	// Assign function pointers based on the current state
	switch (current)
	{
	case GS_LEVEL1:					//level 1
		fpLoad = level1_load;
		fpInitialize = level1_init;
		fpUpdate = level1_update;
		fpDraw = level1_draw;
		fpFree = level1_free;
		fpUnload = level1_unload;
		break;

	case GS_BOSS_ROOM:				//boss room 
		fpLoad = boss_room_load;
		fpInitialize = boss_room_init;
		fpUpdate = boss_room_update;
		fpDraw = boss_room_draw;
		fpFree = boss_room_free;
		fpUnload = boss_room_unload;
		break;

	case CREDIT:					//credit screen
		fpLoad = credit_load;
		fpInitialize = credit_init;
		fpUpdate = credit_update;
		fpDraw = credit_draw;
		fpFree = credit_free;
		fpUnload = credit_unload;
		break;

	case GS_GUIDE:					//guide/tutorial level
		fpLoad = guide_load;
		fpInitialize = guide_init;
		fpUpdate = guide_update;
		fpDraw = guide_draw;
		fpFree = guide_free;
		fpUnload = guide_unload;
		break;

	case OPENING:					//opening logo sequence
		fpLoad = opening_load;
		fpInitialize = opening_init;
		fpUpdate = opening_update;
		fpDraw = opening_draw;
		fpFree = opening_free;
		fpUnload = opening_unload;
		break;

	case MAINMENU:					//main menu
		fpLoad = mainmenu_load;
		fpInitialize = mainmenu_init;
		fpUpdate = mainmenu_update;
		fpDraw = mainmenu_draw;
		fpFree = mainmenu_free;
		fpUnload = mainmenu_unload;
		break;

	case GS_LEVEL_EDITOR:			//internal level editor (not accessible from main menu)
		fpLoad = level_editor_load;
		fpInitialize = level_editor_init;
		fpUpdate = level_editor_update;
		fpDraw = level_editor_draw;
		fpFree = level_editor_free;
		fpUnload = level_editor_unload;
		break;

	case GS_LEVELS:					//level select screen
		fpLoad = levels_load;
		fpInitialize = levels_init;
		fpUpdate = levels_update;
		fpDraw = levels_draw;
		fpFree = levels_free;
		fpUnload = levels_unload;
		break;

	case GS_RESTART:				//restart
		// Logic handled within the Game State Loop in main
		break;

	case GS_WIN:					// win screen
	case GS_LOSE:					// lose screen
		fpLoad = end_screen_load;
		fpInitialize = end_screen_init;
		fpUpdate = end_screen_update;
		fpDraw = end_screen_draw;
		fpFree = end_screen_free;
		fpUnload = end_screen_unload;
		break;

	case GS_QUIT:					// quit
		// Logic handled within the Game State Loop in main
		break;

	default:
		break;
	}
}