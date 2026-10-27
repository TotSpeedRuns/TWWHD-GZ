/* Included only by the proposed adapter.c, so &jwa_gate is the exact same
 * private object passed by all nine preserved entry/return adapters.
 * TRY is nonwaiting; existing R2 LEAVE may block on its metadata OSMutex.
 * No game/resource/actor memory is read, copied, captured or restored here. */
enum { JXP_COLD = 0, JXP_ACTIVE = 1, JXP_HALTED = 2 };

static GzJcaExclusive jwa_probe_token __attribute__((section(".bss.gz_jxp")));
static uint32_t jwa_probe_busy __attribute__((section(".bss.gz_jxp")));
static uint32_t jwa_probe_last_serial __attribute__((section(".bss.gz_jxp")));
static uint32_t jwa_probe_halted __attribute__((section(".bss.gz_jxp")));

enum { JXP_SEQUENCE_REFUSE = 0, JXP_SEQUENCE_NORMAL = 1,
       JXP_SEQUENCE_EXHAUST = 2 };

/* Pure scalar plan: reserve FC->FD->FE for terminal publication. Odd,
 * terminal-even and all halted states are immutable report refusals. */
static uint32_t jwa_probe_sequence_plan(uint32_t sequence, uint32_t halted)
{
    if (halted || (sequence & 1u) || sequence > UINT32_MAX - 3u)
        return JXP_SEQUENCE_REFUSE;
    return sequence == UINT32_MAX - 3u ? JXP_SEQUENCE_EXHAUST
                                     : JXP_SEQUENCE_NORMAL;
}

/* JXP1/128B/v1. Private observations, never read readiness or game ownership.
 * 3 seq;4 state;5 calls;6 tries;7 acquired;8 BUSY;9 other-try;
 * 10 released;11 release-error;12 not-ready skips;13 context/serial skips;
 * 14 last serial;15 actual thread;16 init thread;17 raw IRQ previous;
 * 18 raw restore-previous;19 raw try;20 raw leave;21 actual token.active;
 * 22 token epoch;23 opaque same-Gate EA;24 WORLD0;25 MISSING15;26 READ0;
 * 27 seq exhausted;28 counter saturated;29 TRY/metadata-LEAVE mode1.
 * UINT32_MAX raw-code sentinels mean no such call, never a supplied result. */
volatile uint32_t _gz_jwa_exclusive_probe_report[32] __attribute__((section(".data.gz_jxp"))) = {
    0x4a585031u, 128, 1, 0, JXP_COLD, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, UINT32_MAX, UINT32_MAX, 0, 0, 0,
    0, 15, 0, 0, 0, 1, 0, 0
};

/* Single admitted frame producer owns these plain saturating increments.
 * Reentrant entry refuses before touching the report; no new indexed atomic
 * counter is introduced. R4's separate complete-EA atomic helper is unchanged. */
static void jwa_probe_count(uint32_t index)
{
    if (_gz_jwa_exclusive_probe_report[index] != UINT32_MAX)
        ++_gz_jwa_exclusive_probe_report[index];
    else
        _gz_jwa_exclusive_probe_report[28] = 1;
}

void _gz_jwa_exclusive_probe(uint32_t serial, uint32_t expected_thread)
{
    volatile uint32_t *report = _gz_jwa_exclusive_probe_report;
    uint32_t thread;
    uint32_t previous;
    uint32_t restore_previous;
    uint32_t tried;
    uint32_t sequence_plan;
    uint32_t left = UINT32_MAX;

    if (!serial || serial == UINT32_MAX || !expected_thread ||
        !__sync_bool_compare_and_swap(&jwa_probe_busy, 0, 1))
        return;
    sequence_plan = jwa_probe_sequence_plan(report[3], jwa_probe_halted);
    if (sequence_plan == JXP_SEQUENCE_REFUSE) {
        /* Preserve the first terminal raw codes and actual outstanding token.
         * A refused odd/terminal sequence is never repaired by report stores. */
        jwa_probe_halted = 1;
        goto unlock;
    }
    ++report[3];
    jwa_barrier();
    if (sequence_plan == JXP_SEQUENCE_EXHAUST) {
        /* Reserved FC->FD transaction. No TRY or raw-code reset occurs. */
        report[27] = 1;
        report[4] = JXP_HALTED;
        jwa_probe_halted = 1;
        goto done;
    }
    jwa_probe_count(5);
    report[19] = UINT32_MAX;
    report[20] = UINT32_MAX;

    /* Genuine one-time init must have published OPEN after OSInitMutex/Cond.
     * Current JWA fault/state is checked separately from JWT's init success. */
    if (*(volatile uint32_t *)&jwa_initialization != OPEN) {
        jwa_probe_count(12);
        goto done;
    }
    jwa_barrier();
    if (_gz_jwa_report[4] || !jwa_init_thread) {
        jwa_probe_count(12);
        goto done;
    }
    thread = (uint32_t)(uintptr_t)OSGetCurrentThread();
    report[15] = thread;
    report[16] = jwa_init_thread;
    if (!thread || thread != expected_thread || thread != jwa_init_thread ||
        serial <= jwa_probe_last_serial) {
        jwa_probe_count(13);
        goto done;
    }
    jwa_probe_last_serial = serial;
    report[14] = serial;
    previous = (uint32_t)OSDisableInterrupts();
    restore_previous = (uint32_t)OSRestoreInterrupts((BOOL)previous);
    report[17] = previous;
    report[18] = restore_previous;
    if (!previous || restore_previous != 0) {
        jwa_probe_count(13);
        goto done;
    }
    if (jwa_probe_token.active) {
        /* Never overwrite, clear or forge cleanup for an outstanding token. */
        jwa_probe_halted = 1;
        report[4] = JXP_HALTED;
        goto done;
    }

    report[4] = JXP_ACTIVE;
    report[23] = (uint32_t)(uintptr_t)&jwa_gate;
    jwa_probe_count(6);
    /* No game read, private report update, extra lock or callback between
     * successful acquisition and genuine same-thread conditional release. */
    tried = _gz_jca_exclusive_try(&jwa_gate, &jwa_probe_token);
    if (tried == GZ_JCA_OK)
        left = _gz_jca_exclusive_leave(&jwa_gate, &jwa_probe_token);

    report[19] = tried;
    report[20] = left;
    report[21] = jwa_probe_token.active;
    report[22] = jwa_probe_token.generation;
    if (tried == GZ_JCA_BUSY) {
        /* Ordinary active writers or metadata contention are normal skips. */
        jwa_probe_count(8);
    } else if (tried == GZ_JCA_OK) {
        jwa_probe_count(7);
        if (left == GZ_JCA_OK && !jwa_probe_token.active &&
            jwa_probe_token.gate == NULL) {
            jwa_probe_count(10);
        } else {
            jwa_probe_count(11);
            jwa_probe_halted = 1;
            report[4] = JXP_HALTED;
        }
    } else {
        jwa_probe_count(9);
        jwa_probe_halted = 1;
        report[4] = JXP_HALTED;
    }
done:
    report[21] = jwa_probe_token.active;
    report[22] = jwa_probe_token.generation;
    if (report[28]) {
        jwa_probe_halted = 1;
        report[4] = JXP_HALTED;
    }
    jwa_barrier();
    ++report[3];
    jwa_barrier();
unlock:
    jwa_barrier();
    jwa_probe_busy = 0;
}
