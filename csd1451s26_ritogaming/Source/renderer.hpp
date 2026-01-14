/*!************************************************************************
\file:			renderr.hpp
\author:		Jerod
\par DP email:	jerodyaowei.lim@digipen.edu
\brief:			This header declares rendering-related data structures
				and drawing functions using AEEngine.
**************************************************************************/
#pragma once
#include "AEEngine.h"

typedef struct Color {
	f32 r, g, b, a;
}Color;

extern const Color red;
extern const Color green;
extern const Color blue;
extern const Color dark_red;
extern const Color white;
extern const Color black;

constexpr f32 anim_duration_per_frame = 0.1f;

//Rect pos is at top right corner
extern AEGfxVertexList* rectMeshCorner;
extern AEGfxVertexList* rectMeshCenter;
extern AEGfxVertexList* circleMesh;

extern f32 zoom;

//Shortcut, js pass in color struct
void AEGfxSetColorToAdd(const Color& c);

//Initializes rectMeshCorner, rectMeshCenter & circleMesh
void initMesh();

void freeMesh();

//circleMesh - Corner or Center mode.
//trans_x - translate x
//trans_y - translate y
//Color is set outside this function, before it is called
void drawCircle(AEGfxVertexList* circleMesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y);

//rectMesh - Corner or Center mode.
//trans_x - translate x
//trans_y - translate y
//Color is set outside this function, before it is called
void drawRect(AEGfxVertexList* rectMesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y);

//rectMesh - Corner or Center mode (USE RECT MESH)
//texture is loaded outside this function
void renderSprite(AEGfxVertexList* rectMesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture);

//pos_x - posX in normalized coords (-1.0f to 1.0f)
//pos_y - posY in normalized coords (-1.0f to 1.0f)
//Text pos is in the middle
void renderTextCenter(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, Color color);

//pos_x - posX in normalized coords (-1.0f to 1.0f)
//pos_y - posY in normalized coords (-1.0f to 1.0f)
//Text pos is at btm right corner
void renderTextCorner(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, Color color);

void getZoom(f32 dt);