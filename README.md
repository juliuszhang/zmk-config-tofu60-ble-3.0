# zmk-config-tofu60-ble-3.0

## HHKB build

The `tofu60_ble_v3_hhkb` shield adds a true 60-key HHKB physical layout and
uses a macOS-only keymap:

- Layer 0 — macOS: `Option`, `Command`, Space, `Command`, `Option`
- Layer 1 — HHKB Fn layer
- Layers 2–7 — reserved

The HHKB conventions are preserved: Control replaces Caps Lock, Backspace is
at the end of the QWERTY row, grave is at the top-right, and Fn is at the
bottom-right of the Shift row.

### Ctrl navigation

Holding either Control key changes the Vim home-row keys into unmodified arrow
keys:

- `Ctrl+H` — Left
- `Ctrl+J` — Down
- `Ctrl+K` — Up
- `Ctrl+L` — Right

The GitHub Actions artifact for this layout is built with:

```text
board: klink
shield: tofu60_ble_v3_hhkb
```
