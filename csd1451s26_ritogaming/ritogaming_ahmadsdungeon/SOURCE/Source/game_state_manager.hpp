/*!************************************************************************
\file:			game_state_manager.hpp
\author:		Goh Qi En
\par DP email:	g.qien@digipen.edu

\brief:			Declares the Game State Manager (GSM) interface. Provides
                the mechanism for transitioning between different game
                states (e.g., Menu, Level 1, Game Over) using a system of
                global state trackers and function pointers.
**************************************************************************/

#pragma once
#include "AETypes.h"

// ----------------------------------------------------------------------------
//
// Typedef: fp
// A standard function pointer definition for state-specific lifecycle 
// functions (Load, Initialize, Update, Draw, Free, Unload).
//
// ---------------------------------------------------------------------------- 
typedef void(*fp)(void);

// ----------------------------------------------------------------------------
//
// Global State Trackers (Externs)
// Used to manage the flow of the state machine throughout the application.
//
// ---------------------------------------------------------------------------- 
extern s32 current;   // The state currently being processed in the loop
extern s32 previous;  // The state that was active before the current one
extern s32 next;      // The target state to transition to after the current loop

// ----------------------------------------------------------------------------
//
// Function Pointers (Externs)
// These are dynamically assigned based on the 'current' state to point 
// to specific logic (e.g., Level1_Load, Level1_Update, etc.).
//
// ---------------------------------------------------------------------------- 
extern fp fpLoad;       // Called once when entering a state to load assets
extern fp fpInitialize; // Called once when entering a state to set variables
extern fp fpUpdate;     // Called every frame to process logic
extern fp fpDraw;       // Called every frame to render graphics
extern fp fpFree;       // Called once when leaving a state to reset variables
extern fp fpUnload;     // Called once when leaving a state to free assets

// ----------------------------------------------------------------------------
//
// Function: gsm_initialize
// Sets the initial starting state of the game and resets state trackers.
//
// ---------------------------------------------------------------------------- 
void gsm_initialize(s32 startingState);

// ----------------------------------------------------------------------------
//
// Function: gsm_update
// Evaluates the 'current' state and updates the function pointers (fpLoad, 
// fpUpdate, etc.) to match the logic required for that specific state.
//
// ---------------------------------------------------------------------------- 
void gsm_update();