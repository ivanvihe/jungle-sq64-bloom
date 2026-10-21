#!/usr/bin/env python3
"""Candidate-specific native-emulator acceptance test."""
import argparse
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
from headless_test import boot, set_row  # MIT-licensed baseline harness


def note_ons(data):
    result = []
    index = 0
    while index < len(data):
        status = data[index]
        if status >= 0xF8:
            index += 1
        elif 0x80 <= status <= 0xEF and index + 2 < len(data):
            if status & 0xF0 == 0x90 and data[index + 2]:
                result.append((status & 0x0F, data[index + 1], data[index + 2]))
            index += 3
        else:
            index += 1
    return result


def performance(image):
    harness = boot(image)
    harness.run(1_200_000)
    harness.din_bytes()
    # PATTERN: row 7, bit 18. Enable the generative performance layer.
    set_row(harness, 7, 1 << 18, 400_000)
    set_row(harness, 7, 0, 400_000)
    data = harness.exchange([0xFA], 10_000_000)
    notes = note_ons(data)
    assert len(notes) > 12, "generative run emitted too few notes"
    a_notes = {note for channel, note, _ in notes if channel == 0}
    b_notes = {note for channel, note, _ in notes if channel == 1}
    assert len(a_notes) > 2, "fractal track A did not vary pitch"
    assert len(b_notes) > 2, "Turing track B did not vary pitch"
    # SHIFT+STOP freezes mutation but must not issue a transport stop.
    set_row(harness, 7, 1 << 17, 300_000)
    set_row(harness, 0, 1 << 1, 300_000)
    set_row(harness, 0, 0, 300_000)
    set_row(harness, 7, 0, 300_000)
    after_freeze = harness.run(2_000_000)
    assert not after_freeze["critical"]["terminated"]
    data += bytes(harness.din_bytes())
    return bytes(data), a_notes, b_notes, after_freeze


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--image", type=Path, default=ROOT / "build" / "jungle-sq64-bloom.vsb")
    parser.add_argument("--emulator", type=Path)
    args = parser.parse_args()
    if args.emulator:
        sys.path.insert(0, str(args.emulator))
    image = args.image.read_bytes()
    first, a_notes, b_notes, report = performance(image)
    second, _, _, _ = performance(image)
    assert first == second, "same candidate and controls did not replay identically"
    assert report["error"] is None
    print(f"PASS: emulator generative replay; A notes={sorted(a_notes)}, B notes={sorted(b_notes)}")


if __name__ == "__main__":
    main()
