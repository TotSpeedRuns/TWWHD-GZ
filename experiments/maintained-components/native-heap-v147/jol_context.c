/* Dedicated HG-only context for the real completed native-frame producer.
 * No game/root/vector bytes are followed. Every sampled byte belongs to fixed
 * component-owned GFO/GCC metadata. No READ_READY or WORLD route is added. */
#include "jol_context.h"
#include "coupled_hold.h"
#include <coreinit/interrupts.h>

extern uint32_t _gz_gfo_observer_serial(uint32_t phase);
extern uint32_t _gz_hg_platform_thread(void);
extern uint32_t _gz_jol_hg_context_previous(uint32_t cookie);
extern volatile uint32_t _gz_pbac_frame;
extern uint32_t _gz_gcc_report[280];

enum { JLC_AFTER_RETURN = 2, JLC_ACTIVE = 1, JLC_HALTED = 2 };
typedef struct {
    uint32_t busy, active, cookie, parent, thread, frame;
    uint32_t last_parent, last_gcc_sequence, halted;
} JolContext;
static volatile JolContext jol_context
    __attribute__((section(".bss.gz_jlc")));

/* JLC1/128B/v1:3sequence;4state;5producer calls;6stale skips;
 * 7cohort skips;8context/IRQ skips;9admitted;10retired;11halt reason;
 * 12cookie;13parent;14thread;15frame;16GCCsequence;17rawIRQprevious;
 * 18rawIRQrestore;19outstanding-mask;20WORLD0;21MISSING15;22READ0;
 * 23..31reserved0. Raw UINT32_MAX means no API query, never success. */
volatile uint32_t _gz_jol_context_report[32]
    __attribute__((section(".data.gz_jlc"))) = {
    0x4a4c4331u, 128, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, UINT32_MAX, UINT32_MAX, 0, 0, 15, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

static void jol_context_barrier(void)
{
    __asm__ volatile("sync" ::: "memory");
}

static void jol_context_halt(uint32_t reason)
{
    if (!_gz_jol_context_report[11])
        _gz_jol_context_report[11] = reason;
    jol_context.halted = 1;
    _gz_jol_context_report[4] = JLC_HALTED;
}

static void jol_context_count(uint32_t index)
{
    if (_gz_jol_context_report[index] == UINT32_MAX)
        jol_context_halt(2);
    else
        ++_gz_jol_context_report[index];
}

/* Pure private-report filter. A detached success is only a format result;
 * the live producer/observer/thread checks below remain mandatory. This is
 * deliberately the already observed3jobs+FIFO cohort, not arbitrary work. */
static __attribute__((noinline, noclone)) uint32_t jol_cohort_matches(
    const volatile uint32_t *report, uint32_t parent, uint32_t thread,
    uint32_t frame)
{
    static const uint32_t jol_context_callers[4] = {
        0x027f9c70u, 0x0272b5ecu, 0x0272b758u, 0x027fa320u
    };
    uint32_t i, j, event = 0, job_serial = 0;
    if (report[0] != 0x47434331u || report[1] != 1120 || report[2] != 1 ||
        !report[3] || (report[3] & 1u) || report[3] >= UINT32_MAX - 1u ||
        report[4] != parent || report[5] != JLC_AFTER_RETURN ||
        report[6] > frame || report[7] != frame || report[8] != thread ||
        report[9] != 1 || report[10] != 15 || report[11] ||
        report[12] == UINT32_MAX || report[13] == UINT32_MAX ||
        report[13] < report[12] || report[13] - report[12] != 4 ||
        report[14] != 4 || report[15] != 4 || report[16] != 4 ||
        report[17] || report[18] || report[19] != 16 || report[20] != 4 ||
        report[21] == UINT32_MAX || report[21] != report[22] || report[23])
        return 0;
    for (i = 0; i < 16; ++i) {
        const volatile uint32_t *row = report + 24 + i * 16;
        if (i >= 4) {
            for (j = 0; j < 16; ++j)
                if (row[j])
                    return 0;
            continue;
        }
        if (row[0] != (i == 3 ? 2u : 1u) || !row[1] ||
            row[1] == UINT32_MAX || row[2] != parent || row[3] != parent ||
            row[4] < report[6] || row[5] < row[4] || row[5] > frame ||
            row[6] != thread || row[7] != thread || !row[8] || (row[8] & 3) ||
            row[9] != jol_context_callers[i] || !row[10] || row[10] <= event ||
            row[11] <= row[10] || row[12] != i + 1 ||
            row[13] || row[14] || row[15])
            return 0;
        if (i < 3 && row[1] <= job_serial)
            return 0;
        if (i < 3)
            job_serial = row[1];
        event = row[11];
    }
    /* The two general waits must be the same actual opaque native label;
     * this is provenance identity only and is never followed as a pointer. */
    return report[24 + 16 + 8] == report[24 + 32 + 8];
}

uint32_t _gz_jol_context_owned(uint32_t cookie)
{
    uint32_t parent, thread, frame, previous, restored;
    if (!cookie || jol_context.halted || jol_context.active != 1 ||
        jol_context.cookie != cookie ||
        !jol_context.parent || !jol_context.thread)
        return 0;
    parent = jol_context.parent;
    thread = jol_context.thread;
    frame = jol_context.frame;
    previous = (uint32_t)OSDisableInterrupts();
    restored = (uint32_t)OSRestoreInterrupts((BOOL)previous);
    if (!previous || restored)
        return 0;
    jol_context_barrier();
    if (_gz_hg_platform_thread() != thread || _gz_pbac_frame != frame ||
        _gz_gfo_observer_serial(JLC_AFTER_RETURN) != parent)
        return 0;
    jol_context_barrier();
    return !jol_context.halted && jol_context.active == 1 &&
           jol_context.cookie == cookie &&
           jol_context.parent == parent && jol_context.thread == thread &&
           jol_context.frame == frame ? 1u : 0u;
}

/* Existing KSR/SRH/NG code is retained under an exact renamed entry. JLC
 * adds no caller-selected callback or alternate read-context provider. */
uint32_t _gz_hg_context_owned(uint32_t cookie)
{
    /* A live/frozen JOL number is reserved for this producer. A failed JOL
     * authentication must not fall through to an unrelated same-number
     * legacy cookie. Other legacy requests retain their original route. */
    if ((jol_context.active || jol_context.halted) &&
        cookie == jol_context.cookie)
        return _gz_jol_context_owned(cookie);
    return _gz_jol_hg_context_previous(cookie);
}

void _gz_jol_after_native_frame(uint32_t parent, uint32_t thread, uint32_t frame)
{
    volatile uint32_t *report = _gz_jol_context_report;
    const volatile uint32_t *cohort = _gz_gcc_report;
    uint32_t sequence, gcc_sequence, previous, restored;
    if (!parent || !thread ||
        !__sync_bool_compare_and_swap(&jol_context.busy, 0, 1))
        return;
    sequence = report[3];
    if (jol_context.halted || (sequence & 1u) || sequence > UINT32_MAX - 3u) {
        jol_context.halted = 1;
        goto unlock;
    }
    ++report[3];
    jol_context_barrier();
    if (sequence == UINT32_MAX - 3u) {
        jol_context_halt(1);
        goto done;
    }
    jol_context_count(5);
    if (jol_context.halted)
        goto done;
    report[17] = report[18] = UINT32_MAX;
    if (jol_context.active || _gz_jol_coupled_halted() ||
        _gz_jol_coupled_outstanding()) {
        jol_context_halt(3);
        goto done;
    }
    if (parent <= jol_context.last_parent || parent == UINT32_MAX ||
        jol_context.cookie == UINT32_MAX) {
        jol_context_count(6);
        goto done;
    }
    if (_gz_gfo_observer_serial(JLC_AFTER_RETURN) != parent ||
        _gz_hg_platform_thread() != thread || _gz_pbac_frame != frame) {
        jol_context_count(8);
        goto done;
    }
    previous = (uint32_t)OSDisableInterrupts();
    restored = (uint32_t)OSRestoreInterrupts((BOOL)previous);
    report[17] = previous;
    report[18] = restored;
    if (!previous || restored) {
        jol_context_count(8);
        goto done;
    }
    gcc_sequence = cohort[3];
    jol_context_barrier();
    if (gcc_sequence <= jol_context.last_gcc_sequence ||
        !jol_cohort_matches(cohort, parent, thread, frame)) {
        jol_context_count(7);
        goto done;
    }
    jol_context_barrier();
    if (cohort[3] != gcc_sequence ||
        _gz_gfo_observer_serial(JLC_AFTER_RETURN) != parent ||
        _gz_hg_platform_thread() != thread || _gz_pbac_frame != frame) {
        jol_context_count(8);
        goto done;
    }
    jol_context.parent = parent;
    jol_context.thread = thread;
    jol_context.frame = frame;
    jol_context.last_parent = parent;
    jol_context.last_gcc_sequence = gcc_sequence;
    ++jol_context.cookie;
    report[12] = jol_context.cookie;
    report[13] = parent;
    report[14] = thread;
    report[15] = frame;
    report[16] = gcc_sequence;
    jol_context_count(9);
    if (jol_context.halted)
        goto done;
    report[4] = JLC_ACTIVE;
    jol_context_barrier();
    jol_context.active = 1;
    _gz_jol_coupled_probe(jol_context.cookie);
    report[19] = _gz_jol_coupled_outstanding();
    if (_gz_jol_coupled_halted() || report[19]) {
        /* Preserve all identity diagnostics and actual retained tokens.
         * This does not extend authority: context_owned STILL requires the
         * genuine live AFTER_RETURN phase/thread/frame on every call. */
        jol_context_halt(4);
        goto done;
    }
    jol_context.active = 0;
    jol_context_barrier();
    report[4] = 0;
    jol_context_count(10);
done:
    jol_context_barrier();
    ++report[3];
unlock:
    jol_context_barrier();
    jol_context.busy = 0;
}

/* Authenticated scalar labels for the fixed metadata consumer. */
#include "context_labels.inc"
