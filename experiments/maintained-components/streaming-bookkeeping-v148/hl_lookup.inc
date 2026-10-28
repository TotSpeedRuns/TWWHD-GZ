/* Exact selected v148 existing-key/create selectors. Include after the
 * real HlStoredAllocation/HL_* declarations in the owning observer TU.
 * Callers retain the original lock, validation, lifetime and writer rules. */
static HlStoredAllocation*allocation_find(u32 p,u32 heap,u32 create){u32 i,j,dead=HL_ALLOCS,start=((p>>2)*0x9e3779b1u)>>14;HlStoredAllocation*e;
 /* Existing-key lookup never consumes a tombstone selection. Branch once,
  * preserving match-before-empty behavior and the original create loop. */
 if(!create){
  for(i=0;i<HL_ALLOCS;i++){
   j=(start+i)&(HL_ALLOCS-1);e=&_gz_hl_allocations[j];
   if(e->pointer==p&&e->heap==heap)return e;
   if(!e->pointer)return 0;
  }
  return 0;
 }
 for(i=0;i<HL_ALLOCS;i++){
  j=(start+i)&(HL_ALLOCS-1);e=&_gz_hl_allocations[j];if(e->pointer==p&&e->heap==heap)return e;
  if(e->state==HL_DEAD&&dead==HL_ALLOCS)dead=j;
  if(!e->pointer)return create?&_gz_hl_allocations[dead==HL_ALLOCS?j:dead]:0;
 }return create&&dead<HL_ALLOCS?&_gz_hl_allocations[dead]:0;
}
static HlStoredAllocation*custom_find(u32 p,u32 heap,u32 descriptor,u32 create){u32 i,j,dead=HL_ALLOCS,start=((p>>2)*0x9e3779b1u)>>14;HlStoredAllocation*e;
 /* Existing-key lookup never consumes a tombstone selection. Branch once,
  * preserving match-before-empty behavior and the original create loop. */
 if(!create){
  for(i=0;i<HL_ALLOCS;i++){
   j=(start+i)&(HL_ALLOCS-1);e=&_gz_hl_allocations[j];
   if(e->pointer==p&&e->heap==heap&&e->kind==HL_CUSTOM&&e->block==descriptor)return e;
   if(!e->pointer)return 0;
  }
  return 0;
 }
 for(i=0;i<HL_ALLOCS;i++){
  j=(start+i)&(HL_ALLOCS-1);e=&_gz_hl_allocations[j];
  if(e->pointer==p&&e->heap==heap&&e->kind==HL_CUSTOM&&e->block==descriptor)return e;
  if(e->state==HL_DEAD&&dead==HL_ALLOCS)dead=j;
  if(!e->pointer)return create?&_gz_hl_allocations[dead==HL_ALLOCS?j:dead]:0;
 }return create&&dead<HL_ALLOCS?&_gz_hl_allocations[dead]:0;
}
