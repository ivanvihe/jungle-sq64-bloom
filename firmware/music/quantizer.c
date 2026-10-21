#include "quantizer.h"

typedef struct { uint16_t mask; uint8_t count; } scale_def_t;

static const scale_def_t SCALES[BLOOM_SCALE_COUNT] = {
    {0x0ab5, 7}, {0x05ad, 7}, {0x09ad, 7}, {0x0aad, 7},
    {0x06ad, 7}, {0x05ab, 7}, {0x0ad5, 7}, {0x06b5, 7},
    {0x056b, 7}, {0x0295, 5}, {0x04a9, 5}, {0x04e9, 6},
    {0x0555, 6}, {0x0fff, 12},
};

static bloom_scale_t safe_scale(bloom_scale_t scale)
{
    return scale < BLOOM_SCALE_COUNT ? scale : BLOOM_SCALE_CHROMATIC;
}

bool bloom_note_in_scale(uint8_t note, uint8_t root, bloom_scale_t scale)
{
    uint8_t pc = (uint8_t)((note + 12u - (root % 12u)) % 12u);
    return (SCALES[safe_scale(scale)].mask & (1u << pc)) != 0;
}

uint8_t bloom_quantize(uint8_t note, uint8_t root, bloom_scale_t scale)
{
    scale = safe_scale(scale);
    for (uint8_t distance = 0; distance < 12; distance++) {
        if (note >= distance && bloom_note_in_scale((uint8_t)(note - distance), root, scale))
            return (uint8_t)(note - distance);
        if (note + distance <= 127 && bloom_note_in_scale((uint8_t)(note + distance), root, scale))
            return (uint8_t)(note + distance);
    }
    return note;
}

uint8_t bloom_scale_note(int degree, uint8_t root, bloom_scale_t scale,
                         uint8_t low, uint8_t high)
{
    scale = safe_scale(scale);
    if (low > high) { uint8_t t = low; low = high; high = t; }
    int count = SCALES[scale].count;
    int octave = degree / count;
    int index = degree % count;
    if (index < 0) { index += count; octave--; }
    int semitone = 0;
    for (int seen = 0; semitone < 12; semitone++) {
        if (!(SCALES[scale].mask & (1u << semitone))) continue;
        if (seen++ == index) break;
    }
    int note = (int)root + octave * 12 + semitone;
    while (note < low) note += 12;
    while (note > high) note -= 12;
    if (note < low) note = low;
    if (note > high) note = high;
    return bloom_quantize((uint8_t)note, root, scale);
}
