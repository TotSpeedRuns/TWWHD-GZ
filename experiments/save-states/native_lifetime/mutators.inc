/* Common native heap mutation observations; no native payload replay/API calls. */
HmScope _gz_hm_scopes[HL_HEAPS];
HmRecord _gz_hm_records[HM_CAP];
HmControl _gz_hm_control={.magic=HM_MAGIC,.bytes=64,.version=1};
static u32 hm_index(HlHeap*h){return (u32)(h-_gz_hl_heaps);}
static u32 hm_active(HlHeap*h){return h&&(h->state==HL_LIVE||h->state==HL_MUTATING);}
static HlAllocation*hm_anchor(u32 p,u32 n,u32 heap){u32 i;HlAllocation*a,*found=0;HlHeap*h;
 a=heap?allocation_find(p,heap,0):allocation_unique(p);
 if(a&&a->state==HL_LIVE&&within(a->pointer,a->capacity,p,n)){
  h=heap_find(a->heap,0);
  if(hm_active(h)&&h->generation==a->heap_generation&&h->epoch==a->heap_epoch)return a;
 }
 for(i=0;i<HL_ALLOCS;i++){
  a=&_gz_hl_allocations[i];if(a->state!=HL_LIVE||(heap&&a->heap!=heap)||!within(a->pointer,a->capacity,p,n))continue;
  h=heap_find(a->heap,0);if(!hm_active(h)||h->generation!=a->heap_generation||h->epoch!=a->heap_epoch)continue;
  if(found)return 0;found=a;
 }return found;
}
void _gz_hm_bind_heap(HlHeap*h){HmScope*s=&_gz_hm_scopes[hm_index(h)];HlHeap*parent;HlAllocation*a;
 zero(s,sizeof(*s));
 if(h->kind==HL_CUSTOM){HlCustomMeta*c=&_gz_hl_custom_meta[hm_index(h)];
  c->anchor_pointer=c->anchor_generation=0;
  if(c->heap_generation==h->generation&&c->slots&&c->slots<=0x0fffffffu){
   a=hm_anchor(c->table,c->slots*16,0);
   if(a){c->anchor_pointer=a->pointer;c->anchor_generation=a->generation;}
  }
 }
 if(!h->parent)return;
 parent=heap_find(h->parent,0);if(!hm_active(parent)||parent->generation!=h->parent_generation){s->flags=1;return;}
 s->parent_epoch=parent->epoch;
 a=hm_anchor(h->start,h->bytes,h->parent);
 if(a){s->managed_pointer=a->pointer;s->managed_generation=a->generation;}
 if(a&&within(a->pointer,a->capacity,h->heap,header_bytes(h->kind))){
  s->header_pointer=a->pointer;s->header_generation=a->generation;
 }else {
  a=hm_anchor(h->heap,header_bytes(h->kind),0);
  if(a){s->header_pointer=a->pointer;s->header_generation=a->generation;}
 }
 if(!s->managed_pointer||!s->header_pointer)s->flags|=2;
}
static u32 hm_scope_live(HlHeap*h,u32 depth,u32*budget){HmScope*s;HlHeap*parent,*header_owner;HlAllocation*m,*header;
 for(;depth<32;depth++){
  if(!*budget)return 0;--*budget;
  if(!h||h->state!=HL_LIVE)return 0;
  if(h->kind==HL_CUSTOM){HlCustomMeta*c=&_gz_hl_custom_meta[hm_index(h)];HlAllocation*d;HlHeap*owner;
   if(c->heap_generation!=h->generation||!c->slots||c->slots>0x0fffffffu)return 0;
   d=allocation_generation(c->anchor_pointer,c->anchor_generation);
   if(!d||d->state!=HL_LIVE||!within(d->pointer,d->capacity,c->table,c->slots*16))return 0;
   owner=heap_find(d->heap,0);
   if(!owner||owner==h||owner->state!=HL_LIVE||owner->generation!=d->heap_generation||owner->epoch!=d->heap_epoch||!hm_scope_live(owner,depth+1,budget))return 0;
  }
  if(!h->parent)return 1;
  s=&_gz_hm_scopes[hm_index(h)];parent=heap_find(h->parent,0);
  if(s->flags||!parent||parent->state!=HL_LIVE||parent->generation!=h->parent_generation||parent->epoch!=s->parent_epoch)return 0;
  m=allocation_generation(s->managed_pointer,s->managed_generation);header=allocation_generation(s->header_pointer,s->header_generation);
  if(!m||!header||m->state!=HL_LIVE||header->state!=HL_LIVE||m->generation!=s->managed_generation||header->generation!=s->header_generation||
   m->heap!=parent->heap||m->heap_generation!=parent->generation||m->heap_epoch!=parent->epoch||
   !within(m->pointer,m->capacity,h->start,h->bytes)||!within(header->pointer,header->capacity,h->heap,header_bytes(h->kind)))return 0;
  header_owner=heap_find(header->heap,0);
  if(!header_owner||header_owner->state!=HL_LIVE||header_owner->generation!=header->heap_generation||header_owner->epoch!=header->heap_epoch)return 0;
  /* A separate header may belong to another heap. Its own backing storage
   * chain must survive as well; cycles/deep graphs fail within the same bound. */
  if(header_owner!=parent&&!hm_scope_live(header_owner,depth+1,budget))return 0;
  h=parent;
 }return 0;
}
u32 _gz_hm_scope_live(HlHeap*h){u32 budget=64;return hm_scope_live(h,0,&budget);}
static void hm_retire(HlAllocation*a,u32 site,u32 reason){
 if(a->state!=HL_LIVE&&a->state!=HL_ENDING)return;
 a->end_frame=_gz_hl_frame;a->end_site=site;a->flags|=reason;a->state=HL_DEAD;
 if(_gz_hl_control.active_allocations)--_gz_hl_control.active_allocations;else flag(HL_BAD_HEADER);
 bump(&_gz_hm_control.retired);
}
static void hm_retire_heap(HlHeap*h,u32 site){u32 i;
 for(i=0;i<HL_ALLOCS;i++)if(_gz_hl_allocations[i].heap==h->heap&&_gz_hl_allocations[i].heap_generation==h->generation)
  hm_retire(&_gz_hl_allocations[i],site,HM_ARENA_RETIRED);
}
static u32 hm_reset(u32 op){return op==HM_EXP_RESET||op==HM_SOLID_RESET||op==HM_UNIT_INITIALIZE||op==HM_UNIT_RESET||op==HM_CUSTOM_RESET;}
static u32 hm_kind(u32 op){return op>=HM_CUSTOM_RESET&&op<=HM_CUSTOM_BACK?HL_CUSTOM:op==HM_UNIT_INITIALIZE||op==HM_UNIT_RESET?HL_UNIT:
 op==HM_EXP_RESET||op==HM_EXP_FRONT||op==HM_EXP_BACK||op==HM_EXP_ADJUST_FRONT||op==HM_EXP_ADJUST_BACK?HL_EXP:HL_SOLID;}
static u32 hm_header(void){return _gz_hm_control.magic==HM_MAGIC&&_gz_hm_control.bytes==64&&_gz_hm_control.version==1&&!_gz_hm_control.reserved;}
/* Match the native Custom find method's first descriptor at this user
 * pointer, bounded to observed descriptor storage and a fixed visit budget. */
static u32 custom_descriptor(HlHeap*h,u32 pointer){HlCustomMeta*c=&_gz_hl_custom_meta[hm_index(h)];u32 count,node,previous,i,descriptor;
 if(c->heap_generation!=h->generation||!c->slots||c->slots>0x0fffffffu||R32(h->heap,0xa4)!=c->table||R32(h->heap,0xa8)!=c->slots)return 0;
 count=R32(h->heap,0x9c);node=R32(h->heap,0x98);previous=h->heap+0x94;
 if(count>c->slots||count>HL_ALLOCS)return 0;
 for(i=0;i<count;i++){
  if(node<8)return 0;descriptor=node-8;
  if(!within(c->table,c->slots*16,descriptor,16)||(descriptor-c->table)%16||R32(descriptor,8)!=previous)return 0;
  if(R32(descriptor,0)==pointer)return descriptor;
  previous=node;node=R32(descriptor,12);
 }return 0;
}
u32 _gz_hm_begin(u32 heap,u32 op,u32 p,u32 request,u32 site){HlHeap*h;HlAllocation*a;HmScope*s;HmRecord*r;u32 token=0;
 if(!lock())return 0;bump(&_gz_hm_control.attempts);
 if(!hm_header()||!op||op>HM_CUSTOM_BACK){flag(HL_BAD_HEADER);goto done;}
 h=heap_find(heap,0);if(!h||h->state!=HL_LIVE||h->kind!=hm_kind(op)){flag(HL_UNKNOWN_HEAP);goto done;}
 s=&_gz_hm_scopes[hm_index(h)];if(s->active_token){flag(HL_REENTRY);goto done;}
 if(R32(heap,0xc)!=h->vtable||R32(heap,0x1c)!=h->start||R32(heap,0x20)!=h->bytes||R32(heap,0x24)!=h->parent||R32(heap,0x50)!=h->direction){flag(HL_BAD_BLOCK);goto done;}
 token=serial(&_gz_hl_control.release_serial);if(!token)goto done;r=&_gz_hm_records[token&(HM_CAP-1)];
 if(r->state==1){flag(HL_OVERFLOW);token=0;goto done;}
 zero(r,sizeof(*r));r->token=token;r->operation=op;r->heap=heap;r->heap_generation=h->generation;r->old_epoch=h->epoch;
 r->pointer=p;r->request=request;r->site=site;r->frame=_gz_hl_frame;r->old_start=h->start;r->old_bytes=h->bytes;
 r->old_head=R32(heap,0x94);r->old_tail=R32(heap,0x98);
 if(op==HM_EXP_FRONT||op==HM_EXP_BACK||op==HM_CUSTOM_FRONT||op==HM_CUSTOM_BACK){
  a=(op==HM_CUSTOM_FRONT||op==HM_CUSTOM_BACK)?custom_find(p,heap,custom_descriptor(h,p),0):allocation_find(p,heap,0);
  if(a&&a->state==HL_LIVE&&a->heap==heap&&a->heap_generation==h->generation&&a->heap_epoch==h->epoch){
   r->have_old=1;copy(&r->old,a,sizeof(*a));a->state=HL_ENDING;a->release_token=token;
  }
 }
 r->state=1;s->active_token=token;h->state=HL_MUTATING;
 if(hm_reset(op)){h->epoch=token;hm_retire_heap(h,site);}
 ++_gz_hm_control.active;_gz_hm_control.last_token=token;_gz_hm_control.last_heap=heap;_gz_hm_control.last_operation=op;
done:unlock();return token;
}
static u32 hm_block(HlHeap*h,u32 p,HlAllocation*r){u32 word,block,cap,pad;
 if(p<h->start||p>h->start+h->bytes||p<16||!within(h->start,h->bytes,p-4,4))return 0;
 word=R32(p,-4);block=word&1?word-1:p-16;
 if(!within(h->start,h->bytes,block,16)||block+16>p)return 0;
 cap=R32(block,8);pad=R32(block,12);
 if(pad!=p-block-16||cap>h->start+h->bytes-p)return 0;
 r->block=block;r->capacity=cap;r->padding=pad;return 1;
}
static void hm_rebind_retained(HlAllocation*old,HlAllocation*now){u32 i;HlHeap*h;HmScope*s;
 for(i=0;i<HL_HEAPS;i++){
  h=&_gz_hl_heaps[i];if(!hm_active(h))continue;s=&_gz_hm_scopes[i];
  if(h->parent==old->heap&&s->managed_pointer==old->pointer&&s->managed_generation==old->generation&&
   within(now->pointer,now->capacity,h->start,h->bytes)){s->managed_pointer=now->pointer;s->managed_generation=now->generation;}
  if(h->kind==HL_CUSTOM){HlCustomMeta*c=&_gz_hl_custom_meta[i];
   if(c->anchor_pointer==old->pointer&&c->anchor_generation==old->generation&&c->slots<=0x0fffffffu&&
    within(now->pointer,now->capacity,c->table,c->slots*16)){c->anchor_pointer=now->pointer;c->anchor_generation=now->generation;}
  }
  if(s->header_pointer==old->pointer&&s->header_generation==old->generation&&
   within(now->pointer,now->capacity,h->heap,header_bytes(h->kind))){s->header_pointer=now->pointer;s->header_generation=now->generation;}
 }
}
#include "custom-resize.inc"
static u32 hm_resize(HmRecord*r,HlHeap*h,u32 returned){HlAllocation*a,*out,now;u32 gen;
 a=r->have_old?allocation_find(r->old.pointer,r->heap,0):0;
 if(r->have_old&&(!a||a->generation!=r->old.generation||a->state!=HL_ENDING||a->release_token!=r->token))return 0;
 if(!returned){if(a)copy(a,&r->old,sizeof(*a));bump(&_gz_hm_control.noops);return 1;}
 zero(&now,sizeof(now));now.pointer=returned;now.kind=HL_EXP;now.heap=h->heap;now.heap_generation=h->generation;
 now.heap_epoch=h->epoch;now.requested=r->request;now.alignment=r->have_old?r->old.alignment:0;now.missing=HL_MISSING_ALL;
 now.born_frame=_gz_hl_frame;now.birth_site=r->site;now.flags=HM_RESIZED;
 if(!hm_block(h,returned,&now))return 0;
 if(a&&returned==r->old.pointer&&now.block==r->old.block&&now.capacity==r->old.capacity&&now.padding==r->old.padding){
  copy(a,&r->old,sizeof(*a));bump(&_gz_hm_control.noops);return 1;
 }
 if(a)hm_retire(a,r->site,HM_RESIZED);else flag(HL_UNKNOWN_ALLOCATION);
 out=allocation_find(returned,h->heap,1);if(!out)return 0;
 gen=serial(&_gz_hl_control.serial);if(!gen)return 0;
 if(out->pointer==returned&&(out->state==HL_LIVE||out->state==HL_ENDING))flag(HL_REPLACED);else ++_gz_hl_control.active_allocations;
 now.generation=gen;now.state=HL_LIVE;copy(out,&now,sizeof(now));
 if(a)hm_rebind_retained(&r->old,out);bump(&_gz_hm_control.changed);return 1;
}
static u32 hm_solid_filter(HlHeap*h,u32 site){u32 i,head,tail,low,high;HlAllocation*a;
 if(h->bytes<0x9c)return 0;head=R32(h->heap,0x94);tail=R32(h->heap,0x98);
 low=h->start+(h->direction==1?0x9c:0);high=h->start+h->bytes-(h->direction==1?0:0x9c);
 if(head<low||head>tail||tail>high)return 0;
 for(i=0;i<HL_ALLOCS;i++){
  a=&_gz_hl_allocations[i];if(a->state!=HL_LIVE||a->heap!=h->heap||a->heap_generation!=h->generation)continue;
  if(!((a->pointer>=low&&a->pointer<=head&&a->capacity<=head-a->pointer)||
   (a->pointer>=tail&&a->pointer<=high&&a->capacity<=high-a->pointer)))hm_retire(a,site,HM_PARTIAL_RETIRED);
 }return 1;
}
void _gz_hm_finish(u32 token,u32 returned){HmRecord*r;HlHeap*h;HmScope*s;u32 start,bytes,result=1;
 if(!token)return;if(!lock())return;r=&_gz_hm_records[token&(HM_CAP-1)];
 if(!hm_header()||r->token!=token||r->state!=1){flag(HL_STALE_COMPLETION);unlock();return;}
 h=heap_find(r->heap,0);s=h?&_gz_hm_scopes[hm_index(h)]:0;r->returned=returned;
 if(!h||h->generation!=r->heap_generation||h->state!=HL_MUTATING||!s||s->active_token!=token){result=0;goto done;}
 start=R32(h->heap,0x1c);bytes=R32(h->heap,0x20);
 if(R32(h->heap,0xc)!=h->vtable||R32(h->heap,0x24)!=h->parent||R32(h->heap,0x50)!=h->direction||!span(start,bytes)){result=0;goto done;}
 h->start=start;h->bytes=bytes;
 if(hm_reset(r->operation)){
  hm_retire_heap(h,r->site);h->epoch=serial(&_gz_hl_control.release_serial);if(!h->epoch){result=0;goto done;}
 }else if(r->operation==HM_EXP_FRONT||r->operation==HM_EXP_BACK)result=hm_resize(r,h,returned);
 else if(r->operation==HM_CUSTOM_FRONT||r->operation==HM_CUSTOM_BACK)result=hm_custom_resize(r,h,returned);
 else if(h->kind==HL_SOLID)result=hm_solid_filter(h,r->site);
 /* Exp adjust retains native used allocations. Verify every observed body
  * still fits the new managed span; never guess a moved/expired pointer. */
 else {
  u32 i;HlAllocation*a;
  for(i=0;i<HL_ALLOCS;i++){
   a=&_gz_hl_allocations[i];if(a->state!=HL_LIVE||a->heap!=h->heap||a->heap_generation!=h->generation)continue;
   if(!within(h->start,h->bytes,a->pointer,a->capacity)){hm_retire(a,r->site,HM_PARTIAL_RETIRED);result=0;}
  }
 }
 if(result){h->state=HL_LIVE;s->active_token=0;_gz_hm_bind_heap(h);}
done:
 if(!result){flag(HL_STALE_COMPLETION);bump(&_gz_hm_control.failed);r->state=3;}
 else {r->state=2;bump(&_gz_hm_control.completed);}
 if(_gz_hm_control.active)--_gz_hm_control.active;else flag(HL_BAD_HEADER);
 _gz_hm_control.last_result=result;unlock();
}
