#include "directory_metadata.h"

uint32_t _gz_jwa_plan_private(const JwaInvocation records[JWA_RECORDS],
                              uint32_t thread, uint32_t incoming_sp,
                              uint32_t serial, uint32_t output[3])
{
    uint32_t i;
    uint32_t free_index = JWA_RECORDS;
    uint32_t depth = 0;

    if (!thread || !incoming_sp || (incoming_sp & 7))
        return JWA_F_CONTEXT;
    for (i = 0; i < JWA_RECORDS; ++i) {
        const JwaInvocation *record = &records[i];
        uint32_t control = record->control;
        uint32_t phase = control & 7;

        if (control == JWA_FREE) {
            if (free_index == JWA_RECORDS)
                free_index = i;
            continue;
        }
        /* RESERVED cannot escape the same mutex's publication interval. */
        if ((phase != JWA_LIVE && phase != JWA_RETIRING &&
             phase != JWA_QUARANTINED) || !(control >> 3) ||
            record->serial != (control >> 3))
            return JWA_F_STORAGE;
        if (record->incoming_sp == incoming_sp)
            return JWA_F_REENTRANCY;
        if (record->thread == thread && record->depth > depth)
            depth = record->depth;
    }
    /* This is a mutex-stable complete scan, not a moving FREE-slot sample. */
    if (free_index == JWA_RECORDS || depth >= JWA_MAX_DEPTH ||
        serial >= JWA_SERIAL_MAX)
        return JWA_F_STORAGE | JWA_F_EXHAUSTED;
    output[0] = free_index;
    output[1] = depth + 1;
    output[2] = serial + 1;
    return 0;
}

uint32_t _gz_jwa_identity_private(const JwaInvocation *record,
                                  JwaIdentity *output)
{
    uint32_t control = record->control;

    if ((control & 7) != JWA_LIVE || !(control >> 3))
        return 0;
    __asm__ volatile("sync" ::: "memory");
    output->serial = record->serial;
    output->thread = record->thread;
    output->incoming_sp = record->incoming_sp;
    output->caller_lr = record->caller_lr;
    output->kind = record->kind;
    output->depth = record->depth;
    __asm__ volatile("sync" ::: "memory");
    if (record->control != control || output->serial != (control >> 3))
        return 0;
    return control;
}
