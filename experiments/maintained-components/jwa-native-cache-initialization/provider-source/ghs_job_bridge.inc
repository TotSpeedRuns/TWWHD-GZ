/* New recovery implementation, not a recovered copy of the missing provider.
 * Include AFTER the complete published JWA adapter and its five includes.
 * Uses that adapter's real static Gate; never initialize a second Gate here.
 * The GHS coordinator must also hold the real heap and particle owners.
 */
#include <stdint.h>

enum { GHSJ_CONTEXT = 0x2f0, GHSJ_NOT_READY = 0x2f1, GHSJ_STORAGE = 0x2f2 };
enum { GHSJ_ZERO = 0, GHSJ_BUSY = 1, GHSJ_BROKEN = 2 };
extern uint32_t _gz_ghs_context_owned(uint32_t request);

typedef struct {
    GzJcaExclusive token;
    uint32_t request;
} GhsJobHold;
static GhsJobHold ghsj __attribute__((section(".bss.gz_ghsj")));
_Static_assert(sizeof(GhsJobHold) == 20, "fixed private save-state job hold");
#define GHSJ_FN __attribute__((section(".text.gz_ghsj"), noinline, noclone))

GHSJ_FN uint32_t _gz_jwa_ghs_pending(void)
{
    const volatile GhsJobHold *hold = &ghsj;
    /* Inconsistent storage is still outstanding. Only a genuine successful
     * LEAVE clears it; a failed check must never erase a retained hold. */
    jwa_barrier();
    return hold->request || hold->token.active || hold->token.gate ||
           hold->token.thread || hold->token.generation ? 1u : 0u;
}

GHSJ_FN uint32_t _gz_jwa_ghs_try(uint32_t request)
{
    uint32_t result;
    if (!request || !_gz_ghs_context_owned(request) || !jwa_ordinary_context())
        return GHSJ_CONTEXT;
    if (!_gz_jwa_lifetime_ready())
        return GHSJ_NOT_READY;
    if ((uint32_t)(uintptr_t)OSGetCurrentThread() != jwa_init_thread)
        return GHSJ_CONTEXT;
    if (_gz_jwa_ghs_pending())
        return GHSJ_STORAGE;
    result = _gz_jca_exclusive_try(&jwa_gate, &ghsj.token);
    if (result == GZ_JCA_OK) {
        ghsj.request = request;
        jwa_barrier();
    }
    return result;
}

GHSJ_FN uint32_t _gz_jwa_ghs_check(uint32_t request)
{
    const volatile GzJcaExclusive *token = &ghsj.token;
    const volatile GzJcaGate *gate = &jwa_gate;
    OSThread *thread;
    if (!request || request != ghsj.request ||
        !_gz_ghs_context_owned(request) || !jwa_ordinary_context())
        return GHSJ_CONTEXT;
    if (!_gz_jwa_lifetime_ready())
        return GHSJ_NOT_READY;
    thread = OSGetCurrentThread();
    jwa_barrier();
    return thread && (uint32_t)(uintptr_t)thread == jwa_init_thread &&
           token->active == 1 && token->gate == &jwa_gate &&
           token->thread == thread && token->generation &&
           gate->closed == 1 && !gate->broken && !gate->active &&
           gate->owner == thread && gate->epoch == token->generation
               ? GZ_JCA_OK : GZ_JCA_BAD;
}

GHSJ_FN uint32_t _gz_jwa_ghs_workers_zero(uint32_t request)
{
    uint32_t a, b, c, d;
    if (_gz_jwa_ghs_check(request) != GZ_JCA_OK ||
        _gz_jwf_atomic_load(&_gz_jwf_report[3]))
        return GHSJ_BROKEN;
    a = _gz_jwf_atomic_load(&_gz_jwf_report[16]);
    b = _gz_jwf_atomic_load(&_gz_jwf_report[17]);
    c = _gz_jwf_atomic_load(&_gz_jwf_report[18]);
    d = _gz_jwf_atomic_load(&_gz_jwf_report[19]);
    jwf_barrier();
    if (_gz_jwf_atomic_load(&_gz_jwf_report[3]) ||
        _gz_jwa_ghs_check(request) != GZ_JCA_OK)
        return GHSJ_BROKEN;
    /* Initial admission requires ZERO. Once HELD, a new OPEN caller may be
     * counted while waiting outside this closed Gate. BUSY is not BROKEN;
     * this distinction is required by the existing GHS coordinator. */
    return a || b || c || d ? GHSJ_BUSY : GHSJ_ZERO;
}

GHSJ_FN uint32_t _gz_jwa_ghs_generation(uint32_t request)
{
    return _gz_jwa_ghs_check(request) == GZ_JCA_OK ? ghsj.token.generation : 0;
}

GHSJ_FN uint32_t _gz_jwa_ghs_leave(void)
{
    uint32_t result;
    if (!jwa_ordinary_context())
        return GHSJ_CONTEXT;
    if (!ghsj.request || ghsj.token.active != 1 ||
        ghsj.token.gate != &jwa_gate || !ghsj.token.generation)
        return GHSJ_STORAGE;
    /* Deliberately independent of the expired request and readiness flags.
     * commonR2 checks the actual thread, Gate, token and generation, and
     * performs the real wakeup. Failed cleanup keeps all evidence intact. */
    result = _gz_jca_exclusive_leave(&jwa_gate, &ghsj.token);
    if (result == GZ_JCA_OK) {
        ghsj.token.thread = 0;
        ghsj.token.generation = 0;
        ghsj.request = 0;
        jwa_barrier();
    }
    return result;
}

static uint32_t ghsj_apart(uint32_t pointer, uint32_t bytes,
                           uint32_t base, uint32_t size)
{
    return pointer && bytes && bytes <= UINT32_MAX - pointer &&
           size <= UINT32_MAX - base &&
           (pointer + bytes <= base || base + size <= pointer);
}

GHSJ_FN uint32_t _gz_jwa_ghs_private_apart(uint32_t pointer, uint32_t bytes)
{
    /* Rewinding the ownership machinery itself would invalidate every hold.
     * Protect all writable objects in this composed adapter, not just the
     * new token. Keep this list synchronized with the composed parent. */
#define GHSJ_APART(object) ghsj_apart(pointer, bytes, (uint32_t)(uintptr_t)&(object), sizeof(object))
    return GHSJ_APART(ghsj) && GHSJ_APART(jwa_gate) &&
           GHSJ_APART(jwa_invocation) && GHSJ_APART(jwa_directory_mutex) &&
           GHSJ_APART(jwa_serial) && GHSJ_APART(jwa_initialization) &&
           GHSJ_APART(jwa_init_thread) && GHSJ_APART(_gz_jwa_first) &&
           GHSJ_APART(_gz_jwa_report) && GHSJ_APART(jwa_probe_token) &&
           GHSJ_APART(jwa_probe_busy) && GHSJ_APART(jwa_probe_last_serial) &&
           GHSJ_APART(jwa_probe_halted) && GHSJ_APART(_gz_jwa_exclusive_probe_report) &&
           GHSJ_APART(jwf_invocation) && GHSJ_APART(jwf_serial) &&
           GHSJ_APART(_gz_jwf_report) && GHSJ_APART(_gz_wct_job_token) &&
           GHSJ_APART(_gz_jol_job_token) && GHSJ_APART(lrw) &&
           GHSJ_APART(lrw_hg) && GHSJ_APART(lrw_job) && GHSJ_APART(_gz_lrw_report);
#undef GHSJ_APART
}
#undef GHSJ_FN
