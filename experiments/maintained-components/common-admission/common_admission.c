/* Shared implementation for a future private probe and reviewed adapters.
 * Gate/token identity is private bookkeeping, never a world-read grant. */
#include "common_admission.h"
#include <stddef.h>

enum {
    SLOTS = GZ_JCA_SLOTS,
    OK = GZ_JCA_OK,
    BUSY = GZ_JCA_BUSY,
    BAD = GZ_JCA_BAD,
    ABORTED = GZ_JCA_ABORTED,
    FULL = GZ_JCA_FULL
};

_Static_assert(sizeof(GzJcaGate) == 864 && sizeof(GzJcaShared) == 24 &&
               sizeof(GzJcaExclusive) == 16 && sizeof(GzJcaSlot) == 12,
               "Cafe Gate and gate-bound token ABI");
_Static_assert(offsetof(GzJcaGate, slot) == 96 &&
               offsetof(GzJcaGate, owner) == 92, "unchanged Gate layout");

void _gz_jca_init_cold(GzJcaGate *gate)
{
    OSInitMutex(&gate->mutex);
    OSInitCond(&gate->changed);
}

static uint32_t metadata_depth_one(GzJcaGate *gate, OSThread *thread)
{
    return gate->mutex.owner == thread && gate->mutex.count == 1;
}

uint32_t _gz_jca_enter_shared(GzJcaGate *gate, GzJcaShared *token)
{
    if (token->active)
        return BAD;
    OSThread *thread = OSGetCurrentThread();
    uint32_t i, free_slot = SLOTS;
    GzJcaSlot *slot;
    token->active = 0;
    token->gate = 0;
    if (!thread)
        return BAD;
    OSLockMutex(&gate->mutex);
    if (!metadata_depth_one(gate, thread)) {
        OSUnlockMutex(&gate->mutex);
        return BAD;
    }
    while (gate->closed && !gate->broken) {
        /* Never wait for this same thread to release its exclusive token. */
        if (gate->owner == thread) {
            OSUnlockMutex(&gate->mutex);
            return BUSY;
        }
        if (gate->waiters == UINT32_MAX) {
            OSUnlockMutex(&gate->mutex);
            return FULL;
        }
        ++gate->waiters;
        OSWaitCond(&gate->changed, &gate->mutex);
        --gate->waiters;
        if (!metadata_depth_one(gate, thread)) {
            OSUnlockMutex(&gate->mutex);
            return BAD;
        }
    }
    if (gate->broken) {
        OSUnlockMutex(&gate->mutex);
        return ABORTED;
    }
    for (i = 0; i < SLOTS; ++i) {
        if (gate->slot[i].thread == thread && gate->slot[i].depth)
            break;
        if (free_slot == SLOTS && !gate->slot[i].depth &&
            gate->slot[i].generation != UINT32_MAX)
            free_slot = i;
    }
    if (i == SLOTS)
        i = free_slot;
    if (i == SLOTS || gate->active == UINT32_MAX) {
        OSUnlockMutex(&gate->mutex);
        return FULL;
    }
    slot = &gate->slot[i];
    if (!slot->depth) {
        ++slot->generation;
        slot->thread = thread;
    }
    if (slot->depth == UINT32_MAX) {
        OSUnlockMutex(&gate->mutex);
        return FULL;
    }
    ++slot->depth;
    ++gate->active;
    token->thread = thread;
    token->slot = i;
    token->generation = slot->generation;
    token->depth = slot->depth;
    token->gate = gate;
    token->active = 1;
    OSUnlockMutex(&gate->mutex);
    return OK;
}

uint32_t _gz_jca_leave_shared(GzJcaGate *gate, GzJcaShared *token)
{
    if (token->gate != gate)
        return BAD;
    OSThread *thread = OSGetCurrentThread();
    GzJcaSlot *slot;
    OSLockMutex(&gate->mutex);
    if (!metadata_depth_one(gate, thread)) {
        OSUnlockMutex(&gate->mutex);
        return BAD;
    }
    if (!token->active || token->thread != thread || token->slot >= SLOTS) {
        OSUnlockMutex(&gate->mutex);
        return BAD;
    }
    slot = &gate->slot[token->slot];
    if (slot->thread != thread || slot->generation != token->generation ||
        slot->depth != token->depth || !slot->depth || !gate->active) {
        OSUnlockMutex(&gate->mutex);
        return BAD;
    }
    --slot->depth;
    --gate->active;
    token->active = 0;
    token->gate = 0;
    if (!slot->depth)
        slot->thread = 0;
    OSUnlockMutex(&gate->mutex);
    return OK;
}

uint32_t _gz_jca_exclusive_try(GzJcaGate *gate, GzJcaExclusive *token)
{
    if (token->active)
        return BAD;
    OSThread *thread = OSGetCurrentThread();
    token->active = 0;
    token->gate = 0;
    if (!thread)
        return BAD;
    if (!OSTryLockMutex(&gate->mutex))
        return BUSY;
    if (!metadata_depth_one(gate, thread)) {
        OSUnlockMutex(&gate->mutex);
        return BAD;
    }
    if (gate->broken) {
        OSUnlockMutex(&gate->mutex);
        return ABORTED;
    }
    if (gate->closed || gate->active) {
        OSUnlockMutex(&gate->mutex);
        return BUSY;
    }
    if (gate->epoch == UINT32_MAX) {
        OSUnlockMutex(&gate->mutex);
        return FULL;
    }
    ++gate->epoch;
    gate->closed = 1;
    gate->owner = thread;
    token->thread = thread;
    token->generation = gate->epoch;
    token->gate = gate;
    token->active = 1;
    OSUnlockMutex(&gate->mutex);
    return OK;
}

uint32_t _gz_jca_exclusive_leave(GzJcaGate *gate, GzJcaExclusive *token)
{
    if (token->gate != gate)
        return BAD;
    OSThread *thread = OSGetCurrentThread();
    OSLockMutex(&gate->mutex);
    if (!metadata_depth_one(gate, thread)) {
        OSUnlockMutex(&gate->mutex);
        return BAD;
    }
    if (!token->active || token->thread != thread || gate->owner != thread ||
        !gate->closed || token->generation != gate->epoch) {
        OSUnlockMutex(&gate->mutex);
        return BAD;
    }
    gate->closed = 0;
    gate->owner = 0;
    token->active = 0;
    token->gate = 0;
    OSSignalCond(&gate->changed);
    OSUnlockMutex(&gate->mutex);
    return OK;
}

uint32_t _gz_jca_active_count(GzJcaGate *gate)
{
    uint32_t n;
    OSLockMutex(&gate->mutex);
    n = gate->active;
    OSUnlockMutex(&gate->mutex);
    return n;
}

void _gz_jca_break(GzJcaGate *gate)
{
    OSLockMutex(&gate->mutex);
    gate->broken = 1;
    gate->closed = 0;
    gate->owner = 0;
    OSSignalCond(&gate->changed);
    OSUnlockMutex(&gate->mutex);
}

uint32_t _gz_jca_world_ready(void)
{
    return 0;
}
