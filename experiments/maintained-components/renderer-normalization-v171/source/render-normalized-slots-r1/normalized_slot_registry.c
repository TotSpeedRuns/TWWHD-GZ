/* Private RAM provenance for freshly normalized user slots. This file has
 * no native graph reader/reset, allocator, source publication or GPU call.
 * It is not wired into a game image. All owner/function bindings still need
 * qualification against the final image. Old unsealed slots are refused. */
#include "normalized_slot_registry.h"
#include "render_trace.h"
#include "workspace_overlay_private.h"
#include "owned_heap_dual_cut.h"

typedef uint32_t U;
extern const volatile WsState _gz_ois_mem_state;
extern const volatile WsHeap _gz_ois_mem_heap;
extern const volatile WsCut _gz_ois_mem_cut;
extern U _gz_ois_mem_header(void),_gz_ois_mem_slot_ok(U),_gz_ois_mem_cut_header(void);
extern U _gz_ghs_scope(U,U,U),_gz_ghs_hg(U,U);
extern U _gz_ois_heap_stage_match(U,U,U,U);
#define RNS_STATE _gz_ois_mem_state
#define RNS_HEAP _gz_ois_mem_heap
#define RNS_CUT _gz_ois_mem_cut

typedef struct {
 U valid,version,slot,request,frame,thread;
 U region_heap,pointer,capacity,identity,used,type,generation;
 U old_generation,reserved[2];
 RclWitness witness;
} RnsSeal;
typedef struct {
 U phase,request,frame,thread,role,target,old_generation,used;
 U staging,capacity,identity,hg,reserved[4];
} RnsPending;

RclWitness _gz_rcl_pending;
RnsOrder _gz_rns_order;
static RnsSeal seals[OIS_SLOTS];
static RnsPending pending;
_Static_assert(sizeof(RnsOrder)==32 && sizeof(RnsPending)==64,"private order ABI");
_Static_assert(sizeof(RnsSeal)==8288,"private Saved seal ABI");

/* Only the already-private four pending words are diagnostic output. Matching
 * request/frame is attribution, never ownership or permission. The final frame
 * word is advisory: a sequential external read is still not an atomic sample. */
__attribute__((noinline)) void _gz_rns_trace(U request,U frame,U step,U raw,U detail){
 volatile U *out=(volatile U*)pending.reserved;
 if(!request||!frame||pending.request!=request||pending.frame!=frame)return;
 out[0]=((_gz_rns_order.stage&255u)<<24)|(step&0x00ffffffu);
 out[1]=raw;out[2]=detail;out[3]=frame;
 __sync_synchronize();
}

static void zero(void*p,U n){U i;for(i=0;i<n;i++)((unsigned char*)p)[i]=0;}
static void copy(void*d,const void*s,U n){U i;for(i=0;i<n;i++)((unsigned char*)d)[i]=((const unsigned char*)s)[i];}
static U equal(const void*a,const void*b,U n){U i;for(i=0;i<n;i++)if(((const unsigned char*)a)[i]!=((const unsigned char*)b)[i])return 0;return 1;}
static U apart(U p,U n,U q,U m){return n && m && n<=0xffffffffu-p && m<=0xffffffffu-q && (p+n<=q || q+m<=p);}

static U owner(U request,U frame,U phase){
 if(!request||!frame||!_gz_ghs_scope(request,frame,phase)||!_gz_ois_mem_header())return 0;
 if(RNS_STATE.entered!=1||!RNS_STATE.thread||RNS_STATE.thread!=RNS_HEAP.thread||RNS_STATE.tx.phase||
  RNS_HEAP.request!=request||RNS_HEAP.frame!=frame||RNS_HEAP.purpose!=GHS_VERSION||
  RNS_HEAP.image_version!=GHC_PROCESS_VERSION||RNS_HEAP.target>=OIS_SLOTS||
  (RNS_HEAP.role!=OIS_CUT_SAVED&&RNS_HEAP.role!=OIS_CUT_CURRENT)||!_gz_ois_mem_slot_ok(RNS_HEAP.target))return 0;
 if(phase==2)return RNS_HEAP.phase==2&&RNS_STATE.writer==4&&RNS_STATE.staging.pointer&&
  _gz_ois_heap_stage_match(request,frame,RNS_STATE.staging.pointer,RNS_STATE.staging.capacity);
 return phase==3&&RNS_HEAP.phase==3&&!RNS_STATE.writer;
}
static U transaction(U request,U frame){
 return owner(request,frame,2)&&pending.phase&&pending.request==request&&pending.frame==frame&&
  pending.thread==RNS_STATE.thread&&pending.role==RNS_HEAP.role&&pending.target==RNS_HEAP.target&&
  pending.old_generation==RNS_STATE.slots[RNS_HEAP.target].generation&&
  pending.staging==RNS_STATE.staging.pointer&&pending.capacity==RNS_STATE.staging.capacity&&
  pending.identity==RNS_STATE.staging.identity&&pending.hg&&pending.hg==_gz_ghs_hg(request,frame);
}
static U witness_format(const RclWitness*w){U i,j;
 if(w->magic!=RCL_MAGIC||w->version!=RCL_VERSION||w->bytes!=sizeof(*w)||w->valid!=1||
  !w->request||!w->frame||!w->hg||!w->owner||w->initialized!=1||
  !w->j3d_opa||!w->j3d_xlu||!w->heap_count||w->heap_count>RCL_HEAPS||
  w->allocation_count!=RCL_ALLOCS||w->queue_count!=RCL_QUEUES||!w->raw||!w->raw_bytes)return 0;
 for(i=0;i<RCL_QUEUES;i++)if(!w->queues[i].count||!w->queues[i].array||w->queues[i].used||
  w->queues[i].array!=w->allocations[2*i+1].pointer)return 0;
 for(i=0;i<RCL_ALLOCS;i++)if(!w->allocations[i].pointer||!w->allocations[i].generation||
  !w->allocations[i].heap||!w->allocations[i].heap_generation||!w->allocations[i].requested||
  w->allocations[i].capacity<w->allocations[i].requested||w->allocations[i].kind<1||w->allocations[i].kind>3)return 0;
 for(i=0;i<w->heap_count;i++){
  if(!w->heaps[i].heap[0]||!w->heaps[i].heap[1]||w->heaps[i].heap[2]!=1)return 0;
  for(j=0;j<i;j++)if(w->heaps[i].heap[0]==w->heaps[j].heap[0])return 0;
 }
 return 1;
}
static U current_witness(U request,U frame){
 return witness_format(&_gz_rcl_pending)&&_gz_rcl_pending.request==request&&
  _gz_rcl_pending.frame==frame&&_gz_rcl_pending.hg==pending.hg;
}
static U lifetime_equal(const RclWitness*a,const RclWitness*b){U i,j,found;
 if(!witness_format(a)||!witness_format(b)||a->owner!=b->owner||a->initialized!=b->initialized||
  a->j3d_opa!=b->j3d_opa||a->j3d_xlu!=b->j3d_xlu||a->heap_count!=b->heap_count||
  !equal(a->queues,b->queues,sizeof(a->queues))||!equal(a->allocations,b->allocations,sizeof(a->allocations)))return 0;
 /* Closure traversal order is not a lifetime stamp. Compare the bounded
  * distinct heap/scope sets by native heap key, byte for byte. */
 for(i=0;i<a->heap_count;i++){
  found=0;
  for(j=0;j<b->heap_count;j++)if(a->heaps[i].heap[0]==b->heaps[j].heap[0]){
   if(!equal(a->heaps+i,b->heaps+j,sizeof(RclHeap)))return 0;
   found=1;break;
  }
  if(!found)return 0;
 }
 return 1;
}
static U sealed_slot(U target){const volatile WsSlot*s=RNS_STATE.slots+target;const RnsSeal*r=seals+target;
 return r->valid==1&&r->version==1&&r->slot==target&&r->request&&r->frame&&r->thread&&
  r->region_heap==s->region.heap&&r->pointer==s->region.pointer&&r->capacity==s->region.capacity&&
  r->identity==s->region.identity&&r->used==s->used&&r->type==GHC_PROCESS_VERSION&&r->type==s->type&&
  r->generation&&r->generation==s->generation&&r->old_generation!=0xffffffffu&&
  r->generation==r->old_generation+1&&!r->reserved[0]&&!r->reserved[1]&&witness_format(&r->witness)&&
  r->witness.request==r->request&&r->witness.frame==r->frame;
}
static U saved_pin(void){U i,found=0;const volatile OisLease*l=&RNS_CUT.saved;const volatile WsSlot*s=RNS_STATE.slots+RNS_HEAP.target;
 if(!_gz_ois_mem_cut_header()||RNS_CUT.phase!=1||RNS_CUT.request!=RNS_HEAP.request||RNS_CUT.frame!=RNS_HEAP.frame||
  RNS_CUT.thread!=RNS_STATE.thread||RNS_CUT.slot!=RNS_HEAP.target||!l->pin||l->reserved||!s->readers||
  l->allocation!=s->region.pointer||l->archive!=s->region.pointer||l->capacity!=s->region.capacity||
  l->bytes!=s->used||l->identity!=s->region.identity||l->generation!=s->generation)return 0;
 for(i=0;i<OIS_PINS;i++)if(RNS_STATE.pins[i].active&&RNS_STATE.pins[i].lease.pin==l->pin){
  U j;const volatile U*a=(const volatile U*)&RNS_STATE.pins[i].lease,*b=(const volatile U*)l;
  if(RNS_STATE.pins[i].active!=1||RNS_STATE.pins[i].slot!=RNS_HEAP.target)return 0;
  for(j=0;j<sizeof(*l)/4;j++)if(a[j]!=b[j])return 0;
  ++found;
 }
 return found==1;
}

U _gz_rns_current_begin(U request,U frame){
 if(!owner(request,frame,2))return GHC_CONTEXT;
 if(_gz_rns_order.active||_gz_rns_order.stage)return GHC_RETAINED;
 if(RNS_STATE.staging.capacity<RCG_SCRATCH_BYTES)return GHC_SPACE;
 zero(&pending,sizeof(pending));zero(&_gz_rns_order,sizeof(_gz_rns_order));_gz_rcl_pending.valid=0;
 pending.request=request;pending.frame=frame;pending.thread=RNS_STATE.thread;pending.role=RNS_HEAP.role;
 pending.target=RNS_HEAP.target;pending.old_generation=RNS_STATE.slots[RNS_HEAP.target].generation;
 pending.staging=RNS_STATE.staging.pointer;pending.capacity=RNS_STATE.staging.capacity;
 pending.identity=RNS_STATE.staging.identity;pending.hg=_gz_ghs_hg(request,frame);
 if(!pending.hg)return GHC_CONTEXT;
 __sync_synchronize();pending.phase=1;return 0;
}
U _gz_rns_prefix_claim(U request,U frame,U pointer,U capacity){
 if(!transaction(request,frame)||pending.phase!=1||_gz_rns_order.active||_gz_rns_order.stage||
  pointer!=pending.staging||capacity!=pending.capacity||capacity<RCG_SCRATCH_BYTES)return GHC_CONTEXT;
 _gz_rns_order.request=request;_gz_rns_order.frame=frame;_gz_rns_order.staging=pointer;
 _gz_rns_order.capacity=capacity;_gz_rns_order.hg=pending.hg;_gz_rns_order.role=pending.role;
 _gz_rns_order.stage=RNS_CLAIMED;__sync_synchronize();_gz_rns_order.active=1;return 0;
}
U _gz_rns_saved_ready(U request,U frame){
 if(!transaction(request,frame)||pending.role!=OIS_CUT_CURRENT)return GHC_CONTEXT;
 return saved_pin()&&sealed_slot(pending.target)?0:GHC_CHANGED;
}
U _gz_rns_saved_match(U request,U frame){
 U z=_gz_rns_saved_ready(request,frame);if(z)return z;
 if(!current_witness(request,frame)||!lifetime_equal(&seals[pending.target].witness,&_gz_rcl_pending))return GHC_CHANGED;
 pending.phase=2;return 0;
}
U _gz_rns_capture_done(U request,U frame,U written,U result){
 const GhcHeader*h;const GhcSaveProducer*p;
 if(result){if(pending.request==request&&pending.frame==frame){pending.phase=0;_gz_rcl_pending.valid=0;}return result;}
 if(!transaction(request,frame)||_gz_rns_order.active||_gz_rns_order.stage!=RNS_EXPORTED||
  _gz_rns_order.request!=request||_gz_rns_order.frame!=frame||_gz_rns_order.hg!=pending.hg||
  _gz_rns_order.staging!=pending.staging||_gz_rns_order.capacity!=pending.capacity||
  _gz_rns_order.role!=pending.role||!current_witness(request,frame)||
  written<192||written>pending.capacity||(written&3))return GHC_CONTEXT;
 h=(const GhcHeader*)(uintptr_t)pending.staging;p=(const GhcSaveProducer*)(uintptr_t)(pending.staging+128);
 if(h->magic!=GHC_MAGIC||h->version!=GHC_PROCESS_VERSION||h->bytes!=written||h->valid||
  h->request!=request||h->frame!=frame||h->thread!=pending.thread||h->hg!=pending.hg||
  p->magic!=GHS_MAGIC||p->bytes!=sizeof(*p)||p->version!=2||p->request!=request||p->frame!=frame||
  p->thread!=pending.thread||p->slot!=pending.target||p->hg!=pending.hg||p->reserved!=pending.role)return GHC_FORMAT;
 if(pending.role==OIS_CUT_CURRENT){if(pending.phase!=2)return GHC_CHANGED;pending.phase=0;return 0;}
 if(pending.phase!=1||pending.old_generation==0xffffffffu)return GHC_CHANGED;
 pending.used=written;__sync_synchronize();pending.phase=2;return 0;
}
U _gz_rns_publish(U request,U frame){const volatile WsSlot*s;RnsSeal*r;
 if(!owner(request,frame,3)||RNS_HEAP.role!=OIS_CUT_SAVED)return GHC_RETAINED;
 if(pending.phase!=2||pending.request!=request||pending.frame!=frame||pending.thread!=RNS_STATE.thread||
  pending.role!=OIS_CUT_SAVED||pending.target!=RNS_HEAP.target||pending.old_generation==0xffffffffu||
  !current_witness(request,frame)||_gz_rns_order.active||_gz_rns_order.stage)return GHC_CHANGED;
 s=RNS_STATE.slots+pending.target;
 if(s->generation!=pending.old_generation+1||s->type!=GHC_PROCESS_VERSION||s->readers||
  s->used!=pending.used||RNS_HEAP.used!=pending.used||!s->region.pointer||!s->region.identity||
  s->region.capacity<s->used||RNS_STATE.staging.pointer)return GHC_CHANGED;
 r=seals+pending.target;r->valid=0;__sync_synchronize();
 r->version=1;r->slot=pending.target;r->request=request;r->frame=frame;r->thread=pending.thread;
 r->region_heap=s->region.heap;r->pointer=s->region.pointer;r->capacity=s->region.capacity;
 r->identity=s->region.identity;r->used=s->used;r->type=s->type;r->generation=s->generation;
 r->old_generation=pending.old_generation;r->reserved[0]=r->reserved[1]=0;
 copy(&r->witness,&_gz_rcl_pending,sizeof(r->witness));__sync_synchronize();r->valid=1;
 pending.phase=0;return 0;
}
U _gz_rns_private_apart(U pointer,U bytes){
 return apart(pointer,bytes,(U)(uintptr_t)&pending,sizeof(pending))&&
  apart(pointer,bytes,(U)(uintptr_t)seals,sizeof(seals))&&
  apart(pointer,bytes,(U)(uintptr_t)&_gz_rcl_pending,sizeof(_gz_rcl_pending))&&
  apart(pointer,bytes,(U)(uintptr_t)&_gz_rns_order,sizeof(_gz_rns_order));
}
