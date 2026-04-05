/*!************************************************************************
\file:			camera.cpp
\author:		Jerod Lim
\par DP email:	jerodyaowei.lim@digipen.edu

\brief:			This source file implements the camera structure and camera
				control functions. Manages a single camera instance that
				follows player. The camera deadzones are adjusted in this
				file.
**************************************************************************/

#include "camera.hpp"
#include "map.hpp"
#include "AEEngine.h"

namespace {
	// ------------------------------------------------------------------------
	// Camera deadzones, how far the player can move from the camera center
	// before the camera begins to follow
	// ------------------------------------------------------------------------ 
	constexpr f32 deadzone_x = 60.0f;
	constexpr f32 deadzone_y = 5.0f;
}

namespace Camera {
	camera cam{};
	f32 zoom{};

	camera init_cam(f32* player_x, f32* player_y) {
		AEVec2 cam_pos{};
		AEVec2Set(&cam_pos, *player_x, *player_y);

		//Initializes camera with same pos as player
		camera c = { cam_pos };
		return c;
	}

	void simulate_camera(camera* c, f32* player_x, f32* player_y) {

		//Get current players pos
		AEVec2 curr_player_pos;
		AEVec2Set(&curr_player_pos, *player_x, *player_y);

		//Checks if player is within the deadzone
		if (*player_x > c->pos.x + deadzone_x ||
			*player_x < c->pos.x - deadzone_x ||
			*player_y > c->pos.y + deadzone_y ||
			*player_y < c->pos.y - deadzone_y) {

			//Compute distance between cam and player
			f32 dist = AEVec2Distance(&curr_player_pos, &c->pos);

			//Exponential ease in, camera moves faster as distance increases
			constexpr f32 k = 0.005f;			//controls how fast speed increases
			constexpr f32 max_speed = 100.0f;	//speed cap
			f32 rel_speed = max_speed * (1.0f - expf(-k * dist));

			//Lerps toward the player, normalize then /7 to "smoothen" the movement
			AEVec2Lerp(&c->pos, &c->pos, &curr_player_pos,
				rel_speed / max_speed / 7.0f);
		}
	}
}

