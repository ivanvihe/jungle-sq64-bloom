#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engines/euclidean.h"
#include "engines/fractal.h"
#include "engines/turing.h"

int main(int argc, char **argv)
{
    unsigned steps = 128;
    if (argc == 3 && !strcmp(argv[1], "--steps")) steps = (unsigned)strtoul(argv[2], 0, 10);
    if (!steps || steps > 1000000u) { fputs("steps must be 1..1000000\n", stderr); return 2; }
    bloom_fractal_t fractal;
    bloom_turing_t turing;
    bloom_fractal_init(&fractal, 0x424c4f4fu, 16, 48, BLOOM_SCALE_DORIAN);
    bloom_turing_init(&turing, 0x5455524eu, 13);
    turing.mutation = 8;
    fractal.path = BLOOM_PATH_ALTERNATING;
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
        if (bloom_euclidean_hit((uint8_t)tick, 16, 5, 1))
            printf("%u,D,euclidean,0,36,110,25,0,1\n", tick);
    }
    return 0;
}
