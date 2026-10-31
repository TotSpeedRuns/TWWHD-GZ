#ifndef GZ_GENERIC_PROCESS_CUT_NATIVE_H
#define GZ_GENERIC_PROCESS_CUT_NATIVE_H
#include "generic_process_reconcile.h"

/* Private cut consumer. These operations describe REQUIRED work; they are
 * neither callable native tickets nor a Restore certificate. The raw owned
 * domain, rather than per-profile serializers/constructors, remains the
 * source of saved opaque bytes and original identity. */
enum { GPN_VERSION=1,GPN_MAGIC=0x47504e31u,
 GPN_OK=0,GPN_ARGUMENT=1,GPN_CUT=2,GPN_NATIVE_HEADER=3,GPN_PLAN=4,
 GPN_INCOMPLETE=5,
 GPN_DOMAIN_REWIND=1,GPN_CURRENT_CLEANUP_AND_DOMAIN_REWIND=2,
 GPN_SAVED_DOMAIN_REINSTATE=3,GPN_REUSED_ADDRESS_RECONCILE=4,GPN_AUDIT_ONLY=5,
 GPN_MISSING_ALL=255,
 GPN_NEEDS_FPC_GLOBALS=1,GPN_NEEDS_NATIVE_LAYERS=2,
 GPN_NEEDS_REVERSE_REFS=4,GPN_NEEDS_NATIVE_RESOURCE_CLOSURE=8,
 GPN_NEEDS_GPU_AUDIO=16,GPN_NEEDS_REAL_WRITER=32,
 GPN_NEEDS_CONSTRUCTOR_HISTORY=64,GPN_NEEDS_CURRENT_CUT=128 };
typedef struct {const void *image;u32 bytes,identity,generation;} GpnImage;

/* Full base bytes are copied from the PRIVATE domain union at a checked
 * offset. None of the native-address scalars in this record is followed. */
typedef struct {
 u32 index,raw_offset,checksum,profile,layer,methods,create_request,append;
 u32 subtype_present,subtype,missing,reserved;
 unsigned char base[0xb8];
} GpnBase;
typedef char GpnBase232[sizeof(GpnBase)==232?1:-1];
typedef struct {
 u32 saved_index,current_index,operation,missing;
 u32 saved_offset,current_offset,saved_profile,current_profile;
 u32 saved_pid,current_pid,saved_generation,current_generation;
 /* Proven common framework entries, evidence only, never cast/called. */
 u32 native_delete_entry,native_create_request_entry,requirements,reserved;
} GpnAction;
typedef char GpnAction64[sizeof(GpnAction)==64?1:-1];
typedef struct {
 u32 magic,bytes,version,valid,status,missing,native_eligible,replay_eligible;
 u32 count,rewind,current_cleanup,saved_reinstate,reused,audit_only,requirements,reserved;
} GpnReport;
typedef char GpnReport64[sizeof(GpnReport)==64?1:-1];
/* Caller-owned private work, no allocator/global BSS. SF's cut projections
 * and full stamp comparisons remain genuine code, not synthetic accessors. */
typedef struct {
 GrcCutStamp saved_stamp,current_stamp;
 GrcCutView saved_view,current_view;
 GpnBase saved_base,current_base;
} GpnWork;

/* Uses the frozen AS6 parser and grc_plan, then consumes actual native base
 * fields from both PRIVATE raw unions. All output/work spans must be apart.
 * The caller must independently retain real OIS read leases, call genuine
 * _gz_ghc_validate at its boundaries and provide a genuinely fresh current
 * cut. This pure code obtains no physical hold itself. Completion returns
 * GPN_INCOMPLETE with a valid diagnostic plan and missing255, including the
 * all-matching case. Format/cut/native-header failures never publish valid.
 * Native Delete/Create and allocation-domain replay are never executed. */
u32 gpn_build(const GpnImage *saved,const GpnImage *current,
 const GrcCutStamp *expected_current,GpnWork *work,u32 *sort,u32 sort_words,
 GrcPlan *plan,GrcPlanRow *plan_rows,GpnAction *actions,u32 capacity,GpnReport *report);

/* Pure generic base decoder shared with ROOT host fixtures. It accepts
 * PRIVATE bytes only, compares native PID/profile/profile-entry/name with
 * the genuine PO row, and permits PID0/hash0/heap-epoch0. */
u32 gpn_decode_base(const PoEntry*,const unsigned char*,u32 available,GpnBase*);
#endif
