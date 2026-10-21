# Algorithms

## Randomness

Runtime streams use xorshift32; addressable decisions use a four-input integer
hash. Addressable randomness prevents an added UI draw from changing branch
notes. Zero seeds are mapped to a fixed nonzero state. Bounded selection uses
multiply-high to avoid modulo bias.

## Quantizer

Scales are 12-bit pitch-class masks. Pitch quantization searches down then up,
giving stable tie behavior. Scale-degree generation handles negative degrees,
wraps octaves, clamps to a caller-provided MIDI range and requantizes.

## Turing machine

Each tick rotates the least-significant register bit to the top. Mutation may
invert that feedback bit unless the independent lock draw wins. At zero
mutation the exact register repeats after `length` ticks. Pitch is a folded
view of several register taps, then range-limited and quantized. Freeze disables
mutation while rotation/playback continues, preserving a repeating pattern.

## Fractal branches

The trunk stores 1–64 complete steps. A branch is a deterministic transformation
pipeline over its parent levels: sparse scale-aware pitch displacement, sparse
enable-state changes, occasional ratchets and bounded gate changes. Deeper
branches accumulate more changes while retaining the same trunk and seed.
Only the requested step is materialized.

Paths choose a branch at bar boundaries. Sequential, reverse and alternating
are geometric traversals; deterministic-random is addressable; weighted favors
shallow branches; evolving drifts over multiple bars. Return-to-trunk is a
separate deterministic probability.

Persistent mutation snapshots the trunk once for undo, applies scale-aware
pitch/gate edits, and can restore the immutable original. Temporary mutation
stores only an epoch and is applied lazily, so clearing it restores the exact
trunk without another 64-step buffer. Freeze rejects new mutation. Reseed can
independently replace notes and rhythm fields.

## Euclidean rhythm

The implementation uses the equivalent modular form of Bjorklund distribution:
step `i` is active when `((i - rotation) * pulses) mod steps < pulses`.
It has fixed O(steps) pattern generation, O(1) hit testing and exactly the
requested pulse count for every 1–64-step configuration.

## Additional engines

- Random Walk supports biased direction, 1–N step sizes, center attraction and
  clamp/wrap/reflect boundaries before scale quantization.
- Weighted Random independently weights twelve scale degrees and four octave
  bands, with repeat and rest probability.
- Markov Melody contains explicit first-order and second-order transition
  tables over eight scale states. Defaults favor small motion, repetition and
  contrary motion.
- Arpeggiator supports up, down, alternating and deterministic-random traversal
  over eight configurable chord degrees with octave probability.
- Harmonic Generator selects a progression by bar, applies major/minor/
  diminished/suspended intervals and requantizes to the shared scale.

## Scheduling and persistence

Each note request expands into up to eight evenly spaced ratchet note-on/off
pairs. Gate percentage applies within each ratchet slice. The fixed 72-event
queue handles millisecond wraparound and refuses over-capacity groups atomically.

Project records use little-endian fields, magic `JBLM`, version 1, declared
length and CRC32 over the payload. Decode performs both checksum and musical
range validation before accepting a project.
