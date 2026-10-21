#ifndef BLOOM_SCHEDULER_H
#define BLOOM_SCHEDULER_H

#include <stdbool.h>
#include <stdint.h>

#define BLOOM_EVENT_CAPACITY 72u

typedef enum {
    BLOOM_EVENT_NOTE_ON,
    BLOOM_EVENT_NOTE_OFF,
} bloom_event_type_t;

typedef struct {
    uint32_t due_ms;
    uint16_t order;
    uint8_t type;
    uint8_t track;
    uint8_t note;
    uint8_t velocity;
    uint8_t modulation;
    bool used;
} bloom_event_t;

typedef struct {
    bloom_event_t events[BLOOM_EVENT_CAPACITY];
    uint16_t next_order;
    uint16_t dropped;
    uint32_t next_due_ms;
    uint8_t count;
} bloom_scheduler_t;

typedef void (*bloom_event_fn)(const bloom_event_t *event, void *context);

void bloom_scheduler_init(bloom_scheduler_t *scheduler);
bool bloom_scheduler_note(bloom_scheduler_t *scheduler, uint32_t start_ms,
                          uint32_t step_ms, uint8_t track, uint8_t note,
                          uint8_t velocity, uint8_t modulation,
                          uint8_t gate_percent, uint8_t ratchets);
unsigned bloom_scheduler_poll(bloom_scheduler_t *scheduler, uint32_t now_ms,
                              bloom_event_fn callback, void *context);
void bloom_scheduler_cancel_track(bloom_scheduler_t *scheduler, uint8_t track);
void bloom_scheduler_clear(bloom_scheduler_t *scheduler);

#endif
