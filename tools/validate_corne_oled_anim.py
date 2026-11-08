#!/usr/bin/env python3
"""Corne OLED Dashboard V1.1 animation regression checks.

Runs in CI before ZMK compilation, in addition to the V2.1 keymap
and V1 dual-display validators. No external Python dependencies.
"""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
MODULE = ROOT / "modules/corne_oled_dashboard"
ANIMATION = MODULE / "src/animation.c"
SCREEN = MODULE / "src/status_screen.c"
BUILD = MODULE / "CMakeLists.txt"
WORKFLOW = ROOT / ".github/workflows/main.yml"
BUILD_WORKFLOW = ROOT / ".github/workflows/build-example-corne-nice_nano-display.yml"
CONFIGS = [ROOT / "config/corne_left.conf", ROOT / "config/corne_right.conf"]
errors = []


def verify(condition, message):
    if not condition:
        errors.append(message)


def sprite_arrays(text):
    arrays = {}
    regex = r"static const uint16_t (\w+)\[(ANIM_\w+)\] = \{([^}]+)\};"
    for name, dim, payload in re.findall(regex, text):
        values = [int(part.strip().rstrip("uU"), 0) for part in payload.split(",")]
        arrays[name] = (dim, values)
    return arrays


def run():
    anim = ANIMATION.read_text()
    screen = SCREEN.read_text()
    cmake = BUILD.read_text()
    work = WORKFLOW.read_text()
    trigger = BUILD_WORKFLOW.read_text()
    sprites = sprite_arrays(anim)

    expected = {"usb_sprite": 8, "ble_sprite": 8,
                "cat_open_sprite": 12, "cat_blink_sprite": 12}
    for name, height in expected.items():
        entry = sprites.get(name)
        verify(entry is not None, f"Missing {name}")
        if entry:
            _, rows = entry
            verify(len(rows) == height, f"{name}: expected {height} rows")
            verify(all(0 <= value < 1 << 12 for value in rows),
                   f"{name}: must fit 12 horizontal pixels")
            verify(sum(row.bit_count() for row in rows) > 0,
                   f"{name}: empty sprite")
    if all(k in sprites for k in expected):
        verify(sprites["cat_open_sprite"][1] != sprites["cat_blink_sprite"][1],
               "Mascot must have a distinct blink frame")
        verify(sprites["usb_sprite"][1] != sprites["ble_sprite"][1],
               "USB and BLE glyphs must differ")

    period = re.search(r"#define ANIM_PERIOD_MS\s+(\d+)", anim)
    verify(period is not None, "Missing frame interval")
    if period:
        verify(250 <= int(period.group(1)) <= 500,
               "Animation must stay at <=4FPS (and >=2FPS)")

    verify("LV_DRAW_BUF_DEFINE_STATIC(output_buffer" in anim and
           "LV_DRAW_BUF_DEFINE_STATIC(typing_buffer" in anim,
           "Missing left OLED 1-bit buffers")
    verify("LV_DRAW_BUF_DEFINE_STATIC(split_buffer" in anim and
           "LV_DRAW_BUF_DEFINE_STATIC(cat_buffer" in anim,
           "Missing right OLED 1-bit buffers")
    verify(anim.count("LV_DRAW_BUF_DEFINE_STATIC(") == 4,
           "Unexpected additional framebuffers")
    verify(anim.count("LV_COLOR_FORMAT_I1") >= 4, "Framebuffers must be 1-bit")
    verify("lv_canvas_set_palette(obj, 0" in anim and
           "lv_canvas_set_palette(obj, 1" in anim,
           "Sprite palette must preserve V1 panel polarity")
    verify("lv_draw_buf_goto_xy(draw_buf, 0, 0)" in anim,
           "Must draw after the I1 palette header")
    verify("lv_obj_invalidate(canvas);" in anim,
           "Each updated sprite must be invalidated exactly once")
    verify("lv_canvas_set_px(" not in anim,
           "Avoid per-pixel object invalidations")

    verify("ZMK_DISPLAY_WIDGET_LISTENER(dash_anim_activity" in anim and
           "ZMK_SUBSCRIPTION(dash_anim_activity, zmk_activity_state_changed)" in anim,
           "Missing activity-driven pause/resume")
    verify("zmk_activity_get_state() == ZMK_ACTIVITY_ACTIVE" in anim and
           "k_work_cancel_delayable(&animation_work)" in anim,
           "Stop animation during both idle and sleep")
    verify("k_work_reschedule_for_queue(zmk_display_work_q()" in anim,
           "Animation must run in the dedicated display queue")
    verify("K_WORK_DELAYABLE_DEFINE(animation_work" in anim,
           "Animation must use delayable work, not busy polling")
    verify("K_TIMER_DEFINE" not in anim and "k_sleep(" not in anim,
           "Do not create an independent timer or blocking delay")
    verify("zmk_keycode_state_changed" not in anim and
           "zmk_position_state_changed" not in anim,
           "No per-keystroke input listeners allowed")
    verify("zmk_wpm_state_changed" in anim and
           "zmk_endpoint_changed" in anim and
           "zmk_split_peripheral_status_changed" in anim,
           "Animations must use real WPM/transport/split state")

    verify("corne_oled_animation_init(screen);" in screen,
           "Custom screen must initialize the sprite module")
    verify("src/animation.c" in cmake, "Animation source not linked")
    verify("validate_corne_v21.py" in work and
           "validate_corne_oled.py" in work and
           "validate_corne_oled_anim.py" in work,
           "All three static regressions must run before west build")
    verify("feature/corne-oled-dashboard-v1.1-animations" in trigger,
           "V1.1 branch must trigger both-shield CI")
    verify('["corne_left","corne_right"]' in trigger,
           "Both halves must be built")

    for conf_path in CONFIGS:
        conf = conf_path.read_text()
        verify("CONFIG_LV_USE_CANVAS=y" in conf,
               f"{conf_path.name}: LVGL canvas missing")
        verify("CONFIG_ZMK_DISPLAY_WORK_QUEUE_DEDICATED=y" in conf,
               f"{conf_path.name}: dedicated display work queue missing")
        verify("CONFIG_ZMK_DISPLAY_BLANK_ON_IDLE=y" in conf,
               f"{conf_path.name}: display blank-on-idle must remain")
        verify("CONFIG_ZMK_DISPLAY_INVERT=y" in conf,
               f"{conf_path.name}: confirmed SSD1306 polarity lost")

    if errors:
        print("FAIL: Corne OLED V1.1 animation regression")
        for err in errors:
            print("  -", err)
        sys.exit(1)
    print("PASS: 4 small original monochrome sprite assets; two split OLED roles;")
    print("      WPM/endpoint/split events; <=4FPS; idle cancel; no key hooks;")
    print("      V2.1/V1 regression and both-shield CI remain enabled")


if __name__ == "__main__":
    run()
