#ifndef GZ_GENERIC_CPU_WORKER_FAMILY_H
#define GZ_GENERIC_CPU_WORKER_FAMILY_H
#include <stdint.h>
#include "common_admission.h"
enum { JWF_FAMILIES=3, JWF_RECORDS=128, JWF_TRACKED_KINDS=12, JWF_OPEN_RACED=2 };
enum { JWF_ZERO_OK=0, JWF_ZERO_BUSY=0x200, JWF_ZERO_BROKEN=0x201 };
/* Real same-token/context check first, then atomic new3 whole-invocation and old9 bootstrap counts.
 * Nonzero refuses a reader; zero alone is no lifetime or body-read grant.
 * New OPEN calls waiting outside the closed Gate can conservatively be BUSY. */
uint32_t _gz_jwa_workers_zero(const GzJcaExclusive *,uint32_t cookie);
uint32_t _gz_jwf_enter(uint32_t family,uint32_t incoming_sp,uint32_t caller_lr);
void _gz_jwf_after(uint32_t incoming_sp,uint32_t output[2]);
/* Bootstrap-only routes for the unchanged nine native wrapper shims. */
uint32_t _gz_jwf_cold_jwa_enter(uint32_t kind,uint32_t incoming_sp,uint32_t caller_lr);
uint32_t _gz_jwf_cold_jwa_after(uint32_t incoming_sp,uint32_t output[2]);
uint32_t _gz_jwf_cold_jwa_conflict(uint32_t incoming_sp);
extern volatile uint32_t _gz_jwf_report[64];

/* Exact private identity-key predicate; no supplied caller/Gate outcome. */
static inline uint32_t gz_jwf_cold_identity_matches(uint32_t kind,uint32_t stored_sp,uint32_t incoming_sp)
{ return kind>=JWF_FAMILIES && stored_sp==incoming_sp; }

/* Pure scalar boundaries, not OS/Gate/context outcomes. */
static inline uint32_t gz_jwf_active_next(uint32_t value,uint32_t entering)
{
    if (entering) return value==UINT32_MAX ? 0 : value+1;
    return value ? value-1 : UINT32_MAX;
}
static inline uint32_t gz_jwf_control(uint32_t serial,uint32_t phase)
{
    return serial && serial<=0x1fffffffu && phase>=1 && phase<=4 ?
            (serial<<3)|phase : 0;
}
#endif
