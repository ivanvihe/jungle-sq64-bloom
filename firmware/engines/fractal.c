#include "fractal.h"
#include "../core/prng.h"

static void copy_steps(bloom_step_t *to, const bloom_step_t *from, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++) to[i] = from[i];
}

void bloom_fractal_init(bloom_fractal_t *f, uint32_t seed, uint8_t length,
                        uint8_t root, bloom_scale_t scale)
{
    f->seed = seed;
    f->previous_seed = seed;
    f->bar = 0;
    f->length = length < 1 ? 1 : length > 64 ? 64 : length;
    f->depth = 4;
    f->branch = 0;
    f->return_probability = 20;
    f->mutation_probability = 12;
    f->mutation_intensity = 20;
    f->pitch_range = 3;
    f->path = BLOOM_PATH_SEQUENTIAL;
    f->scale = scale;
    f->root = root;
    f->frozen = false;
    f->undo_valid = false;
    for (uint8_t i = 0; i < BLOOM_MAX_STEPS; i++) {
        bloom_step_t empty = {0};
        f->trunk[i] = f->original[i] = f->undo[i] = empty;
    }
    for (uint8_t i = 0; i < f->length; i++) {
        uint32_t h = bloom_hash4(seed, i, 0, 0);
        bloom_step_t s;
        s.note = bloom_scale_note((int)(h % 9u), root, scale, root, (uint8_t)(root + 24));
        s.velocity = (uint8_t)(78u + h % 36u);
        s.probability = 100;
        s.gate = (uint8_t)(45u + (h >> 8) % 36u);
        s.modulation = (uint8_t)(h >> 24);
        s.ratchets = 1;
        s.flags = BLOOM_STEP_ENABLED;
        f->trunk[i] = f->original[i] = s;
    }
}

uint8_t bloom_fractal_select_branch(bloom_fractal_t *f, uint32_t bar)
{
    uint8_t count = (uint8_t)(f->depth + 1u);
    if (count > BLOOM_MAX_BRANCH_DEPTH + 1u) count = BLOOM_MAX_BRANCH_DEPTH + 1u;
    uint32_t h = bloom_hash4(f->seed, bar, f->path, 0x50415448u);
    uint8_t branch;
    switch (f->path) {
    case BLOOM_PATH_REVERSE: branch = (uint8_t)(f->depth - (bar % count)); break;
    case BLOOM_PATH_ALTERNATING: {
        uint32_t cycle = count > 1 ? (uint32_t)(count * 2u - 2u) : 1u;
        uint32_t p = bar % cycle;
        branch = (uint8_t)(p < count ? p : cycle - p);
        break;
    }
    case BLOOM_PATH_RANDOM: branch = (uint8_t)(h % count); break;
    case BLOOM_PATH_WEIGHTED: branch = (uint8_t)(((h & 0xffu) * (h & 0xffu)) / (256u * 256u / count)); if (branch >= count) branch = (uint8_t)(count - 1); break;
    case BLOOM_PATH_EVOLVING: branch = (uint8_t)((bar / 2u + (h % 3u)) % count); break;
    default: branch = (uint8_t)(bar % count); break;
    }
    if ((h >> 24) % 100u < f->return_probability) branch = 0;
    f->bar = bar;
    f->branch = branch;
    return branch;
}

bloom_step_t bloom_fractal_step(const bloom_fractal_t *f, uint32_t absolute_step)
{
    uint8_t index = (uint8_t)(absolute_step % f->length);
    bloom_step_t s = f->trunk[index];
    for (uint8_t level = 1; level <= f->branch; level++) {
        uint32_t h = bloom_hash4(f->seed, level, index, f->bar);
        if (h % 100u < (uint32_t)(10u + level * 4u)) {
            int delta = (int)((h >> 8) % 3u) - 1;
            int degree = (int)s.note - (int)f->root + delta * (1 + level / 3);
            s.note = bloom_scale_note(degree, f->root, f->scale, 0, 127);
        }
        if ((h >> 12) % 100u < 8u + level * 2u)
            s.flags ^= BLOOM_STEP_ENABLED;
        if ((h >> 20) % 100u < 12u) s.ratchets = (uint8_t)(2u + h % 2u);
        int gate_delta = (int)((h >> 24) % 11u) - 5;
        int gate = (int)s.gate + gate_delta;
        s.gate = (uint8_t)(gate < 5 ? 5 : gate > 95 ? 95 : gate);
    }
    return s;
}

void bloom_fractal_mutate(bloom_fractal_t *f, uint32_t epoch, bool persistent)
{
    if (f->frozen) return;
    if (persistent) { copy_steps(f->undo, f->trunk, f->length); f->undo_valid = true; }
    for (uint8_t i = 0; i < f->length; i++) {
        uint32_t h = bloom_hash4(f->seed, epoch, i, 0x4d555441u);
        if (h % 100u >= f->mutation_probability) continue;
        int range = 1 + (int)((uint16_t)f->pitch_range * f->mutation_intensity / 100u);
        int delta = (int)((h >> 8) % (uint32_t)(range * 2 + 1)) - range;
        bloom_step_t s = f->trunk[i];
        s.note = bloom_scale_note((int)s.note - f->root + delta, f->root, f->scale, 0, 127);
        int gd = (int)((h >> 16) % 9u) - 4;
        int gate = (int)s.gate + gd * f->mutation_intensity / 20;
        s.gate = (uint8_t)(gate < 5 ? 5 : gate > 95 ? 95 : gate);
        if (persistent) f->trunk[i] = s;
    }
}

bool bloom_fractal_undo(bloom_fractal_t *f)
{
    if (!f->undo_valid) return false;
    copy_steps(f->trunk, f->undo, f->length);
    f->undo_valid = false;
    return true;
}

void bloom_fractal_restore(bloom_fractal_t *f)
{
    copy_steps(f->trunk, f->original, f->length);
    f->undo_valid = false;
}

void bloom_fractal_reseed(bloom_fractal_t *f, uint32_t seed, bool notes, bool rhythm)
{
    bloom_fractal_t fresh;
    bloom_fractal_init(&fresh, seed, f->length, f->root, f->scale);
    f->previous_seed = f->seed;
    f->seed = seed;
    for (uint8_t i = 0; i < f->length; i++) {
        if (notes) f->trunk[i].note = fresh.trunk[i].note;
        if (rhythm) {
            f->trunk[i].gate = fresh.trunk[i].gate;
            f->trunk[i].probability = fresh.trunk[i].probability;
            f->trunk[i].flags = fresh.trunk[i].flags;
        }
    }
}
