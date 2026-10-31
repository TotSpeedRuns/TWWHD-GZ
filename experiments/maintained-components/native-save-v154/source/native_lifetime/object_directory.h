#ifndef GZ_OBJECT_DIRECTORY_H
#define GZ_OBJECT_DIRECTORY_H
#include "lifetime.h"
#include "object_binding_try.h"

/* Current PRIVATE metadata directory, not a heap image, lease, native read
 * permission or certification of each parent's physical backing. The caller
 * owns aligned high-DATA output, excludes other writers, and uses it only
 * after OK. Every failure leaves all output bytes unchanged. No allocation
 * or callback occurs here. Capacity includes header and both row arrays.
 */
enum {
    OB_SPACE = 8, OB_DIRECTORY_MAGIC = 0x4f444231u,
    OB_DIRECTORY_FULL = 0, OB_DIRECTORY_MEASURE = 1,
    OB_DIRECTORY_WORK_LIMIT = 1048576,
    OB_DIRECTORY_MAX_BYTES = 64 + HL_HEAPS * 80 + HL_ALLOCS * 96
};
typedef struct {
    u32 magic, bytes, version, heap_count, allocation_count, heap_row_bytes, allocation_row_bytes, serial;
    u32 release_serial, frame, active_heaps, active_allocations, mode, missing, session, reserved;
} ObDirectoryHeader;
typedef struct { u32 index; HlHeap heap; u32 reserved[3]; } ObDirectoryHeap;
typedef struct { u32 index, heap_index; HlAllocation allocation; u32 reserved[2]; } ObDirectoryAllocation;
typedef char ObDirectoryHeaderSize[sizeof(ObDirectoryHeader) == 64 ? 1 : -1];
typedef char ObDirectoryHeapSize[sizeof(ObDirectoryHeap) == 80 ? 1 : -1];
typedef char ObDirectoryAllocationSize[sizeof(ObDirectoryAllocation) == 96 ? 1 : -1];

/* Layout: header64, heap_count * heap_row80, allocation_count * alloc_row96.
 * Heap rows are in ascending private slot order. Allocation rows follow the
 * validated list order within each heap. Row indices are metadata identities
 * only. Missing remains31, session0; ENDING rows do not become live leases.
 */
ObU32 _gz_ob_try_directory(void *output, ObU32 capacity);
/* Same census and policies, but publishes ONLY header64 with mode=MEASURE.
 * bytes is the full required size, not the number of bytes written. It is an
 * advisory footprint at this observation, never a reservation or row array.
 * Release actual holds before allocating storage; full capture checks again
 * and returns SPACE without stores if the observed directory has grown.
 */
ObU32 _gz_ob_try_directory_measure(ObDirectoryHeader *output);
#endif
