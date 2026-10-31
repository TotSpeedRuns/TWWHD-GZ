#ifndef GZ_OWNED_HEAP_IMAGES_H
#define GZ_OWNED_HEAP_IMAGES_H
#include "joint_restore.h"
enum {OIS_SLOTS=10,OIS_PINS=32,OIS_NONE=0,OIS_PREPARED=1,OIS_APPLYING=2,OIS_RESULT=3,OIS_DIRTY=4};
enum {OIS_OK=0,OIS_BUSY=8,OIS_ARGUMENT=70,OIS_CHANGED=71,OIS_INCOMPLETE=72,OIS_SPACE=73,OIS_EXHAUSTED=74,OIS_CORRUPT=75,OIS_TYPE=76,OIS_UNAVAILABLE=77};
typedef struct {AgU32 allocation,capacity,archive,bytes,identity,generation,pin,reserved;} OisLease;
typedef struct {
 AgU32 magic,bytes,version,token,phase,code,slot,source,source_bytes,undo,undo_capacity,map,map_capacity,missing,reserved[2];
 AjRestoreResult result;
} OisInfo;
/* Alternative variable-size image owner based on the OSS exact pin protocol.
 * Captures algorithm images only; neither API supplies world/resource hold,
 * a playable user save, durable disk storage, or full-game restore admission.
 * Fixed Cafe MEM2 calls produce every region: caller pointers are never
 * adopted as allocations. All API arguments must be mapped caller-owned
 * stack/private objects; raw archive fields never become API arguments.
 * Lock order is this owner -> lifetime metadata. No reverse callback is used.
 */
AgU32 _gz_ois_capture_resource(AgU32 slot,AgU32 object,AgU32 heap,AgU32 capacity);
AgU32 _gz_ois_capture_forest(AgU32 slot,const AfDomain*,AgU32 capacity);
AgU32 _gz_ois_acquire_read(AgU32 slot,OisLease*);
AgU32 _gz_ois_validate_read(const OisLease*);
AgU32 _gz_ois_release_read(const OisLease*);
AgU32 _gz_ois_drop(AgU32 slot);
/* Prepare retains the actual source domain from capture, a private read pin,
 * independent undo and maps. Apply keeps all three on return. Finish may be
 * called only after the real caller has consumed maps and finalized its held
 * operation. code+epoch must match the retained result exactly. Dirty recovery
 * refuses finish and retains every region; no force-discard API exists.
 * This ownership does NOT replace the actual whole-world held-owner protocol.
 */
AgU32 _gz_ois_prepare_restore(AgU32 slot,const AfDomain*current,AgU32 target_epoch,AgU32 rollback_epoch,AgU32 undo_capacity,AgU32 map_capacity,AgU32*token);
AgU32 _gz_ois_apply_restore(AgU32 token);
AgU32 _gz_ois_cancel_restore(AgU32 token); /* prepared/unmutated only */
AgU32 _gz_ois_restore_info(AgU32 token,OisInfo*);
AgU32 _gz_ois_finish_restore(AgU32 token,AgU32 expected_code,AgU32 expected_epoch);
AgU32 _gz_ois_recovery_pending(void);
/* Resume the original retained transaction using only its independent undo.
 * The actual outer world hold must remain active; dirty refusals keep pins. */
AgU32 _gz_ois_recover_restore(AgU32 token);
#endif
