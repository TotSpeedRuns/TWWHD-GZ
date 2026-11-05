#ifndef GZ_NATIVE_HEAP_LIFETIME_H
#define GZ_NATIVE_HEAP_LIFETIME_H
typedef unsigned int u32;
enum {HL_MAGIC=0x484c4231u,HL_HEAPS=4096,HL_ALLOCS=262144,HL_EXP=1,HL_SOLID=2,HL_UNIT=3,HL_CUSTOM=4};
enum {HL_LIVE=1,HL_ENDING=2,HL_DEAD=3,HL_MUTATING=4};
enum {HL_BAD_HEADER=1,HL_REENTRY=2,HL_BAD_ARGUMENT=4,HL_OVERFLOW=8,
 HL_EXHAUSTED=16,HL_REPLACED=32,HL_UNKNOWN_HEAP=64,HL_UNKNOWN_ALLOCATION=128,
 HL_BAD_BLOCK=256,HL_STALE_COMPLETION=512,HL_COUNTER=1024,HL_ORIGIN=2048};
enum {HL_HEAP_RETIRED=1,HL_OBSERVED_RELEASE=2,HL_UNBOUND_HEAP=4,HL_REBORN=8};
enum {HL_MISSING_HOLD=1,HL_MISSING_REFERENCES=2,HL_MISSING_SESSION=4,
 HL_MISSING_MUTATORS=8,HL_MISSING_ALLOCATOR_CERTIFICATE=16,HL_MISSING_ALL=31};
typedef struct {
 u32 heap,generation,state,kind,start,bytes,parent,parent_generation;
 u32 vtable,direction,born_frame,end_frame,ctor_site,dtor_site,flags,epoch;
} HlHeap;
typedef struct {
 u32 pointer,generation,state,kind,heap,heap_generation,requested,alignment;
 u32 block,capacity,padding,born_frame,end_frame,birth_site,end_site,release_token;
 u32 flags,heap_epoch,missing,reserved;
} HlAllocation;
typedef struct {
 u32 magic,bytes,version,busy,flags,serial,release_serial,active_heaps,active_allocations;
 u32 births,allocations,release_begin,release_end,heap_end,dropped,session;
} HlControl;
typedef struct {u32 valid,result,flags,session;HlAllocation allocation;HlHeap heap;} HlObservation;
typedef char HlHeapSize[(sizeof(HlHeap)==64)?1:-1];
typedef char HlAllocationSize[(sizeof(HlAllocation)==80)?1:-1];
typedef char HlControlSize[(sizeof(HlControl)==64)?1:-1];
typedef char HlObservationSize[(sizeof(HlObservation)==160)?1:-1];
extern HlControl _gz_hl_control;
/* Scheduler-safe common metadata lock; no allocator or wait inside. */
u32 _gz_hl_meta_try_lock(void);
void _gz_hl_meta_unlock(void);
extern HlHeap _gz_hl_heaps[HL_HEAPS];
/* Stored rows are private; callers must never assume an80byte stride.
 * The two projection slots are module-owned output only. A caller must hold
 * the common metadata lock continuously while requesting/reading either. */
extern HlAllocation _gz_hl_projected[2];
u32 _gz_hl_project_locked(u32 heap,u32 pointer,u32 slot);
extern HlObservation _gz_hl_observation;
extern volatile u32 _gz_hl_frame;
extern const unsigned char _gz_hl_exp_table[],_gz_hl_solid_table[],_gz_hl_unit_table[],_gz_hl_custom_table[];
extern const unsigned char _gz_hl_exp_alloc_site[],_gz_hl_solid_alloc_site[],_gz_hl_free_accepted_return[],_gz_hl_unit_alloc_site[],_gz_hl_unit_free_site[],_gz_hl_custom_alloc_site[],_gz_hl_custom_free_site[];
/* Exact native constructor returns and allocator success epilogues only.
 * Unit free-list and initialize/reset lifetimes are included.
 * Allocator bodies are never copied. Known fresh return/header metadata is
 * bounded by an observed LIVE heap before any padded block-header read.
 * All heap generations are observations within this boot, not physical
 * allocator/held-boundary/session/reverse-reference certificates. */
void _gz_hl_heap_born(u32 heap,u32 kind,u32 site);
void _gz_hl_heap_ending(u32 heap,u32 kind,u32 site);
void _gz_hl_allocated(u32 heap,u32 pointer,u32 bytes,u32 alignment,u32 kind);
void _gz_hl_custom_allocated(u32 heap,u32 pointer,u32 bytes,u32 alignment,u32 descriptor);
/* Custom heap's descriptor table is a separate native storage anchor. These
 * fields are metadata observations, not an allocator/world certificate. */
typedef struct {u32 heap_generation,table,slots,anchor_pointer,anchor_generation;} HlCustomMeta;
extern HlCustomMeta _gz_hl_custom_meta[HL_HEAPS];
/* Accepted ExpHeap used-list removal only, not ordinary free entry. The real
 * free wrapper carries this exact cookie in its OWN stack frame across the
 * native operation. A no-op/null/foreign/disposing free produces cookie0.
 * End paths never dereference allocation/heap memory, which may be unmapped. */
u32 _gz_hl_release_begin(u32 heap,u32 block,u32 pointer,u32 site,u32 origin);
void _gz_hl_release_return(u32 heap,u32 pointer,u32 cookie);
/* Fixed private metadata query. No native pointer dereferences or permission
 * grant: valid results still carry missing31. Unobserved reset/resize/custom
 * allocators remain explicit required coverage; ordinary admission unchanged. */
u32 _gz_hl_lookup(u32 pointer);
/* Pair-qualified metadata inspection; unqualified lookup refuses ambiguous
 * identical pointers from distinct parent/child heaps. No restore authority. */
u32 _gz_hl_lookup_heap(u32 heap,u32 pointer);
/* Exact Custom descriptor qualifier; generic lookup refuses aliased same-heap
 * pointers (zero-size extents can share the next allocation's user address). */
u32 _gz_hl_lookup_custom(u32 heap,u32 pointer,u32 descriptor);
/* Interior extent query over observed allocations only. Overlapping parent/
 * child allocations are resolved solely through exact observed LIVE heap
 * parent generations and confinement; size/order never decides ownership.
 * Any observation gap, unrelated overlap or missing parent stamp denies a
 * valid owner. A valid observation still retains missing31 and session0. */
u32 _gz_hl_owner(u32 pointer,u32 bytes);
/* Side-index diagnostics, under the same lock. Full audit is deliberately
 * explicit; normal per-heap walks visit only that heap's LIVE/ENDING rows.
 * Neither API grants native writer exclusion or restore authority. */
u32 _gz_hi_check_heap(u32 heap);
u32 _gz_hi_audit(void);
enum {HM_MAGIC=0x484d5531u,HM_CAP=256,HM_EXP_RESET=1,HM_SOLID_RESET=2,
 HM_EXP_FRONT=3,HM_EXP_BACK=4,HM_SOLID_RESTORE=5,HM_SOLID_FRONT=6,
 HM_SOLID_BACK=7,HM_EXP_ADJUST_FRONT=8,HM_EXP_ADJUST_BACK=9,HM_UNIT_INITIALIZE=10,HM_UNIT_RESET=11,HM_CUSTOM_RESET=12,HM_CUSTOM_FRONT=13,HM_CUSTOM_BACK=14};
enum {HM_RESIZED=16,HM_ARENA_RETIRED=32,HM_BORN_DURING=64,HM_PARTIAL_RETIRED=128};
typedef struct {
 u32 parent_epoch,managed_pointer,managed_generation,header_pointer,header_generation,active_token,flags,reserved;
} HmScope;
typedef struct {
 u32 token,operation,state,heap,heap_generation,old_epoch,pointer,request,site,frame;
 u32 old_start,old_bytes,old_head,old_tail,have_old,returned;
 HlAllocation old;
} HmRecord;
typedef struct {
 u32 magic,bytes,version,flags,attempts,completed,failed,active;
 u32 last_token,last_heap,last_operation,last_result,retired,changed,noops,reserved;
} HmControl;
typedef char HmScopeSize[(sizeof(HmScope)==32)?1:-1];
typedef char HmRecordSize[(sizeof(HmRecord)==144)?1:-1];
extern HmScope _gz_hm_scopes[HL_HEAPS];
extern HmRecord _gz_hm_records[HM_CAP];
extern HmControl _gz_hm_control;
/* Internal fixed metadata, called only under the common observer lock. */
void _gz_hm_bind_heap(HlHeap*heap);
u32 _gz_hm_scope_live(HlHeap*heap);
/* Native entry/return wrappers own the exact token in their stack. No native
 * function is skipped on observer failure, and stale returns cannot revive
 * newer generations. All existing missing31 authority remains required. */
u32 _gz_hm_begin(u32 heap,u32 operation,u32 pointer,u32 request,u32 site);
void _gz_hm_finish(u32 token,u32 returned);
#endif
