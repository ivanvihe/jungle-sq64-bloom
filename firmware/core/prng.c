#include "prng.h"

static __attribute__((noinline)) uint32_t mix32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    return x ^ (x >> 16);
}

void bloom_prng_seed(bloom_prng_t *rng, uint32_t seed)
{
    rng->state = seed ? seed : 0x6d2b79f5u;
}

uint32_t bloom_prng_next(bloom_prng_t *rng)
{
    uint32_t x = rng->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng->state = x;
    return x;
}

uint32_t bloom_prng_bounded(bloom_prng_t *rng, uint32_t bound)
{
    if (!bound) return 0;
    return (uint32_t)(((uint64_t)bloom_prng_next(rng) * bound) >> 32);
}

uint32_t bloom_hash4(uint32_t seed, uint32_t a, uint32_t b, uint32_t c)
{
    uint32_t x = seed ^ 0x9e3779b9u;
    x = mix32(x ^ mix32(a + 0x85ebca6bu));
    x = mix32(x ^ mix32(b + 0xc2b2ae35u));
    return mix32(x ^ mix32(c + 0x27d4eb2fu));
}
