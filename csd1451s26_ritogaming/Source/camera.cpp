/*!************************************************************************
\file:			camera.cpp
\author:		Jerod
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			This file handles camera initialization and player-following
				behavior with boundary constraints using AEEngine.
**************************************************************************/
#include "camera.hpp"
#include "AEEngine.h"

#define CAMERA_SPEED 0.04f

Camera initCam(f32* player_x, f32* player_y, f32 limit_x, f32 limit_y) {
	AEVec2 cam_pos{};
	AEVec2Set(&cam_pos, *player_x, *player_y);
	f32 cam_limit_x = limit_x;
	f32 cam_limit_y = limit_y;

	Camera cam = { cam_pos, cam_limit_x, cam_limit_y };
	return cam;
}

void simulateCamera(Camera* cam, f32* player_x, f32* player_y,
	f32 left_bound, f32 right_bound, f32 bottom_bound, f32 top_bound) {
	AEVec2 curr_player_pos;
	AEVec2Set(&curr_player_pos, *player_x, *player_y);

	if (*player_x > cam->pos.x + cam->limit_x ||
		*player_x < cam->pos.x - cam->limit_x ||
		*player_y > cam->pos.y + cam->limit_y ||
		*player_y < cam->pos.y - cam->limit_y) {
		AEVec2Lerp(&cam->pos, &cam->pos, &curr_player_pos, CAMERA_SPEED);
	}

	cam->pos.x = AEClamp(cam->pos.x, left_bound, right_bound);
	cam->pos.y = AEClamp(cam->pos.y, bottom_bound, top_bound);

	AEGfxSetCamPosition(cam->pos.x, cam->pos.y);
}