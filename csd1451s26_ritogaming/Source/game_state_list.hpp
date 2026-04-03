/*!************************************************************************
\file:						game_state_list.hpp
\author:					Goh Qi En
\par DP email:				g.qien@digipen.edu
\brief:						This header defines the enumeration of game
                            states used by the Game State Manager (GSM).
                            It provides a centralized list of all possible
                            states, such as levels, menus, and system states.
**************************************************************************/

#pragma once

// ----------------------------------------------------------------------------
//
// Enum: GS_STATES
// Defines unique identifiers for every possible state in the game.
// These are used by the GSM to switch function pointers and manage 
// the game loop flow.
//
// ----------------------------------------------------------------------------
enum GS_STATES
{
    OPENING,            // The initial state displaying the DigiPen logo
    MAINMENU,           // The primary navigation hub for the player
    GS_LEVEL1,          // The first playable stage of the game
    GS_LEVELS,          // The screen used for selecting different levels
    CREDIT,             // Displays the development team and attributions
    GS_GUIDE,           // The tutorial or instructional level
    GS_LEVEL_EDITOR,    // A state dedicated to internal level creation tools
    GS_BOSS_ROOM,       // The final encounter or second level of the game
    GS_QUIT,            // Signal to exit the game loop and terminate the application
    GS_RESTART,         // Special state to reload the current active level
    GS_WIN,             // The end screen displayed upon victory
    GS_LOSE             // The end screen displayed upon defeat
};