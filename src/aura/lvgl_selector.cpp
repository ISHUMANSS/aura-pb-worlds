#include "aura/lvgl_selector.hpp"

LV_IMG_DECLARE(vexfield);

static std::vector<LVGLAuton> autons;
static int selected_auton = 0;

static const int FIELD_SIZE = 160;  // enlarged canvas
static const int FIELD_HALF = FIELD_SIZE / 2;
static const int TILE_SIZE  = FIELD_SIZE / 6;

static LVGLTheme theme = {
    lv_color_hex(0x111111),
    lv_color_hex(0x1a1a1a),
    lv_color_hex(0x2a2a2a),
    lv_color_hex(0xcc2222),
    lv_color_hex(0xff4444),
    lv_color_hex(0xff8888),
    lv_color_hex(0xffffff),
    lv_color_hex(0x888888),
};

static lv_obj_t* btn_list     = nullptr;
static lv_obj_t* desc_label   = nullptr;
static lv_obj_t* field_canvas = nullptr;

// Buffer sized for 160x160 ARGB8888
static uint8_t canvas_buf[160 * 160 * 4];

// Map field inches (-72..72) to canvas pixels (0..159)
static lv_coord_t field_px(float inch) {
    return (lv_coord_t)((inch + 72.0f) / 144.0f * (float)(FIELD_SIZE - 1));
}

static void draw_field(int auton_idx) {
    if (field_canvas == nullptr) return;

    lv_canvas_fill_bg(field_canvas, lv_color_hex(0x000000), LV_OPA_COVER);

    // 1. Draw Field image
    lv_draw_img_dsc_t img_draw_dsc;
    lv_draw_img_dsc_init(&img_draw_dsc);
    img_draw_dsc.opa = LV_OPA_70;
    lv_canvas_draw_img(field_canvas, 0, 0, &vexfield, &img_draw_dsc);

    // 2. Setup Grid Drawing
    lv_draw_rect_dsc_t tile_dsc;
    lv_draw_rect_dsc_init(&tile_dsc);
    tile_dsc.bg_opa = LV_OPA_TRANSP;
    tile_dsc.radius = 0;

    // We want a 24x24 total grid (4 subdivisions per 6 tiles)
    // 160px / 24 = ~6.66px per 6-inch square. 
    // It's cleaner to calculate the pixel position directly for each line.
    int total_subdivisions = 24; 

    for (int i = 0; i <= total_subdivisions; i++) {
        float percent = (float)i / total_subdivisions;
        lv_coord_t pos = (lv_coord_t)(percent * (FIELD_SIZE - 1));

        // Determine line style: 
        // Every 4th line is a "Major" line (the actual 24" foam tile border)
        bool is_major = (i % 4 == 0);

        lv_draw_line_dsc_t grid_line_dsc;
        lv_draw_line_dsc_init(&grid_line_dsc);
        grid_line_dsc.width = 1;
        grid_line_dsc.color = lv_color_hex(0xffffff);
        grid_line_dsc.opa = is_major ? LV_OPA_40 : LV_OPA_10; // Major lines are brighter

        // Horizontal line
        lv_point_t h_pts[2] = {{0, pos}, {FIELD_SIZE - 1, pos}};
        lv_canvas_draw_line(field_canvas, h_pts, 2, &grid_line_dsc);

        // Vertical line
        lv_point_t v_pts[2] = {{pos, 0}, {pos, FIELD_SIZE - 1}};
        lv_canvas_draw_line(field_canvas, v_pts, 2, &grid_line_dsc);
    }

    // 3. Centre crosshair (highlighted in a different color)
    lv_draw_line_dsc_t cross_dsc;
    lv_draw_line_dsc_init(&cross_dsc);
    cross_dsc.color = lv_color_hex(0x00ff00); // Slight green tint for center
    cross_dsc.width = 1;
    cross_dsc.opa = LV_OPA_50;
    lv_coord_t mid = FIELD_SIZE / 2;
    lv_point_t h_mid[2] = {{0, mid}, {FIELD_SIZE - 1, mid}};
    lv_point_t v_mid[2] = {{mid, 0}, {mid, FIELD_SIZE - 1}};
    lv_canvas_draw_line(field_canvas, h_mid, 2, &cross_dsc);

    // 4. Draw Waypoints (Rest of the code remains the same)
    if (auton_idx < 0 || auton_idx >= (int)autons.size()) return;
    const auto& aut = autons[auton_idx];
    if (aut.waypoints.empty()) return;

    // Start circle (larger: 12x12)
    lv_draw_rect_dsc_t start_dsc;
    lv_draw_rect_dsc_init(&start_dsc);
    start_dsc.bg_opa       = LV_OPA_TRANSP;
    start_dsc.border_color = theme.accent;
    start_dsc.border_width = 2;
    start_dsc.border_opa   = LV_OPA_COVER;
    start_dsc.radius       = LV_RADIUS_CIRCLE;
    lv_canvas_draw_rect(field_canvas,
        field_px(aut.waypoints[0].x) - 6,
        field_px(-aut.waypoints[0].y) - 6,
        12, 12, &start_dsc);

    // Path lines + dots
    lv_draw_line_dsc_t path_dsc;
    lv_draw_line_dsc_init(&path_dsc);
    path_dsc.width = 2;
    path_dsc.opa   = LV_OPA_COVER;

    lv_draw_rect_dsc_t dot_dsc;
    lv_draw_rect_dsc_init(&dot_dsc);
    dot_dsc.bg_opa       = LV_OPA_COVER;
    dot_dsc.border_width = 0;
    dot_dsc.radius       = LV_RADIUS_CIRCLE;

    for (int i = 1; i < (int)aut.waypoints.size(); i++) {
        const auto& from = aut.waypoints[i - 1];
        const auto& to   = aut.waypoints[i];

        path_dsc.color = to.reverse ? theme.reverse_path : theme.accent;
        lv_point_t seg[2] = {
            {field_px(from.x), field_px(-from.y)},
            {field_px(to.x),   field_px(-to.y)}
        };
        lv_canvas_draw_line(field_canvas, seg, 2, &path_dsc);

        // Larger dots: 10x10
        dot_dsc.bg_color = to.reverse ? theme.reverse_path : theme.accent;
        lv_coord_t dx = field_px(to.x) - 5;
        lv_coord_t dy = field_px(-to.y) - 5;
        lv_canvas_draw_rect(field_canvas, dx, dy, 10, 10, &dot_dsc);
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

// Truncate name to maxLen chars, appending ".." if clipped
static void truncate_name(const char* src, char* dst, int maxLen) {
    int len = 0;
    while (src[len] && len < 64) len++;
    if (len <= maxLen) {
        for (int i = 0; i <= len; i++) dst[i] = src[i];
    } else {
        for (int i = 0; i < maxLen - 2; i++) dst[i] = src[i];
        dst[maxLen - 2] = '.';
        dst[maxLen - 1] = '.';
        dst[maxLen]     = '\0';
    }
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

    // ── Left panel (narrowed to 148px, name only, no description preview) ──
    lv_obj_t* left = lv_obj_create(scr);
    lv_obj_set_size(left, 148, 208);
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
        lv_obj_set_size(btn, 138, 32);  // shorter height — name only
        lv_obj_set_style_bg_color(btn,
            i == 0 ? theme.button_selected : theme.button, 0);
        lv_obj_set_style_border_color(btn,
            i == 0 ? theme.accent : lv_color_hex(0x333333), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_radius(btn, 6, 0);
        lv_obj_set_style_pad_all(btn, 6, 0);
        lv_obj_add_event_cb(btn, auton_btn_event, LV_EVENT_CLICKED,
                            (void*)(intptr_t)i);

        // Truncate to 12 visible characters
        char trunc[16];
        truncate_name(autons[i].name, trunc, 12);

        lv_obj_t* name_lbl = lv_label_create(btn);
        lv_label_set_text(name_lbl, trunc);
        lv_label_set_long_mode(name_lbl, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(name_lbl, 126);
        lv_obj_set_style_text_font(name_lbl, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(name_lbl, theme.text, 0);
        lv_obj_center(name_lbl);
    }

    // ── Right panel (widened to 316px, taller canvas) ──
    lv_obj_t* right = lv_obj_create(scr);
    lv_obj_set_size(right, 316, 208);
    lv_obj_align(right, LV_ALIGN_TOP_RIGHT, -4, 36);
    lv_obj_set_style_bg_color(right, theme.panel, 0);
    lv_obj_set_style_bg_opa(right, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(right, 0, 0);
    lv_obj_set_style_pad_all(right, 6, 0);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right,
        LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(right, 4, 0);

    field_canvas = lv_canvas_create(right);
    lv_canvas_set_buffer(field_canvas, canvas_buf, FIELD_SIZE, FIELD_SIZE,
                         LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_size(field_canvas, FIELD_SIZE, FIELD_SIZE);
    draw_field(0);

    // Legend
    lv_obj_t* legend = lv_obj_create(right);
    lv_obj_set_size(legend, 304, 16);
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
    make_legend_item(theme.accent,       "forward");
    make_legend_item(theme.reverse_path, "reverse");

    // Description (now lives under the field in the right panel)
    desc_label = lv_label_create(right);
    lv_obj_set_size(desc_label, 304, LV_SIZE_CONTENT);
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
    lv_label_set_text(status_lbl, "AURA  |  Press A + B to run auto");
    lv_obj_set_style_text_font(status_lbl, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(status_lbl, theme.text_muted, 0);
    lv_obj_center(status_lbl);
}

void lvgl_selector_run_selected() {
    if (!autons.empty())
        autons[selected_auton].fn();
}