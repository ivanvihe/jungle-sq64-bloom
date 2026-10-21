# Repository guidance

Read `README.md`, `docs/HARDWARE_SAFETY.md`, `docs/ARCHITECTURE.md`, and the
pinned independent SQ-64 spec before modifying firmware behavior.

- This repository is emulator-only until every hardware gate is independently closed.
- Never flash a device, write SysEx, mutate device projects, or claim physical safety without explicit user authorization and verified recovery.
- Keep all sequencing paths bounded and allocation-free. No heap use in firmware.
- Preserve deterministic seed replay. New algorithms need host tests, edge tests at lengths 1/64 where applicable, and accelerated timing tests.
- Hardware registers must come from the machine-readable board contract; do not invent pins, voltages, calibration, MCU identity or storage ownership.
- Run `make host-test`, the ARM build, the spec image profile, and an emulator smoke run before reporting an image as emulator-ready.
- Preserve Aleph Void's MIT attribution for adapted firmware files. Do not copy emulator GPL code into the firmware.
- Mark unexecuted conformance work `not-run`; never describe an API-only feature as complete.
