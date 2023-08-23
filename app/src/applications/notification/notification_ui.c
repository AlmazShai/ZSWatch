#include <lvgl.h>
#include <notification/notification_ui.h>
#include "zsw_ui_utils.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(notif_ui, LOG_LEVEL_DBG);

static void not_button_pressed(lv_event_t* e);
static void scroll_event_cb(lv_event_t* e);
static void build_notification_entry(lv_obj_t* parent, not_mngr_notification_t * not, lv_group_t* input_group);

static on_notification_remove_cb_t not_removed_callback;

static lv_obj_t* main_page;
static lv_group_t* group;

void notifications_page_init(on_notification_remove_cb_t not_removed_cb)
{
    not_removed_callback = not_removed_cb;
}

void notifications_page_create(not_mngr_notification_t* notifications, uint8_t num_notifications,
    lv_group_t* input_group)
{
    group = input_group;

    main_page = lv_obj_create(lv_scr_act());
    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_size(main_page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_border_side(main_page, LV_BORDER_SIDE_NONE, 0);
    lv_obj_center(main_page);

    lv_obj_set_flex_flow(main_page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(main_page, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(main_page, LV_SCROLL_SNAP_CENTER);
    lv_obj_clear_flag(main_page, LV_OBJ_FLAG_SCROLL_MOMENTUM); // TODO: doublecheck
    lv_obj_set_scrollbar_mode(main_page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(main_page, scroll_event_cb, LV_EVENT_SCROLL, NULL);

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x331c2a), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(main_page, global_watchface_bg_img, LV_STATE_DEFAULT);

    for (int i = 0; i < num_notifications; i++) {
        build_notification_entry(main_page, &notifications[i], input_group);
    }

    /* Update the notifications position manually first time */
    lv_event_send(main_page, LV_EVENT_SCROLL, NULL);

    /* Be sure the fist notification is in the middle */
    lv_obj_scroll_to_view(lv_obj_get_child(main_page, 0), LV_ANIM_OFF);
}

void notifications_page_close(void)
{
    lv_obj_del(main_page);
}

static void build_notification_entry(lv_obj_t* parent, not_mngr_notification_t * not, lv_group_t* input_group)
{
    lv_obj_t* title;
    lv_obj_t* body;
    lv_obj_t* cont;
    static lv_style_t outline_default;
    static lv_style_t outline_focused;

    // Border around selected menu row when focused
    // lv_style_init(&outline_default);
    // lv_style_set_border_color(&outline_default, lv_palette_main(LV_PALETTE_BLUE_GREY));
    // lv_style_set_border_width(&outline_default, lv_disp_dpx(lv_disp_get_next(NULL), 1));
    // lv_style_set_border_opa(&outline_default, LV_OPA_50);
    // lv_style_set_border_side(&outline_default, LV_BORDER_SIDE_BOTTOM);

    // lv_style_init(&outline_focused);
    // lv_style_set_border_color(&outline_focused, lv_palette_main(LV_PALETTE_LIGHT_BLUE));
    // lv_style_set_border_width(&outline_focused, );
    // lv_style_set_border_side(&outline_focused, LV_BORDER_SIDE_FULL);
    // // lv_style_set_radius(&outline_focused, 15);
    // lv_style_set_bg_color(&outline_default, lv_color_hex(0xC1C1C1));

    cont = lv_obj_create(parent);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(cont, 0, LV_STATE_DEFAULT);
    // lv_style_set_pad_column(cont, 2);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    // lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);

    // Configure size
    lv_obj_set_size(cont, LV_PCT(95), LV_SIZE_CONTENT);
    lv_obj_set_style_max_height(cont, 80, 0);
    lv_obj_set_style_pad_top(cont, 5, 0);
    lv_obj_set_style_pad_bottom(cont, 5, 0);
    lv_obj_set_style_pad_right(cont, 8, 0);
    lv_obj_set_style_pad_left(cont, 8, 0);
    lv_obj_set_style_radius(cont, 15, 0);
    lv_obj_set_style_border_width(cont, lv_disp_dpx(lv_disp_get_next(NULL), 1), LV_STATE_DEFAULT);

    // Configure style
    lv_obj_set_style_border_color(cont, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cont, lv_disp_dpx(lv_disp_get_next(NULL), 1), LV_STATE_DEFAULT);

    lv_obj_add_event_cb(cont, not_button_pressed, LV_EVENT_SHORT_CLICKED, (void*)not ->id);
    lv_group_add_obj(input_group, cont);
    // lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    title = lv_label_create(cont);
    lv_label_set_text_fmt(title, "%s %s", LV_SYMBOL_BELL, not ->title);
    lv_obj_set_size(title, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_max_height(title, 20, 0);
    lv_obj_set_style_text_color(title, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);

    body = lv_label_create(cont);
    lv_label_set_text(body, not ->body);
    lv_obj_set_size(body, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_max_height(body, LV_PCT(100), LV_STATE_DEFAULT);
    // lv_obj_align_to(body, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_label_set_long_mode(body, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_font(body, &lv_font_montserrat_16, 0);
}

static void not_button_pressed(lv_event_t* e)
{
    if(e->code != LV_EVENT_SHORT_CLICKED)
        return;
    lv_obj_del(lv_event_get_target(e));
    not_removed_callback((uint32_t)lv_event_get_user_data(e));
    LOG_DBG("Short press");
}

static void notif_button_long_press(lv_event_t* e)
{
    if(e->code != LV_EVENT_LONG_PRESSED)
        return;
    lv_obj_t* cont = lv_event_get_target(e);
    lv_obj_set_style_max_height(cont, 160, 0);
    lv_obj_remove_event_cb(cont, notif_button_long_press);

    lv_label_t * body = lv_obj_get_child(cont, 1);
    lv_label_set_long_mode(body, LV_LABEL_LONG_WRAP);
    LOG_DBG("Long press");
}

static void scroll_event_cb(lv_event_t* e)
{
    lv_obj_t* cont = lv_event_get_target(e);
    lv_area_t cont_a;
    lv_obj_get_coords(cont, &cont_a);
    lv_coord_t cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;
    lv_coord_t r = lv_obj_get_height(cont) * 7 / 10;
    uint32_t i;
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    for (i = 0; i < child_cnt; i++) {
        lv_obj_t* child = lv_obj_get_child(cont, i);
        lv_area_t child_a;
        lv_obj_get_coords(child, &child_a);

        lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;

        lv_coord_t diff_y = child_y_center - cont_y_center;
        diff_y = LV_ABS(diff_y);

        /* Get the x of diff_y on a circle. */
        lv_coord_t x;
        /* If diff_y is out of the circle use the last point of the circle (the radius) */
        if (diff_y >= r) {
            x = r;
        } else {
            /* Use Pythagoras theorem to get x from radius and y */
            uint32_t x_sqr = r * r - diff_y * diff_y;
            lv_sqrt_res_t res;
            lv_sqrt(x_sqr, &res, 0x8000); /* Use lvgl's built in sqrt root function */
            x = r - res.i;
        }

        /* Translate the item by the calculated X coordinate */
        lv_obj_set_style_translate_x(child, x, 0);
        lv_obj_set_style_translate_y(child, -13, 0);

        /* Use some opacity with larger translations */
        lv_opa_t opa = lv_map(x, 0, r, LV_OPA_TRANSP, LV_OPA_COVER);
        lv_obj_set_style_opa(child, LV_OPA_COVER - opa, 0);

        /* Change boarder style for central object */
        if (diff_y == 0) {
            lv_obj_set_style_border_color(child, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(child, lv_disp_dpx(lv_disp_get_next(NULL), 3), LV_STATE_DEFAULT);
            lv_obj_add_event_cb(child, notif_button_long_press, LV_EVENT_LONG_PRESSED, NULL);
        } else {
            lv_obj_set_style_border_color(child, lv_palette_main(LV_PALETTE_GREY), LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(child, lv_disp_dpx(lv_disp_get_next(NULL), 1), LV_STATE_DEFAULT);
            lv_obj_remove_event_cb(child, notif_button_long_press);
        }
    }
}