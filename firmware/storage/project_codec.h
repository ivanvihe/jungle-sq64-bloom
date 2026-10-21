#ifndef BLOOM_PROJECT_CODEC_H
#define BLOOM_PROJECT_CODEC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../core/bloom_types.h"
#include "../music/quantizer.h"

#define BLOOM_PROJECT_TRACKS 4u
#define BLOOM_PROJECT_VERSION 1u
#define BLOOM_PROJECT_ENCODED_SIZE 1880u

typedef enum {
    BLOOM_ENGINE_FRACTAL,
    BLOOM_ENGINE_TURING,
    BLOOM_ENGINE_EUCLIDEAN,
    BLOOM_ENGINE_RANDOM_WALK,
    BLOOM_ENGINE_MARKOV,
    BLOOM_ENGINE_WEIGHTED,
    BLOOM_ENGINE_ARPEGGIATOR,
    BLOOM_ENGINE_HARMONIC,
    BLOOM_ENGINE_COUNT,
} bloom_engine_t;

typedef struct {
    bloom_engine_t engine;
    uint8_t root;
    bloom_scale_t scale;
    uint8_t length;
    uint8_t clock_division;
    uint8_t clock_multiplier;
    uint8_t mutation;
    uint8_t path;
    uint32_t seed;
    uint32_t turing_bits;
    bloom_step_t steps[BLOOM_MAX_STEPS];
} bloom_project_track_t;

typedef struct {
    uint16_t bpm;
    uint8_t swing;
    uint8_t global_root;
    bloom_scale_t global_scale;
    uint8_t selected_track;
    bloom_project_track_t tracks[BLOOM_PROJECT_TRACKS];
} bloom_project_t;

typedef enum {
    BLOOM_PROJECT_OK,
    BLOOM_PROJECT_TOO_SMALL,
    BLOOM_PROJECT_BAD_MAGIC,
    BLOOM_PROJECT_BAD_VERSION,
    BLOOM_PROJECT_BAD_LENGTH,
    BLOOM_PROJECT_BAD_CHECKSUM,
    BLOOM_PROJECT_BAD_DATA,
} bloom_project_result_t;

void bloom_project_defaults(bloom_project_t *project);
bloom_project_result_t bloom_project_encode(const bloom_project_t *project,
                                            uint8_t *output, size_t capacity);
bloom_project_result_t bloom_project_decode(bloom_project_t *project,
                                            const uint8_t *input, size_t length);
uint32_t bloom_project_crc32(const uint8_t *data, size_t length);

#endif
