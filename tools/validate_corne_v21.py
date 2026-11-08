#!/usr/bin/env python3
"""Static regression checks for Corne 3x5+3 / Fast Base V2.1.

Run: python3 tools/validate_corne_v21.py
No third-party Python dependencies. Must run before invoking west build.
"""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
KEYMAP = ROOT / "config" / "corne.keymap"
CENTRAL_CONF = ROOT / "config" / "corne_left.conf"
WORKFLOW = ROOT / ".github" / "workflows" / "build-example-corne-nice_nano-display.yml"
MAIN = ROOT / ".github" / "workflows" / "main.yml"

EXPECTED_LAYERS = ["base", "nav", "mouse", "media", "num", "sym", "fun"]
EXPECTED_THUMBS = [
    "&u_lt 3 ESC", "&u_lt 1 SPACE", "&u_lt 2 TAB",
    "&u_lt 5 RET", "&u_lt 4 BSPC", "&u_lt 6 DEL",
]
errors = []


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def binding_count(text: str) -> int:
    return len(re.findall(r"&[A-Za-z_]\w*", text))


def verify() -> None:
    source = KEYMAP.read_text(encoding="utf-8")
    conf = CENTRAL_CONF.read_text(encoding="utf-8")
    workflow = WORKFLOW.read_text(encoding="utf-8")
    main = MAIN.read_text(encoding="utf-8")

    found = re.findall(r'^\s*(base|nav|mouse|media|num|sym|fun|extra|tap|button)\s*\{\s*$', source, re.M)
    check(found == EXPECTED_LAYERS, f"Expected seven layers in order {EXPECTED_LAYERS}; got {found}")
    check("miryoku.dtsi" not in "\n".join(
        ln for ln in source.splitlines() if ln.lstrip().startswith("#include")
    ), "V2.1 must not include the legacy 10-layer Miryoku keymap")
    check("&u_mt" not in source, "Home row mod-tap must not be reintroduced on Fast Base")

    blocks = dict(re.findall(
        r'^\s*(base|nav|mouse|media|num|sym|fun)\s*\{(.*?)^\s*\};',
        source, re.M | re.S
    ))
    for name in EXPECTED_LAYERS:
        block = blocks.get(name, "")
        rows = re.findall(r'/\* Row [123] \*/([^\n]+)', block)
        thumbs = re.findall(r'/\* Thumbs \*/([^\n]+)', block)
        check(len(rows) == 3, f"{name}: expected exactly three matrix rows")
        check(len(thumbs) == 1, f"{name}: expected one thumb row")
        for idx, row in enumerate(rows):
            check(binding_count(row) == 12, f"{name} row {idx + 1}: expected 12 bindings")
            bindings = re.findall(r"&[A-Za-z_]\w*", row)
            check(bool(bindings) and bindings[0] == "&none" and bindings[-1] == "&none",
                  f"{name} row {idx + 1}: absent outer keys must be &none")
        if thumbs:
            check(binding_count(thumbs[0]) == 6, f"{name}: expected 6 thumb bindings")
        if len(rows) == 3 and thumbs:
            check(sum(map(binding_count, rows)) + binding_count(thumbs[0]) == 42,
                  f"{name}: expected exactly 42 Corne shield bindings")

    base = blocks.get("base", "")
    for binding in EXPECTED_THUMBS:
        check(binding in base, f"Base thumb mapping changed: {binding}")
    check(binding_count(" ".join(re.findall(r'/\* Row [123] \*/([^\n]+)', base))) == 36,
          "Base must have three rows of 12 shield bindings")
    check("BT_CLR" in blocks.get("media", ""), "BLE clear binding missing from Media")
    check(all(f"BT_SEL {idx}" in blocks.get("media", "") for idx in range(5)),
          "Not all five BLE profile select bindings are present")
    check("&sk LCTRL" in blocks.get("nav", ""), "One-shot Ctrl missing")
    check("&sk LSHFT" in blocks.get("nav", ""), "One-shot Shift missing")
    check("bindings = <&none>, <&bootloader>;" in source, "Bootloader double-tap guard missing")
    check(all(b in blocks.get("mouse", "") for b in ("&mmv", "&msc", "&mkp")),
          "Mouse emulation bindings missing")
    check(re.search(r"^CONFIG_ZMK_POINTING=y$", conf, re.M) is not None,
          "Pointing feature must be enabled on central (corne_left.conf)")
    check("zmk_sha:" in workflow and re.search(r"[a-f0-9]{40}", workflow) is not None,
          "Build workflow must pin an exact ZMK SHA")
    check("git checkout --detach" in main and "zmk-revision.txt" in main,
          "Reusable workflow must check out and record pinned revision")


if __name__ == "__main__":
    verify()
    if errors:
        print("FAIL: Corne Fast Base V2.1 static checks")
        for err in errors:
            print("  -", err)
        sys.exit(1)
    print("PASS: 7 layers, 42 shield bindings per layer, 36 physical keys,")
    print("      thumb layout, BLE controls, mouse support, ZMK pin, bootloader guard")
