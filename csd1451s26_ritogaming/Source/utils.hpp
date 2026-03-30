#pragma once
#include "AEEngine.h"
#include <string>

int IsButtonClicked(f32 rectX, f32 rectY, f32 rectW, f32 rectH);
int IsButtonHovered(f32 rectX, f32 rectY, f32 rectW, f32 rectH);
extern f32 dt;
extern std::vector<std::string> level_paths;
extern std::vector<std::string> level_puzzle_paths;

extern int current_level_index;
extern std::string selected_level_path;
extern std::string selected_level_puzzle_path;


// Returns true if "Yes" was clicked, false if "No" was clicked, -1 if neither
int draw_exit_confirm(AEGfxTexture* panel, AEGfxTexture* btn_yes, AEGfxTexture* btn_no, s8 font, const char* message, f32 font_scale = 1.f);