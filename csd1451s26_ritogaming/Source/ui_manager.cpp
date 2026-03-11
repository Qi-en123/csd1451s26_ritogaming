/*!************************************************************************
\file:          ui_manager.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
<<<<<<< Updated upstream
\brief:
**************************************************************************/
=======
\
\brief:         Implements UI rendering functions for enemy health bars
\               above enemies and player HUD displaying character health
\               hearts in the corner of the screen.
**************************************************************************/

>>>>>>> Stashed changes
#include "ui_manager.hpp"
#include "renderer.hpp"

UIManager ui_manager;

void load_ui_textures(UIManager* p_ui_manager)
{
	p_ui_manager->hp_empty_texture = AEGfxTextureLoad("Assets/UI/hp_empty.png");
	p_ui_manager->hp_half_texture = AEGfxTextureLoad("Assets/UI/hp_half.png");
	p_ui_manager->hp_full_texture = AEGfxTextureLoad("Assets/UI/hp_full.png");
}

void free_ui_textures(UIManager* p_ui_manager)
{
	if (p_ui_manager->hp_empty_texture) AEGfxTextureUnload(p_ui_manager->hp_empty_texture);
	if (p_ui_manager->hp_half_texture) AEGfxTextureUnload(p_ui_manager->hp_half_texture);
	if (p_ui_manager->hp_full_texture) AEGfxTextureUnload(p_ui_manager->hp_full_texture);

	p_ui_manager->hp_empty_texture = nullptr;
	p_ui_manager->hp_half_texture = nullptr;
	p_ui_manager->hp_full_texture = nullptr;
}

void render_enemy_hp(UIManager* p_ui_manager, EnemyManager* enemy_manager)
{
	for (auto const& enemy : enemy_manager->get_enemies())
	{
		if (!enemy || !enemy->is_active()) continue; // skip dead/inactive enemies
		int hearts_to_draw = enemy->get_max_health() / 2 + enemy->get_max_health() % 2;
		int full_hearts = enemy->get_current_health() / 2;
		bool has_half_heart = (enemy->get_current_health() % 2);

		float heart_scale = ENTITY_HP_SCALE * enemy->get_size();
		float canvas_width = hearts_to_draw * heart_scale;
		float start_x = enemy->get_x() - canvas_width / 2 + heart_scale / 2;

		for (int i{ 1 }; i < hearts_to_draw + 1; ++i)
		{
			f32 x_position = start_x + (i - 1) * heart_scale;
			f32 y_position = enemy->get_y() + enemy->get_size() / 2 + heart_scale / 2;
<<<<<<< Updated upstream

			if (i <= full_hearts)
				render_sprite(rect_mesh_center, x_position, y_position, heart_scale, heart_scale, ui_manager->hp_full_texture);

			else if (i == full_hearts + 1 && has_half_heart)
				render_sprite(rect_mesh_center, x_position, y_position, heart_scale, heart_scale, ui_manager->hp_half_texture);

			else
				render_sprite(rect_mesh_center, x_position, y_position, heart_scale, heart_scale, ui_manager->hp_empty_texture);

=======

			if (i <= full_hearts)
				render_sprite(rect_mesh_center, x_position, y_position, heart_scale, heart_scale, p_ui_manager->hp_full_texture);

			else if (i == full_hearts + 1 && has_half_heart)
				render_sprite(rect_mesh_center, x_position, y_position, heart_scale, heart_scale, p_ui_manager->hp_half_texture);

			else
				render_sprite(rect_mesh_center, x_position, y_position, heart_scale, heart_scale, p_ui_manager->hp_empty_texture);

>>>>>>> Stashed changes
		}
	}
}

<<<<<<< Updated upstream
void render_player_hud(UIManager* ui_manager, Player* player) {
=======
void render_player_hud(UIManager* p_ui_manager, Player* player) {
>>>>>>> Stashed changes
	int hearts_to_draw = player->stats.char1_max_health / 2 + player->stats.char1_max_health % 2;
	int full_hearts = player->stats.char1_curr_health / 2;
	bool has_half_heart = (player->stats.char1_curr_health % 2);

<<<<<<< Updated upstream
	float canvas_width = hearts_to_draw * HUD_HEART_SIZE;
=======
	//float canvas_width = hearts_to_draw * HUD_HEART_SIZE;
>>>>>>> Stashed changes
	float start_x = AEGfxGetWinMinX() + HUD_HEART_SIZE / 2;
	float y_position = AEGfxGetWinMaxY() - HUD_HEART_SIZE / 2;

	render_ui_sprite(rect_mesh_center, start_x, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, player->char1.texture);

	for (int i{ 1 }; i < hearts_to_draw + 1; ++i)
	{
		f32 x_position = start_x + i * HUD_HEART_SIZE;

		if (i <= full_hearts) {
			render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, p_ui_manager->hp_full_texture);
		}
		else if (i == full_hearts + 1 && has_half_heart) {
			render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, p_ui_manager->hp_half_texture);
		}
		else {
			render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, p_ui_manager->hp_empty_texture);
		}
	}

	hearts_to_draw = player->stats.char2_max_health / 2 + player->stats.char2_max_health % 2;
	full_hearts = player->stats.char2_curr_health / 2;
	has_half_heart = (player->stats.char2_curr_health % 2);

	y_position = y_position - HUD_HEART_SIZE;

	render_ui_sprite(rect_mesh_center, start_x, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, player->char2.texture);

	for (int i{ 1 }; i < hearts_to_draw + 1; ++i)
	{
		f32 x_position = start_x + i * HUD_HEART_SIZE;
		if (i <= full_hearts) {
			render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, p_ui_manager->hp_full_texture);
		}
		else if (i == full_hearts + 1 && has_half_heart) {
			render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, p_ui_manager->hp_half_texture);
		}
		else {
			render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, p_ui_manager->hp_empty_texture);
		}
	}
}