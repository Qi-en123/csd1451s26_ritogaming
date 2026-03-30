#include "AEEngine.h"
#include <string>
#include <vector>
#include "renderer.hpp"

std::vector<std::string> level_paths;
std::vector<std::string> level_puzzle_paths;
int current_level_index = 0;
std::string selected_level_path = "";
std::string selected_level_puzzle_path = "";
f32 dt;

int IsButtonClicked(f32 rectX, f32 rectY, f32 rectW, f32 rectH)
{
    // Get mouse position in screen coordinates
    s32 mouse_x, mouse_y;
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    // Convert button world position to screen coordinates
    // rectX and rectY are world coords (centered), convert to screen space
    f32 screen_x = rectX + 800.f - rectW / 2.f; // center origin to top-left
    f32 screen_y = -rectY + 450.f - rectH / 2.f; // flip Y axis

    // Check if mouse is inside button bounds
    if ((f32)mouse_x >= screen_x && (f32)mouse_x <= screen_x + rectW &&
        (f32)mouse_y >= screen_y && (f32)mouse_y <= screen_y + rectH)
    {
        // Check left mouse button click
        if (AEInputCheckTriggered(AEVK_LBUTTON))
            return 1;
    }
    return 0;
}

int IsButtonHovered(f32 rectX, f32 rectY, f32 rectW, f32 rectH)
{
    s32 mouse_x, mouse_y;
    AEInputGetCursorPosition(&mouse_x, &mouse_y);

    f32 screen_x = rectX + 800.f - rectW / 2.f;
    f32 screen_y = -rectY + 450.f - rectH / 2.f;

    if ((f32)mouse_x >= screen_x && (f32)mouse_x <= screen_x + rectW &&
        (f32)mouse_y >= screen_y && (f32)mouse_y <= screen_y + rectH)
        return 1;

    return 0;
}

int draw_exit_confirm(AEGfxTexture* panel, AEGfxTexture* btn_yes, AEGfxTexture* btn_no, s8 font, const char* message, f32 font_scale)
{
    draw_black_overlay(0.6f);
    render_editor_ui(panel, 800.f, 400.f, 1.0f, 0.f, 0.f);
    render_editor_ui(btn_yes, 200.f, 80.f, 1.0f, -100.f, -50.f,
        IsButtonHovered(-100.f, -50.f, 200.f, 80.f));
    render_editor_ui(btn_no, 200.f, 80.f, 1.0f, 150.f, -50.f,
        IsButtonHovered(150.f, -50.f, 200.f, 80.f));

    if (font != -1)
    {
        AEGfxPrint(font, message, -0.40f, 0.1f, font_scale, 0.f, 0.f, 0.f, 1.f);
        AEGfxPrint(font, "Yes", -0.18f, -0.14f, font_scale, 0.f, 0.f, 0.f, 1.f);
        AEGfxPrint(font, "No", 0.15f, -0.14f, font_scale, 0.f, 0.f, 0.f, 1.f);
    }

    if (IsButtonClicked(-100.f, -50.f, 200.f, 80.f)) return 1;
    if (IsButtonClicked(150.f, -50.f, 200.f, 80.f)) return 0;
    return -1;
}