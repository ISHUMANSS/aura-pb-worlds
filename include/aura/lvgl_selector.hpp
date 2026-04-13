#pragma once
#include "main.h"
#include <vector>

typedef void (*auton_fn)();

struct LVGLWaypoint {
    float x;// field X in inches (-72 to 72)
    float y;// field Y in inches (-72 to 72)
    bool reverse = false; // true if driving backwards to this point
};

struct LVGLAuton {
    const char* name;
    const char* description;
    auton_fn fn;
    std::vector<LVGLWaypoint> waypoints;
};

struct LVGLTheme {
    lv_color_t background; //back ground behind the display
    lv_color_t panel; //all colours for the pannels
    lv_color_t button; //default button colour
    lv_color_t button_selected; //picked button
    lv_color_t accent; //i forgor :3
    lv_color_t reverse_path; //reverse path line colour
    lv_color_t text;
    lv_color_t text_muted;
};

void lvgl_selector_init();
void lvgl_selector_set_autons(std::vector<LVGLAuton> list);
void lvgl_selector_run_selected();
void lvgl_selector_set_theme(const LVGLTheme& theme);