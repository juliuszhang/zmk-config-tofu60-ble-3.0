# Tofu60 BLE 3.0 — macOS HHKB firmware

This repository builds only the macOS HHKB layout. The dedicated
`tofu60_ble_v3_hhkb` shield keeps the vendor-tested matrix wiring; the two
unused outer bottom-row matrix positions are disabled.

## Layout

- Layer 0 — macOS: `Option`, `Command`, Space, `Command`, `Option`
- Layer 1 — HHKB Fn layer
- Control replaces Caps Lock.
- Backspace is at the end of the QWERTY row.
- Grave is at the top-right.
- Fn is at the bottom-right of the Shift row.

Holding either Control key changes the Vim home-row keys into unmodified arrow
keys:

- `Ctrl+H` — Left
- `Ctrl+J` — Down
- `Ctrl+K` — Up
- `Ctrl+L` — Right

On the Fn layer, `U` selects USB and `I`, `O`, `P` select Bluetooth
profiles 1–3. Holding a Bluetooth profile key for three seconds clears that
profile. Hold `Fn+B` for about three seconds to enter the bootloader.

ZMK Studio is intentionally disabled so persisted Studio layout data cannot
interfere with the fixed HHKB keymap.

## Firmware files

- `tofu60_ble_v3_hhkb-klink-zmk.uf2` — normal firmware
- `tofu60_ble_v3_hhkb_CLEAR-klink-zmk.uf2` — recovery build that clears
  persistent settings on every boot

The recovery build uses the USB name `HHKB CLEAR USB` and Bluetooth name
`HHKB CLEAR`, so it is visibly different from the normal firmware. Do not
leave the recovery build installed.

## Safe flashing without a physical reset button

1. On the currently working firmware, hold `Fn+B` for about three seconds.
2. Flash `tofu60_ble_v3_hhkb_CLEAR-klink-zmk.uf2`.
3. Wait for it to boot and clear the persisted settings.
4. Hold `Fn+B` for about three seconds to enter the bootloader again.
5. Flash `tofu60_ble_v3_hhkb-klink-zmk.uf2`.

The recovery build clears Bluetooth pairings and other saved ZMK settings.
Re-pair Bluetooth devices after the normal firmware is installed.
