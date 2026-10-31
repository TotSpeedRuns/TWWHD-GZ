/* Native PO metadata adapter. The selected runtime producer owns this exact
 * PoHeader.busy CAS protocol. This NEW try-only metadata reader acquires that
 * same word and never flags a normal BUSY refusal. Original producer/query
 * lock functions and every native writer remain byte-exact. No synthetic
 * table, report authority, native pointer follow, allocator or wait. */
GHC_FN u32 ghp_po_lock(void){PoHeader*h=&_gz_po_header;
 if(h->magic!=PO_MAGIC||h->bytes!=64||h->version!=1||h->capacity!=PO_CAP||h->rows!=(u32)_gz_po_rows)return 0;
 return __sync_bool_compare_and_swap(&h->busy,0,1);}
GHC_FN void ghp_po_unlock(void){__sync_synchronize();*(volatile u32*)&_gz_po_header.busy=0;}
GHC_FN u32 ghp_po_header(void){return _gz_po_header.magic==PO_MAGIC&&
 _gz_po_header.bytes==64&&_gz_po_header.version==1&&_gz_po_header.capacity==PO_CAP&&
 _gz_po_header.rows==(u32)_gz_po_rows&&_gz_po_header.busy==1;}
GHC_FN u32 ghp_po_entry(const PoEntry*r){return ghcp_process_format(r);}
GHC_FN u32 ghp_po_count(u32*out){u32 i,n=0;
 if(!ghp_po_header())return 0;
 for(i=0;i<PO_CAP;i++){const PoEntry*r=_gz_po_rows+i;
  if(r->object){if(!ghp_po_entry(r))return 0;++n;}
  else{u32 j;for(j=0;j<16;j++)if(((const u32*)r)[j])return 0;}}
 *out=n;return 1;}
GHC_FN u32 ghp_measure(u32*out){u32 z;
 if(!ghp_po_lock())return 0;z=ghp_po_count(out);ghp_po_unlock();return z;}
/* Called only after the whole fresh source/destination exclusion and the
 * genuine OIS stage check. Count before copy; a larger cut refuses SPACE. */
GHC_FN u32 ghp_begin(GhcProcessDirectory*d,u32 capacity){u32 i,j,n=0,k=0,z=0;GhcProcessRow*r;
 if(!ghp_po_lock())return GHC_OBSERVER;
 if(!ghp_po_count(&n)){z=GHC_OBSERVER;goto done;}
 if(n>capacity){z=GHC_SPACE;goto done;}
 zero(d,sizeof(*d)+n*sizeof(GhcProcessRow));d->magic=GHCP_MAGIC;
 d->bytes=sizeof(*d)+n*sizeof(GhcProcessRow);d->version=GHCP_VERSION;
 d->row_bytes=sizeof(GhcProcessRow);d->count=n;d->row_offset=sizeof(*d);
 ghc_copy(&d->po_before,&_gz_po_header,64);r=(GhcProcessRow*)(d+1);
 for(i=0;i<PO_CAP;i++)if(_gz_po_rows[i].object){
  ghc_copy(&r[k].process,_gz_po_rows+i,64);r[k].po_index=i;
  j=r[k].process.state;if(j==PO_LIVE)++d->live;else if(j==PO_ENDING)++d->ending;else ++d->ended;++k;}
 if(d->po_before.flags||d->po_before.active!=d->live+d->ending)d->flags|=GHCP_PO_GAP;
 done:ghp_po_unlock();return z;}
GHC_FN u32 ghp_heap_member(const GhcHeapRow*rows,u32 count,u32 slot,const HlHeap*h,const HmScope*s){u32 i;
 for(i=0;i<count;i++)if(rows[i].slot==slot)return ghc_equal(rows[i].heap,h,64)&&ghc_equal(rows[i].scope,s,32);
 return 0;}
/* One integrated HI/HL traversal under the original real HL lock. Sorting
 * and cached ancestry use only pre-reserved PRIVATE staging. */
#include "owned_heap_process_batch.inc"
GHC_FN u32 ghp_join(GhcProcessDirectory*d,const GhdCut*c,const GhcHeapRow*heaps,u32 count,u32 raw,u32 bytes,
 void*scratch,u32 scratch_bytes,u32 capacity){
 return ghpb_join(d,c,heaps,count,raw,bytes,scratch,scratch_bytes,capacity);}
GHC_FN u32 ghp_end(GhcProcessDirectory*d,const GhdCut*c,void*scratch,u32 scratch_bytes){u32 i,k=0,n=0,z=0;GhcProcessRow*r=(GhcProcessRow*)(d+1);
 if(!ghp_po_lock())return GHC_OBSERVER;
 if(!ghp_po_count(&n)||n!=d->count||!ghc_equal(&d->po_before,&_gz_po_header,64)){z=GHC_CHANGED;goto po_done;}
 for(i=0;i<PO_CAP;i++)if(_gz_po_rows[i].object){if(k>=n||r[k].po_index!=i||
  !ghc_equal(&r[k].process,_gz_po_rows+i,64)){z=GHC_CHANGED;goto po_done;}++k;}
 ghc_copy(&d->po_after,&_gz_po_header,64);
 po_done:ghp_po_unlock();if(z)return z;
 z=ghpb_end_bindings(d,c,scratch,scratch_bytes);if(z)return z;
 d->row_checksum=ghc_plan_hash(r,n*sizeof(*r));return 0;}
/* Pure PRIVATE archive validation; every recorded address is an opaque key. */
GHC_FN u32 ghp_bound_format(const GhcProcessRow*r){const HlAllocation*a=&r->allocation;const HlHeap*h=&r->owner;u32 begin,bytes;
 if(r->process.state!=PO_LIVE||r->process.flags||r->heap_index<1||r->heap_index>HL_HEAPS||
  !a->generation||a->state!=HL_LIVE||a->kind!=h->kind||a->heap!=h->heap||a->heap_generation!=h->generation||
  a->heap_epoch!=h->epoch||a->release_token||a->end_frame||a->end_site||a->reserved||a->missing!=31||
  !(!(a->flags&~(HM_BORN_DURING|256u))||(a->flags&~256u)==HM_RESIZED)||
  !ghc_contains(a->pointer,a->capacity,r->process.object,r->process.allocation_bytes)||
  !ghc_bounds(h,&begin,&bytes)||(h->direction!=1u&&(h->direction!=0xffffffffu||h->kind==HL_CUSTOM))||
  (!h->parent&&h->parent_generation)||
  h->vtable!=ghc_table(h->kind)||(h->flags&~HL_REBORN)||h->end_frame||h->dtor_site||
  !ghc_contains(h->start,h->bytes,a->pointer,a->capacity)||
  r->scope.active_token||r->scope.flags||r->scope.reserved)return 0;
 if(a->kind==HL_EXP){if(!a->block||a->block>a->pointer||a->pointer-a->block<16||
  a->padding!=a->pointer-a->block-16||!ghc_contains(h->start,h->bytes,a->block,16))return 0;}
 else if(a->padding)return 0;return 1;}
GHC_FN u32 ghp_format(const GhcHeader*h){const unsigned char*p=(const unsigned char*)h;
 const GhcProcessDirectory*d;const GhcProcessRow*r;const GhdCut*c=(const GhdCut*)(p+h->cut);
 const GhcSaveProducer*producer=(const GhcSaveProducer*)(p+128);
 const GhcHeapRow*heaps=(const GhcHeapRow*)(p+h->table);u32 i,j,live=0,ending=0,ended=0,bound=0,outside=0,unbound=0,last=0,gap;
 if(h->reserved[0]!=ghc_process_offset(h->count)||h->reserved[0]>h->bytes||
  sizeof(*d)>h->bytes-h->reserved[0])return 0;
 d=(const GhcProcessDirectory*)(p+h->reserved[0]);
 if(d->count>PO_CAP||d->bytes!=sizeof(*d)+d->count*sizeof(*r)||d->bytes!=h->reserved[1]||
  d->bytes>h->bytes-h->reserved[0]||((h->version!=GHC_PROCESS_VERSION&&h->version!=GHC_LEGACY_COMMON_VERSION&&h->version!=GHC_LEGACY_PACKED_VERSION)&&h->data!=ghc_process_data(h->count,d->count))||
  ((h->version==GHC_PROCESS_VERSION||h->version==GHC_LEGACY_COMMON_VERSION||h->version==GHC_LEGACY_PACKED_VERSION)&&h->data<ghc_allocation_offset(h->count,d->count)+GHC_ALLOC_HEADER)||
  h->data<h->reserved[0]+d->bytes||d->magic!=GHCP_MAGIC||d->version!=GHCP_VERSION||
  d->row_bytes!=sizeof(*r)||d->row_offset!=sizeof(*d)||d->reserved||d->flags&~7u||d->work_used>GHCP_WORK||
  !ghc_equal(&d->po_before,&d->po_after,64)||!ghc_equal(&d->hl_before,&d->hl_after,64)||
  d->po_before.magic!=PO_MAGIC||d->po_before.bytes!=64||d->po_before.version!=1||
  d->po_before.capacity!=PO_CAP||!d->po_before.rows||d->po_before.busy!=1||
  d->hl_before.magic!=HL_MAGIC||d->hl_before.bytes!=64||d->hl_before.version!=1||d->hl_before.busy!=1||
  d->hl_before.flags||d->hl_before.session||d->hl_before.active_heaps>HL_HEAPS||
  d->hl_before.active_allocations>HL_ALLOCS||d->hl_before.serial!=c->serial||
  d->hl_before.release_serial!=c->release_serial||!d->scene.epoch||d->scene.epoch!=producer->epoch||
  d->scene.hash!=producer->scene_hash||d->scene.hash!=ghcp_scene_hash(&d->scene)||d->scene.reserved)return 0;
 r=(const GhcProcessRow*)(d+1);gap=d->po_before.flags||d->po_before.active!=d->live+d->ending;
 if(!!(d->flags&GHCP_PO_GAP)!=!!gap)return 0;
 for(i=0;i<d->count;i++){
  if(!ghp_po_entry(&r[i].process)||r[i].po_index>=PO_CAP||(i&&r[i].po_index<=last)||r[i].reserved||r[i].binding>GHCP_BIND_OUTSIDE)return 0;
  last=r[i].po_index;
  if(r[i].process.state==PO_LIVE)++live;else if(r[i].process.state==PO_ENDING)++ending;else ++ended;
  if(!r[i].binding){for(j=0;j<(80+64+32)/4;j++)if(((const u32*)&r[i].allocation)[j])return 0;
   if(r[i].heap_index)return 0;++unbound;continue;}
  if(gap||!ghp_bound_format(r+i))return 0;
  if(r[i].binding==GHCP_BIND_LIVE){if(!ghc_contains(h->domain.start,h->domain.span,r[i].process.object,r[i].process.allocation_bytes)||
    !ghp_heap_member(heaps,h->count,r[i].heap_index,&r[i].owner,&r[i].scope))return 0;++bound;}
  else{if(!ghc_apart(h->domain.start,h->domain.span,r[i].process.object,r[i].process.allocation_bytes))return 0;++outside;}
 }
 if(live!=d->live||ending!=d->ending||ended!=d->ended||bound!=d->bound||outside!=d->outside||unbound!=d->unbound||
  live+ending+ended!=d->count||bound+outside+unbound!=d->count||!!(d->flags&GHCP_UNBOUND)!=!!unbound||
  d->row_checksum!=ghc_plan_hash(r,d->count*sizeof(*r)))return 0;
 for(i=h->table+h->count*GHC_ROW;i<h->reserved[0];i++)if(p[i])return 0;
 for(i=h->reserved[0]+d->bytes;i<((h->version==GHC_PROCESS_VERSION||h->version==GHC_LEGACY_COMMON_VERSION||h->version==GHC_LEGACY_PACKED_VERSION)?ghc_allocation_offset(h->count,d->count):h->data);i++)if(p[i])return 0;
 return 1;}
