/* Same-TU native metadata only. Every caller already owns the REAL HL lock;
 * body reads remain exclusively in the original HG/GPA/whole-apart reader. */
GHC_FN HlHeap*ghc_common_live_root(const GhdLabels*labels,ObTryWork*w){
 u32 paths[3][GHD_DEPTH],counts[3]={0},r,i,j,at;const u32*values=(const u32*)labels;
 HlHeap*h,*parent;HmScope*s;
 if(labels->reserved||!values[0]||!values[1]||!values[2]||values[0]==values[1]||values[0]==values[2]||values[1]==values[2])return 0;
 for(r=0;r<3;r++){
  h=ghc_hl_heap(values[r],w);if(!h)return 0;
  for(i=0;i<GHD_DEPTH;i++){
   if(!obt_take(w)||h->state!=HL_LIVE||h->kind!=HL_EXP||!h->generation)return 0;
   at=(u32)(h-_gz_hl_heaps)+1;
   for(j=0;j<counts[r];j++)if(paths[r][j]==at)return 0;
   paths[r][counts[r]++]=at;s=_gz_hm_scopes+at-1;
   if(s->flags||s->active_token||s->reserved)return 0;
   if(!h->parent){if(h->parent_generation)return 0;break;}
   parent=ghc_hl_heap(h->parent,w);
   if(!parent||!h->parent_generation||parent->generation!=h->parent_generation||parent->epoch!=s->parent_epoch)return 0;
   h=parent;
  }
  if(i==GHD_DEPTH||w->exhausted)return 0;
 }
 for(i=0;i<counts[0];i++){
  u32 matches=1;
  for(r=1;r<3;r++){for(j=0;j<counts[r];j++)if(paths[r][j]==paths[0][i])break;if(j<counts[r])++matches;}
  if(matches==3){h=_gz_hl_heaps+paths[0][i]-1;return h->parent?h:0;}
 }
 return 0;
}
GHC_FN u32 ghc_common_ancestor(const GhdCut*c,u32 selected,u32 slot){
 u32 at=c->node[selected-1].parent_node;
 while(at){if(c->node[at-1].slot==slot)return at;at=c->node[at-1].parent_node;}
 return 0;
}
GHC_FN u32 ghc_common_marks(const GhdCut*c,u32 selected,u32 raw,u32 bytes,ObTryWork*w){
 u32 i,j,at,parent,inside,ancestor;HlHeap*h;
 if(!selected||selected>c->nodes)return 0;
 /* Rebuild genuine generation/epoch-qualified parent slots under this lock.
  * No stale cross-lock GHD membership receipt authorizes a heap. */
 for(i=0;i<HL_HEAPS;i++){
  if(!obt_take(w))return 0;ghd_scratch.parent[i]=0;ghd_scratch.mask[i]&=(unsigned char)~GHC_ENROLLED_HEAP;
 }
 if(!ghd_parents(w))return 0;
 for(i=0;i<HL_HEAPS;i++){
  if(!obt_take(w))return 0;h=_gz_hl_heaps+i;
  if(!h->heap||h->state==HL_DEAD)continue;
  at=i+1;inside=0;
  for(j=0;j<GHD_DEPTH;j++){
   if(!obt_take(w))return 0;
   if(at==c->node[selected-1].slot){inside=1;break;}
   parent=ghd_scratch.parent[at-1];
   if(parent==GHD_BAD_PARENT||parent>HL_HEAPS)return 0;
   if(!parent)break;at=parent;
  }
  if(j==GHD_DEPTH)return 0;
  if(inside){
   if(h->state!=HL_LIVE||!ghc_contains(raw,bytes,h->heap,header_bytes(h->kind))||
    !ghc_contains(raw,bytes,h->start,h->bytes))return 0;
   ghd_scratch.mask[i]|=GHC_ENROLLED_HEAP;
  }else if(!ghc_apart(raw,bytes,h->heap,header_bytes(h->kind))||
   !ghc_apart(raw,bytes,h->start,h->bytes)){
   ancestor=ghc_common_ancestor(c,selected,i+1);
   if(!ancestor||!ghc_equal(h,&c->node[ancestor-1].heap,64)||
    !ghc_equal(_gz_hm_scopes+i,&c->node[ancestor-1].scope,32))return 0;
  }
 }
 return !w->exhausted;
}
/* Complete HI topology plus ascending allocation-index projection. Capacity
 * is a fresh pre-reserve bound; increased selected count refuses SPACE rather
 * than truncating rows. OUTSIDE SDK allocations are never copied as a domain.
 * mode0 measures, mode1 captures, mode2 compares an existing private census. */
GHC_FN u32 ghc_common_allocations(const GhdCut*c,u32 selected,u32 raw,u32 bytes,
 GhcAllocationDirectory*d,u32 capacity,u32 mode,u32*out_count){
 ObTryWork w={GHD_WORK,0};u32 i,j,node,previous,total=0,count=0,live=0,ending=0,heaps=0,k=0;
 GhcAllocationRow*rows=d?(GhcAllocationRow*)(d+1):0;HlHeap*h;HiHeap*index;HlStoredAllocation*a;
 if(mode>2||(!d&&mode)||!hi_header()||!ghc_hl_healthy()||
  _gz_hl_control.serial!=c->serial||_gz_hl_control.release_serial!=c->release_serial||
  !ghc_common_marks(c,selected,raw,bytes,&w))return GHC_CHANGED;
 for(i=0;i<HL_HEAPS;i++){
  if(!obt_take(&w))return GHC_OBSERVER;h=_gz_hl_heaps+i;index=_gz_hi_heaps+i;
  if(!h->heap){if(index->head||index->count||index->generation||index->reserved)return GHC_OBSERVER;continue;}
  if(index->generation!=h->generation||index->reserved||index->count>HL_ALLOCS-total||
   (!index->head)!=(!index->count))return GHC_OBSERVER;
  total+=index->count;node=index->head;previous=0;
  if(ghd_scratch.mask[i]&GHC_ENROLLED_HEAP){++heaps;count+=index->count;}
  for(j=0;j<index->count;j++){
   if(!obt_take(&w)||!hi_member(node,i+1,h->generation)||_gz_hi_links[node-1].previous!=previous)return GHC_OBSERVER;
   a=_gz_hl_allocations+node-1;
   if(ghd_scratch.mask[i]&GHC_ENROLLED_HEAP){
    if(a->heap_epoch!=h->epoch||a->kind!=h->kind||a->reserved||a->missing!=31||
     !within(h->start,h->bytes,a->pointer,a->capacity)||
     (a->kind==HL_EXP&&a->block&&(a->block>a->pointer||a->pointer-a->block<16||!within(h->start,h->bytes,a->block,16))))return GHC_OBSERVER;
    if(a->state==HL_LIVE)++live;else ++ending;
   }
   previous=node;node=_gz_hi_links[node-1].next;
  }
  if(node)return GHC_OBSERVER;
 }
 if(total!=_gz_hi_control.nodes||total!=_gz_hl_control.active_allocations||count>capacity||
  !heaps||!count||live+ending!=count)return count>capacity?GHC_SPACE:GHC_OBSERVER;
 if(!mode){*out_count=count;return 0;}
 if(mode==2&&(d->magic!=GHC_ALLOC_MAGIC||d->version!=GHC_ALLOC_VERSION||d->row_bytes!=GHC_ALLOC_ROW||
  d->count!=count||d->bytes!=sizeof(*d)+count*sizeof(*rows)||d->live!=live||d->ending!=ending||
  d->flags!=(ending?GHC_ALLOC_ENDING:0)||d->root_node!=selected||d->root_slot!=c->node[selected-1].slot||
  d->serial!=c->serial||d->release_serial!=c->release_serial||d->heap_count!=heaps||!d->bank_offset||d->bank_bytes!=WGC_BYTES))return GHC_CHANGED;
 for(i=0;i<HL_ALLOCS;i++){
  u32 owner;GhcAllocationRow projected;
  if(!obt_take(&w))return GHC_OBSERVER;a=_gz_hl_allocations+i;if(!hi_active(a))continue;
  owner=_gz_hi_links[i].owner;
  if(!owner||owner>HL_HEAPS||!hi_member(i+1,owner,_gz_hl_heaps[owner-1].generation))return GHC_OBSERVER;
  if(!(ghd_scratch.mask[owner-1]&GHC_ENROLLED_HEAP))continue;
  if(k>=count)return GHC_CHANGED;
  hl_export(&projected.allocation,a);projected.index=i+1;projected.owner_slot=owner;
  if(mode==1)ghc_copy(rows+k,&projected,sizeof(projected));
  else if(!ghc_equal(rows+k,&projected,sizeof(projected)))return GHC_CHANGED;
  ++k;
 }
 if(k!=count||w.exhausted)return GHC_CHANGED;
 if(mode==1){zero(d,sizeof(*d));d->magic=GHC_ALLOC_MAGIC;d->version=GHC_ALLOC_VERSION;
  d->bytes=sizeof(*d)+count*sizeof(*rows);d->row_bytes=sizeof(*rows);d->count=count;d->live=live;d->ending=ending;
  d->flags=ending?GHC_ALLOC_ENDING:0;d->root_node=selected;d->root_slot=c->node[selected-1].slot;
  d->serial=c->serial;d->release_serial=c->release_serial;d->heap_count=heaps;
  d->row_checksum=ghc_plan_hash(rows,count*sizeof(*rows));
 }else if(d->row_checksum!=ghc_plan_hash(rows,count*sizeof(*rows)))return GHC_CHANGED;
 *out_count=count;return 0;
}
/* Genuine pre-reserve metadata census. It reuses the ACTUAL original GHD
 * resolver under the original HL lock; no fabricated SDK root or body read.
 * This fixed scratch is rebuilt by the genuine held cut before payload copy. */
GHC_FN u32 ghc_common_propose_locked(void){
 ObTryWork w={GHD_WORK,0};GhdCut*c=&ghd_scratch.cut;u32 i,selected,capacity=0;
 if(!ghc_hl_healthy()||!obd_reserve(&w,sizeof(ghd_scratch)/4))return 0;
 zero(&ghd_scratch,sizeof(ghd_scratch));
 if(!ghd_parents(&w)||!ghd_roots(&ghc_proposal.labels,&w))return 0;
 for(i=0;i<HL_HEAPS;i++)if(!ghd_membership(i,&w))return 0;
 if(!ghd_backing(&w)||!ghd_census(&w)||w.exhausted)return 0;
 c->magic=0x47484431u;c->bytes=sizeof(*c);c->version=1;c->global_heaps=_gz_hl_control.active_heaps;
 c->global_allocations=_gz_hl_control.active_allocations;c->serial=_gz_hl_control.serial;
 c->release_serial=_gz_hl_control.release_serial;c->frame=_gz_hl_frame;c->missing=31;
 selected=ghc_common_node(c,&ghc_proposal.labels);
 if(!selected||!ghc_bounds(&c->node[selected-1].heap,&ghc_proposal.raw,&ghc_proposal.bytes))return 0;
 ghc_copy(&ghc_proposal.root,&c->node[selected-1].heap,64);ghc_copy(&ghc_proposal.scope,&c->node[selected-1].scope,32);
 if(ghc_proposal.scope.flags||ghc_proposal.scope.active_token||ghc_proposal.scope.reserved||
  ghc_common_allocations(c,selected,ghc_proposal.raw,ghc_proposal.bytes,0,HL_ALLOCS,0,&capacity))return 0;
 ghc_proposal.allocation_capacity=capacity;return capacity!=0;
}
/* No authority: validates complete PRIVATE AS7 allocation records and their
 * owner-heap joins; native producer is responsible for the complete held cut. */
GHC_FN u32 ghc_common_format(const GhcHeader*h,const GhdCut*c,u32 selected){return ghc_common_image_format(h,c,selected);}
