/* Explicit face participant inside the held player service. The standalone
 * replay leaf has no authority; only these gates admit its native writes.
 * The original general service9 SDK/constructor gate remains unchanged. */
#include "face_replay.c"
typedef struct {
 u32 magic,bytes,valid,token,frame,source,used,pin,body_plan_seal;
 PoEntry process;GhIdentity heaps[3];LpcArc arcs[2];u32 archives[6];
 FaceReplayPlan leaf;
} LpfPlan;
typedef char LpfPlanSize[sizeof(LpfPlan)==6844?1:-1];
static LpfPlan _gz_lpf_plan;
static FaceReplayRun _gz_lpf_run;
static u32 _gz_lpf_seal,_gz_lpf_run_seal,_gz_lpf_entered;
u32 _gz_lpf_report[32]={0x4c504631u,128};
static void lpf_run_seal(void){_gz_lpf_run_seal=btr_hash(&_gz_lpf_run,sizeof(_gz_lpf_run));}
static u32 lpf_sealed(void){const LpfPlan*p=&_gz_lpf_plan;
 return p->magic==0x4c504631u&&p->bytes==sizeof(*p)&&p->valid==1&&p->token&&
  _gz_lpf_seal==btr_hash(p,sizeof(*p))&&_gz_lpf_run_seal==btr_hash(&_gz_lpf_run,sizeof(_gz_lpf_run))&&
  fr_sealed(&p->leaf,&_gz_lpf_run)&&lpc_sealed()&&p->body_plan_seal==_gz_lpc_seal&&
  p->token==_gz_lpc_plan.token&&p->source==_gz_lpc_plan.source&&p->used==_gz_lpc_plan.used&&p->pin==_gz_lpc_plan.pin;
}
/* Reacquire every current native root. The immutable source body is never
 * consulted here, so the same check is usable by source-free recovery. */
static u32 lpf_binding(u32 capture){LpfPlan*p=&_gz_lpf_plan;PoEntry e;LpcArc arc;u32 i,j,z,triplet[3],heap,a,id,wrapper,data;
 z=lpc_binding(0);if(z)return z;z=lpc_process(&e);if(z)return z;
 a=e.object;heap=R32(a,0xf4);
 if(capture)btr_copy(&p->process,&e,sizeof(e));
 else if(!btr_equal(&p->process,&e,sizeof(e)))return LPC_PROCESS;
 for(i=0;i<2;i++){
  z=fpc_archive(i?0x969e36cfu:237211244u,&arc,&triplet[0],&triplet[1],&triplet[2]);if(z)return z;
  if(capture){btr_copy(&p->arcs[i],&arc,sizeof(arc));btr_copy(&p->archives[3*i],triplet,12);}
  else if(!btr_equal(&p->arcs[i],&arc,sizeof(arc))||!btr_equal(&p->archives[3*i],triplet,12))return LPC_RESOURCE;
 }
 for(i=0;i<3;i++){
  u32 object=a+fr_offsets[i],buffer=R32(object,0x30);
  if(_gz_gh_solid(heap,buffer,8192))return LPC_HEAP;
  if(capture)btr_copy(&p->heaps[i],(const void*)&_gz_gh_identity,sizeof(GhIdentity));
  else if(!btr_equal(&p->heaps[i],(const void*)&_gz_gh_identity,sizeof(GhIdentity))||buffer!=p->leaf.before_plans[i].buffer)return LPC_HEAP;
  if(i&&!btr_equal(&p->heaps[0],&p->heaps[i],40))return LPC_HEAP;
  for(j=0;j<i;j++)if(buffer<p->heaps[j].model+8192&&p->heaps[j].model<buffer+8192)return LPC_HEAP;
 }
 if(capture)return 0;
 if(a!=p->leaf.bounds.actor||heap!=p->leaf.bounds.heap||p->leaf.bounds.head!=p->heaps[0].head||
  p->leaf.bounds.tail!=p->heaps[0].tail||p->leaf.bounds.end!=p->heaps[0].start+p->heaps[0].bytes)return LPC_HEAP;
 /* Certify BOTH saved-target and private before-target wrappers through the
  * current resident table. Neither is permission to follow a saved pointer. */
 for(i=0;i<2;i++){
  z=fr_recipe(p->leaf.source.recipes+8*i,&id);if(z)return z;
  z=fpc_wrapper(&p->arcs[0],id,!i,&wrapper,&data);if(z)return z;
  if(wrapper!=p->leaf.wrappers[i]||data!=p->leaf.target_plans[i?2:0].resource)return LPC_RESOURCE;
  z=fr_recipe(p->leaf.before.recipes+8*i,&id);if(z)return z;
  z=fpc_wrapper(&p->arcs[0],id,!i,&wrapper,&data);if(z)return z;
  if(wrapper!=p->leaf.before.wrappers[i]||data!=p->leaf.before_plans[i?2:0].resource)return LPC_RESOURCE;
 }
 /* The current model wrapper must still be the same registered Link member. */
 wrapper=R32(a,0x444);for(i=0;i<p->arcs[1].count;i++)if(R32(p->arcs[1].table,i*4)==wrapper)break;
 if(i==p->arcs[1].count||R32(wrapper,0)!=p->leaf.bounds.model)return LPC_RESOURCE;
 return 0;
}
u32 _gz_lpf_prepare(u32 token,u32 frame){LpfPlan*p=&_gz_lpf_plan;FaceSource source;u32 z,id,i,wrappers[2],data,flags;
 if(_gz_lpf_entered||p->valid||_gz_lpf_run.phase||_gz_lpf_seal)return 28;
 if(!token||_gz_roh_gl_context(token,frame,1)!=1||!_gz_hbp_read(token,frame)||!lpc_sealed()||
  _gz_lpc_plan.token!=token||_gz_lpc_run.phase!=LPC_PREPARED)return 42;
 _gz_lpf_entered=1;
 p->magic=0x4c504631u;p->bytes=sizeof(*p);p->token=token;p->frame=frame;
 p->source=_gz_lpc_plan.source;p->used=_gz_lpc_plan.used;p->pin=_gz_lpc_plan.pin;p->body_plan_seal=_gz_lpc_seal;
 z=lpf_binding(1);if(z)goto done;
 if(!lpc_value(_gz_lpc_plan.saved,0x6980,&flags)){z=LPC_SOURCE;goto done;}
 z=_gz_face_source_normalize(_gz_lpc_plan.saved,flags,&source);if(z)goto done;
 z=fpc_observe(p->process.object);if(z)goto done;
 for(i=0;i<2;i++){
  z=fr_recipe(source.recipes+8*i,&id);if(z)goto done;
  z=fpc_wrapper(&p->arcs[0],id,!i,&wrappers[i],&data);if(z)goto done;
 }
 z=_gz_face_replay_prepare(&_gz_fpc_observed_bounds,wrappers,&source,&p->leaf,&_gz_lpf_run);if(z)goto done;
 z=lpf_binding(0);if(z)goto done;
 if(_gz_roh_gl_context(token,frame,1)!=1||!_gz_hbp_read(token,frame)){z=42;goto done;}
 p->valid=1;_gz_lpf_seal=btr_hash(p,sizeof(*p));lpf_run_seal();
 _gz_lpf_report[2]=token;_gz_lpf_report[3]=1;_gz_lpf_report[5]=frame;
 _gz_lpf_report[8]=p->process.object;_gz_lpf_report[9]=p->process.pid;
done:_gz_lpf_report[4]=z;_gz_lpf_entered=0;return z;
}
static u32 lpf_gate(u32 token,u32 frame){
 if(!lpf_sealed()||_gz_lpf_plan.token!=token||_gz_lpc_run.phase!=LPC_APPLIED)return 28;
 /* Retain the complete original-source pin, exact player component,
  * completed constructor, service2 receipt and held same-frame owner gates. */
 return lpc_gate(token,frame);
}
u32 _gz_lpf_apply(u32 token){LpfPlan*p=&_gz_lpf_plan;u32 z,frame=_gz_pbac_frame;
 if(_gz_lpf_entered||!lpf_sealed()||p->token!=token||_gz_lpf_run.phase!=1)return 28;
 _gz_lpf_entered=1;z=lpf_gate(token,frame);if(z)goto done;z=lpf_binding(0);if(z)goto done;
 z=_gz_face_replay_apply(&p->leaf,&_gz_lpf_run,1);lpf_run_seal();if(z)goto done;
 z=lpf_binding(0);if(z)goto done;z=lpf_gate(token,frame);if(z)goto done;
 _gz_lpf_report[3]=3;_gz_lpf_report[6]=frame;_gz_lpf_report[10]=_gz_lpf_run.native_calls;
done:
 _gz_lpf_run.error=z;lpf_run_seal();_gz_lpf_report[4]=z;_gz_lpf_entered=0;return z;
}
u32 _gz_lpf_recovery_rollback(u32 token){LpfPlan*p=&_gz_lpf_plan;u32 z,frame=_gz_pbac_frame;
 if(_gz_lpf_entered||!lpf_sealed()||p->token!=token||_gz_lpf_run.phase==5)return 28;
 _gz_lpf_entered=1;
 if(!_gz_boh_recovery_authority(token,_gz_boh_journal.actor,_gz_boh_plan.link,frame)){z=40;goto done;}
 z=lpf_binding(0);if(z)goto done;
 z=_gz_face_replay_undo(&p->leaf,&_gz_lpf_run);lpf_run_seal();if(z)goto done;
 z=lpf_binding(0);if(z)goto done;
 if(!_gz_boh_recovery_authority(token,_gz_boh_journal.actor,_gz_boh_plan.link,frame)){z=40;goto done;}
 _gz_lpf_report[3]=5;_gz_lpf_report[7]=frame;
done:_gz_lpf_run.error=z;lpf_run_seal();_gz_lpf_report[4]=z;_gz_lpf_entered=0;return z;
}
