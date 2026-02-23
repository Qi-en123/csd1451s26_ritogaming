#include "AEEngine.h"
#include "cstdio"
static s8  credit_font = -1;
static f32 scrollY = -1.3f;
static bool finishedScrolling = false;
static f32 scrollSpeed = 0.25f;          // normal speed
static f32 fastScrollMultiplier = 3;     // how much faster when holding space
static f32 totalHeight = 0.f;

static const char* credit_lines[] =
{
    "Team Rito Gaming",
    "Team Members:",
    "DIRECTOR and PROGRAMMER",
    "Ahmad Qusyairi",
    "PRODUCER and PROGRAMMER",
    "Claire Cheong",
    "PRODUCER, DESIGN LEAD and PROGRAMMER",
    "Goh Qi En",
    "AUDIO LEAD, COMPOSER, and PROGRAMMER",
    "Jerod Lim",
    " ",
    "Faculty and Advisors",
    "Lab management and Instructor",
    "MR Gerald Wong",
    "MR Tommy Tan",
    "DR Soroor",
    " ",
    "Created at",
    "DigiPen Institute of Technology Singapore",
    "PRESIDENT",
    "Claude COMAIR",
    "EXECUTIVES",
    "Claude COMAIR CHU Jason Yeu Tat Michael GATS",
    "TAN Chek Ming Prasanna Kumar GHALI",
    "Mandy WONG Johnny DEEK",
    " ",
    "Copyrights for software, tools and libraries",
    "(C) Kenney Assets",
    "(C) MetaDigger",
    NULL
};

void credit_load()
{
    // Load font from Assets folder
    credit_font = AEGfxCreateFont("Assets/Exo2-Regular.ttf", 32);
    printf("Font value: %d\n", credit_font);
    if (credit_font == -1)
    {
        printf("hi");
    }
}

void credit_initialize()
{
    scrollY = -1.3f;  // start below screen

    f32 lineSpacing = 0.12f;

    int lineCount = 0;
    while (credit_lines[lineCount] != NULL) lineCount++;
    printf("Total lines: %d\n", lineCount);

    totalHeight = lineCount * lineSpacing;
}

void credit_update()
{
    // Default speed
    f32 speed = 0.25f;

    // Fast scroll when space is held
    if (AEInputCheckCurr(AEVK_SPACE))
        speed *= 3.f;

    scrollY += speed * AEFrameRateControllerGetFrameTime();

    // Restart after the last line has fully moved above the top
    if (scrollY > 1.f + totalHeight)  // 1.f is top of normalized coordinates
        scrollY = -1.3f;              // reset below screen
}

void credit_draw()
{
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.05f);

    f32 y = scrollY;
    const f32 baseLineSpacing = 0.18f;
    const f32 baseScale = 2.0f;
    const f32 minScale = 0.05f;

    // perspective factor (how fast text shrinks toward top)
    const f32 perspective = 1.6f;

    for (int i = 0; credit_lines[i] != NULL; ++i)
    {
        float r = 1.f, g = 0.85f, b = 0.3f; // all yellow

        // Normalize t from 0 (bottom) to 1 (top of crawl)
        f32 t = (y + 1.3f) / (1.5f + totalHeight);
        if (t > 1.f) t = 1.f;
        if (t < 0.f) t = 0.f;

        // scale shrinks faster toward top
        f32 scale = baseScale * powf(1.f - t, perspective);
        if (scale < minScale) scale = minScale;

        // Perspective tilt: y gets smaller upward to simulate receding
        f32 yOffset = y * (1.f - t * 0.5f); // tweak 0.5 for tilt angle

        f32 textWidth, textHeight;
        AEGfxGetPrintSize(credit_font, credit_lines[i], scale, &textWidth, &textHeight);

        // Draw centered horizontally
        AEGfxPrint(credit_font,
            credit_lines[i],
            -textWidth / 2,
            yOffset,
            scale,
            r, g, b, 1.f);

        y -= baseLineSpacing * scale; // line spacing shrinks as scale shrinks
    }
}

void credit_free()
{
}

void credit_unload()
{
    if (credit_font != -1)
    {
        AEGfxDestroyFont(credit_font);
        credit_font = -1;
    }
}