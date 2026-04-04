/*!************************************************************************
\file:						render.cpp
\author:					Jerod Lim
\par DP email:				jerodyaowei.lim@digipen.edu
\co-author:					Claire
\par co-author DP email:	c.cheong@digipen.edu
\co-author:					Goh Qi En
\par co-author DP email:	g.qien@digipen.edu

\brief:						This file handles mesh initialization and
							rendering of shapes, sprites, and text, including
							camera zoom support and a dynamic lighting system
							using AEEngine.
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
AEGfxVertexList* ring_mesh = nullptr;

// ------------------------------------------------------------------------
// Colors (shortcuts)
// ------------------------------------------------------------------------ 
const color red = { 1.0f, 0.0f, 0.0f, 0.0f };
const color green = { 0.0f, 1.0f, 0.0f, 0.0f };
const color blue = { 0.0f, 0.0f, 1.0f, 0.0f };
const color dark_red = { 0.5f, 0.0f, 0.0f, 0.0f };
const color black = { 0.0f, 0.0f, 0.0f, 0.0f };
const color white = { 1.0f, 1.0f, 1.0f, 0.0f };
const color yellow = { 1.0f, 1.0f, 0.0f, 0.0f };
const color cyan = { 0.0f, 1.0f, 1.0f, 0.0f };

// ----------------------------------------------------------------------------
//
// Function: AEGfxSetColorToAdd
// Sets the additive color using the custom color struct.
//
// ---------------------------------------------------------------------------- 
void AEGfxSetColorToAdd(const color& c)
{
	AEGfxSetColorToAdd(c.r, c.g, c.b, c.a);
}

// ----------------------------------------------------------------------------
//
// Function: AEGfxSetColorToMultiply
// Sets the multiplicative color using the custom color struct.
//
// ---------------------------------------------------------------------------- 
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

	// ----------------------------------------------------------------------------
	//
	// Function: create_rect_corner_mesh
	// Creates a rectangle mesh with the pivot at the top-left corner.
	//
	// ---------------------------------------------------------------------------- 
	AEGfxVertexList* create_rect_corner_mesh() {
		AEGfxMeshStart();
		AEGfxTriAdd(
			0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 0.0f,
			0.0f, -1.0f, 0xFFFFFFFF, 0.0f, 1.0f,
			1.0f, 0.0f, 0xFFFFFFFF, 1.0f, 0.0f
		);
		AEGfxTriAdd(
			1.0f, -1.0f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.0f, -1.0f, 0xFFFFFFFF, 0.0f, 1.0f,
			1.0f, 0.0f, 0xFFFFFFFF, 1.0f, 0.0f
		);
		return AEGfxMeshEnd();
	}

	// ----------------------------------------------------------------------------
	//
	// Function: create_square_mesh
	// Creates a 1x1 square mesh centered at the origin.
	//
	// ---------------------------------------------------------------------------- 
	AEGfxVertexList* create_square_mesh() {
		AEGfxMeshStart();
		AEGfxTriAdd(
			-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
		);
		AEGfxTriAdd(
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
		);
		return AEGfxMeshEnd();
	}

	// ----------------------------------------------------------------------------
	//
	// Function: create_circle_mesh
	// Creates a circular mesh using a high triangle count (128 segments).
	//
	// ---------------------------------------------------------------------------- 
	AEGfxVertexList* create_circle_mesh() {
		AEGfxMeshStart();
		s32 tri_count = 128;
		f32 theta = 360.0f / tri_count;
		for (s32 i = 0; i < tri_count; ++i) {
			f32 curr_angle = theta * i;
			f32 curr_x = (f32)cos(curr_angle * PI / 180.0f);
			f32 curr_y = (f32)sin(curr_angle * PI / 180.0f);
			f32 next_x = (f32)cos((curr_angle + theta) * PI / 180.0f);
			f32 next_y = (f32)sin((curr_angle + theta) * PI / 180.0f);
			AEGfxTriAdd(
				0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 0.0f,
				curr_x, curr_y, 0xFFFFFFFF, curr_x, curr_y,
				next_x, next_y, 0xFFFFFFFF, next_x, next_y
			);
		}
		return AEGfxMeshEnd();
	}

	// ----------------------------------------------------------------------------
	//
	// Function: create_triangle_mesh
	// Creates an inverted triangle mesh pointing downwards.
	//
	// ---------------------------------------------------------------------------- 
	AEGfxVertexList* create_triangle_mesh() {
		AEGfxMeshStart();
		AEGfxTriAdd(
			0.0f, -0.5f, 0xFFFFFFFF, 0.5f, 1.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
			0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f
		);
		return AEGfxMeshEnd();
	}

	// ----------------------------------------------------------------------------
	//
	// Function: create_ls_mesh
	// Creates a light source mesh with color gradients for lighting effects.
	//
	// ---------------------------------------------------------------------------- 
	AEGfxVertexList* create_ls_mesh() {
		AEGfxMeshStart();
		s32 tri_count = 128;
		f32 theta = 360.0f / tri_count;
		for (s32 i = 0; i < tri_count; ++i) {
			f32 curr_angle = theta * i;
			f32 curr_x = (f32)cos(curr_angle * PI / 180.0f);
			f32 curr_y = (f32)sin(curr_angle * PI / 180.0f);
			f32 next_x = (f32)cos((curr_angle + theta) * PI / 180.0f);
			f32 next_y = (f32)sin((curr_angle + theta) * PI / 180.0f);
			AEGfxTriAdd(
				0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 0.0f,
				curr_x, curr_y, 0xFF000000, curr_x, curr_y,
				next_x, next_y, 0xFF000000, next_x, next_y
			);
		}
		return AEGfxMeshEnd();
	}
#pragma endregion

	// ----------------------------------------------------------------------------
	//
	// Function: on_screen
	// Checks if the given world coordinates are within the camera's view (Culling).
	//
	// ---------------------------------------------------------------------------- 
	bool on_screen(f32 x, f32 y) {
		return (x < Camera::cam.pos.x + (AEGfxGetWindowWidth() / 2.0f / Camera::zoom) + tile_size &&
			x > Camera::cam.pos.x - (AEGfxGetWindowWidth() / 2.0f / Camera::zoom) - tile_size &&
			y < Camera::cam.pos.y + (AEGfxGetWindowHeight() / 2.0f / Camera::zoom) + tile_size &&
			y > Camera::cam.pos.y - (AEGfxGetWindowHeight() / 2.0f / Camera::zoom) - tile_size);
	}

	// ----------------------------------------------------------------------------
	//
	// Function: draw_shape
	// Internal helper to apply transformations and draw a mesh relative to camera.
	//
	// ---------------------------------------------------------------------------- 
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
			AEMtx33Trans(&cam_translate, -Camera::cam.pos.x, -Camera::cam.pos.y);
			AEMtx33Concat(&transform, &transform, &cam_zoom);
			AEMtx33Concat(&transform, &transform, &cam_translate);
		}

		AEMtx33Concat(&transform, &transform, &translate);
		AEMtx33Concat(&transform, &transform, &scale);

		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
	}

	// ----------------------------------------------------------------------------
	//
	// Function: update_light_level
	// Calculates light levels for a position based on players and light sources.
	//
	// ---------------------------------------------------------------------------- 
	void update_light_level(f32 x, f32 y, color& color) {
		f32 total_fade{};
		AEVec2 ppos1 = { player_instance.char1.x,player_instance.char1.y };
		AEVec2 ppos2 = { player_instance.char2.x,player_instance.char2.y };
		AEVec2 tpos = { x, y };

		f32 dist1 = AEVec2Distance(&ppos1, &tpos);
		f32 dist2 = AEVec2Distance(&ppos2, &tpos);
		f32 min_dist = min(dist1, dist2);
		f32 max_rad = player_light * tile_size;

		total_fade = 1.0f - min(min_dist / max_rad, 1.0f);
		total_fade = pow(total_fade, player_light_fade);

		for (light& l : lights) {
			f32 dist = AEVec2Distance(&l.pos, &tpos);
			f32 fade = 1.0f - min(dist / (l.radius * tile_size), 1.0f);
			fade = pow(fade, l.effectiveness);
			total_fade += fade;
			color.r += (l.color.r - color.r) * fade;
			color.g += (l.color.g - color.g) * fade;
			color.b += (l.color.b - color.b) * fade;
		}

		total_fade = min(total_fade, 1.0f);
		total_fade = max(total_fade, 0.1f);
		color.r *= total_fade;
		color.g *= total_fade;
		color.b *= total_fade;
	}
}

#pragma region SET UP FUNCTIONS

// ----------------------------------------------------------------------------
//
// Function: init_mesh
// Initializes all global meshes used by the renderer.
//
// ---------------------------------------------------------------------------- 
void init_mesh() {
	rect_mesh_corner = create_rect_corner_mesh();
	square_mesh = create_square_mesh();
	circle_mesh = create_circle_mesh();
	triangle_mesh = create_triangle_mesh();
	light_source_mesh = create_ls_mesh();

	//------------ Ring Mesh (boss projectiles) by Claire------------//
	const f32 outer_r = BossProjCfg::RING_RADIUS + BossProjCfg::RING_THICKNESS * 0.5f;
	const f32 inner_r = BossProjCfg::RING_RADIUS - BossProjCfg::RING_THICKNESS * 0.5f;
	const s32   SEGS = 32;
	const f32 step = 2.f * 3.14159265f / static_cast<float>(SEGS);

	AEGfxMeshStart();
	for (s32 i = 0; i < SEGS; ++i)
	{
		f32 a0 = step * static_cast<float>(i);
		f32 a1 = step * static_cast<float>(i + 1);
		f32 ox0 = cosf(a0) * outer_r, oy0 = sinf(a0) * outer_r;
		f32 ox1 = cosf(a1) * outer_r, oy1 = sinf(a1) * outer_r;
		f32 ix0 = cosf(a0) * inner_r, iy0 = sinf(a0) * inner_r;
		f32 ix1 = cosf(a1) * inner_r, iy1 = sinf(a1) * inner_r;

		AEGfxTriAdd(ox0, oy0, 0xFF2699FF, 0.f, 0.f,
			ox1, oy1, 0xFF2699FF, 1.f, 0.f,
			ix0, iy0, 0xFF2699FF, 0.f, 1.f);
		AEGfxTriAdd(ox1, oy1, 0xFF2699FF, 1.f, 0.f,
			ix1, iy1, 0xFF2699FF, 1.f, 1.f,
			ix0, iy0, 0xFF2699FF, 0.f, 1.f);
	}
	ring_mesh = AEGfxMeshEnd();

}

// ----------------------------------------------------------------------------
//
// Function: free_mesh
// Frees the memory of all initialized meshes.
//
// ---------------------------------------------------------------------------- 
void free_mesh() {
	if (light_source_mesh) { AEGfxMeshFree(light_source_mesh); light_source_mesh = nullptr; }
	if (circle_mesh) { AEGfxMeshFree(circle_mesh);       circle_mesh = nullptr; }
	if (rect_mesh_corner) { AEGfxMeshFree(rect_mesh_corner);  rect_mesh_corner = nullptr; }
	if (square_mesh) { AEGfxMeshFree(square_mesh);       square_mesh = nullptr; }
	if (triangle_mesh) { AEGfxMeshFree(triangle_mesh);     triangle_mesh = nullptr; }
	if (ring_mesh) { AEGfxMeshFree(ring_mesh);			   ring_mesh = nullptr; }
}

#pragma endregion

#pragma region RENDER TEXT

// ----------------------------------------------------------------------------
//
// Function: render_text_center
// Renders text centered around the specified position (normalized 0-1).
//
// ---------------------------------------------------------------------------- 
void render_text_center(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, color color) {

	f32 width, height;
	AEGfxGetPrintSize(font, text, scale, &width, &height);

	AEGfxPrint(font, text, pos_x - (width / 2.0f), pos_y - (height / 2.0f),
		scale, color.r, color.g, color.b, 1.0f);
	AEGfxTextureSet(nullptr, 0, 0);
}

// ----------------------------------------------------------------------------
//
// Function: render_text_corner
// Renders text from the specified corner position (normalized 0-1).
//
// ---------------------------------------------------------------------------- 
void render_text_corner(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, color color) {

	AEGfxPrint(font, text, pos_x, pos_y,
		scale, color.r, color.g, color.b, 1.0f);
	AEGfxTextureSet(nullptr, 0, 0);
}

#pragma endregion

#pragma region RENDER SHAPES

// ----------------------------------------------------------------------------
//
// Function: draw_circle
// Public function to draw a circular mesh in world space.
//
// ---------------------------------------------------------------------------- 
void draw_circle(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool is_zoom) {
	draw_shape(circle_mesh, trans_x, trans_y, scale_x, scale_y, is_zoom);
}

// ----------------------------------------------------------------------------
//
// Function: draw_rect_corner
// Public function to draw a rectangle mesh from a corner pivot.
//
// ---------------------------------------------------------------------------- 
void draw_rect_corner(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool is_zoom) {
	draw_shape(rect_mesh_corner, trans_x, trans_y, scale_x, scale_y, is_zoom);
}

// ----------------------------------------------------------------------------
//
// Function: draw_square
// Public function to draw a square mesh in world space.
//
// ---------------------------------------------------------------------------- 
void draw_square(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool is_zoom) {
	draw_shape(square_mesh, trans_x, trans_y, scale_x, scale_y, is_zoom);
}

// ----------------------------------------------------------------------------
//
// Function: draw_triangle
// Public function to draw a triangle mesh in world space.
//
// ---------------------------------------------------------------------------- 
void draw_triangle(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool is_zoom) {
	draw_shape(triangle_mesh, trans_x, trans_y, scale_x, scale_y, is_zoom);
}

#pragma endregion

// ----------------------------------------------------------------------------
//
// Function: render_sprite
// Renders a textured sprite in world space with light and culling applied.
//
// ---------------------------------------------------------------------------- 
void render_sprite(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture, color color) {
	if (on_screen(trans_x, trans_y)) {
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEMtx33 transform, translate, scale, cam_translate, cam_zoom;
		AEMtx33Identity(&transform);
		AEMtx33Trans(&translate, trans_x, trans_y);
		AEMtx33Scale(&scale, scale_x, scale_y);
		AEMtx33Trans(&cam_translate, -Camera::cam.pos.x, -Camera::cam.pos.y);
		AEMtx33Scale(&cam_zoom, Camera::zoom, Camera::zoom);

		AEMtx33Concat(&transform, &transform, &cam_zoom);
		AEMtx33Concat(&transform, &transform, &cam_translate);
		AEMtx33Concat(&transform, &transform, &translate);
		AEMtx33Concat(&transform, &transform, &scale);

		update_light_level(trans_x, trans_y, color);
		AEGfxSetColorToMultiply(color.r, color.g, color.b, 1.0f);
		AEGfxTextureSet(texture, 0, 0);
		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(rect_mesh, AE_GFX_MDM_TRIANGLES);
		AEGfxTextureSet(nullptr, 0, 0);
	}
}

// ----------------------------------------------------------------------------
//
// Function: render_ui_sprite
// Renders a textured sprite specifically for UI, ignoring camera transforms.
//
// ---------------------------------------------------------------------------- 
void render_ui_sprite(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture, color color) {

	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
	AEMtx33 transform, translate, scale;
	AEMtx33Identity(&transform);
	AEMtx33Trans(&translate, trans_x, trans_y);
	AEMtx33Scale(&scale, scale_x, scale_y);
	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);

	AEGfxSetColorToMultiply(color.r, color.g, color.b, 1.0f);
	AEGfxTextureSet(texture, 0, 0);
	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(rect_mesh, AE_GFX_MDM_TRIANGLES);
	AEGfxTextureSet(nullptr, 0, 0);
}

// ----------------------------------------------------------------------------
//
// Function: render_anim_sprite
// Renders an animated sprite using dynamic UV coordinates on a temporary mesh.
//
// ---------------------------------------------------------------------------- 
void render_anim_sprite(f32 x, f32 y, f32 w, f32 h, Animator& anim)
{
	if (!anim.sheet || !square_mesh) return;

	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);

	AEGfxMeshStart();
	f32 u0 = anim.u0(), u1 = anim.u1();
	f32 v0 = anim.v0(), v1 = anim.v1();
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

// ----------------------------------------------------------------------------
//
// Function: get_zoom
// Updates the global camera zoom factor based on mouse wheel input.
//
// ---------------------------------------------------------------------------- 
void get_zoom(f32 dt, f32 zoom_min, f32 zoom_max) {
	s32 p_delta;
	AEInputMouseWheelDelta(&p_delta);
	if (p_delta > 0) {
		Camera::zoom += dt * 2.0f;
	}
	else if (p_delta < 0) {
		Camera::zoom -= dt * 2.0f;
	}
	Camera::zoom = AEClamp(Camera::zoom, zoom_min, zoom_max);
}

// ----------------------------------------------------------------------------
//
// Function: render_editor_ui
// Renders static UI elements for the level editor with highlight support.
//
// ---------------------------------------------------------------------------- 
void render_editor_ui(AEGfxTexture* texture,
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
	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);

	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(square_mesh, AE_GFX_MDM_TRIANGLES);
	AEGfxTextureSet(nullptr, 0, 0);
}

// ----------------------------------------------------------------------------
//
// Function: render_editor_tiles
// Renders map tiles in world space for the editor view.
//
// ---------------------------------------------------------------------------- 
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

	AEMtx33 transform, scale, translate, cam_translate, cam_zoom;
	AEMtx33Identity(&transform);
	AEMtx33Scale(&scale, width, height);
	AEMtx33Trans(&translate, trans_x, trans_y);
	AEMtx33Trans(&cam_translate, -Camera::cam.pos.x, -Camera::cam.pos.y);
	AEMtx33Scale(&cam_zoom, Camera::zoom, Camera::zoom);

	AEMtx33Concat(&transform, &transform, &cam_zoom);
	AEMtx33Concat(&transform, &transform, &cam_translate);
	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);

	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(square_mesh, AE_GFX_MDM_TRIANGLES);
	AEGfxTextureSet(nullptr, 0, 0);
}

// ----------------------------------------------------------------------------
//
// Function: draw_black_overlay
// Draws a full-screen black quad with variable transparency for screen fades.
//
// ---------------------------------------------------------------------------- 
void draw_black_overlay(f32 alpha)
{
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToMultiply(0.f, 0.f, 0.f, 1.f);
	AEGfxSetColorToAdd(-1.f, -1.f, -1.f, 0.f);
	AEGfxSetTransparency(alpha);
	draw_shape(square_mesh, 0.0f, 0.0f, AEGfxGetWindowWidth(), AEGfxGetWindowHeight(), false);

	AEGfxSetTransparency(1.f);
	AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
}

// ----------------------------------------------------------------------------
//
// Function: render_tooltip
// Renders the global tooltip text string at a padded corner position.
//
// ---------------------------------------------------------------------------- 
void render_tooltip(s8 font) {
	f32 width, height;
	f32 padding = 0.03f;
	AEGfxGetPrintSize(font, curr_tooltip, 1.0f, &width, &height);
	AEGfxPrint(font, curr_tooltip, 1.0f - width - padding, -1.0f + padding,
		1.0f, green.r, green.g, green.b, 1.0f);
	curr_tooltip = "";
	AEGfxTextureSet(nullptr, 0, 0);
}

// ----------------------------------------------------------------------------
//
// Function: render_boss_projectiles
// Renders circular ring projectiles for boss encounters using dynamic meshes.
//
// ---------------------------------------------------------------------------- 
void render_boss_projectiles(const BossProjectileManager& manager)
{
	const f32 outer_r = BossProjCfg::RING_RADIUS + BossProjCfg::RING_THICKNESS * 0.5f;
	const f32 inner_r = BossProjCfg::RING_RADIUS - BossProjCfg::RING_THICKNESS * 0.5f;
	const s32 SEGS = 32;
	const f32 step = 2.f * PI / static_cast<float>(SEGS);

	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetTransparency(1.f);
	AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	AEGfxSetColorToMultiply(0.5f, 1.f, 1.f, 1.f);

	for (const auto& p : manager.get_projectiles())
	{
		if (!p.in_use) continue;
		AEGfxMeshStart();
		for (s32 i = 0; i < SEGS; ++i)
		{
			f32 a0 = step * static_cast<float>(i);
			f32 a1 = step * static_cast<float>(i + 1);
			f32 ox0 = cosf(a0) * outer_r, oy0 = sinf(a0) * outer_r;
			f32 ox1 = cosf(a1) * outer_r, oy1 = sinf(a1) * outer_r;
			f32 ix0 = cosf(a0) * inner_r, iy0 = sinf(a0) * inner_r;
			f32 ix1 = cosf(a1) * inner_r, iy1 = sinf(a1) * inner_r;
			AEGfxTriAdd(ox0, oy0, 0xFF2699FF, 0.f, 0.f, ox1, oy1, 0xFF2699FF, 1.f, 0.f, ix0, iy0, 0xFF2699FF, 0.f, 1.f);
			AEGfxTriAdd(ox1, oy1, 0xFF2699FF, 1.f, 0.f, ix1, iy1, 0xFF2699FF, 1.f, 1.f, ix0, iy0, 0xFF2699FF, 0.f, 1.f);
		}
		AEGfxVertexList* ring_mesh = AEGfxMeshEnd();
		if (!ring_mesh) continue;

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

// ----------------------------------------------------------------------------
//
// Function: render_sprite_flash
// Renders a sprite with an additive white flash (damage feedback).
//
// ---------------------------------------------------------------------------- 
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
		AEGfxSetColorToAdd(flash, flash, flash, 0.f);

		AEGfxTextureSet(texture, 0, 0);
		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(rect_mesh, AE_GFX_MDM_TRIANGLES);
		AEGfxTextureSet(nullptr, 0, 0);

		AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	}
}
