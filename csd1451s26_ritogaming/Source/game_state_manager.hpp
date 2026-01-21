/*!************************************************************************
\file:          game_state_manager.hpp
\author:        Qi En
\par DP email:  g.qien@digipen.edu
\brief:         Declares the Game State Manager interface, including function pointers for loading, 
initializing, updating, drawing, freeing, and unloading game states, 
and functions to initialize and update the game state manager
as well as global variables to keep track of current, previous and next game states
**************************************************************************/

#pragma once

//type definition for function pointer
typedef void(*fp)(void);


extern int current, previous, next;


extern fp fpLoad, fpInitialize, fpUpdate, fpDraw, fpFree, fpUnload;


void gsm_initialize(int startingState);


void gsm_update();