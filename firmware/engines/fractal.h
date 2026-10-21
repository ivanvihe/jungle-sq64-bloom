#ifndef BLOOM_FRACTAL_H
#define BLOOM_FRACTAL_H

#include <stdbool.h>
#include <stdint.h>
#include "../core/bloom_types.h"
#include "../music/quantizer.h"

typedef enum {
    BLOOM_PATH_SEQUENTIAL,
    BLOOM_PATH_REVERSE,
    BLOOM_PATH_ALTERNATING,
    BLOOM_PATH_RANDOM,
    BLOOM_PATH_WEIGHTED,
    BLOOM_PATH_EVOLVING,
} bloom_path_t;

typedef struct {
    bloom_step_t trunk[BLOOM_MAX_STEPS];
    bloom_step_t original[BLOOM_MAX_STEPS];
    bloom_step_t undo[BLOOM_MAX_STEPS];
    uint32_t seed;
    uint32_t previous_seed;
    uint32_t bar;
    uint8_t length;
    uint8_t depth;
    uint8_t branch;
    uint8_t return_probability;
    uint8_t mutation_probability;
    uint8_t mutation_intensity;
    uint8_t pitch_range;
    bloom_path_t path;
    bloom_scale_t scale;
    uint8_t root;
    bool frozen;
    bool undo_valid;
} bloom_fractal_t;

void bloom_fractal_init(bloom_fractal_t *f, uint32_t seed, uint8_t length,
                        uint8_t root, bloom_scale_t scale);
uint8_t bloom_fractal_select_branch(bloom_fractal_t *f, uint32_t bar);
bloom_step_t bloom_fractal_step(const bloom_fractal_t *f, uint32_t absolute_step);
void bloom_fractal_mutate(bloom_fractal_t *f, uint32_t epoch, bool persistent);
bool bloom_fractal_undo(bloom_fractal_t *f);
void bloom_fractal_restore(bloom_fractal_t *f);
void bloom_fractal_reseed(bloom_fractal_t *f, uint32_t seed, bool notes, bool rhythm);

#endif
