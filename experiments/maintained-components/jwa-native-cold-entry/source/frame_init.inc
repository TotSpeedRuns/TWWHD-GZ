/* Included in the coherent JWA TU: this is the existing private state/Gate.
 * Non-COLD frame entry does not query any SDK, GFO or frame role. Preflight is
 * not atomic with another writer; final checks and the constructor CAS arbitrate.
 * The Native cache prepare call can yield: all later checks remain mandatory. */
#include "frame_init.h"
#include "worker_atomic.h"
extern uint32_t _gz_gfo_observer_serial(uint32_t phase);
extern volatile uint32_t _gz_pbac_frame;
/* Genuine zero-argument Native role, deliberately UND FUNC, with no address
 * alias in Source. Future physical binding must authenticate loader and owners.
 * Return1 means cache READY2; busy1/failed3 return0 without reset or retry. */
extern uint32_t _gz_jwa_native_sdk_prepare(void);
__asm__(".global _gz_jwa_native_sdk_prepare\n"
        ".type _gz_jwa_native_sdk_prepare,@function");

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
    if (_gz_jwa_native_sdk_prepare() != 1)
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

/* Future replacement for the Native JWT constructor call only. It neither
 * changes JWT/probe gates nor initializes an unaliased Native Gate. Acceptance
 * of existing OPEN is a bounded observation, not an atomic return-time promise.
 * INITIALIZING/POISONED are never reset, and no initialized object is recreated. */
__attribute__((noinline,noclone))
uint32_t _gz_jwa_init_for_native_jwt(void)
{
    uint32_t thread, state;
    if (!jwa_ordinary_context())
        return 0;
    thread = (uint32_t)(uintptr_t)OSGetCurrentThread();
    if (!thread)
        return 0;
    if (_gz_jwa_native_sdk_prepare() != 1)
        return 0;
    jwa_barrier();
    if (!jwa_ordinary_context() ||
        (uint32_t)(uintptr_t)OSGetCurrentThread() != thread)
        return 0;
    state = _gz_jwf_atomic_load(&jwa_initialization);
    if (state == COLD)
        return _gz_jwa_init_open();
    if (state != OPEN)
        return 0;
    jwa_barrier();
    if (_gz_jwa_report[3] != OPEN ||
        *(volatile uint32_t *)&jwa_init_thread != thread ||
        _gz_jwa_report[59] != thread)
        return 0;
    if ((uint32_t)(uintptr_t)OSGetCurrentThread() != thread)
        return 0;
    jwa_barrier();
    if (_gz_jwf_atomic_load(&jwa_initialization) != OPEN ||
        _gz_jwa_report[3] != OPEN ||
        *(volatile uint32_t *)&jwa_init_thread != thread ||
        _gz_jwa_report[59] != thread)
        return 0;
    return 1;
}


/* Future zeroarg retirement target for the old Native constructor entry.
 * Preserve non-COLD return0 before SDK queries. This guard does not poison on
 * invalid preflight/cache failure, unlike Native260's post-CAS context failure.
 * SDK lock/module lifetime, physical owner and cold-first-use remain premises.
 * Any state/context change after these observations is arbitrated again by
 * the fixed constructor CAS/context checks; no state is reset here. */
__attribute__((noinline,noclone))
uint32_t _gz_jwa_init_cold_for_native_entry(void)
{
    uint32_t thread;
    if (_gz_jwf_atomic_load(&jwa_initialization) != COLD)
        return 0;
    if (!jwa_ordinary_context())
        return 0;
    thread = (uint32_t)(uintptr_t)OSGetCurrentThread();
    if (!thread)
        return 0;
    if (_gz_jwa_native_sdk_prepare() != 1)
        return 0;
    jwa_barrier();
    if (!jwa_ordinary_context() ||
        (uint32_t)(uintptr_t)OSGetCurrentThread() != thread)
        return 0;
    if (_gz_jwf_atomic_load(&jwa_initialization) != COLD)
        return 0;
    return _gz_jwa_init_open();
}
