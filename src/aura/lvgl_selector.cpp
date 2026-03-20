#include "aura/lvgl_selector.hpp"

static std::vector<LVGLAuton> autons;
static int selected_auton = 0;

static LVGLTheme theme = {
    lv_color_hex(0x111111),
    lv_color_hex(0x1a1a1a),
    lv_color_hex(0x2a2a2a),
    lv_color_hex(0xcc2222),
    lv_color_hex(0xff4444),
    lv_color_hex(0xffffff),
    lv_color_hex(0x888888),
};

static lv_obj_t* btn_list     = nullptr;
static lv_obj_t* desc_label   = nullptr;
static lv_obj_t* field_canvas = nullptr;

// FIX 3: LV_COLOR_DEPTH 32 = 4 bytes per pixel
static uint8_t canvas_buf[144 * 144 * 4];

static lv_coord_t field_px(float inch) {
    return (lv_coord_t)(inch + 72.0f);
}

static void draw_field(int auton_idx) {
    if (field_canvas == nullptr) return;

    uint32_t child_cnt = lv_obj_get_child_cnt(field_canvas);
    for (uint32_t i = child_cnt; i > 0; i--) {
        lv_obj_del(lv_obj_get_child(field_canvas, i - 1));
    }

    lv_canvas_fill_bg(field_canvas, lv_color_hex(0x1a2a1a), LV_OPA_COVER);

    // Grid tiles
    lv_draw_rect_dsc_t tile_dsc;
    lv_draw_rect_dsc_init(&tile_dsc);
    tile_dsc.bg_opa       = LV_OPA_TRANSP;
    tile_dsc.border_color = lv_color_hex(0x2a3a2a);
    tile_dsc.border_width = 1;
    tile_dsc.border_opa   = LV_OPA_50;
    tile_dsc.radius       = 0;
    for (int col = 0; col < 6; col++)
        for (int row = 0; row < 6; row++)
            lv_canvas_draw_rect(field_canvas, col * 24, row * 24, 24, 24, &tile_dsc);

    // FIX 2: proper 2-point arrays for centre lines
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.color = lv_color_hex(0x3a4a3a);
    line_dsc.width = 1;
    line_dsc.opa   = LV_OPA_70;
    lv_point_t hline[2] = {{0, 72},  {143, 72}};
    lv_point_t vline[2] = {{72, 0},  {72, 143}};
    lv_canvas_draw_line(field_canvas, hline, 2, &line_dsc);
    lv_canvas_draw_line(field_canvas, vline, 2, &line_dsc);

    if (auton_idx < 0 || auton_idx >= (int)autons.size()) return;
    const auto& aut = autons[auton_idx];
    if (aut.waypoints.empty()) return;

    // Start circle
    lv_draw_rect_dsc_t start_dsc;
    lv_draw_rect_dsc_init(&start_dsc);
    start_dsc.bg_opa       = LV_OPA_TRANSP;
    start_dsc.border_color = theme.accent;
    start_dsc.border_width = 2;
    start_dsc.border_opa   = LV_OPA_COVER;
    start_dsc.radius       = LV_RADIUS_CIRCLE;
    lv_canvas_draw_rect(field_canvas,
        field_px(aut.waypoints[0].x) - 4,
        field_px(-aut.waypoints[0].y) - 4,
        8, 8, &start_dsc);

    // Path segments + dots
    lv_draw_line_dsc_t path_dsc;
    lv_draw_line_dsc_init(&path_dsc);
    path_dsc.width = 2;
    path_dsc.opa   = LV_OPA_COVER;

    lv_draw_rect_dsc_t dot_dsc;
    lv_draw_rect_dsc_init(&dot_dsc);
    dot_dsc.bg_opa       = LV_OPA_COVER;
    dot_dsc.border_width = 0;
    dot_dsc.radius       = LV_RADIUS_CIRCLE;
    dot_dsc.bg_color     = theme.accent;

    for (int i = 1; i < (int)aut.waypoints.size(); i++) {
        const auto& from = aut.waypoints[i - 1];
        const auto& to   = aut.waypoints[i];

        path_dsc.color = to.reverse ? lv_color_hex(0xff8888) : theme.accent;
        lv_point_t seg[2] = {
            {field_px(from.x), field_px(-from.y)},
            {field_px(to.x),   field_px(-to.y)}
        };
        lv_canvas_draw_line(field_canvas, seg, 2, &path_dsc);

        lv_coord_t dx = field_px(to.x) - 3;
        lv_coord_t dy = field_px(-to.y) - 3;
        lv_canvas_draw_rect(field_canvas, dx, dy, 6, 6, &dot_dsc);

        // FIX 4: lv_canvas_draw_text doesn't exist in LVGL 8.3 — use a label widget
        if (to.label != nullptr) {
            lv_obj_t* lbl = lv_label_create(field_canvas);
            lv_label_set_text(lbl, to.label);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_10, 0);
            lv_obj_set_style_text_color(lbl, theme.text, 0);
            lv_obj_set_style_bg_opa(lbl, LV_OPA_TRANSP, 0);
            lv_obj_set_pos(lbl, dx + 5, dy - 10);
        }
    }
}

static void auton_btn_event(lv_event_t* e) {
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    selected_auton = idx;

    if (desc_label) {
        lv_label_set_text(desc_label,
            (autons[idx].description && autons[idx].description[0])
                ? autons[idx].description
                : "No description.");
    }

    draw_field(idx);

    if (btn_list) {
        uint32_t n = lv_obj_get_child_cnt(btn_list);
        for (uint32_t i = 0; i < n; i++) {
            lv_obj_t* btn = lv_obj_get_child(btn_list, i);
            bool sel = ((int)i == idx);
            lv_obj_set_style_bg_color(btn,
                sel ? theme.button_selected : theme.button, 0);
            lv_obj_set_style_border_color(btn,
                sel ? theme.accent : lv_color_hex(0x333333), 0);
        }
    }
}

void lvgl_selector_set_autons(std::vector<LVGLAuton> list) {
    autons = list;
}

void lvgl_selector_set_theme(const LVGLTheme& t) {
    theme = t;
}

void lvgl_selector_init() {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, theme.background, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    // Header
    lv_obj_t* hdr = lv_obj_create(scr);
    lv_obj_set_size(hdr, 480, 32);
    lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(hdr, theme.panel, 0);
    lv_obj_set_style_border_width(hdr, 0, 0);
    lv_obj_set_style_pad_all(hdr, 0, 0);

    lv_obj_t* hdr_title = lv_label_create(hdr);
    lv_label_set_text(hdr_title, "Autonomous Selector");
    lv_obj_set_style_text_font(hdr_title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(hdr_title, theme.text, 0);
    lv_obj_center(hdr_title);

    // Left panel
    lv_obj_t* left = lv_obj_create(scr);
    lv_obj_set_size(left, 200, 208);
    lv_obj_align(left, LV_ALIGN_TOP_LEFT, 4, 36);
    lv_obj_set_style_bg_color(left, theme.panel, 0);
    lv_obj_set_style_bg_opa(left, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(left, 0, 0);
    lv_obj_set_style_pad_all(left, 4, 0);
    lv_obj_set_style_pad_row(left, 4, 0);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(left, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(left, LV_SCROLLBAR_MODE_AUTO);
    btn_list = left;

    for (int i = 0; i < (int)autons.size(); i++) {
        lv_obj_t* btn = lv_btn_create(left);
        lv_obj_set_size(btn, 190, 44);
        lv_obj_set_style_bg_color(btn,
            i == 0 ? theme.button_selected : theme.button, 0);
        lv_obj_set_style_border_color(btn,
            i == 0 ? theme.accent : lv_color_hex(0x333333), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_radius(btn, 6, 0);
        lv_obj_set_style_pad_all(btn, 6, 0);
        lv_obj_add_event_cb(btn, auton_btn_event, LV_EVENT_CLICKED,
                            (void*)(intptr_t)i);

        lv_obj_t* name_lbl = lv_label_create(btn);
        lv_label_set_text(name_lbl, autons[i].name);
        lv_label_set_long_mode(name_lbl, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(name_lbl, 176);
        lv_obj_set_style_text_font(name_lbl, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(name_lbl, theme.text, 0);
        lv_obj_align(name_lbl, LV_ALIGN_TOP_LEFT, 0, 2);

        if (autons[i].description && autons[i].description[0]) {
            lv_obj_t* pre = lv_label_create(btn);
            lv_label_set_text(pre, autons[i].description);
            lv_label_set_long_mode(pre, LV_LABEL_LONG_CLIP);
            lv_obj_set_width(pre, 176);
            lv_obj_set_style_text_font(pre, &lv_font_montserrat_10, 0);
            lv_obj_set_style_text_color(pre, theme.text_muted, 0);
            lv_obj_align(pre, LV_ALIGN_BOTTOM_LEFT, 0, -2);
        }
    }

    // Right panel
    lv_obj_t* right = lv_obj_create(scr);
    lv_obj_set_size(right, 268, 208);
    lv_obj_align(right, LV_ALIGN_TOP_RIGHT, -4, 36);
    lv_obj_set_style_bg_color(right, theme.panel, 0);
    lv_obj_set_style_bg_opa(right, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(right, 0, 0);
    lv_obj_set_style_pad_all(right, 6, 0);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(right, 6, 0);

    field_canvas = lv_canvas_create(right);
    // FIX 3: 4 bytes per pixel for 32-bit colour depth
    lv_canvas_set_buffer(field_canvas, canvas_buf, 144, 144, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_size(field_canvas, 144, 144);
    lv_obj_align(field_canvas, LV_ALIGN_TOP_MID, 0, 0);
    draw_field(0);

    // Legend
    lv_obj_t* legend = lv_obj_create(right);
    lv_obj_set_size(legend, 256, 16);
    lv_obj_set_style_bg_opa(legend, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(legend, 0, 0);
    lv_obj_set_style_pad_all(legend, 0, 0);
    lv_obj_set_flex_flow(legend, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(legend,
        LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(legend, 8, 0);

    auto make_legend_item = [&](lv_color_t colour, const char* txt) {
        lv_obj_t* swatch = lv_obj_create(legend);
        lv_obj_set_size(swatch, 18, 4);
        lv_obj_set_style_bg_color(swatch, colour, 0);
        lv_obj_set_style_bg_opa(swatch, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(swatch, 0, 0);
        lv_obj_t* lbl = lv_label_create(legend);
        lv_label_set_text(lbl, txt);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_color(lbl, theme.text_muted, 0);
    };
    make_legend_item(theme.accent,           "forward");
    make_legend_item(lv_color_hex(0xff8888), "reverse");

    // Description
    desc_label = lv_label_create(right);
    lv_obj_set_size(desc_label, 256, LV_SIZE_CONTENT);
    lv_label_set_long_mode(desc_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(desc_label, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(desc_label, theme.text_muted, 0);
    lv_label_set_text(desc_label,
        (!autons.empty() && autons[0].description)
            ? autons[0].description : "Select an autonomous routine.");

    // Status bar
    lv_obj_t* status = lv_obj_create(scr);
    lv_obj_set_size(status, 480, 20);
    lv_obj_align(status, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(status, theme.panel, 0);
    lv_obj_set_style_border_width(status, 0, 0);
    lv_obj_set_style_pad_all(status, 2, 0);

    lv_obj_t* status_lbl = lv_label_create(status);
    lv_label_set_text(status_lbl, "AURA  |  Press A and B to run auto");
    lv_obj_set_style_text_font(status_lbl, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(status_lbl, theme.text_muted, 0);
    lv_obj_center(status_lbl);
}

void lvgl_selector_run_selected() {
    if (!autons.empty())
        autons[selected_auton].fn();
}