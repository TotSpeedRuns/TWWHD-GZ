#ifndef GZ_JOB_ALLOCATOR_COUPLED_HOLD_H
#define GZ_JOB_ALLOCATOR_COUPLED_HOLD_H

#include <stdint.h>
#include "common_admission.h"

/* Private bridge statuses are outside actual GZ_JCA results0..4. They mean
 * no common-API call occurred; they must never be reported as native success. */
enum {
    JOL_BRIDGE_CONTEXT = 0x100,
    JOL_BRIDGE_STORAGE = 0x101,
    JOL_BRIDGE_NOT_READY = 0x102
};

/* Only the actual fixed GFO returned-frame producer enters this diagnostic.
 * The context provider authenticates the current parent/thread/phase/cookie;
 * the integer cookie by itself issues no native owner or read permission. */
void _gz_jol_coupled_probe(uint32_t cookie);
uint32_t _gz_jol_context_owned(uint32_t cookie);
uint32_t _gz_jol_coupled_halted(void);
/* Private diagnostic bits only:1 retained HG token,2 active/inconsistent job
 * token. A nonzero result is unsafe terminal, never a read/owner capability. */
uint32_t _gz_jol_coupled_outstanding(void);

/* One fixed genuinely mapped private token. The same-TU JWA bridge must
 * require this exact address and the actual existing static jwa_gate.
 * No caller-selected token storage or second Gate is accepted. */
extern GzJcaExclusive _gz_jol_job_token;
extern volatile uint32_t _gz_jol_report[32];

uint32_t _gz_jwa_lifetime_ready(void);
uint32_t _gz_jwa_lifetime_try(GzJcaExclusive *, uint32_t cookie);
uint32_t _gz_jwa_lifetime_check(const GzJcaExclusive *, uint32_t cookie);
uint32_t _gz_jwa_lifetime_leave(GzJcaExclusive *);

#endif
