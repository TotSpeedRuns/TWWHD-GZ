#ifndef GZ_SAVE_PARENT_BACKING_OBSERVER_H
#define GZ_SAVE_PARENT_BACKING_OBSERVER_H
#include "observation.h"
typedef SroU32 SpbU32;
enum { SPB_OK=0,SPB_NO_CHANGE=1,SPB_BUSY=5,SPB_DENIED=28,
 SPB_INVALID=70,SPB_CHANGED=71,SPB_EXHAUSTED=74,SPB_CORRUPT=75 };
enum { SPB_EMPTY=0,SPB_PARENT=1,SPB_BORN=2,SPB_RETURNED=3,SPB_BOUND=4,SPB_RETIRED=5 };
enum { SPB_OUTER_PUBLISH=1,SPB_BOOT_BEGIN=2,SPB_FACTORY_POST=3,SPB_BOOT_RETURN=4,SPB_ROOT_BIND=5,
 SPB_DIRECT_FREE=6,SPB_TAIL_SHRINK=7,SPB_FRONT_SHRINK=8,SPB_CHILD_RANGE=9,
 SPB_CHILD_FULL=10,SPB_CHILD_DESTROY=11,SPB_PARENT_FULL=12,SPB_PARENT_DESTROY=13,SPB_BIRTH_MISMATCH=14 };
enum { SPB_MAGIC=0x53504231u,SPB_VERSION=1,SPB_BYTES=128,SPB_CHILD_BYTES=156 };
/* These are linker aliases into the native module, not allocated objects.
 * Their addresses must receive the module's actual TEXT/DATA relocations. */
#include "native_addresses.h"
#define SPB_ROOT_SLOT ((SpbU32)_gz_spo_native_root_slot)
/* All addresses are observed INTEGER VALUES. Neither this record nor0-return
 * authorizes a native dereference, allocation, source read, world mutation,
 * thread claim, mapping or lifetime pin. Event origin is unauthenticated here.
 * A separately reviewed nonyielding fixed hook must establish the exact native
 * event and supply its genuine scalars/retained prior epochs. Caller booleans,
 * scalar equality and a valid arithmetic interval cannot establish that origin.
 * Epochs are private monotonic observation counters, not native allocator IDs.
 * No reset exists. Matching retirement never increments/exhausts a counter.
 */
typedef struct {
 SpbU32 magic,version,bytes,phase;
 SpbU32 parent_epoch,parent,backing_epoch,child;
 SpbU32 user,backing_bytes,direction,root;
 SpbU32 root_generation,root_bytes,last_event,retire_reason;
 SpbU32 scope_serial,initial_backing_bytes,backing_start,backing_end;
 SpbU32 parent_live,parent_retire_reason,root_bound,backing_live;
 SpbU32 reserved[8];
} SpbValue;
typedef char SpbSize[(sizeof(SpbValue)==SPB_BYTES)?1:-1];
/* Fixed future seams: outer publish0203E710; bootpre0203F454 (r5 parent);
 * factory direction1 success027554EC (r31child/r27bytes/r29parent/r30direction);
 * bootreturn0203F458 (r3child). No generic factory event can issue a birth
 * without the separately issued exact boot scope and matching return.
 * These are source-only callback contracts; NO hook is installed by this code.
 */
SpbU32 _gz_spb_outer_publish(SpbU32 parent,SpbU32 expected_parent_epoch);
SpbU32 _gz_spb_boot_begin(SpbU32 parent,SpbU32 parent_epoch);
SpbU32 _gz_spb_factory_post(SpbU32 child,SpbU32 bytes,SpbU32 parent,SpbU32 direction);
SpbU32 _gz_spb_boot_return(SpbU32 child);
/* Called after actual included SRW ctor-note success; generation/heap/root are
 * read from that one private SRO record, never issued from caller completion.
 */
SpbU32 _gz_spb_root_bind(SpbU32 root,SpbU32 child);
/* Premutation event-only APIs: no success/admitted boolean, native header read,
 * guessed USER-16 pointer or containing-heap query. Exact private parent/USER
 * and epochs select the backing. Tail excludes native nonsplit/no-op(<16byte)
 * cases; front USER movement invalidates child identity even padding-only.
 */
SpbU32 _gz_spb_direct_free(SpbU32 parent,SpbU32 user,SpbU32 parent_epoch,SpbU32 backing_epoch);
SpbU32 _gz_spb_tail_shrink(SpbU32 parent,SpbU32 user,SpbU32 old_bytes,SpbU32 new_bytes,SpbU32 parent_epoch,SpbU32 backing_epoch);
SpbU32 _gz_spb_front_shrink(SpbU32 parent,SpbU32 user,SpbU32 new_user,SpbU32 old_bytes,SpbU32 new_bytes,SpbU32 parent_epoch,SpbU32 backing_epoch);
SpbU32 _gz_spb_child_range(SpbU32 child,SpbU32 lower,SpbU32 upper,SpbU32 parent_epoch,SpbU32 backing_epoch);
/* Distinguish child cursor reset(root invalid/backing kept), child destroy,
 * parent full pool reset(backing invalid/parent kept), and parent destroy.
 * A generic matching walker invocation is deliberately not an event here.
 */
SpbU32 _gz_spb_scope_retire(SpbU32 event,SpbU32 heap,SpbU32 parent_epoch,SpbU32 backing_epoch);
/* out must actually be a mapped caller-owned writable128-byte object. Numeric
 * span/alignment/private-output guards are only shape checks. Failure leaves it
 * unchanged. Success copies values after actual SRW boot-query, no readable
 * native lease or permission. Same-address old epochs/generation refuse.
 */
SpbU32 _gz_spb_query(SpbU32 root,SpbU32 generation,SpbU32 parent_epoch,SpbU32 backing_epoch,SpbValue*out);
SpbU32 _gz_spb_native_admit(void);
#endif
