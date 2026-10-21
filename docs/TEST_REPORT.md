# Test report — 2026-10-21

Candidate source branch: `feature/generative-bloom`.

Candidate VSB SHA-256:
`bb151e6dd7617e330dbda074a84d2bfbd2e25d37425b328a62f22e5ed5c9094a`.

## Executed successfully

- Independent specification v0.8.0: 73/73 upstream Python tests passed.
- Upstream emulator: 46 tests passed; one C-probe test was skipped because its
  process did not inherit the separately unpacked Arm toolchain.
- `make host-test`: nine suites passed, including deterministic PRNG replay,
  quantization, exhaustive Euclidean cases, Turing mutation, fractal paths and
  temporary restore, fixed-capacity event scheduling, five advanced engines,
  project codec validation, and a 36,000-tick accelerated 30-minute run.
- GNU Arm Embedded 16.2 produced a 229,632-byte VSB. ELF usage was 23,816 bytes
  text and 14,604 bytes BSS (38,420 total); application RAM headroom is tight.
  Browser straight-line basic blocks stayed within the enforced 80-instruction
  bound.
- Specification image profile: PKG-001..005 and BOOT-001..002 all passed for
  the hash above.
- Native emulator ran 2,000,000 instructions with status `paused`, no error and
  no critical termination. Untriggered emulator hazards are not a physical
  safety certificate.
- Headless behavioral compatibility passed normal playback, editing, project
  operations, firmware version reporting and updater-mode checks.
- Candidate generative replay passed with byte-identical MIDI: fractal A
  produced eight distinct pitches and Turing B nine distinct pitches.
- Browser simulated installation transferred and acknowledged all 224 packets;
  emulated internal/SPI flash matched the candidate and no critical hazard
  tripped.
- Browser panel acceptance passed display/LEDs, MIDI start/clock/stop, CV,
  gates, drum triggers, editing and loops. It exercised slots 0/32/63,
  corruption rejection, deletion, GLOBAL commit and cold-restart recovery.
- Browser generative acceptance passed Random Walk, Weighted Random, Markov,
  arpeggiator and harmonic track-C modes; all emitted Dorian pitches. Freeze
  emitted five repetitions of the same pitch.
- The inherited SPI driver was corrected to wait for the flash WIP bit after
  erase/program. The full cold-restart persistence test passes after the fix.
- Passive physical-device enumeration only found USB `0944:0146`, ALSA card
  `SQ64`, three sequencer ports and raw MIDI endpoints. No MIDI/SysEx was sent
  and no device state was changed.

Reports are retained under `build/` and are candidate-specific. Rebuilding
changes the image hash and requires fresh validation.

## Not run / not yet sufficient

- External MIDI clock, analog clock/reset, electrical CV calibration or
  physical timing.
- The new generative project codec is host-tested but is not connected to the
  device slot UI or unverified physical storage regions.
- Full nine-page performance UI and preset loader.
- Any install, flash, recovery or electrical test on physical hardware.

The release remains **EXPERIMENTAL / EMULATOR ONLY**. Physical installation is
blocked until the hardware and recovery items in `HARDWARE_SAFETY.md` are
independently verified and the user explicitly approves that separate phase.
