#include "AEEngine.h"

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