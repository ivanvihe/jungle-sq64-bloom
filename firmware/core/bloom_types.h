#ifndef BLOOM_TYPES_H
#define BLOOM_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#define BLOOM_MAX_STEPS 64u
#define BLOOM_MAX_BRANCH_DEPTH 8u

typedef struct {
    uint8_t note;
    uint8_t velocity;
    uint8_t probability;
    uint8_t gate;
    uint8_t modulation;
    uint8_t ratchets;
    uint8_t flags;
} bloom_step_t;

enum {
    BLOOM_STEP_ENABLED = 1u,
    BLOOM_STEP_SLEW = 2u,
    BLOOM_STEP_ACCENT = 4u,
};

#endif
