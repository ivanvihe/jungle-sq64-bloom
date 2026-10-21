#include "euclidean.h"

bool bloom_euclidean_hit(uint8_t step, uint8_t steps, uint8_t pulses, uint8_t rotation)
{
    if (!steps || steps > 64 || !pulses) return false;
    if (pulses > steps) pulses = steps;
    step %= steps;
    uint8_t rotated = (uint8_t)((step + steps - (rotation % steps)) % steps);
    return ((uint16_t)rotated * pulses) % steps < pulses;
}

uint64_t bloom_euclidean_pattern(uint8_t steps, uint8_t pulses, uint8_t rotation)
{
    uint64_t result = 0;
    if (steps > 64) steps = 64;
    for (uint8_t i = 0; i < steps; i++)
        if (bloom_euclidean_hit(i, steps, pulses, rotation)) result |= 1ull << i;
    return result;
}
