#ifndef BLOOM_PRNG_H
#define BLOOM_PRNG_H

#include <stdint.h>

typedef struct { uint32_t state; } bloom_prng_t;

void bloom_prng_seed(bloom_prng_t *rng, uint32_t seed);
uint32_t bloom_prng_next(bloom_prng_t *rng);
uint32_t bloom_prng_bounded(bloom_prng_t *rng, uint32_t bound);
uint32_t bloom_hash4(uint32_t seed, uint32_t a, uint32_t b, uint32_t c);

#endif
