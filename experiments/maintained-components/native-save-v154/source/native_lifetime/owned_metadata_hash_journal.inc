/* Fixed compiled projections of the actual private tables, under their
 * existing locks. The saved archive supplies keys, never destination slots.
 * A shadow insertion uses the real heap/allocation/PO probing rules.
 */
#include "metadata_hash_plan.h"

typedef struct {
    const GhcHeader *saved;
    const unsigned char *members;
    const u32 *po_sources;
    u32 kind;
} HmdHashContext;

GHC_FN u32 hmd_current_hash_cell(const void *input,u32 index,HhtCell *out) {
    const HmdHashContext *c=input;zero(out,sizeof(*out));
    if(c->kind==1){const HlHeap *r;
        if(index>=HL_HEAPS)return 0;r=_gz_hl_heaps+index;if(!r->heap)return 1;
        out->primary=r->heap;out->flags=c->members[index]||r->state==HL_DEAD?HHT_RETIRED:HHT_LIVE_OUTSIDE;
    }else if(c->kind==2){const HlStoredAllocation *r;
        if(index>=HL_ALLOCS)return 0;r=_gz_hl_allocations+index;if(!r->pointer)return 1;
        out->primary=r->pointer;out->secondary=r->heap;out->descriptor=r->kind==HL_CUSTOM?r->block:0;
        out->flags=(c->members[index]||r->state==HL_DEAD?HHT_RETIRED:HHT_LIVE_OUTSIDE)|(r->kind==HL_CUSTOM?HHT_EXTENDED:0);
    }else if(c->kind==3){const PoEntry*r;
        if(index>=PO_CAP)return 0;r=_gz_po_rows+index;if(!r->object)return 1;
        out->primary=r->object;out->flags=c->members[index]||r->state==PO_ENDED?HHT_RETIRED:HHT_LIVE_OUTSIDE;
    }else return 0;
    return 1;
}

GHC_FN u32 hmd_saved_hash_cell(const void *input,u32 index,HhtCell *out) {
    const HmdHashContext*c=input;zero(out,sizeof(*out));
    if(c->kind==1){const GhcHeapRow*r=(const GhcHeapRow*)((u32)c->saved+c->saved->table);
        if(index>=c->saved->count)return 0;out->primary=r[index].heap[0];
    }else if(c->kind==2){const GhcAllocationDirectory*d=hmd_allocations(c->saved);
        const GhcAllocationRow*r=(const GhcAllocationRow*)(d+1);const HlAllocation*a;
        if(index>=d->count)return 0;a=&r[index].allocation;
        out->primary=a->pointer;out->secondary=a->heap;out->descriptor=a->kind==HL_CUSTOM?a->block:0;
        out->flags=a->kind==HL_CUSTOM?HHT_EXTENDED:0;
    }else if(c->kind==3){const GhcProcessDirectory*d=(const GhcProcessDirectory*)((u32)c->saved+c->saved->reserved[0]);
        const GhcProcessRow*r=(const GhcProcessRow*)(d+1);
        if(index>=d->bound||c->po_sources[index]>=d->count)return 0;
        out->primary=r[c->po_sources[index]].process.object;
    }else return 0;
    return 1;
}

/* Capacity covers the UNION of source destinations and CURRENT retirements.
 * Unused capacity stays completely zero and is not a table destination.
 */
GHC_FN u32 hmd_build_locked(const GhcHeader*saved,const GhcHeader*current,HmdHeader*header) {
    const GhcAllocationDirectory*ca=hmd_allocations(current);
    const GhcAllocationRow*cr=(const GhcAllocationRow*)(ca+1);
    const GhcProcessDirectory*cp=(const GhcProcessDirectory*)((u32)current+current->reserved[0]);
    const GhcProcessDirectory*sp=(const GhcProcessDirectory*)((u32)saved+saved->reserved[0]);
    const GhcProcessRow*pr=(const GhcProcessRow*)(cp+1),*sr=(const GhcProcessRow*)(sp+1);
    const GhcHeapRow*ch=(const GhcHeapRow*)((u32)current+current->table);
    const GhcHeapRow*sh=(const GhcHeapRow*)((u32)saved+saved->table);
    const GhdCut*cut=(const GhdCut*)((u32)current+current->cut);GhdLabels labels;
    unsigned char *members=(unsigned char*)header+header->classes_offset;
    u32 *shadow=(u32*)(members+HL_ALLOCS),*dest=shadow+HL_ALLOCS,*po_sources=dest+hmd_max_sources(header);
    HmdHeap*heaps=(HmdHeap*)((u32)header+header->heap_offset);
    HmdAllocation*allocations=(HmdAllocation*)((u32)header+header->allocation_offset);
    HmdProcess*processes=(HmdProcess*)((u32)header+header->process_offset);
    HmdHashContext context={saved,members,po_sources,1};
    HhtInput input={hmd_current_hash_cell,hmd_saved_hash_cell,&context,&context};HhtSpec spec;
    u32 i,k,n,written,selected,checked=0,result;
    if(!ghp_po_header()||!ghc_hl_healthy()||!hi_header()||
       !ghc_equal(&cp->po_before,&_gz_po_header,sizeof(PoHeader))||!ghc_equal(&cp->hl_before,&_gz_hl_control,sizeof(HlControl)))return HMP_CHANGED;
    for(i=0;i<cp->count;i++)if(!ghc_equal(&pr[i].process,_gz_po_rows+pr[i].po_index,sizeof(PoEntry)))return HMP_CHANGED;
    ghc_copy(&labels,(const unsigned char*)current+192,sizeof(labels));selected=ghc_common_node(cut,&labels);
    if(!selected||ghc_common_allocations(cut,selected,current->domain.start,current->domain.span,
       (GhcAllocationDirectory*)ca,ca->count,2,&checked)||checked!=ca->count)return HMP_CHANGED;
    if(saved->count>0xffffffffu-_gz_hl_control.serial||header->saved_allocations>0xffffffffu-_gz_hl_control.serial-saved->count)return HMP_EXHAUSTED;
    header->hl_serial=_gz_hl_control.serial;header->po_serial=_gz_po_header.serial;

    zero(members,HL_ALLOCS);
    for(i=0;i<current->count;i++){
        u32 id=ch[i].slot;if(!id||id>HL_HEAPS||!ghc_equal(ch[i].heap,_gz_hl_heaps+id-1,sizeof(HlHeap))||
            !ghc_equal(ch[i].scope,_gz_hm_scopes+id-1,sizeof(HmScope)))return HMP_CHANGED;
        members[id-1]=1;
    }
    spec=(HhtSpec){HL_HEAPS,20,header->saved_heaps,HHT_PRIMARY_KEY};written=0;
    result=_gz_hht_plan(&spec,&input,shadow,dest,&written);
    if(result||written!=header->saved_heaps)return result?HMP_FORMAT:HMP_CHANGED;
    n=header->saved_heaps;
    for(i=0;i<n;i++){
        heaps[i].plan=(HmpSlot){dest[i],i+1,header->hl_serial+i+1};
        if(sh[i].heap[0]==saved->domain.heap){
            if(_gz_hl_heaps[dest[i]-1].heap!=current->domain.heap||_gz_hl_heaps[dest[i]-1].generation!=current->domain.heap_generation)return HMP_CHANGED;
            heaps[i].plan.generation=current->domain.heap_generation;
        }
    }
    for(i=0;i<current->count;i++)if(!shadow[ch[i].slot-1]){
        if(n>=header->heap_count)return HMP_SPACE;heaps[n++].plan.index=ch[i].slot;
    }
    for(i=0;i<n;i++){u32 id=heaps[i].plan.index-1;
        ghc_copy(&heaps[i].heap,_gz_hl_heaps+id,sizeof(HlHeap));ghc_copy(&heaps[i].scope,_gz_hm_scopes+id,sizeof(HmScope));
        ghc_copy(&heaps[i].custom,_gz_hl_custom_meta+id,sizeof(HlCustomMeta));ghc_copy(&heaps[i].links,_gz_hi_heaps+id,sizeof(HiHeap));
    }

    zero(members,HL_ALLOCS);for(i=0;i<ca->count;i++)members[cr[i].index-1]=1;
    context.kind=2;spec=(HhtSpec){HL_ALLOCS,14,header->saved_allocations,HHT_ALLOCATION_KEY};written=0;
    result=_gz_hht_plan(&spec,&input,shadow,dest,&written);
    if(result||written!=header->saved_allocations)return result?HMP_FORMAT:HMP_CHANGED;
    n=header->saved_allocations;
    for(i=0;i<n;i++)allocations[i].plan=(HmpSlot){dest[i],i+1,header->hl_serial+header->saved_heaps+i+1};
    for(i=0;i<ca->count;i++)if(!shadow[cr[i].index-1]){
        if(n>=header->allocation_count)return HMP_SPACE;allocations[n++].plan.index=cr[i].index;
    }
    for(i=0;i<n;i++){u32 id=allocations[i].plan.index-1;
        ghc_copy(&allocations[i].allocation,_gz_hl_allocations+id,sizeof(HlStoredAllocation));ghc_copy(&allocations[i].link,_gz_hi_links+id,sizeof(HiLink));
    }

    zero(members,HL_ALLOCS);k=0;
    for(i=0;i<cp->count;i++)if(pr[i].binding==GHCP_BIND_LIVE){members[pr[i].po_index]=1;++k;}
    if(k!=cp->bound)return HMP_CHANGED;
    for(i=0,k=0;i<sp->count;i++)if(sr[i].binding==GHCP_BIND_LIVE){if(k>=sp->bound)return HMP_FORMAT;po_sources[k++]=i;}
    if(k!=sp->bound)return HMP_FORMAT;
    if(sp->bound>0xffffffffu-header->po_serial)return HMP_EXHAUSTED;
    context.kind=3;spec=(HhtSpec){PO_CAP,19,header->saved_processes,HHT_PRIMARY_KEY};written=0;
    result=_gz_hht_plan(&spec,&input,shadow,dest,&written);
    if(result||written!=header->saved_processes)return result?HMP_FORMAT:HMP_CHANGED;
    n=header->saved_processes;
    for(i=0;i<n;i++)processes[i].plan=(HmpSlot){dest[i],i+1,header->po_serial+i+1};
    for(i=0;i<cp->count;i++)if(pr[i].binding==GHCP_BIND_LIVE&&!shadow[pr[i].po_index]){
        if(n>=header->process_count)return HMP_SPACE;processes[n++].plan.index=pr[i].po_index+1;
    }
    for(i=0;i<n;i++)ghc_copy(&processes[i].process,_gz_po_rows+processes[i].plan.index-1,sizeof(PoEntry));
    ghc_copy(&header->hl,&_gz_hl_control,sizeof(HlControl));ghc_copy(&header->hi,&_gz_hi_control,sizeof(HiControl));
    ghc_copy(&header->hm,&_gz_hm_control,sizeof(HmControl));ghc_copy(&header->po,&_gz_po_header,sizeof(PoHeader));
    return hmd_before_equal(header)?HMP_OK:HMP_CHANGED;
}
