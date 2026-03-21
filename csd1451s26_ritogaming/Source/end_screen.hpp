#pragma once

extern bool is_win;
extern bool is_paused;
extern int restart_level_state;

void end_screen_load();
void end_screen_unload();
void end_screen_free();
void end_screen_init();
void end_screen_update();
void end_screen_draw();

void pause_update();
void pause_draw();