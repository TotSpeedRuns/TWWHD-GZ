#ifndef GZ_LIFETIME_STORAGE_H
#define GZ_LIFETIME_STORAGE_H
/* Private60-byte fixed-PPC storage only. Public HlAllocation remains80bytes. The
 * emitted target layout is independently tested; no C bitfield wire ABI is
 * exposed. No pointer/generation/frame/request/alignment word is narrowed. */
typedef struct {
 u32 pointer,generation,heap,heap_generation,requested,alignment;
 u32 block,capacity,born_frame,end_frame,birth_site,end_site,release_token,heap_epoch;
 union {u32 policy;struct {u32 state:3,kind:3,flags:9,missing:5,reserved:12;};};
} HlStoredAllocation;
typedef char HlStoredAllocationSize[sizeof(HlStoredAllocation)==60?1:-1];
extern HlStoredAllocation _gz_hl_allocations[HL_ALLOCS];
/* Every accepted producer validates this exact Exp relation. Unknown-heap
 * observations retain block0/padding0; Custom keeps its full descriptor block
 * word, including same-pointer aliases. No public word is narrowed. */
static u32 hl_padding(const HlStoredAllocation*s){
 return s->kind==HL_EXP&&s->block?s->pointer-s->block-16u:0;
}
static void hl_export(HlAllocation*d,const HlStoredAllocation*s){
 d->pointer=s->pointer;d->generation=s->generation;d->state=s->state;d->kind=s->kind;
 d->heap=s->heap;d->heap_generation=s->heap_generation;d->requested=s->requested;d->alignment=s->alignment;
 d->block=s->block;d->capacity=s->capacity;d->padding=hl_padding(s);d->born_frame=s->born_frame;
 d->end_frame=s->end_frame;d->birth_site=s->birth_site;d->end_site=s->end_site;d->release_token=s->release_token;
 d->flags=s->flags;d->heap_epoch=s->heap_epoch;d->missing=s->missing;d->reserved=s->reserved;
}
static u32 hl_import(HlStoredAllocation*d,const HlAllocation*s){HlStoredAllocation n;
 /* Public/archive rows are validated before any narrowing or destination
  * write. In particular provenance256 and all eight lower flag bits survive. */
 if(s->state>7||s->kind>7||s->flags>511||s->missing>31||s->reserved)return 0;
 /* Rebind already proves this relation before mutation; the other import
  * callers pass exact prior exports. Refuse an unrepresentable private input
  * before touching its destination rather than truncate its public padding. */
 if(s->kind==HL_EXP&&s->block){
  if(s->block>s->pointer||s->pointer-s->block<16u||s->padding!=s->pointer-s->block-16u)return 0;
 }else if(s->padding)return 0;
 n.pointer=s->pointer;n.generation=s->generation;n.heap=s->heap;n.heap_generation=s->heap_generation;
 n.requested=s->requested;n.alignment=s->alignment;n.block=s->block;n.capacity=s->capacity;
 n.born_frame=s->born_frame;n.end_frame=s->end_frame;n.birth_site=s->birth_site;n.end_site=s->end_site;
 n.release_token=s->release_token;n.heap_epoch=s->heap_epoch;n.policy=0;
 n.state=s->state;n.kind=s->kind;n.flags=s->flags;n.missing=s->missing;
 for(u32 i=0;i<15;i++)((u32*)d)[i]=((const u32*)&n)[i];return 1;
}
#endif
