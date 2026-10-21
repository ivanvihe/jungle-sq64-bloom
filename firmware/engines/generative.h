#ifndef BLOOM_GENERATIVE_H
#define BLOOM_GENERATIVE_H

#include <stdbool.h>
#include <stdint.h>
#include "../core/prng.h"
#include "../music/quantizer.h"

typedef enum {
    BLOOM_BOUNDARY_CLAMP,
    BLOOM_BOUNDARY_WRAP,
    BLOOM_BOUNDARY_REFLECT,
} bloom_boundary_t;

typedef struct {
    int16_t position;
    int16_t minimum;
    int16_t maximum;
    int16_t center;
    uint8_t maximum_step;
    uint8_t upward_weight;
    uint8_t center_attraction;
    uint8_t root;
    bloom_scale_t scale;
    bloom_boundary_t boundary;
    bloom_prng_t rng;
} bloom_random_walk_t;

void bloom_random_walk_init(bloom_random_walk_t *walk, uint32_t seed,
                            uint8_t low_note, uint8_t high_note);
uint8_t bloom_random_walk_tick(bloom_random_walk_t *walk);

typedef struct {
    uint8_t degree_weights[12];
    uint8_t octave_weights[4];
    uint8_t repeat_bias;
    uint8_t rest_probability;
    uint8_t last_degree;
    uint8_t root;
    uint8_t low_note;
    uint8_t high_note;
    bloom_scale_t scale;
    bloom_prng_t rng;
} bloom_weighted_t;

void bloom_weighted_init(bloom_weighted_t *weighted, uint32_t seed,
                         uint8_t root, bloom_scale_t scale);
bool bloom_weighted_tick(bloom_weighted_t *weighted, uint8_t *note);

#define BLOOM_MARKOV_STATES 8u
typedef struct {
    uint8_t first[BLOOM_MARKOV_STATES][BLOOM_MARKOV_STATES];
    uint8_t second[BLOOM_MARKOV_STATES][BLOOM_MARKOV_STATES][BLOOM_MARKOV_STATES];
    uint8_t previous;
    uint8_t current;
    uint8_t order;
    uint8_t rest_probability;
    uint8_t root;
    uint8_t low_note;
    uint8_t high_note;
    bloom_scale_t scale;
    bloom_prng_t rng;
} bloom_markov_t;

void bloom_markov_init(bloom_markov_t *markov, uint32_t seed,
                       uint8_t root, bloom_scale_t scale, uint8_t order);
bool bloom_markov_tick(bloom_markov_t *markov, uint8_t *note);

typedef enum {
    BLOOM_ARP_UP,
    BLOOM_ARP_DOWN,
    BLOOM_ARP_ALTERNATING,
    BLOOM_ARP_RANDOM,
} bloom_arp_mode_t;

typedef struct {
    int8_t degrees[8];
    uint8_t count;
    uint8_t cursor;
    int8_t direction;
    uint8_t octave_probability;
    uint8_t root;
    uint8_t low_note;
    uint8_t high_note;
    bloom_scale_t scale;
    bloom_arp_mode_t mode;
    bloom_prng_t rng;
} bloom_arp_t;

void bloom_arp_init(bloom_arp_t *arp, uint32_t seed, uint8_t root,
                    bloom_scale_t scale);
uint8_t bloom_arp_tick(bloom_arp_t *arp);

typedef enum {
    BLOOM_CHORD_MAJOR,
    BLOOM_CHORD_MINOR,
    BLOOM_CHORD_DIMINISHED,
    BLOOM_CHORD_SUSPENDED,
} bloom_chord_quality_t;

typedef struct {
    int8_t scale_degree;
    bloom_chord_quality_t quality;
} bloom_chord_t;

typedef struct {
    bloom_chord_t chords[16];
    uint8_t length;
    uint8_t bars_per_chord;
    uint8_t root;
    bloom_scale_t scale;
} bloom_harmony_t;

void bloom_harmony_init(bloom_harmony_t *harmony, uint8_t root, bloom_scale_t scale);
uint8_t bloom_harmony_note(const bloom_harmony_t *harmony, uint32_t bar,
                           uint8_t chord_tone, int8_t octave,
                           uint8_t low_note, uint8_t high_note);

#endif
