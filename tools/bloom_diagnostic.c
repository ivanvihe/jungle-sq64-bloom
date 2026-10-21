#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engines/euclidean.h"
#include "engines/fractal.h"
#include "engines/generative.h"
#include "engines/turing.h"

int main(int argc, char **argv)
{
    unsigned steps = 128;
    if (argc == 3 && !strcmp(argv[1], "--steps")) steps = (unsigned)strtoul(argv[2], 0, 10);
    if (!steps || steps > 1000000u) { fputs("steps must be 1..1000000\n", stderr); return 2; }
    bloom_fractal_t fractal;
    bloom_turing_t turing;
    bloom_random_walk_t walk;
    bloom_weighted_t weighted;
    bloom_markov_t markov;
    bloom_arp_t arp;
    bloom_harmony_t harmony;
    bloom_fractal_init(&fractal, 0x424c4f4fu, 16, 48, BLOOM_SCALE_DORIAN);
    bloom_turing_init(&turing, 0x5455524eu, 13);
    turing.mutation = 8;
    fractal.path = BLOOM_PATH_ALTERNATING;
    bloom_random_walk_init(&walk, 0x57414c4bu, 36, 72);
    walk.scale = BLOOM_SCALE_DORIAN;
    bloom_weighted_init(&weighted, 0x57454947u, 36, BLOOM_SCALE_DORIAN);
    bloom_markov_init(&markov, 0x4d41524bu, 36, BLOOM_SCALE_DORIAN, 2);
    bloom_arp_init(&arp, 0x41525000u, 36, BLOOM_SCALE_DORIAN);
    bloom_harmony_init(&harmony, 36, BLOOM_SCALE_DORIAN);
    puts("tick,track,engine,branch,note,velocity,gate,modulation,ratchets");
    for (unsigned tick = 0; tick < steps; tick++) {
        if (!(tick % 16)) bloom_fractal_select_branch(&fractal, tick / 16);
        bloom_step_t s = bloom_fractal_step(&fractal, tick);
        if (s.flags & BLOOM_STEP_ENABLED)
            printf("%u,A,fractal,%u,%u,%u,%u,%u,%u\n", tick, fractal.branch,
                   s.note, s.velocity, s.gate, s.modulation, s.ratchets);
        uint8_t note, velocity;
        if (bloom_turing_tick(&turing, &note, &velocity))
            printf("%u,B,turing,0,%u,%u,55,%u,1\n", tick, note, velocity,
                   (unsigned)(turing.bits & 255u));
        const char *engine = "walk";
        bool c_hit = true;
        switch ((tick / 16u) % 6u) {
        case 0: note = bloom_random_walk_tick(&walk); break;
        case 1: engine = "weighted"; c_hit = bloom_weighted_tick(&weighted, &note); break;
        case 2: engine = "markov"; c_hit = bloom_markov_tick(&markov, &note); break;
        case 3: engine = "arp"; note = bloom_arp_tick(&arp); c_hit = bloom_euclidean_hit((uint8_t)tick, 16, 7, 0); break;
        case 4: engine = "harmony"; note = bloom_harmony_note(&harmony, tick / 16u,
                                                               (uint8_t)tick, 0, 36, 84);
                c_hit = bloom_euclidean_hit((uint8_t)tick, 16, 5, 3); break;
        default: engine = "euclidean"; note = 48; c_hit = bloom_euclidean_hit((uint8_t)tick, 13, 5, 2); break;
        }
        if (c_hit) printf("%u,C,%s,0,%u,96,50,96,1\n", tick, engine, note);
        if (bloom_euclidean_hit((uint8_t)tick, 16, 5, 1))
            printf("%u,D,euclidean,0,36,110,25,0,1\n", tick);
    }
    return 0;
}
