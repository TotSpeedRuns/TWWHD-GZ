#ifndef GZ_GENERIC_FRAME_RETURN_OBSERVER_H
#define GZ_GENERIC_FRAME_RETURN_OBSERVER_H

typedef unsigned int GfoU32;

enum {
    GFO_FRAME_RETURN_OBSERVED = 1,
    GFO_MISSING_CPU_COMPLETION = 1,
    GFO_MISSING_NO_NEW_SUBMITTER = 2,
    GFO_MISSING_EXTERNAL_WRITERS = 4,
    GFO_MISSING_PHYSICAL_INSTANCE = 8
};

typedef struct {
    GfoU32 serial;
    GfoU32 entered;
    GfoU32 thread;
    GfoU32 instance;
    GfoU32 original_caller;
    GfoU32 entry_frame;
    GfoU32 reserved[2];
} GfoInvocation;

/* Fixed-image diagnostics only. No pointer in this report is followed. */
extern GfoU32 _gz_gfo_report[32];

/* Private wrapper helpers. A helper call or report copy issues no read scope. */
GfoU32 _gz_gfo_enter(GfoU32 instance, GfoU32 original_caller);
void _gz_gfo_return(GfoU32 serial);
GfoU32 _gz_gfo_read_ready(void);
GfoU32 _gz_gfo_context_owned(GfoU32 cookie);

#endif
