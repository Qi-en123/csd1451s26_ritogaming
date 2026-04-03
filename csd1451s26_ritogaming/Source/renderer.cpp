/*!************************************************************************
\file:						render.cpp
\author:					Jerod Lim
\par DP email:				jerodyaowei.lim@digipen.edu
\co-author:					Claire
\par co-author DP email:	c.cheong@digipen.edu
\co-author:					Goh Qi En
\par co-author DP email:	g.qien@digipen.edu
\brief:						This file handles mesh initialization and
							rendering of shapes,sprites, and text, including
							camera zoom support using AEEngine.
**************************************************************************/
#include "renderer.hpp"
#include "AEEngine.h"
#include "camera.hpp"
#include "map.hpp"

// ------------------------------------------------------------------------
// Light settings
// ------------------------------------------------------------------------ 
constexpr s32 player_light = 7;
constexpr f32 player_light_fade = 1.0f;
constexpr f32 max_brightness = 0.8f;
std::vector<light> lights;

// ------------------------------------------------------------------------
// Meshes
// ------------------------------------------------------------------------ 
AEGfxVertexList* rect_mesh_corner = nullptr;
AEGfxVertexList* square_mesh = nullptr;
AEGfxVertexList* circle_mesh = nullptr;
AEGfxVertexList* triangle_mesh = nullptr;
AEGfxVertexList* light_source_mesh = nullptr;

// ------------------------------------------------------------------------
// Colors (shorcuts)
// ------------------------------------------------------------------------ 
const color red =		{ 1.0f, 0.0f, 0.0f, 0.0f };
const color green =		{ 0.0f, 1.0f, 0.0f, 0.0f };
const color blue =		{ 0.0f, 0.0f, 1.0f, 0.0f };
const color dark_red =	{ 0.5f, 0.0f, 0.0f, 0.0f };
const color black =		{ 0.0f, 0.0f, 0.0f, 0.0f };
const color white =		{ 1.0f, 1.0f, 1.0f, 0.0f };
const color yellow =	{ 1.0f, 1.0f, 0.0f, 0.0f };
const color cyan =		{ 0.0f, 1.0f, 1.0f, 0.0f };

void AEGfxSetColorToAdd(const color& c)
{
	AEGfxSetColorToAdd(c.r, c.g, c.b, c.a);
}
void AEGfxSetColorToMultiply(const color& c)
{
	AEGfxSetColorToMultiply(c.r, c.g, c.b, 1.0f);
}

// ------------------------------------------------------------------------
// Stores current tooltip to show
// ------------------------------------------------------------------------ 
const char* curr_tooltip = "";

namespace {
#pragma region Mesh creation helpers
	AEGfxVertexList* create_rect_corner_mesh() {
		AEGfxMeshStart();
		AEGfxTriAdd( //1st tri
			0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 0.0f,
			0.0f, -1.0f, 0xFFFFFFFF, 0.0f, 1.0f,
			1.0f, 0.0f, 0xFFFFFFFF, 1.0f, 0.0f
		);

		AEGfxTriAdd( //2nd tri
			1.0f, -1.0f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.0f, -1.0f, 0xFFFFFFFF, 0.0f, 1.0f,
			1.0f, 0.0f, 0xFFFFFFFF, 1.0f, 0.0f
		);
		return AEGfxMeshEnd();
	}
	AEGfxVertexList* create_square_mesh() {
		AEGfxMeshStart();

		// First triangle
		AEGfxTriAdd(
			-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
		);

		// Second triangle
		AEGfxTriAdd(
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
		);

		return AEGfxMeshEnd();
	}
	AEGfxVertexList* create_circle_mesh() {
		AEGfxMeshStart();
		//how many triangles to make up the circle
		s32 tri_count = 128;
		f32 theta = 360.0f / tri_count;
		for (s32 i = 0; i < tri_count; ++i) {
			f32 curr_angle = theta * i;
			//x = r*cos0 || y = r*sin0 /r=1
			f32 curr_x = (f32)cos(curr_angle * PI / 180.0f); //convert to rad
			f32 curr_y = (f32)sin(curr_angle * PI / 180.0f);

			f32 next_x = (f32)cos((curr_angle + theta) * PI / 180.0f);
			f32 next_y = (f32)sin((curr_angle + theta) * PI / 180.0f);

			AEGfxTriAdd( //1st tri
				0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 0.0f, //mid-point
				curr_x, curr_y, 0xFFFFFFFF, curr_x, curr_y,
				next_x, next_y, 0xFFFFFFFF, next_x, next_y
			);
		}
		return AEGfxMeshEnd();
	}
	AEGfxVertexList* create_triangle_mesh() {
		AEGfxMeshStart();

		// Create a triangle pointing DOWNWARD (inverted Y coordinates)
		AEGfxTriAdd(
			0.0f, -0.5f, 0xFFFFFFFF, 0.5f, 1.0f,  // Bottom point (now at bottom)
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,  // Top-left (now at top)
			0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f   // Top-right (now at top)
		);

		return AEGfxMeshEnd();
	}
	AEGfxVertexList* create_ls_mesh() {
		AEGfxMeshStart();
		//how many triangles to make up the circle
		s32 tri_count = 128;
		f32 theta = 360.0f / tri_count;
		for (s32 i = 0; i < tri_count; ++i) {
			f32 curr_angle = theta * i;
			//x = r*cos0 || y = r*sin0 /r=1
			f32 curr_x = (f32)cos(curr_angle * PI / 180.0f); //convert to rad
			f32 curr_y = (f32)sin(curr_angle * PI / 180.0f);

			f32 next_x = (f32)cos((curr_angle + theta) * PI / 180.0f);
			f32 next_y = (f32)sin((curr_angle + theta) * PI / 180.0f);

			AEGfxTriAdd( //1st tri
				0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 0.0f, //mid-point
				curr_x, curr_y, 0xFF000000, curr_x, curr_y,
				next_x, next_y, 0xFF000000, next_x, next_y
			);
		}
		return AEGfxMeshEnd();
	}
#pragma endregion
	
	//Checks if sprite is on screen (Used for culling)
	bool on_screen(f32 x, f32 y) {
		return (x < Camera::cam.pos.x + (AEGfxGetWindowWidth() / 2.0f / Camera::zoom) + tile_size &&
			x > Camera::cam.pos.x - (AEGfxGetWindowWidth() / 2.0f / Camera::zoom) - tile_size &&
			y < Camera::cam.pos.y + (AEGfxGetWindowHeight() / 2.0f / Camera::zoom) + tile_size &&
			y > Camera::cam.pos.y - (AEGfxGetWindowHeight() / 2.0f / Camera::zoom) - tile_size);
	}

	//Draws a shape on screen that moves with the camera
	void draw_shape(AEGfxVertexList* mesh, f32 trans_x, f32 trans_y,
		f32 scale_x, f32 scale_y, bool is_zoom) {

		AEMtx33 transform;
		AEMtx33Identity(&transform);

		AEMtx33 translate;
		AEMtx33Trans(&translate, trans_x, trans_y);

		AEMtx33 scale;
		AEMtx33Scale(&scale, scale_x, scale_y);

		if (is_zoom) {
			AEMtx33 cam_zoom;
			AEMtx33Scale(&cam_zoom, Camera::zoom, Camera::zoom);

			AEMtx33 cam_translate;
			AEMtx33Trans(&translate, trans_x, trans_y);
			AEMtx33Trans(&cam_translate, -Camera::cam.pos.x, -Camera::cam.pos.y);

			AEMtx33Concat(&transform, &transform, &cam_zoom);
			AEMtx33Concat(&transform, &transform, &cam_translate);
		}

		AEMtx33Concat(&transform, &transform, &translate);
		AEMtx33Concat(&transform, &transform, &scale);

		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
	}

	void update_light_level(f32 x, f32 y, color& color) {
		//Brighten up near players
		f32 total_fade{};

		AEVec2 ppos1 = { player_instance.char1.x,player_instance.char1.y };
		AEVec2 ppos2 = { player_instance.char2.x,player_instance.char2.y };
		AEVec2 tpos = { x, y };

		//dist between p1 and tile
		f32 dist1 = AEVec2Distance(&ppos1, &tpos);
		//dist between p2 and tile
		f32 dist2 = AEVec2Distance(&ppos2, &tpos);
		//the shorter of the dist
		f32 min_dist = min(dist1, dist2);
		//"view range" of players
		f32 max_rad = player_light * tile_size;

		//fade is higher the further the players
		total_fade = 1.0f - min(min_dist / max_rad, 1.0f);
		//exp <1 makes fade less effective (light more consistent around player)
		total_fade = pow(total_fade, player_light_fade);

		//Brighten up near light sources as well
		for (light& l : lights) {
			//dist between light source and tile
			f32 dist = AEVec2Distance(&l.pos, &tpos);
			//amount to fade based on this light source
			f32 fade = 1.0f - min(dist / (l.radius * tile_size), 1.0f);
			//exp >1 makes light brighter in the middle, less effective on the edges
			fade = pow(fade, l.effectiveness);

			//adds "light" from each light source including players
			total_fade += fade;
			color.r += (l.color.r - color.r) * fade;
			color.g += (l.color.g - color.g) * fade;
			color.b += (l.color.b - color.b) * fade;
		}

		//prevents fade exceeding 1
		total_fade = min(total_fade, 1.0f);
		//sets min light level
		total_fade = max(total_fade, 0.1f);

		//adding the fade to the sprites color
		color.r *= total_fade;
		color.g *= total_fade;
		color.b *= total_fade;
	}
}

#pragma region SET UP FUNCTIONS

void init_mesh() {
	rect_mesh_corner = create_rect_corner_mesh();
	square_mesh = create_square_mesh();
	circle_mesh = create_circle_mesh();
	triangle_mesh = create_triangle_mesh();
	light_source_mesh = create_ls_mesh();
}
void free_mesh() {
	if (light_source_mesh) { AEGfxMeshFree(light_source_mesh); light_source_mesh = nullptr; }
	if (circle_mesh) { AEGfxMeshFree(circle_mesh);       circle_mesh = nullptr; }
	if (rect_mesh_corner) { AEGfxMeshFree(rect_mesh_corner);  rect_mesh_corner = nullptr; }
	if (square_mesh) { AEGfxMeshFree(square_mesh);       square_mesh = nullptr; }
	if (triangle_mesh) { AEGfxMeshFree(triangle_mesh);     triangle_mesh = nullptr; }
}

#pragma endregion

#pragma region RENDER TEXT

void render_text_center(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, color color) {

	f32 width, height;
	AEGfxGetPrintSize(font, text, scale, &width, &height);

	AEGfxPrint(font, text, pos_x - (width / 2.0f), pos_y - (height / 2.0f),
		scale, color.r, color.g, color.b, 1.0f);
	AEGfxTextureSet(nullptr, 0, 0);
}
void render_text_corner(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, color color) {

	AEGfxPrint(font, text, pos_x, pos_y,
		scale, color.r, color.g, color.b, 1.0f);
	AEGfxTextureSet(nullptr, 0, 0);
}

#pragma endregion

#pragma region RENDER SHAPES

void draw_circle(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool is_zoom) {
	
	draw_shape(circle_mesh, trans_x, trans_y, scale_x, scale_y, is_zoom);
}
void draw_rect_corner(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool is_zoom) {

	draw_shape(rect_mesh_corner, trans_x, trans_y, scale_x, scale_y, is_zoom);
}
void draw_square(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool is_zoom) {

	draw_shape(square_mesh, trans_x, trans_y, scale_x, scale_y, is_zoom);
}
void draw_triangle(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool is_zoom) {

	draw_shape(triangle_mesh, trans_x, trans_y, scale_x, scale_y, is_zoom);
}

#pragma endregion

void render_sprite(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture, color color) {
	//CULLING
	if (on_screen(trans_x, trans_y)) {
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEMtx33 transform;
		AEMtx33Identity(&transform);

		AEMtx33 translate;
		AEMtx33Trans(&translate, trans_x, trans_y);

		AEMtx33 scale;
		AEMtx33Scale(&scale, scale_x, scale_y);

		AEMtx33 cam_translate;
		AEMtx33Trans(&cam_translate, -Camera::cam.pos.x, -Camera::cam.pos.y);

		AEMtx33 cam_zoom;
		AEMtx33Scale(&cam_zoom, Camera::zoom, Camera::zoom);

		AEMtx33Concat(&transform, &transform, &cam_zoom);
		AEMtx33Concat(&transform, &transform, &cam_translate);

		AEMtx33Concat(&transform, &transform, &translate);
		AEMtx33Concat(&transform, &transform, &scale);

		//Update light levels (fade)
		update_light_level(trans_x, trans_y, color);

		//multiplies sprite with color, and fade
		AEGfxSetColorToMultiply(color.r, color.g, color.b, 1.0f);
		AEGfxTextureSet(texture, 0, 0);
		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(rect_mesh, AE_GFX_MDM_TRIANGLES);
		AEGfxTextureSet(nullptr, 0, 0);
	}
}

void render_ui_sprite(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture, color color) {

	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
	AEMtx33 transform;
	AEMtx33Identity(&transform);

	AEMtx33 translate;
	AEMtx33Trans(&translate, trans_x, trans_y);

	AEMtx33 scale;
	AEMtx33Scale(&scale, scale_x, scale_y);

	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);

	//AEMtx33Concat(&transform, &translate, &scale);
	AEGfxSetColorToMultiply(color.r, color.g, color.b, 1.0f);
	AEGfxTextureSet(texture, 0, 0);
	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(rect_mesh, AE_GFX_MDM_TRIANGLES);
	AEGfxTextureSet(nullptr, 0, 0);
}

void render_anim_sprite(f32 x, f32 y, f32 w, f32 h, Animator& anim)
{
	if (!anim.sheet || !square_mesh) return;

	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);

	// Build a one-off mesh with the correct UV window
	AEGfxMeshStart();
	float u0 = anim.u0(), u1 = anim.u1();
	float v0 = anim.v0(), v1 = anim.v1();
	AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, u0, v1,
		0.5f, -0.5f, 0xFFFFFFFF, u1, v1,
		-0.5f, 0.5f, 0xFFFFFFFF, u0, v0);
	AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, u1, v1,
		0.5f, 0.5f, 0xFFFFFFFF, u1, v0,
		-0.5f, 0.5f, 0xFFFFFFFF, u0, v0);
	AEGfxVertexList* mesh = AEGfxMeshEnd();

	AEGfxTextureSet(anim.sheet, 0, 0);

	AEMtx33 transform, scale, translate, cam_t, cam_z;
	AEMtx33Scale(&scale, w, h);
	AEMtx33Trans(&translate, x, y);
	AEMtx33Trans(&cam_t, -Camera::cam.pos.x, -Camera::cam.pos.y);
	AEMtx33Scale(&cam_z, Camera::zoom, Camera::zoom);
	AEMtx33Identity(&transform);
	AEMtx33Concat(&transform, &transform, &cam_z);
	AEMtx33Concat(&transform, &transform, &cam_t);
	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);

	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
	AEGfxMeshFree(mesh);
	AEGfxTextureSet(nullptr, 0, 0);
}

void get_zoom(f32 dt, f32 zoom_min, f32 zoom_max) {
	s32 p_delta;
	AEInputMouseWheelDelta(&p_delta);
	if (p_delta > 0) {
		Camera::zoom += dt * 2.0f;
	}
	else if (p_delta < 0) {
		//scroll down
		Camera::zoom -= dt * 2.0f;
	}
	Camera::zoom = AEClamp(Camera::zoom, zoom_min, zoom_max);
}

// for blending
void render_editor_ui(AEGfxTexture* texture,
	f32 width, f32 height,
	f32 alpha,
	f32 trans_x , f32 trans_y,
	bool highlighted)
{
	if (!texture || !square_mesh) return;

	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

	if (highlighted)
		AEGfxSetColorToAdd(0.3f, 0.3f, 0.3f, 0.f);
	else
		AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);

	AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
	AEGfxSetTransparency(alpha);
	AEGfxTextureSet(texture, 0, 0);

	AEMtx33 transform, scale, translate;
	AEMtx33Identity(&transform);

	AEMtx33Scale(&scale, width, height);
	AEMtx33Trans(&translate, trans_x, trans_y); 

	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);

	AEGfxSetTransform(transform.m);

	AEGfxMeshDraw(square_mesh, AE_GFX_MDM_TRIANGLES);
	AEGfxTextureSet(nullptr, 0, 0);
}

void render_editor_tiles(AEGfxTexture* texture,
	f32 width, f32 height,
	f32 alpha,
	f32 trans_x, f32 trans_y,
	bool highlighted)
{
	if (!texture || !square_mesh) return;

	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

	if (highlighted)
		AEGfxSetColorToAdd(0.3f, 0.3f, 0.3f, 0.f);
	else
		AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);

	AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
	AEGfxSetTransparency(alpha);
	AEGfxTextureSet(texture, 0, 0);

	AEMtx33 transform, scale, translate;
	AEMtx33Identity(&transform);

	AEMtx33Scale(&scale, width, height);
	AEMtx33Trans(&translate, trans_x, trans_y);

	AEMtx33 cam_translate;
	AEMtx33Trans(&cam_translate, -Camera::cam.pos.x, -Camera::cam.pos.y);

	AEMtx33 cam_zoom;
	AEMtx33Scale(&cam_zoom, Camera::zoom, Camera::zoom);

	AEMtx33Concat(&transform, &transform, &cam_zoom);
	AEMtx33Concat(&transform, &transform, &cam_translate);

	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);

	AEGfxSetTransform(transform.m);

	AEGfxMeshDraw(square_mesh, AE_GFX_MDM_TRIANGLES);
	AEGfxTextureSet(nullptr, 0, 0);
}

// for tint in mainmenu
void draw_black_overlay(f32 alpha)
{
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToMultiply(0.f, 0.f, 0.f, 1.f);
	AEGfxSetColorToAdd(-1.f, -1.f, -1.f, 0.f);
	AEGfxSetTransparency(alpha);

	//Draws a black square covering the screen
	draw_shape(square_mesh, 0.0f, 0.0f, AEGfxGetWindowWidth(), AEGfxGetWindowHeight(), false);

	AEGfxSetTransparency(1.f);
	AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
}

void render_tooltip(s8 font) {
	f32 width, height;
	f32 padding = 0.03f;

	AEGfxGetPrintSize(font, curr_tooltip, 1.0f, &width, &height);

	AEGfxPrint(font, curr_tooltip, 1.0f - width - padding, -1.0f + padding,
		1.0f, green.r, green.g, green.b, 1.0f);

	//resets tooltip
	curr_tooltip = "";
	AEGfxTextureSet(nullptr, 0, 0);
}

void render_boss_projectiles(const BossProjectileManager& manager)
{
	const float outer_r = BossProjCfg::RING_RADIUS + BossProjCfg::RING_THICKNESS * 0.5f;
	const float inner_r = BossProjCfg::RING_RADIUS - BossProjCfg::RING_THICKNESS * 0.5f;
	const int   SEGS = 32;   // number of segments around the ring
	const float step = 2.f * PI / static_cast<float>(SEGS);

	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetTransparency(1.f);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetColorToMultiply(0.5f, 1.f, 1.f, 1.f);

	for (const auto& p : manager.get_projectiles())
	{
		if (!p.in_use) continue;

		// Build a ring mesh made of SEGS quads (2 triangles each).
		// Each quad bridges inner_r to outer_r at angle i and i+1.
		// The centre gap is never filled � fully transparent.
		AEGfxMeshStart();
		for (int i = 0; i < SEGS; ++i)
		{
			float a0 = step * static_cast<float>(i);
			float a1 = step * static_cast<float>(i + 1);

			float ox0 = cosf(a0) * outer_r, oy0 = sinf(a0) * outer_r;
			float ox1 = cosf(a1) * outer_r, oy1 = sinf(a1) * outer_r;
			float ix0 = cosf(a0) * inner_r, iy0 = sinf(a0) * inner_r;
			float ix1 = cosf(a1) * inner_r, iy1 = sinf(a1) * inner_r;

			// Triangle 1: outer0, outer1, inner0
			AEGfxTriAdd(ox0, oy0, 0xFF2699FF, 0.f, 0.f,
				ox1, oy1, 0xFF2699FF, 1.f, 0.f,
				ix0, iy0, 0xFF2699FF, 0.f, 1.f);

			// Triangle 2: outer1, inner1, inner0
			AEGfxTriAdd(ox1, oy1, 0xFF2699FF, 1.f, 0.f,
				ix1, iy1, 0xFF2699FF, 1.f, 1.f,
				ix0, iy0, 0xFF2699FF, 0.f, 1.f);
		}
		AEGfxVertexList* ring_mesh = AEGfxMeshEnd();
		if (!ring_mesh) continue;

		// Position the ring in world space using the same camera transform
		// as draw_circle() / render_sprite()
		AEMtx33 transform, translate, cam_t, cam_z;
		AEMtx33Trans(&translate, p.x, p.y);
		AEMtx33Trans(&cam_t, -Camera::cam.pos.x, -Camera::cam.pos.y);
		AEMtx33Scale(&cam_z, Camera::zoom, Camera::zoom);

		AEMtx33Identity(&transform);
		AEMtx33Concat(&transform, &transform, &cam_z);
		AEMtx33Concat(&transform, &transform, &cam_t);
		AEMtx33Concat(&transform, &transform, &translate);

		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(ring_mesh, AE_GFX_MDM_TRIANGLES);
		AEGfxMeshFree(ring_mesh);
	}
}


void render_sprite_flash(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture, f32 flash)
{
	if (on_screen(trans_x, trans_y)) {
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

		AEMtx33 transform, translate, scale, cam_translate, cam_zoom;
		AEMtx33Trans(&translate, trans_x, trans_y);
		AEMtx33Scale(&scale, scale_x, scale_y);
		AEMtx33Trans(&cam_translate, -Camera::cam.pos.x, -Camera::cam.pos.y);
		AEMtx33Scale(&cam_zoom, Camera::zoom, Camera::zoom);
		AEMtx33Identity(&transform);
		AEMtx33Concat(&transform, &transform, &cam_zoom);
		AEMtx33Concat(&transform, &transform, &cam_translate);
		AEMtx33Concat(&transform, &transform, &translate);
		AEMtx33Concat(&transform, &transform, &scale);

		color c = white;
		update_light_level(trans_x, trans_y, c);
		AEGfxSetColorToMultiply(c.r, c.g, c.b, 1.0f);

		// Set color add AFTER render mode — this is what makes the flash work
		AEGfxSetColorToAdd(flash, flash, flash, 0.f);

		AEGfxTextureSet(texture, 0, 0);
		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(rect_mesh, AE_GFX_MDM_TRIANGLES);
		AEGfxTextureSet(nullptr, 0, 0);

		AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	}
}