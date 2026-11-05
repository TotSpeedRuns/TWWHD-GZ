#ifndef GZ_ARENA_REBUILD_H
#define GZ_ARENA_REBUILD_H
#include "forest_snapshot.h"
/* Unbound algorithm receipts, NOT engine/resource ownership certificates.
 * source generation identifies immutable saved data; current generation
 * identifies prepared undo. A real owner must rebind lifetimes and finalize
 * pending_epoch before world/slot publication. No such binding exists here. */
typedef struct {AfDomain source,current;AgU32 target_epoch,rollback_epoch;} ArDomain;
typedef struct {AgU32 code,stage,mutation_started,owner_action,undo_bytes,pending_epoch,heaps,objects;} ArResult;
enum {AR_OK=0,AR_ARGUMENT=1,AR_SCOPE=2,AR_LAYOUT=3,AR_SPACE=5,AR_CORRUPT=6,
 AR_CHANGED=7,AR_BUSY=8,AR_ROLLED_BACK=9,AR_RECOVERY=10,AR_UNSUPPORTED=11};
enum {AR_OWNER_NONE=0,AR_OWNER_FINALIZE=1,AR_OWNER_RECOVER=2};
AgU32 _gz_ar_restore(const ArDomain*,const void*source,AgU32 bytes,
                     void*undo,AgU32 undo_capacity,ArResult*);
/* Same transaction with a fully validated, immutable prepared current AFS.
 * Compares all saved live bytes before mutation; does not write that image.
 * The caller retains allocation leases on both source and prepared undo. */
AgU32 _gz_ar_restore_prepared(const ArDomain*,const void*source,AgU32 bytes,
                              const void*undo,AgU32 undo_bytes,ArResult*);
extern volatile AgU32 _gz_ar_stage;
#endif
