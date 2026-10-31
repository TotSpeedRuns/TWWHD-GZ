#include "lifetime.h"
#include "lifetime_storage.h"
#include "production_features.h"
#include "process_arena.h"
const u32 _gz_resource_capture_lab_enabled=GZ_RESOURCE_CAPTURE_LAB_ENABLED;
#define R32(p,o) (*(volatile const u32*)((p)+(o)))
HlControl _gz_hl_control={.magic=HL_MAGIC,.bytes=64,.version=1};
HlHeap _gz_hl_heaps[HL_HEAPS];
HlStoredAllocation _gz_hl_allocations[HL_ALLOCS];
HlObservation _gz_hl_observation;
HlAllocation _gz_hl_projected[2];
HlCustomMeta _gz_hl_custom_meta[HL_HEAPS];
static void flag(u32 f){__sync_fetch_and_or(&_gz_hl_control.flags,f);}
static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xc0000000u&&n&&n<=0xc0000000u-p;}
static u32 header_bytes(u32 kind){return kind==HL_EXP?0xb8:(kind==HL_UNIT||kind==HL_CUSTOM)?0xac:0x9c;}
/* Native Custom can manage non-CPU extents such as E000004C..E2800000.
 * This is a scalar extent check, never a native read/write permission. The
 * original span() remains required for native heap/descriptor headers and
 * CPU payload/owner-query admission. Avoid unsigned end-address wrap. */
static u32 extent(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&n&&n<=0xffffffffu-p;}
static u32 managed(u32 kind,u32 p,u32 n){return kind==HL_CUSTOM?extent(p,n):span(p,n);}
static u32 within(u32 start,u32 bytes,u32 p,u32 n){return extent(p,n)&&p>=start&&p-start<bytes&&n<=bytes-(p-start);}
static void zero(void*p,u32 n){u32 i;for(i=0;i<n/4;i++)((u32*)p)[i]=0;}
static void copy(void*d,const void*s,u32 n){u32 i;for(i=0;i<n/4;i++)((u32*)d)[i]=((const u32*)s)[i];}
/* Genuine coreinit imports. Their REL24 records are emitted by the native
 * payload builder; linker placeholder addresses are never called at runtime. */
extern u32 OSDisableInterrupts(void),OSRestoreInterrupts(u32);
static u32 _gz_hl_prior_interrupts;
/* Every owner of the common metadata lock uses this protocol, including the
 * save-root reader. Prevent same-core priority inversion while retaining the
 * atomic exclusion against the other two cores. No allocation, waiting API,
 * native heap operation or yielding is allowed inside the critical section. */
u32 _gz_hl_meta_try_lock(void){u32 prior=OSDisableInterrupts();
 if(__sync_bool_compare_and_swap(&_gz_hl_control.busy,0,1)){
  _gz_hl_prior_interrupts=prior;return 1;
 }
 OSRestoreInterrupts(prior);return 0;
}
void _gz_hl_meta_unlock(void){u32 prior=_gz_hl_prior_interrupts;
 __sync_synchronize();*(volatile u32*)&_gz_hl_control.busy=0;
 OSRestoreInterrupts(prior);
}
static u32 lock(void){HlControl*c=&_gz_hl_control;
 for(;;){
  if(_gz_hl_meta_try_lock())break;
  if(*(volatile u32*)&c->busy>1){flag(HL_BAD_HEADER);return 0;}
 }
 if(c->magic!=HL_MAGIC||c->bytes!=64||c->version!=1||c->active_heaps>HL_HEAPS||c->active_allocations>HL_ALLOCS){
  flag(HL_BAD_HEADER);_gz_hl_meta_unlock();return 0;
 }
 return 1;
}
static void unlock(void){_gz_hl_meta_unlock();}
static void bump(u32*p){if(*p==0xffffffffu)flag(HL_COUNTER);else ++*p;}
static u32 serial(u32*p){if(*p==0xffffffffu){flag(HL_EXHAUSTED);return 0;}return ++*p;}
static HlHeap*heap_find(u32 p,u32 create){u32 i,j,dead=HL_HEAPS,start=((p>>2)*0x9e3779b1u)>>20;HlHeap*e;
 for(i=0;i<HL_HEAPS;i++){
  j=(start+i)&(HL_HEAPS-1);e=&_gz_hl_heaps[j];if(e->heap==p)return e;
  if(e->state==HL_DEAD&&dead==HL_HEAPS)dead=j;
  if(!e->heap)return create?&_gz_hl_heaps[dead==HL_HEAPS?j:dead]:0;
 }return create&&dead<HL_HEAPS?&_gz_hl_heaps[dead]:0;
}
/* Native nested heaps can legitimately return identical user pointers. The
 * observation identity is (heap, pointer), never pointer alone. */
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
/* Custom's zero-byte extents can alias another descriptor in this same
 * heap. Key those observations by (heap,user pointer,descriptor), never erase
 * the adjacent live block. Generic heap+pointer inspection refuses ambiguity. */
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
static HlStoredAllocation*pair_unique(u32 p,u32 heap){u32 i,j,start=((p>>2)*0x9e3779b1u)>>14;HlStoredAllocation*e,*found=0;
 for(i=0;i<HL_ALLOCS;i++){j=(start+i)&(HL_ALLOCS-1);e=&_gz_hl_allocations[j];if(!e->pointer)break;
  if(e->pointer==p&&e->heap==heap){if(found)return 0;found=e;}
 }return found;
}
static HlStoredAllocation*release_find(u32 p,u32 heap,u32 token){u32 i,j,start=((p>>2)*0x9e3779b1u)>>14;HlStoredAllocation*e;
 for(i=0;i<HL_ALLOCS;i++){j=(start+i)&(HL_ALLOCS-1);e=&_gz_hl_allocations[j];if(!e->pointer)return 0;
  if(e->pointer==p&&e->heap==heap&&e->state==HL_ENDING&&e->release_token==token)return e;
 }return 0;
}
static HlStoredAllocation*allocation_generation(u32 p,u32 generation){u32 i,j,start=((p>>2)*0x9e3779b1u)>>14;HlStoredAllocation*e;
 for(i=0;i<HL_ALLOCS;i++){
  j=(start+i)&(HL_ALLOCS-1);e=&_gz_hl_allocations[j];if(!e->pointer)return 0;
  if(e->pointer==p&&e->generation==generation)return e;
 }return 0;
}
static HlStoredAllocation*allocation_unique(u32 p){u32 i,j,start=((p>>2)*0x9e3779b1u)>>14;HlStoredAllocation*e,*found=0;
 for(i=0;i<HL_ALLOCS;i++){
  j=(start+i)&(HL_ALLOCS-1);e=&_gz_hl_allocations[j];if(!e->pointer)break;
  if(e->pointer==p){if(found)return 0;found=e;}
 }return found;
}
/* Fixed projection output, never a caller-selected native address. This
 * leaf deliberately does not acquire the already-held observer lock. The
 * save-root reader holds that lock before the call and through both copies. */
u32 _gz_hl_project_locked(u32 heap,u32 pointer,u32 slot){HlStoredAllocation*a;
 if(slot>=2||_gz_hl_control.busy!=1||_gz_hl_control.magic!=HL_MAGIC||
    _gz_hl_control.bytes!=64||_gz_hl_control.version!=1||_gz_hl_control.flags||
    _gz_hl_control.active_heaps>HL_HEAPS||_gz_hl_control.active_allocations>HL_ALLOCS||
    !heap||!pointer)return 1;
 a=allocation_find(pointer,heap,0);if(!a)return 1;
 hl_export(&_gz_hl_projected[slot],a);return 0;
}
#include "hr_cost_diagnostic.inc"
#include "allocation-index.inc"
static void retire(HlHeap*h,u32 site){u32 i,next;HlStoredAllocation*a;
 if(!hi_ready(h))return;
 for(i=hi_head(h);i;i=next){
  a=&_gz_hl_allocations[i-1];next=hi_next(i);
  if(a->heap==h->heap&&(a->state==HL_LIVE||a->state==HL_ENDING)){
   if(!hi_detach(a))return;
   a->end_frame=_gz_hl_frame;a->end_site=site;a->flags|=HL_HEAP_RETIRED;a->state=HL_DEAD;
   if(_gz_hl_control.active_allocations)--_gz_hl_control.active_allocations;else flag(HL_BAD_HEADER);
  }
 }
}
void _gz_hl_heap_born(u32 p,u32 kind,u32 site){HlHeap r,*h,*parent;u32 vt,gen;
 if(!p)return;if(!lock())return;bump(&_gz_hl_control.births);
 if((kind!=HL_EXP&&kind!=HL_SOLID&&kind!=HL_UNIT&&kind!=HL_CUSTOM)||!span(p,header_bytes(kind)))goto bad;
 vt=kind==HL_EXP?(u32)_gz_hl_exp_table:kind==HL_UNIT?(u32)_gz_hl_unit_table:kind==HL_CUSTOM?(u32)_gz_hl_custom_table:(u32)_gz_hl_solid_table;
 zero(&r,sizeof(r));r.heap=p;r.kind=kind;r.vtable=R32(p,0xc);r.start=R32(p,0x1c);r.bytes=R32(p,0x20);
 r.parent=R32(p,0x24);r.direction=R32(p,0x50);r.born_frame=_gz_hl_frame;r.ctor_site=site;
 if(r.vtable!=vt||!managed(kind,r.start,r.bytes)||(r.direction!=1&&r.direction!=0xffffffffu))goto bad;
 parent=heap_find(r.parent,0);if(parent&&(parent->state==HL_LIVE||parent->state==HL_MUTATING))r.parent_generation=parent->generation;
 h=heap_find(p,1);if(!h){flag(HL_OVERFLOW);goto drop;}
 gen=serial(&_gz_hl_control.serial);if(!gen)goto drop;r.generation=gen;r.state=HL_LIVE;
 if(h->heap==p){retire(h,site);r.flags|=HL_REBORN;if(h->state==HL_LIVE||h->state==HL_MUTATING)flag(HL_REPLACED);}
 if(h->heap!=p||(h->state!=HL_LIVE&&h->state!=HL_MUTATING))++_gz_hl_control.active_heaps;
 copy(h,&r,sizeof(r));
 if(!hi_heap_born(h))goto drop;
 {HlCustomMeta*c=&_gz_hl_custom_meta[(u32)(h-_gz_hl_heaps)];zero(c,sizeof(*c));
  if(kind==HL_CUSTOM){u32 base=R32(p,0xa4),slots=R32(p,0xa8);
   c->heap_generation=gen;c->table=base;c->slots=slots;
   if(r.direction!=1||slots>0x0fffffffu||(slots&&!span(base,slots*16)))flag(HL_BAD_BLOCK);
  }
 }
 _gz_hm_bind_heap(h);_gz_gpa_hl_born(h,&_gz_hl_control);unlock();return;
bad:flag(HL_BAD_ARGUMENT);
drop:bump(&_gz_hl_control.dropped);unlock();
}
void _gz_hl_heap_ending(u32 p,u32 kind,u32 site){HlHeap*h;
 _gz_gpa_hl_ending(p,kind,site);
 if(!p)return;if(!lock())return;bump(&_gz_hl_control.heap_end);h=heap_find(p,0);
 if(!h||(h->state!=HL_LIVE&&h->state!=HL_MUTATING)||h->kind!=kind){flag(HL_UNKNOWN_HEAP);unlock();return;}
 retire(h,site);h->end_frame=_gz_hl_frame;h->dtor_site=site;h->state=HL_DEAD;
 if(_gz_hl_control.active_heaps)--_gz_hl_control.active_heaps;else flag(HL_BAD_HEADER);unlock();
}
static void allocated(u32 heap,u32 p,u32 n,u32 align,u32 kind,u32 descriptor){HlHeap*h;HlStoredAllocation r,*a;u32 norm,word,block,pad,cap,gen;
 if(!p)return;if(!lock())return;bump(&_gz_hl_control.allocations);
 if((kind!=HL_EXP&&kind!=HL_SOLID&&kind!=HL_UNIT&&kind!=HL_CUSTOM)||!span(heap,header_bytes(kind))||n>0xfffffffcu)goto bad;
 norm=((n<4?4:n)+3)&~3u;if(!norm||!managed(kind,p,norm))goto bad;
 zero(&r,sizeof(r));r.pointer=p;r.kind=kind;r.heap=heap;r.requested=n;r.alignment=align;
 r.born_frame=_gz_hl_frame;r.birth_site=kind==HL_EXP?(u32)_gz_hl_exp_alloc_site:kind==HL_UNIT?(u32)_gz_hl_unit_alloc_site:kind==HL_CUSTOM?(u32)_gz_hl_custom_alloc_site:(u32)_gz_hl_solid_alloc_site;r.missing=HL_MISSING_ALL;
 h=heap_find(heap,0);
 if(!h||(h->state!=HL_LIVE&&h->state!=HL_MUTATING)||h->kind!=kind){flag(HL_UNKNOWN_HEAP);r.flags|=HL_UNBOUND_HEAP;}
 else {
  r.heap_generation=h->generation;r.heap_epoch=h->epoch;if(h->state==HL_MUTATING)r.flags|=HM_BORN_DURING;
  if(!within(h->start,h->bytes,p,norm)){flag(HL_BAD_BLOCK);goto drop;}
  if(kind==HL_CUSTOM){
   HlCustomMeta*c=&_gz_hl_custom_meta[(u32)(h-_gz_hl_heaps)];
   if(c->heap_generation!=h->generation||!c->slots||c->slots>0x0fffffffu||
    R32(heap,0xa4)!=c->table||R32(heap,0xa8)!=c->slots||
    !within(c->table,c->slots*16,descriptor,16)||(descriptor-c->table)%16){flag(HL_BAD_BLOCK);goto drop;}
   cap=R32(descriptor,4);
   if(R32(descriptor,0)!=p||cap<norm||(cap&3)||!within(h->start,h->bytes,p,cap)){flag(HL_BAD_BLOCK);goto drop;}
   r.block=descriptor;r.capacity=cap;
  }
  else if(kind==HL_SOLID){r.block=p;r.capacity=norm;}
  else if(kind==HL_UNIT){
   u32 stride=R32(heap,0x94),base=R32(heap,0x98),bytes=R32(heap,0x9c),usable;
   if(stride<4||(stride&3)||stride<norm||!within(h->start,h->bytes,base,bytes)){flag(HL_BAD_BLOCK);goto drop;}
   usable=(bytes/stride)*stride;
   if(!within(base,usable,p,stride)||(p-base)%stride){flag(HL_BAD_BLOCK);goto drop;}
   r.block=p;r.capacity=stride;
  }
  else {
   if(p<16||!within(h->start,h->bytes,p-4,4)){flag(HL_BAD_BLOCK);goto drop;}
   word=R32(p,-4);block=(word&1)?word-1:p-16;
   if(!within(h->start,h->bytes,block,16)){flag(HL_BAD_BLOCK);goto drop;}
   cap=R32(block,8);pad=R32(block,12);
   if(pad>p-block||block+16>p||pad!=p-block-16||cap<norm||!within(h->start,h->bytes,p,cap)){
    flag(HL_BAD_BLOCK);goto drop;
   }r.block=block;r.capacity=cap;
  }
 }
 a=kind==HL_CUSTOM?custom_find(p,heap,descriptor,1):allocation_find(p,heap,1);if(!a){flag(HL_OVERFLOW);goto drop;}
 gen=serial(&_gz_hl_control.serial);if(!gen)goto drop;r.generation=gen;r.state=HL_LIVE;
 if(a->pointer==p&&(a->state==HL_LIVE||a->state==HL_ENDING)){flag(HL_REPLACED);r.flags|=HL_REBORN;}
 else ++_gz_hl_control.active_allocations;
 if(!hi_store(a,&r))goto drop;unlock();return;
bad:flag(HL_BAD_ARGUMENT);
drop:bump(&_gz_hl_control.dropped);unlock();
}
void _gz_hl_allocated(u32 heap,u32 p,u32 n,u32 align,u32 kind){allocated(heap,p,n,align,kind,0);}
void _gz_hl_custom_allocated(u32 heap,u32 p,u32 n,u32 align,u32 descriptor){allocated(heap,p,n,align,HL_CUSTOM,descriptor);}
u32 _gz_hl_release_begin(u32 heap,u32 block,u32 pointer,u32 site,u32 origin){HlStoredAllocation*a;u32 token=0;
 if(!lock())return 0;bump(&_gz_hl_control.release_begin);
 if(!((origin==0x484c4631u&&site==(u32)_gz_hl_free_accepted_return)||
      (origin==0x484c4633u&&site==(u32)_gz_hl_unit_free_site)||
      (origin==0x484c4634u&&site==(u32)_gz_hl_custom_free_site))){flag(HL_ORIGIN);goto done;}
 a=origin==0x484c4634u?custom_find(pointer,heap,block,0):allocation_find(pointer,heap,0);
 if(a&&a->heap==heap&&a->state==HL_DEAD&&(a->flags&HM_ARENA_RETIRED))goto done;
 if(!a||a->state!=HL_LIVE||a->heap!=heap||!a->block||a->block!=block||
    (origin==0x484c4631u?a->kind!=HL_EXP:origin==0x484c4634u?a->kind!=HL_CUSTOM:(a->kind!=HL_UNIT||block!=pointer))){flag(HL_UNKNOWN_ALLOCATION);goto done;}
 token=serial(&_gz_hl_control.release_serial);if(!token)goto done;
 a->release_token=token;a->end_site=site;a->state=HL_ENDING;
done:unlock();return token;
}
void _gz_hl_release_return(u32 heap,u32 p,u32 token){HlStoredAllocation*a;
 if(!token)return;if(!lock())return;bump(&_gz_hl_control.release_end);a=release_find(p,heap,token);
 if(!a||a->state!=HL_ENDING||a->heap!=heap||a->release_token!=token){flag(HL_STALE_COMPLETION);unlock();return;}
 if(!hi_detach(a)){unlock();return;}
 a->end_frame=_gz_hl_frame;a->flags|=HL_OBSERVED_RELEASE;a->state=HL_DEAD;
 if(_gz_hl_control.active_allocations)--_gz_hl_control.active_allocations;else flag(HL_BAD_HEADER);unlock();
}
static u32 lookup(u32 p,u32 heap,u32 descriptor){HlStoredAllocation*a;HlHeap*h;u32 result=1;_gz_hl_observation.valid=0;
 if(!p||!lock())return 1;a=descriptor?custom_find(p,heap,descriptor,0):heap?pair_unique(p,heap):allocation_unique(p);
 if(a){hl_export(&_gz_hl_observation.allocation,a);h=heap_find(a->heap,0);
  if(h)copy(&_gz_hl_observation.heap,h,sizeof(*h));else zero(&_gz_hl_observation.heap,sizeof(HlHeap));result=0;
 }
 _gz_hl_observation.result=result;_gz_hl_observation.flags=_gz_hl_control.flags;_gz_hl_observation.session=_gz_hl_control.session;
 if(a){__sync_synchronize();_gz_hl_observation.valid=1;}unlock();return result;
}
u32 _gz_hl_lookup(u32 p){return lookup(p,0,0);}
u32 _gz_hl_lookup_heap(u32 heap,u32 p){if(!heap){_gz_hl_observation.valid=0;return 1;}return lookup(p,heap,0);}
u32 _gz_hl_lookup_custom(u32 heap,u32 p,u32 descriptor){if(!heap||!descriptor){_gz_hl_observation.valid=0;return 1;}return lookup(p,heap,descriptor);}
static u32 descends(HlHeap*child,HlHeap*ancestor,HlStoredAllocation*outer){u32 i;HlHeap*h=child,*parent;
 if(!within(outer->pointer,outer->capacity,child->start,child->bytes))return 0;
 for(i=0;i<32;i++){
  if(!h->parent||!h->parent_generation)return 0;
  parent=heap_find(h->parent,0);
  if(!parent||parent->state!=HL_LIVE||parent->generation!=h->parent_generation)return 0;
  if(parent==ancestor)return 1;h=parent;
 }return 0;
}
#include "owner-index.inc"
u32 _gz_hl_owner(u32 p,u32 n){u32 result=1;HlStoredAllocation*found=0;HlHeap*owner=0;
 _gz_hl_observation.valid=0;if(!span(p,n)||!lock())return 1;
 result=owner_index_locked(p,n,&found,&owner);
 if(!result){hl_export(&_gz_hl_observation.allocation,found);copy(&_gz_hl_observation.heap,owner,sizeof(*owner));}
 _gz_hl_observation.result=result;_gz_hl_observation.flags=_gz_hl_control.flags;_gz_hl_observation.session=_gz_hl_control.session;
 if(!result){__sync_synchronize();_gz_hl_observation.valid=1;}unlock();return result;
}
#include "mutators.inc"
#include "resource_dependencies.h"
#include "rebind.inc"
#include "resource-dependencies.inc"
#if GZ_RESOURCE_CAPTURE_LAB_ENABLED
#include "source-directory.inc"
#include "resource-payload.inc"
#include "resource-references.inc"
#else
#include "source-resource-unavailable.inc"
#endif

extern volatile u32 _gz_ghc_report[32];
#include "object-binding.inc"

#if GZ_RESOURCE_CAPTURE_LAB_ENABLED
#include "source-storage.inc"
#endif

/* New try-only metadata observations; retained APIs above stay unchanged. */
#include "object-binding-try.inc"

/* Counted directory: two linear passes, one genuine trylock, no native bodies. */
#include "object-directory.inc"

/* UNADOPTED actor/scene root subset metadata source proposal. */
#include "heap_domain_cut.inc"
#include "resident_labels.inc"

#include "owned_heap_native.inc"
