/*!************************************************************************
\file:			camera.cpp
\author:		Jerod
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			This file handles camera initialization and player-following
				behavior with boundary constraints using AEEngine.
**************************************************************************/
#include "camera.hpp"
#include "AEEngine.h"

camera init_cam(f32* player_x, f32* player_y, f32 limit_x, f32 limit_y) {
	AEVec2 cam_pos{};
	AEVec2Set(&cam_pos, *player_x, *player_y);
	f32 cam_limit_x = limit_x;
	f32 cam_limit_y = limit_y;

	camera c = { cam_pos, cam_limit_x, cam_limit_y };
	return c;
}

void simulate_camera(camera* c, f32* player_x, f32* player_y,
	f32 left_bound, f32 right_bound, f32 bottom_bound, f32 top_bound) {
	AEVec2 curr_player_pos;
	AEVec2Set(&curr_player_pos, *player_x, *player_y);

	if (*player_x > c->pos.x + c->limit_x ||
		*player_x < c->pos.x - c->limit_x ||
		*player_y > c->pos.y + c->limit_y ||
		*player_y < c->pos.y - c->limit_y) {

		float dist = AEVec2Distance(&curr_player_pos, &c->pos);
		float k = 0.005f;
		float max = 100.0f;

		float rel_speed = max * (1.0f - expf(-k * dist));

		AEVec2Lerp(&c->pos, &c->pos, &curr_player_pos,
			rel_speed/max/7.0f);
	}

	c->pos.x = AEClamp(c->pos.x, left_bound, right_bound);
	c->pos.y = AEClamp(c->pos.y, bottom_bound, top_bound);
}