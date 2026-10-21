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
| Firmware routing | Implemented but not validated | Emulator smoke boot; detailed generative event capture pending. |
| Performance UI | Partial | Toggle/reseed/freeze vertical slice only. |
| Ratchets/ornaments/slew | Partial | Data model/branch generation only; scheduler pending. |
| External MIDI/analog sync | Not implemented | Incoming start/stop only from upstream. |
| Random Walk/Weighted/Markov/Arp/Harmony | Not implemented | Phase 3. |
| Versioned project persistence | Not implemented | Legacy settings/project format does not save generative state. |
| Browser acceptance scripts | Partial | Upstream scripts retained; new generative scripts pending. |
| Physical hardware | Blocked by hardware verification | See `HARDWARE_SAFETY.md`. |

Next milestone: introduce a transport/event-queue module with ratchet-safe
note-offs, per-track division/multiplication and external-clock state tests;
then persist versioned engine state and expose the nine-page UI.
