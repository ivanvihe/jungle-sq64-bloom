#ifndef BLOOM_TURING_H
#define BLOOM_TURING_H

#include <stdbool.h>
#include <stdint.h>
#include "../core/prng.h"
#include "../music/quantizer.h"

typedef struct {
    uint32_t bits;
    uint32_t initial_bits;
    uint8_t length;
    uint8_t mutation;
    uint8_t lock_probability;
    uint8_t rest_probability;
    uint8_t root;
    uint8_t low_note;
    uint8_t high_note;
    bloom_scale_t scale;
    bool frozen;
    bloom_prng_t rng;
} bloom_turing_t;

void bloom_turing_init(bloom_turing_t *t, uint32_t seed, uint8_t length);
void bloom_turing_reseed(bloom_turing_t *t, uint32_t seed);
void bloom_turing_restore(bloom_turing_t *t);
bool bloom_turing_tick(bloom_turing_t *t, uint8_t *note, uint8_t *velocity);

#endif
