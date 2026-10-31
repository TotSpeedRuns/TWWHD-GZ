/* Only exact observed Custom descriptor identities may survive a resize.
 * The native function returns a pointer, including an unaligned/zero-length
 * endpoint for byte-size resizes; these are metadata, never actor authority. */
static u32 hm_custom_resize(HmRecord*r,HlHeap*h,u32 returned){HlStoredAllocation*a,*out,now;HlCustomMeta*c;u32 descriptor,capacity,pointer,gen;
 if(!r->have_old){if(!returned){bump(&_gz_hm_control.noops);return 1;}return 0;}
 a=custom_find(r->old.pointer,r->heap,r->old.block,0);
 if(!a||a->generation!=r->old.generation||a->state!=HL_ENDING||a->release_token!=r->token)return 0;
 c=&_gz_hl_custom_meta[hm_index(h)];descriptor=r->old.block;
 if(c->heap_generation!=h->generation||!c->slots||c->slots>0x0fffffffu||
  R32(h->heap,0xa4)!=c->table||R32(h->heap,0xa8)!=c->slots||
  !within(c->table,c->slots*16,descriptor,16)||(descriptor-c->table)%16)return 0;
 pointer=R32(descriptor,0);capacity=R32(descriptor,4);
 if(!returned){
  if(pointer!=r->old.pointer||capacity!=r->old.capacity)return 0;
  if(!hl_import(a,&r->old))return 0;bump(&_gz_hm_control.noops);return 1;
 }
 if(pointer!=returned||pointer<h->start||pointer>h->start+h->bytes||capacity>h->start+h->bytes-pointer)return 0;
 if(pointer==r->old.pointer&&capacity==r->old.capacity){if(!hl_import(a,&r->old))return 0;bump(&_gz_hm_control.noops);return 1;}
 zero(&now,sizeof(now));now.pointer=pointer;now.kind=HL_CUSTOM;now.heap=h->heap;now.heap_generation=h->generation;
 now.heap_epoch=h->epoch;now.requested=r->request;now.alignment=r->old.alignment;now.block=descriptor;now.capacity=capacity;
 now.missing=HL_MISSING_ALL;now.born_frame=_gz_hl_frame;now.birth_site=r->site;now.flags=HM_RESIZED;
 hm_retire(a,r->site,HM_RESIZED);out=custom_find(pointer,h->heap,descriptor,1);if(!out)return 0;
 gen=serial(&_gz_hl_control.serial);if(!gen)return 0;
 if(out->pointer==pointer&&(out->state==HL_LIVE||out->state==HL_ENDING))flag(HL_REPLACED);else ++_gz_hl_control.active_allocations;
 now.generation=gen;now.state=HL_LIVE;if(!hi_store(out,&now))return 0;hm_rebind_retained(&r->old,out);bump(&_gz_hm_control.changed);return 1;
}
