#ifndef GZ_ALLOCATED_RUN_POLICY_H
#define GZ_ALLOCATED_RUN_POLICY_H
#include "lifetime.h"
#include "lifetime_storage.h"
/* Pure metadata arithmetic. A true result alone is never native authority. */
typedef struct { u32 heap,generation,start,bytes; } GarRootSpan;
static inline u32 gz_gar_contains(u32 p,u32 n,u32 q,u32 m)
{ return n&&m&&n<=0xffffffffu-p&&m<=0xffffffffu-q&&q>=p&&q-p<=n&&m<=n-(q-p); }
static inline u32 gz_gar_page_room(u32 visited,u32 heap_advances,u32 emitted)
{ return visited<64u&&heap_advances<64u&&emitted<64u; }
static inline u32 gz_gar_heap_policy(const HlHeap*h,const GarRootSpan*r,u32 table,u32 header)
{
 if(h->state!=HL_LIVE||!h->generation||h->kind<HL_EXP||h->kind>HL_CUSTOM||
    !table||h->vtable!=table||!header||!h->heap||(h->heap&3u)||
    h->heap<0x10000000u||h->heap>=0xc0000000u||header>0xc0000000u-h->heap||
    h->start<0x10000000u||(h->start&3u)||!h->bytes||h->bytes>0xffffffffu-h->start||
    (h->flags&~HL_REBORN)||h->end_frame||h->dtor_site||
    (h->direction!=1u&&(h->direction!=0xffffffffu||h->kind==HL_CUSTOM))||
    r->heap<0x10000000u||(r->heap&3u)||!r->generation||r->start<0x10000000u||
    r->start>=0xc0000000u||!r->bytes||r->bytes>0xc0000000u-r->start)return 0;
 if(h->heap==r->heap)
  return h->generation==r->generation&&h->start==r->start&&h->bytes==r->bytes&&
         !h->parent&&!h->parent_generation;
 return h->parent&&h->parent_generation&&
        gz_gar_contains(r->start,r->bytes,h->heap,header)&&
        gz_gar_contains(r->start,r->bytes,h->start,h->bytes);
}
static inline u32 gz_gar_allocation_policy(const HlStoredAllocation*a,const HlHeap*h)
{
 return a->state==HL_LIVE&&a->generation&&a->heap==h->heap&&
  a->heap_generation==h->generation&&a->heap_epoch==h->epoch&&a->kind==h->kind&&
  !a->reserved&&a->missing==31&&!a->release_token&&!a->end_frame&&!a->end_site&&
  (!(a->flags&~(HM_BORN_DURING|256u))||a->flags==HM_RESIZED)&&
  (!a->capacity||(gz_gar_contains(h->start,h->bytes,a->pointer,a->capacity)&&
   (a->kind!=HL_EXP||(a->block&&a->block<=a->pointer&&a->pointer-a->block>=16u&&
     gz_gar_contains(h->start,h->bytes,a->block,16)))));
}
#endif
