# Corne 36-key — Miryoku Fast Base V2

This is an experimental **7-layer** replacement for the original generated Miryoku
10-layer keymap. It is implemented in `config/corne.keymap` and is designed for the
Corne 3x5+3 with a ZMK nice_nano@2.0.0//zmk central (left).

## Design

- 0 **Base**: direct QWERTY; no home-row mod-taps.
- 1 **Nav**: arrows, Home/End, clipboard, held modifiers and sticky Ctrl/Shift.
- 2 **Mouse**: right-side pointer, wheel, and click buttons.
- 3 **Media**: Bluetooth profiles 0–4, Clear current profile, USB/BLE output, media.
- 4 **Num**: left-hand keypad 0–9 and selected punctuation.
- 5 **Sym**: left-hand programming punctuation.
- 6 **Fun**: left-hand F1–F12 and system keys.

## Thumb positions, from inner/outer as physically mapped

Left: `ESC / hold Media`, `SPACE / hold Nav`, `TAB / hold Mouse`.

Right: `ENTER / hold Sym`, `BACKSPACE / hold Num`, `DELETE / hold Fun`.

Hold one of the above thumb keys to temporarily activate the named layer.
The alpha keys on Base always use `&kp` rather than home-row hold-tap.

## Essential shortcuts

The following positions refer to the ordinary QWERTY key location, not the active
layer key legend.

- **BLE clear**: hold **left ESC (Media)** + press **left G**. This clears only
  the currently selected host profile. On the PC, forget the old Bluetooth bond
  before pairing again.
- **Select BLE profile**: hold **left ESC** + press **right N/M/,/./**
  to choose profiles **0/1/2/3/4**, respectively.
- **Next/previous BLE profile**: hold **left ESC** + press **right O/P**.
- **Output toggle / force USB / force BLE**: hold **left ESC** +
  press **right Y/U/I**, respectively.
- **Volume down/up**: hold **left ESC** + press **right K/L**.
- **Copy/Paste/Cut**: hold **left SPACE (Nav)** + press
  **right I/U/O**, respectively.
- **Arrow keys**: hold **left SPACE** + press **right J/K/L/'**
  for left/down/up/right.
- **Sticky Ctrl**: hold **left SPACE**, tap **right Y**, release
  SPACE, then press the desired key (e.g., S for save).
- **Sticky Shift**: hold **left SPACE**, tap **right P**, release
  SPACE, then press the desired key (e.g., A for uppercase).
- **Hold modifier**: hold **left SPACE** and **left A/S/D/F**
  to hold GUI/Alt/Ctrl/Shift, respectively.
- **Bootloader**: hold **left SPACE** and **double-tap left Q**
  within 250 ms. Do not use this for normal pairing resets.

## Firmware build

GitHub Actions > **Build Corne 3x5 Optimized** > Run workflow >
choose **feature/miryoku-fast-base-v2**. The existing matrix and board
settings are unchanged (including the `nrf52840-nosd` snippet).

Flash **corne_left** first. Reflash **corne_right** only when changes to the
peripheral-side firmware are needed.

## What is intentionally absent?

**Extra**, **Tap**, and **Button** no longer appear as layers. Extra previously
had the laggy home-row mods; Tap duplicated Fast Base; Button overlapped Mouse
and clipboard functions. No sticky switch to the old layers is required.

## Validation

Source mapping and key count have been checked (42 shield bindings per layer,
including 6 physically absent row-edge keys; 36 physical switches per layer).
**GitHub Actions compilation and hardware behavior still need to be tested**.
