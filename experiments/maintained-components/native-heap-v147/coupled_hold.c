/* Real HG-first/job-TRY hold with a fixed once/300-frame three-root GHD
 * metadata consumer. This is JOL ABI4: family-zero after real TRY gates the existing metadata consumer.
 * No native object is dereferenced.
 * Existing HG and commonR2 APIs are linked directly, never provided by a
 * callback table or replaced with successful fixture/OS results. */
#include "coupled_hold.h"
#include "worker_family.h"

extern uint32_t _gz_hg_capture_begin(uint32_t cookie);
extern uint32_t _gz_hg_capture_seal(uint32_t token);
extern uint32_t _gz_hg_capture_end(uint32_t token);
extern uint32_t _gz_hg_platform_thread(void);

enum { JOL_COLD = 0, JOL_ACTIVE = 1, JOL_HALTED = 2 };
enum {
    JOL_E_SEQUENCE = 1, JOL_E_COUNTER = 2, JOL_E_STALE = 3,
    JOL_E_THREAD = 4, JOL_E_JOB_TRY = 5, JOL_E_CHECK = 6,
    JOL_E_JOB_LEAVE = 7, JOL_E_HG_END = 8, JOL_E_MEASURE = 9, JOL_E_FAMILY = 10
};
enum {
    JOL_CALLED_HG_BEGIN = 1, JOL_CALLED_HG_SEAL = 2,
    JOL_CALLED_HG_END = 4, JOL_CALLED_JOB_TRY = 8,
    JOL_CALLED_JOB_CHECK = 16, JOL_CALLED_JOB_LEAVE = 32,
    JOL_CALLED_FAMILY_ZERO = 64
};

GzJcaExclusive _gz_jol_job_token __attribute__((section(".bss.gz_jol")));
static uint32_t jol_busy __attribute__((section(".bss.gz_jol")));
static uint32_t jol_halted __attribute__((section(".bss.gz_jol")));
static uint32_t jol_hg_token __attribute__((section(".bss.gz_jol")));
static uint32_t jol_cookie __attribute__((section(".bss.gz_jol")));
static uint32_t jol_thread __attribute__((section(".bss.gz_jol")));

/* JOL1/v4, three-root metadata plus family-zero preflight; private observations only:
 * Family BUSY skips the consumer, cleans both holds and does not count16 or halt.
 * The distinct JWF1 report records actual private zero status/counters.
 * 3sequence;4state;5calls;6notready;7contextskip;8HGbegin calls;9HGzero;
 * 10HGacquired;11HGended;12jobTRY calls;13jobacquired;14jobBUSY;
 * 15jobLEFT;16checksOK;17first terminal reason;18rawHGbegin token;
 * 19rawHGseal;20rawHGend;21rawjobTRY;22rawjobcheck;23rawjobLEAVE;
 * 24actual retained HGtoken;25actual jobtoken.active;26cookie;27thread;
 * 28WORLD0;29MISSING15;30READ0;31called-bitmask above.
 * Raw UINT32_MAX with a clear called bit means no call. A real HG token may
 * itself equal UINT32_MAX; the called bit and retained token disambiguate it. */
volatile uint32_t _gz_jol_report[32] __attribute__((section(".data.gz_jol"))) = {
    0x4a4f4c31u, 128, 4, 0, JOL_COLD, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
    UINT32_MAX, UINT32_MAX, 0, 0, 0, 0, 0, 15, 0, 0
};

static void jol_barrier(void)
{
    __asm__ volatile("sync" ::: "memory");
}

uint32_t _gz_jol_coupled_outstanding(void)
{
    jol_barrier();
    return (jol_hg_token ? 1u : 0u) |
           ((_gz_jol_job_token.active || _gz_jol_job_token.gate) ? 2u : 0u);
}

uint32_t _gz_jol_coupled_halted(void)
{
    return jol_halted || _gz_jol_coupled_outstanding() ? 1u : 0u;
}

static void jol_fault(uint32_t reason)
{
    if (!_gz_jol_report[17])
        _gz_jol_report[17] = reason;
    jol_halted = 1;
    _gz_jol_report[4] = JOL_HALTED;
}

static void jol_count(uint32_t index)
{
    if (_gz_jol_report[index] != UINT32_MAX)
        ++_gz_jol_report[index];
    else
        jol_fault(JOL_E_COUNTER);
}

#include "actor_heap_cut.inc"

/* Cleanup is exact-owner API cleanup, including after context expiry.
 * Clear our private HG identity only after actual HG_OK. Never clear a real
 * job token here: only genuine commonR2 LEAVE may consume that token. */
static uint32_t jol_end_hg(void)
{
    uint32_t ended;
    _gz_jol_report[31] |= JOL_CALLED_HG_END;
    ended = _gz_hg_capture_end(jol_hg_token);
    _gz_jol_report[20] = ended;
    if (ended != 0) {
        jol_fault(JOL_E_HG_END);
        return 0;
    }
    jol_count(11);
    jol_hg_token = jol_cookie = jol_thread = 0;
    return 1;
}

void _gz_jol_coupled_probe(uint32_t cookie)
{
    uint32_t sequence;
    uint32_t tried;
    uint32_t checked_hg;
    uint32_t checked_job;
    uint32_t checked_family = UINT32_MAX;
    uint32_t left;
    JgdAttempt measurement;
    measurement.pending = 0;

    if (!cookie || !__sync_bool_compare_and_swap(&jol_busy, 0, 1))
        return;
    sequence = _gz_jol_report[3];
    if (jol_halted || (sequence & 1u) || sequence > UINT32_MAX - 3u) {
        jol_halted = 1;
        goto unlock;
    }
    ++_gz_jol_report[3];
    jol_barrier();
    if (sequence == UINT32_MAX - 3u) {
        jol_fault(JOL_E_SEQUENCE);
        goto done;
    }
    jol_count(5);
    _gz_jol_report[18] = _gz_jol_report[19] = UINT32_MAX;
    _gz_jol_report[20] = _gz_jol_report[21] = UINT32_MAX;
    _gz_jol_report[22] = _gz_jol_report[23] = UINT32_MAX;
    _gz_jol_report[31] = 0;
    if (jol_halted)
        goto done;
    if (jol_hg_token || jol_cookie || jol_thread ||
        _gz_jol_job_token.active || _gz_jol_job_token.gate) {
        jol_fault(JOL_E_STALE);
        goto done;
    }
    if (!_gz_jol_context_owned(cookie)) {
        jol_count(7);
        goto done;
    }
    if (!_gz_jwa_lifetime_ready()) {
        jol_count(6);
        goto done;
    }
    jgd_plan(cookie, &measurement);
    if (jgd_halted) jol_fault(JOL_E_MEASURE);
    if (jol_halted) goto done;
    _gz_jol_report[4] = JOL_ACTIVE;
    _gz_jol_report[26] = cookie;
    jol_count(8);
    if (jol_halted)
        goto done;
    _gz_jol_report[31] |= JOL_CALLED_HG_BEGIN;
    jol_hg_token = _gz_hg_capture_begin(cookie);
    _gz_jol_report[18] = jol_hg_token;
    if (!jol_hg_token) {
        /* Genuine HG begin returned no hold. Its shared diagnostic result is
         * not treated as a per-call BUSY certificate or fabricated success. */
        jol_count(9);
        goto done;
    }
    jol_cookie = cookie;
    jol_thread = _gz_hg_platform_thread();
    _gz_jol_report[27] = jol_thread;
    jol_count(10);
    if (!jol_thread || !_gz_jol_context_owned(cookie) || jol_halted) {
        jol_fault(JOL_E_THREAD);
        (void)jol_end_hg();
        goto done;
    }
    jol_count(12);
    if (jol_halted) {
        (void)jol_end_hg();
        goto done;
    }
    _gz_jol_report[31] |= JOL_CALLED_JOB_TRY;
    tried = _gz_jwa_lifetime_try(&_gz_jol_job_token, cookie);
    if (tried != GZ_JCA_OK) {
        _gz_jol_report[21] = tried;
        if (_gz_jol_job_token.active || _gz_jol_job_token.gate) {
            /* An actual outstanding job token keeps the genuine HG token.
             * This is unsafe terminal ownership, never forced cleanup. */
            jol_fault(JOL_E_STALE);
            goto done;
        }
        if (tried == GZ_JCA_BUSY)
            jol_count(14);
        else if (tried == JOL_BRIDGE_CONTEXT ||
                 tried == JOL_BRIDGE_NOT_READY)
            jol_count(7);
        else
            jol_fault(JOL_E_JOB_TRY);
        (void)jol_end_hg();
        goto done;
    }

    /* Both actual holds live. The fixed GHD consumer may run one REAL
     * nonwaiting private-HL three-root census, bracketed by genuine seal/job
     * checks. No native label/body, allocator, callback or wait is added here.
     * Only private3328B output and stack diagnostics change. Existing
     * commonR2 LEAVE still may contend on its private metadata mutex. */
    checked_hg = _gz_hg_capture_seal(jol_hg_token);
    checked_job = _gz_jwa_lifetime_check(&_gz_jol_job_token, cookie);
    if (checked_hg == 0 && checked_job == GZ_JCA_OK) {
        _gz_jol_report[31] |= JOL_CALLED_FAMILY_ZERO;
        checked_family = _gz_jwa_workers_zero(&_gz_jol_job_token, cookie);
        if (checked_family == JWF_ZERO_OK)
            jgd_consume(cookie, &measurement, checked_hg, checked_job,
                        jol_hg_token, &_gz_jol_job_token);
        /* BUSY: no GHD/API/body call, no wait/drain. The pending diagnostic
         * publishes its real no-call sentinels after matching cleanup. */
    } else {
        jgd_consume(cookie, &measurement, checked_hg, checked_job,
                    jol_hg_token, &_gz_jol_job_token);
    }
    left = _gz_jwa_lifetime_leave(&_gz_jol_job_token);

    _gz_jol_report[31] |= JOL_CALLED_HG_SEAL | JOL_CALLED_JOB_CHECK |
                             JOL_CALLED_JOB_LEAVE;
    _gz_jol_report[21] = tried;
    _gz_jol_report[19] = checked_hg;
    _gz_jol_report[22] = checked_job;
    _gz_jol_report[23] = left;
    jol_count(13);
    if (left != GZ_JCA_OK || _gz_jol_job_token.active ||
        _gz_jol_job_token.gate) {
        jol_fault(JOL_E_JOB_LEAVE);
        goto done; /* Retain actual HG identity and any actual job token. */
    }
    jol_count(15);
    if (checked_hg == 0 && checked_job == GZ_JCA_OK &&
        checked_family == JWF_ZERO_BUSY) {
        /* A cold whole invocation (or queued new OPEN entry) is normal BUSY.
         * Both real holds were cleaned; retry at the original future cut. */
    } else if (checked_hg == 0 && checked_job == GZ_JCA_OK &&
               checked_family == JWF_ZERO_OK &&
               (!measurement.pending || measurement.clean)) {
        jol_count(16);
    } else {
        jol_fault(checked_family == JWF_ZERO_BROKEN ? JOL_E_FAMILY : JOL_E_CHECK);
    }
    (void)jol_end_hg();
done:
    _gz_jol_report[24] = jol_hg_token;
    _gz_jol_report[25] = _gz_jol_job_token.active;
    jgd_publish(&measurement);
    if (jgd_halted) jol_fault(JOL_E_MEASURE);
    jol_barrier();
    ++_gz_jol_report[3];
    jol_barrier();
unlock:
    jol_barrier();
    jol_busy = 0;
}
