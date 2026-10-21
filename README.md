# Jungle SQ-64 Bloom

> **EXPERIMENTAL — EMULATOR ONLY. DO NOT FLASH PHYSICAL HARDWARE.**

Jungle SQ-64 Bloom is an independently developed generative sequencer firmware
prototype for the Korg SQ-64. It combines lazy fractal variations, a true
shift-register Turing engine, scale quantization and Euclidean rhythm while
retaining the proven panel/output baseline from Aleph Void's MIT-licensed
Euclidean firmware.

This is an early playable milestone, not a hardware release. The independent
board contract does not verify the exact MCU, electrical limits, calibration,
protected-storage ownership or recovery path. Emulator success is not evidence
that flashing is safe. See `docs/HARDWARE_SAFETY.md`.

## Working now

- Host-tested deterministic PRNG and seed replay.
- Fourteen built-in scale masks and scale-degree generation.
- Turing register lengths 2–32, gradual mutation, lock, rests and freeze.
- A 1–64-step trunk with up to eight lazy branch levels.
- Sequential, reverse, alternating, deterministic-random, weighted and evolving paths.
- Controlled persistent mutation with undo and original-trunk restore.
- 1–64-step Euclidean patterns, pulses and rotation.
- Fixed-capacity ratchet scheduler with ordered note-on/off events and overflow rejection.
- Random Walk, Weighted Random, first/second-order Markov, arpeggiator and harmonic progression engines.
- Versioned 1,880-byte generative project codec with CRC32 corruption detection.
- ARM firmware image with A/B/C pitch, gate, mod and MIDI routing and D trigger/MIDI routing inherited from the verified emulator interface.
- CSV diagnostic output and an accelerated 30-minute deterministic test.
- VSB packaging and SQ-64 specification image validation.

In the firmware's normal page, `PATTERN` switches between the compatibility
Euclidean layer and the prototype generative layer. In the generative layer,
track A is fractal, B is Turing, C has six selectable engines, and D remains
Euclidean percussion. `SHIFT+PATTERN` reseeds A/B/C; `SHIFT+STOP` freezes or
releases evolution. These deliberately small mappings provide an
emulator-playable vertical slice while the full page system is developed.

While track C is selected, `SHIFT+C` cycles its live engine through Euclidean,
Random Walk, Weighted Random, Markov, arpeggiator and harmonic modes.

## Build and test

Requirements: Python 3, a C11 host compiler, and GNU Arm Embedded. `make` also
supports a Clang bare-metal fallback when its ARM compiler-rt is installed.

```sh
make host-test
make diagnostic > build/events.csv
make
make check SPEC_DIR=/path/to/Korg-SQ-64-Custom-Firmware-Spec
make smoke EMULATOR_DIR=/path/to/Korg-SQ-64-Emulator
```

The firmware artifact is `build/jungle-sq64-bloom.vsb`. To inspect it in the
browser emulator:

```sh
cd /path/to/Korg-SQ-64-Emulator/web
npm ci
npm run dev
```

Open the local URL, load the VSB in **Browser emulator**, press Power, then
PLAY. Press PATTERN to enable Bloom/Turing playback. Nothing in this workflow
requires or authorizes a physical device update.

See `docs/EMULATOR_SESSION.md` for the exact acceptance checklist and controls.

For a readable event stream:

```sh
./build/bloom-diagnostic --steps 512 > build/events.csv
```

## Status and provenance

See `docs/TEST_REPORT.md` for exact executed checks and `docs/DEVELOPMENT_ROADMAP.md`
for explicit status. The hardware/UI base originates from
[Korg-SQ-64-Euclidean-Firmware](https://github.com/aleph-void/Korg-SQ-64-Euclidean-Firmware)
at commit `1fc65790f0f528de355c3a2a8ca3141fda49aaea`, MIT License,
Copyright 2026 Aleph Void LLC. The generative algorithms are original and do
not contain Qu-Bit firmware or proprietary implementation details. Bloom v2
product behavior is used only as public functional inspiration.

## License

MIT. Upstream attribution and license text are preserved in `LICENSE`; see
`NOTICE.md` for dependency and source boundaries.
