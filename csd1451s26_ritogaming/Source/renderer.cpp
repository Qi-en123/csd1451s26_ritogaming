/*!************************************************************************
\file:						render.cpp
\author:					Jerod
\par DP email:				jerodyaowei.lim@digipen.edu
\co-author:					Claire
\par co-author DP email:	c.cheong@digipen.edu
\brief:						This file handles mesh initialization and
							rendering of shapes,sprites, and text, including
							camera zoom support using AEEngine.
**************************************************************************/
#include "renderer.hpp"
#include "AEEngine.h"
#include "camera.hpp"
#include "map.hpp"

#define ZOOM_MAX 1.65f
#define ZOOM_MIN 0.8f

AEGfxVertexList* rect_mesh_corner = nullptr;
AEGfxVertexList* rect_mesh_center = nullptr;
AEGfxVertexList* circle_mesh = nullptr;
AEGfxVertexList* square_mesh = nullptr;
AEGfxVertexList* triangle_mesh = nullptr;


const color red = { 1.0f, 0.0f,0.0f,0.0f };
const color green = { 0.0f, 1.0f, 0.0f, 0.0f };
const color blue = { 0.0f, 0.0f, 1.0f, 0.0f };
const color dark_red = { 0.5f, 0.0f, 0.0f, 0.0f };
const color black = { 0.0f, 0.0f, 0.0f, 0.0f };
const color white = { 1.0f, 1.0f, 1.0f, 0.0f };

//Shortcut, js pass in color struct
void AEGfxSetColorToAdd(const color& c)
{
	AEGfxSetColorToAdd(c.r, c.g, c.b, c.a);
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

void init_mesh() {
	//---------------- Rect Mesh Corner ----------------//
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
	rect_mesh_corner = AEGfxMeshEnd();

	//---------------- Rect Mesh Center ----------------//
	AEGfxMeshStart();
	AEGfxTriAdd( //1st tri
		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
	);

	AEGfxTriAdd( //2nd tri
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
	);
	rect_mesh_center = AEGfxMeshEnd();

	//---------------- Circle Mesh ----------------//
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
	circle_mesh = AEGfxMeshEnd();
	square_mesh = create_square_mesh();
	triangle_mesh = create_triangle_mesh();
}

void free_mesh() {
	AEGfxMeshFree(circle_mesh);
	AEGfxMeshFree(rect_mesh_corner);
	AEGfxMeshFree(rect_mesh_center);
	AEGfxMeshFree(square_mesh);
	AEGfxMeshFree(triangle_mesh);
}

bool on_screen(f32 x, f32 y) {
	return (x < cam.pos.x + (AEGfxGetWindowWidth() / 2 / zoom) + tile_size &&
		x > cam.pos.x - (AEGfxGetWindowWidth() / 2 / zoom) - tile_size &&
		y < cam.pos.y + (AEGfxGetWindowHeight() / 2 / zoom) + tile_size &&
		y > cam.pos.y - (AEGfxGetWindowHeight() / 2 / zoom) - tile_size);
}

void draw_circle(AEGfxVertexList* circleMesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y) {
	AEMtx33 transform;
	AEMtx33Identity(&transform);

	AEMtx33 translate;
	AEMtx33Trans(&translate, trans_x, trans_y);

	AEMtx33 scale;
	AEMtx33Scale(&scale, scale_x, scale_y);

	AEMtx33 cam_zoom;
	AEMtx33Scale(&cam_zoom, zoom, zoom);

	AEMtx33 cam_translate;
	AEMtx33Trans(&translate, trans_x, trans_y);
	AEMtx33Trans(&cam_translate, -cam.pos.x, -cam.pos.y);

	AEMtx33Concat(&transform, &transform, &cam_zoom);
	AEMtx33Concat(&transform, &transform, &cam_translate);
	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);

	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(circle_mesh, AE_GFX_MDM_TRIANGLES);

}

void draw_rect(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y) {
	AEMtx33 transform;
	AEMtx33Identity(&transform);

	AEMtx33 translate;
	AEMtx33Trans(&translate, trans_x, trans_y);

	AEMtx33 cam_translate;
	AEMtx33Trans(&cam_translate, -cam.pos.x, -cam.pos.y);

	AEMtx33 scale;
	AEMtx33Scale(&scale, scale_x, scale_y);

	AEMtx33 cam_zoom;
	AEMtx33Scale(&cam_zoom, zoom, zoom);

	AEMtx33Concat(&transform, &transform, &cam_zoom);
	AEMtx33Concat(&transform, &transform, &cam_translate);

	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);

	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(rect_mesh, AE_GFX_MDM_TRIANGLES);
}

void draw_square(f32 trans_x, f32 trans_y, f32 scale_x, f32 scale_y) {
	draw_rect(square_mesh, trans_x, trans_y, scale_x, scale_y);
}

void draw_triangle(f32 trans_x, f32 trans_y, f32 scale_x, f32 scale_y) {
	AEMtx33 transform;
	AEMtx33Identity(&transform);

	AEMtx33 translate;
	AEMtx33Trans(&translate, trans_x, trans_y);
	
	AEMtx33 cam_translate;
	AEMtx33Trans(&cam_translate, -cam.pos.x, -cam.pos.y);

	AEMtx33 scale;
	AEMtx33Scale(&scale, scale_x, scale_y);

	AEMtx33 cam_zoom;
	AEMtx33Scale(&cam_zoom, zoom, zoom);

	AEMtx33Concat(&transform, &transform, &cam_zoom);
	AEMtx33Concat(&transform, &transform, &cam_translate);

	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);

	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(triangle_mesh, AE_GFX_MDM_TRIANGLES);
}

void render_square_sprite(f32 trans_x, f32 trans_y, f32 scale_x, f32 scale_y,
	AEGfxTexture* texture) {
	render_sprite(square_mesh, trans_x, trans_y, scale_x, scale_y, texture);
}

void render_sprite(AEGfxVertexList* rect_mesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture) {
	//CULLING
	if (on_screen(trans_x, trans_y)) {
		AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
		AEMtx33 transform;
		AEMtx33Identity(&transform);

		AEMtx33 translate;
		AEMtx33Trans(&translate, trans_x, trans_y);

		AEMtx33 scale;
		AEMtx33Scale(&scale, scale_x, scale_y);

		AEMtx33 cam_translate;
		AEMtx33Trans(&cam_translate, -cam.pos.x, -cam.pos.y);

		AEMtx33 cam_zoom;
		AEMtx33Scale(&cam_zoom, zoom, zoom);

		AEMtx33Concat(&transform, &transform, &cam_zoom);
		AEMtx33Concat(&transform, &transform, &cam_translate);

		AEMtx33Concat(&transform, &transform, &translate);
		AEMtx33Concat(&transform, &transform, &scale);

		//AEMtx33Concat(&transform, &translate, &scale);
		AEGfxTextureSet(texture, 0, 0);
		AEGfxSetTransform(transform.m);
		AEGfxMeshDraw(rect_mesh, AE_GFX_MDM_TRIANGLES);
		AEGfxTextureSet(nullptr, 0, 0);
	}
}

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

void get_zoom(f32 dt) {
	s32 p_delta;
	AEInputMouseWheelDelta(&p_delta);
	if (p_delta > 0) {
		zoom += dt * 2.0f;
	}
	else if (p_delta < 0) {
		//scroll down
		zoom -= dt * 2.0f;
	}
	zoom = AEClamp(zoom, ZOOM_MIN, ZOOM_MAX);
}
