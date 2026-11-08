# Corne OLED Dashboard V1.1 — Clean Tech + Tiny Cat

**Status: experimental, draft PR.** Based on the working OLED V1
(`feature/corne-oled-dashboard-v1`), which itself branches from
Fast Base V2.1. The actual 7-layer keymap remains unchanged.

## SSD1306 128×32 pixel layout

```text
LEFT / CENTRAL (illustrative)            RIGHT / PERIPHERAL (illustrative)
+------------------------+                +------------------------+
| L USB   [plug pulse] 100|                | R                   97%|
|                        |                |                        |
| Base       [▏▍▎]  48WPM |                | SPLIT [wave] WiFi✓ cat|
+------------------------+                +------------------------+
```

The layout is intentionally **not** a fullscreen animation. Existing
V1 status widgets still show layer, host output, battery, WPM and split link.

## Four tiny 1-bit animations

| Half | Region (x,y), size | Effect | Behavior |
| --- | --- | --- | --- |
| Central | (73,3), 12×8 | USB/BLE activity | Small plug or Bluetooth symbol with an animated signal pixel; dash when no endpoint selected |
| Central | (89,23), 12×8 | Typing equalizer | Three bars move when measured WPM > 0; three quiet dots otherwise |
| Peripheral | (50,20), 12×8 | Split connection wave | Wave pulses when the right-half BLE link is connected; static cross when disconnected |
| Peripheral | (114,19), 12×12 | Tiny cat mascot | 1-bit original pixel cat blinks for one frame approximately every 4.2 seconds |

The preceding text is a design description, **not a screenshot**. Actual
OLED polarity is preserved from the previously tested V1.

## Architecture

- `modules/corne_oled_dashboard/src/animation.c` implements four sprite
  buffers in LVGL `LV_COLOR_FORMAT_I1`, but **only two are allocated per half**
  via compile-time role selection.
- Two original 12-wide images are static; dynamic bars, pulse and waves are
  calculated into stack-local 12-bit rows. Each canvas is invalidated
  **once per frame**, not once for each pixel.
- Updates use `K_WORK_DELAYABLE_DEFINE` and
  `k_work_reschedule_for_queue(zmk_display_work_q(), …)`: animation draws
  execute on ZMK's **dedicated display work queue**.
- Frame interval = **350 ms**, ~**2.86 FPS** at most.
- State comes from ZMK's event listeners: transport, BLE profile, WPM,
  split peripheral connection and activity. **No keypress listeners**.
- When ZMK transitions to idle or sleep, the animation's delayed work
  is canceled. It resumes when the keyboard becomes active again.
- No GIF decoder, frame-by-frame assets, external Zephyr module or host app.
  The existing OLED blank-on-idle logic remains enabled.
- ZMK source revision remains pinned to
  `4dd95d7840eb3d5a3f59bb29fbf1ab054715ef85`.

## Build and hardware test

1. In GitHub Actions, select **Build Corne 3x5 Optimized** and run branch
   `feature/corne-oled-dashboard-v1.1-animations`. CI is also triggered
   on pushes to this branch.
2. Ensure three test scripts pass:
   - `tools/validate_corne_v21.py`
   - `tools/validate_corne_oled.py`
   - `tools/validate_corne_oled_anim.py`
3. Verify both `corne_left` and `corne_right` compile and output UF2.
   Confirm no warnings such as `attempt to assign the value 'y' to the undefined symbol LV_USE_CANVAS`.
4. Back up the **currently known-good left/right UF2** images before flashing.
5. Flash both halves, allow the peripheral to reconnect. Do not settings-reset
   the devices unless it is otherwise required.
6. Check left: USB icon pulses, typing bars move when WPM > 0, idle shows dots.
   In BLE output mode the icon changes; no connection shows a short dash.
7. Check right: link pulse while connected; cat blinks once in ~4.2 seconds.
   With split link disconnected, wave becomes an X; do not expect layer/WPM
   on the right.
8. Observe both OLEDs for **at least one idle transition** and ensure the
   animations stop and the OLED blanks, then wake after the next keypress.
9. Compare USB typing, Telex, layer switching, one-shot modifiers, Mouse and
   BLE Clear to the established V2.1 firmware. No noticeable lag or dropped
   keys should be introduced.
10. Inspect `zmk-revision.txt` in artifacts, and if diagnostics are enabled
    look for LVGL allocation, stack overflow or display queue warnings.

## Acceptance / performance budget

- [ ] 4 sprites display with correct **physical dark background / white pixels**.
- [ ] All text, symbols and pixels stay within 128×32.
- [ ] WPM and link/transport events correctly drive the respective animations.
- [ ] <=4 FPS, only local canvas invalidations; animation cancels in idle/sleep.
- [ ] No new typing latency and no changes to the Fast Base 7-layer keymap.
- [ ] Left/right firmware both build with pinned ZMK and no Kconfig errors.

## Recovery

Reflash last working OLED V1 or Fast Base V2.1 left/right UF2 builds.
No keyboard settings migration and no Bluetooth bonding reset is expected;
avoid unnecessary `settings_reset` firmware.

V1.1 does not attempt to address host Bluetooth lag or battery calibration.
The output is a low-power visual enhancement only.
