#include "aura/lvgl_selector.hpp"

// Store autons
static std::vector<LVGLAuton> autons;
static int selected_auton = 0;

// LVGL objects
static lv_obj_t* main_label;


//default colours
static LVGLTheme theme = {
    lv_color_hex(0x111111), // background
    lv_color_hex(0x1f1f1f), // header
    lv_color_hex(0x2a2a2a), // button
    lv_color_hex(0x00aaff), // selected button
    lv_color_hex(0xffffff)  // text
};

// Button event
void auton_btn_event(lv_event_t* e) {
    int index = (int)lv_event_get_user_data(e);
    selected_auton = index;

    // Update label
    if (main_label != nullptr) {
        lv_label_set_text_fmt(main_label, "Selected: %s", autons[index].name);
    }

    // Highlight selected button
    lv_obj_t* cont = lv_obj_get_parent(lv_event_get_target(e));
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);

    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t* btn = lv_obj_get_child(cont, i);

        if (i == index) {
            lv_obj_set_style_bg_color(btn, theme.button_selected, 0); // selected
        } else {
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x2a2a2a), 0); // normal
        }
    }
}

// Set auton list
void lvgl_selector_set_autons(std::vector<LVGLAuton> list) {
    autons = list;
}

// Initialize LVGL UI
void lvgl_selector_init() {
    lv_obj_t* screen = lv_scr_act();

    // ===== BACKGROUND =====
    lv_obj_set_style_bg_color(screen, theme.background, 0);

    // ===== HEADER BAR =====
    lv_obj_t* header = lv_obj_create(screen);
    lv_obj_set_size(header, 480, 40);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, theme.header, 0);
    lv_obj_set_style_border_width(header, 0, 0);

    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "Autonomous Selector");
    lv_obj_center(title);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);

    // ===== SELECTED LABEL =====
    main_label = lv_label_create(screen);
    lv_label_set_text(main_label, "Selected: None");
    lv_obj_align(main_label, LV_ALIGN_TOP_MID, 0, 50);

    // ===== BUTTON CONTAINER (FLEX) =====
    lv_obj_t* cont = lv_obj_create(screen);
    lv_obj_set_size(cont, 460, 160);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_obj_set_style_pad_all(cont, 10, 0);
    lv_obj_set_style_border_width(cont, 0, 0);

    // FLEX SETTINGS
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont,
        LV_FLEX_ALIGN_SPACE_EVENLY,   // horizontal spacing
        LV_FLEX_ALIGN_CENTER,         // vertical alignment
        LV_FLEX_ALIGN_CENTER          // row alignment
    );

    // ===== CREATE BUTTONS =====
    for (int i = 0; i < autons.size(); i++) {
        lv_obj_t* btn = lv_btn_create(cont);

        // Size (acts like grid)
        lv_obj_set_size(btn, 200, 50);

        // Style
        lv_obj_set_style_bg_color(btn, theme.button, 0);
        lv_obj_set_style_radius(btn, 10, 0);

        // Event
        lv_obj_add_event_cb(btn, auton_btn_event, LV_EVENT_CLICKED, (void*)i);

        // Label
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, autons[i].name);
        lv_obj_center(label);
    }
}

// Run selected auton
void lvgl_selector_run_selected() {
    if (autons.size() > 0) {
        autons[selected_auton].fn();
    }
}

//set the all the colours
//run before innitalise
void lvgl_selector_set_theme(const LVGLTheme& new_theme) {
    theme = new_theme;
}