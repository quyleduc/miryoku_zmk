#!/usr/bin/env python3
"""Corne OLED Dashboard V1: static regression and configuration invariants.

Runs before both shield builds, without Zephyr or third-party packages.
Hardware/LVGL compilation must be verified separately in GitHub Actions.
"""
from pathlib import Path
import hashlib
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
KEYMAP = ROOT / "config/corne.keymap"
LEFT = ROOT / "config/corne_left.conf"
RIGHT = ROOT / "config/corne_right.conf"
MODULE = ROOT / "modules/corne_oled_dashboard"
WORKFLOW = ROOT / ".github/workflows/main.yml"
SHIELD_WORKFLOW = ROOT / ".github/workflows/build-example-corne-nice_nano-display.yml"

# Original seven-layer keymap Git blob SHA from tested V2.1.
V21_KEYMAP_BLOB = "28210af6001509047a890d555987abd3dddac78d"
ZMK_PIN = "4dd95d7840eb3d5a3f59bb29fbf1ab054715ef85"
EXPECTED_LAYERS = ["Base", "Nav", "Mouse", "Media", "Num", "Sym", "Fun"]
errors = []


def verify(condition: bool, label: str) -> None:
    if not condition:
        errors.append(label)


def enabled(conf: str, name: str) -> bool:
    return re.search(rf"^CONFIG_{re.escape(name)}=y$", conf, re.M) is not None


def main() -> None:
    keymap = KEYMAP.read_bytes()
    blob = hashlib.sha1(b"blob " + str(len(keymap)).encode() + b"\0" + keymap).hexdigest()
    verify(blob == V21_KEYMAP_BLOB, "Keymap changed from tested V2.1")
    names = re.findall(r'display-name\s*=\s*"([^"]+)"', keymap.decode())
    verify(names == EXPECTED_LAYERS, f"Expected seven exact layers, found {names}")

    left = LEFT.read_text()
    right = RIGHT.read_text()
    baseline = [
        "ZMK_DISPLAY", "ZMK_DISPLAY_STATUS_SCREEN_CUSTOM",
        "ZMK_DISPLAY_WORK_QUEUE_DEDICATED", "ZMK_DISPLAY_BLANK_ON_IDLE",
        "LV_COLOR_DEPTH_1", "LV_FONT_MONTSERRAT_12",
    ]
    for side, conf in [("left", left), ("right", right)]:
        for flag in baseline:
            verify(enabled(conf, flag), f"{side}: missing CONFIG_{flag}=y")
        verify("CONFIG_ZMK_DISPLAY_TICK_PERIOD_MS=100" in conf,
               f"{side}: expected low-update-rate display configuration")
        verify("CONFIG_LV_Z_BITS_PER_PIXEL=1" in conf,
               f"{side}: expected monochrome LVGL rendering")
        verify("CONFIG_LV_Z_MEM_POOL_SIZE=8192" in conf,
               f"{side}: expected bounded LVGL heap")
        verify("CONFIG_LV_Z_VDB_SIZE=32" in conf,
               f"{side}: expected small 128x32 LVGL I2C framebuffer")

    for flag in ["ZMK_POINTING", "ZMK_WIDGET_OUTPUT_STATUS",
                 "ZMK_WIDGET_LAYER_STATUS", "ZMK_WIDGET_WPM_STATUS"]:
        verify(enabled(left, flag), f"left: missing CONFIG_{flag}")
    verify(not enabled(right, "ZMK_POINTING"), "Peripheral must not enable mouse stack")
    for flag in ["ZMK_WIDGET_PERIPHERAL_STATUS"]:
        verify(enabled(right, flag), f"right: missing CONFIG_{flag}")
    for flag in ["ZMK_WIDGET_LAYER_STATUS", "ZMK_WIDGET_WPM_STATUS",
                 "ZMK_WIDGET_OUTPUT_STATUS"]:
        verify(not enabled(right, flag), f"right must not use central-only {flag}")

    for side, conf in [("left", left), ("right", right)]:
        verify(enabled(conf, "ZMK_WIDGET_BATTERY_STATUS"), f"{side}: local battery missing")
        verify(enabled(conf, "ZMK_WIDGET_BATTERY_STATUS_SHOW_PERCENTAGE"),
               f"{side}: battery percentage missing")

    yaml = (MODULE / "zephyr/module.yml").read_text()
    cmake = (MODULE / "CMakeLists.txt").read_text()
    code = (MODULE / "src/status_screen.c").read_text()
    verify("build:" in yaml and "cmake:" in yaml, "Zephyr module not registered")
    verify("zephyr_library_sources_ifdef" in cmake,
           "OLED screen compilation not conditional on Kconfig")
    verify("zmk_display_status_screen(void)" in code, "Custom status-screen entry missing")
    verify("#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)" in code,
           "Central/peripheral UI must be different")
    for w in ["battery", "output", "layer", "wpm", "peripheral"]:
        verify(f"zmk_widget_{w}_status_init" in code,
               f"Missing {w} event-driven widget")
    verify(not any(x in code for x in
                   ["K_TIMER_DEFINE", "k_timer_start(", "k_sleep(", "ZMK_SUBSCRIPTION("]),
           "Dashboard must not poll, delay or introduce new input listeners")
    verify("SCREEN_WIDTH 128" in code and "SCREEN_HEIGHT 32" in code,
           "Dashboard must target SSD1306 128x32")

    workflow = WORKFLOW.read_text()
    build = SHIELD_WORKFLOW.read_text()
    verify("modules/corne_oled_dashboard" in workflow and
           "ZMK_EXTRA_MODULES" in workflow, "OLED Zephyr module not passed to build")
    verify("validate_corne_oled.py" in workflow,
           "OLED regression validator must run before west build")
    verify(ZMK_PIN in build, "ZMK pinned SHA must remain unchanged")
    verify('["corne_left","corne_right"]' in build,
           "OLED must build both Corne shields")

    if errors:
        print("FAIL: Corne OLED Dashboard V1")
        for message in errors:
            print(" -", message)
        sys.exit(1)
    print("PASS: V2.1 keymap unchanged; left/right role separation; 128x32 widgets;")
    print("      mouse preservation; pinned ZMK; event-driven UI; 7 layer layout")


if __name__ == "__main__":
    main()
