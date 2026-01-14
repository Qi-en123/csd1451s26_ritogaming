#include "renderer.hpp"
#include "AEEngine.h"

AEGfxVertexList* rectMeshCorner = nullptr;
AEGfxVertexList* rectMeshCenter = nullptr;
AEGfxVertexList* circleMesh = nullptr;

const Color red = { 1.0f, 0.0f,0.0f,0.0f };
const Color green = { 0.0f, 1.0f, 0.0f, 0.0f };
const Color blue = { 0.0f, 0.0f, 1.0f, 0.0f };
const Color dark_red = { 0.5f, 0.0f, 0.0f, 0.0f };
const Color black = { 0.0f, 0.0f, 0.0f, 0.0f };
const Color white = { 1.0f, 1.0f, 1.0f, 0.0f };

//Shortcut, js pass in color struct
void AEGfxSetColorToAdd(const Color& c)
{
	AEGfxSetColorToAdd(c.r, c.g, c.b, c.a);
}

void initMesh() {
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
	rectMeshCorner = AEGfxMeshEnd();

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
	rectMeshCenter = AEGfxMeshEnd();

	//---------------- Circle Mesh ----------------//
	AEGfxMeshStart();
	//how many triangles to make up the circle
	s32 triCount = 128;
	f32 theta = 360.0f / triCount;
	for (int i = 0; i < triCount; ++i) {
		f32 curr_angle = theta * i;
		//x = r*cos0 || y = r*sin0 /r=1
		f32 curr_x = (f32)cos(curr_angle * PI / 180.0f); //convert to rad
		f32 curr_y = (f32)sin(curr_angle * PI / 180.0f);

		f32 next_x = (f32)cos((curr_angle + theta) * PI / 180.0f);
		f32 next_y = (f32)sin((curr_angle + theta) * PI / 180.0f);

		//printf("curr: %f %f %f, next %f %f %f\n",
		// curr_angle, curr_x, curr_y,
		// curr_angle + theta, next_x, next_y);

		AEGfxTriAdd( //1st tri
			0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 0.0f, //mid-point
			curr_x, curr_y, 0xFFFFFFFF, curr_x, curr_y,
			next_x, next_y, 0xFFFFFFFF, next_x, next_y
		);

	}
	circleMesh = AEGfxMeshEnd();
}

void freeMesh() {
	AEGfxMeshFree(circleMesh);
	AEGfxMeshFree(rectMeshCorner);
	AEGfxMeshFree(rectMeshCenter);
}

void drawCircle(AEGfxVertexList* circleMesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y) {
	AEMtx33 transform;
	AEMtx33Identity(&transform);

	AEMtx33 translate;
	AEMtx33Trans(&translate, trans_x, trans_y);

	AEMtx33 scale;
	AEMtx33Scale(&scale, scale_x, scale_y);

	AEMtx33 cam_zoom;
	AEMtx33Scale(&cam_zoom, zoom, zoom);

	AEMtx33Concat(&transform, &transform, &cam_zoom);
	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);
	//AEMtx33Concat(&transform, &translate, &scale);
	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(circleMesh, AE_GFX_MDM_TRIANGLES);
}

void drawRect(AEGfxVertexList* rectMesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y) {
	AEMtx33 transform;
	AEMtx33Identity(&transform);

	AEMtx33 translate;
	AEMtx33Trans(&translate, trans_x, trans_y);

	AEMtx33 scale;
	AEMtx33Scale(&scale, scale_x, scale_y);

	AEMtx33 cam_zoom;
	AEMtx33Scale(&cam_zoom, zoom, zoom);

	AEMtx33Concat(&transform, &transform, &cam_zoom);
	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);
	//AEMtx33Concat(&transform, &translate, &scale);
	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
}

void renderSprite(AEGfxVertexList* rectMesh, f32 trans_x, f32 trans_y,
	f32 scale_x, f32 scale_y, AEGfxTexture* texture) {
	AEMtx33 transform;
	AEMtx33Identity(&transform);

	AEMtx33 translate;
	AEMtx33Trans(&translate, trans_x, trans_y);

	AEMtx33 scale;
	AEMtx33Scale(&scale, scale_x, scale_y);

	AEMtx33 cam_zoom;
	AEMtx33Scale(&cam_zoom, zoom, zoom);

	AEMtx33Concat(&transform, &transform, &cam_zoom);
	AEMtx33Concat(&transform, &transform, &translate);
	AEMtx33Concat(&transform, &transform, &scale);

	//AEMtx33Concat(&transform, &translate, &scale);
	AEGfxTextureSet(texture, 0, 0);
	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(rectMesh, AE_GFX_MDM_TRIANGLES);
	AEGfxTextureSet(nullptr, 0, 0);
}

void renderTextCenter(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, Color color) {

	f32 width, height;
	AEGfxGetPrintSize(font, text, scale, &width, &height);
	AEGfxPrint(font, text, pos_x-(width/2.0f), pos_y - (height/2.0f),
		scale, color.r, color.g, color.b, 1.0f);
	AEGfxTextureSet(nullptr, 0, 0);
}

void renderTextCorner(char const* text, s8 font, f32 pos_x, f32 pos_y,
	f32 scale, Color color) {

	AEGfxPrint(font, text, pos_x, pos_y,
		scale, color.r, color.g, color.b, 1.0f);
	AEGfxTextureSet(nullptr, 0, 0);
}