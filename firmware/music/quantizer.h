#ifndef BLOOM_QUANTIZER_H
#define BLOOM_QUANTIZER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BLOOM_SCALE_MAJOR,
    BLOOM_SCALE_NATURAL_MINOR,
    BLOOM_SCALE_HARMONIC_MINOR,
    BLOOM_SCALE_MELODIC_MINOR,
    BLOOM_SCALE_DORIAN,
    BLOOM_SCALE_PHRYGIAN,
    BLOOM_SCALE_LYDIAN,
    BLOOM_SCALE_MIXOLYDIAN,
    BLOOM_SCALE_LOCRIAN,
    BLOOM_SCALE_MAJOR_PENTATONIC,
    BLOOM_SCALE_MINOR_PENTATONIC,
    BLOOM_SCALE_BLUES,
    BLOOM_SCALE_WHOLE_TONE,
    BLOOM_SCALE_CHROMATIC,
    BLOOM_SCALE_COUNT
} bloom_scale_t;

uint8_t bloom_quantize(uint8_t note, uint8_t root, bloom_scale_t scale);
uint8_t bloom_scale_note(int degree, uint8_t root, bloom_scale_t scale,
                         uint8_t low, uint8_t high);
bool bloom_note_in_scale(uint8_t note, uint8_t root, bloom_scale_t scale);

#endif
