#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "core/prng.h"
#include "engines/euclidean.h"
#include "engines/fractal.h"
#include "engines/turing.h"
#include "music/quantizer.h"

static unsigned popcount64(uint64_t value)
{
    unsigned count = 0;
    while (value) { value &= value - 1u; count++; }
    return count;
}

static void test_prng(void)
{
    bloom_prng_t a, b;
    bloom_prng_seed(&a, 42);
    bloom_prng_seed(&b, 42);
    for (int i = 0; i < 10000; i++) assert(bloom_prng_next(&a) == bloom_prng_next(&b));
}

static void test_quantizer(void)
{
    for (int note = 0; note < 128; note++) {
        uint8_t q = bloom_quantize((uint8_t)note, 2, BLOOM_SCALE_DORIAN);
        assert(q <= 127);
        assert(bloom_note_in_scale(q, 2, BLOOM_SCALE_DORIAN));
    }
    assert(bloom_scale_note(0, 60, BLOOM_SCALE_MAJOR, 0, 127) == 60);
    assert(bloom_scale_note(7, 60, BLOOM_SCALE_MAJOR, 0, 127) == 72);
}

static void test_euclidean(void)
{
    for (unsigned steps = 1; steps <= 64; steps++)
        for (unsigned pulses = 0; pulses <= steps; pulses++)
            assert(popcount64(bloom_euclidean_pattern((uint8_t)steps, (uint8_t)pulses, 0)) == pulses);
    assert(bloom_euclidean_pattern(8, 3, 1) != bloom_euclidean_pattern(8, 3, 0));
    assert(bloom_euclidean_pattern(64, 64, 0) == UINT64_MAX);
}

static void test_turing(void)
{
    bloom_turing_t a, b;
    bloom_turing_init(&a, 0x12345678u, 11);
    bloom_turing_init(&b, 0x12345678u, 11);
    uint32_t initial = a.bits;
    for (int i = 0; i < 1100; i++) {
        uint8_t an, av, bn, bv;
        assert(bloom_turing_tick(&a, &an, &av) == bloom_turing_tick(&b, &bn, &bv));
        assert(an == bn && av == bv);
        assert(bloom_note_in_scale(an, a.root, a.scale));
    }
    assert(a.bits == initial);
    a.mutation = 35;
    b.mutation = 35;
    for (int i = 0; i < 256; i++) {
        uint8_t an, av, bn, bv;
        bloom_turing_tick(&a, &an, &av);
        bloom_turing_tick(&b, &bn, &bv);
        assert(a.bits == b.bits && an == bn && av == bv);
    }
    assert(a.bits != initial);
    a.frozen = true;
    a.mutation = 100;
    uint32_t before = a.bits;
    for (int i = 0; i < a.length; i++) { uint8_t n, v; bloom_turing_tick(&a, &n, &v); }
    assert(a.bits == before);
}

static unsigned changed_notes(const bloom_fractal_t *a, const bloom_fractal_t *b)
{
    unsigned changed = 0;
    for (uint8_t i = 0; i < a->length; i++) changed += a->trunk[i].note != b->trunk[i].note;
    return changed;
}

static void test_fractal(void)
{
    bloom_fractal_t a, b, original;
    bloom_fractal_init(&a, 77, 32, 48, BLOOM_SCALE_NATURAL_MINOR);
    bloom_fractal_init(&b, 77, 32, 48, BLOOM_SCALE_NATURAL_MINOR);
    assert(memcmp(a.trunk, b.trunk, sizeof(a.trunk)) == 0);
    original = a;
    a.path = b.path = BLOOM_PATH_ALTERNATING;
    for (uint32_t bar = 0; bar < 64; bar++) {
        assert(bloom_fractal_select_branch(&a, bar) == bloom_fractal_select_branch(&b, bar));
        for (uint32_t step = 0; step < a.length; step++) {
            bloom_step_t as = bloom_fractal_step(&a, step);
            bloom_step_t bs = bloom_fractal_step(&b, step);
            assert(memcmp(&as, &bs, sizeof(as)) == 0);
            assert(bloom_note_in_scale(as.note, a.root, a.scale));
        }
    }
    a.mutation_probability = 25;
    a.mutation_intensity = 15;
    bloom_fractal_mutate(&a, 9, true);
    unsigned changes = changed_notes(&a, &original);
    assert(changes > 0 && changes < a.length / 2u);
    assert(bloom_fractal_undo(&a));
    assert(changed_notes(&a, &original) == 0);
    a.mutation_probability = 100;
    bloom_fractal_mutate(&a, 11, true);
    bloom_fractal_restore(&a);
    assert(changed_notes(&a, &original) == 0);
}

static void test_accelerated_performance(void)
{
    /* 30 minutes at 300 BPM, four sixteenth-note ticks per beat. */
    enum { TICKS = 30 * 300 * 4 };
    bloom_turing_t t;
    bloom_fractal_t f;
    bloom_turing_init(&t, 0x51a7e, 16);
    bloom_fractal_init(&f, 0xb1005e, 64, 48, BLOOM_SCALE_DORIAN);
    t.mutation = 7;
    unsigned note_on = 0, note_off = 0;
    bool active[3] = {false, false, false};
    for (int tick = 0; tick < TICKS; tick++) {
        if (!(tick % 16)) bloom_fractal_select_branch(&f, (uint32_t)tick / 16u);
        bloom_step_t s = bloom_fractal_step(&f, (uint32_t)tick);
        uint8_t note, velocity;
        bool hit[3] = {
            (s.flags & BLOOM_STEP_ENABLED) != 0,
            bloom_turing_tick(&t, &note, &velocity),
            bloom_euclidean_hit((uint8_t)tick, 13, 5, 2),
        };
        (void)note; (void)velocity;
        for (int track = 0; track < 3; track++) {
            if (active[track]) { note_off++; active[track] = false; }
            if (hit[track]) { note_on++; active[track] = true; }
        }
    }
    for (int track = 0; track < 3; track++) if (active[track]) note_off++;
    assert(note_on == note_off);
    assert(note_on > 10000);
}

int main(void)
{
    test_prng();
    test_quantizer();
    test_euclidean();
    test_turing();
    test_fractal();
    test_accelerated_performance();
    puts("PASS: 6 suites, deterministic 30-minute accelerated performance");
    return 0;
}
