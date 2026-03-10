	#include "ui_manager.hpp"
	#include "renderer.hpp"

	UIManager ui_manager;

	void load_ui_textures(UIManager* ui_manager)
	{
		ui_manager->hp_empty_texture = AEGfxTextureLoad("Assets/UI/hp_empty.png");
		ui_manager->hp_half_texture = AEGfxTextureLoad("Assets/UI/hp_half.png");
		ui_manager->hp_full_texture = AEGfxTextureLoad("Assets/UI/hp_full.png");
		ui_manager->primary_bg_texture = AEGfxTextureLoad("Assets/Projectile/tile_0078.png");
		ui_manager->primary_counter_texture = AEGfxTextureLoad("Assets/Projectile/tile_0079.png");
		ui_manager->primary_ability_texture = AEGfxTextureLoad("Assets/Projectile/fireball1.png");
		ui_manager->cursor_knight = AEGfxTextureLoad("Assets/Admin/cursorSword_silver.png");
		ui_manager->cursor_wizard = AEGfxTextureLoad("Assets/Admin/wizard_wand_nonactive1.png");
		ui_manager->cursor_wizard_active = AEGfxTextureLoad("Assets/Admin/wizard_wand_active1.png");
		ui_manager->font = AEGfxCreateFont("Assets/Exo2-Regular.ttf", 48);
	}

	void free_ui_textures(UIManager* ui_manager)
	{
		if (ui_manager->hp_empty_texture) AEGfxTextureUnload(ui_manager->hp_empty_texture);
		if (ui_manager->hp_half_texture) AEGfxTextureUnload(ui_manager->hp_half_texture);
		if (ui_manager->hp_full_texture) AEGfxTextureUnload(ui_manager->hp_full_texture);
		if (ui_manager->primary_bg_texture) AEGfxTextureUnload(ui_manager->primary_bg_texture);
		if (ui_manager->primary_ability_texture) AEGfxTextureUnload(ui_manager->primary_ability_texture);
		if (ui_manager->primary_counter_texture) AEGfxTextureUnload(ui_manager->primary_counter_texture);
		if (ui_manager->cursor_knight)        AEGfxTextureUnload(ui_manager->cursor_knight);
		if (ui_manager->cursor_wizard)        AEGfxTextureUnload(ui_manager->cursor_wizard);
		if (ui_manager->cursor_wizard_active) AEGfxTextureUnload(ui_manager->cursor_wizard_active);
		if (ui_manager->font != -1) AEGfxDestroyFont(ui_manager->font);

	}

	void render_character_ui(UIManager* ui_manager, Player* player)
	{
		if (player->current_player == &player->char2)
		{
			float icon_size = HUD_HEART_SIZE * 2.f;
			float paddingX = 10.f;
			float paddingY = 20.f;
			float x = AEGfxGetWinMinX() + paddingX + icon_size / 2.f; // half size keeps it fully on screen
			float y = AEGfxGetWinMinY() + paddingY + icon_size / 2.f;
			render_ui_sprite(rect_mesh_center, x, y, icon_size, icon_size, ui_manager->primary_bg_texture);
			render_ui_sprite(rect_mesh_center, x, y, icon_size /2.f, icon_size/2.f	, ui_manager->primary_ability_texture);

			float counter_x = x + (paddingX * 10.f);
			render_ui_sprite(rect_mesh_center, counter_x, y, icon_size / 1.5f, icon_size / 1.5f, ui_manager->primary_counter_texture);

			char fireball_text[8];
			sprintf_s(fireball_text, "%d", player->fireball_count);
			float text_w = 0.f, text_h = 0.f;
			AEGfxGetPrintSize(ui_manager->font, fireball_text, 1.f, &text_w, &text_h);

			// convert counter sprite center to normalized coords, then offset by half text size
			float norm_x = (counter_x / AEGfxGetWinMaxX()) - text_w / 2.f;
			float norm_y = (y / AEGfxGetWinMaxY()) - text_h / 2.f;

			AEGfxPrint(ui_manager->font, fireball_text, norm_x, norm_y, 1.f, 1.f, 1.f, 1.f, 1.f);
		}
	}

	void render_cursor(UIManager* ui_manager, Player* player, s32 mouse_x, s32 mouse_y, bool cursor_active)
	{
		f32 world_x = (f32)mouse_x - 800.f;
		f32 world_y = -(f32)mouse_y + 450.f;

		AEGfxTexture* current_cursor = nullptr;
		if (player->current_player == &player->char1)
			current_cursor = ui_manager->cursor_knight;
		else
			current_cursor = cursor_active ? ui_manager->cursor_wizard_active : ui_manager->cursor_wizard;

		if (current_cursor)
			render_blending_sprite(current_cursor, 50.f, 50.f, 1.0f, world_x, world_y);
	}

	void render_enemy_hp(UIManager* ui_manager, EnemyManager* enemy_manager)
	{
		for (auto const& enemy : enemy_manager->get_enemies())
		{
			AEGfxTexture* texture_to_draw = nullptr;
			int hearts_to_draw = enemy->get_max_health()/2 + enemy->get_max_health() % 2;
			int full_hearts = enemy->get_current_health() / 2;
			bool has_half_heart = (enemy->get_current_health() % 2);

			float heart_scale = ENTITY_HP_SCALE * enemy->get_size();
			float canvas_width = hearts_to_draw * heart_scale;
			float start_x = enemy->get_x() - canvas_width / 2 + heart_scale/2;
			//for (int i{1}; i < hearts_to_draw+1; ++i)
			//{
			//	f32 x_position = start_x + i * heart_scale;
			//	f32 y_position = enemy->get_y() + enemy->get_size() + heart_scale/2;
			//	if (i<=full_hearts) {
			//		render_sprite(rect_mesh_center, x_position, y_position, heart_scale, heart_scale, ui_manager->hp_full_texture);
			//	}
			//	else if(has_half_heart){
			//		render_sprite(rect_mesh_center, x_position, y_position, heart_scale, heart_scale, ui_manager->hp_half_texture);
			//	}
			//	else {
			//		render_sprite(rect_mesh_center, x_position, y_position, heart_scale, heart_scale, ui_manager->hp_empty_texture);
			//	}
			//}
			for (int i{ 1 }; i < hearts_to_draw + 1; ++i)
			{
				f32 x_position = start_x + i * heart_scale;
				f32 y_position = enemy->get_y() + enemy->get_size() + heart_scale / 2;

				// i=1 is leftmost, fill from left ? right
				if (i <= full_hearts) 
					render_sprite(rect_mesh_center, x_position, y_position, heart_scale, heart_scale, ui_manager->hp_full_texture);
			
				else if (i == full_hearts + 1 && has_half_heart) 
					render_sprite(rect_mesh_center, x_position, y_position, heart_scale, heart_scale, ui_manager->hp_half_texture);
			
				else 
					render_sprite(rect_mesh_center, x_position, y_position, heart_scale, heart_scale, ui_manager->hp_empty_texture);
			
			}
		}
	}

	void render_player_hud(UIManager* ui_manager, Player* player) {
		int hearts_to_draw = player->stats.char1_max_health/ 2 + player->stats.char1_max_health % 2;
		int full_hearts = player->stats.char1_curr_health / 2;
		bool has_half_heart = (player->stats.char1_curr_health % 2);

		float canvas_width = hearts_to_draw * HUD_HEART_SIZE;
		float start_x = AEGfxGetWinMinX()  + HUD_HEART_SIZE / 2;
		float y_position = AEGfxGetWinMaxY() - HUD_HEART_SIZE / 2;

		render_ui_sprite(rect_mesh_center, start_x, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, player->char1.texture);

		//for (int i{1}; i < hearts_to_draw+1; ++i)
		//{
		//	f32 x_position = start_x + i * HUD_HEART_SIZE;
		//	if (i <= full_hearts) {
		//		render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, ui_manager->hp_full_texture);
		//	}
		//	else if (has_half_heart) {
		//		render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, ui_manager->hp_half_texture);
		//	}
		//	else {
		//		render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, ui_manager->hp_empty_texture);
		//	}
		//}

		for (int i{ 1 }; i < hearts_to_draw + 1; ++i)
		{
			f32 x_position = start_x + i * HUD_HEART_SIZE;

			if (i <= full_hearts) {
				render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, ui_manager->hp_full_texture);
			}
			else if (i == full_hearts + 1 && has_half_heart) {
				render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, ui_manager->hp_half_texture);
			}
			else {
				render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, ui_manager->hp_empty_texture);
			}
		}

		hearts_to_draw = player->stats.char2_max_health / 2 + player->stats.char2_max_health % 2;
		full_hearts = player->stats.char2_curr_health / 2;
		has_half_heart = (player->stats.char2_curr_health % 2);

		y_position = y_position - HUD_HEART_SIZE;

		render_ui_sprite(rect_mesh_center, start_x, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, player->char2.texture);
	
		//for (int i{1}; i < hearts_to_draw+1; ++i)
		//{
		//	f32 x_position = start_x + i * HUD_HEART_SIZE;
		//	if (i <= full_hearts) {
		//		render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, ui_manager->hp_full_texture);
		//	}
		//	else if (has_half_heart) {
		//		render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, ui_manager->hp_half_texture);
		//	}
		//	else {
		//		render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, ui_manager->hp_empty_texture);
		//	}
		//}
		for (int i{ 1 }; i < hearts_to_draw + 1; ++i)
		{
			f32 x_position = start_x + i * HUD_HEART_SIZE;
			if (i <= full_hearts) {
				render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, ui_manager->hp_full_texture);
			}
			else if (i == full_hearts + 1 && has_half_heart) {
				render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, ui_manager->hp_half_texture);
			}
			else {
				render_ui_sprite(rect_mesh_center, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, ui_manager->hp_empty_texture);
			}
		}
	}


