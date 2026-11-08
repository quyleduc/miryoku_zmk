# Corne 36-key — Fast Base V2.1 quick reference

Derived from `config/corne.keymap` on the V2.1 stabilization branch.
These are **physical key positions**, with outer absent shield locations omitted.
The six thumb keys keep the same physical positions across layers even when the behavior changes.

Legend: `·` = no action; `OS-` = one-shot modifier; `BT CLR` clears current profile.
Use these as a printable reference when learning to type without looking down.

## Base (normal typing)
```text
LEFT                    RIGHT
 Q   W   E   R   T       Y   U   I   O   P
 A   S   D   F   G       H   J   K   L   '
 Z   X   C   V   B       N   M   ,   .   /

 ESC SPACE TAB           ENTER BSPC DEL
 [Media] [Nav] [Mouse]  [Sym] [Num] [Fun]   <- when held
```

## Nav — hold LEFT SPACE
```text
BOOT   ·    ·    ·    ·      OS-Ctrl  Paste   Copy  Cut   OS-Shift
GUI   Alt  Ctrl Shift ·      Caps     Left   Down   Up    Right
Undo  Redo OS-Alt OS-GUI ·    Insert   Home   PgDn   PgUp  End

Left thumbs: inactive       Right thumbs: Enter  Backspace  Delete
```
**Ctrl+S:** hold Space, tap physical Y, release Space, then S.
**Shift+A:** hold Space, tap physical P, release Space, then A.
**Bootloader:** hold Space, double tap physical Q (250 ms).

## Sym — hold RIGHT ENTER
```text
{    &    *    (    }         ·    ·    ·    ·    ·
:    $    %    ^    +         ·  Shift Ctrl Alt GUI
~    !    @    #    |         ·    ·    ·  RAlt  ·

Left thumbs: (  )  _         Right thumbs: inactive
```

## Num — hold RIGHT BACKSPACE
```text
[    7    8    9    ]         ·    ·    ·    ·    ·
;    4    5    6    =         ·  Shift Ctrl Alt GUI
`    1    2    3    \         ·    ·    ·  RAlt  ·

Left thumbs: .  0  -         Right thumbs: inactive
```

## Fun — hold RIGHT DELETE
```text
F12  F7   F8   F9   PrtSc      ·    ·    ·    ·    ·
F11  F4   F5   F6   ScrLk      ·  Shift Ctrl Alt GUI
F10  F1   F2   F3   Pause      ·    ·    ·  RAlt  ·

Left thumbs: Menu  Space  Tab  Right thumbs: inactive
```

## Media — hold LEFT ESC
```text
·    ·    ·     ·    ·         OutTog ForceUSB ForceBLE BTnext BTprev
GUI  Alt  Ctrl  Shift BT CLR    ·     Prev     Vol-     Vol+   Next
·   RAlt  ·     ·    ·         BT0     BT1      BT2     BT3    BT4

Left thumbs: inactive        Right thumbs: Stop  Play/Pause  Mute
```
**BLE Clear:** hold left ESC + tap physical G (clears selected profile).
**Select host 0–4:** hold left ESC + tap physical N / M / , / . / /.
**Choose USB output:** hold left ESC + tap physical U.

## Mouse — hold LEFT TAB
```text
·   ·    ·    ·    ·        Redo Paste Copy Cut Undo
GUI Alt Ctrl Shift ·         ·   Move← Move↓ Move↑ Move→
·  RAlt  ·    ·    ·         ·   Wheel← Wheel↓ Wheel↑ Wheel→

Left thumbs: inactive       Right thumbs: RightClick LeftClick MiddleClick
```
Mouse movement: hold TAB + J/K/L/'.
Mouse scroll: hold TAB + J/K/L/' **on the bottom row** instead means
M/,/./ on the physical Base, because that row is N M , . /.
Thus: hold TAB + M/,/./ to scroll left/down/up/right.
Mouse click: hold TAB + right Enter / Backspace / Delete thumb positions.

## Practice order

1. Base QWERTY and thumb roles.
2. Space/Nav: arrows, Ctrl, Shift, copy/paste.
3. Right Enter/Sym and right Backspace/Num.
4. Right Delete/Fun.
5. Left ESC/Media: USB output, Bluetooth profiles and Clear.
6. Left TAB/Mouse when the pointing feature is enabled.

Note: Enabling Mouse changes the HID descriptor. After updating USB firmware,
unplug/replug to force enumeration; for Bluetooth mouse reports, host re-pair may be necessary.
