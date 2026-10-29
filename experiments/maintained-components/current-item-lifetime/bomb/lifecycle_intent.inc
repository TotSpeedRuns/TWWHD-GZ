/* Include after park_plan.inc. Source-only first lifecycle experiment:
 * genuine R5 inverse -> private intent -> NEW NBD native/shared HG writer +
 * same job Gate -> original schedule once. Natural engine pumping is NOT
 * owned by LRW. No old actor is followed after its schedule call. */
#include "lifecycle_intent.h"
#include "lifecycle_writer.h"
#include "lifecycle_scalar.h"
#include "object_binding_try.h"
enum {LRR_MAGIC=0x4c525231u,LRR_CONTEXT=2860,LRR_SOURCE=2861,LRR_OWNER=2862,
 LRR_SHAPE=2863,LRR_SCHEDULE=2864,LRR_CHANGED=2865,LRR_TIMEOUT=2866,
 LRR_EXHAUSTED=2867,LRR_OBSERVER=2868,LRR_CAP=8,LRR_CO_CAP=32,LRR_WAIT_LIMIT=600};
typedef struct {PoEntry process;ObBinding allocation;HlHeap solid;
 u32 po_index,heap_index,layer,layer_id,pause;} LrrActor;
typedef struct {u32 object,generation,index,owner;} LrrCo;
typedef struct {
 u32 magic,bytes,version,phase,nonce,error,request,issued_frame,thread,
 slot,identity,generation,used,hash,epoch,stage0,stage1,room,layer,
 count,co_count,next,scheduled,retired,callbacks,provider_serial,frame;
 LrrActor actor[LRR_CAP];LrrCo co[LRR_CO_CAP];u32 seal;
} LrrIntent;
typedef char LrrIntentSize[sizeof(LrrIntent)==2320?1:-1];
static LrrIntent lrr __attribute__((section(".bss.gz_lrr")));
static u32 lrr_serial __attribute__((section(".bss.gz_lrr")));
/* Fixed scratch is private DATA, as required by the real OB try ABI. */
static ObBinding lrr_binding __attribute__((section(".bss.gz_lrr")));
/* Observations only: phase/nonce/error/request/frame, logical source tuple,
 * counts/scheduled/retired, actual raw schedule result/provider serial.
 * Neither FINISHED nor REPLAY_READY says a world restore committed. */
volatile u32 _gz_lrr_report[32] __attribute__((section(".data.gz_lrr")))={
 LRR_MAGIC,128,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,BAD,0,0,0,0,0,0,0,31,0,0,0,0,0};
extern volatile u32 _gz_lrw_report[32];
extern u32 _gz_osa_intent_match(u32,u32,u32,u32,u32);
extern u32 _gz_bli_native_resolve(u32);
static void lrr_seal(void){lrr.seal=btr_hash(&lrr,sizeof(lrr)-4);__asm__ volatile("sync":::"memory");}
static u32 lrr_valid(void){return lrr.magic==LRR_MAGIC&&lrr.bytes==sizeof(lrr)&&lrr.version==1&&
 lrr.nonce&&lrr.count&&lrr.count<=LRR_CAP&&lrr.co_count<=LRR_CO_CAP&&
 lrr.next<lrr.count&&lrr.seal==btr_hash(&lrr,sizeof(lrr)-4);}
static u32 lrr_fail(u32 z){if(!lrr.error)lrr.error=z;lrr.phase=LRR_HALTED;lrr_seal();
 _gz_lrr_report[3]=lrr.phase;if(!_gz_lrr_report[5])_gz_lrr_report[5]=z;return z;}
u32 _gz_lrr_blocked(void){return lrr.magic||lrr.phase||lrr.nonce?
 (lrr.magic==LRR_MAGIC&&lrr.phase==LRR_FINISHED?0u:1u):0u;}
/* Called by real LRW twice before/after its native body. This contains NO
 * API/native/table traversal. The nonce survives all HELD/postbody checks.
 * Only a LATER call, after genuine release/producer expiry, unblocks tools. */
u32 _gz_lrr_requested(void){
 if(lrr_release_observed(lrr.phase,lrr.nonce,lrr.provider_serial,
  _gz_lrw_report[5],_gz_lrw_report[4],_gz_lrw_report[3],_gz_lrw_report[15],
  _gz_lrw_report[16],_gz_lrw_report[17],_gz_lrw_report[18])){
  if(lrr.phase==LRR_HALTED)return 0; /* stays blocked, no repeated writer calls */
  lrr.phase=LRR_FINISHED;lrr_seal();_gz_lrr_report[3]=LRR_FINISHED;
 }
 return _gz_lrr_blocked()?lrr.nonce:0;
}
u32 _gz_lrr_private_apart(u32 p,u32 n){return epj_apart(p,n,(u32)&lrr,sizeof(lrr))&&
 epj_apart(p,n,(u32)&lrr_serial,4)&&epj_apart(p,n,(u32)&lrr_binding,sizeof(lrr_binding))&&
 epj_apart(p,n,(u32)_gz_lrr_report,sizeof(_gz_lrr_report));}
static u32 lrr_headers(void){return _gz_po_header.magic==PO_MAGIC&&_gz_po_header.bytes==64&&
 _gz_po_header.version==1&&_gz_po_header.capacity==PO_CAP&&_gz_po_header.rows==(u32)_gz_po_rows&&
 _gz_po_header.active<=PO_CAP&&!_gz_po_header.busy&&!_gz_po_header.flags&&
 _gz_co_header.magic==CO_MAGIC&&_gz_co_header.bytes==64&&_gz_co_header.version==1&&
 _gz_co_header.capacity==CO_CAP&&_gz_co_header.rows==(u32)_gz_co_rows&&_gz_co_header.active<=CO_CAP&&
 _gz_po_header.serial&&_gz_po_header.serial!=BAD&&_gz_co_header.serial&&_gz_co_header.serial!=BAD&&
 !_gz_co_header.busy&&!_gz_co_header.flags&&!_gz_po_header.session&&!_gz_co_header.session;}
static u32 lrr_source(void){return lrr.slot<10&&lrr.identity&&lrr.generation&&lrr.used>=216&&
 _gz_osa_intent_match(lrr.slot,lrr.identity,lrr.generation,lrr.used,lrr.hash);}
static u32 lrr_frame(void){return lrr.epoch==_gz_pbac_epoch&&lrr.stage0==_gz_pbac_stage[0]&&
 lrr.stage1==_gz_pbac_stage[1]&&lrr.room==_gz_pbac_room&&lrr.layer==_gz_pbac_layer&&
 !*(volatile u8*)&_gz_pbac_loading&&!_gz_pbac_event&&!_gz_pbac_world_resume_pending&&
 !_gz_pbac_pending&&_gz_hg_platform_thread()==lrr.thread;}
static u32 lrr_empty_root(u32 p){return !R32(p,0)&&!R32(p,4)&&!R32(p,8);}
/* Only current native fields are followed, after exact LIVE Po key and
 * allocation metadata checks under the separate genuine writer interval.
 * No resource manager/node/refcount is read. All addresses use retained
 * native TEXT/DATA anchors, not observed process addresses or ABS aliases. */
static u32 lrr_item_shape(const LrrActor*x){u32 a=x->process.object,d=GZ_OB_DATA_BEGIN,t=GZ_OB_TEXT_BEGIN;
 if(x->process.profile!=255||x->process.process_name!=255||x->process.allocation_bytes!=0x7f0||
  x->process.profile_entry!=d+0x1b7a3c||x->process.init_state!=0||x->process.flags||
  x->process.state!=PO_LIVE||!x->process.generation||!x->process.pid||!span(a,0x7f0)||
  R32(a,4)!=x->process.pid||R16(a,8)!=255||R8(a,10)==1||R8(a,11)!=x->pause||
  (x->pause&3u)||R8(a,12)!=2||R32(a,0x14)||R32(a,0x2c)!=x->layer||!x->layer||
  x->layer_id>=0xfffffffdu||_gz_bli_native_resolve(x->layer_id)!=x->layer||
  !span(x->layer,0x2c)||R32(x->layer,12)!=x->layer_id||R16(x->layer,0x2a)>=0x7fffu||
  R32(a,0xa8)!=d+0x1f3bdc||R32(a,0xbc)!=d+0x1f3088||R32(a,0xf0)!=d+0x1b79f8||
  R32(a,0xb4)!=d+0x11ff8||R32(a,0xf4)!=x->solid.heap||R8(a,0x2dc)||R8(a,0x3aa)!=1||
  R32(a,0x3ac)||R32(a,0x3b0)!=2||R8(a,0x74e)!=1||R32(a,0x7a8)||R32(a,0x7bc)||
  R32(a,0x7d0)||R32(a,0x7ec)||
  R32(d+0x1f3bdc,12)!=t+(0x25df2ccu-0x02000000u)||
  R32(d+0x1f3088,12)!=t+(0x25d4ab8u-0x02000000u)||
  R32(d+0x1b79f8,12)!=t+(0x217f890u-0x02000000u)||
  R32(d+0x1e6a90,0)!=d+0x4e690||
  R32(d+0x4e690,0)!=0x416c7761u||R16(d+0x4e690,4)!=0x7973||R8(d+0x4e690,6))return 0;
 /* No node recursion, parent callbacks or pending layer/create requests.
  * The native IsDelete also performs DrawCut; its exact original path,
  * including that native mutation, is the reviewed scheduling contract. */
 if(_gz_ipk_native_is_node(IPK_NODE_TYPE,R32(a,0xb8))||R8(a+0x68,16)||
  !lrr_empty_root(d+0x1f39a4u)||!lrr_empty_root(d+0x1f3a1cu)||
  !lrr_empty_root(d+0x48ab40u)||
  R32(a+0x4c,0)||R32(a+0x4c,4)||R32(a+0x4c,8)||R32(a+0x4c,12)!=a||R8(a+0x4c,16)||
  R32(a+0xc8,4)!=d+0x1f3328u||R32(a+0xc8,12)!=a||R8(a+0xc8,16)!=1||
  !R32(d+0x1f3328u,8)||R32(d+0x1f3328u,8)>IPR_SOURCE_CAP)return 0;
 return 1;
}
/* One-time private index capture; no per-frame full Po/Co table rescans. */
static u32 lrr_indices(void){u32 i,j,k,start,found;
 /* Exact current producer lookup geometry; no tombstone/create policy is
  * changed or used. Each key is resolved ONCE, with the original EMPTY stop. */
 for(j=0;j<lrr.count;j++){found=0;start=((lrr.actor[j].process.object>>2)*0x9e3779b1u)>>19;
  for(i=0;i<PO_CAP;i++){k=(start+i)&(PO_CAP-1);if(!_gz_po_rows[k].object)break;
   if(_gz_po_rows[k].object!=lrr.actor[j].process.object)continue;
   if(!btr_equal(_gz_po_rows+k,&lrr.actor[j].process,sizeof(PoEntry)))return 0;
   lrr.actor[j].po_index=k;found=1;break;}if(!found)return 0;
 }
 for(j=0;j<lrr.co_count;j++){found=0;start=((lrr.co[j].object>>2)*0x9e3779b1u)>>19;
  for(i=0;i<CO_CAP;i++){k=(start+i)&(CO_CAP-1);if(!_gz_co_rows[k].object)break;
   if(_gz_co_rows[k].object!=lrr.co[j].object)continue;
   if(_gz_co_rows[k].generation!=lrr.co[j].generation||_gz_co_rows[k].state!=CO_LIVE)return 0;
   lrr.co[j].index=k;found=1;break;}if(!found)return 0;
 }
 return 1;
}
static u32 lrr_heap_capture(void){u32 i,j,k,start,found,ok=0;
 if(_gz_hl_control.magic!=HL_MAGIC||_gz_hl_control.bytes!=64||_gz_hl_control.version!=1||!_gz_hl_meta_try_lock())return 0;
 if(_gz_hl_control.flags||_gz_hl_control.session||_gz_hl_control.active_heaps>HL_HEAPS||_gz_hl_control.active_allocations>HL_ALLOCS)goto done;
 for(j=0;j<lrr.count;j++){u32 heap=R32(lrr.actor[j].process.object,0xf4);found=0;start=((heap>>2)*0x9e3779b1u)>>20;
  for(i=0;i<HL_HEAPS;i++){const HlHeap*h;k=(start+i)&(HL_HEAPS-1);h=_gz_hl_heaps+k;
   if(!h->heap)break;if(h->heap!=heap)continue;
   if(!heap||h->state!=HL_LIVE||h->kind!=HL_SOLID||!h->generation||(h->flags&~HL_REBORN)||h->end_frame||h->dtor_site)goto done;
   btr_copy(&lrr.actor[j].solid,h,sizeof(*h));lrr.actor[j].heap_index=k;found=1;break;
  }if(!found)goto done;
 }ok=1;
done:_gz_hl_meta_unlock();return ok;
}
/* Optional genuine issuance, never a weakening of old finish admission.
 * Unsupported cases decline the NEW experiment and retain the old refusal.
 * No source address/pin or WHL/HG/job capability survives in this receipt. */
static void lrr_arm(u32 t,u32 f){u32 i,z=LRR_CONTEXT;
 if(_gz_ipk_idle())return;
 if(_gz_lrr_blocked()||!wct_sealed(t,f)||wct.phase!=WCT_ROLLED||wct.returned!=1||
  !wct.error||wct.fault_group||!wct_source.entered||!wct_source.request||wct_source.frame!=f||
  _gz_wct_report[94]!=1||!_gz_wcm_requested(wct_source.request)||
  _gz_wct_closure_hash!=wct_seal||!ipk_plan_valid(t)||!ipk_run_valid(t)||
  ipk_run.phase!=IPK_RESTORED||ipk_run.error||!ctp_restored(t,f)||
  !ipk_plan.extra_count||ipk_plan.extra_count>LRR_CAP||_gz_ipk_before_end(t,f)||
  !_gz_wct_heap_held(t,f)||!lrr_headers())return;
 if(lrr_serial==BAD){_gz_lrr_report[5]=LRR_EXHAUSTED;return;}
 bcl_zero(&lrr,sizeof(lrr));lrr.magic=LRR_MAGIC;lrr.bytes=sizeof(lrr);lrr.version=1;
 lrr.request=wct_source.request;lrr.issued_frame=f;lrr.thread=_gz_hg_platform_thread();
 lrr.slot=wct_source.slot;lrr.identity=ipk_plan.identity;lrr.generation=ipk_plan.generation;
 lrr.used=ipk_plan.used;lrr.hash=ipk_plan.source_hash;lrr.epoch=_gz_pbac_epoch;
 lrr.stage0=_gz_pbac_stage[0];lrr.stage1=_gz_pbac_stage[1];lrr.room=_gz_pbac_room;lrr.layer=_gz_pbac_layer;
 lrr.count=ipk_plan.extra_count;lrr.co_count=ipk_plan.extra_co_count;
 if(lrr.co_count>LRR_CO_CAP)goto decline;
 for(i=0;i<lrr.count;i++){
  btr_copy(&lrr.actor[i].process,ipk_plan.extra_process+i,sizeof(PoEntry));
  lrr.actor[i].layer=R32(lrr.actor[i].process.object,0x2c);
  if(!span(lrr.actor[i].layer,44))goto decline;
  lrr.actor[i].layer_id=R32(lrr.actor[i].layer,12);lrr.actor[i].pause=ipk_plan.pause[i];
  z=_gz_ob_try_capture(lrr.actor[i].process.object,lrr.actor[i].process.allocation_bytes,&lrr_binding);
  if(z)goto decline;btr_copy(&lrr.actor[i].allocation,&lrr_binding,sizeof(lrr_binding));
 }
 for(i=0;i<lrr.co_count;i++){const IprExtraCo*x=ipk_plan.extra_co+i;
  if(x->owner_index>=lrr.count||(x->status_bytes&&x->status_owner_index!=x->owner_index))goto decline;
  lrr.co[i].object=x->collider.object;lrr.co[i].generation=x->collider.generation;lrr.co[i].owner=x->owner_index;
 }
 z=LRR_OBSERVER;if(!lrr_indices()||!lrr_heap_capture())goto decline;
 z=LRR_SHAPE;for(i=0;i<lrr.count;i++)if(!lrr_item_shape(lrr.actor+i))goto decline;
 z=LRR_CHANGED;if(!lrr_headers()||!lrr_frame()||!ctp_restored(t,f)||!_gz_wct_heap_held(t,f))goto decline;
 /* Publish nonce LAST, from this actual private monotonically-issued serial. */
 lrr.phase=LRR_ARMED;lrr.nonce=++lrr_serial;lrr_seal();
 for(i=3;i<32;i++)_gz_lrr_report[i]=0;_gz_lrr_report[18]=BAD;_gz_lrr_report[26]=31;
 _gz_lrr_report[3]=lrr.phase;_gz_lrr_report[4]=lrr.nonce;_gz_lrr_report[5]=0;
 _gz_lrr_report[6]=lrr.request;_gz_lrr_report[7]=f;_gz_lrr_report[8]=lrr.slot;
 _gz_lrr_report[9]=lrr.identity;_gz_lrr_report[10]=lrr.generation;_gz_lrr_report[11]=lrr.used;
 _gz_lrr_report[12]=lrr.hash;_gz_lrr_report[13]=lrr.count;_gz_lrr_report[14]=lrr.co_count;return;
decline:_gz_lrr_report[28]=z;bcl_zero(&lrr,sizeof(lrr));
}
/* Try-lock copies/read-only metadata only. No native heap/object is followed.
 * Return0=still LIVE/ENDING,1=all observed completion labels,2=unknown/change. */
static u32 lrr_retirement(u32 n){const LrrActor*x=lrr.actor+n;const PoEntry*p;
 const HlHeap*h;PoHeader ph;CoHeader ch;u32 i,all=1,z=2;
 if(x->po_index>=PO_CAP||x->heap_index>=HL_HEAPS)return 2;p=_gz_po_rows+x->po_index;
 btr_copy(&ph,&_gz_po_header,sizeof(ph));btr_copy(&ch,&_gz_co_header,sizeof(ch));
 if(!lrr_headers()||p->object!=x->process.object||p->pid!=x->process.pid||p->generation!=x->process.generation||p->flags)return 2;
 if(!lrr_po_completion(p->state,p->end_frame,p->flags)){
  if(p->state!=PO_LIVE&&p->state!=PO_ENDING)return 2;all=0;
 }
 for(i=0;i<lrr.co_count;i++)if(lrr.co[i].owner==n){const LrrCo*k=lrr.co+i;const CoEntry*c;
  if(k->index>=CO_CAP)return 2;c=_gz_co_rows+k->index;
  if(c->object!=k->object||c->generation!=k->generation||(c->flags&~CO_REMOVED))return 2;
  if(!lrr_co_completion(c->state,c->dead_frame,c->dtor_site,c->flags)){if(c->state!=CO_LIVE)return 2;all=0;}
 }
 if(!_gz_hl_meta_try_lock())return 0;
 if(_gz_hl_control.flags||_gz_hl_control.session)goto done;
 h=_gz_hl_heaps+x->heap_index;
 if(h->heap!=x->solid.heap||h->generation!=x->solid.generation||h->kind!=HL_SOLID)goto done;
 if(!lrr_heap_completion(h->state,h->end_frame,h->dtor_site)){
  if(h->state!=HL_LIVE&&h->state!=HL_ENDING)goto done;all=0;
 }z=all;
done:_gz_hl_meta_unlock();
 if(!lrr_headers()||!btr_equal(&ph,&_gz_po_header,sizeof(ph))||!btr_equal(&ch,&_gz_co_header,sizeof(ch)))return 2;
 return z;
}
u32 _gz_lrr_native_step(void){LrrActor*x;u32 i,z,raw,ambient,f=_gz_pbac_frame;
 if(!lrr_valid()||lrr.error||!_gz_lrw_owned())return lrr_fail(LRR_CONTEXT);
 /* Observe this genuine provider BEFORE any fallible consumer check.
  * A terminal SOURCE/frame/timeout refusal can then recognize its real
  * cleanup on the next entry, without requesting repeated writer slots. */
 lrr.provider_serial=_gz_lrw_report[4];
 if(!lrr_source()||!lrr_frame())return lrr_fail(LRR_SOURCE);
 if(lrr.callbacks==LRR_WAIT_LIMIT)return lrr_fail(LRR_TIMEOUT);
 ++lrr.callbacks;lrr.frame=f;
 _gz_lrr_report[17]=lrr.callbacks;_gz_lrr_report[19]=lrr.provider_serial;_gz_lrr_report[20]=f;
 if(lrr.phase==LRR_WAIT){z=lrr_retirement(lrr.next);
  if(z==2)return lrr_fail(LRR_OBSERVER);
  if(!z){lrr_seal();return 5;}
  lrr.retired|=1u<<lrr.next;_gz_lrr_report[16]=lrr.retired;
  if(lrr.retired==lrr_mask(lrr.count)){lrr.phase=LRR_REPLAY_READY;lrr_seal();_gz_lrr_report[3]=lrr.phase;return 0;}
  ++lrr.next;lrr.phase=LRR_ARMED;
 }
 if(lrr.phase!=LRR_ARMED)return lrr_fail(LRR_CONTEXT);
 i=lrr.next;x=lrr.actor+i;
 /* Metadata identity precedes EVERY actor follow. No old scheduled actor
  * is dereferenced by any subsequent call, including error paths. */
 if(!lrr_headers()||x->po_index>=PO_CAP||x->heap_index>=HL_HEAPS||
  !btr_equal(_gz_po_rows+x->po_index,&x->process,sizeof(PoEntry))||_gz_ob_try_validate(&x->allocation))return lrr_fail(LRR_OWNER);
 if(!_gz_hl_meta_try_lock()){lrr_seal();return 5;}
 z=_gz_hl_control.flags||_gz_hl_control.session||!btr_equal(_gz_hl_heaps+x->heap_index,&x->solid,sizeof(HlHeap));
 _gz_hl_meta_unlock();if(z)return lrr_fail(LRR_OWNER);
 if(!lrr_item_shape(x)||!lrr_source()||!_gz_lrw_owned())return lrr_fail(LRR_SHAPE);
 ambient=R32(GZ_OB_DATA_BEGIN,0x1f3ae8);
 lrr.phase=LRR_CALLING;lrr.scheduled|=1u<<i;lrr_seal();_gz_lrr_report[3]=LRR_CALLING;
 _gz_lrr_report[15]=lrr.scheduled;_gz_lrr_report[18]=BAD;_gz_lrr_report[21]=i;
 raw=_gz_pbac_native_delete_actor_entry((void*)x->process.object);
 /* Native call may have changed queue/phase. No actor/tag/model/heap/native
  * resource pointer is read from this edge onward for the scheduled actor. */
 _gz_lrr_report[18]=raw;lrr.phase=LRR_WAIT;lrr_seal();_gz_lrr_report[3]=LRR_WAIT;
 if(raw!=1)return lrr_fail(LRR_SCHEDULE);
 if(f!=_gz_pbac_frame||ambient!=R32(GZ_OB_DATA_BEGIN,0x1f3ae8)||!lrr_source()||!_gz_lrw_owned())return lrr_fail(LRR_CHANGED);
 return 5;
}
