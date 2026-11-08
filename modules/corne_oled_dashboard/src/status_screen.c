/*
 * Corne OLED Dashboard V1 — SSD1306 128x32
 * ZMK pinned revision: 4dd95d7840eb3d5a3f59bb29fbf1ab054715ef85
 *
 * The existing ZMK widgets subscribe to input/endpoint/battery events.
 * There are no custom timers, screen-polling loops, or key handlers.
 * The central shows output, layer, battery, WPM.
 * The peripheral shows its OWN battery and its BLE link with central.
 */
#include <zephyr/kernel.h>
#include <lvgl.h>

#include <zmk/display/status_screen.h>
#include <zmk/display/widgets/battery_status.h>
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/display/widgets/output_status.h>
#include <zmk/display/widgets/layer_status.h>
#include <zmk/display/widgets/wpm_status.h>
#else
#include <zmk/display/widgets/peripheral_status.h>
#endif

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define ROW_TOP 1
#define ROW_BOTTOM 17

/* The photographed SSD1306 panel renders the previous logical black as
 * illuminated pixels. Swap logical monochrome colors as a board-local,
 * reversible workaround. If another OLED has normal polarity, set 0.
 */
#define CORNE_OLED_SWAP_MONO_PIXELS 1
#if CORNE_OLED_SWAP_MONO_PIXELS
#define OLED_BG_COLOR lv_color_white()
#define OLED_FG_COLOR lv_color_black()
#else
#define OLED_BG_COLOR lv_color_black()
#define OLED_FG_COLOR lv_color_white()
#endif


static struct zmk_widget_battery_status battery_widget;

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static struct zmk_widget_output_status output_widget;
static struct zmk_widget_layer_status layer_widget;
static struct zmk_widget_wpm_status wpm_widget;
#else
static struct zmk_widget_peripheral_status peripheral_widget;
#endif

static void place_label(lv_obj_t *label, int16_t x, int16_t y) {
    lv_obj_set_style_text_font(label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, OLED_FG_COLOR, LV_PART_MAIN);
    lv_obj_set_pos(label, x, y);
}

/* Static role markers need no events. */
static void place_static(lv_obj_t *parent, const char *text, int16_t x, int16_t y) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    place_label(label, x, y);
}

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(screen);
    lv_obj_set_size(screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(screen, OLED_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    place_static(screen, "L", 0, ROW_TOP);
#else
    place_static(screen, "R", 0, ROW_TOP);
#endif

    zmk_widget_battery_status_init(&battery_widget, screen);
    /* Small battery font keeps charging icon + 100% inside 128px. */
    lv_obj_t *battery_label = zmk_widget_battery_status_obj(&battery_widget);
    lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_8, LV_PART_MAIN);
    lv_obj_set_style_text_color(battery_label, OLED_FG_COLOR, LV_PART_MAIN);
    lv_obj_set_pos(battery_label, 92, ROW_TOP + 3);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    zmk_widget_output_status_init(&output_widget, screen);
    place_label(zmk_widget_output_status_obj(&output_widget), 17, ROW_TOP);

    zmk_widget_layer_status_init(&layer_widget, screen);
    place_label(zmk_widget_layer_status_obj(&layer_widget), 0, ROW_BOTTOM);

    zmk_widget_wpm_status_init(&wpm_widget, screen);
    place_label(zmk_widget_wpm_status_obj(&wpm_widget), 105, ROW_BOTTOM);
#else
    /* Local peripheral status only; no fictional central layer/WPM data. */
    place_static(screen, "SPLIT", 0, ROW_BOTTOM);
    zmk_widget_peripheral_status_init(&peripheral_widget, screen);
    place_label(zmk_widget_peripheral_status_obj(&peripheral_widget), 76, ROW_BOTTOM);
#endif

    return screen;
}
