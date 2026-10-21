#include "scheduler.h"

static bool due(uint32_t now, uint32_t time)
{
    return (int32_t)(now - time) >= 0;
}

static bool before(const bloom_event_t *a, const bloom_event_t *b)
{
    int32_t delta = (int32_t)(a->due_ms - b->due_ms);
    return delta < 0 || (delta == 0 && (int16_t)(a->order - b->order) < 0);
}

void bloom_scheduler_init(bloom_scheduler_t *scheduler)
{
    scheduler->next_order = 0;
    scheduler->dropped = 0;
    scheduler->next_due_ms = 0;
    scheduler->count = 0;
    for (unsigned i = 0; i < BLOOM_EVENT_CAPACITY; i++) scheduler->events[i].used = false;
}

static bool push(bloom_scheduler_t *scheduler, uint32_t at, bloom_event_type_t type,
                 uint8_t track, uint8_t note, uint8_t velocity, uint8_t modulation)
{
    for (unsigned i = 0; i < BLOOM_EVENT_CAPACITY; i++) {
        bloom_event_t *event = &scheduler->events[i];
        if (event->used) continue;
        event->due_ms = at;
        event->order = scheduler->next_order++;
        event->type = (uint8_t)type;
        event->track = track;
        event->note = note;
        event->velocity = velocity;
        event->modulation = modulation;
        event->used = true;
        if (!scheduler->count || (int32_t)(at - scheduler->next_due_ms) < 0)
            scheduler->next_due_ms = at;
        scheduler->count++;
        return true;
    }
    scheduler->dropped++;
    return false;
}

bool bloom_scheduler_note(bloom_scheduler_t *scheduler, uint32_t start_ms,
                          uint32_t step_ms, uint8_t track, uint8_t note,
                          uint8_t velocity, uint8_t modulation,
                          uint8_t gate_percent, uint8_t ratchets)
{
    if (!step_ms || track > 3 || note > 127 || !velocity) return false;
    if (!ratchets) ratchets = 1;
    if (ratchets > 8) ratchets = 8;
    if (gate_percent < 1) gate_percent = 1;
    if (gate_percent > 99) gate_percent = 99;
    if ((unsigned)scheduler->count + (unsigned)ratchets * 2u > BLOOM_EVENT_CAPACITY) {
        scheduler->dropped++;
        return false;
    }
    uint32_t slice = step_ms / ratchets;
    if (!slice) slice = 1;
    uint32_t gate = slice * gate_percent / 100u;
    if (!gate) gate = 1;
    if (gate >= slice && slice > 1) gate = slice - 1;
    for (uint8_t i = 0; i < ratchets; i++) {
        uint32_t on = start_ms + (uint32_t)i * slice;
        if (!push(scheduler, on, BLOOM_EVENT_NOTE_ON, track, note, velocity, modulation)
            || !push(scheduler, on + gate, BLOOM_EVENT_NOTE_OFF, track, note, 0, 0))
            return false;
    }
    return true;
}

unsigned bloom_scheduler_poll(bloom_scheduler_t *scheduler, uint32_t now_ms,
                              bloom_event_fn callback, void *context)
{
    unsigned dispatched = 0;
    if (!scheduler->count || !due(now_ms, scheduler->next_due_ms)) return 0;
    for (;;) {
        bloom_event_t *next = 0;
        for (unsigned i = 0; i < BLOOM_EVENT_CAPACITY; i++) {
            bloom_event_t *candidate = &scheduler->events[i];
            if (!candidate->used || !due(now_ms, candidate->due_ms)) continue;
            if (!next || before(candidate, next)) next = candidate;
        }
        if (!next) break;
        bloom_event_t event = *next;
        next->used = false;
        scheduler->count--;
        dispatched++;
        if (callback) callback(&event, context);
    }
    if (scheduler->count) {
        bloom_event_t *next = 0;
        for (unsigned i = 0; i < BLOOM_EVENT_CAPACITY; i++) {
            bloom_event_t *candidate = &scheduler->events[i];
            if (candidate->used && (!next || before(candidate, next))) next = candidate;
        }
        scheduler->next_due_ms = next->due_ms;
    }
    return dispatched;
}

static void refresh_next(bloom_scheduler_t *scheduler)
{
    bloom_event_t *next = 0;
    for (unsigned i = 0; i < BLOOM_EVENT_CAPACITY; i++) {
        bloom_event_t *candidate = &scheduler->events[i];
        if (candidate->used && (!next || before(candidate, next))) next = candidate;
    }
    if (next) scheduler->next_due_ms = next->due_ms;
}

void bloom_scheduler_cancel_track(bloom_scheduler_t *scheduler, uint8_t track)
{
    for (unsigned i = 0; i < BLOOM_EVENT_CAPACITY; i++) {
        bloom_event_t *event = &scheduler->events[i];
        if (event->used && event->track == track) {
            event->used = false;
            scheduler->count--;
        }
    }
    if (scheduler->count) refresh_next(scheduler);
}

void bloom_scheduler_clear(bloom_scheduler_t *scheduler)
{
    for (unsigned i = 0; i < BLOOM_EVENT_CAPACITY; i++) scheduler->events[i].used = false;
    scheduler->count = 0;
    scheduler->next_due_ms = 0;
}
