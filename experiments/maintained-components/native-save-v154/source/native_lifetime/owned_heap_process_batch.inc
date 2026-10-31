/* Included after genuine PO/cut helpers. One validated native HI/HL census,
 * sorted PRIVATE PO queries, exact old scope/parent semantics. No native
 * body pointers are followed; all output is unsealed OIS-owned staging. */
#include "owned_heap_process_batch_policy.h"
GHC_FN GhpbHeap*ghpb_heaps(GhpbWork*b){return(GhpbHeap*)((u32)b+32+1024);}
GHC_FN u32*ghpb_buckets(GhpbWork*b){return(u32*)((u32)b+32);}
GHC_FN u32*ghpb_a(GhpbWork*b){return(u32*)((u32)ghpb_heaps(b)+HL_HEAPS*sizeof(GhpbHeap));}
GHC_FN u32*ghpb_b(GhpbWork*b){return ghpb_a(b)+b->capacity;}
/* First matching scalar is charged even when a prior conflict already made
 * the query unresolved. The original1M metadata/visit cap is not raised. */
GHC_FN u32 ghpb_charge(ObTryWork*w){return obt_take(w);}
GHC_FN u32 ghpb_lower(GhpbWork*b,const GhcProcessRow*r,u32 value){
 return ghpb_order_lower(r,ghpb_a(b),b->queries,value,&b->private_steps);}
GHC_FN void ghpb_sort(GhpbWork*b,const GhcProcessRow*r){
 ghpb_order_sort(r,ghpb_a(b),ghpb_b(b),ghpb_buckets(b),b->queries,&b->private_steps);}
GHC_FN u32 ghpb_parent(GhpbWork*b,u32 slot,ObTryWork*w){GhpbHeap*c;HlHeap*h,*parent;
 if(!slot||slot>HL_HEAPS)return 0;c=ghpb_heaps(b)+slot-1;
 if(c->status&GHPB_PARENT_LOADED)return c->parent!=GHPB_BAD_PARENT;
 if(!ghpb_charge(w))return 0;
 h=_gz_hl_heaps+slot-1;c->start=h->start;c->bytes=h->bytes;c->status|=GHPB_PARENT_LOADED;
 if(!h->parent){c->parent=0;return 1;}
 if(!h->parent_generation){c->parent=GHPB_BAD_PARENT;return 0;}
 parent=ghc_hl_heap(h->parent,w);
 if(!parent||w->exhausted||parent->state!=HL_LIVE||parent->generation!=h->parent_generation){c->parent=GHPB_BAD_PARENT;return 0;}
 c->parent=(u32)(parent-_gz_hl_heaps)+1;return 1;}
GHC_FN u32 ghpb_scope(GhpbWork*b,u32 slot,ObTryWork*w){GhpbHeap*c;u32 state,depth=0,at=slot,left=64;
 if(!slot||slot>HL_HEAPS)return 0;c=ghpb_heaps(b)+slot-1;state=c->status&GHPB_SCOPE_MASK;
 if(state)return state==GHPB_VALID;
 /* Retain the real resolver's exact scope/custom/backing semantics. This
  * runs ONCE per matching heap, never once per actor or allocation. */
 if(!ghc_hl_scope(_gz_hl_heaps+slot-1,0,&left,w))goto invalid;
 for(;;){GhpbHeap*cur;
  if(!at||at>HL_HEAPS||depth>=32||!ghpb_parent(b,at,w))goto invalid;
  cur=ghpb_heaps(b)+at-1;if(!cur->parent)break;at=cur->parent;++depth;++b->private_steps;
 }
 c->depth=depth;c->status=(c->status&~GHPB_SCOPE_MASK)|GHPB_VALID;return 1;
 invalid:c->status=(c->status&~GHPB_SCOPE_MASK)|(w->exhausted?GHPB_BUDGET:GHPB_INVALID);return 0;}
GHC_FN void ghpb_unknown(GhcProcessRow*r){r->binding=GHPB_UNRESOLVED;r->reserved=0;r->heap_index=0;
 zero(&r->allocation,sizeof(r->allocation));zero(&r->owner,sizeof(r->owner));zero(&r->scope,sizeof(r->scope));}
GHC_FN void ghpb_none(GhcProcessRow*r){r->binding=0;r->reserved=0;r->heap_index=0;
 zero(&r->allocation,sizeof(r->allocation));zero(&r->owner,sizeof(r->owner));zero(&r->scope,sizeof(r->scope));}
GHC_FN u32 ghpb_candidates(GhpbWork*b,GhcProcessRow*r,const HlStoredAllocation*a,u32 id,u32 slot,ObTryWork*w){
 u32 first,last,i,scope=0;const u32*order=ghpb_a(b);
 if(a->state!=HL_LIVE||!a->capacity)return 1;
 first=ghpb_lower(b,r,a->pointer);
 /* Match original within() even for an oversized/corrupt outer extent;
  * wrapping the range end must never hide a competing candidate. */
 last=ghpb_order_end(r,order,b->queries,a->pointer,a->capacity,&b->private_steps);
 for(i=first;i<last;i++){
  GhcProcessRow*q=r+order[i];u32 pick;
  ++b->private_steps;
  if(!ghpb_charge(w))return 0;
  if(!within(_gz_hl_heaps[slot-1].start,_gz_hl_heaps[slot-1].bytes,q->process.object,q->process.allocation_bytes)||
   !within(a->pointer,a->capacity,q->process.object,q->process.allocation_bytes))continue;
  if(q->binding==GHPB_UNRESOLVED)continue;
  if(!scope)scope=ghpb_scope(b,slot,w)?1u:2u;
  if(w->exhausted)return 0;
  if(scope!=1u){ghpb_unknown(q);continue;}
  /* obt_owner validates these before accepting ANY candidate. */
  if(_gz_hl_heaps[slot-1].state!=HL_LIVE||!a->heap_generation||a->heap_generation!=_gz_hl_heaps[slot-1].generation||
   a->heap_epoch!=_gz_hl_heaps[slot-1].epoch){ghpb_unknown(q);continue;}
  pick=ghpb_choose(ghpb_heaps(b),slot,q->heap_index,a->pointer,a->capacity,
   q->allocation.pointer,q->allocation.capacity,&b->private_steps);
  if(pick==GHPB_TAKE){q->binding=GHPB_CANDIDATE;q->reserved=id;q->heap_index=slot;
   q->allocation.pointer=a->pointer;q->allocation.capacity=a->capacity;}
  else if(pick!=GHPB_KEEP)ghpb_unknown(q);
 }
 return 1;}
/* Same validated HI-list shape as the original counted directory census,
 * combined with query matching in THIS traversal. No hi_ready full walk,
 * no per-actor4096heap scan, no owner/report success flag is trusted. */
GHC_FN u32 ghpb_census(GhpbWork*b,GhcProcessRow*r,ObTryWork*w){u32 i,j,node,previous,ac=0;HlHeap*h;HiHeap*index;HlStoredAllocation*a;HiLink*link;
 if(!hi_header()||_gz_hi_control.nodes!=_gz_hl_control.active_allocations)return 0;
 for(i=0;i<HL_HEAPS;i++){
  if(!ghpb_charge(w))return 0;h=_gz_hl_heaps+i;index=_gz_hi_heaps+i;
  if(!h->heap){if(index->head||index->count||index->generation||index->reserved)return 0;continue;}
  if(index->generation!=h->generation||index->reserved||index->count>HL_ALLOCS-ac||(!index->head)!=(!index->count))return 0;
  node=index->head;previous=0;
  for(j=0;j<index->count;j++){
   if(!ghpb_charge(w)||!hi_member(node,i+1,h->generation))return 0;
   a=_gz_hl_allocations+node-1;link=_gz_hi_links+node-1;
   if(link->previous!=previous)return 0;
   if(!ghpb_candidates(b,r,a,node,i+1,w))return 0;
   previous=node;node=link->next;
  }
  if(node)return 0;ac+=index->count;
 }
 return ac==_gz_hl_control.active_allocations&&ac==_gz_hi_control.nodes;}
GHC_FN u32 ghpb_join(GhcProcessDirectory*d,const GhdCut*c,const GhcHeapRow*heaps,u32 count,u32 raw,u32 bytes,void*scratch,u32 scratch_bytes,u32 capacity){
 GhpbWork*b=scratch;GhcProcessRow*r=(GhcProcessRow*)(d+1);ObTryWork w={GHCP_WORK,0};u32 i,z=0,ok;
 if(capacity>PO_CAP||d->count>capacity||scratch_bytes!=ghc_batch_bytes(capacity))return GHC_SPACE;
 zero(scratch,scratch_bytes);b->magic=0x47484231u;b->bytes=scratch_bytes;b->capacity=capacity;b->phase=1;
 for(i=0;i<d->count;i++)if(r[i].process.state==PO_LIVE&&!r[i].process.flags&&!(d->flags&GHCP_PO_GAP))ghpb_a(b)[b->queries++]=i;
 ghpb_sort(b,r);
 if(!ghc_hl_lock())return GHC_OBSERVER;
 if(!ghc_hl_healthy()||!ghc_same_root()||_gz_hl_control.serial!=c->serial||_gz_hl_control.release_serial!=c->release_serial){z=GHC_CHANGED;goto done;}
 ghc_copy(&d->hl_before,&_gz_hl_control,64);
 ok=ghpb_census(b,r,&w);
 if(!ok){
  if(!w.exhausted){z=GHC_OBSERVER;goto done;}
  /* Incomplete scan cannot rule out a later competing owner. Refuse EVERY
   * binding, retaining every original PO64 row and explicit BUDGET. */
  for(i=0;i<d->count;i++)ghpb_none(r+i);
  d->flags|=GHCP_BUDGET;
 }else for(i=0;i<d->count;i++){
  GhcProcessRow*q=r+i;HlStoredAllocation*a;HlHeap*h;HmScope*s;u32 id=q->reserved,slot=q->heap_index;
  if(q->binding!=GHPB_CANDIDATE||!id||id>HL_ALLOCS||!slot||slot>HL_HEAPS){ghpb_none(q);continue;}
  if(!ghpb_charge(&w)){ghpb_none(q);d->flags|=GHCP_BUDGET;continue;}
  a=_gz_hl_allocations+id-1;h=_gz_hl_heaps+slot-1;s=_gz_hm_scopes+slot-1;
  /* Original gar policy applies to the FINAL unique owner, not an outer
   * candidate which the original owner resolver would later supersede. */
  if(!hi_member(id,slot,h->generation)||!gz_gar_allocation_policy(a,h)||
   s->flags||s->active_token||s->reserved){ghpb_none(q);continue;}
  if(ghc_contains(raw,bytes,q->process.object,q->process.allocation_bytes)&&ghp_heap_member(heaps,count,slot,h,s))q->binding=GHCP_BIND_LIVE;
  else if(ghc_apart(raw,bytes,q->process.object,q->process.allocation_bytes))q->binding=GHCP_BIND_OUTSIDE;
  else{ghpb_none(q);continue;}
  hl_export(&q->allocation,a);ghc_copy(&q->owner,h,64);ghc_copy(&q->scope,s,32);
  /* Private current index survives raw copy only; end rechecks and clears it
   * before seals/validation. It is NEVER a published reserved wire field. */
 }
 d->bound=d->outside=d->unbound=0;
 for(i=0;i<d->count;i++){if(r[i].binding==GHCP_BIND_LIVE)++d->bound;
  else if(r[i].binding==GHCP_BIND_OUTSIDE)++d->outside;else{ghpb_none(r+i);++d->unbound;}}
 if(d->unbound)d->flags|=GHCP_UNBOUND;
 if(w.exhausted)d->flags|=GHCP_BUDGET;
 d->work_used=GHCP_WORK-w.left;b->metadata_used=d->work_used;b->phase=2;
 done:ghc_hl_unlock();return z;}
GHC_FN u32 ghpb_end_bindings(GhcProcessDirectory*d,const GhdCut*c,void*scratch,u32 scratch_bytes){
 GhpbWork*b=scratch;GhcProcessRow*r=(GhcProcessRow*)(d+1);ObTryWork w={GHCP_WORK,0};u32 i,z=0;
 if(b->magic!=0x47484231u||b->bytes!=scratch_bytes||b->phase!=2||b->capacity>PO_CAP||d->count>b->capacity||
  b->bytes!=ghc_batch_bytes(b->capacity))return GHC_CHANGED;
 if(!ghc_hl_lock())return GHC_OBSERVER;
 if(!ghc_hl_healthy()||!ghc_same_root()||_gz_hl_control.serial!=c->serial||_gz_hl_control.release_serial!=c->release_serial||
  !ghc_equal(&d->hl_before,&_gz_hl_control,64)||!hi_header()||_gz_hi_control.nodes!=_gz_hl_control.active_allocations){z=GHC_CHANGED;goto done;}
 for(i=0;i<d->count;i++)if(r[i].binding){u32 id=r[i].reserved,slot=r[i].heap_index;HlStoredAllocation*a;HlHeap*h;HlAllocation again;
  if(!ghpb_charge(&w)||!id||id>HL_ALLOCS||!slot||slot>HL_HEAPS){z=GHC_CHANGED;goto done;}
  a=_gz_hl_allocations+id-1;h=_gz_hl_heaps+slot-1;
  if(!hi_member(id,slot,h->generation)||!gz_gar_allocation_policy(a,h)||
   !ghc_equal(h,&r[i].owner,64)||!ghc_equal(_gz_hm_scopes+slot-1,&r[i].scope,32)){z=GHC_CHANGED;goto done;}
  hl_export(&again,a);if(!ghc_equal(&again,&r[i].allocation,80)){z=GHC_CHANGED;goto done;}
 }
 ghc_copy(&d->hl_after,&_gz_hl_control,64);b->phase=3;
 done:ghc_hl_unlock();if(z)return z;
 for(i=0;i<d->count;i++)r[i].reserved=0;
 return 0;}
