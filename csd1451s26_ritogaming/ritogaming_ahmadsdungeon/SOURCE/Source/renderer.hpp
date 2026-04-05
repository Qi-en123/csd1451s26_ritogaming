/*!************************************************************************
\file:						renderer.hpp
\author:					Jerod Lim
\par DP email:				jerodyaowei.lim@digipen.edu
\co-author:					Claire
\par co-author DP email:	c.cheong@digipen.edu
\co-author:					Goh Qi En
\par co-author DP email:	g.qien@digipen.edu

\brief:						This header declares rendering-related data
							structures and drawing functions using
							AEEngine.
**************************************************************************/

#pragma once
#include "AEEngine.h"
#include "player.hpp"
#include "enemy.hpp"
#include <vector>

// ------------------------------------------------------------------------
// Color (red, green, blue, alpha) [0.0f - 1.0f]
// ------------------------------------------------------------------------ 
struct color {
	f32 r, g, b, a;
};

// ------------------------------------------------------------------------
// Contains light variables
// pos:				Center of the light
// color:			Color of light (Multiplied onto sprites)
// radius:			How far light reaches
// effectiveness:	>1 makes light brighter in the middle,
//					less effective on the edges (vice versa)
// ------------------------------------------------------------------------ 
struct light {
	AEVec2 pos;
	color color;
	s32 radius;
	f32 effectiveness;
};
extern std::vector<light> lights; //stores list of lights in the level

// ------------------------------------------------------------------------
// Color constants
// ------------------------------------------------------------------------ 
extern const color red;
extern const color green;
extern const color blue;
extern const color dark_red;
extern const color white;
extern const color black;
extern const color yellow;
extern const color cyan;

// ------------------------------------------------------------------------
// Meshes
// ------------------------------------------------------------------------ 
extern AEGfxVertexList* rect_mesh_corner;
extern AEGfxVertexList* circle_mesh;
extern AEGfxVertexList* square_mesh;
extern AEGfxVertexList* triangle_mesh;
extern AEGfxVertexList* ring_mesh; 

// ------------------------------------------------------------------------
// Default zoom caps for camera
// ------------------------------------------------------------------------ 
constexpr f32 ZOOM_MAX = 1.65f;
constexpr f32 ZOOM_MIN = 0.8f;

// ------------------------------------------------------------------------
// Stores tooltip to render
// ------------------------------------------------------------------------ 
extern const char* curr_tooltip;

// ----------------------------------------------------------------------------
//
// Function: AEGfxSetColorToAdd
// Overload using color struct to set the additive color.
//
// ---------------------------------------------------------------------------- 
void AEGfxSetColorToAdd(const color& c);

// ----------------------------------------------------------------------------
//
// Function: AEGfxSetColorToMultiply
// Overload using color struct to set the multiplicative color.
//
// ---------------------------------------------------------------------------- 
void AEGfxSetColorToMultiply(const color& c);

// ----------------------------------------------------------------------------
//
// Function: init_mesh
// Initializes all meshes for render functions to use.
//
// ---------------------------------------------------------------------------- 
void init_mesh();

// ----------------------------------------------------------------------------
//
// Function: free_mesh
// Frees all mesh created by init_mesh.
//
// ---------------------------------------------------------------------------- 
void free_mesh();

// ----------------------------------------------------------------------------
//
// Function: get_zoom
// Updates zoom level. Should be called every frame.
//
// ---------------------------------------------------------------------------- 
void get_zoom(f32 zoom_min = ZOOM_MIN, f32 zoom_max = ZOOM_MAX);

// ----------------------------------------------------------------------------
//
// Function: draw_circle
// Draws a circle onto screen (Position is in the center). Affected by
// camera translation and optionally zoom.
//
// ---------------------------------------------------------------------------- 
void draw_circle(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool zoom = true);

// ----------------------------------------------------------------------------
//
// Function: draw_rect_corner
// Draws a square/rectangle onto screen (Position is at top left corner)
// Affected by camera translation and optionally zoom.
//
// ---------------------------------------------------------------------------- 
void draw_rect_corner(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool zoom = true);

// ----------------------------------------------------------------------------
//
// Function: draw_square
// Draws a square/rectangle onto screen (Position is in the center).
// Affected by camera translation and optionally zoom.
//
// ---------------------------------------------------------------------------- 
void draw_square(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool zoom = true);

// ----------------------------------------------------------------------------
//
// Function: draw_triangle
// Draws a triangle onto screen (Position is in the center).
// Affected by camera translation and optionally zoom.
//
// ---------------------------------------------------------------------------- 
void draw_triangle(f32 trans_x, f32 trans_y, f32 scale_x, f32 scale_y, bool zoom = true);

// ----------------------------------------------------------------------------
//
// Function: render_text_center
// Draws a text unto screen (Position is in center). Unaffected by camera
// zoom and translation.
//
// ---------------------------------------------------------------------------- 
void render_text_center(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, color color);

// ----------------------------------------------------------------------------
//
// Function: render_text_corner
// Draws a text unto screen (Position is in bottom right). Unaffected by camera
// zoom and translation.
//
// ---------------------------------------------------------------------------- 
void render_text_corner(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, color color);

// ----------------------------------------------------------------------------
//
// Function: render_tooltip
// Draws a text onto bottom right of screen (in yellow). Update 
// curr_tooltip every frame to change text.
//
// ---------------------------------------------------------------------------- 
void render_tooltip(s8 font);

// ----------------------------------------------------------------------------
//
// Function: draw_black_overlay
// Draws a black overlay onto screen. Used for darkening background for
// UI to be drawn on top.
//
// ---------------------------------------------------------------------------- 
void draw_black_overlay(f32 alpha);

// ----------------------------------------------------------------------------
//
// Function: render_sprite
// Draws a sprite onto screen that is affected by camera zoom, translation,
// light levels and culling.
//
// ---------------------------------------------------------------------------- 
void render_sprite(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture, color color = { 1.0f,1.0f,1.0f,1.0f });

// ----------------------------------------------------------------------------
//
// Function: render_ui_sprite
// Renders a sprite for the User Interface. Not affected by camera zoom
// or translation.
//
// ---------------------------------------------------------------------------- 
void render_ui_sprite(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture, color color = { 1.0f,1.0f,1.0f,1.0f });

// ----------------------------------------------------------------------------
//
// Function: render_editor_ui
// Renders UI elements specifically for the Level Editor, with support
// for alpha transparency and hover highlighting.
//
// ---------------------------------------------------------------------------- 
void render_editor_ui(AEGfxTexture* texture, f32 width, f32 height, f32 alpha,
	f32 trans_x = 0.f, f32 trans_y = 0.f, bool highlighted = false);

// ----------------------------------------------------------------------------
//
// Function: render_editor_tiles
// Renders individual map tiles in the editor. Affected by camera
// transforms but used primarily for grid layout.
//
// ---------------------------------------------------------------------------- 
void render_editor_tiles(AEGfxTexture* texture, f32 width, f32 height, f32 alpha,
	f32 trans_x = 0.f, f32 trans_y = 0.f, bool highlighted = false);

// ----------------------------------------------------------------------------
//
// Function: render_sprite_flash
// Renders a sprite with an additive white color flash. Used to indicate
// feedback such as taking damage.
//
// ---------------------------------------------------------------------------- 
void render_sprite_flash(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture, f32 flash);

// ----------------------------------------------------------------------------
//
// Function: render_anim_sprite
// Renders a single frame of an animated sprite based on the Animator's
// current state and UV coordinates.
//
// ---------------------------------------------------------------------------- 
struct Animator;
void render_anim_sprite(f32 x, f32 y, f32 w, f32 h, Animator& anim);

// ----------------------------------------------------------------------------
//
// Function: render_boss_projectiles
// Processes and renders all active projectiles managed by the 
// BossProjectileManager.
//
// ---------------------------------------------------------------------------- 
class BossProjectileManager;
void render_boss_projectiles(const BossProjectileManager& manager);