/*!************************************************************************
\file:          credit.hpp
\author:        Goh Qi En
\par DP email:  g.qien@digipen.edu
\brief:         Declares functions to load, initialize, update, draw, free, and unload
			    the game's credits. Handles scrolling text with adjustable speed,
			    perspective scaling, and automatic reset when the credits finish.
**************************************************************************/

#pragma once

void credit_load();
void credit_initialize();
void credit_update();
void credit_draw();
void credit_free();
void credit_unload();