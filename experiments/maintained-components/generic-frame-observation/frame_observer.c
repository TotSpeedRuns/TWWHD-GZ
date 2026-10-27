#include "frame_observer.h"

extern GfoU32 _gz_hg_platform_thread(void);
extern volatile GfoU32 _gz_pbac_frame;

static GfoInvocation gfo_invocation;
GfoU32 _gz_gfo_report[32] = {0x47464f31u, 128, 1};

static void gfo_increment(GfoU32 *value)
{
    if (*value != 0xffffffffu)
        ++*value;
}

/* This observer deliberately has no HG/JPA admission route. A real returned
 * native frame is useful provenance, but it does not prove CPU job completion,
 * no new submissions, physical object ownership or external writer exclusion.
 */
GfoU32 _gz_gfo_read_ready(void)
{
    return 0;
}

GfoU32 _gz_gfo_context_owned(GfoU32 cookie)
{
    (void)cookie;
    return 0;
}

GfoU32 _gz_gfo_enter(GfoU32 instance, GfoU32 original_caller)
{
    GfoU32 thread;

    if (!__sync_bool_compare_and_swap(&gfo_invocation.entered, 0, 1)) {
        gfo_increment(_gz_gfo_report + 16);
        return 0;
    }
    if (gfo_invocation.serial == 0xffffffffu) {
        gfo_increment(_gz_gfo_report + 19);
        __asm__ volatile("sync" ::: "memory");
        gfo_invocation.entered = 0;
        return 0;
    }
    thread = _gz_hg_platform_thread();
    if (!thread) {
        gfo_increment(_gz_gfo_report + 18);
        gfo_invocation.entered = 0;
        return 0;
    }
    ++gfo_invocation.serial;
    gfo_invocation.thread = thread;
    gfo_invocation.instance = instance;
    gfo_invocation.original_caller = original_caller;
    gfo_invocation.entry_frame = _gz_pbac_frame;
    __asm__ volatile("sync" ::: "memory");
    return gfo_invocation.serial;
}

/* Separate scalar commit allows an emitted pure-metadata test without issuing
 * or supplying an OS/native result. It never creates a read permission.
 */
static __attribute__((noinline, noclone)) void gfo_record_return(
    GfoU32 serial, GfoU32 thread, GfoU32 frame)
{
    GfoU32 *report = _gz_gfo_report;

    if (!serial || !gfo_invocation.entered ||
        serial != gfo_invocation.serial) {
        if (serial)
            gfo_increment(report + 17);
        return;
    }
    if (report[3] > 0xfffffffcu) {
        gfo_increment(report + 19);
        gfo_invocation.entered = 0;
        return;
    }
    ++report[3];
    __asm__ volatile("sync" ::: "memory");
    report[4] = serial;
    report[5] = GFO_FRAME_RETURN_OBSERVED;
    report[6] = gfo_invocation.entry_frame;
    report[7] = frame;
    report[8] = gfo_invocation.thread;
    report[9] = thread;
    report[10] = gfo_invocation.instance;
    report[11] = gfo_invocation.original_caller;
    report[12] = 0x02034ffcu;
    report[13] = 0x02035000u;
    report[14] = thread && thread == gfo_invocation.thread ? 0 : 1;
    gfo_increment(report + 15);
    report[20] = 0;
    report[21] = GFO_MISSING_CPU_COMPLETION | GFO_MISSING_NO_NEW_SUBMITTER |
                 GFO_MISSING_EXTERNAL_WRITERS | GFO_MISSING_PHYSICAL_INSTANCE;
    report[22] = serial;
    report[23] = 1;
    report[24] = 0;
    report[25] = 0;
    report[26] = report[27] = report[28] = report[29] = report[30] = report[31] = 0;
    gfo_invocation.entered = 0;
    __asm__ volatile("sync" ::: "memory");
    ++report[3];
}

void _gz_gfo_return(GfoU32 serial)
{
    GfoU32 thread;

    /* A nested wrapper's zero cookie never clears the outer invocation. */
    if (!serial || !gfo_invocation.entered ||
        serial != gfo_invocation.serial)
        return;
    thread = _gz_hg_platform_thread();
    gfo_record_return(serial, thread, _gz_pbac_frame);
}
