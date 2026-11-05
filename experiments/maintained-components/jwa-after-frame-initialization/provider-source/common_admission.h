#ifndef GZ_COMMON_ADMISSION_R2_H
#define GZ_COMMON_ADMISSION_R2_H

#include <stdint.h>
#include <coreinit/condition.h>
#include <coreinit/mutex.h>
#include <coreinit/thread.h>

enum {
    GZ_JCA_SLOTS = 64,
    GZ_JCA_OK = 0,
    GZ_JCA_BUSY = 1,
    GZ_JCA_BAD = 2,
    GZ_JCA_ABORTED = 3,
    GZ_JCA_FULL = 4
};

typedef struct GzJcaGate GzJcaGate;

typedef struct {
    OSThread *thread;
    uint32_t generation;
    uint32_t depth;
} GzJcaSlot;

typedef struct {
    GzJcaGate *gate;
    OSThread *thread;
    uint32_t slot;
    uint32_t generation;
    uint32_t depth;
    uint32_t active;
} GzJcaShared;

typedef struct {
    GzJcaGate *gate;
    OSThread *thread;
    uint32_t generation;
    uint32_t active;
} GzJcaExclusive;

/* Caller-owned metadata; never copy/rewind these initialized OS objects. */
struct GzJcaGate {
    OSMutex mutex;
    OSCondition changed;
    uint32_t closed;
    uint32_t broken;
    uint32_t active;
    uint32_t waiters;
    uint32_t epoch;
    OSThread *owner;
    GzJcaSlot slot[GZ_JCA_SLOTS];
};

/* Initialize once on cold zero storage, before exposing it to any thread.
 * All pointers must be valid owner-held objects; tokens must start inactive.
 * Never reuse Gate storage while an old token/caller/waiter can still exist. */
void _gz_jca_init_cold(GzJcaGate *gate);
uint32_t _gz_jca_enter_shared(GzJcaGate *gate, GzJcaShared *token);
uint32_t _gz_jca_leave_shared(GzJcaGate *gate, GzJcaShared *token);
uint32_t _gz_jca_exclusive_try(GzJcaGate *gate, GzJcaExclusive *token);
uint32_t _gz_jca_exclusive_leave(GzJcaGate *gate, GzJcaExclusive *token);
void _gz_jca_break(GzJcaGate *gate);
uint32_t _gz_jca_active_count(GzJcaGate *gate);
uint32_t _gz_jca_world_ready(void);

#endif
