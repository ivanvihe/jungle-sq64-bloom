# Prototype control mapping

| Control | Action |
| --- | --- |
| PLAY / STOP | Start / stop and balance all active MIDI notes and gates. |
| PATTERN | Toggle upstream Euclidean compatibility mode and generative mode. |
| SHIFT + PATTERN | Deterministically advance seed and reseed fractal/Turing tracks. |
| SHIFT + STOP | Freeze/unfreeze fractal/Turing evolution and the current track-C pitch. |
| SHIFT + C | While not in Project mode, cycle track C through Euclidean, Random Walk, Weighted, Markov, Arp and Harmony. |
| A/B/C/D | Select track for existing Euclidean parameter editing. |
| PITCH/GATE/MOD/CONTROL | Existing per-track edit modes. |
| BAR 1–4 | Select the visible 16-step window. |
| GLOBAL | Commit legacy settings only; generative state is not persisted yet. |

Generative mode routes A to fractal melody, B to Turing bass/melody, C to the
selected pitched generative engine and D to Euclidean trigger/MIDI percussion.
The display title changes to `BLOOM`; `FREEZE` appears when held.

The requested PERFORMANCE, ENGINE, FRACTAL, TURING, RHYTHM, SCALE, MUTATION,
MODULATION and PROJECT pages are not yet implemented. Persistent mutation
controls currently exist in the engine API and host tests but are not exposed
on the panel. This mapping is intentionally non-destructive and provisional.
