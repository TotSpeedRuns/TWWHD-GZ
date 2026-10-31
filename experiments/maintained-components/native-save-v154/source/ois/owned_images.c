extern unsigned int _gz_rsa_private_apart(unsigned int,unsigned int);
extern unsigned int _gz_ghc_all_private_apart(unsigned int,unsigned int);
/* Derived from retained OSS storage.c: exact original pin matching, monotonic
 * identities, stale-release behavior and argument exclusion. Legacy OSS and
 * its complete-GZS validator remain unchanged. This owner captures SPB1/AJB1
 * through fixed producers, using separate actual Cafe allocations. */
#include "owned_images.h"
#include "owned_heap_dual_cut.h"
#include "owned_heap_native.h"
#include "owned_heap_held_view.h"
#include "production_features.h"
#include "joint.h"
#include "resource_payload.h"
#include "rebind.h"
#include "native_abi.h"
extern unsigned int so_active_write_apart(unsigned int,unsigned int);
extern unsigned int _gz_osa_private_apart(unsigned int,unsigned int);
typedef AgU32 U;typedef unsigned char B;
/* Keep legacy archives and the actual current producer format in sync.
 * Type admission does not replace image, allocation, identity or pin checks. */
static U ois_slot_type(U type){return (type>=1u&&type<=8u)||type==GHC_PROCESS_VERSION;}
extern void*MEMGetBaseHeapHandle(U);extern void*MEMAllocFromExpHeapEx(void*,U,int);extern void MEMFreeToExpHeap(void*,void*);
extern void*OSGetCurrentThread(void);
extern U MEMGetAllocatableSizeForExpHeapEx(void*,int);
typedef struct {U heap,pointer,capacity,identity;} Region;
typedef struct {Region region;U used,type,generation,readers;AfDomain domain;} Slot;
typedef struct {U active,slot;OisLease lease;} Pin;
typedef struct {U phase,token,code,slot,thread;OisLease source;Region undo,map;AjRestoreRequest request;AjRestoreResult result;} Tx;
typedef struct {U magic,version,bytes,serial,inverse,entered,thread,writer;Slot slots[OIS_SLOTS];Pin pins[OIS_PINS];Region staging;Tx tx;} State;
static State state={.magic=0x4f495331u,.version=1,.bytes=sizeof(State),.serial=1,.inverse=~1u};
static U retained_conflict;
static U ois_cut_pending(void);
static void ois_held_invalidate(void);
static U ois_cut_header(void);
static U ois_cut_external(U pointer, U bytes);
static U ois_cut_protected_pin(const OisLease *lease);
static const Region *ois_cut_region(void);
static const Region *ois_cut_work_region(void);
typedef char LeaseSize[sizeof(OisLease)==32?1:-1];typedef char InfoSize[sizeof(OisInfo)==128?1:-1];
static void copy(void*d,const void*s,U n){U i;for(i=0;i<n;i++)((B*)d)[i]=((const B*)s)[i];}
static void zero(void*d,U n){U i;for(i=0;i<n;i++)((B*)d)[i]=0;}
static U same(const void*a,const void*b,U n){U i;for(i=0;i<n;i++)if(((const B*)a)[i]!=((const B*)b)[i])return 0;return 1;}
static U apart(U p,U n,U q,U m){return n<=0xffffffffu-p&&m<=0xffffffffu-q&&(p+n<=q||q+m<=p);}
static U span(U p,U n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static U header(void){return !retained_conflict&&ois_cut_header()&&state.magic==0x4f495331u&&state.version==1&&state.bytes==sizeof(state)&&state.serial&&state.inverse==~state.serial&&state.entered<=1&&state.writer<=4&&(state.entered||(!state.thread&&!state.writer));}
static U ois_heap_pending(void);
static U lock(void){if(ois_heap_pending()||ois_cut_pending())return OIS_BUSY;if(!header())return OIS_CORRUPT;if(!__sync_bool_compare_and_swap(&state.entered,0,1))return OIS_BUSY;
 state.thread=(U)OSGetCurrentThread();if(!state.thread){state.entered=0;return OIS_CORRUPT;}return 0;
}
static U done(U z){state.writer=0;state.thread=0;__sync_synchronize();state.entered=0;return z;}
static U region_ok(const Region*r){return !r->pointer?!(r->heap|r->capacity|r->identity):r->heap&&r->identity&&span(r->pointer,r->capacity)&&!(r->pointer&31)&&!(r->capacity&31);}
static U external(U p,U n){U i;if(!ois_cut_external(p,n)||!_gz_ghc_all_private_apart(p,n)||!_gz_abi_metadata_span(p,n)||!apart(p,n,(U)&state,sizeof(state)))return 0;
 for(i=0;i<OIS_SLOTS;i++)if(state.slots[i].region.pointer&&!apart(p,n,state.slots[i].region.pointer,state.slots[i].region.capacity))return 0;
 if(state.staging.pointer&&!apart(p,n,state.staging.pointer,state.staging.capacity))return 0;
 if(state.tx.undo.pointer&&!apart(p,n,state.tx.undo.pointer,state.tx.undo.capacity))return 0;
 if(state.tx.map.pointer&&!apart(p,n,state.tx.map.pointer,state.tx.map.capacity))return 0;
 /* Original _gz_abi_metadata_span above authenticates any MEM1 stack.
  * A fully bounded low interval is disjoint from every high-DATA SO span.
  * High DATA still requires the complete physical SO/adapter exclusion. */
 return ((p<0x10000000u&&n<=0x10000000u-p)||_gz_osa_private_apart(p,n))&&
  _gz_hr_buffer_private((const void*)p,n);
}
static U contains(U base,U bytes,U p,U n){return p>=base&&n<=bytes&&p-base<=bytes-n;}
/* Fixed producer/application exceptions only. Published slot bytes are never
 * writable through any participating capture/metadata-output API, even when
 * no reader pin exists. No caller callback or archive field sets this mode. */
U _gz_image_write_allowed(U p,U n){U i,owned=0;
 if(!ois_cut_external(p,n)||!_gz_ghc_all_private_apart(p,n)||!so_active_write_apart(p,n))return 0;
 if(!header())return 0;
 if(state.entered&&state.thread!=(U)OSGetCurrentThread())return 0;
 if(!apart(p,n,(U)&state,sizeof(state)))return state.entered&&state.writer==3&&state.tx.phase==OIS_APPLYING&&contains((U)&state.tx.result,sizeof(state.tx.result),p,n);
 for(i=0;i<OIS_SLOTS;i++)if(state.slots[i].region.pointer&&!apart(p,n,state.slots[i].region.pointer,state.slots[i].region.capacity))return 0;
 if(state.staging.pointer&&!apart(p,n,state.staging.pointer,state.staging.capacity))return state.entered&&(state.writer==1||state.writer==2||state.writer==4)&&contains(state.staging.pointer,state.staging.capacity,p,n);
 if(state.tx.undo.pointer&&!apart(p,n,state.tx.undo.pointer,state.tx.undo.capacity))owned=contains(state.tx.undo.pointer,state.tx.undo.capacity,p,n);
 else if(state.tx.map.pointer&&!apart(p,n,state.tx.map.pointer,state.tx.map.capacity))owned=contains(state.tx.map.pointer,state.tx.map.capacity,p,n);
 else return 1;
 return owned&&state.entered&&state.writer==3&&state.tx.phase==OIS_APPLYING;
}
static U issue(U*out){if(state.serial==0xffffffffu)return OIS_EXHAUSTED;*out=++state.serial;state.inverse=~state.serial;return 0;}
static void release_region(Region*r){Region old;copy(&old,r,sizeof(old));zero(r,sizeof(*r));if(old.pointer)MEMFreeToExpHeap((void*)old.heap,(void*)old.pointer);}
static U allocate_region(Region*r,U cap){void*h,*p;U z;
 if(cap<192||cap>0x40000000u||cap>0xffffffe0u)return OIS_ARGUMENT;cap=(cap+31)&~31u;
 h=MEMGetBaseHeapHandle(1);if(!h)return OIS_SPACE;p=MEMAllocFromExpHeapEx(h,cap,32);if(!p)return OIS_SPACE;
 /* A rejected SDK address may alias a live foreign owner. It has no
  * independent free authority. Fail the OIS owner closed instead of freeing
  * that address. Metadata-output external() keeps its existing policy. */
 if(!_gz_rsa_private_apart((U)p,cap)||!external((U)p,cap)||!_gz_hr_storage_private(p,cap)){retained_conflict=1;return OIS_CORRUPT;}
 z=issue(&r->identity);if(z){MEMFreeToExpHeap(h,p);return z;}
 r->heap=(U)h;r->pointer=(U)p;r->capacity=cap;return 0;
}
static U slot_ok(U slot){Slot*s=state.slots+slot;U i,n=0;const OisLease*l;
 if(!region_ok(&s->region)||s->readers>OIS_PINS)return 0;
 if(!s->region.pointer){if(s->used||s->type||s->readers)return 0;}
 else if(!s->generation||s->used<192||s->used>s->region.capacity||(s->used&3)||!ois_slot_type(s->type))return 0;
 for(i=0;i<OIS_PINS;i++)if(state.pins[i].active){
  if(state.pins[i].active!=1||state.pins[i].slot>=OIS_SLOTS)return 0;
  if(state.pins[i].slot!=slot)continue;l=&state.pins[i].lease;n++;
  if(!s->region.pointer||l->allocation!=s->region.pointer||l->archive!=s->region.pointer||l->capacity!=s->region.capacity||l->bytes!=s->used||
   l->identity!=s->region.identity||l->generation!=s->generation||!l->pin||l->reserved)return 0;
 }return n==s->readers;
}
static U pin_find(const OisLease*l){U i;for(i=0;i<OIS_PINS;i++)if(state.pins[i].active&&same(l,&state.pins[i].lease,sizeof(*l)))return i;return OIS_PINS;}
static U pin_valid(const OisLease*l){U i=pin_find(l);return i<OIS_PINS&&state.pins[i].slot<OIS_SLOTS&&slot_ok(state.pins[i].slot);}
static U pin_acquire(U slot,OisLease*l){Slot*s=state.slots+slot;Pin*p;U i,z;
 if(!slot_ok(slot))return OIS_CORRUPT;if(!s->region.pointer)return OIS_INCOMPLETE;
 for(i=0;i<OIS_PINS;i++)if(!state.pins[i].active)break;if(i==OIS_PINS)return OIS_BUSY;
 zero(l,sizeof(*l));z=issue(&l->pin);if(z)return z;l->allocation=l->archive=s->region.pointer;l->capacity=s->region.capacity;l->bytes=s->used;l->identity=s->region.identity;l->generation=s->generation;
 p=state.pins+i;p->slot=slot;copy(&p->lease,l,sizeof(*l));p->active=1;s->readers++;return 0;
}
static U pin_release(const OisLease*l){U i;if(ois_cut_protected_pin(l))return OIS_BUSY;i=pin_find(l);Pin*p;Slot*s;if(i==OIS_PINS)return 0;p=state.pins+i;
 if(p->slot>=OIS_SLOTS||!slot_ok(p->slot))return OIS_CORRUPT;s=state.slots+p->slot;s->readers--;zero(p,sizeof(*p));return 0;
}
static U capture(U slot,U object,U heap,const AfDomain*domain,U cap){U z,n=0;Slot*s,old;AfDomain saved;
 z=lock();if(z)return z;if(slot>=OIS_SLOTS)return done(OIS_ARGUMENT);if(state.tx.phase)return done(OIS_BUSY);
 s=state.slots+slot;if(!slot_ok(slot))return done(OIS_CORRUPT);if(s->readers)return done(OIS_BUSY);if(s->generation==0xffffffffu)return done(OIS_EXHAUSTED);
 zero(&saved,sizeof(saved));if(domain){if(!external((U)domain,sizeof(*domain)))return done(OIS_ARGUMENT);copy(&saved,domain,sizeof(saved));if(!same(&saved,domain,sizeof(saved)))return done(OIS_CHANGED);}
 if(state.staging.pointer)return done(OIS_CORRUPT);z=allocate_region(&state.staging,cap);if(z)return done(z);
 /* Fresh allocator ownership, not old payload contents, determines whether
  * staging is writable. A reused block may retain an old valid SPB1 marker. */
 zero((void*)state.staging.pointer,64);
 state.writer=domain?2:1;
 if(domain)z=_gz_aj_capture(&saved,(void*)state.staging.pointer,state.staging.capacity,&n);
 else z=_gz_sp_capture(object,heap,(void*)state.staging.pointer,state.staging.capacity,&n);
 state.writer=0;
 if(!z){if(n<192||n>state.staging.capacity||(n&3))z=OIS_CORRUPT;
  else z=domain?_gz_aj_validate(&saved,(void*)state.staging.pointer,n):_gz_sp_validate((void*)state.staging.pointer,n);}
 if(z){release_region(&state.staging);return done(z);}
 copy(&old,s,sizeof(old));copy(&s->region,&state.staging,sizeof(s->region));zero(&state.staging,sizeof(state.staging));
 s->used=n;s->type=domain?2:1;s->generation++;copy(&s->domain,&saved,sizeof(saved));__sync_synchronize();release_region(&old.region);return done(0);
}
#include "ois_cpu_diagnostic.inc"
U _gz_ois_capture_resource(U slot,U object,U heap,U cap){
#if GZ_RESOURCE_CAPTURE_LAB_ENABLED
 return capture(slot,object,heap,0,cap);
#else
 (void)slot;(void)object;(void)heap;(void)cap;return OIS_UNAVAILABLE;
#endif
}
U _gz_ois_capture_forest(U slot,const AfDomain*d,U cap){if(!d)return OIS_ARGUMENT;return capture(slot,0,0,d,cap);}
U _gz_ois_acquire_read(U slot,OisLease*out){U z;OisLease l;z=lock();if(z)return z;
 if(slot>=OIS_SLOTS||!external((U)out,sizeof(*out)))return done(OIS_ARGUMENT);z=pin_acquire(slot,&l);if(!z)copy(out,&l,sizeof(l));return done(z);
}
U _gz_ois_validate_read(const OisLease*input){U z;OisLease l;z=lock();if(z)return z;if(!external((U)input,sizeof(*input)))return done(OIS_ARGUMENT);
 copy(&l,input,sizeof(l));return done(pin_valid(&l)?0:OIS_CHANGED);
}
U _gz_ois_release_read(const OisLease*input){U z;OisLease l;z=lock();if(z)return z;if(!external((U)input,sizeof(*input)))return done(OIS_ARGUMENT);copy(&l,input,sizeof(l));return done(pin_release(&l));}
U _gz_ois_drop(U slot){U z;Slot*s;z=lock();if(z)return z;if(slot>=OIS_SLOTS)return done(OIS_ARGUMENT);if(state.tx.phase)return done(OIS_BUSY);
 s=state.slots+slot;if(!slot_ok(slot))return done(OIS_CORRUPT);if(s->readers)return done(OIS_BUSY);release_region(&s->region);s->used=s->type=0;zero(&s->domain,sizeof(s->domain));return done(0);
}
U _gz_ois_prepare_restore(U slot,const AfDomain*current,U target,U rollback,U undo_cap,U map_cap,U*out_token){U z;Tx*t=&state.tx;AfDomain c;
 z=lock();if(z)return z;if(state.tx.phase)return done(OIS_BUSY);
 if(slot>=OIS_SLOTS||!external((U)current,sizeof(*current))||!external((U)out_token,4)||!apart((U)current,sizeof(*current),(U)out_token,4))return done(OIS_ARGUMENT);
 if(!slot_ok(slot))return done(OIS_CORRUPT);if(state.slots[slot].type!=2)return done(OIS_TYPE);
 copy(&c,current,sizeof(c));if(!same(&c,current,sizeof(c)))return done(OIS_CHANGED);
 zero(t,sizeof(*t));z=pin_acquire(slot,&t->source);if(z)return done(z);t->slot=slot;
 z=allocate_region(&t->undo,undo_cap);if(!z)z=allocate_region(&t->map,map_cap);if(!z)z=issue(&t->token);
 if(z){release_region(&t->map);release_region(&t->undo);pin_release(&t->source);zero(t,sizeof(*t));return done(z);}
 copy(&t->request.domain.source,&state.slots[slot].domain,sizeof(c));copy(&t->request.domain.current,&c,sizeof(c));t->request.domain.target_epoch=target;t->request.domain.rollback_epoch=rollback;
 t->request.source=t->source.archive;t->request.source_bytes=t->source.bytes;t->request.undo=t->undo.pointer;t->request.undo_capacity=t->undo.capacity;t->request.map=t->map.pointer;t->request.map_capacity=t->map.capacity;
 t->thread=state.thread;t->phase=OIS_PREPARED;*out_token=t->token;return done(0);
}
 U _gz_ois_apply_restore(U token){U z;Tx*t=&state.tx;z=lock();if(z)return z;if(!token||token!=t->token||t->phase!=OIS_PREPARED||t->thread!=state.thread)return done(OIS_CHANGED);
 if(!pin_valid(&t->source)||!region_ok(&t->undo)||!region_ok(&t->map)){t->phase=OIS_DIRTY;t->code=OIS_CORRUPT;return done(OIS_CORRUPT);}
 t->phase=OIS_APPLYING;state.writer=3;zero(&t->result,sizeof(t->result));z=_gz_aj_restore(&t->request,&t->result);state.writer=0;t->code=z;
 if(z==AR_RECOVERY||t->result.arena.owner_action==AR_OWNER_RECOVER||(t->result.arena.mutation_started&&(!t->result.valid||(z!=AR_OK&&z!=AR_ROLLED_BACK))))t->phase=OIS_DIRTY;
 else t->phase=OIS_RESULT;return done(z);
}
U _gz_ois_restore_info(U token,OisInfo*out){U z;OisInfo r;Tx*t=&state.tx;z=lock();if(z)return z;
 if(!token||token!=t->token||!t->phase)return done(OIS_CHANGED);if(!external((U)out,sizeof(*out)))return done(OIS_ARGUMENT);
 zero(&r,sizeof(r));r.magic=0x4f495231u;r.bytes=sizeof(r);r.version=1;r.token=token;r.phase=t->phase;r.code=t->code;r.slot=t->slot;r.source=t->source.archive;r.source_bytes=t->source.bytes;
 r.undo=t->undo.pointer;r.undo_capacity=t->undo.capacity;r.map=t->map.pointer;r.map_capacity=t->map.capacity;r.missing=31;copy(&r.result,&t->result,sizeof(r.result));copy(out,&r,sizeof(r));return done(0);
}
U _gz_ois_cancel_restore(U token){U z;Tx*t=&state.tx;z=lock();if(z)return z;
 if(!token||token!=t->token||t->thread!=state.thread)return done(OIS_CHANGED);if(t->phase==OIS_DIRTY)return done(AR_RECOVERY);
 if(t->phase!=OIS_PREPARED)return done(OIS_CHANGED);if(!pin_valid(&t->source)){t->phase=OIS_DIRTY;return done(OIS_CORRUPT);}
 z=pin_release(&t->source);if(z){t->phase=OIS_DIRTY;return done(z);}release_region(&t->map);release_region(&t->undo);zero(t,sizeof(*t));return done(0);
}
U _gz_ois_finish_restore(U token,U expected_code,U expected_epoch){U z;Tx*t=&state.tx;z=lock();if(z)return z;
 if(!token||token!=t->token||t->thread!=state.thread)return done(OIS_CHANGED);if(t->phase==OIS_DIRTY)return done(AR_RECOVERY);
 if(t->phase!=OIS_RESULT||t->code!=expected_code||t->result.arena.pending_epoch!=expected_epoch)return done(OIS_CHANGED);
 if(!pin_valid(&t->source)){t->phase=OIS_DIRTY;return done(OIS_CORRUPT);}
 z=pin_release(&t->source);if(z){t->phase=OIS_DIRTY;return done(z);}release_region(&t->map);release_region(&t->undo);zero(t,sizeof(*t));return done(0);
}
U _gz_ois_recovery_pending(void){U z=lock(),r;if(z)return 1;r=state.tx.phase==OIS_DIRTY;return done(r);}
U _gz_ois_recover_restore(U token){U z;Tx*t=&state.tx;z=lock();if(z)return z;
 if(!token||token!=t->token||t->phase!=OIS_DIRTY||t->thread!=state.thread)return done(OIS_CHANGED);
 if(!pin_valid(&t->source)||!region_ok(&t->undo)||!region_ok(&t->map)||!_gz_aj_recovery_pending())return done(OIS_CORRUPT);
 t->phase=OIS_APPLYING;state.writer=3;z=_gz_aj_recover(&t->request,&t->result);state.writer=0;t->code=z;
 if(z==AR_ROLLED_BACK&&t->result.valid&&t->result.arena.owner_action==AR_OWNER_FINALIZE&&t->result.arena.pending_epoch==t->request.domain.rollback_epoch)t->phase=OIS_RESULT;
 else t->phase=OIS_DIRTY;
 return done(z);
}

/* Pure physical exclusion is stricter than image write permission. Fixed OIS
 * staging/apply write exceptions cannot authorize an unrelated allocation.
 * While entered, old regions may be in a clear-before-free retirement window;
 * deny every foreign span until that serialized operation has finished. */
U _gz_ois_physical_apart(U p,U n){const Region*r[OIS_SLOTS+5];U i,j;
 if(!_gz_ghc_all_private_apart(p,n)||!header()||state.entered||state.tx.phase>OIS_DIRTY||!span(p,n)||
  !apart(p,n,(U)&state,sizeof(state)))return 0;
 for(i=0;i<OIS_SLOTS;i++){if(!slot_ok(i))return 0;r[i]=&state.slots[i].region;}
 r[OIS_SLOTS]=&state.staging;r[OIS_SLOTS+1]=&state.tx.undo;r[OIS_SLOTS+2]=&state.tx.map;r[OIS_SLOTS+3]=ois_cut_region();r[OIS_SLOTS+4]=ois_cut_work_region();
 for(i=0;i<OIS_SLOTS+5;i++){
  if(!region_ok(r[i]))return 0;if(!r[i]->pointer)continue;
  if(!span(r[i]->heap,4)||!apart(r[i]->pointer,r[i]->capacity,(U)&state,sizeof(state))||
   !apart(p,n,r[i]->pointer,r[i]->capacity))return 0;
  for(j=0;j<i;j++)if(r[j]->pointer&&!apart(r[i]->pointer,r[i]->capacity,r[j]->pointer,r[j]->capacity))return 0;
 }
 return 1;
}

#include "owned_heap_dual_cut.inc"
#include "owned_heap_storage.inc"
#include "owned_heap_held_view.inc"

/* Phase1 metadata preflight BEFORE allocating the new whole output. This is
 * not a readable-range grant; the held original whole-stage check repeats. */
U _gz_ois_heap_preallocate_apart(U request,U frame,U pointer,U bytes){
 const Region*r[OIS_SLOTS+5];U i,serial=*(volatile U*)&state.serial;
 if(!_gz_ghs_scope(request,frame,1)||!_gz_ois_physical_apart(pointer,bytes))return 0;
 for(i=0;i<OIS_SLOTS;i++)r[i]=&state.slots[i].region;
 r[OIS_SLOTS]=&state.staging;r[OIS_SLOTS+1]=&state.tx.undo;r[OIS_SLOTS+2]=&state.tx.map;
 r[OIS_SLOTS+3]=ois_cut_region();r[OIS_SLOTS+4]=ois_cut_work_region();
 for(i=0;i<OIS_SLOTS+5;i++)if(r[i]->pointer){
  if(!region_ok(r[i])||!span(r[i]->heap,0x54)||r[i]->pointer<0x10000040u||r[i]->capacity>0xffffffffu-64u||
   !apart(pointer,bytes,r[i]->heap,0x54)||!apart(pointer,bytes,r[i]->pointer-64u,r[i]->capacity+64u))return 0;
 }
 __asm__ volatile("" ::: "memory");
 return serial==*(volatile U*)&state.serial&&!state.entered&&state.inverse==~serial&&
  _gz_ghs_scope(request,frame,1)&&_gz_ois_physical_apart(pointer,bytes);
}
