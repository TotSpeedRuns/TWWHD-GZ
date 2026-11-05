/* Included in the coherent JWA TU: this is the existing private state/Gate.
 * Non-COLD entry does not query any SDK, GFO or frame role. Preflight is not
 * atomic with another writer; the final check and constructor CAS arbitrate. */
#include "frame_init.h"
#include "worker_atomic.h"
extern uint32_t _gz_gfo_observer_serial(uint32_t phase);
extern volatile uint32_t _gz_pbac_frame;

__attribute__((noinline,noclone))
uint32_t _gz_jwa_init_after_native_frame(uint32_t parent, uint32_t thread, uint32_t frame)
{
    if (_gz_jwf_atomic_load(&jwa_initialization) != COLD)
        return 0;
    if (!parent || parent == UINT32_MAX || !thread || _gz_pbac_frame != frame)
        return 0;
    if (!jwa_ordinary_context())
        return 0;
    if ((uint32_t)(uintptr_t)OSGetCurrentThread() != thread ||
        _gz_gfo_observer_serial(2) != parent || _gz_pbac_frame != frame)
        return 0;
    jwa_barrier();
    if (_gz_gfo_observer_serial(2) != parent ||
        (uint32_t)(uintptr_t)OSGetCurrentThread() != thread || _gz_pbac_frame != frame)
        return 0;
    jwa_barrier();
    if (_gz_jwf_atomic_load(&jwa_initialization) != COLD)
        return 0;
    return _gz_jwa_init_open();
}
