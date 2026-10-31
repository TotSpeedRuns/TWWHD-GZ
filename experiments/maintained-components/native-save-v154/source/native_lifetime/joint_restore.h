#ifndef GZ_JOINT_RESTORE_DRIVER_H
#define GZ_JOINT_RESTORE_DRIVER_H
#include "rebuild.h"
typedef struct {ArDomain domain;
 AgU32 source,source_bytes,undo,undo_capacity,scratch,scratch_capacity,map,map_capacity,reserved[2];
} AjRestoreRequest;
typedef struct {ArResult arena;
 AgU32 undo_bundle_bytes,mapped,valid,phase,hr_error,missing,reserved[2];
} AjRestoreResult;
/* Algorithm-only internal driver. The caller still requires actual native
 * held-owner, SDK/resource/process/global authority. Nonzero descriptor flags
 * are not leases. This function never publishes slots or releases the world.
 * Source and independent undo are AJB1. The embedded prepared undo AFS is
 * reused directly, without a second current AFS copy. scratch may be zero/zero;
 * if provided its complete span is still alias-checked and remains untouched.
 * Result.valid acknowledges private maps for the returned phase; code9/phase2
 * means rollback, not requested-source success. The caller must inspect code.
 * No hook/function pointer is supplied by the caller or loaded from images. */
AgU32 _gz_aj_restore(const AjRestoreRequest*,AjRestoreResult*);
/* Resume the exact retained failed operation, same caller pointers/thread.
 * Currently supports healthy-observer, complete undo graph failures at final
 * payload verification. Other dirty failures stay retained for typed repair.
 * This never captures dirty current memory as a new undo image, releases the
 * world, clears observer faults or creates a whole-world permission. */
AgU32 _gz_aj_recover(const AjRestoreRequest*,AjRestoreResult*);
AgU32 _gz_aj_recovery_pending(void);
#endif
