/* SOURCE-ONLY extension proposal. Preserved R4 code below still observes
 * normal callbacks; appended probe temporarily closes only its logical Gate. */
/* Native-call observation with an empty logical admission probe. Immutable
 * return records are independent of Gate admission; native EAs stay opaque.
 * Initialization OPEN records one-time success, not the current Gate door. */
#include "adapter.h"
#include "directory_metadata.h"
#include "worker_family.h"
#include <coreinit/interrupts.h>
#include <coreinit/thread.h>
#include <stddef.h>

enum { COLD = 0, INITIALIZING = 1, OPEN = 2, POISONED = 3 };

static GzJcaGate jwa_gate;
static JwaInvocation jwa_invocation[JWA_RECORDS];
static OSMutex jwa_directory_mutex;
static uint32_t jwa_serial;
static uint32_t jwa_initialization;
static uint32_t jwa_init_thread;

volatile uint32_t _gz_jwa_first[9][8];
volatile uint32_t _gz_jwa_report[64] = {
    0x4a574131, 256, 3, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 15, JWA_RECORDS, GZ_JCA_SLOTS, JWA_MAX_DEPTH,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, JWA_KINDS, 32, JWA_SERIAL_MAX, 1
};

_Static_assert(sizeof(JwaInvocation) == 56, "bounded invocation metadata");
_Static_assert(sizeof(GzJcaGate) == 864, "exact common Gate ABI");
_Static_assert(sizeof(OSMutex) == 44, "actual WUT directory mutex ABI");
_Static_assert(sizeof(JwaIdentity) == 24, "scalar identity read");

static void jwa_barrier(void)
{
    __asm__ volatile("sync" ::: "memory");
}

/* The separate C translation unit receives a complete effective address.
 * This avoids Cemu 2.6's indexed stwcx. base-register clobber while retaining
 * the same native atomic operation. No LTO may merge the helper back here. */
void _gz_jwa_increment_counter(volatile uint32_t *counter);

static void jwa_count(uint32_t index)
{
    if (index < sizeof(_gz_jwa_report) / sizeof(_gz_jwa_report[0]))
        _gz_jwa_increment_counter(&_gz_jwa_report[index]);
}

static void jwa_poison(uint32_t reason)
{
    __sync_fetch_and_or(&_gz_jwa_report[4], reason);
    jwa_initialization = POISONED;
    _gz_jwa_report[3] = POISONED;
    jwa_barrier();
}

static uint32_t jwa_ordinary_context(void)
{
    uint32_t previous = (uint32_t)OSDisableInterrupts();
    uint32_t restore_previous = (uint32_t)OSRestoreInterrupts((BOOL)previous);

    return previous != 0 && restore_previous == 0;
}

static void jwa_first(uint32_t kind, const JwaIdentity *identity,
                      uint32_t original_r3, uint32_t original_r5,
                      uint32_t original_r6)
{
    volatile uint32_t *row = _gz_jwa_first[kind];

    if (!__sync_bool_compare_and_swap(&row[0], 0, 1))
        return;
    row[1] = identity->thread;
    row[2] = identity->incoming_sp;
    row[3] = identity->caller_lr;
    row[4] = original_r3;
    row[5] = original_r5;
    row[6] = original_r6;
    row[7] = identity->serial;
    jwa_barrier();
    row[0] = 2;
}

uint32_t _gz_jwa_init_open(void)
{
    uint32_t thread;

    if (!__sync_bool_compare_and_swap(&jwa_initialization, COLD, INITIALIZING))
        return 0;
    thread = (uint32_t)(uintptr_t)OSGetCurrentThread();
    if (!thread || !jwa_ordinary_context()) {
        jwa_poison(JWA_F_CONTEXT);
        return 0;
    }
    /* Cold private BSS only, once, before any entry may use this mutex. */
    OSInitMutex(&jwa_directory_mutex);
    _gz_jca_init_cold(&jwa_gate);
    jwa_init_thread = thread;
    _gz_jwa_report[59] = thread;
    jwa_barrier();
    jwa_initialization = OPEN;
    _gz_jwa_report[3] = OPEN;
    return 1;
}

uint32_t _gz_jwa_enter(uint32_t kind, uint32_t incoming_sp,
                     uint32_t caller_lr, uint32_t original_r3,
                     uint32_t original_r5, uint32_t original_r6)
{
    uint32_t thread;
    uint32_t plan[3];
    uint32_t reason;
    uint32_t admitted;
    JwaInvocation *record;
    JwaIdentity identity;

    if (*(volatile uint32_t *)&jwa_initialization != OPEN) {
        uint32_t cold = _gz_jwf_cold_jwa_enter(kind, incoming_sp, caller_lr);
        if (cold != JWF_OPEN_RACED) return cold;
    }
    if (_gz_jwf_cold_jwa_conflict(incoming_sp)) return 0;
    jwa_barrier();
    jwa_count(5);
    if (kind >= JWA_KINDS || !incoming_sp || (incoming_sp & 7) ||
        !caller_lr || (caller_lr & 3)) {
        jwa_poison(JWA_F_CONTEXT);
        jwa_count(7);
        return 0;
    }
    thread = (uint32_t)(uintptr_t)OSGetCurrentThread();
    if (!thread || !jwa_ordinary_context()) {
        jwa_count(14);
        jwa_poison(JWA_F_CONTEXT);
        jwa_count(7);
        return 0;
    }

    /* Queries and IRQ restoration are complete BEFORE locking. This region
     * holds only this private mutex and performs bounded private metadata
     * work. No Gate, native callback, allocation, wait or other API is called
     * while held; ordinary mutex contention never discards an invocation. */
    OSLockMutex(&jwa_directory_mutex);
    if (jwa_initialization != OPEN) {
        OSUnlockMutex(&jwa_directory_mutex);
        jwa_count(7);
        return 0;
    }
    reason = _gz_jwa_plan_private(jwa_invocation, thread, incoming_sp,
                                  jwa_serial, plan);
    if (reason) {
        OSUnlockMutex(&jwa_directory_mutex);
        jwa_count(13);
        jwa_poison(reason);
        jwa_count(7);
        return 0;
    }
    record = &jwa_invocation[plan[0]];
    record->control = JWA_RESERVED;
    jwa_serial = plan[2];
    record->serial = plan[2];
    record->thread = thread;
    record->incoming_sp = incoming_sp;
    record->caller_lr = caller_lr;
    record->kind = kind;
    record->depth = plan[1];
    record->admission = UINT32_MAX;
    record->token.active = 0;
    record->token.gate = NULL;
    identity.serial = plan[2];
    identity.thread = thread;
    identity.incoming_sp = incoming_sp;
    identity.caller_lr = caller_lr;
    identity.kind = kind;
    identity.depth = plan[1];
    jwa_barrier();
    record->control = (plan[2] << 3) | JWA_LIVE;
    OSUnlockMutex(&jwa_directory_mutex);

    jwa_first(kind, &identity, original_r3, original_r5, original_r6);
    /* The optional proposal probe may briefly close logical admission.
     * Admission refusal still
     * retains the immutable LR record and forwards the original exactly once. */
    admitted = _gz_jca_enter_shared(&jwa_gate, &record->token);
    record->admission = admitted;
    jwa_barrier();
    if (admitted == GZ_JCA_OK) {
        jwa_count(9);
    } else {
        jwa_count(10);
        jwa_poison(JWA_F_GATE);
    }
    jwa_count(6);
    jwa_count(16 + kind);
    if (thread != jwa_init_thread)
        jwa_count(41 + kind);
    return 1;
}

void _gz_jwa_after(uint32_t incoming_sp, uint32_t output[2])
{
    JwaInvocation *record = NULL;
    JwaIdentity selected = {0};
    uint32_t selected_control = 0;
    uint32_t matches = 0;
    uint32_t thread;
    uint32_t top = 0;
    uint32_t i;

    output[0] = 0;
    output[1] = 0;
    if (_gz_jwf_cold_jwa_after(incoming_sp, output)) return;
    /* LR recovery precedes every OS call. It does not depend on initialization,
     * current-thread getter, Gate admission or the private directory mutex. */
    for (i = 0; i < JWA_RECORDS; ++i) {
        JwaIdentity identity;
        uint32_t control = _gz_jwa_identity_private(&jwa_invocation[i], &identity);

        if (control && identity.incoming_sp == incoming_sp) {
            record = &jwa_invocation[i];
            selected = identity;
            selected_control = control;
            ++matches;
        }
    }
    if (matches != 1 || !record || !selected.caller_lr ||
        !__sync_bool_compare_and_swap(&record->control, selected_control,
                                      (selected.serial << 3) | JWA_RETIRING)) {
        jwa_poison(JWA_F_RETURN);
        return;
    }
    output[0] = selected.caller_lr;
    output[1] = selected.kind != 2 && selected.kind != 6;
    jwa_barrier();

    thread = (uint32_t)(uintptr_t)OSGetCurrentThread();
    if (!thread || thread != selected.thread || !jwa_ordinary_context()) {
        record->control = (selected.serial << 3) | JWA_QUARANTINED;
        jwa_poison(JWA_F_CONTEXT | JWA_F_RETURN);
        return;
    }

    OSLockMutex(&jwa_directory_mutex);
    for (i = 0; i < JWA_RECORDS; ++i) {
        const JwaInvocation *other = &jwa_invocation[i];

        if (other != record && other->control != JWA_FREE &&
            other->thread == selected.thread && other->depth > top)
            top = other->depth;
    }
    OSUnlockMutex(&jwa_directory_mutex);
    if (top >= selected.depth) {
        record->control = (selected.serial << 3) | JWA_QUARANTINED;
        jwa_poison(JWA_F_RETURN);
        return;
    }
    if (record->token.active &&
        _gz_jca_leave_shared(&jwa_gate, &record->token) != GZ_JCA_OK) {
        jwa_count(11);
        record->control = (selected.serial << 3) | JWA_QUARANTINED;
        jwa_poison(JWA_F_CLEANUP);
        return;
    }
    /* Successful cleanup releases the slot under the same publication mutex.
     * A racing lock-free reader must still match the nonreused control serial. */
    OSLockMutex(&jwa_directory_mutex);
    jwa_barrier();
    record->control = JWA_FREE;
    OSUnlockMutex(&jwa_directory_mutex);
    jwa_count(8);
    jwa_count(32 + selected.kind);
    if (thread != jwa_init_thread)
        jwa_count(50 + selected.kind);
}

uint32_t _gz_jwa_world_ready(void)
{
    return 0;
}

/* Same translation unit is required: no second Gate or exported Gate pointer. */
#include "exclusive_probe.inc"
#include "lifetime_bridge.inc"
#include "worker_family.inc"
#include "wct_job_bridge.inc"
#include "lifecycle_writer.inc"
