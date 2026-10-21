# Development roadmap and status

| Area | Status | Evidence / next work |
| --- | --- | --- |
| Upstream audit and licenses | Implemented and tested | Three pinned repositories inspected; attribution preserved. |
| Reproducible host build | Implemented and tested | `make host-test`, strict C11 warnings. |
| ARM/VSB build | Implemented and tested | GNU Arm 16.2; block-budget and image-profile checks. |
| Quantizer | Implemented and tested | 14 scales, membership and degree tests. User scales pending. |
| Turing engine | Implemented and tested | 2–32 bits, mutation/lock/rest/freeze/reseed; richer modes partial. |
| Fractal trunk/branch/path | Implemented and tested | 64 steps, eight lazy levels, six paths, return-to-trunk. |
| Mutation/undo/restore | Implemented and tested | Engine layer; temporary mutation and panel editing partial. |
| Euclidean engine | Implemented and tested | 1–64, pulses/rotation; evolution/fills/accent scheduling partial. |
| Ratchet event scheduler | Implemented and tested | 72 static events, 1–8 ratchets, wrap, overflow and stop cleanup. |
| Random Walk/Weighted/Markov/Arp/Harmony | Implemented and tested | Host determinism plus WASM track-C acceptance. |
| Firmware routing | Implemented and tested | Native MIDI replay plus browser panel/CV/MIDI tests. |
| Performance UI | Partial | Toggle/reseed/freeze vertical slice only. |
| Ratchets/ornaments/slew | Partial | Ratchets scheduled; grace notes, trills and slew pending. |
| External MIDI/analog sync | Not implemented | Incoming start/stop only from upstream. |
| Versioned project persistence | Partial | `JBLM` v1 codec and corruption tests pass; physical slot integration blocked. |
| Browser acceptance scripts | Implemented and tested | Update, panel/storage and all track-C engines covered. |
| Physical hardware | Blocked by hardware verification | See `HARDWARE_SAFETY.md`. |

Next milestone: per-track division/multiplication and external-clock state
tests, then the nine-page UI and emulator-only integration of `JBLM` snapshots.
