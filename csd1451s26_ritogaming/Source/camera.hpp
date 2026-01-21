/*!************************************************************************
\file:			camera.hpp
\author:		Jerod
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			This header declares the camera structure and camera
				control functions.
**************************************************************************/
#pragma once
#include "AEEngine.h"

typedef struct camera {
	AEVec2 pos;
	//how far the player can move before the camera moves along
	f32 limit_x, limit_y;
}camera;

extern camera cam;

camera init_cam(f32* player_x, f32* player_y, f32 limit_x, f32 limit_y);

void simulate_camera(camera* cam, f32* player_x, f32* player_y,
	f32 left_bound, f32 right_bound, f32 bottom_bound, f32 top_bound);