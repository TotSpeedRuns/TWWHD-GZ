#ifndef GZ_RENDER_LIFETIME_WITNESS_H
#define GZ_RENDER_LIFETIME_WITNESS_H
#include <stdint.h>
#include "../render-current-graph-preflight-r1/render_graph.h"
typedef uint32_t RclU;
enum {RCL_MAGIC=0x524c5731u,RCL_VERSION=1,RCL_QUEUES=24,RCL_ALLOCS=48,RCL_HEAPS=64,RCL_DEPTH=32};
/* Exactly nine stable native allocation words, never public HlAllocation80.
 * LIVE, flags(nonprojected), missing31, reserved0, release/end fields and full
 * heap/backing scope are revalidated when this tuple is exported/rechecked. */
typedef struct {RclU pointer,generation,heap,heap_generation,heap_epoch,requested,capacity,block,kind;} RclAllocation;
typedef struct {RclU count,array,used;} RclQueue;
typedef struct {RclU heap[16],scope[8];} RclHeap;
typedef struct {
 RclU magic,version,bytes,valid,request,frame,hg,owner;
 RclU initialized,j3d_opa,j3d_xlu,heap_count,allocation_count,queue_count,raw,raw_bytes;
 RclQueue queues[RCL_QUEUES];
 /* Native owner reset order. allocations[2*i] is buffer; [2*i+1] its array.
  * Buffer pointers are allocations[2*i].pointer; queue.array is the paired
  * array allocation.pointer. No transient matrix/sort body words are sealed. */
 RclAllocation allocations[RCL_ALLOCS];
 RclHeap heaps[RCL_HEAPS];
} RclWitness;
typedef char RclAllocation36[sizeof(RclAllocation)==36?1:-1];
typedef char RclQueue12[sizeof(RclQueue)==12?1:-1];
typedef char RclHeap96[sizeof(RclHeap)==96?1:-1];
typedef char RclWitness8224[sizeof(RclWitness)==8224?1:-1];
/* Defined by the private normalization registry; fixed owner, never passed as
 * a caller-selected write pointer. It must be excluded from every replay. */
extern RclWitness _gz_rcl_pending;
/* Requires a private actual-reset/order receipt from the coordinator, not a
 * caller boolean. Genuine OISphase2/writer4 scratch is reused sequentially. */
RclU _gz_rcl_export(RclU request,RclU frame,RclU scratch,RclU bytes);
#endif
