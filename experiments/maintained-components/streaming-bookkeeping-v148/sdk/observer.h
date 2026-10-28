#ifndef GZ_SDK_OBSERVER_H
#define GZ_SDK_OBSERVER_H
typedef unsigned int u32;
enum {SK_MAGIC=0x534b4f31u,SK_ALLOCATIONS=4096,SK_PENDING=128,SK_HEAPS=256,SK_MISSING_ALL=31};
enum {SK_BAD_HEADER=1,SK_REENTRY=2,SK_BAD_SITE=4,SK_CAPACITY=8,SK_EXHAUSTED=16,SK_STALE=32,SK_RANGE=64,SK_DUPLICATE=128};
enum {SK_ALLOC_EXP=1,SK_ALLOC_FRM=2,SK_ALLOC_DEFAULT=3,SK_ALLOC_DEFAULT_EX=4,SK_FREE_EXP=5,SK_FREE_DEFAULT=6,SK_CREATE_EXP=7,SK_CREATE_FRM=8,SK_DESTROY_EXP=9,SK_DESTROY_FRM=10};
typedef struct {u32 magic,bytes,version,busy,flags,serial,active,pending,births,retired,attempts,dropped,missing,active_heaps,heap_births,heap_retired;} SkControl;
typedef struct {u32 pointer,bytes,generation,live,kind,heap,site,frame,target,alignment,retire_site,heap_generation;} SkAllocation;
typedef struct {u32 serial,live,site,kind,a0,a1,a2,target;} SkOperation;
typedef struct {u32 valid,result,missing,reserved;SkAllocation allocation;} SkObservation;
enum {SK_BACKING_UNKNOWN=1,SK_BACKING_AMBIGUOUS=2};
typedef struct {u32 heap,generation,live,kind,buffer,bytes,site,frame,backing_heap,backing_pointer,backing_generation,flags,retire_site,reserved[3];} SkHeap;
typedef struct {u32 valid,result,missing,reserved;SkHeap heap;} SkHeapObservation;
typedef struct {u32 site,kind;} SkSite;
typedef char SkControlSize[(sizeof(SkControl)==64)?1:-1];
typedef char SkAllocationSize[(sizeof(SkAllocation)==48)?1:-1];
typedef char SkOperationSize[(sizeof(SkOperation)==32)?1:-1];
typedef char SkObservationSize[(sizeof(SkObservation)==64)?1:-1];
typedef char SkHeapSize[(sizeof(SkHeap)==64)?1:-1];
typedef char SkHeapObservationSize[(sizeof(SkHeapObservation)==80)?1:-1];
extern SkControl _gz_sk_control;
extern SkAllocation _gz_sk_allocations[SK_ALLOCATIONS];
extern SkOperation _gz_sk_operations[SK_PENDING];
extern SkObservation _gz_sk_observation;
extern SkHeap _gz_sk_heaps[SK_HEAPS];
extern SkHeapObservation _gz_sk_heap_observation;
extern volatile u32 _gz_sk_frame;
extern const SkSite _gz_sk_sites[37];
/* Only wrapper-saved scalar arguments/results are observed. These routines
 * never dereference a native heap, allocation, API target or archive pointer.
 * Retiring before a free/destroy call is deliberately conservative: a native
 * no-op may invalidate metadata, but cannot make retired bytes authoritative.
 * No source mapping, complete SDK coverage, held boundary, reference or
 * cross-boot identity is granted by these observations (missing remains31). */
/* Pending allocations overlapping any observed free/destroy are dropped;
 * this conservative rule prevents a delayed post-call observer from reviving
 * an already released result. The original SDK call still runs normally. */
u32 _gz_sk_begin(u32 site,u32 a0,u32 a1,u32 a2,u32 target);
void _gz_sk_finish(u32 serial,u32 returned);
u32 _gz_sk_lookup(u32 heap,u32 pointer);
u32 _gz_sk_heap_lookup(u32 heap);
#endif
