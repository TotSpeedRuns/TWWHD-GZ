/* Included only after the unchanged R832 adapter implementation. No second
 * Gate is initialized or exported. These functions operate on the SAME static
 * jwa_gate as the nine whole-entry native job adapters. */
#include "coupled_hold.h"

#define JLC_TEXT __attribute__((section(".text.gz_jlc"), noinline, noclone))

JLC_TEXT uint32_t _gz_jwa_lifetime_ready(void)
{
    if (*(volatile uint32_t *)&jwa_initialization != OPEN)
        return 0;
    jwa_barrier();
    return !_gz_jwa_report[4] && jwa_init_thread && !jwa_probe_halted &&
           !jwa_probe_token.active && !jwa_probe_token.gate ? 1u : 0u;
}

JLC_TEXT uint32_t _gz_jwa_lifetime_try(GzJcaExclusive *token, uint32_t cookie)
{
    if (token != &_gz_jol_job_token)
        return JOL_BRIDGE_STORAGE;
    if (!_gz_jol_context_owned(cookie) || !jwa_ordinary_context())
        return JOL_BRIDGE_CONTEXT;
    if (!_gz_jwa_lifetime_ready())
        return JOL_BRIDGE_NOT_READY;
    if ((uint32_t)(uintptr_t)OSGetCurrentThread() != jwa_init_thread)
        return JOL_BRIDGE_CONTEXT;
    /* Actual commonR2 TRY result0..4, including genuine BUSY. No simulated
     * result, retry, drain, extra Gate, callback or native object access. */
    return _gz_jca_exclusive_try(&jwa_gate, token);
}

JLC_TEXT uint32_t _gz_jwa_lifetime_check(const GzJcaExclusive *token,
                                       uint32_t cookie)
{
    const volatile GzJcaExclusive *actual;
    volatile GzJcaGate *gate = &jwa_gate;
    OSThread *thread;
    if (token != &_gz_jol_job_token)
        return JOL_BRIDGE_STORAGE;
    if (!_gz_jol_context_owned(cookie) || !jwa_ordinary_context())
        return JOL_BRIDGE_CONTEXT;
    if (!_gz_jwa_lifetime_ready())
        return JOL_BRIDGE_NOT_READY;
    thread = OSGetCurrentThread();
    actual = token;
    jwa_barrier();
    /* This is a private owner-held consistency check, not an unlocked
     * acquisition or a native lifetime/read grant. Existing Gate BREAK or
     * identity drift makes the actual hold unusable and terminal upstream. */
    return thread && (uint32_t)(uintptr_t)thread == jwa_init_thread &&
           actual->active == 1 && actual->gate == &jwa_gate &&
           actual->thread == thread && actual->generation &&
           gate->closed == 1 && !gate->broken && !gate->active &&
           gate->owner == thread && gate->epoch == actual->generation
               ? GZ_JCA_OK : GZ_JCA_BAD;
}

JLC_TEXT uint32_t _gz_jwa_lifetime_leave(GzJcaExclusive *token)
{
    if (token != &_gz_jol_job_token)
        return JOL_BRIDGE_STORAGE;
    if (!jwa_ordinary_context())
        return JOL_BRIDGE_CONTEXT;
    /* Cleanup deliberately does NOT depend on a live context or healthy
     * initialization. Actual same-thread/token checks belong to commonR2.
     * Its LEAVE may contend on its private metadata mutex; no native lock or
     * payload callback is held here. Only actual LEAVE consumes this token. */
    return _gz_jca_exclusive_leave(&jwa_gate, token);
}

#undef JLC_TEXT
