# Test report — 2026-10-21

Candidate source branch: `feature/generative-bloom`.

## Executed successfully

- Independent spec v0.8.0: 73/73 Python tests passed.
- Emulator: 46 passed, one C-probe test skipped because its process did not
  inherit the separately unpacked Arm toolchain.
- `make host-test`: six suites passed, including exhaustive Euclidean
  length/pulse counts and a 36,000-tick (30 minutes at 300 BPM) accelerated run.
- GNU Arm Embedded 16.2 build completed; browser straight-line budget maximum
  was 54 instructions (limit 80).
- VSB package generated at 229,632 bytes.
- Spec v0.8.0 image profile: PKG-001..005 and BOOT-001..002 all passed.
- Native emulator executed 2,000,000 instructions: status `paused`, no error,
  no critical termination. All five critical hazard rows remained `not-run`,
  which means no observed trigger, **not** a safety certificate.
- Candidate-specific emulator acceptance passed twice with byte-identical MIDI:
  fractal A produced eight distinct pitches, Turing B nine distinct pitches,
  and the panel freeze chord caused no critical termination.
- Passive physical-device enumeration only: USB `0944:0146`, ALSA card `SQ64`,
  three sequencer ports and raw MIDI endpoints were present. No MIDI/SysEx was
  sent and no device state was changed.

## Not run / not yet sufficient

- Browser Playwright/UI acceptance for the modified candidate.
- Browser-engine assertions for PATTERN/SHIFT mappings (native assertions pass).
- External MIDI clock, analog clock/reset, CV calibration or physical timing.
- Persistence/corruption tests for generative state (format not implemented).
- Any flash/install/recovery test on physical hardware.

Reports are retained under `build/` and are candidate-specific. Rebuilding
changes the image hash and requires fresh validation.
