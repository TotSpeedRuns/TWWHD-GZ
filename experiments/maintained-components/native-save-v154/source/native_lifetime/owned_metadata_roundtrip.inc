/* Actual same-TU private table application. Both real metadata locks remain
 * held through apply, verification and inverse. No native constructor,
 * destructor, allocator or device API is substituted or called here.
 * Permission cursors advance even when the before-image is restored: issued
 * generations/epochs are never rewound or reused.
 */
typedef struct { HlHeap heap; HmScope scope; HlCustomMeta custom; HiHeap links; } HmxHeap;
typedef struct { HlStoredAllocation allocation; HiLink link; } HmxAllocation;
typedef struct {
    u32 magic,bytes,version,state,request,frame,hg,job;
    u32 journal_bytes,source_checksum,hl_serial,po_serial;
    u32 heaps,allocations,processes,root;
    u32 heap_offset,allocation_offset,process_offset,marks_offset;
    u32 marks_bytes,end_serial,end_release,end_po_serial;
    u32 checksum,applied_verified,inverse_verified,dirty,error,reserved[3];
    HlControl hl; HiControl hi; PoHeader po;
} HmxHeader;
typedef char HmxHeap132[sizeof(HmxHeap)==132?1:-1];
typedef char HmxAllocation72[sizeof(HmxAllocation)==72?1:-1];
typedef char HmxHeader320[sizeof(HmxHeader)==320?1:-1];

GHC_FN HmdHeader *hmx_journal(u32 req,u32 f,u32 phase) {
    const OisHeldView *v=&_gz_ois_held_record;HmdHeader shape,*h;u32 p;
    if(!_gz_ghw_scope(req,f,phase)||!_gz_ois_held_owner(req,f,v)||
       !hmd_shape((const GhcHeader*)v->saved.archive,(const GhcHeader*)v->current.archive,&shape))return 0;
    p=_gz_ois_held_work(req,f,v,shape.bytes);if(!p)return 0;h=(HmdHeader*)p;
    if(h->magic!=shape.magic||h->bytes!=shape.bytes||h->version!=2||h->valid!=1||
       h->request!=req||h->frame!=f||h->hg!=v->hg||h->job!=v->job_generation||h->missing!=255||
       h->saved_identity!=v->saved.identity||h->current_identity!=v->current.identity||
       !ghc_equal(&h->saved_heaps,&shape.saved_heaps,14*sizeof(u32))||h->checksum!=hmd_hash(h))return 0;
    return h;
}

GHC_FN u32 hmx_shape(const HmdHeader *j,HmxHeader *h) {
    u32 at=sizeof(*h),hash=1,marks;
    zero(h,sizeof(*h));h->magic=0x484d5831u;h->version=1;h->journal_bytes=j->bytes;h->hl_serial=j->hl.serial;h->po_serial=j->po.serial;
    h->heaps=j->saved_heaps;h->allocations=j->saved_allocations;h->processes=j->saved_processes;
    if(j->hl.serial>0xffffffffu-h->heaps||h->allocations>0xffffffffu-j->hl.serial-h->heaps||
       h->heaps>0xffffffffu-j->hl.release_serial||h->processes>0xffffffffu-j->po.serial)return 0;
    h->end_serial=j->hl.serial+h->heaps+h->allocations;
    h->end_release=j->hl.release_serial+h->heaps;h->end_po_serial=j->po.serial+h->processes;
    h->heap_offset=at;if(!hmd_add(&at,h->heaps,sizeof(HmxHeap)))return 0;
    h->allocation_offset=at;if(!hmd_add(&at,h->allocations,sizeof(HmxAllocation)))return 0;
    h->process_offset=at;if(!hmd_add(&at,h->processes,sizeof(PoEntry)))return 0;
    while(hash<h->allocations){if(hash>0x10000000u)return 0;hash<<=1;}
    if(hash>0x10000000u)return 0;hash<<=1;
    /* Heap ordinal map + per-heap list head ordinal + generation hash +
     * allocation/heap/process destination bitsets. All transient only. */
    marks=HL_HEAPS*8u+hash*4u+(HL_ALLOCS+HL_HEAPS+PO_CAP)/8u;
    h->marks_offset=at;h->marks_bytes=marks;
    if(!hmd_add(&at,marks,1))return 0;h->bytes=at;
    return j->bytes<=0x40000000u-at;
}

GHC_FN u32 hmx_mark(u32 *bits,u32 count,u32 index) {
    u32 mask;if(!index||index>count)return 0;--index;mask=1u<<(index&31u);
    if(bits[index>>5]&mask)return 0;bits[index>>5]|=mask;return 1;
}
GHC_FN u32 hmx_marked(const u32 *bits,u32 index){return(bits[(index-1)>>5]>>((index-1)&31u))&1u;}

GHC_FN u32 *hmx_heap_map(HmxHeader*h){return(u32*)((u32)h+h->marks_offset);}
GHC_FN u32 *hmx_heads(HmxHeader*h){return hmx_heap_map(h)+HL_HEAPS;}
GHC_FN u32 hmx_hash_count(const HmxHeader*h){u32 n=1;while(n<h->allocations)n<<=1;return n<<1;}
GHC_FN u32 *hmx_generations(HmxHeader*h){return hmx_heads(h)+HL_HEAPS;}
GHC_FN u32 *hmx_alloc_marks(HmxHeader*h){return hmx_generations(h)+hmx_hash_count(h);}
GHC_FN u32 *hmx_heap_marks(HmxHeader*h){return hmx_alloc_marks(h)+HL_ALLOCS/32;}
GHC_FN u32 *hmx_po_marks(HmxHeader*h){return hmx_heap_marks(h)+HL_HEAPS/32;}

GHC_FN u32 hmx_source_allocation(const GhcAllocationRow *a,HmxHeader*h,u32 pointer,u32 generation) {
    const u32 *table=hmx_generations(h);u32 mask=hmx_hash_count(h)-1,at=(generation*2654435761u)&mask,i;
    for(i=0;i<=mask;i++,at=(at+1)&mask){u32 n=table[at];if(!n)return 0;
        if(a[n-1].allocation.generation==generation)return a[n-1].allocation.pointer==pointer?n:0;}
    return 0;
}

GHC_FN u32 hmx_reference(const GhcAllocationRow *a,const HmdHeader*j,HmxHeader*h,u32 pointer,u32 generation) {
    const HmdAllocation *plans=(const HmdAllocation*)((u32)j+j->allocation_offset);
    HlStoredAllocation *outside;HlHeap *owner;u32 n=hmx_source_allocation(a,h,pointer,generation),id;
    if(n)return plans[n-1].plan.generation;
    outside=allocation_generation(pointer,generation);if(!outside||outside->state!=HL_LIVE)return 0;
    id=(u32)(outside-_gz_hl_allocations)+1;
    if(hmx_marked(hmx_alloc_marks(h),id))return 0;
    owner=heap_find(outside->heap,0);
    return owner&&owner->state==HL_LIVE&&owner->generation==outside->heap_generation&&
        owner->epoch==outside->heap_epoch?outside->generation:0;
}

GHC_FN u32 hmx_ancestor(const GhcHeapRow *rows,const u32 *map,u32 ancestor,u32 child) {
    u32 depth;for(depth=0;child&&depth<32;depth++){
        if(child==ancestor)return 1;child=rows[child-1].reserved?map[rows[child-1].reserved-1]:0;
    }return 0;
}

GHC_FN u32 hmx_custom(const GhcHeader *source,const GhcHeapRow *rows,const GhcAllocationRow *a,
                      const HmdHeader*j,HmxHeader*h,u32 ordinal,HmxHeap *out) {
    const HmdAllocation *plans=(const HmdAllocation*)((u32)j+j->allocation_offset);
    const OisHeldView*v=&_gz_ois_held_record;const GhcHeader*current=(const GhcHeader*)v->current.archive;
    u32 at=rows[ordinal].raw_offset,table,slots,n,i,best=0,owner=0,*map=hmx_heap_map(h);
    if(at>current->domain.span||0xacu>current->domain.span-at)return 0;
    /* Offset belongs to the genuine held CURRENT physical domain. Never
     * follow an archived heap/table pointer to enroll another native range. */
    table=R32(current->domain.start+at,0xa4);slots=R32(current->domain.start+at,0xa8);
    if(!slots||slots>0x0fffffffu)return 0;n=slots*16u;
    for(i=0;i<h->allocations;i++){
        const HlAllocation*r=&a[i].allocation;u32 candidate=map[a[i].owner_slot-1];
        if(r->state!=HL_LIVE||!ghc_contains(r->pointer,r->capacity,table,n))continue;
        if(!best){best=i+1;owner=candidate;continue;}
        if(candidate==owner)return 0;
        if(hmx_ancestor(rows,map,owner,candidate)){best=i+1;owner=candidate;}
        else if(!hmx_ancestor(rows,map,candidate,owner))return 0;
    }
    if(!best)return 0;
    out->custom.heap_generation=out->heap.generation;out->custom.table=table;out->custom.slots=slots;
    out->custom.anchor_pointer=a[best-1].allocation.pointer;
    out->custom.anchor_generation=plans[best-1].plan.generation;
    (void)source;return 1;
}

GHC_FN u32 hmx_prepare_locked(const GhcHeader*s,const GhcHeader*c,const HmdHeader*j,HmxHeader*h) {
    const GhcHeapRow *rows=(const GhcHeapRow*)((u32)s+s->table);
    const GhcAllocationDirectory *ad=hmd_allocations(s);const GhcAllocationRow *a=(const GhcAllocationRow*)(ad+1);
    const GhcProcessDirectory *pd=(const GhcProcessDirectory*)((u32)s+s->reserved[0]);
    const GhcProcessDirectory *cd=(const GhcProcessDirectory*)((u32)c+c->reserved[0]);
    const GhcProcessRow *p=(const GhcProcessRow*)(pd+1),*cp=(const GhcProcessRow*)(cd+1);
    const HmdHeap *hp=(const HmdHeap*)((u32)j+j->heap_offset);
    const HmdAllocation *ap=(const HmdAllocation*)((u32)j+j->allocation_offset);
    const HmdProcess *pp=(const HmdProcess*)((u32)j+j->process_offset);
    HmxHeap *heaps=(HmxHeap*)((u32)h+h->heap_offset);
    HmxAllocation *allocs=(HmxAllocation*)((u32)h+h->allocation_offset);
    PoEntry *processes=(PoEntry*)((u32)h+h->process_offset);
    u32 *map=hmx_heap_map(h),*heads=hmx_heads(h),*hash=hmx_generations(h),mask=hmx_hash_count(h)-1;
    u32 i,k,ordinal,parent,at,old;
    if(j->reserved[0]||!hmd_before_equal(j)||!ghc_hl_healthy()||!ghp_po_header())return HMP_CHANGED;
    if(j->current_heaps>j->hl.active_heaps||j->current_allocations>j->hl.active_allocations||
       j->current_processes>j->po.active)return HMP_FORMAT;
    for(i=0;i<j->heap_count;i++){
        if(!hp[i].plan.index){if(i<h->heaps||!hmd_zero_row(hp+i,sizeof(HmdHeap)))return HMP_FORMAT;continue;}
        if(!hmx_mark(hmx_heap_marks(h),HL_HEAPS,hp[i].plan.index))return HMP_FORMAT;
        if(hp[i].heap.state!=HL_LIVE&&(hp[i].links.head||hp[i].links.count||hp[i].links.reserved))return HMP_CHANGED;
        if(i<h->heaps){if(hp[i].plan.source!=i+1||!rows[i].slot||rows[i].slot>HL_HEAPS)return HMP_FORMAT;
            map[rows[i].slot-1]=i+1;if(rows[i].heap[0]==s->domain.heap)h->root=i+1;}
        else if(hp[i].plan.source)return HMP_FORMAT;
    }
    if(!h->root||hp[h->root-1].heap.heap!=c->domain.heap||
       hp[h->root-1].heap.generation!=c->domain.heap_generation)return HMP_CHANGED;
    for(i=0;i<j->allocation_count;i++){
        if(!ap[i].plan.index){if(i<h->allocations||!hmd_zero_row(ap+i,sizeof(HmdAllocation)))return HMP_FORMAT;continue;}
        if(!hmx_mark(hmx_alloc_marks(h),HL_ALLOCS,ap[i].plan.index))return HMP_FORMAT;
        if(!hi_active(&ap[i].allocation)&&(ap[i].link.previous||ap[i].link.next||ap[i].link.owner||
            ap[i].link.generation||ap[i].link.reserved))return HMP_CHANGED;
        if(ap[i].plan.source!=(i<h->allocations?i+1:0)||
           ap[i].plan.generation!=(i<h->allocations?j->hl.serial+h->heaps+i+1:0))return HMP_FORMAT;
    }
    for(i=0;i<j->process_count;i++){
        if(!pp[i].plan.index){if(i<h->processes||!hmd_zero_row(pp+i,sizeof(HmdProcess)))return HMP_FORMAT;continue;}
        if(!hmx_mark(hmx_po_marks(h),PO_CAP,pp[i].plan.index)||pp[i].plan.source!=(i<h->processes?i+1:0)||
           pp[i].plan.generation!=(i<h->processes?j->po.serial+i+1:0))return HMP_FORMAT;
    }
    /* A LIVE process inside this physical domain must have a captured
     * binding. An outside process is preserved and never silently retired. */
    for(k=0;k<2;k++){const GhcProcessDirectory*d=k?cd:pd;const GhcProcessRow*r=k?cp:p;
        for(i=0;i<d->count;i++)if(r[i].process.state!=PO_ENDED&&
            ghc_contains(c->domain.start,c->domain.span,r[i].process.object,r[i].process.allocation_bytes)&&
            r[i].binding!=GHCP_BIND_LIVE)return HMP_FORMAT;}
    for(i=0;i<h->allocations;i++){
        if(a[i].allocation.state!=HL_LIVE)return HMP_FORMAT;
        at=(a[i].allocation.generation*2654435761u)&mask;
        for(k=0;k<=mask&&hash[at];k++,at=(at+1)&mask)
            if(a[hash[at]-1].allocation.generation==a[i].allocation.generation)return HMP_FORMAT;
        if(k>mask)return HMP_SPACE;hash[at]=i+1;
    }
    for(i=0;i<h->heaps;i++){
        HmxHeap *out=heaps+i;ghc_copy(&out->heap,rows[i].heap,sizeof(HlHeap));
        if(i+1==h->root){ghc_copy(&out->heap,&hp[i].heap,sizeof(HlHeap));ghc_copy(&out->scope,&hp[i].scope,sizeof(HmScope));}
        else{
            if(!rows[i].reserved||rows[i].reserved>HL_HEAPS||!(parent=map[rows[i].reserved-1]))return HMP_FORMAT;
            out->heap.generation=hp[i].plan.generation;
            if(out->heap.generation!=j->hl.serial+i+1)return HMP_FORMAT;
            out->heap.parent_generation=hp[parent-1].plan.generation;out->heap.epoch=j->hl.release_serial+i+1;
            out->heap.flags|=HL_REBORN;out->heap.born_frame=j->frame;
            ghc_copy(&out->scope,rows[i].scope,sizeof(HmScope));
            out->scope.parent_epoch=parent==h->root?hp[parent-1].heap.epoch:j->hl.release_serial+parent;
            out->scope.managed_generation=hmx_reference(a,j,h,out->scope.managed_pointer,out->scope.managed_generation);
            out->scope.header_generation=hmx_reference(a,j,h,out->scope.header_pointer,out->scope.header_generation);
            if(!out->scope.managed_generation||!out->scope.header_generation||out->scope.flags||out->scope.active_token)return HMP_FORMAT;
        }
        out->links.generation=out->heap.generation;
        if(out->heap.kind==HL_CUSTOM&&!hmx_custom(s,rows,a,j,h,i,out))return HMP_FORMAT;
    }
    for(i=0;i<h->allocations;i++){
        HlAllocation next;HmxAllocation *out=allocs+i;HmxHeap *heap;
        ghc_copy(&next,&a[i].allocation,sizeof(next));
        ordinal=map[a[i].owner_slot-1];if(!ordinal)return HMP_FORMAT;heap=heaps+ordinal-1;
        next.generation=ap[i].plan.generation;next.heap_generation=heap->heap.generation;next.heap_epoch=heap->heap.epoch;
        next.flags|=256u;next.born_frame=j->frame;next.birth_site=0x484d5831u;
        next.end_frame=next.end_site=next.release_token=0;
        if(!hl_import(&out->allocation,&next))return HMP_FORMAT;
        out->link.owner=hp[ordinal-1].plan.index;out->link.generation=next.generation;
        old=heads[ordinal-1];out->link.next=old?ap[old-1].plan.index:0;
        if(old)allocs[old-1].link.previous=ap[i].plan.index;
        heads[ordinal-1]=i+1;heap->links.head=ap[i].plan.index;++heap->links.count;
    }
    for(i=0,k=0;i<pd->count;i++)if(p[i].binding==GHCP_BIND_LIVE){
        if(k>=h->processes||!hmx_source_allocation(a,h,p[i].allocation.pointer,p[i].allocation.generation))return HMP_FORMAT;
        ghc_copy(processes+k,&p[i].process,sizeof(PoEntry));processes[k].generation=pp[k].plan.generation;
        processes[k].born_frame=j->frame;processes[k].birth_caller=0x484d5831u;processes[k].end_frame=processes[k].end_caller=0;++k;
    }
    if(k!=h->processes)return HMP_FORMAT;
    ghc_copy(&h->hl,&j->hl,sizeof(h->hl));ghc_copy(&h->hi,&j->hi,sizeof(h->hi));
    ghc_copy(&h->po,&j->po,sizeof(h->po));
    h->hl.serial=h->end_serial;h->hl.release_serial=h->end_release;
    h->hl.active_heaps=h->hl.active_heaps-j->current_heaps+h->heaps;
    h->hl.active_allocations=h->hl.active_allocations-j->current_allocations+h->allocations;
    h->hi.nodes=h->hl.active_allocations;h->po.serial=h->end_po_serial;
    h->po.active=h->po.active-j->current_processes+h->processes;
    return h->hl.active_heaps<=HL_HEAPS&&h->hl.active_allocations<=HL_ALLOCS&&h->po.active<=PO_CAP?HMP_OK:HMP_SPACE;
}

GHC_FN void hmx_apply(const HmdHeader*j,HmxHeader*h) {
    const HmdHeap*hp=(const HmdHeap*)((u32)j+j->heap_offset);const HmdAllocation*ap=(const HmdAllocation*)((u32)j+j->allocation_offset);
    const HmdProcess*pp=(const HmdProcess*)((u32)j+j->process_offset);
    const HmxHeap*heaps=(const HmxHeap*)((u32)h+h->heap_offset);const HmxAllocation*a=(const HmxAllocation*)((u32)h+h->allocation_offset);
    const PoEntry*p=(const PoEntry*)((u32)h+h->process_offset);u32 i,id;
    /* Reserve all generations before the first row store. */
    _gz_hl_control.serial=h->end_serial;_gz_hl_control.release_serial=h->end_release;_gz_po_header.serial=h->end_po_serial;
    for(i=0;i<j->heap_count;i++){if(!hp[i].plan.index)continue;id=hp[i].plan.index-1;
        if(i<h->heaps){ghc_copy(_gz_hl_heaps+id,&heaps[i].heap,sizeof(HlHeap));ghc_copy(_gz_hm_scopes+id,&heaps[i].scope,sizeof(HmScope));
            ghc_copy(_gz_hl_custom_meta+id,&heaps[i].custom,sizeof(HlCustomMeta));ghc_copy(_gz_hi_heaps+id,&heaps[i].links,sizeof(HiHeap));}
        else{_gz_hl_heaps[id].state=HL_DEAD;_gz_hl_heaps[id].end_frame=j->frame;_gz_hl_heaps[id].dtor_site=0x484d5831u;
            _gz_hi_heaps[id].head=_gz_hi_heaps[id].count=0;}
    }
    for(i=0;i<j->allocation_count;i++){if(!ap[i].plan.index)continue;id=ap[i].plan.index-1;
        if(i<h->allocations){ghc_copy(_gz_hl_allocations+id,&a[i].allocation,sizeof(HlStoredAllocation));ghc_copy(_gz_hi_links+id,&a[i].link,sizeof(HiLink));}
        else{_gz_hl_allocations[id].state=HL_DEAD;_gz_hl_allocations[id].end_frame=j->frame;_gz_hl_allocations[id].end_site=0x484d5831u;
            _gz_hl_allocations[id].flags|=HL_HEAP_RETIRED;_gz_hl_allocations[id].release_token=0;zero(_gz_hi_links+id,sizeof(HiLink));}
    }
    for(i=0;i<j->process_count;i++){if(!pp[i].plan.index)continue;id=pp[i].plan.index-1;
        if(i<h->processes)ghc_copy(_gz_po_rows+id,p+i,sizeof(PoEntry));
        else{_gz_po_rows[id].state=PO_ENDED;_gz_po_rows[id].end_frame=j->frame;_gz_po_rows[id].end_caller=0x484d5831u;}
    }
    ghc_copy(&_gz_hl_control,&h->hl,sizeof(HlControl));ghc_copy(&_gz_hi_control,&h->hi,sizeof(HiControl));ghc_copy(&_gz_po_header,&h->po,sizeof(PoHeader));
}

GHC_FN u32 hmx_forward_matches(const HmdHeader*j,const HmxHeader*h) {
    const HmdHeap*hp=(const HmdHeap*)((u32)j+j->heap_offset);const HmdAllocation*ap=(const HmdAllocation*)((u32)j+j->allocation_offset);
    const HmdProcess*pp=(const HmdProcess*)((u32)j+j->process_offset);const HmxHeap*heaps=(const HmxHeap*)((u32)h+h->heap_offset);
    const HmxAllocation*a=(const HmxAllocation*)((u32)h+h->allocation_offset);const PoEntry*p=(const PoEntry*)((u32)h+h->process_offset);
    u32 i,id,node,previous,total=0,count;
    if(!ghc_equal(&_gz_hl_control,&h->hl,sizeof(HlControl))||!ghc_equal(&_gz_hi_control,&h->hi,sizeof(HiControl))||
       !ghc_equal(&_gz_po_header,&h->po,sizeof(PoHeader))||!ghc_hl_healthy()||!ghp_po_header())return 0;
    for(i=0;i<h->heaps;i++){if(!hp[i].plan.index)continue;id=hp[i].plan.index-1;
        if(!ghc_equal(_gz_hl_heaps+id,&heaps[i].heap,sizeof(HlHeap))||!ghc_equal(_gz_hm_scopes+id,&heaps[i].scope,sizeof(HmScope))||
           !ghc_equal(_gz_hl_custom_meta+id,&heaps[i].custom,sizeof(HlCustomMeta))||!ghc_equal(_gz_hi_heaps+id,&heaps[i].links,sizeof(HiHeap))||
           !_gz_hm_scope_live(_gz_hl_heaps+id))return 0;
        node=_gz_hi_heaps[id].head;previous=0;count=_gz_hi_heaps[id].count;
        for(u32 k=0;k<count;k++){if(!hi_member(node,id+1,_gz_hl_heaps[id].generation)||_gz_hi_links[node-1].previous!=previous)return 0;
            previous=node;node=_gz_hi_links[node-1].next;}if(node)return 0;total+=count;
    }
    if(total!=h->allocations)return 0;
    for(i=0;i<h->allocations;i++){if(!ap[i].plan.index)continue;id=ap[i].plan.index-1;
        if(!ghc_equal(_gz_hl_allocations+id,&a[i].allocation,sizeof(HlStoredAllocation))||!ghc_equal(_gz_hi_links+id,&a[i].link,sizeof(HiLink)))return 0;}
    for(i=0;i<h->processes;i++)if(!ghc_equal(_gz_po_rows+pp[i].plan.index-1,p+i,sizeof(PoEntry))||!ghp_po_entry(_gz_po_rows+pp[i].plan.index-1))return 0;
    for(i=h->heaps;i<j->heap_count;i++)if(hp[i].plan.index&&(_gz_hl_heaps[hp[i].plan.index-1].state!=HL_DEAD||_gz_hi_heaps[hp[i].plan.index-1].head||_gz_hi_heaps[hp[i].plan.index-1].count))return 0;
    for(i=h->allocations;i<j->allocation_count;i++)if(ap[i].plan.index&&(_gz_hl_allocations[ap[i].plan.index-1].state!=HL_DEAD||hi_active(_gz_hl_allocations+ap[i].plan.index-1)))return 0;
    for(i=h->processes;i<j->process_count;i++)if(pp[i].plan.index&&(_gz_po_rows[pp[i].plan.index-1].state!=PO_ENDED))return 0;
    return 1;
}

GHC_FN void hmx_inverse(const HmdHeader*j,const HmxHeader*h) {
    const HmdHeap*hp=(const HmdHeap*)((u32)j+j->heap_offset);const HmdAllocation*ap=(const HmdAllocation*)((u32)j+j->allocation_offset);
    const HmdProcess*pp=(const HmdProcess*)((u32)j+j->process_offset);HlControl hl;PoHeader po;u32 i,id;
    ghc_copy(&hl,&j->hl,sizeof(hl));ghc_copy(&po,&j->po,sizeof(po));
    for(i=0;i<j->heap_count;i++){if(!hp[i].plan.index)continue;id=hp[i].plan.index-1;ghc_copy(_gz_hl_heaps+id,&hp[i].heap,sizeof(HlHeap));
        ghc_copy(_gz_hm_scopes+id,&hp[i].scope,sizeof(HmScope));ghc_copy(_gz_hl_custom_meta+id,&hp[i].custom,sizeof(HlCustomMeta));ghc_copy(_gz_hi_heaps+id,&hp[i].links,sizeof(HiHeap));}
    for(i=0;i<j->allocation_count;i++){if(!ap[i].plan.index)continue;id=ap[i].plan.index-1;ghc_copy(_gz_hl_allocations+id,&ap[i].allocation,sizeof(HlStoredAllocation));ghc_copy(_gz_hi_links+id,&ap[i].link,sizeof(HiLink));}
    for(i=0;i<j->process_count;i++)if(pp[i].plan.index)ghc_copy(_gz_po_rows+pp[i].plan.index-1,&pp[i].process,sizeof(PoEntry));
    hl.serial=h->end_serial;hl.release_serial=h->end_release;po.serial=h->end_po_serial;
    ghc_copy(&_gz_hl_control,&hl,sizeof(hl));ghc_copy(&_gz_hi_control,&j->hi,sizeof(HiControl));ghc_copy(&_gz_hm_control,&j->hm,sizeof(HmControl));ghc_copy(&_gz_po_header,&po,sizeof(po));
}

GHC_FN u32 hmx_roundtrip_or_commit(u32 req,u32 f,u32 keep) {
    const OisHeldView*v=&_gz_ois_held_record;const GhcHeader*s,*c;HmdHeader*j;HmxHeader shape,*h;u32 base,result,verified;
    j=hmx_journal(req,f,GHW_VERIFY);if(!j)return HMP_SCOPE;
    s=(const GhcHeader*)v->saved.archive;c=(const GhcHeader*)v->current.archive;
    if(!_gz_ghc_held_domain(req,f,c->domain.start,c->domain.span)||!hmx_shape(j,&shape))return HMP_SPACE;
    base=_gz_ois_held_work(req,f,v,j->bytes+shape.bytes);if(base!=(u32)j)return HMP_SPACE;
    h=(HmxHeader*)(base+j->bytes);zero(h,shape.bytes);ghc_copy(h,&shape,sizeof(shape));
    h->request=req;h->frame=f;h->hg=v->hg;h->job=v->job_generation;h->source_checksum=s->checksum;
    if(!ghp_po_lock())return HMP_CHANGED;if(!ghc_hl_lock()){ghp_po_unlock();return HMP_CHANGED;}
    result=hmx_prepare_locked(s,c,j,h);
    if(result){ghc_hl_unlock();ghp_po_unlock();return result;}
    /* Both mutexes remain held until a verified commit or completed
     * inverse. Issued generation/epoch cursors never move backward. */
    h->state=2;h->dirty=1;j->reserved[0]=1;
    j->reserved[1]=h->end_serial;j->reserved[2]=h->end_release;j->reserved[3]=h->end_po_serial;
    hmx_apply(j,h);verified=hmx_forward_matches(j,h);h->applied_verified=verified;
    if(keep==1u&&verified){
        /* Actual desired rows have verified while both metadata locks
         * remain held. This commits CPU metadata only, not device or
         * external-reference completeness. No snapshot supplies keep. */
        j->reserved[0]=3;j->checksum=hmd_hash(j);h->dirty=0;h->state=5;
        ghc_hl_unlock();ghp_po_unlock();return HMP_CPU_COMMITTED;
    }
    hmx_inverse(j,h);j->reserved[0]=2;j->checksum=hmd_hash(j);
    h->inverse_verified=hmd_before_equal(j);
    if(!h->inverse_verified){h->error=HMP_RT_DIRTY;return HMP_RT_DIRTY;}
    h->dirty=0;h->state=4;ghc_hl_unlock();ghp_po_unlock();
    return verified?HMP_RT_OK:HMP_RT_RECOVERED;
}

GHC_API u32 _gz_hmp_roundtrip_held(u32 req,u32 f) {
    return hmx_roundtrip_or_commit(req,f,0);
}
GHC_API u32 _gz_hmp_commit_held(u32 req,u32 f) {
    return hmx_roundtrip_or_commit(req,f,1);
}
