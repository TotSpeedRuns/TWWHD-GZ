#ifndef GZ_JOB_WRITER_DIRECTORY_METADATA_R3_H
#define GZ_JOB_WRITER_DIRECTORY_METADATA_R3_H

#include <stdint.h>
#include "common_admission.h"

enum { JWA_RECORDS = 128, JWA_MAX_DEPTH = 8, JWA_KINDS = 9 };
enum {
    JWA_FREE = 0, JWA_RESERVED = 1, JWA_LIVE = 2,
    JWA_RETIRING = 3, JWA_QUARANTINED = 4
};
enum {
    JWA_F_CONTEXT = 1, JWA_F_STORAGE = 2, JWA_F_GATE = 4,
    JWA_F_RETURN = 8, JWA_F_CLEANUP = 16,
    JWA_F_REENTRANCY = 32, JWA_F_EXHAUSTED = 64
};
#define JWA_SERIAL_MAX UINT32_C(0x1fffffff)

typedef struct {
    /* One publication word binds the nonwrapping serial and phase. */
    volatile uint32_t control;
    volatile uint32_t serial;
    volatile uint32_t thread;
    volatile uint32_t incoming_sp;
    volatile uint32_t caller_lr;
    volatile uint32_t kind;
    volatile uint32_t depth;
    volatile uint32_t admission;
    GzJcaShared token;
} JwaInvocation;

typedef struct {
    uint32_t serial;
    uint32_t thread;
    uint32_t incoming_sp;
    uint32_t caller_lr;
    uint32_t kind;
    uint32_t depth;
} JwaIdentity;

/* Detached metadata only. Caller holds its actual private directory mutex for
 * the complete plan/reserve/publish interval. No API or pointer following. */
uint32_t _gz_jwa_plan_private(const JwaInvocation records[JWA_RECORDS],
                              uint32_t thread, uint32_t incoming_sp,
                              uint32_t serial, uint32_t output[3]);

/* Lock-free immutable identity read. Returned control must be matched exactly
 * by the caller's LIVE->RETIRING CAS. The serial/phase recheck rejects reuse. */
uint32_t _gz_jwa_identity_private(const JwaInvocation *record,
                                  JwaIdentity *output);

#endif
