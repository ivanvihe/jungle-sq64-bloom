#include "turing.h"

static uint32_t length_mask(uint8_t length)
{
    return length == 32 ? 0xffffffffu : ((1u << length) - 1u);
}

void bloom_turing_init(bloom_turing_t *t, uint32_t seed, uint8_t length)
{
    bloom_prng_seed(&t->rng, seed);
    t->length = length < 2 ? 2 : length > 32 ? 32 : length;
    t->bits = bloom_prng_next(&t->rng) & length_mask(t->length);
    if (!t->bits) t->bits = 1;
    t->initial_bits = t->bits;
    t->mutation = 0;
    t->lock_probability = 0;
    t->rest_probability = 0;
    t->root = 36;
    t->low_note = 36;
    t->high_note = 60;
    t->scale = BLOOM_SCALE_NATURAL_MINOR;
    t->frozen = false;
}

void bloom_turing_reseed(bloom_turing_t *t, uint32_t seed)
{
    bloom_prng_seed(&t->rng, seed);
    t->bits = bloom_prng_next(&t->rng) & length_mask(t->length);
    if (!t->bits) t->bits = 1;
    t->initial_bits = t->bits;
}

void bloom_turing_restore(bloom_turing_t *t) { t->bits = t->initial_bits; }

bool bloom_turing_tick(bloom_turing_t *t, uint8_t *note, uint8_t *velocity)
{
    uint32_t mask = length_mask(t->length);
    uint32_t outgoing = t->bits & 1u;
    bool mutate = !t->frozen && bloom_prng_bounded(&t->rng, 100) < t->mutation;
    bool locked = bloom_prng_bounded(&t->rng, 100) < t->lock_probability;
    uint32_t incoming = outgoing ^ (uint32_t)(mutate && !locked);
    t->bits = ((t->bits >> 1) | (incoming << (t->length - 1))) & mask;
    uint32_t value = t->bits ^ (t->bits >> 5) ^ (t->bits >> 11);
    uint8_t span = (uint8_t)(t->high_note - t->low_note + 1u);
    uint8_t raw = (uint8_t)(t->low_note + (value % span));
    *note = bloom_quantize(raw, t->root, t->scale);
    *velocity = (t->bits & 8u) ? 112u : 84u;
    return bloom_prng_bounded(&t->rng, 100) >= t->rest_probability;
}
