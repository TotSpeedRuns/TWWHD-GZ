/* Include AFTER land_smoke_restore.inc, itself after bomb_factory_contacts.c.
 * This file uses original private BOH/LSR receipts, not exported audit reports.
 * The original BSG1542 source guard remains installed and unchanged. */
#include "bundle_state.h"
#include "bundle_state.c"
enum {LSB_MAGIC=0x4c534231u,LSB_CONTEXT=2951,LSB_RETAINED=2952,LSB_CHANGED=2953};
typedef struct {
 u32 magic,bytes,version,valid,token,frame,pin,hbp_seal,boh_plan,source_hash;
 u32 source,used,smoke_pid,smoke_generation,layer,source_bomb_pid;
 PbaKey bomb;
 u32 seal;
} LsbKey;
/* target is sealed only by the real entered native writer below. It is
 * a private discriminator, never a caller-supplied grant. */
enum {LSB_BOMB=294,LSB_SMOKE=467};
typedef struct {u32 active,operation,target,frame,thread,nbd_serial,hg_serial,hg_frame,jobs,halted,preissue,seal;} LsbIngress;
typedef char LsbIngressSize[sizeof(LsbIngress)==48?1:-1];
typedef struct {u32 valid,token,frame,operation,key_seal,lsr_seal,owner_hash,pin,
 parent_seal,seal;} LsbTail;
static LsbKey lsb_key __attribute__((section(".bss.gz_lsb")));
static LsbIngress lsb_in __attribute__((section(".bss.gz_lsb")));
static LsbTail lsb_tail __attribute__((section(".bss.gz_lsb")));
static LswHgTicket lsb_hg __attribute__((section(".bss.gz_lsb")));
extern u32 _gz_nbd_context_owned(u32),_gz_nbd_report[16];
extern u32 _gz_lsb_jobs_try(u32,u32,u32),_gz_lsb_jobs_check(u32,u32,u32),_gz_lsb_jobs_leave(u32);
typedef char LsbKeyFits[sizeof(LsbKey)<=512?1:-1];
static void lsb_in_seal(void){lsb_in.seal=btr_hash(&lsb_in,sizeof(lsb_in)-4);__asm__ volatile("sync":::"memory");}
static u32 lsb_active(void){return lsb_key.valid!=0;}
static u32 lsb_key_sealed(void){return lsb_key.magic==LSB_MAGIC&&lsb_key.bytes==sizeof(lsb_key)&&
 lsb_key.version==2&&lsb_key.valid==1&&lsb_key.token&&lsb_key.pin&&lsb_key.smoke_pid&&
 lsb_key.smoke_pid<0xfffffffeu&&lsb_key.source_bomb_pid&&lsb_key.source_bomb_pid<0xfffffffeu&&
 lsb_key.smoke_pid!=lsb_key.source_bomb_pid&&lsb_key.bomb.pid==BAD&&
 lsb_key.seal==btr_hash(&lsb_key,sizeof(lsb_key)-4);}
static u32 lsb_sealed(void){return lsb_key_sealed()&&boh_sealed()&&lsr_sealed(lsb_key.token)&&
 _gz_boh_plan.role==BOH_CREATED&&_gz_boh_plan.token==lsb_key.token&&_gz_boh_plan.pin==lsb_key.pin&&
 _gz_boh_plan.source_pid==lsb_key.source_bomb_pid&&
 _gz_boh_plan_seal==lsb_key.boh_plan&&_gz_boh_plan.hbp_seal==lsb_key.hbp_seal&&
 btr_equal(&_gz_boh_plan.recipe,&lsb_key.bomb,sizeof(PbaKey))&&
 lsr.pin==lsb_key.pin&&lsr.source==lsb_key.source&&lsr.used==lsb_key.used&&lsr.source_hash==lsb_key.source_hash&&
 lsr.source_pid==lsb_key.smoke_pid&&lsr.source_generation==lsb_key.smoke_generation&&lsr.layer==lsb_key.layer;}
/* No queue, SOURCE, actor or callback validation here. This is also used by
 * the real adapter during same-Gate acquisition, so it MUST stay nonrecursive. */
static u32 lsb_owner(u32 t,u32 f,u32 mode){const volatile u32*o=_gz_roh_owner;
 if(!lsb_sealed()||t!=lsb_key.token||f!=_gz_pbac_frame||_gz_roh_serial!=t||
  o[0]!=ROH_MAGIC||o[1]!=240||o[2]!=2||o[4]!=1||o[5]!=t||!o[11])return 0;
 if(mode==3)return o[3]==ROH_FAILED_DIRTY&&o[6]==1&&o[7]&&
  (_gz_scf_has_owner()?scf_original(t,f):bor_original())&&
  !_gz_roh_recovery_common(t,f);
 if(o[7]||_gz_boh_journal.error||lsr.error||(mode!=0&&o[10]!=f))return 0;
 if(mode==1)return o[3]==ROH_CLEAN&&!o[6]&&o[12]==1&&_gz_boh_observe_entered==1&&
  _gz_roh_gl_context(t,f,GL_CHECK)==1;
 if(o[3]!=ROH_MUTATING||o[6]!=1)return 0;
 if(mode==4)return o[12]>=2&&o[12]<=8&&boh_service_bound(t)&&!_gz_roh_replay_common(t,f,0);
 if(!boh_bound(t))return 0;
 if(mode==0)return !o[12];
 if(mode==2)return o[12]==9&&!_gz_roh_lifetime_common(t,0);
 return 0;
}
static u32 lsb_ingress_valid(void){return lsb_in.active==1&&!lsb_in.halted&&lsb_in.preissue<=1&&
 (lsb_in.target==LSB_BOMB||lsb_in.target==LSB_SMOKE)&&lsb_in.seal==btr_hash(&lsb_in,sizeof(lsb_in)-4)&&
 lsb_in.frame==_gz_pbac_frame&&lsb_in.thread==_gz_hg_platform_thread()&&_gz_nbd_context_owned(lsb_in.frame)&&
 _gz_nbd_report[0]==0x4e424431u&&_gz_nbd_report[1]==64&&_gz_nbd_report[2]==1&&
 _gz_nbd_report[4]==lsb_in.nbd_serial&&_gz_nbd_report[5]==lsb_in.frame;}
static u32 lsb_hg_owned(void){const volatile u32*c=_gz_hg_control;const LswHgTicket*k=&lsb_hg;
 return lsb_in.hg_serial&&k->magic==0x48475431u&&k->self==(u32)k&&k->serial==lsb_in.hg_serial&&k->live==1&&
 k->site==(u32)_gz_lsr_ready_callback&&k->frame==lsb_in.hg_frame&&_gz_hg_frame==lsb_in.hg_frame&&
 c[0]==0x48474331u&&c[1]==64&&c[2]==1&&!c[3]&&!(c[4]&0x80000000u)&&(c[4]&0x7fffffffu)&&!c[7];}
/* Metadata purpose only. Physical HG and same-Gate ownership are checked
 * independently. The bomb cases exist only around the ORIGINAL two native
 * calls; no callback, observer, or ordinary actor path may set this scope. */
static u32 lsb_target_context(u32 target,u32 op){
 if(_gz_scf_has_owner())return scf_target_context(target,op);
 if(target==LSB_BOMB){
  if(lsr.phase!=LSR_PREPARED||lsr.pid||lsr.ready)return 0;
  if(op==LSR_ISSUE)return _gz_boh_lifetime_entered==1&&!_gz_boh_pumping&&
   _gz_boh_journal.phase==BOH_ISSUING;
  return op==LSR_PUMP&&!_gz_boh_lifetime_entered&&_gz_boh_pumping==1&&
   _gz_boh_journal.phase==BOH_ACTIVE&&_gz_boh_journal.pid;
 }
 if(target!=LSB_SMOKE||_gz_boh_lifetime_entered||_gz_boh_pumping)return 0;
 if(op==LSR_ISSUE)return _gz_boh_journal.ready&&
  (lsr.phase==LSR_PREPARED||lsr.phase==LSR_ISSUING||lsr.phase==LSR_CREATING);
 if(op==LSR_PUMP)return _gz_boh_journal.ready&&
  (lsr.phase==LSR_CREATING||lsr.phase==LSR_READY);
 return op==LSR_RETIRE&&(lsr.phase==LSR_APPLIED||lsr.phase==LSR_RETIRING||lsr.phase==LSR_DETACHED);
}
/* Physical reservation only, before the ORIGINAL first native Create.
 * CLEAN/pre-service ownership never satisfies lsb_writer_owned or the birth
 * witness. Only original read-only owner/source/queue validation runs in this
 * span; no native method or new source read grant is introduced. */
static u32 lsb_preissue_purpose(u32 t,u32 f){const volatile u32*o=_gz_roh_owner;
 if(!lsb_sealed()||t!=lsb_key.token||f!=lsb_key.frame||f!=_gz_pbac_frame||
  _gz_boh_plan.role!=BOH_CREATED||_gz_boh_journal.pid||_gz_boh_journal.actor||
  _gz_boh_journal.ready||_gz_boh_journal.armed||_gz_boh_journal.error||
  lsr.phase!=LSR_PREPARED||lsr.pid||lsr.ready||lsr.error||lsr.workspace_open||
  _gz_boh_pumping||_gz_lsr_globals_ready(t)!=1)return 0;
 if(_gz_boh_journal.phase==BOH_PREPARED){
  if(_gz_boh_lifetime_entered||o[12])return 0;
  if(o[3]==ROH_CLEAN&&_gz_boh_observe_entered!=1)return 0;
  if(o[3]==ROH_MUTATING&&_gz_boh_observe_entered>1)return 0;
  return _gz_roh_boh_preissue_context(t,f)==1;
 }
 return _gz_boh_journal.phase==BOH_ISSUING&&_gz_boh_lifetime_entered==1&&
  !_gz_boh_observe_entered&&lsb_owner(t,f,2);
}
u32 _gz_lsb_native_claim(u32 t,u32 f,u32 op){
 if(!lsb_ingress_valid()||t!=lsb_key.token||f!=lsb_in.frame||op!=lsb_in.operation||
  (op!=LSR_ISSUE&&op!=LSR_PUMP&&op!=LSR_RETIRE)||lsr.workspace_open||
  !lsb_hg_owned()||_gz_jpg_check(t,f))return 0;
 if(lsb_in.preissue)return lsb_in.target==LSB_BOMB&&op==LSR_ISSUE&&lsb_preissue_purpose(t,f);
 return lsb_owner(t,f,op==LSR_RETIRE?3:2)&&lsb_target_context(lsb_in.target,op);
}
static u32 lsb_writer_owned(u32 op){
 return lsb_ingress_valid()&&!lsb_in.preissue&&lsb_in.operation==op&&lsb_target_context(lsb_in.target,op)&&
 lsb_in.hg_serial&&lsb_in.jobs==1&&
 lsb_hg_owned()&&
 !_gz_lsb_jobs_check(lsb_key.token,lsb_in.frame,op);
}
/* Current heads provide ALL followed request/tag/actor pointers. Never call
 * boh_queues/delta/validate here; those now route into this provider. */
static u32 lsb_current(u32*br,u32*ba,u32*bn,u32*sa,u32*sn,u32*dt){u32 r,a=0,n,z,tag;
 *br=*ba=*bn=*sa=*sn=*dt=0;
 if(!lsb_sealed()||_gz_rll_priority_head)return LSB_CHANGED;
 if(_gz_boh_journal.pid){
  if(!boh_tracker_valid())return LSB_CHANGED;
  n=boh_target_node(_gz_boh_journal.pid,ba);if(n==BAD)return LSB_CHANGED;*bn=n;
 }
 if(lsr.pid){z=lsr_actor(sa,sn);if(z)return z;}
 if((*ba&&*ba==*sa)||(*bn&&*bn==*sn))return LSB_CHANGED;
 r=_gz_pba_native_create_queue;
 if(r){
  if(_gz_pba_native_delete_queue||!span(r,0x50)||R32(r,0)||R32(r,8)||
   R32((u32)&_gz_pba_native_create_queue,4)!=r||R32((u32)&_gz_pba_native_create_queue,8)!=1)return LSB_CHANGED;
  if(lsr.pid&&lsr.phase==LSR_CREATING){z=lsr_request(&n);if(z||n!=r)return LSB_CHANGED;}
  else{
   if(!_gz_boh_journal.pid||lsr.pid||!request_owned(&_gz_boh_tracker,r)||
    R32(r,0x3c)!=_gz_boh_journal.pid)return LSB_CHANGED;*br=r;
   a=R32(r,0x40);if(a&&(!span(a,0xb50)||R32(a,4)!=_gz_boh_journal.pid||R16(a,8)!=294))return LSB_CHANGED;
   if(a&&*ba&&a!=*ba)return LSB_CHANGED;
  }
 }else if(!bor_empty((u32)&_gz_pba_native_create_queue))return LSB_CHANGED;
 tag=_gz_pba_native_delete_queue;
 if(tag){
  if(r||!span(tag,28)||R32(tag,0)||R32(tag,4)!=(u32)&_gz_pba_native_delete_queue||R32(tag,8)||
   R8(tag,16)!=1||R32((u32)&_gz_pba_native_delete_queue,4)!=tag||
   R32((u32)&_gz_pba_native_delete_queue,8)!=1)return LSB_CHANGED;
  if(lsr.phase==LSR_RETIRING){
   if(!*sa||*sa!=lsr.actor||*sn!=lsr.node||tag!=*sa+0x4c||R32(tag,12)!=*sa||
    R32(tag,20)!=lsr.layer||lsr.delete_calls!=1)return LSB_CHANGED;
  }else if(_gz_scf_has_owner()){
   if(!scf_bomb_tag(*ba,*bn,tag))return LSB_CHANGED;
  }else if(lsr.phase==LSR_DEAD&&bor_original()&&_gz_boh_recovery.phase==BOR_RETIRING){
   if(!*ba||*ba!=_gz_boh_recovery.actor||tag!=*ba+0x4c||R32(tag,12)!=*ba||
    R32(tag,20)!=_gz_boh_tracker.request_layer||_gz_boh_recovery.delete_calls!=1)return LSB_CHANGED;
  }else return LSB_CHANGED;
 }else if(!bor_empty((u32)&_gz_pba_native_delete_queue))return LSB_CHANGED;
 *dt=tag;
 if(_gz_boh_journal.ready&&*ba){
  if(_gz_scf_has_owner()&&boh_binding_current(*ba,_gz_boh_tracker.model_audit,1))return LSB_CHANGED;
  if(*ba!=_gz_boh_journal.actor||!actor_owned(&_gz_boh_tracker,*ba))return LSB_CHANGED;
  if(_gz_roh_owner[3]!=ROH_FAILED_DIRTY&&
   (boh_binding_current(*ba,_gz_boh_tracker.model_audit,1)||R8(*ba,12)!=2))return LSB_CHANGED;
 }
 if(lsr.ready&&*sa&&(*sa!=lsr.actor||*sn!=lsr.node||_gz_ob_validate(&lsr.actor_binding)))return LSB_CHANGED;
 return 0;
}
static u32 lsb_descriptor(RllPairTarget*d){u32 br,ba,bn,sa,sn,dt,z,op,mask;LsbPhaseInput s;
 z=lsb_current(&br,&ba,&bn,&sa,&sn,&dt);if(z)return z;
 s.bomb_phase=_gz_boh_journal.phase;s.bomb_pid=_gz_boh_journal.pid;s.bomb_ready=_gz_boh_journal.ready;
 s.smoke_phase=lsr.phase;s.smoke_pid=lsr.pid;s.recovery_phase=bor_sealed()?_gz_boh_recovery.phase:0;
 s.bomb_present=!!bn;s.smoke_present=!!sn;
 if(_gz_scf_has_owner()){if(!scf_pair_phase(&op,&mask,!!bn,!!sn))return LSB_CHANGED;}
 else if(!lsb_phase(&s,&op,&mask))return LSB_CHANGED;
 bcl_zero(d,sizeof(*d));d->bytes=sizeof(*d);d->version=1;d->token=lsb_key.token;d->frame=_gz_pbac_frame;
 d->key=(u32)&lsb_key;d->key_bytes=sizeof(lsb_key);d->operation=op;d->issued_mask=mask;
 d->target[0].source_pid=lsb_key.source_bomb_pid;d->target[1].source_pid=lsb_key.smoke_pid;
 if(mask&1){d->target[0].live_pid=_gz_boh_journal.pid;d->target[0].live_actor=bn?ba:0;d->target[0].actor_queue_node=bn;}
 if(mask&2){d->target[1].live_pid=lsr.pid;d->target[1].live_actor=sn?sa:0;d->target[1].actor_queue_node=sn;}
 return 0;
}
u32 _gz_rll2_backend_context(const RllPairTarget*d,u32 mode){RllPairTarget actual;
 u32 p=(u32)d;
 if((p&3)||p<0x01800000u||p>0xbffe0000u-sizeof(*d)||!lsb_owner(d->token,d->frame,mode)||
  lsb_descriptor(&actual)||!btr_equal(d,&actual,sizeof(actual)))return 0;
 /* Native calls require the separate actual writer. Poll/replay comparisons
  * are read-only and retain their original ROH lane; no writer is invented. */
 return !lsb_in.active||lsb_ingress_valid();
}
static u32 lsb_delta(u32 mode){RllPairTarget d;u32 z=lsb_descriptor(&d);if(z)return z;
 if(mode==1)return _gz_rll2_observe_create_delta(&d);
 if(mode==3)return _gz_rll2_recovery_create_delta(&d);
 if(mode==4)return _gz_rll2_replay_create_delta(&d);
 return mode==2?_gz_rll2_check_create_delta(&d):_gz_rll2_poll_create_delta(&d);}
static u32 lsb_unissued(u32 t){return t==lsb_key.token?
 lsb_delta(_gz_roh_owner[3]==ROH_CLEAN?1:(_gz_roh_owner[12]?2:0)):LSB_CONTEXT;}
static u32 lsb_queues(u32*r,u32*a,u32*n){u32 sa,sn,dt,z;
 z=lsb_current(r,a,n,&sa,&sn,&dt);if(z)return z;
 if(dt)return LSB_CHANGED;
 if(!*a&&*r)*a=R32(*r,0x40);return 0;}
static u32 lsb_recovery_queues(u32*a,u32*n,u32*t){u32 r,sa,sn,z;
 z=lsb_current(&r,a,n,&sa,&sn,t);if(z||r)return z?z:LSB_CHANGED;
 if(lsr.phase==LSR_RETIRING&&*t){const BorJournal*j=&_gz_boh_recovery;
  /* The single exact467 tag was authenticated above. Preserve every original
   * live294 allocation/method/model/layer check while its own tag is absent. */
  if(!bor_original()||j->phase!=BOR_ROLLED||!*a||!*n||*a!=j->actor||*n!=*a+0xc8||
   !span(*n,20)||R8(*n,16)!=1||boh_binding_current(*a,_gz_boh_tracker.model_audit,1)||
   !actor_owned(&_gz_boh_tracker,*a)||R32(*a,0x10)!=_gz_boh_tracker.profile_audit||
   R32(*a,0xa8)!=_gz_boh_tracker.methods_audit||R32(*a,0x3bc)!=_gz_boh_tracker.model_audit||
   !_gz_boh_retirement_layer_certificate(*a,j->phase,_gz_boh_tracker.request_layer)||R32(*a,0xac))return LSB_CHANGED;
  *t=0;return 0;}
 return bor_queues_single(a,n,t);}
static u32 lsb_recovery_delta(void){return lsb_delta(3);}

/* Return0 preserves the original single-bomb preparation verbatim. Only the
 * fixed captured one-LAND_SMOKE form selects this collective participant. */
static u32 lsb_prepare(u32 t,u32 f){u32 source,cg,fx,z;const FxbRow*r;RllPairTarget d;
 if(lsb_active()||lsb_in.halted||lsb_in.active||lsb_in.preissue||lsb_hg.live||lsb_tail.valid)return LSB_RETAINED;
 source=_gz_hbp_recipe.source;
 if(!_gz_hbp_read(t,f))return LSB_CONTEXT;
 if(_gz_bcb_archive_validate(source,_gz_hbp_recipe.used,0)||_gz_cga_archive_validate(source,_gz_hbp_recipe.used,0))return LSB_CONTEXT;
 cg=source+R32(source,240);if(R32(cg,8)!=4)return 0;
 fx=cg+R32(cg,120);if(R32(fx,48)!=1||R32(fx,56)!=1)return 0;
 /* Full archive validation above authenticates the versioned row offset.
  * LSR itself requires FXB2 global evidence; legacy FXB1 is not inferred. */
 if(!((R32(fx,8)==1&&R32(fx,72)==FXB_HEADER)||
      (R32(fx,8)==2&&R32(fx,72)==FXB2_HEADER)))return LSB_CONTEXT;
 r=(const FxbRow*)(fx+R32(fx,72));if(r->kind!=3||r->resource!=0x2027||r->group!=2)return 0;
 if(_gz_boh_plan.role!=BOH_CREATED||!_gz_boh_plan.source_pid||_gz_boh_plan.source_pid>=0xfffffffeu||
  _gz_boh_plan.source_pid!=_gz_hbp_recipe.saved.pid||_gz_boh_plan.recipe.pid!=BAD||
  _gz_hbp_recipe.recipe.pid!=BAD||_gz_boh_plan.source_pid==r->pid)return LSB_CONTEXT;
 _gz_boh_journal.phase=BOH_PREPARED;_gz_boh_journal.token=t;boh_seal_run();
 _gz_boh_plan.valid=1;_gz_boh_plan_seal=btr_hash(&_gz_boh_plan,sizeof(_gz_boh_plan));
 z=_gz_lsr_prepare(t,f);if(z)return z;
 bcl_zero(&lsb_key,sizeof(lsb_key));lsb_key.magic=LSB_MAGIC;lsb_key.bytes=sizeof(lsb_key);lsb_key.version=2;
 lsb_key.valid=1;lsb_key.token=t;lsb_key.source_bomb_pid=_gz_boh_plan.source_pid;lsb_key.frame=f;lsb_key.pin=lsr.pin;lsb_key.hbp_seal=_gz_hbp_seal;
 lsb_key.boh_plan=_gz_boh_plan_seal;lsb_key.source_hash=lsr.source_hash;lsb_key.source=lsr.source;lsb_key.used=lsr.used;
 lsb_key.smoke_pid=lsr.source_pid;lsb_key.smoke_generation=lsr.source_generation;lsb_key.layer=lsr.layer;
 btr_copy(&lsb_key.bomb,&_gz_boh_plan.recipe,sizeof(PbaKey));lsb_key.seal=btr_hash(&lsb_key,sizeof(lsb_key)-4);
 z=lsb_descriptor(&d);if(z)return z;z=_gz_rll2_prepare_create(&d);return z?z:1;
}
static u32 lsb_native_leave(u32 result){u32 z;
 if(lsb_in.jobs){z=_gz_lsb_jobs_leave(lsb_key.token);if(z){lsb_in.halted=1;lsb_in_seal();return LSB_RETAINED;}
  lsb_in.jobs=0;lsb_in_seal();}
 if(lsb_in.hg_serial){z=_gz_lsw_hg_exit(&lsb_hg,lsb_in.hg_serial);
  if(z||lsb_hg.live){lsb_in.halted=1;lsb_in_seal();return LSB_RETAINED;}
  lsb_in.hg_serial=0;lsb_in_seal();}
 lsb_in.active=0;lsb_in.operation=0;lsb_in.target=0;lsb_in.preissue=0;lsb_in_seal();return result;
}
/* Admission is before mark_mutating. The genuine service1 GPU fence is
 * still current: JPG invalidates it on every admitted particle method. Never
 * wait for the GPU after taking HG/the same-Gate reservation. */
static u32 lsb_bomb_preissue_begin(u32 t,u32 f){u32 z;const volatile u32*c=_gz_hg_control;
 if(!lsb_active())return 0;
 if(lsb_in.active||lsb_in.halted||lsb_in.jobs||lsb_in.hg_serial||lsb_in.preissue||lsb_hg.live)return LSB_RETAINED;
 if(_gz_roh_owner[3]!=ROH_CLEAN||!lsb_preissue_purpose(t,f)||!_gz_nbd_context_owned(f)||
  !_gz_nbd_report[4]||_gz_nbd_report[4]==BAD||_gz_nbd_report[5]!=f||
  c[3]||(c[4]&0x80000000u)||c[7]||_gz_jpg_check(t,f)||_gz_jpg_gpu_restore_ready(t,f))return LSB_CONTEXT;
 bcl_zero(&lsb_in,sizeof(lsb_in));lsb_in.active=1;lsb_in.operation=LSR_ISSUE;
 lsb_in.target=LSB_BOMB;lsb_in.preissue=1;lsb_in.frame=f;
 lsb_in.thread=_gz_hg_platform_thread();lsb_in.nbd_serial=_gz_nbd_report[4];lsb_in.hg_frame=_gz_hg_frame;lsb_in_seal();
 z=_gz_lsw_hg_enter(&lsb_hg,(u32)_gz_lsr_ready_callback);
 if(!z){lsb_in.halted=1;lsb_in_seal();return LSB_RETAINED;}lsb_in.hg_serial=z;lsb_in_seal();
 z=_gz_lsb_jobs_try(t,f,LSR_ISSUE);
 if(z==LSB_RETAINED){lsb_in.halted=1;lsb_in_seal();return LSB_RETAINED;}
 if(z)return lsb_native_leave(z);
 lsb_in.jobs=1;lsb_in_seal();
 if(!_gz_lsb_native_claim(t,f,LSR_ISSUE)||_gz_lsb_jobs_check(t,f,LSR_ISSUE)||
  _gz_jpg_gpu_restore_ready(t,f))return lsb_native_leave(LSB_CONTEXT);
 return 0;
}
/* Unwind only a positively sealed unpromoted reservation. Native/token
 * uncertainty retains BOTH tokens and the original root; no clean receipt
 * can consume an active/halted ingress. This never clears an issued PID. */
static u32 lsb_bomb_preissue_end(u32 t,u32 result){
 if(!lsb_active()||!lsb_in.preissue)return result;
 if(t!=lsb_key.token||!lsb_ingress_valid()||lsb_in.target!=LSB_BOMB||lsb_in.operation!=LSR_ISSUE){
  lsb_in.halted=1;lsb_in_seal();return LSB_RETAINED;}
 return lsb_native_leave(result);
}
static u32 lsb_native_begin(u32 t,u32 op,u32 target){u32 z,f=_gz_pbac_frame;const volatile u32*c=_gz_hg_control;
 if(lsb_in.active||lsb_in.halted||lsb_in.jobs||lsb_in.hg_serial||lsb_in.preissue||lsb_hg.live)return LSB_RETAINED;
 if(!lsb_target_context(target,op)||!lsb_owner(t,f,op==LSR_RETIRE?3:2)||!_gz_nbd_context_owned(f)||!_gz_nbd_report[4]||
  _gz_nbd_report[4]==BAD||_gz_nbd_report[5]!=f||c[3]||(c[4]&0x80000000u)||c[7]||_gz_jpg_check(t,f))return LSB_CONTEXT;
 /* Fence before writer acquisition; no exclusive/read lease may be upgraded. */
 z=_gz_jpg_gpu_restore(t,f);if(z)return z;
 bcl_zero(&lsb_in,sizeof(lsb_in));lsb_in.active=1;lsb_in.operation=op;lsb_in.target=target;lsb_in.frame=f;
 lsb_in.thread=_gz_hg_platform_thread();lsb_in.nbd_serial=_gz_nbd_report[4];lsb_in.hg_frame=_gz_hg_frame;lsb_in_seal();
 z=_gz_lsw_hg_enter(&lsb_hg,(u32)_gz_lsr_ready_callback);
 if(!z){lsb_in.halted=1;lsb_in_seal();return LSB_RETAINED;}lsb_in.hg_serial=z;lsb_in_seal();
 z=_gz_lsb_jobs_try(t,f,op);
 /* A real RETAINED outcome may carry an active same-Gate token even before
  * this caller marks jobs=1. Preserve BOTH permanent tokens on uncertainty;
  * native_leave must not exit HG while that physical job lease can be live. */
 if(z==LSB_RETAINED){lsb_in.halted=1;lsb_in_seal();return LSB_RETAINED;}
 if(z)return lsb_native_leave(z);
 lsb_in.jobs=1;lsb_in_seal();if(!lsb_writer_owned(op))return lsb_native_leave(LSB_CONTEXT);
 return 0;
}
static u32 lsb_bomb_begin(u32 t,u32 op){
 if(!lsb_active())return 0;
 if(op!=LSR_ISSUE&&op!=LSR_PUMP)return LSB_CONTEXT;
 if(op==LSR_ISSUE){
  /* Promotion consumes only the SAME genuine pre-mutation reservation.
   * There is no new fallible Gate TRY after BOH_ISSUING/mark_mutating. */
  if(!lsb_in.preissue)return LSB_CONTEXT;
  if(!lsb_ingress_valid()||t!=lsb_key.token||lsb_in.target!=LSB_BOMB||lsb_in.operation!=op||
   !lsb_target_context(LSB_BOMB,op)||!lsb_owner(t,_gz_pbac_frame,2)||!lsb_hg_owned()||
   lsb_in.jobs!=1||_gz_lsb_jobs_check(t,_gz_pbac_frame,op)||
   _gz_jpg_gpu_restore_ready(t,_gz_pbac_frame))return lsb_bomb_preissue_end(t,LSB_CONTEXT);
  lsb_in.preissue=0;lsb_in_seal();
  return lsb_writer_owned(op)?0:lsb_native_leave(LSB_CONTEXT);
 }
 return lsb_native_begin(t,op,LSB_BOMB);
}
static u32 lsb_bomb_end(u32 t,u32 op,u32 result){
 if(!lsb_active())return result;
 if(t!=lsb_key.token||lsb_in.target!=LSB_BOMB||!lsb_writer_owned(op))
  return lsb_native_leave(LSB_CONTEXT);
 return lsb_native_leave(result);
}
static u32 lsb_native(u32 t,u32 op){u32 z;
 z=lsb_native_begin(t,op,LSB_SMOKE);if(z)return z;
 if(op==LSR_ISSUE)z=_gz_lsr_issue(t);
 else if(op==LSR_PUMP)z=_gz_lsr_pump(t);
 else if(op==LSR_RETIRE){
  z=_gz_scf_has_owner()?scf_smoke_preflight():0;
  if(!z)z=_gz_lsr_retire_native(t);
 }
 else z=LSB_CONTEXT;
 if(!lsb_writer_owned(op))return lsb_native_leave(LSB_CONTEXT);
 return lsb_native_leave(z);
}
/* Leaf context: writer checks are separate from pair list comparison to avoid
 * recursive BOH validation, and from workspace claim to avoid Gate recursion. */
/* Extended scopes authenticate the actual original reader/recovery entry.
 * Never call BOH_validate, a FINAL commit certificate, or a source validator
 * from here. Op12 is purpose-only both before and INSIDE the actual LSW writer.
 * The leaf separately proves lsw_owned(); demanding HG OPEN here would strand
 * that legitimate second check after writer acquisition. */
static u32 bmv2_identity_scope(u32,u32);
static u32 lsb_extended_context(u32 t,u32 f,u32 op){const volatile u32*o=_gz_roh_owner;
 if(!lsb_sealed()||t!=lsb_key.token||f!=_gz_pbac_frame||_gz_roh_serial!=t||
  o[0]!=ROH_MAGIC||o[1]!=240||o[2]!=2||o[4]!=1||o[5]!=t||!o[11]||
  _gz_osa_pin_id()!=lsb_key.pin||lsb_in.active||lsb_in.jobs||lsb_in.hg_serial||
  lsb_in.halted||lsb_in.preissue||lsb_hg.live||lsr.workspace_open||_gz_jpg_check(t,f))return 0;
 if(op==LSR_SAVED_MATCH)return o[3]==ROH_MUTATING&&o[6]==1&&!o[7]&&
  lsr.phase==LSR_APPLIED&&lsr.ready&&!lsr.error&&
  (bsg_held(t,2)||bsg_held(t,3)||bsg_held(t,5));
 if(op==LSR_IDENTITY_MATCH)return bmv2_identity_scope(t,f);
 if(op==LSR_GLOBAL_PREPARE)return o[3]==ROH_CLEAN&&!o[6]&&!o[7]&&o[12]==1&&
  _gz_boh_observe_entered==1&&bsg_held(t,1)&&lsr.phase==LSR_PREPARED&&!lsr.error&&
  !lsr.pid&&!lsr.ready&&!_gz_boh_journal.pid&&!_gz_boh_journal.ready;
 if(op==LSR_GLOBAL_ROLLBACK)return o[3]==ROH_FAILED_DIRTY&&o[6]==1&&o[7]&&
  lsr.phase==LSR_DEAD&&lsr.remove_returned&&lsr.jpa_before_valid&&lsr.jpa_saved_valid&&
  _gz_boh_recovery_globals_authority(t,f)==1;
 if(op==LSR_GLOBAL_BEFORE_MATCH)return o[3]==ROH_FAILED_DIRTY&&o[6]==1&&o[7]&&
  lsr.phase==LSR_DEAD&&lsr.remove_returned&&lsr.jpa_before_valid&&lsr.jpa_rolled&&bsg_held(t,6);
 return 0;
}
u32 _gz_lsr_bundle_context(u32 t,u32 f,u32 op){const volatile u32*o=_gz_roh_owner;u32 mode,z;
 if(op>=LSR_SAVED_MATCH&&op<=LSR_GLOBAL_BEFORE_MATCH)return lsb_extended_context(t,f,op);
 if(op>=LSR_FINAL_COMMIT&&op<=LSR_FINAL_CANCEL){
  /* These are PRE-release validation scopes. No postrelease JEL/native read
   * or forget grant exists. The original owner alone enters the fixed tail. */
  if(!lsb_sealed()||t!=lsb_key.token||f!=_gz_pbac_frame||_gz_roh_serial!=t||o[4]!=1||o[5]!=t||
   _gz_osa_pin_id()!=lsb_key.pin||lsb_in.active||lsb_in.jobs||lsb_in.hg_serial||lsb_in.halted||lsb_in.preissue||
   lsb_hg.live||lsr.workspace_open||_gz_jpg_check(t,f))return 0;
  if(op==LSR_FINAL_COMMIT)return o[3]==ROH_MUTATING&&o[6]==1&&!o[7]&&!o[12]&&_gz_boh_commit_scope(t,f)==1;
  if(op==LSR_FINAL_ROLLBACK)return o[3]==ROH_FAILED_DIRTY&&o[6]==1&&o[7]&&_gz_boh_recovery_finish_scope(t,f)==1;
  return o[3]==ROH_FAILED_CLEAN&&!o[6]&&o[7]&&o[12]<=1&&_gz_bfm_clean_scope(t,f)==1;
 }
 mode=o[3]==ROH_FAILED_DIRTY?3:o[3]==ROH_CLEAN?1:o[12]?2:0;
 if(!lsb_owner(t,f,mode)||_gz_jpg_check(t,f)||lsb_delta(mode))return 0;
 if(op==LSR_OBSERVE)return mode==1;
 if(op==LSR_ISSUE||op==LSR_PUMP)return mode==2&&lsb_writer_owned(op);
 if(op==LSR_PARTICLES){const volatile u32*c=_gz_hg_control;
  return mode==2&&!lsb_in.active&&!lsb_in.halted&&!lsb_hg.live&&!c[3]&&!(c[4]&0x80000000u)&&!c[7]&&
   _gz_nbd_context_owned(f)&&_gz_nbd_report[5]==f;
 }
 if(op!=LSR_RETIRE||mode!=3)return 0;
 if(lsb_in.active)return lsb_writer_owned(op);
 z=lsr.phase==LSR_DETACHED&&lsr.remove_returned&&lsr_emitter_dead();
 return z&&!lsb_in.halted&&!lsb_in.jobs&&!lsb_in.hg_serial&&!lsb_hg.live&&_gz_nbd_context_owned(f);
}
static u32 lsb_forward(u32 t){u32 z;
 if(!lsb_active())return 0;
 if(!lsb_owner(t,_gz_pbac_frame,2)||!_gz_boh_journal.ready)return LSB_CONTEXT;
 if(lsr.phase==LSR_PREPARED){z=lsb_native(t,LSR_ISSUE);return z?z:5;}
 if(lsr.phase==LSR_CREATING){z=lsb_native(t,LSR_PUMP);if(z)return z;}
 if(lsr.phase==LSR_READY||lsr.phase==LSR_ALLOCATING){z=_gz_lsr_apply_particles(t);if(z)return z;}
 return lsr.phase==LSR_APPLIED?lsb_delta(2):LSB_CONTEXT;
}
static u32 lsb_recovery_smoke(u32 t){u32 z;
 if(!lsb_active())return 0;
 if(lsr.phase==LSR_DEAD)return 0;
 if(!lsb_owner(t,_gz_pbac_frame,3)||_gz_boh_recovery.phase!=BOR_ROLLED||lsr.phase<LSR_READY)return LSB_CONTEXT;
 if(lsr.phase!=LSR_DETACHED||!lsr.remove_returned){z=lsb_native(t,LSR_RETIRE);if(z)return z;}
 z=_gz_lsr_retire_finish(t);return z?z:lsb_delta(3);
}
/* The ORIGINAL owner calls these at its final checked edge and after its
 * actual root/pin release. The outcome argument selects a proof, never grants
 * one. No report array, prior unrelated commit or external callback is used. */
u32 _gz_lsb_tail_prepare(u32 t,u32 f,u32 op){u32 outcome,parent;
 if(!lsb_active())return 0;
 if(!lsb_sealed()||t!=lsb_key.token||f!=_gz_pbac_frame||lsb_in.active||lsb_in.halted||lsb_in.jobs||
  lsb_in.hg_serial||lsb_in.preissue||lsb_hg.live||lsb_tail.valid||lsr.workspace_open)return LSB_CONTEXT;
 if(op==LSR_FINAL_COMMIT){if(_gz_boh_commit_scope(t,f)!=1||lsr.phase!=LSR_APPLIED||lsr.error||lsb_delta(0))return LSB_CONTEXT;
  outcome=1;parent=_gz_boh_run_seal;}
 else if(op==LSR_FINAL_ROLLBACK){if(_gz_boh_recovery_finish_scope(t,f)!=1||lsr.phase!=LSR_DEAD||
  !lsr.remove_returned||!lsr_emitter_dead()||lsb_delta(3))return LSB_CONTEXT;outcome=0;parent=_gz_boh_finish_seal;}
 else if(op==LSR_FINAL_CANCEL){if(_gz_bfm_clean_scope(t,f)!=1||lsr.phase!=LSR_PREPARED||lsr.pid||lsr.ready||
  lsr.callback_count||lsr.delete_calls||lsr.remove_issued)return LSB_CONTEXT;outcome=2;parent=_gz_bcr_seal;}
 else return LSB_CONTEXT;
 if(!parent||_gz_lsr_tail_validate(t,outcome)||_gz_osa_pin_id()!=lsb_key.pin)return LSB_CONTEXT;
 bcl_zero(&lsb_tail,sizeof(lsb_tail));lsb_tail.valid=1;lsb_tail.token=t;lsb_tail.frame=f;lsb_tail.operation=op;
 lsb_tail.key_seal=lsb_key.seal;lsb_tail.lsr_seal=lsr.seal;lsb_tail.owner_hash=btr_hash((const void*)_gz_roh_owner,240);
 lsb_tail.pin=lsb_key.pin;lsb_tail.parent_seal=parent;lsb_tail.seal=btr_hash(&lsb_tail,sizeof(lsb_tail)-4);return 0;
}
void _gz_lsb_tail_finish(u32 t,u32 f,u32 op){
 /* Called only in each original owner's fixed tail after ALL current checks
  * and root/pin release. No native/JEL/source/context reads or error branches.
  * The parameters remain an explicit call-site audit, not capabilities. */
 (void)t;(void)f;(void)op;
 if(!lsb_active())return;
 lsb_tail.valid=0;
 _gz_lsr_finish_leaf();lsc2_finish_leaf();
 _gz_rll2_recovery_finish_leaf();bcl_zero(&lsb_key,sizeof(lsb_key));bcl_zero(&lsb_tail,sizeof(lsb_tail));
 bcl_zero(&lsb_in,sizeof(lsb_in));bcl_zero(&lsb_hg,sizeof(lsb_hg));
}

#include "bundle_jpg_admission.inc"
