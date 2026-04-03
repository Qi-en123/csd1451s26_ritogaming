/*!************************************************************************
\file:						render.hpp
\author:					Jerod
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

// ------------------------------------------------------------------------
// Default zoom caps for camera
// ------------------------------------------------------------------------ 
constexpr f32 ZOOM_MAX = 1.65f;
constexpr f32 ZOOM_MIN = 0.8f;

// ------------------------------------------------------------------------
// Stores tooltip to render
// ------------------------------------------------------------------------ 
extern const char* curr_tooltip;

// ------------------------------------------------------------------------
// 
// Overloads using color struct instead of using r,g,b,a seperately
// 
// ------------------------------------------------------------------------ 
void AEGfxSetColorToAdd(const color& c);
void AEGfxSetColorToMultiply(const color& c);

// ------------------------------------------------------------------------
// 
// Initializes all meshes for render functions to use
// 
// ------------------------------------------------------------------------ 
void init_mesh();

// ------------------------------------------------------------------------
// 
// Frees all mesh created by init_mesh
// 
// ------------------------------------------------------------------------ 
void free_mesh();

// ------------------------------------------------------------------------
// 
// Updates zoom level. Should be called every frame.
// dt		- deltatime
// zoom_min - minimum zoom cap
// zoom_max - maximum zoom cap
// 
// ------------------------------------------------------------------------ 
void get_zoom(f32 dt, f32 zoom_min = ZOOM_MIN, f32 zoom_max = ZOOM_MAX);

// ------------------------------------------------------------------------
// 
// Draws a circle onto screen (Position is in the center). Affected by
// camera translation and optionally zoom.
// trans_x - x pos centered on circle
// trans_y - y pos centered on circle
// scale_x - width of circle/oval
// scale_y - height of circle/oval
// zoom    - determines whether it should be affected by zoom or not
// 
// ------------------------------------------------------------------------ 
void draw_circle(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool zoom = true);

// ------------------------------------------------------------------------
// 
// Draws a square/rectangle onto screen (Position is at top left corner)
// Affected by camera translation and optionally zoom.
// trans_x - x pos of top left of rect
// trans_y - y pos of top left of rect
// scale_x - width of rect
// scale_y - height of rect
// zoom    - determines whether it should be affected by zoom or not
// 
// ------------------------------------------------------------------------ 
void draw_rect_corner(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool zoom = true);

// ------------------------------------------------------------------------
// 
// Draws a square/rectangle onto screen (Position is in the center).
// Affected by camera translation and optionally zoom.
// trans_x - x pos of center of rect
// trans_y - y pos of center of rect
// scale_x - width of rect
// scale_y - height of rect
// zoom    - determines whether it should be affected by zoom or not
// 
// ------------------------------------------------------------------------ 
void draw_square(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool zoom = true);

// ------------------------------------------------------------------------
// 
// Draws a triangle onto screen (Position is in the center).
// Affected by camera translation and optionally zoom.
// trans_x - x pos of center of triangle
// trans_y - y pos of center of triangle
// scale_x - width of triangle
// scale_y - height of triangle
// zoom    - determines whether it should be affected by zoom or not
// 
// ------------------------------------------------------------------------ 
void draw_triangle(f32 trans_x, f32 trans_y, f32 scale_x, f32 scale_y, bool zoom = true);

// ------------------------------------------------------------------------
// 
// Draws a text unto screen (Position is in center). Unaffected by camera
// zoom and translation.
// text  - text to draw
// font  - font of text
// pos_x - x pos of center of text (Normalized position [0.0f - 1.0f])
// pos_y - y pos of center of text (Normalized position [0.0f - 1.0f])
// scale - scale of text (differs from font size)
// color - color of text
// 
// ------------------------------------------------------------------------ 
void render_text_center(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, color color);

// ------------------------------------------------------------------------
// 
// Draws a text unto screen (Position is in bottom right). Unaffected by camera
// zoom and translation.
// text  - text to draw
// font  - font of text
// pos_x - x pos of bottom right of text (Normalized position [0.0f - 1.0f])
// pos_y - y pos of bottom right of text (Normalized position [0.0f - 1.0f])
// scale - scale of text (differs from font size)
// color - color of text
// 
// ------------------------------------------------------------------------ 
void render_text_corner(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, color color);

// ------------------------------------------------------------------------
// 
// Draws a text onto bottom right of screen (in yellow). Text/Tooltip to
// print is represented by "curr_tooltip". Update curr_tooltip every frame
// to change text. Default is nothing "".
// font - tooltip font
// 
// ------------------------------------------------------------------------ 
void render_tooltip(s8 font);

// ------------------------------------------------------------------------
// 
// Draws a black overlay onto screen. Used for darkening background for
// UI to be drawn on top.
// alpha - controls how dark the overlay is
// 
// ------------------------------------------------------------------------ 
void draw_black_overlay(f32 alpha);

// ------------------------------------------------------------------------
// 
// Draws a sprite onto screen that is affected by camera zoom, translation,
// light levels and culling.
// rect_mesh - determines center of sprite (center or top left)
// trans_x	 - x pos of center of sprite
// trans_y	 - y pos of center of sprite
// scale_x	 - width of sprite
// scale_y	 - height of sprite
// texture	 - texture of sprite
// color	 - optional color to highlight sprite (affected by light)
// 
// ------------------------------------------------------------------------ 
void render_sprite(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture, color color = { 1.0f,1.0f,1.0f,1.0f });


//rect_mesh - Corner or Center mode (USE RECT MESH)
//texture is loaded outside this function
void render_ui_sprite(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture, color color = { 1.0f,1.0f,1.0f,1.0f });


void render_editor_ui(AEGfxTexture* texture, f32 width, f32 height, f32 alpha,
	f32 trans_x = 0.f, f32 trans_y = 0.f, bool highlighted = false);

void render_editor_tiles(AEGfxTexture* texture, f32 width, f32 height, f32 alpha,
	f32 trans_x = 0.f, f32 trans_y = 0.f, bool highlighted = false);

// Animation for boss
struct Animator;
void render_anim_sprite(f32 x, f32 y, f32 w, f32 h, Animator& anim);

class BossProjectileManager;
void render_boss_projectiles(const BossProjectileManager& manager);
