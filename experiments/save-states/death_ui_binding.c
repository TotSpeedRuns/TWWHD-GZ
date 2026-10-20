/* Current GameOver graphics topology under the existing real root/HG READ
 * domain. This is evidence for the forthcoming UI transaction, not a saved
 * object image or a permission to reset/free/call native graphics objects.
 * Missing31 remains on every allocation observation. No native byte store,
 * native callback, saved address or caller-selected pointer is accepted.
 */
enum {DUI_CONTEXT=680,DUI_BINDING=681,DUI_LAYOUT=682,DUI_CHANGED=683,DUI_LIMIT=684};
enum {DUI_ROOT=1,DUI_ANIMATION=2,DUI_WINDOW=3,DUI_SCREEN=4,DUI_ARRAY=5,DUI_CHILD=6,DUI_EFFECT=7,DUI_NODES=53};
typedef struct {
 u32 role,pointer,span,vtable;
 HlAllocation allocation;HlHeap heap;
 u8 image[0x258];
} DuiNode;
typedef struct {
 u32 magic,bytes,version,valid,nonce,sequence,frame,epoch,stage0,stage1,room,layer,root,kind,nodes,complete;
 DuiNode node[DUI_NODES];u32 checksum;
} DuiRecord;
typedef char DuiNodeSize[sizeof(DuiNode)==760?1:-1];
typedef char DuiRecordSize[sizeof(DuiRecord)==40348?1:-1];
static DuiRecord _gz_dui_scratch;
DuiRecord _gz_dui_record;
volatile u32 _gz_dui_enabled;
u32 _gz_dui_report[16]={0x44554931u,64};
static u32 dui_private_apart(u32 p,u32 n){return dto_native(p,n)&&
 !(p<(u32)&_gz_dui_scratch+sizeof(_gz_dui_scratch)&&(u32)&_gz_dui_scratch<p+n)&&
 !(p<(u32)&_gz_dui_record+sizeof(_gz_dui_record)&&(u32)&_gz_dui_record<p+n)&&
 !(p<(u32)_gz_dui_report+64&&(u32)_gz_dui_report<p+n);}
static u32 dui_scope(u32 frame){return _gz_dui_enabled==1&&_gz_dto_kernel_scope(frame)&&
 _gz_dth_read_scope(frame)&&_gz_dto_scratch.magic==DTO_MAGIC&&
 _gz_dto_scratch.frame==frame&&_gz_dto_scratch.request_nonce==_gz_dto_active&&
 _gz_dto_scratch.kind>=DTO_BEFORE_REQUEST&&_gz_dto_scratch.kind<=DTO_FINISHING;}
static u32 dui_hash(const DuiRecord*p){const u8*b=(const u8*)p;u32 i,h=2166136261u;
 for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=12&&i<16)?0:b[i]))*16777619u;return h;}
/* A JPA emitter prefix can be an interior view of its pool allocation.
 * Bind that entire current owner independently; the child pointer alone is
 * not an allocation/lifetime receipt. The complete owner query also rejects
 * ambiguous overlapping allocations. No emitter generation, JPA idle boundary or replay capability is
 * inferred from these bytes. Default-disabled diagnostic path only. */
static u32 dui_effect_allocation(u32 p,u32 n,HlAllocation*a,HlHeap*h){
 const HlObservation*o=&_gz_hl_observation;u32 base,offset;
 if(!dto_native(p,n)||_gz_hl_owner(p,n)||o->valid!=1||o->flags)return DUI_BINDING;
 base=o->allocation.pointer;
 if(!base||base>p)return DUI_BINDING;offset=p-base;
 if(!o->allocation.generation||o->allocation.state!=HL_LIVE||o->allocation.flags||
  o->allocation.end_frame||o->allocation.release_token||o->allocation.requested<n||
  offset>o->allocation.requested-n||o->allocation.capacity<n||offset>o->allocation.capacity-n||
  o->allocation.missing!=HL_MISSING_ALL||o->allocation.reserved||
  !dto_native(base,o->allocation.requested)||!o->heap.generation||o->heap.state!=HL_LIVE||
  (o->heap.flags&~HL_REBORN)||o->heap.end_frame||(o->heap.kind!=HL_EXP&&o->heap.kind!=HL_SOLID)||
  o->allocation.heap!=o->heap.heap||o->allocation.heap_generation!=o->heap.generation||
  o->allocation.heap_epoch!=o->heap.epoch)return DUI_BINDING;
 btr_copy(a,&o->allocation,sizeof(*a));btr_copy(h,&o->heap,sizeof(*h));return 0;
}
static u32 dui_allocation(u32 role,u32 p,u32 n,HlAllocation*a,HlHeap*h){
 return role==DUI_EFFECT?dui_effect_allocation(p,n,a,h):dto_allocation(p,n,a,h);
}
static u32 dui_add(u32 role,u32 a,u32 n,u32 vtable,u32 exact){DuiRecord*p=&_gz_dui_scratch;
 DuiNode*d;u32 i,z;
 if(!dui_scope(p->frame)||!n||n>0x258||p->nodes>=DUI_NODES)return DUI_CONTEXT;
 if(!dui_private_apart(a,n))return DUI_BINDING;
 for(i=0;i<p->nodes;i++)if(a<p->node[i].pointer+p->node[i].span&&p->node[i].pointer<a+n){
  /* Two native hide edges may intentionally name the same emitter prefix. Retain one
   * independently bound node, while rejecting every other logical overlap. */
  if(role==DUI_EFFECT&&p->node[i].role==DUI_EFFECT&&a==p->node[i].pointer&&n==p->node[i].span)return 0;
  return DUI_BINDING;
 }
 d=p->node+p->nodes;z=dui_allocation(role,a,n,&d->allocation,&d->heap);if(z)return DUI_BINDING;
 if(exact&&d->allocation.requested!=exact)return DUI_LAYOUT;
 d->role=role;d->pointer=a;d->span=n;
 btr_copy(d->image,(const void*)a,n);
 if(vtable&&R32(d->image,4)!=vtable)return DUI_LAYOUT;
 if(vtable)d->vtable=vtable;
 else if(role==DUI_SCREEN)d->vtable=R32(d->image,0xe0);
 else if(role==DUI_CHILD)d->vtable=R32(d->image,4);
 p->nodes++;return 0;
}
static u32 dui_current(void){DuiRecord*p=&_gz_dui_scratch;HlAllocation a;HlHeap h;u32 i,z;
 if(!dui_scope(p->frame)||p->root!=R32(0x101f835cu,0)||p->epoch!=_gz_pbac_epoch||
  p->stage0!=_gz_pbac_stage[0]||p->stage1!=_gz_pbac_stage[1]||p->room!=_gz_pbac_room||p->layer!=_gz_pbac_layer)return DUI_CHANGED;
 for(i=0;i<p->nodes;i++){const DuiNode*d=p->node+i;
  if(!dui_private_apart(d->pointer,d->span))return DUI_BINDING;
  z=dui_allocation(d->role,d->pointer,d->span,&a,&h);if(z)return DUI_CHANGED;
  if(!btr_equal(&a,&d->allocation,sizeof(a))||!btr_equal(&h,&d->heap,sizeof(h))||
   !btr_equal(d->image,(const void*)d->pointer,d->span))return DUI_CHANGED;
 }
 return dui_scope(p->frame)?0:DUI_CONTEXT;
}
u32 _gz_dui_read_under_root(u32 frame){DuiRecord*p=&_gz_dui_scratch;DuiRecord*out=&_gz_dui_record;
 const DtoRecord*t=&_gz_dto_scratch;u32 root,animation,window,screen,array,count,child,effect,i,z;
 if(!dui_scope(frame))return DUI_CONTEXT;
 /* Invalidate a prior audit immediately on this new attempt. A stale record
  * cannot stand in for an object whose new generation has not been checked. */
 out->valid=0;eks_zero(p,sizeof(*p));p->magic=0x44555031u;p->bytes=sizeof(*p);p->version=2;
 p->nonce=_gz_dto_active;p->sequence=_gz_dto_count+1;p->frame=frame;p->epoch=_gz_pbac_epoch;
 p->stage0=_gz_pbac_stage[0];p->stage1=_gz_pbac_stage[1];p->room=_gz_pbac_room;p->layer=_gz_pbac_layer;
 p->kind=t->kind;p->root=root=R32(0x101f835cu,0);
 z=dui_add(DUI_ROOT,root,0x24c,0x1013ef28u,0x24c);if(z)return z;
 if(R32(root,8)!=root||R32(root,0x24)!=0x1013f6f8u)return DUI_LAYOUT;
 animation=R32(root,0x190);window=R32(root,0x1bc);
 z=dui_add(DUI_ANIMATION,animation,100,0x100f4e30u,100);if(z)return z;
 z=dui_add(DUI_WINDOW,window,220,0x101046d8u,220);if(z)return z;
 if(R8(animation,0x60)>1||R8(animation,0x61)>1)return DUI_LAYOUT;
 screen=R32(animation,0x50);count=R32(animation,0x54);array=R32(animation,0x58);
 if(count>16)return DUI_LIMIT;
 if(screen){z=dui_add(DUI_SCREEN,screen,0xe4,0,0);if(z)return z;}
 if(count){
  z=dui_add(DUI_ARRAY,array,count*4,0,0);if(z)return z;
  for(i=0;i<count;i++){child=R32(array,i*4);if(!child)continue;
   z=dui_add(DUI_CHILD,child,0x68,0,0);if(z)return z;
   /* Native GameOver child hide helpers0266C698/0266C6BC touch emitter max-frame+5C and
    * emitter flags+254. Observe both complete prefixes before any future journal is
    * allowed to invoke them. Current code remains a read-only topology map. */
   effect=R32(child,0x58);if(effect){z=dui_add(DUI_EFFECT,effect,0x258,0,0);if(z)return z;}
   effect=R32(child,0x5c);if(effect){z=dui_add(DUI_EFFECT,effect,0x258,0,0);if(z)return z;}
  }
 }else if(array)return DUI_LAYOUT;
 z=dui_current();if(z)return z;
 /* Top-level, immediate animation and its two child emitter edges only. Window
  * layouts, graphics resources and state-manager callbacks still need their
  * typed contracts. Never advertise this audit as complete ownership. */
 p->complete=0;p->checksum=dui_hash(p);btr_copy(out,p,sizeof(*out));
 __asm__ volatile("sync":::"memory");out->valid=1;return 0;
}
static void dui_observe(u32 frame){u32 z;
 if(_gz_dui_enabled!=1)return;
 z=_gz_dui_read_under_root(frame);_gz_dui_report[2]=frame;_gz_dui_report[3]=z;
 _gz_dui_report[5]=_gz_dto_active;_gz_dui_report[6]=_gz_dto_scratch.kind;
 if(!z){_gz_dui_report[4]++;_gz_dui_report[7]=_gz_dui_record.nodes;}
 /* A graphics diagnostic refusal never suppresses the original independent
  * death-transition capture or grants the caller a restore capability. */
}
