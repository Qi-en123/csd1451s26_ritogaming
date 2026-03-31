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

struct color {
	f32 r, g, b, a;
};

//pos, color, radius
struct light {
	AEVec2 pos;
	color color;
	s32 radius;
	f32 effectiveness;
};

//Colors
extern const color red;
extern const color green;
extern const color blue;
extern const color dark_red;
extern const color white;
extern const color black;
extern const color yellow;

extern std::vector<light> lights;

//Rect pos is at top right corner
extern AEGfxVertexList* rect_mesh_corner;
extern AEGfxVertexList* rect_mesh_center;
extern AEGfxVertexList* circle_mesh;
extern AEGfxVertexList* square_mesh;
extern AEGfxVertexList* triangle_mesh;

extern f32 zoom;
constexpr f32 ZOOM_MAX = 1.65f;
constexpr f32 ZOOM_MIN = 0.8f;

extern const char* curr_tooltip;

//Shortcut, js pass in color struct
void AEGfxSetColorToAdd(const color& c);

void AEGfxSetColorToMultiply(const color& c);

//Initializes rect_mesh_corner, rect_mesh_center & circle_mesh
void init_mesh();

void free_mesh();

//circle_mesh - Corner or Center mode.
//trans_x - translate x
//trans_y - translate y
//color is set outside this function, before it is called
void draw_circle(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool zoom = true);

//rect_mesh - Corner or Center mode.
//trans_x - translate x
//trans_y - translate y
//color is set outside this function, before it is called
void draw_rect_center(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool zoom = true);

void draw_rect_corner(f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, bool zoom = true);

/**************************************************************************
	\brief
	Draws a square mesh at the specified position and scale.

	Uses the square mesh and applies camera zoom, translation,
	and scaling before rendering.

	\param trans_x X-axis translation of the square.
	\param trans_y Y-axis translation of the square.
	\param scale_x X-axis scale of the square.
	\param scale_y Y-axis scale of the square.
**************************************************************************/
void draw_square(f32 trans_x, f32 trans_y, f32 scale_x, f32 scale_y, bool zoom = true);

/**************************************************************************
	\brief
	Draws a triangle mesh at the specified position and scale.

	Applies camera zoom, translation, and scaling transformations
	before rendering the triangle mesh.

	\param trans_x X-axis translation of the triangle.
	\param trans_y Y-axis translation of the triangle.
	\param scale_x X-axis scale of the triangle.
	\param scale_y Y-axis scale of the triangle.
**************************************************************************/
void draw_triangle(f32 trans_x, f32 trans_y, f32 scale_x, f32 scale_y, bool zoom = true);

/**************************************************************************
	\brief
	Renders a square sprite using the given texture.

	Applies camera zoom, translation, and scaling transformations
	before drawing the square mesh with the specified texture.

	\param trans_x X-axis translation of the sprite.
	\param trans_y Y-axis translation of the sprite.
	\param scale_x X-axis scale of the sprite.
	\param scale_y Y-axis scale of the sprite.
	\param texture Pointer to the texture to apply to the sprite.
**************************************************************************/
void render_square_sprite(f32 trans_x, f32 trans_y, f32 scale_x, f32 scale_y,
	AEGfxTexture* texture);

//rect_mesh - Corner or Center mode (USE RECT MESH)
//texture is loaded outside this function
void render_sprite(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture, color color = { 1.0f,1.0f,1.0f,1.0f });

//rect_mesh - Corner or Center mode (USE RECT MESH)
//texture is loaded outside this function
void render_ui_sprite(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture, color color = { 1.0f,1.0f,1.0f,1.0f });


//pos_x - posX in normalized coords (-1.0f to 1.0f)
//pos_y - posY in normalized coords (-1.0f to 1.0f)
//Text pos is in the middle
void render_text_center(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, color color);

//pos_x - posX in normalized coords (-1.0f to 1.0f)
//pos_y - posY in normalized coords (-1.0f to 1.0f)
//Text pos is at btm right corner
void render_text_corner(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, color color);

void get_zoom(f32 dt, f32 zoom_min = ZOOM_MIN, f32 zoom_max = ZOOM_MAX);


void render_editor_ui(AEGfxTexture* texture, f32 width, f32 height, f32 alpha,
	f32 trans_x = 0.f, f32 trans_y = 0.f, bool highlighted = false);

void render_editor_tiles(AEGfxTexture* texture, f32 width, f32 height, f32 alpha,
	f32 trans_x = 0.f, f32 trans_y = 0.f, bool highlighted = false);

void draw_black_overlay(f32 alpha);
void render_tooltip(s8 font);
