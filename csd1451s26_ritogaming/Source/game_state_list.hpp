/*!************************************************************************
\file:						game_State_list.hpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This header Defines the enumeration of game states, including Level 1, Quit, 
and Restart, providing a central list of all possible game states.
**************************************************************************/

#pragma once

enum GS_STATES
{
    OPENING,
	MAINMENU,
    GS_LEVEL1,   // First level of the game
	GS_LEVELS,    // Level selection screen
    CREDIT,
    GS_LEVEL_EDITOR, // Level editor state
    GS_BOSS_ROOM,    // Boss room state
    GS_QUIT,         // Exits the game loop and terminates the system
    GS_RESTART,       // Restarts the current level
    GS_WIN,           // Win condition
    GS_LOSE           // Lose condition
};