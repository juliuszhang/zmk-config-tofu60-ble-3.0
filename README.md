# zmk-config-tofu60-ble-3.0

## HHKB build

The `tofu60_ble_v3_hhkb` shield uses the vendor-tested ANSI 7U matrix with
the `layout3` physical layout and a macOS-only HHKB keymap. The two unused
outer bottom-row matrix positions are disabled.

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

ZMK Studio is intentionally disabled for this build so persisted Studio
layout/keymap data cannot interfere with the fixed HHKB keymap.

### Safe flashing sequence

Changing from a Studio-enabled build may leave persistent layout or keymap
settings in flash. Flash the generated files in this order:

1. `settings_reset-klink-zmk.uf2`
2. `tofu60_ble_v3_hhkb-klink-zmk.uf2`

The settings reset clears Bluetooth pairings and other saved ZMK settings.
Re-pair Bluetooth devices after flashing the HHKB firmware.
