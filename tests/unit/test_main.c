#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "core/prng.h"
#include "core/scheduler.h"
#include "engines/euclidean.h"
#include "engines/fractal.h"
#include "engines/generative.h"
#include "engines/turing.h"
#include "music/quantizer.h"
#include "storage/project_codec.h"

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

typedef struct {
    bloom_event_t events[32];
    unsigned count;
} event_log_t;

static void record_event(const bloom_event_t *event, void *context)
{
    event_log_t *log = context;
    assert(log->count < 32);
    log->events[log->count++] = *event;
}

static void test_scheduler(void)
{
    bloom_scheduler_t scheduler;
    event_log_t log = {0};
    bloom_scheduler_init(&scheduler);
    assert(bloom_scheduler_note(&scheduler, 100, 120, 1, 60, 100, 55, 50, 4));
    assert(scheduler.count == 8);
    assert(bloom_scheduler_poll(&scheduler, 99, record_event, &log) == 0);
    assert(bloom_scheduler_poll(&scheduler, 220, record_event, &log) == 8);
    assert(log.count == 8 && scheduler.count == 0);
    for (unsigned i = 0; i < 4; i++) {
        assert(log.events[i * 2].type == BLOOM_EVENT_NOTE_ON);
        assert(log.events[i * 2 + 1].type == BLOOM_EVENT_NOTE_OFF);
        assert(log.events[i * 2].due_ms == 100u + i * 30u);
        assert(log.events[i * 2 + 1].due_ms == 115u + i * 30u);
    }

    /* Signed-delta scheduling remains valid across the uint32_t wrap. */
    bloom_scheduler_init(&scheduler);
    log.count = 0;
    assert(bloom_scheduler_note(&scheduler, UINT32_MAX - 4u, 8, 0, 64, 90, 0, 50, 2));
    assert(bloom_scheduler_poll(&scheduler, 4, record_event, &log) == 4);
    assert(log.count == 4);

    bloom_scheduler_init(&scheduler);
    for (unsigned i = 0; i < BLOOM_EVENT_CAPACITY / 2u; i++)
        assert(bloom_scheduler_note(&scheduler, i, 10, 0, 60, 100, 0, 50, 1));
    assert(!bloom_scheduler_note(&scheduler, 1000, 10, 0, 60, 100, 0, 50, 1));
    assert(scheduler.dropped == 1);
    bloom_scheduler_cancel_track(&scheduler, 0);
    assert(scheduler.count == 0);
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
    bloom_fractal_t a, b, original, temporary;
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

    temporary = original;
    temporary.mutation_probability = 100;
    temporary.mutation_intensity = 40;
    bloom_step_t base[BLOOM_MAX_STEPS];
    for (uint8_t i = 0; i < temporary.length; i++) base[i] = bloom_fractal_step(&temporary, i);
    bloom_fractal_mutate(&temporary, 123, false);
    unsigned temporary_changes = 0;
    for (uint8_t i = 0; i < temporary.length; i++) {
        bloom_step_t changed = bloom_fractal_step(&temporary, i);
        temporary_changes += changed.note != base[i].note || changed.gate != base[i].gate;
        assert(temporary.trunk[i].note == original.trunk[i].note);
    }
    assert(temporary_changes > 0);
    bloom_fractal_clear_temporary(&temporary);
    for (uint8_t i = 0; i < temporary.length; i++) {
        bloom_step_t restored = bloom_fractal_step(&temporary, i);
        assert(memcmp(&restored, &base[i], sizeof(restored)) == 0);
    }
}

static void test_additional_engines(void)
{
    bloom_random_walk_t walk_a, walk_b;
    bloom_random_walk_init(&walk_a, 1234, 48, 72);
    bloom_random_walk_init(&walk_b, 1234, 48, 72);
    walk_a.scale = walk_b.scale = BLOOM_SCALE_DORIAN;
    for (int i = 0; i < 4096; i++) {
        uint8_t a = bloom_random_walk_tick(&walk_a);
        uint8_t b = bloom_random_walk_tick(&walk_b);
        assert(a == b && a >= 48 && a <= 72);
        assert(bloom_note_in_scale(a, walk_a.root, walk_a.scale));
    }

    bloom_weighted_t weighted_a, weighted_b;
    bloom_weighted_init(&weighted_a, 9001, 36, BLOOM_SCALE_NATURAL_MINOR);
    bloom_weighted_init(&weighted_b, 9001, 36, BLOOM_SCALE_NATURAL_MINOR);
    for (int i = 0; i < 12; i++) weighted_a.degree_weights[i] = weighted_b.degree_weights[i] = 0;
    weighted_a.degree_weights[4] = weighted_b.degree_weights[4] = 1;
    weighted_a.repeat_bias = weighted_b.repeat_bias = 0;
    for (int i = 0; i < 512; i++) {
        uint8_t a, b;
        assert(bloom_weighted_tick(&weighted_a, &a));
        assert(bloom_weighted_tick(&weighted_b, &b));
        assert(a == b && bloom_note_in_scale(a, 36, BLOOM_SCALE_NATURAL_MINOR));
    }
    weighted_a.rest_probability = 100;
    { uint8_t note; assert(!bloom_weighted_tick(&weighted_a, &note)); }

    bloom_markov_t markov_a, markov_b;
    bloom_markov_init(&markov_a, 55, 48, BLOOM_SCALE_PHRYGIAN, 2);
    bloom_markov_init(&markov_b, 55, 48, BLOOM_SCALE_PHRYGIAN, 2);
    for (int i = 0; i < 2048; i++) {
        uint8_t a, b;
        assert(bloom_markov_tick(&markov_a, &a));
        assert(bloom_markov_tick(&markov_b, &b));
        assert(a == b && bloom_note_in_scale(a, 48, BLOOM_SCALE_PHRYGIAN));
    }

    bloom_arp_t arp;
    bloom_arp_init(&arp, 8, 48, BLOOM_SCALE_MINOR_PENTATONIC);
    arp.octave_probability = 0;
    arp.mode = BLOOM_ARP_ALTERNATING;
    uint8_t sequence[7];
    for (int i = 0; i < 7; i++) sequence[i] = bloom_arp_tick(&arp);
    assert(sequence[0] == sequence[6] && sequence[1] == sequence[5]
           && sequence[2] == sequence[4]);

    bloom_harmony_t harmony;
    bloom_harmony_init(&harmony, 48, BLOOM_SCALE_NATURAL_MINOR);
    for (uint32_t bar = 0; bar < 32; bar++)
        for (uint8_t tone = 0; tone < 4; tone++) {
            uint8_t note = bloom_harmony_note(&harmony, bar, tone, 0, 36, 84);
            assert(note >= 36 && note <= 84);
            assert(bloom_note_in_scale(note, harmony.root, harmony.scale));
        }
}

static void test_project_codec(void)
{
    bloom_project_t source, decoded;
    uint8_t encoded[BLOOM_PROJECT_ENCODED_SIZE];
    uint8_t replay[BLOOM_PROJECT_ENCODED_SIZE];
    bloom_project_defaults(&source);
    source.bpm = 137;
    source.tracks[0].steps[0].note = 61;
    source.tracks[3].steps[63].note = 99;
    source.tracks[3].steps[63].flags = BLOOM_STEP_ACCENT;
    assert(bloom_project_encode(&source, encoded, sizeof(encoded)) == BLOOM_PROJECT_OK);
    assert(bloom_project_decode(&decoded, encoded, sizeof(encoded)) == BLOOM_PROJECT_OK);
    assert(decoded.bpm == 137 && decoded.tracks[0].steps[0].note == 61);
    assert(decoded.tracks[3].steps[63].note == 99);
    assert(bloom_project_encode(&decoded, replay, sizeof(replay)) == BLOOM_PROJECT_OK);
    assert(memcmp(encoded, replay, sizeof(encoded)) == 0);

    encoded[777] ^= 0x20;
    assert(bloom_project_decode(&decoded, encoded, sizeof(encoded)) == BLOOM_PROJECT_BAD_CHECKSUM);
    encoded[777] ^= 0x20;
    encoded[4] = 2;
    assert(bloom_project_decode(&decoded, encoded, sizeof(encoded)) == BLOOM_PROJECT_BAD_VERSION);
    encoded[4] = BLOOM_PROJECT_VERSION;
    assert(bloom_project_decode(&decoded, encoded, 12) == BLOOM_PROJECT_TOO_SMALL);
    source.tracks[2].length = 0;
    assert(bloom_project_encode(&source, encoded, sizeof(encoded)) == BLOOM_PROJECT_BAD_DATA);
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
    test_scheduler();
    test_quantizer();
    test_euclidean();
    test_turing();
    test_fractal();
    test_additional_engines();
    test_project_codec();
    test_accelerated_performance();
    puts("PASS: 9 suites, deterministic 30-minute accelerated performance");
    return 0;
}
