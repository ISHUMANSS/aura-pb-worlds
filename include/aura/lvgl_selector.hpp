#pragma once
#include "main.h"
#include <vector>

typedef void (*auton_fn)();

struct LVGLWaypoint {
    float x;          // field X in inches (-72 to 72)
    float y;          // field Y in inches (-72 to 72)
    bool  reverse;    // true if driving backwards to this point
    const char* label; // optional, nullptr to skip
};

struct LVGLAuton {
    const char*              name;
    const char*              description;   // shown in right panel
    auton_fn                 fn;
    std::vector<LVGLWaypoint> waypoints;    // empty = no path drawn
};

struct LVGLTheme {
    lv_color_t background;
    lv_color_t panel;           // replaces old header/button
    lv_color_t button;
    lv_color_t button_selected;
    lv_color_t accent;          // path line + waypoint dots
    lv_color_t text;
    lv_color_t text_muted;
};

void lvgl_selector_init();
void lvgl_selector_set_autons(std::vector<LVGLAuton> list);
void lvgl_selector_run_selected();
void lvgl_selector_set_theme(const LVGLTheme& theme);