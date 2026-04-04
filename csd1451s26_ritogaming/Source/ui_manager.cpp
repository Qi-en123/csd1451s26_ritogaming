/*!************************************************************************
\file:          ui_manager.cpp
\author:        Claire
\par DP email:  c.cheong@digipen.edu
\
\brief:         Implements UI rendering functions for enemy health bars
\               above enemies and player HUD displaying character health
\               hearts in the corner of the screen.
**************************************************************************/

#include "ui_manager.hpp"
#include "renderer.hpp"

UIManager ui_manager;

// ------------------------------------------------------------------------
//
// Loads all UI textures and font into UIManager
//
// ------------------------------------------------------------------------
void load_ui_textures(UIManager* p_ui_manager)
{
	p_ui_manager->hp_empty_texture = AEGfxTextureLoad("Assets/Sprites/UI/hp_empty.png");
	p_ui_manager->hp_half_texture = AEGfxTextureLoad("Assets/Sprites/UI/hp_half.png");
	p_ui_manager->hp_full_texture = AEGfxTextureLoad("Assets/Sprites/UI/hp_full.png");

	// Ability / UI elements
	p_ui_manager->primary_bg_texture = AEGfxTextureLoad("Assets/Sprites/Projectile/tile_0078.png");
	p_ui_manager->primary_counter_texture = AEGfxTextureLoad("Assets/Sprites/Projectile/tile_0079.png");
	p_ui_manager->primary_ability_texture = AEGfxTextureLoad("Assets/Sprites/Projectile/fireball1.png");

	// Cursor textures
	p_ui_manager->cursor_knight = AEGfxTextureLoad("Assets/Sprites/Admin/cursorSword_silver.png");
	p_ui_manager->cursor_wizard = AEGfxTextureLoad("Assets/Sprites/Admin/wizard_wand_nonactive1.png");
	p_ui_manager->cursor_wizard_active = AEGfxTextureLoad("Assets/Sprites/Admin/wizard_wand_active1.png");

	// Font used for UI text
	p_ui_manager->font = AEGfxCreateFont("Assets/Fonts/Kenney Mini Square.ttf", 48);
}

// ------------------------------------------------------------------------
//
// Frees all loaded UI textures and font
//
// ------------------------------------------------------------------------
void free_ui_textures(UIManager* p_ui_manager)
{
	// Safely unload textures only if they exist
	if (p_ui_manager->hp_empty_texture)
		AEGfxTextureUnload(p_ui_manager->hp_empty_texture);
	if (p_ui_manager->hp_half_texture)
		AEGfxTextureUnload(p_ui_manager->hp_half_texture);
	if (p_ui_manager->hp_full_texture)
		AEGfxTextureUnload(p_ui_manager->hp_full_texture);

	if (p_ui_manager->primary_bg_texture)
		AEGfxTextureUnload(p_ui_manager->primary_bg_texture);
	if (p_ui_manager->primary_counter_texture)
		AEGfxTextureUnload(p_ui_manager->primary_counter_texture);

	if (p_ui_manager->cursor_knight)
		AEGfxTextureUnload(p_ui_manager->cursor_knight);
	if (p_ui_manager->cursor_wizard)
		AEGfxTextureUnload(p_ui_manager->cursor_wizard);
	if (p_ui_manager->cursor_wizard_active)
		AEGfxTextureUnload(p_ui_manager->cursor_wizard_active);

	// Destroy font if valid
	if (p_ui_manager->font != -1)
		AEGfxDestroyFont(p_ui_manager->font);
}

// ------------------------------------------------------------------------
//
// Renders character-specific UI (ability icon + counters)
//
// ------------------------------------------------------------------------
void render_character_ui(UIManager* p_ui_manager, Player* player)
{
	float icon_size = HUD_HEART_SIZE * 2.f;
	float paddingX = 10.f;
	float paddingY = 20.f;

	// Wizard UI (char2)
	if (player->current_player == &player->char2)
	{
		// Base position (bottom-left of screen)
		float x = AEGfxGetWinMinX() + paddingX + icon_size / 2.f;
		float y = AEGfxGetWinMinY() + paddingY + icon_size / 2.f;

		// Background + icon
		render_ui_sprite(square_mesh, x, y, icon_size, icon_size, p_ui_manager->primary_bg_texture);
		render_ui_sprite(square_mesh, x, y, icon_size / 2.f, icon_size / 2.f, p_ui_manager->cursor_wizard);

		// Counter box (fireball count)
		float counter_x = x + (paddingX * 10.f);
		render_ui_sprite(square_mesh, counter_x, y, icon_size / 1.5f, icon_size / 1.5f, p_ui_manager->primary_counter_texture);

		// Convert fireball count to string
		char fireball_text[8];
		sprintf_s(fireball_text, "%d", player->fireball_count);

		// Get text size for centering
		float text_w = 0.f, text_h = 0.f;
		AEGfxGetPrintSize(p_ui_manager->font, fireball_text, 1.f, &text_w, &text_h);

		// Convert to normalized screen space
		float norm_x = (counter_x / AEGfxGetWinMaxX()) - text_w / 2.f;
		float norm_y = (y / AEGfxGetWinMaxY()) - text_h / 2.f;

		// Render fireball count
		AEGfxPrint(p_ui_manager->font, fireball_text, norm_x, norm_y, 1.f, 1.f, 1.f, 1.f, 1.f);
	}

	// Knight UI (char1)
	if (player->current_player == &player->char1)
	{
		float x = AEGfxGetWinMinX() + paddingX + icon_size / 2.f;
		float y = AEGfxGetWinMinY() + paddingY + icon_size / 2.f;

		render_ui_sprite(square_mesh, x, y, icon_size, icon_size, p_ui_manager->primary_bg_texture);
		render_ui_sprite(square_mesh, x, y, icon_size / 2.f, icon_size / 2.f, p_ui_manager->cursor_knight);
	}
}

// ------------------------------------------------------------------------
//
// Renders mouse cursor based on current character
//
// ------------------------------------------------------------------------
void render_cursor(UIManager* ui_manager, Player* player, s32 mouse_x, s32 mouse_y, bool cursor_active)
{
	// Convert screen space to world space (centered coordinates)
	f32 world_x = (f32)mouse_x - 800.f;
	f32 world_y = -(f32)mouse_y + 450.f;

	AEGfxTexture* current_cursor = nullptr;

	// Select cursor based on active character
	if (player->current_player == &player->char1)
		current_cursor = ui_manager->cursor_knight;
	else
		current_cursor = cursor_active ? ui_manager->cursor_wizard_active : ui_manager->cursor_wizard;

	// Render if valid
	if (current_cursor)
		render_editor_ui(current_cursor, 50.f, 50.f, 1.0f, world_x, world_y);
}

// ------------------------------------------------------------------------
//
// Renders enemy HP as heart icons above each enemy
//
// ------------------------------------------------------------------------
void render_enemy_hp(UIManager* p_ui_manager, EnemyManager* enemy_manager)
{
	for (auto const& enemy : enemy_manager->get_enemies())
	{
		if (!enemy || !enemy->is_active())
			continue; // skip dead/inactive enemies

		// Calculate heart distribution
		int hearts_to_draw = enemy->get_max_health() / 2 + enemy->get_max_health() % 2;
		int full_hearts = enemy->get_current_health() / 2;
		bool has_half_heart = (enemy->get_current_health() % 2);

		// Scale based on enemy size
		float heart_scale = ENTITY_HP_SCALE * enemy->get_size();

		// Center hearts above enemy
		float canvas_width = hearts_to_draw * heart_scale;
		float start_x = enemy->get_x() - canvas_width / 2 + heart_scale / 2;

		for (int i{ 1 }; i < hearts_to_draw + 1; ++i)
		{
			f32 x_position = start_x + (i - 1) * heart_scale;
			f32 y_position = enemy->get_y() + enemy->get_size() / 2 + heart_scale / 2;

			// Decide which heart to draw
			if (i <= full_hearts)
				render_sprite(square_mesh, x_position, y_position, heart_scale, heart_scale, p_ui_manager->hp_full_texture);
			else if (i == full_hearts + 1 && has_half_heart)
				render_sprite(square_mesh, x_position, y_position, heart_scale, heart_scale, p_ui_manager->hp_half_texture);
			else
				render_sprite(square_mesh, x_position, y_position, heart_scale, heart_scale, p_ui_manager->hp_empty_texture);
		}
	}
}

// ------------------------------------------------------------------------
//
// Renders player HUD (both characters' health)
//
// ------------------------------------------------------------------------
void render_player_hud(UIManager* p_ui_manager, Player* player)
{
	// -------- Character 1 (Top row) --------
	int hearts_to_draw = player->stats.char1_max_health / 2 + player->stats.char1_max_health % 2;
	int full_hearts = player->stats.char1_curr_health / 2;
	bool has_half_heart = (player->stats.char1_curr_health % 2);

	float start_x = AEGfxGetWinMinX() + HUD_HEART_SIZE / 2;
	float y_position = AEGfxGetWinMaxY() - HUD_HEART_SIZE / 2;

	// Character icon
	render_ui_sprite(square_mesh, start_x, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, player->char1.texture);

	// Draw hearts
	for (int i{ 1 }; i < hearts_to_draw + 1; ++i)
	{
		f32 x_position = start_x + i * HUD_HEART_SIZE;

		if (i <= full_hearts)
			render_ui_sprite(square_mesh, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, p_ui_manager->hp_full_texture);
		else if (i == full_hearts + 1 && has_half_heart)
			render_ui_sprite(square_mesh, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, p_ui_manager->hp_half_texture);
		else
			render_ui_sprite(square_mesh, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, p_ui_manager->hp_empty_texture);
	}

	// -------- Character 2 (Bottom row) --------
	hearts_to_draw = player->stats.char2_max_health / 2 + player->stats.char2_max_health % 2;
	full_hearts = player->stats.char2_curr_health / 2;
	has_half_heart = (player->stats.char2_curr_health % 2);

	y_position = y_position - HUD_HEART_SIZE;

	// Character icon
	render_ui_sprite(square_mesh, start_x, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, player->char2.texture);

	for (int i{ 1 }; i < hearts_to_draw + 1; ++i)
	{
		f32 x_position = start_x + i * HUD_HEART_SIZE;

		if (i <= full_hearts)
			render_ui_sprite(square_mesh, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, p_ui_manager->hp_full_texture);
		else if (i == full_hearts + 1 && has_half_heart)
			render_ui_sprite(square_mesh, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, p_ui_manager->hp_half_texture);
		else
			render_ui_sprite(square_mesh, x_position, y_position, HUD_HEART_SIZE, HUD_HEART_SIZE, p_ui_manager->hp_empty_texture);
	}
}