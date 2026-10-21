# Bloom v2 feature comparison

Public Bloom behavior comes from Qu-Bit's product page and v1.1.1 manual. The
SQ-64 implementation is an independent approximation; `exact` below means an
exact project behavior/test, not copied firmware behavior.

| Feature | Documented Bloom v2 behavior | SQ-64 equivalent / limitation | Status / tests |
| --- | --- | --- | --- |
| Channels/steps | 3 channels, 64 steps each | 4 logical tracks; A/B/C pitched, D trigger; actual SQ-64 routing is asymmetric | Partial; boundary tests |
| Branches & Paths | Related evolving sequence branches | Eight lazy deterministic levels; six structurally different bar paths | Implemented/tested |
| Mutation range | Controlled note variation | Scale-aware probability/intensity/range, persistent undo/restore | Engine tested; UI partial |
| Reseed | New sequence | Deterministic track reseed; note/rhythm preservation API | Tested; global UI partial |
| Per-step tune | Note, gate, slew, ratchet, mod | Step structure carries all except scheduled slew | Partial |
| Ratchets/trills | Clock-synced per-step ornaments | 1–8 ratchets use bounded note-on/off scheduling; trills pending | Ratchets tested |
| Probabilistic mute/slew | Per-step probability features | Deterministic probability is active; slew scheduling pending | Partial |
| Mod output | Per-channel synced modulation/LFO/envelope | Three modeled SQ-64 mod streams; voltage/calibration unknown | Basic stepped mod inherited |
| MIDI output | Sequence output over TRS MIDI | USB and DIN modeled; active note-off balancing inherited | Basic emulator smoke |
| Save/recall branches | Persistent saved sequences | Versioned codec exists; not wired to unverified physical slots | Partial/blocked |
| Editor/import/export | Browser editor and USB files | CSV diagnostic only; no project editor | Partial |

Qu-Bit's internal fractal and mutation algorithms are not public. Parity claims
are therefore conceptual and musical, never bit-for-bit.
