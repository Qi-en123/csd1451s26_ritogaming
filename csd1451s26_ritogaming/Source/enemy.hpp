/*!************************************************************************
\file:          enemy.hpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\brief:         This header declares the enemy structures and public
                functions for enemy initialization, updating, rendering,
                and cleanup using AEEngine.
**************************************************************************/

#pragma once
#include <vector>
#include <memory>
#include "renderer.hpp"
#include "AEEngine.h"

enum class enemy_type {
	bat,
	dragon
};

class enemy_factory {

		enemy_type type;
	
		// stat
		const float max_health { 100 }; 
		float current_health;
		float damange;

		// start position of the enemy 
		f32 spawn_x, spawn_y;
		f32 speed; 
		f32 size;

		// State
		bool active;
		bool aggroed; // chasing player

		// collision 
		float collision_radius; // no no circle -> player can recieve or deal damage
		float sight_range;		// spotted and chase the player
		float reset_range;

		struct animation {
			std::vector<AEGfxTexture*> frames_path; 
			int current_frame; 
			float frame_timer;
			float frame_duration; 
			bool playing, loop, texture_loaded;

			animation() : 
				current_frame(), frame_timer(), frame_duration(),
				playing(true), loop(true), texture_loaded(false) {}

			void update(float delta_time);


		};

	public:
		enemy_factory();
		~enemy_factory();
};

