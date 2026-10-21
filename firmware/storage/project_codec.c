#include "project_codec.h"

#define HEADER_SIZE 16u
#define TRACK_HEADER_SIZE 16u
#define STEP_SIZE 7u

static void put16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

static void put32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

static uint16_t get16(const uint8_t *p)
{
    return (uint16_t)(p[0] | (uint16_t)p[1] << 8);
}

static uint32_t get32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8
        | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

uint32_t bloom_project_crc32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xffffffffu;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ (0xedb88320u & (uint32_t)-(int32_t)(crc & 1u));
    }
    return ~crc;
}

void bloom_project_defaults(bloom_project_t *project)
{
    project->bpm = 120;
    project->swing = 50;
    project->global_root = 48;
    project->global_scale = BLOOM_SCALE_DORIAN;
    project->selected_track = 0;
    for (uint8_t track = 0; track < BLOOM_PROJECT_TRACKS; track++) {
        bloom_project_track_t *t = &project->tracks[track];
        t->engine = track == 0 ? BLOOM_ENGINE_FRACTAL
            : track == 1 ? BLOOM_ENGINE_TURING : BLOOM_ENGINE_EUCLIDEAN;
        t->root = (uint8_t)(48 - track * 4u);
        t->scale = project->global_scale;
        t->length = 16;
        t->clock_division = 1;
        t->clock_multiplier = 1;
        t->mutation = track < 2 ? 8 : 0;
        t->path = 0;
        t->seed = 0x4a534236u + track * 0x9e3779b9u;
        t->turing_bits = 1;
        for (uint8_t step = 0; step < BLOOM_MAX_STEPS; step++) {
            bloom_step_t value = {0};
            if (step < t->length) {
                value.note = t->root;
                value.velocity = 100;
                value.probability = 100;
                value.gate = 50;
                value.ratchets = 1;
                value.flags = BLOOM_STEP_ENABLED;
            }
            t->steps[step] = value;
        }
    }
}

static bool valid(const bloom_project_t *project)
{
    if (project->bpm < 20 || project->bpm > 300 || project->swing < 50
        || project->swing > 75 || project->global_root > 127
        || project->global_scale >= BLOOM_SCALE_COUNT
        || project->selected_track >= BLOOM_PROJECT_TRACKS) return false;
    for (uint8_t track = 0; track < BLOOM_PROJECT_TRACKS; track++) {
        const bloom_project_track_t *t = &project->tracks[track];
        if (t->engine >= BLOOM_ENGINE_COUNT || t->root > 127
            || t->scale >= BLOOM_SCALE_COUNT || !t->length || t->length > 64
            || !t->clock_division || t->clock_division > 64
            || !t->clock_multiplier || t->clock_multiplier > 8
            || t->mutation > 100 || t->path > 5) return false;
        for (uint8_t step = 0; step < BLOOM_MAX_STEPS; step++) {
            const bloom_step_t *s = &t->steps[step];
            if (s->note > 127 || s->velocity > 127 || s->probability > 100
                || s->gate > 100 || s->ratchets > 8
                || (s->flags & ~(BLOOM_STEP_ENABLED | BLOOM_STEP_SLEW | BLOOM_STEP_ACCENT)))
                return false;
        }
    }
    return true;
}

static __attribute__((noinline)) const uint8_t *decode_track(bloom_project_track_t *t,
                                                              const uint8_t *p)
{
    t->engine = (bloom_engine_t)p[0]; t->root = p[1]; t->scale = (bloom_scale_t)p[2];
    t->length = p[3]; t->clock_division = p[4]; t->clock_multiplier = p[5];
    t->mutation = p[6]; t->path = p[7];
    t->seed = get32(p + 8); t->turing_bits = get32(p + 12);
    p += TRACK_HEADER_SIZE;
    for (uint8_t step = 0; step < BLOOM_MAX_STEPS; step++) {
        bloom_step_t *s = &t->steps[step];
        s->note = p[0]; s->velocity = p[1]; s->probability = p[2];
        s->gate = p[3]; s->modulation = p[4]; s->ratchets = p[5]; s->flags = p[6];
        p += STEP_SIZE;
    }
    return p;
}

static __attribute__((noinline)) uint8_t *encode_track(const bloom_project_track_t *t,
                                                        uint8_t *p)
{
    p[0] = (uint8_t)t->engine; p[1] = t->root; p[2] = (uint8_t)t->scale;
    p[3] = t->length; p[4] = t->clock_division; p[5] = t->clock_multiplier;
    p[6] = t->mutation; p[7] = t->path;
    put32(p + 8, t->seed); put32(p + 12, t->turing_bits);
    p += TRACK_HEADER_SIZE;
    for (uint8_t step = 0; step < BLOOM_MAX_STEPS; step++) {
        const bloom_step_t *s = &t->steps[step];
        p[0] = s->note; p[1] = s->velocity; p[2] = s->probability;
        p[3] = s->gate; p[4] = s->modulation; p[5] = s->ratchets; p[6] = s->flags;
        p += STEP_SIZE;
    }
    return p;
}

bloom_project_result_t bloom_project_encode(const bloom_project_t *project,
                                            uint8_t *output, size_t capacity)
{
    if (capacity < BLOOM_PROJECT_ENCODED_SIZE) return BLOOM_PROJECT_TOO_SMALL;
    if (!valid(project)) return BLOOM_PROJECT_BAD_DATA;
    for (size_t i = 0; i < BLOOM_PROJECT_ENCODED_SIZE; i++) output[i] = 0;
    output[0] = 'J'; output[1] = 'B'; output[2] = 'L'; output[3] = 'M';
    put16(output + 4, BLOOM_PROJECT_VERSION);
    put16(output + 6, HEADER_SIZE);
    put32(output + 8, BLOOM_PROJECT_ENCODED_SIZE);
    uint8_t *p = output + HEADER_SIZE;
    put16(p, project->bpm);
    p[2] = project->swing;
    p[3] = project->global_root;
    p[4] = (uint8_t)project->global_scale;
    p[5] = project->selected_track;
    p += 8;
    for (uint8_t track = 0; track < BLOOM_PROJECT_TRACKS; track++) {
        p = encode_track(&project->tracks[track], p);
    }
    put32(output + 12, bloom_project_crc32(output + HEADER_SIZE,
                                           BLOOM_PROJECT_ENCODED_SIZE - HEADER_SIZE));
    return BLOOM_PROJECT_OK;
}

bloom_project_result_t bloom_project_decode(bloom_project_t *project,
                                            const uint8_t *input, size_t length)
{
    if (length < HEADER_SIZE) return BLOOM_PROJECT_TOO_SMALL;
    if (input[0] != 'J' || input[1] != 'B' || input[2] != 'L' || input[3] != 'M')
        return BLOOM_PROJECT_BAD_MAGIC;
    if (get16(input + 4) != BLOOM_PROJECT_VERSION) return BLOOM_PROJECT_BAD_VERSION;
    if (get16(input + 6) != HEADER_SIZE || get32(input + 8) != BLOOM_PROJECT_ENCODED_SIZE
        || length < BLOOM_PROJECT_ENCODED_SIZE) return BLOOM_PROJECT_BAD_LENGTH;
    if (get32(input + 12) != bloom_project_crc32(input + HEADER_SIZE,
                                                  BLOOM_PROJECT_ENCODED_SIZE - HEADER_SIZE))
        return BLOOM_PROJECT_BAD_CHECKSUM;
    const uint8_t *p = input + HEADER_SIZE;
    project->bpm = get16(p);
    project->swing = p[2]; project->global_root = p[3];
    project->global_scale = (bloom_scale_t)p[4]; project->selected_track = p[5];
    p += 8;
    for (uint8_t track = 0; track < BLOOM_PROJECT_TRACKS; track++) {
        p = decode_track(&project->tracks[track], p);
    }
    return valid(project) ? BLOOM_PROJECT_OK : BLOOM_PROJECT_BAD_DATA;
}
