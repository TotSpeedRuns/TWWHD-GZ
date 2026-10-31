#ifndef GZ_HEAP_LIFETIME_REBIND_H
#define GZ_HEAP_LIFETIME_REBIND_H
typedef unsigned int HrU32;
enum {HR_OK=0,HR_ARGUMENT=1,HR_OBSERVER=2,HR_LAYOUT=3,HR_LIMIT=4,HR_SPACE=5,
 HR_CORRUPT=6,HR_CHANGED=7,HR_EXHAUSTED=8,HR_DIRTY=9,HR_MAX_ALLOCS=128,HR_MAX_BLOCKS=256};
/* source identities are immutable. New identities always consume the live
 * monotonic observer counters; source counters are never restored. */
typedef struct {HrU32 heap,pointer,source_generation,source_heap_generation,source_epoch,
 generation,heap_generation,epoch;} HrMap;
typedef struct {HrU32 code,mutated,mapped,source_heap_generation,heap_generation,
 old_epoch,epoch,serial_before,serial_after,missing,valid,reserved;} HrReceipt;
/* Metadata-only internal service. These APIs neither acquire native writer
 * exclusion nor grant an actor/resource/allocator certificate. Every result
 * retains missing31/session0; an outer held owner must authenticate source,
 * sequence parent-before-child, reconcile references and publish the world.
 * Native heap storage/allocator bodies must already match the saved image.
 * Custom/GPU-backed heaps require their own provider and are refused here. */
HrU32 _gz_hr_capture(HrU32 heap,void*image,HrU32 capacity,HrU32*written);
HrU32 _gz_hr_validate(const void*image,HrU32 bytes);
/* Metadata-only consistency stamp: serial, release serial, live heap count,
 * live allocation count. Success does not certify native writer exclusion. */
HrU32 _gz_hr_stamp(HrU32 output[4]);
HrU32 _gz_hr_buffer_private(const void*buffer,HrU32 bytes);
/* Locked metadata classification only. The buffer must already be mapped
 * and owned by its allocator/caller. No future world exclusion or ownership
 * is manufactured by being outside currently observed heap ranges. */
HrU32 _gz_hr_storage_private(const void*buffer,HrU32 bytes);
/* Current observer identity only, deliberately not scope-live: a parent's
 * rebind may have invalidated a child's cache until its own turn. */
HrU32 _gz_hr_heap_identity(HrU32 heap,HrU32 output[2]);
HrU32 _gz_hr_rebind(const void*image,HrU32 bytes,HrU32 current_generation,HrU32 current_epoch,
 HrMap*map,HrU32 map_capacity,HrReceipt*receipt);
#endif
