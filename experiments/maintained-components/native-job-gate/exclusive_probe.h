/* SPDX-License-Identifier: MIT */
#ifndef GZ_NATIVE_JOB_GATE_EXCLUSIVE_PROBE_H
#define GZ_NATIVE_JOB_GATE_EXCLUSIVE_PROBE_H

#include <stdint.h>

enum {
    GZ_JXP_MAGIC = 0x4a585031u,
    GZ_JXP_VERSION = 1,
    GZ_JXP_WORDS = 32,
    GZ_JXP_BYTES = 128
};

enum GzJxpState {
    GZ_JXP_STATE_COLD = 0,
    GZ_JXP_STATE_ACTIVE = 1,
    GZ_JXP_STATE_HALTED = 2
};

enum GzJxpReportWord {
    GZ_JXP_MAGIC_WORD = 0,
    GZ_JXP_SIZE_WORD = 1,
    GZ_JXP_VERSION_WORD = 2,
    GZ_JXP_SEQUENCE = 3,
    GZ_JXP_STATE = 4,
    GZ_JXP_CALLS = 5,
    GZ_JXP_TRIES = 6,
    GZ_JXP_ACQUIRED = 7,
    GZ_JXP_BUSY = 8,
    GZ_JXP_TRY_ERROR = 9,
    GZ_JXP_RELEASED = 10,
    GZ_JXP_RELEASE_ERROR = 11,
    GZ_JXP_NOT_READY = 12,
    GZ_JXP_CONTEXT_SKIP = 13,
    GZ_JXP_LAST_SERIAL = 14,
    GZ_JXP_ACTUAL_THREAD = 15,
    GZ_JXP_INITIALIZATION_THREAD = 16,
    GZ_JXP_IRQ_PREVIOUS = 17,
    GZ_JXP_IRQ_RESTORE_PREVIOUS = 18,
    GZ_JXP_RAW_TRY = 19,
    GZ_JXP_RAW_LEAVE = 20,
    GZ_JXP_TOKEN_ACTIVE = 21,
    GZ_JXP_TOKEN_GENERATION = 22,
    GZ_JXP_OPAQUE_GATE_ADDRESS = 23,
    GZ_JXP_WORLD_READY = 24,
    GZ_JXP_MISSING_MASK = 25,
    GZ_JXP_READ_READY = 26,
    GZ_JXP_SEQUENCE_EXHAUSTED = 27,
    GZ_JXP_COUNTER_SATURATED = 28,
    GZ_JXP_MODE = 29,
    GZ_JXP_RESERVED_30 = 30,
    GZ_JXP_RESERVED_31 = 31
};

/* Call only after a genuinely returned frame and one-time real Gate init.
 * The expected thread and serial must come from that actual frame callback.
 * TRY is nonwaiting. Successful TRY immediately uses genuine metadata LEAVE,
 * which may block. No game read, capture, restore or world owner is granted. */
void _gz_jwa_exclusive_probe(uint32_t serial, uint32_t expected_thread);

/* Private observations only. Read an unchanged even sequence before/after a
 * copy. UINT32_MAX in a raw result word means no call, not API success.
 * HALTED is terminal and immutable; an outstanding token must not be erased. */
extern volatile uint32_t _gz_jwa_exclusive_probe_report[GZ_JXP_WORDS];

#endif
