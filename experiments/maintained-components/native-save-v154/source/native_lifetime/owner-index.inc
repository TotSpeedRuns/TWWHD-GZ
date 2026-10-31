/* Included after descends(), under the existing common lifetime lock.
 * Only per-heap lists whose native span contains the requested extent can
 * own it. Parent/child ambiguity rules remain the original owner rules. */
static u32 owner_index_locked(u32 p,u32 n,HlStoredAllocation**allocation,HlHeap**heap){
 u32 i,j,node,result=1;HlStoredAllocation*a,*found=0;HlHeap*h,*owner=0;HiHeap*index;
 *allocation=0;*heap=0;
 if(!span(p,n)||_gz_hl_control.flags||!hi_header()||_gz_hi_control.nodes!=_gz_hl_control.active_allocations)return 3;
 for(i=0;i<HL_HEAPS;i++){
  h=_gz_hl_heaps+i;index=_gz_hi_heaps+i;
  if(!h->heap){if(index->head||index->count||index->generation||index->reserved)return 3;continue;}
  if(!within(h->start,h->bytes,p,n))continue;
  if(!hi_ready(h))return 3;
  node=hi_head(h);
  for(j=0;j<index->count;j++){
   a=_gz_hl_allocations+node-1;node=hi_next(node);
   if(a->state!=HL_LIVE||!within(a->pointer,a->capacity,p,n))continue;
   if(h->state!=HL_LIVE||!a->heap_generation||a->heap_generation!=h->generation||a->heap_epoch!=h->epoch||!_gz_hm_scope_live(h))return 3;
   if(found){
    if(h==owner)return 2;
    if(descends(h,owner,found)){found=a;owner=h;}
    else if(!descends(owner,h,a))return 2;
   }else{found=a;owner=h;}
  }
 }
 if(found){*allocation=found;*heap=owner;result=0;}return result;
}
