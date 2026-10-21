# Hardware safety

> **Release state: EXPERIMENTAL / EMULATOR ONLY. DO NOT FLASH.**

The connected SQ-64 may be enumerated passively, but this project must not send
firmware, write SysEx, erase/program storage, alter projects or run power-cut
tests. A successful VSB image check or emulator run is not physical validation.

## Open blockers

- Exact MCU part and board wiring are not independently verified.
- CV voltage limits, polarity, calibration, negative-voltage behavior and safe clamping are unknown.
- Protected SPI/storage ownership is unverified.
- Worst-case interrupt latency, gate timing and electrical behavior are unmeasured.
- There is no independently verified SWD/JTAG recovery method.
- Replacing the 64 KiB internal image also replaces update/recovery code; power loss during programming may be unrecoverable.
- Emulator spec versions differ: the current native emulator embeds v0.7.0 checks while this project separately validates against v0.8.0.

## Recovery prerequisite for any future hardware trial

Before a separately approved trial, retain Korg's official updater VSB, make a
full stock backup, verify the stock GLOBAL+PATTERN update chord, use stable USB
power, validate the exact candidate hash, and have an independently reviewed
write allowlist and recovery plan. Returning to stock would use the updater to
send the official Korg VSB, followed by stock-version verification and backup
restore. This is documentation, not permission or a guarantee.

The upstream Euclidean guide describes a candidate procedure, but its own
warning says the same board and recovery facts remain incomplete. Jungle SQ-64
Bloom therefore applies the stricter emulator-only gate.
