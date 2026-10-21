#ifndef BLOOM_EUCLIDEAN_H
#define BLOOM_EUCLIDEAN_H

#include <stdbool.h>
#include <stdint.h>

uint64_t bloom_euclidean_pattern(uint8_t steps, uint8_t pulses, uint8_t rotation);
bool bloom_euclidean_hit(uint8_t step, uint8_t steps, uint8_t pulses, uint8_t rotation);

#endif
