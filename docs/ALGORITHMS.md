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
pitch/gate edits, and can restore the immutable original. Freeze rejects new
mutation. Reseed can independently replace notes and rhythm fields.

## Euclidean rhythm

The implementation uses the equivalent modular form of Bjorklund distribution:
step `i` is active when `((i - rotation) * pulses) mod steps < pulses`.
It has fixed O(steps) pattern generation, O(1) hit testing and exactly the
requested pulse count for every 1–64-step configuration.
