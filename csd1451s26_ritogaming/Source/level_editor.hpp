/*!************************************************************************
\file:          level_editor.hpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         Declares functions to load, initialize, update, draw, free, and unload
				the game's level editor
**************************************************************************/

#pragma once

enum EditorState {
	EDIT_TILES,
	EDIT_PUZZLES
};

void level_editor_load();
void level_editor_init();
void level_editor_update();
void level_editor_draw();
void level_editor_free();
void level_editor_unload();