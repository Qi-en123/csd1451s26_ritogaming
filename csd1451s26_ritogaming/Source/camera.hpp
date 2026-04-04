/*!************************************************************************
\file:			camera.hpp
\author:		Jerod Lim
\par DP email:	jerodyaowei.lim@digipen.edu

\brief:			This header declares the camera structure and camera
				control functions. Manages a single camera instance that
				follows player.
**************************************************************************/

#pragma once
#include "AEEngine.h"

namespace Camera {
	// ------------------------------------------------------------------------
	// Camera state, stores current position in world space
	// ------------------------------------------------------------------------ 
	struct camera {
		AEVec2 pos;
	};

	extern camera cam;	//active camera instance
	extern f32 zoom;	//current zoom level

	// ------------------------------------------------------------------------
	// 
	// Initializes the camera centered on the player's starting position.
	// 
	// ------------------------------------------------------------------------ 
	camera init_cam(f32* player_x, f32* player_y);

	// ------------------------------------------------------------------------
	// 
	// Updates camera position each frame, follows the player outside deadzone,
	// clamps position within world bounds. It follows with speed proportional
	// to distance from player
	// 
	// ------------------------------------------------------------------------ 
	void simulate_camera(camera* cam, f32* player_x, f32* player_y);
}
