#pragma once
#include "AEEngine.h"

typedef struct Camera {
	AEVec2 pos;
	//how far the player can move before the camera moves along
	f32 limit_x, limit_y;
}Camera;

Camera initCam(f32* player_x, f32* player_y, f32 limit_x, f32 limit_y);

void simulateCamera(Camera* cam, f32* player_x, f32* player_y,
	f32 left_bound, f32 right_bound, f32 bottom_bound, f32 top_bound);