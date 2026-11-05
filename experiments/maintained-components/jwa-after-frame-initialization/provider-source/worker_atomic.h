#ifndef GZ_WORKER_FAMILY_ATOMIC_H
#define GZ_WORKER_FAMILY_ATOMIC_H
#include <stdint.h>
/* Complete effective addresses only; separate TU and no LTO are mandatory.
 * These native LL/SC loops have no wait/OS call or wall-time guarantee. */
uint32_t _gz_jwf_atomic_load(volatile uint32_t *);
uint32_t _gz_jwf_atomic_inc(volatile uint32_t *);
uint32_t _gz_jwf_atomic_dec(volatile uint32_t *);
uint32_t _gz_jwf_atomic_issue(volatile uint32_t *,uint32_t maximum);
uint32_t _gz_jwf_atomic_cas(volatile uint32_t *,uint32_t,uint32_t);
void _gz_jwf_atomic_or(volatile uint32_t *,uint32_t);
#endif
