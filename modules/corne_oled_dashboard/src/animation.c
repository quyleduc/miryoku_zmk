/*
 * Corne OLED Dashboard V1.1 — small monochrome, event-driven animations.
 *
 * Four pixel-art elements across both halves, max 2.86 FPS.
 * Runs only in the dedicated ZMK display work queue, and cancels its
 * delayed work in ZMK_ACTIVITY_IDLE/SLEEP. No keypress subscriptions.
 * Assets: original 12x8 / 12x12 monochrome sprites authored for this repo.
 *
 * IMPORTANT: Dashboard V1 has inverted logical colors for the actual panels.
 * LVGL I1 palette index 0 is logical white (= physically dark background),
 * index 1 is logical black (= physically lit foreground).
 */

#include "animation.h"

#include <stdint.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <lvgl.h>

#include <zmk/activity.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/endpoints.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/wpm.h>
#else
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/split/bluetooth/peripheral.h>
#endif

#define ANIM_WIDTH 12
#define ANIM_SMALL_HEIGHT 8
#define ANIM_CAT_HEIGHT 12
#define ANIM_PERIOD_MS 350

static const uint16_t usb_sprite[ANIM_SMALL_HEIGHT] = {0x080u, 0x1C0u, 0x490u, 0xFE0u, 0x490u, 0x1C0u, 0x080u, 0x000u};
static const uint16_t ble_sprite[ANIM_SMALL_HEIGHT] = {0x240u, 0x340u, 0x280u, 0x340u, 0x280u, 0x340u, 0x240u, 0x000u};
static const uint16_t cat_open_sprite[ANIM_CAT_HEIGHT] = {0x30Cu, 0x79Eu, 0xFFFu, 0xC03u, 0x999u, 0x999u, 0x801u, 0x861u, 0x8F1u, 0xC03u, 0x7FEu, 0x3FCu};
static const uint16_t cat_blink_sprite[ANIM_CAT_HEIGHT] = {0x30Cu, 0x79Eu, 0xFFFu, 0xC03u, 0x801u, 0x999u, 0x801u, 0x861u, 0x8F1u, 0xC03u, 0x7FEu, 0x3FCu};

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
LV_DRAW_BUF_DEFINE_STATIC(output_buffer, ANIM_WIDTH, ANIM_SMALL_HEIGHT, LV_COLOR_FORMAT_I1);
LV_DRAW_BUF_DEFINE_STATIC(typing_buffer, ANIM_WIDTH, ANIM_SMALL_HEIGHT, LV_COLOR_FORMAT_I1);
static lv_obj_t *output_canvas;
static lv_obj_t *typing_canvas;
static enum zmk_transport current_transport = ZMK_TRANSPORT_NONE;
static bool has_typing_activity;
#else
LV_DRAW_BUF_DEFINE_STATIC(split_buffer, ANIM_WIDTH, ANIM_SMALL_HEIGHT, LV_COLOR_FORMAT_I1);
LV_DRAW_BUF_DEFINE_STATIC(cat_buffer, ANIM_WIDTH, ANIM_CAT_HEIGHT, LV_COLOR_FORMAT_I1);
static lv_obj_t *split_canvas;
static lv_obj_t *cat_canvas;
static bool split_connected;
#endif

static bool ready;
static bool screen_active;
static uint8_t phase;

/* Draw into the backing I1 buffer directly, then invalidate exactly once.
 * lv_draw_buf_goto_xy() accounts for the eight-byte indexed palette header.
 */
static void bitmap_draw(lv_obj_t *canvas, const uint16_t *rows, uint8_t height) {
    lv_draw_buf_t *draw_buf = lv_canvas_get_draw_buf(canvas);
    uint8_t *pixels = (uint8_t *)lv_draw_buf_goto_xy(draw_buf, 0, 0);
    const uint32_t stride = draw_buf->header.stride;

    memset(pixels, 0, stride * height);
    for (uint8_t y = 0; y < height; ++y) {
        const uint16_t bits = rows[y];
        for (uint8_t x = 0; x < ANIM_WIDTH; ++x) {
            if (bits & (1u << (ANIM_WIDTH - 1u - x))) {
                pixels[y * stride + (x >> 3)] |= (uint8_t)(0x80u >> (x & 7));
            }
        }
    }
    lv_obj_invalidate(canvas);
}

static void bitmap_dot(uint16_t *rows, uint8_t x, uint8_t y) {
    if (x < ANIM_WIDTH && y < ANIM_SMALL_HEIGHT) {
        rows[y] |= (uint16_t)(1u << (ANIM_WIDTH - 1u - x));
    }
}

static lv_obj_t *make_canvas(lv_obj_t *screen, lv_draw_buf_t *buffer, int16_t x,
                             int16_t y) {
    lv_obj_t *obj = lv_canvas_create(screen);
    lv_canvas_set_draw_buf(obj, buffer);
    /* Keep the V1 polarity fix: logical white displays as dark. */
    lv_canvas_set_palette(obj, 0, lv_color32_make(255, 255, 255, 255));
    lv_canvas_set_palette(obj, 1, lv_color32_make(0, 0, 0, 255));
    lv_obj_set_pos(obj, x, y);
    return obj;
}

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static void draw_left(void) {
    uint16_t rows[ANIM_SMALL_HEIGHT] = {0};
    if (current_transport == ZMK_TRANSPORT_USB ||
        current_transport == ZMK_TRANSPORT_BLE) {
        memcpy(rows, current_transport == ZMK_TRANSPORT_USB ? usb_sprite : ble_sprite,
               sizeof(rows));
        /* Single moving signal pixel: USB activity/BT wave. */
        bitmap_dot(rows, (uint8_t)(9u + (phase & 1u)), (uint8_t)(1u + (phase % 5u)));
        if (current_transport == ZMK_TRANSPORT_BLE && (phase & 1u)) {
            bitmap_dot(rows, 11, 3);
        }
    } else {
        for (uint8_t x = 3; x <= 8; ++x) bitmap_dot(rows, x, 4);
    }
    bitmap_draw(output_canvas, rows, ANIM_SMALL_HEIGHT);

    memset(rows, 0, sizeof(rows));
    if (has_typing_activity) {
        static const uint8_t bar_heights[4][3] = {
            {2, 4, 3}, {4, 2, 6}, {6, 5, 2}, {3, 6, 4},
        };
        for (uint8_t bar = 0; bar < 3; ++bar) {
            uint8_t height = bar_heights[phase & 3u][bar];
            for (uint8_t y = (uint8_t)(ANIM_SMALL_HEIGHT - height); y < ANIM_SMALL_HEIGHT; ++y) {
                bitmap_dot(rows, (uint8_t)(1u + 4u * bar), y);
                bitmap_dot(rows, (uint8_t)(2u + 4u * bar), y);
            }
        }
    } else {
        bitmap_dot(rows, 2, 7);
        bitmap_dot(rows, 6, 7);
        bitmap_dot(rows, 10, 7);
    }
    bitmap_draw(typing_canvas, rows, ANIM_SMALL_HEIGHT);
}

struct dashboard_transport_state { enum zmk_transport transport; };
static struct dashboard_transport_state get_transport(const zmk_event_t *eh) {
    (void)eh;
    return (struct dashboard_transport_state){
        .transport = zmk_endpoint_get_selected().transport,
    };
}
static void update_transport(struct dashboard_transport_state state) {
    current_transport = state.transport;
}
ZMK_DISPLAY_WIDGET_LISTENER(dash_anim_transport, struct dashboard_transport_state,
                            update_transport, get_transport)
ZMK_SUBSCRIPTION(dash_anim_transport, zmk_endpoint_changed);
ZMK_SUBSCRIPTION(dash_anim_transport, zmk_ble_active_profile_changed);

struct dashboard_typing_state { bool active; };
static struct dashboard_typing_state get_typing(const zmk_event_t *eh) {
    (void)eh;
    return (struct dashboard_typing_state){.active = zmk_wpm_get_state() > 0};
}
static void update_typing(struct dashboard_typing_state state) {
    has_typing_activity = state.active;
}
ZMK_DISPLAY_WIDGET_LISTENER(dash_anim_typing, struct dashboard_typing_state,
                            update_typing, get_typing)
ZMK_SUBSCRIPTION(dash_anim_typing, zmk_wpm_state_changed);
#else
static void draw_right(void) {
    uint16_t rows[ANIM_SMALL_HEIGHT] = {0};

    /* Animated connection wave only when right-half link is up. */
    if (split_connected) {
        bitmap_dot(rows, 1, 4);
        bitmap_dot(rows, 2, 4);
        if ((phase & 3u) >= 1u) {
            bitmap_dot(rows, 5, 2);
            bitmap_dot(rows, 5, 6);
            bitmap_dot(rows, 6, 3);
            bitmap_dot(rows, 6, 5);
        }
        if ((phase & 3u) >= 2u) {
            bitmap_dot(rows, 9, 0);
            bitmap_dot(rows, 9, 7);
            bitmap_dot(rows, 10, 2);
            bitmap_dot(rows, 10, 5);
        }
    } else {
        for (uint8_t i = 2; i < 8; ++i) {
            bitmap_dot(rows, i, i);
            bitmap_dot(rows, (uint8_t)(9u - i), i);
        }
    }
    bitmap_draw(split_canvas, rows, ANIM_SMALL_HEIGHT);
    bitmap_draw(cat_canvas, phase == 0 ? cat_blink_sprite : cat_open_sprite, ANIM_CAT_HEIGHT);
}

struct dashboard_split_state { bool connected; };
static struct dashboard_split_state get_split(const zmk_event_t *eh) {
    (void)eh;
    return (struct dashboard_split_state){
        .connected = zmk_split_bt_peripheral_is_connected(),
    };
}
static void update_split(struct dashboard_split_state state) {
    split_connected = state.connected;
}
ZMK_DISPLAY_WIDGET_LISTENER(dash_anim_split, struct dashboard_split_state,
                            update_split, get_split)
ZMK_SUBSCRIPTION(dash_anim_split, zmk_split_peripheral_status_changed);
#endif

static void animation_work_cb(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(animation_work, animation_work_cb);

static void animation_work_cb(struct k_work *work) {
    (void)work;
    if (!ready || !screen_active) return;
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    draw_left();
#else
    draw_right();
#endif
    phase = (uint8_t)((phase + 1u) % 12u);
    if (screen_active) {
        k_work_reschedule_for_queue(zmk_display_work_q(), &animation_work,
                                    K_MSEC(ANIM_PERIOD_MS));
    }
}

struct dashboard_activity_state { bool active; };
static struct dashboard_activity_state get_activity(const zmk_event_t *eh) {
    (void)eh;
    return (struct dashboard_activity_state){
        .active = zmk_activity_get_state() == ZMK_ACTIVITY_ACTIVE,
    };
}
static void update_activity(struct dashboard_activity_state state) {
    screen_active = state.active;
    if (!ready) return;
    if (!screen_active) {
        k_work_cancel_delayable(&animation_work);
    } else {
        k_work_reschedule_for_queue(zmk_display_work_q(), &animation_work, K_NO_WAIT);
    }
}
ZMK_DISPLAY_WIDGET_LISTENER(dash_anim_activity, struct dashboard_activity_state,
                            update_activity, get_activity)
ZMK_SUBSCRIPTION(dash_anim_activity, zmk_activity_state_changed);

void corne_oled_animation_init(lv_obj_t *screen) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    LV_DRAW_BUF_INIT_STATIC(output_buffer);
    LV_DRAW_BUF_INIT_STATIC(typing_buffer);
    /* x=79 leaves space for the 34px LQE2HC label at x=44 and ends
     * before the battery widget at x=92; preserve all existing animation. */
    output_canvas = make_canvas(screen, &output_buffer, 79, 3);
    typing_canvas = make_canvas(screen, &typing_buffer, 89, 23);
    dash_anim_transport_init();
    dash_anim_typing_init();
#else
    LV_DRAW_BUF_INIT_STATIC(split_buffer);
    LV_DRAW_BUF_INIT_STATIC(cat_buffer);
    split_canvas = make_canvas(screen, &split_buffer, 50, 20);
    cat_canvas = make_canvas(screen, &cat_buffer, 114, 19);
    dash_anim_split_init();
#endif
    ready = true;
    /* Uses same active/idle/sleep state as ZMK OLED blanking. */
    dash_anim_activity_init();
}
