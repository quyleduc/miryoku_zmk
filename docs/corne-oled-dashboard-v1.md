# Corne OLED Dashboard V1 — SSD1306 128×32

## Scope and guarantees

Branch: `feature/corne-oled-dashboard-v1`, based on merged Fast Base V2.1.
ZMK upstream is still pinned to `4dd95d7840eb3d5a3f59bb29fbf1ab054715ef85`.
The original `config/corne.keymap` is **unchanged** (Git blob
`28210af6001509047a890d555987abd3dddac78d`).
Mouse remains enabled on the central `corne_left` only.

The dashboard adds no typing callbacks, no custom event handlers, no screen
redraw on individual key events and no animation timers. It uses built-in
ZMK widget listeners and the dedicated display work queue.

## Physical screens and data ownership

This version assumes **SSD1306 monochrome 128×32**, physically mounted on
both Corne halves.

```text
LEFT / CENTRAL                      RIGHT / PERIPHERAL
+----------------------+            +----------------------+
| L  [USB/BT status] 92%|            | R                78% |
|                      |            |                      |
| [Base]          48   |            | SPLIT   [link icon]  |
+----------------------+            +----------------------+
  layer       WPM                    own battery  BLE central link
```
*Mockup, not a runtime screenshot. Actual icons and widths vary with font and
battery reporting; percentage requires supported battery sensing.*

The central knows the active layer, chosen USB/BLE output, battery and typing
rate. The peripheral **does not receive central's layer or WPM state**, so its
screen deliberately shows only locally available battery percentage and
whether its BLE split link is connected. There is no fabricated signal,
RSSI, host connection, or mirrored layer.

## Implementation

- `modules/corne_oled_dashboard/`: local Zephyr module, included via
  `ZMK_EXTRA_MODULES` in the reusable build workflow.
- `src/status_screen.c`: overrides ZMK's weak
  `zmk_display_status_screen()`. Reuses ZMK's
  `battery/output/layer/wpm/peripheral` widgets on the proper split role.
- `config/corne_left.conf`: central UI + existing Mouse.
- `config/corne_right.conf`: peripheral battery and link status.
- Monochrome 1-bit LVGL, 8 KB LVGL heap, font size 12,
  100 ms display service tick and idle blanking.
- `CONFIG_ZMK_DISPLAY_WORK_QUEUE_DEDICATED=y` on both halves to avoid
  serializing UI updates with unrelated system work on the default work queue.
- No new external GitHub module dependency.

## Build

1. GitHub Actions → **Build Corne 3x5 Optimized** → **Run workflow**.
2. Select branch `feature/corne-oled-dashboard-v1`.
3. Verify that both `validate_corne_v21.py` and `validate_corne_oled.py`
   pass before the `west build` step.
4. Verify artifacts for **corne_left** and **corne_right**, and
   `zmk-revision.txt` containing pinned ZMK SHA.
5. **Save V2.1 left/right UF2 files before flashing the new branch.**
6. Flash **both halves separately** and allow them to reconnect. Do not
   flash settings-reset unless a separate, verified bonding issue requires it.

### Acceptance checklist

- [ ] Both shields compile and emit UF2 with no Kconfig warnings regarding
      custom display and LVGL options.
- [ ] Central OLED 128×32 renders output, active layer, battery percentage
      (if battery level is available), and WPM.
- [ ] Peripheral OLED 128×32 renders its own battery and BLE split link status.
- [ ] Switching Base/Nav/Mouse/Media/Num/Sym/Fun changes the central layer
      indicator without a noticeable keypress delay.
- [ ] USB typing (including Telex) remains as smooth as V2.1.
- [ ] One-shot Ctrl/Shift, Nav, Num, Sym, Fun and all media/BLE controls pass
      previous functional regression tests.
- [ ] Mouse move, wheel and 3 clicks still work.
- [ ] Both displays blank while idle and wake on keyboard activity.
- [ ] No truncated labels during USB, BLE disconnected and full battery
      states. Adjust font/layout only if needed.

## Performance and known limitations

This design avoids animation and UI polling, but the ZMK display
infrastructure still ticks every 100 ms while active. Rendering costs and
RAM usage **have not been hardware-benchmarked**. A nonzero update cost and
power consumption remain, so compare with the V2.1 build on actual hardware.
WPM reporting adds some central-side accounting. If there is any typing
regression, disable WPM first before modifying keyscan timings.

This V1 does **not** solve the previously observed BLE host latency when
running without USB. Battery percentages are only as reliable as the
hardware's battery voltage reporting. The immutable ZMK SHA is pinned,
but the `zmkfirmware/zmk-build-arm:stable` builder image is not digest-pinned.

## Rollback

Flash the previously saved V2.1 `corne_left` and `corne_right` UF2
artifacts. Do not merge this new branch before both displays and typing
have passed actual hardware testing.
