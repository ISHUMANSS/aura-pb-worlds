#pragma once
#include "main.h"
#include <vector>

// Function pointer type for autons
typedef void (*auton_fn)();

struct LVGLAuton {
    const char* name;
    auton_fn fn;
};

struct LVGLTheme {
    lv_color_t background;
    lv_color_t header;
    lv_color_t button;
    lv_color_t button_selected;
    lv_color_t text;
};

// Public functions
void lvgl_selector_init();
void lvgl_selector_set_autons(std::vector<LVGLAuton> list);
void lvgl_selector_run_selected();
void lvgl_selector_set_theme(const LVGLTheme& theme);