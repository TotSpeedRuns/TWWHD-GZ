#ifndef GZ_PROCESS_BATCH_POLICY_H
#define GZ_PROCESS_BATCH_POLICY_H
#include "owned_heap_process.h"
/* Private query arithmetic ONLY. Real native adapter fills this cache from
 * actual locked HL/HM and original obt_scope/parent-generation checks. A host
 * fixture or a successful comparison supplies no native ownership grant. */
typedef struct {u32 parent,start,bytes,depth,status;} GhpbHeap;
enum {GHPB_SCOPE_MASK=3,GHPB_VALID=1,GHPB_INVALID=2,GHPB_BUDGET=3,
 GHPB_PARENT_LOADED=4,GHPB_BAD_PARENT=0xffffffffu,
 GHPB_NONE=0,GHPB_CANDIDATE=1,GHPB_UNRESOLVED=3,
 GHPB_KEEP=1,GHPB_TAKE=2,GHPB_AMBIGUOUS=3,GHPB_UNKNOWN=4};
typedef struct {u32 magic,bytes,capacity,queries,metadata_used,private_steps,phase,flags;} GhpbWork;
typedef char GhpbHeap20[sizeof(GhpbHeap)==20?1:-1];
typedef char GhpbWork32[sizeof(GhpbWork)==32?1:-1];
/* Same arithmetic as original within(): validate the query extent only. */
static inline u32 ghpb_contains(u32 p,u32 n,u32 q,u32 m){return !(q&3u)&&q>=0x10000000u&&m&&m<=0xffffffffu-q&&q>=p&&q-p<n&&m<=n-(q-p);}
static inline u32 ghpb_descends(const GhpbHeap*h,u32 child,u32 ancestor,u32 outer,u32 bytes,u32*steps){u32 i;
 if(!child||child>HL_HEAPS||!ancestor||ancestor>HL_HEAPS||
  !ghpb_contains(outer,bytes,h[child-1].start,h[child-1].bytes))return 0;
 for(i=0;i<32;i++){
  u32 next;const GhpbHeap*c=h+child-1;++*steps;
  if(!(c->status&GHPB_PARENT_LOADED)||c->parent==GHPB_BAD_PARENT)return 0;
  next=c->parent;if(!next||next>HL_HEAPS)return 0;
  if(next==ancestor)return 1;child=next;
 }return 0;}
static inline u32 ghpb_choose(const GhpbHeap*h,u32 current,u32 previous,u32 pointer,u32 capacity,
 u32 old_pointer,u32 old_capacity,u32*steps){
 if(!current||current>HL_HEAPS||(h[current-1].status&GHPB_SCOPE_MASK)!=GHPB_VALID)return GHPB_UNKNOWN;
 if(!previous)return GHPB_TAKE;
 if(previous>HL_HEAPS||(h[previous-1].status&GHPB_SCOPE_MASK)!=GHPB_VALID)return GHPB_UNKNOWN;
 if(current==previous)return GHPB_AMBIGUOUS;
 if(ghpb_descends(h,current,previous,old_pointer,old_capacity,steps))return GHPB_TAKE;
 if(ghpb_descends(h,previous,current,pointer,capacity,steps))return GHPB_KEEP;
 return GHPB_AMBIGUOUS;}
/* These searches inspect PRIVATE captured rows/indices only. In particular
 * a wrapping outer end includes the remaining candidates so corrupt extents
 * cannot silently hide an overlapping candidate. Native within() still
 * decides whether the whole query is enclosed. */
static inline u32 ghpb_order_lower(const GhcProcessRow*r,const u32*order,u32 count,u32 value,u32*steps){u32 lo=0,hi=count;
 while(lo<hi){u32 mid=lo+(hi-lo)/2;++*steps;
  if(r[order[mid]].process.object<value)lo=mid+1;else hi=mid;}return lo;}
static inline u32 ghpb_order_end(const GhcProcessRow*r,const u32*order,u32 count,u32 p,u32 n,u32*steps){
 return n>0xffffffffu-p?count:ghpb_order_lower(r,order,count,p+n,steps);}
/* Four byte passes are stable, so the canonical result returns to order. */
static inline void ghpb_order_sort(const GhcProcessRow*r,u32*order,u32*other,u32*bins,u32 count,u32*steps){u32 pass,i,sum;
 for(pass=0;pass<4;pass++){
  for(i=0;i<256;i++)bins[i]=0;
  for(i=0;i<count;i++){++bins[(r[order[i]].process.object>>(pass*8))&255u];++*steps;}
  sum=0;for(i=0;i<256;i++){u32 n=bins[i];bins[i]=sum;sum+=n;++*steps;}
  for(i=0;i<count;i++){u32 k=(r[order[i]].process.object>>(pass*8))&255u;other[bins[k]++]=order[i];++*steps;}
  {u32*t=order;order=other;other=t;}
 }}
#endif
