#include "worker_atomic.h"
#define ATOMIC_FN __attribute__((noinline,noclone))
ATOMIC_FN uint32_t _gz_jwf_atomic_load(volatile uint32_t *p)
{ return __sync_fetch_and_add(p,0); }
ATOMIC_FN uint32_t _gz_jwf_atomic_cas(volatile uint32_t *p,uint32_t old,uint32_t next)
{ return __sync_bool_compare_and_swap(p,old,next); }
ATOMIC_FN uint32_t _gz_jwf_atomic_inc(volatile uint32_t *p)
{
    uint32_t old;
    for (;;) {
        old=__sync_fetch_and_add(p,0);
        if (old==UINT32_MAX) return 0;
        if (__sync_bool_compare_and_swap(p,old,old+1)) return 1;
    }
}
ATOMIC_FN uint32_t _gz_jwf_atomic_dec(volatile uint32_t *p)
{
    uint32_t old;
    for (;;) {
        old=__sync_fetch_and_add(p,0);
        if (!old) return 0;
        if (__sync_bool_compare_and_swap(p,old,old-1)) return 1;
    }
}
ATOMIC_FN uint32_t _gz_jwf_atomic_issue(volatile uint32_t *p,uint32_t maximum)
{
    uint32_t old;
    for (;;) {
        old=__sync_fetch_and_add(p,0);
        if (old>=maximum) return 0;
        if (__sync_bool_compare_and_swap(p,old,old+1)) return old+1;
    }
}
ATOMIC_FN void _gz_jwf_atomic_or(volatile uint32_t *p,uint32_t mask)
{ (void)__sync_fetch_and_or(p,mask); }
#undef ATOMIC_FN
