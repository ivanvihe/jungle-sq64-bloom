# Emulator session checklist

This checklist is for the browser emulator only. The candidate is **not**
approved for installation on a physical SQ-64.

## Start

```sh
cd /home/ivan/git/sq64-upstream-audit/emulator/web
npm run dev
```

Open the printed local URL, choose **Browser emulator**, and load:

`/home/ivan/git/jungle-sq64-bloom/build/jungle-sq64-bloom.vsb`

Power the emulated unit on and press PLAY. Press PATTERN to enter the
generative layer.

## What to verify

1. Track A emits a related but evolving fractal melody.
2. Track B emits a stable Turing melody; SHIFT+STOP freezes/unfreezes it.
3. Select track C and press SHIFT+C repeatedly to cycle `EUCL`, `WALK`,
   `WGHT`, `MARK`, `ARPG`, and `HARM`.
4. Track D continues to emit Euclidean percussion triggers.
5. SHIFT+PATTERN reseeds the generative streams reproducibly.
6. STOP releases all gates and pending MIDI notes.

The current compact control mapping is documented in `CONTROL_MAPPING.md`.
Project codec serialization is tested on the host but is not yet connected to
the SQ-64 project-slot UI; the inherited compatibility project format remains
active in the emulator firmware.

## Inspect generated music

```sh
cd /home/ivan/git/jungle-sq64-bloom
make diagnostic > build/events.csv
less -S build/events.csv
```

The CSV includes track, engine, step, pitch, gate, velocity, modulation and
ratchet fields. Its deterministic output is suitable for comparing changes.

## Automated acceptance

```sh
PATH=/home/ivan/.cache/jungle-sq64-toolchain/root/usr/bin:$PATH \
make test \
  PYTHON=/home/ivan/git/sq64-upstream-audit/emulator/.venv/bin/python \
  EMULATOR_DIR=/home/ivan/git/sq64-upstream-audit/emulator
```

Do not use the emulator's successful simulated updater test as authorization
to update the connected hardware. See `HARDWARE_SAFETY.md` for the unresolved
MCU, storage, calibration, bootloader and recovery blockers.
