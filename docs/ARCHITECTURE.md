# Architecture and feasibility audit

## Audit pins

| Source | Revision/license | Finding |
| --- | --- | --- |
| SQ-64 Euclidean firmware | `1fc6579`, MIT | Reusable startup, HAL, panel/display, MIDI, VSB packer and emulator tests. |
| SQ-64 independent spec | `588208d`, MIT, v0.8.0 | ARMv7E-M model, 64 KiB internal image, 256 KiB SRAM, panel/output mappings and validation contract; physical facts remain release-gated. |
| SQ-64 emulator | `ab2e7bc`, GPL-2.0-only | Native/browser execution and critical write guards; used externally, not linked into firmware. |
| Bloom v2 public docs | manual/product pages, v1.1.1 | Three 64-step channels, branches/paths, mutation/reseed, per-step gate/mod/ratchet/slew/ornaments and MIDI; algorithms are undocumented and therefore approximated independently. |

The Euclidean firmware is a suitable base because its code is freestanding C,
bounded, emulator-tested and already separates panel, display, outputs, MIDI,
USB, storage and update functions. Reused files retain upstream history and MIT
attribution. The generative core is newly implemented under `firmware/` and has
no hardware dependency.

## Runtime boundaries

```text
1 ms timer / MIDI transport
        |
bounded 72-event scheduler ---- ratchets / note-off / gate-off
        |
track engine: fractal | Turing | Euclidean | future engines
        |
scale/transform layer ---- deterministic per-track random streams
        |
event ---- routing policy ---- A/B/C CV+gate+mod+MIDI; D triggers+MIDI
        |
SQ-64 HAL (panel, display, outputs, MIDI, storage)
```

No engine allocates memory. Fractal branches are computed lazily from
`seed + level + step + bar`, so eight levels do not require eight stored
sequences. A track's current, original and undo trunks occupy bounded static
arrays. PRNG state is explicit and serializable.

The scheduler keeps the inactive main-loop path O(1) through a cached next
deadline. Scheduling is atomic: a whole ratchet group is rejected before any
event is inserted if capacity is insufficient. Events with equal deadlines use
a monotonic insertion order, and transport stop clears the queue before forcing
all notes/gates off.

The portable `JBLM` project codec is independent of flash I/O. Version 1 is
1,880 bytes and includes global harmony, four engine assignments, clock ratios,
seeds, Turing registers and all four 64-step trunks. CRC32 and semantic range
validation reject damaged records. Device-slot integration remains disabled
until storage ownership is independently verified.

## Verified modeled capabilities

The v0.8.0 contract models three pitch CV, three mod CV and three gate outputs
for tracks A/B/C; eight active-low triggers for D; USB/DIN MIDI; 64 pads,
21 other buttons, six encoders and a 256x64 4-bpp display. CV uses signed
24-bit transport words but voltage conversion and calibration are unverified.
Consequently firmware emits only the upstream model's normalized code and
makes no voltage-range or 1V/oct hardware claim.

## Current limitations

The first vertical slice still uses millisecond timing and one active MIDI note
per logical track, but ratchet subdivisions are now scheduled safely. External
MIDI/analog clock, full UI pages, installed generative persistence, calibrated
CV and grace-note/slew scheduling remain future work. See roadmap.
