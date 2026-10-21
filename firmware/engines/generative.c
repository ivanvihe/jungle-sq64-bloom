#include "generative.h"

static uint8_t weighted_pick(bloom_prng_t *rng, const uint8_t *weights, uint8_t count)
{
    uint16_t total = 0;
    for (uint8_t i = 0; i < count; i++) total = (uint16_t)(total + weights[i]);
    if (!total) return 0;
    uint16_t choice = (uint16_t)bloom_prng_bounded(rng, total);
    for (uint8_t i = 0; i < count; i++) {
        if (choice < weights[i]) return i;
        choice = (uint16_t)(choice - weights[i]);
    }
    return (uint8_t)(count - 1u);
}

void bloom_random_walk_init(bloom_random_walk_t *walk, uint32_t seed,
                            uint8_t low_note, uint8_t high_note)
{
    if (low_note > high_note) { uint8_t t = low_note; low_note = high_note; high_note = t; }
    bloom_prng_seed(&walk->rng, seed);
    walk->minimum = low_note;
    walk->maximum = high_note;
    walk->center = (int16_t)((low_note + high_note) / 2u);
    walk->position = walk->center;
    walk->maximum_step = 2;
    walk->upward_weight = 50;
    walk->center_attraction = 25;
    walk->root = low_note;
    walk->scale = BLOOM_SCALE_CHROMATIC;
    walk->boundary = BLOOM_BOUNDARY_REFLECT;
}

uint8_t bloom_random_walk_tick(bloom_random_walk_t *walk)
{
    uint8_t step_max = walk->maximum_step ? walk->maximum_step : 1;
    int step = (int)bloom_prng_bounded(&walk->rng, step_max) + 1;
    bool upward = bloom_prng_bounded(&walk->rng, 100) < walk->upward_weight;
    if (walk->position != walk->center
        && bloom_prng_bounded(&walk->rng, 100) < walk->center_attraction)
        upward = walk->position < walk->center;
    int next = walk->position + (upward ? step : -step);
    if (walk->boundary == BLOOM_BOUNDARY_WRAP) {
        int span = walk->maximum - walk->minimum + 1;
        while (next < walk->minimum) next += span;
        while (next > walk->maximum) next -= span;
    } else if (walk->boundary == BLOOM_BOUNDARY_REFLECT) {
        while (next < walk->minimum || next > walk->maximum) {
            if (next < walk->minimum) next = walk->minimum + (walk->minimum - next);
            if (next > walk->maximum) next = walk->maximum - (next - walk->maximum);
        }
    } else {
        if (next < walk->minimum) next = walk->minimum;
        if (next > walk->maximum) next = walk->maximum;
    }
    walk->position = (int16_t)next;
    return bloom_quantize((uint8_t)next, walk->root, walk->scale);
}

void bloom_weighted_init(bloom_weighted_t *weighted, uint32_t seed,
                         uint8_t root, bloom_scale_t scale)
{
    bloom_prng_seed(&weighted->rng, seed);
    for (uint8_t i = 0; i < 12; i++) weighted->degree_weights[i] = i < 7 ? 10 : 0;
    weighted->degree_weights[0] = 24;
    weighted->degree_weights[2] = 16;
    weighted->degree_weights[4] = 18;
    weighted->octave_weights[0] = 3;
    weighted->octave_weights[1] = 12;
    weighted->octave_weights[2] = 6;
    weighted->octave_weights[3] = 1;
    weighted->repeat_bias = 18;
    weighted->rest_probability = 0;
    weighted->last_degree = 0;
    weighted->root = root;
    weighted->low_note = root;
    weighted->high_note = root > 91 ? 127 : (uint8_t)(root + 36);
    weighted->scale = scale;
}

bool bloom_weighted_tick(bloom_weighted_t *weighted, uint8_t *note)
{
    if (bloom_prng_bounded(&weighted->rng, 100) < weighted->rest_probability) return false;
    uint8_t degree;
    if (bloom_prng_bounded(&weighted->rng, 100) < weighted->repeat_bias)
        degree = weighted->last_degree;
    else
        degree = weighted_pick(&weighted->rng, weighted->degree_weights, 12);
    weighted->last_degree = degree;
    uint8_t octave = weighted_pick(&weighted->rng, weighted->octave_weights, 4);
    *note = bloom_scale_note((int)degree + octave * 7, weighted->root,
                             weighted->scale, weighted->low_note, weighted->high_note);
    return true;
}

void bloom_markov_init(bloom_markov_t *markov, uint32_t seed,
                       uint8_t root, bloom_scale_t scale, uint8_t order)
{
    bloom_prng_seed(&markov->rng, seed);
    for (uint8_t previous = 0; previous < BLOOM_MARKOV_STATES; previous++) {
        for (uint8_t current = 0; current < BLOOM_MARKOV_STATES; current++) {
            int distance = (int)current - previous;
            if (distance < 0) distance = -distance;
            markov->first[previous][current] = (uint8_t)(distance == 0 ? 15 : distance == 1 ? 12 : distance == 2 ? 7 : 2);
            for (uint8_t next = 0; next < BLOOM_MARKOV_STATES; next++) {
                int motion = (int)next - current;
                int prior = (int)current - previous;
                int weight = 3;
                if (next == current) weight = 9;
                else if (motion == -prior) weight = 15;
                else if (motion == 1 || motion == -1) weight = 11;
                markov->second[previous][current][next] = (uint8_t)weight;
            }
        }
    }
    markov->previous = 0;
    markov->current = 0;
    markov->order = order >= 2 ? 2 : 1;
    markov->rest_probability = 0;
    markov->root = root;
    markov->low_note = root;
    markov->high_note = root > 103 ? 127 : (uint8_t)(root + 24);
    markov->scale = scale;
}

bool bloom_markov_tick(bloom_markov_t *markov, uint8_t *note)
{
    const uint8_t *weights = markov->order == 2
        ? markov->second[markov->previous][markov->current]
        : markov->first[markov->current];
    uint8_t next = weighted_pick(&markov->rng, weights, BLOOM_MARKOV_STATES);
    markov->previous = markov->current;
    markov->current = next;
    *note = bloom_scale_note(next, markov->root, markov->scale,
                             markov->low_note, markov->high_note);
    return bloom_prng_bounded(&markov->rng, 100) >= markov->rest_probability;
}

void bloom_arp_init(bloom_arp_t *arp, uint32_t seed, uint8_t root,
                    bloom_scale_t scale)
{
    static const int8_t default_degrees[8] = {0, 2, 4, 7, 0, 0, 0, 0};
    bloom_prng_seed(&arp->rng, seed);
    for (uint8_t i = 0; i < 8; i++) arp->degrees[i] = default_degrees[i];
    arp->count = 4;
    arp->cursor = 0;
    arp->direction = 1;
    arp->octave_probability = 20;
    arp->root = root;
    arp->low_note = root;
    arp->high_note = root > 91 ? 127 : (uint8_t)(root + 36);
    arp->scale = scale;
    arp->mode = BLOOM_ARP_UP;
}

uint8_t bloom_arp_tick(bloom_arp_t *arp)
{
    if (!arp->count || arp->count > 8) arp->count = 1;
    uint8_t index = arp->cursor;
    if (arp->mode == BLOOM_ARP_RANDOM) index = (uint8_t)bloom_prng_bounded(&arp->rng, arp->count);
    int degree = arp->degrees[index];
    if (bloom_prng_bounded(&arp->rng, 100) < arp->octave_probability) degree += 7;
    if (arp->mode == BLOOM_ARP_DOWN) {
        arp->cursor = arp->cursor ? (uint8_t)(arp->cursor - 1u) : (uint8_t)(arp->count - 1u);
    } else if (arp->mode == BLOOM_ARP_ALTERNATING) {
        if (arp->count > 1) {
            if (arp->direction > 0 && arp->cursor + 1u >= arp->count) arp->direction = -1;
            else if (arp->direction < 0 && !arp->cursor) arp->direction = 1;
            arp->cursor = (uint8_t)((int)arp->cursor + arp->direction);
        }
    } else if (arp->mode == BLOOM_ARP_UP) {
        arp->cursor = (uint8_t)((arp->cursor + 1u) % arp->count);
    }
    return bloom_scale_note(degree, arp->root, arp->scale, arp->low_note, arp->high_note);
}

void bloom_harmony_init(bloom_harmony_t *harmony, uint8_t root, bloom_scale_t scale)
{
    static const int8_t degrees[4] = {0, 3, 4, 0};
    static const bloom_chord_quality_t qualities[4] = {
        BLOOM_CHORD_MINOR, BLOOM_CHORD_MAJOR, BLOOM_CHORD_MAJOR, BLOOM_CHORD_MINOR,
    };
    harmony->length = 4;
    harmony->bars_per_chord = 2;
    harmony->root = root;
    harmony->scale = scale;
    for (uint8_t i = 0; i < 16; i++) {
        harmony->chords[i].scale_degree = degrees[i % 4u];
        harmony->chords[i].quality = qualities[i % 4u];
    }
}

uint8_t bloom_harmony_note(const bloom_harmony_t *harmony, uint32_t bar,
                           uint8_t chord_tone, int8_t octave,
                           uint8_t low_note, uint8_t high_note)
{
    static const int8_t chord_semitones[4][4] = {
        {0, 4, 7, 11}, {0, 3, 7, 10}, {0, 3, 6, 9}, {0, 5, 7, 10},
    };
    uint8_t length = harmony->length ? harmony->length : 1;
    uint8_t bars = harmony->bars_per_chord ? harmony->bars_per_chord : 1;
    const bloom_chord_t *chord = &harmony->chords[(bar / bars) % length];
    uint8_t quality = chord->quality <= BLOOM_CHORD_SUSPENDED ? chord->quality : 0;
    int base = bloom_scale_note(chord->scale_degree, harmony->root,
                                harmony->scale, 0, 127);
    int note = base + chord_semitones[quality][chord_tone % 4u] + octave * 12;
    while (note < low_note) note += 12;
    while (note > high_note) note -= 12;
    if (note < low_note) note = low_note;
    if (note > high_note) note = high_note;
    return bloom_quantize((uint8_t)note, harmony->root, harmony->scale);
}
