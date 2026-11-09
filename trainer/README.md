# LQE2HC Layer Tutor — Corne 36-key ZMK

A personalized, offline-capable practice app derived from the MIT-licensed
[Layer Tutor](https://github.com/thebiglaskowski/layer-tutor) by Joe Laskowski.

**This is a vendored derivative within `quyleduc/miryoku_zmk`, not a GitHub
fork object**, because the available GitHub integration cannot create forks or
new repositories. The original source and license are preserved.

## What matches the actual keyboard?

The source of truth is `../config/corne.keymap`, copied from the already
tested Fast Base V2.1 branch. It has **36 physical keys (3×5+3 each side)** and
seven layers:

| Layer | Trigger in actual ZMK firmware | Use |
| --- | --- | --- |
| 0 Base | Normal | Direct QWERTY; no Home Row Mods |
| 1 Nav | Hold **left Space** | Arrow keys, clipboard, one-shot Ctrl/Shift |
| 2 Mouse | Hold **left Tab** | Cursor movement, scroll, mouse clicks |
| 3 Media | Hold **left Esc** | USB/BLE, profile switching, BLE Clear |
| 4 Num | Hold **right Backspace** | Digits, square brackets, equals, slash |
| 5 Sym | Hold **right Enter** | Curly braces, operators, colon, punctuation |
| 6 Fun | Hold **right Delete** | Function keys |

The trainer highlights the correct **physical key and thumb hold**, including
the correct 7-layer map and the location of **one-shot Shift**:
hold left Space, tap physical right P, release, then type the letter.

**Browser limitation:** standard keyboard events indicate the resulting
character or arrow, **not which physical ZMK thumb was held**. This app
*coaches* the hold visually and validates the emitted character; it does not
claim to verify the actual sequence of holds. One-shot Ctrl shortcuts, F1–F12,
Media, and Mouse are available on the seven-layer visual cheat sheet; some
are intercepted by browsers and are not reliably scoreable as key exercises.

## Training curriculum

Fifteen structured levels cover Base/home row, Telex keystroke sequences,
Num (right Backspace), Sym (right Enter), arrow navigation (left Space),
one-shot Shift, embedded C bitwise and Python syntax. Missing characters and
mapping drift are checked in CI. Accuracy and WPM are saved locally in your
browser under the dedicated `lqe2hc-layer-tutor-v1` key.

**Vietnamese Telex note:** ASCII exercises train keystroke sequences
(`tieengs vieetj`). If a Vietnamese IME composes those sequences into
non-ASCII characters, disable IME while practicing key positions in the web
tutor. This is not a Vietnamese text-input correctness test.

## Run locally on Windows

Requires Python 3 or an equivalent local static HTTP server.
No npm install / dependencies / compiler.

```powershell
git clone --branch feature/lqe2hc-layer-tutor https://github.com/quyleduc/miryoku_zmk.git
cd miryoku_zmk\trainer\typing-tutor
python -m http.server 8000
```

Open <http://localhost:8000>. Allow browser keystroke focus in the tutor.
For offline installation, serve it once over localhost or HTTPS and install
as a PWA from the browser menu. The app includes its own service worker,
SVG, PNG icons and offline asset list.

## Automated tests

```powershell
cd trainer\typing-tutor
node --test tests/*.test.js
```

Or inspect the GitHub Actions workflow
`.github/workflows/lqe2hc-trainer.yml` for Node 20 tests, syntax checks,
PWA cache integrity and a source snapshot comparison.

## Deploy

A **manual** GitHub Pages workflow is provided under
`.github/workflows/lqe2hc-pages.yml`. GitHub exposes `workflow_dispatch`
for workflows present on the **default branch**. This feature branch and its
PR currently target `feature/miryoku-fast-base-v2`; the Pages workflow
will need to reach the repository default branch before its manual Run button
is available. Enable GitHub Pages with **GitHub Actions** as the source,
then trigger it explicitly if you want public hosting.
Deployment would replace the Pages site for this repository, so do not run
it if the repo already hosts an unrelated site.

The expected site URL after a successful GitHub Pages deployment is
`https://quyleduc.github.io/miryoku_zmk/`; this URL is **not claimed
live** until the workflow has actually succeeded.

## Upstream provenance

- Original upstream repository: <https://github.com/thebiglaskowski/layer-tutor>
- Pinned upstream commit: `ffde7dfb4ad0e1bbee7ffa08df84b554e18a4ec4`
- Upstream copyright and MIT license: `LICENSE`
- Full original README: `UPSTREAM_README.md`
- Upstream JS, CSS, tests and PWA source are preserved under `typing-tutor/`
  with LQE2HC-specific modifications.

The current ZMK keymap is not modified by the web trainer. No firmware
flash, Bluetooth access, QMK/Vial firmware or keyboard-side instrumentation
is required.
