/* Completed-constructor -> typed-service handoff. The original service9 SDK
 * writer gate stays unchanged. Camera service5 has its own typed replay and
 * preimage/undo participant. Pot service8 likewise replays its typed bank;
 * the other services still consume mapped inputs. */
extern u32 _gz_roh_replay_common(u32,u32,u32),_gz_roh_replay_ready(void);
extern u32 _gz_cct_apply(u32);
extern u32 _gz_ppt_apply(u32);
extern u32 _gz_lpc_apply(u32);
extern u32 _gz_lpf_apply(u32);
extern u32 _gz_lct_apply(u32);
typedef struct {
 u32 magic,bytes,valid,token,frame,id,phase,pin,source,used;
 u32 pid,actor,model,view_seal,bank_seal,plan_seal,run_seal,mask_before;
} BmhReceipt;
typedef char BmhSize[sizeof(BmhReceipt)==72?1:-1];
static BmhReceipt _gz_bmh_receipt;
static u32 _gz_bmh_seal,_gz_bmh_busy,_gz_bmh_token,_gz_bmh_mask,_gz_bmh_progress_seal;
/* A callback error retains the original owner's inflight ID. Certify that
 * the scheduler has actually returned before retirement may consume it.
 * This record grants no replay or native writer permission. */
typedef struct {u32 magic,bytes,valid,token,service,frame,error,receipt,progress,owner;} BmhAbort;
static BmhAbort _gz_bmh_abort;
static u32 _gz_bmh_abort_seal;
u32 _gz_bmh_report[64]={0x424d4831u,256};
/* Private diagnostic only. Request1..7 selects the next genuinely completed
 * service2..8. This can only fail an already-owned operation, never forge a
 * receipt or write actors/source. Ordinary gameplay leaves both counters0.
 * Strip this probe before any public delivery.
 */
volatile u32 _gz_bof_request,_gz_bof_ack;
u32 _gz_bof_report[16]={0x424f4631u,64};
static u32 bmh_progress_hash(void){u32 a[2];a[0]=_gz_bmh_token;a[1]=_gz_bmh_mask;return btr_hash(a,8);}
static u32 bmh_sealed(void){const BmhReceipt*s=&_gz_bmh_receipt;
 return s->magic==0x424d4831u&&s->bytes==sizeof(*s)&&s->valid==1&&s->token&&s->id>=2&&s->id<=8&&
  s->phase>=1&&s->phase<=2&&s->mask_before<=127&&_gz_bmh_seal==btr_hash(s,sizeof(*s))&&
  _gz_bmh_token==s->token&&_gz_bmh_mask<=127&&_gz_bmh_progress_seal==bmh_progress_hash();
}
u32 _gz_bmh_aborted_scope(u32 token){const BmhAbort*a=&_gz_bmh_abort;const BmhReceipt*s=&_gz_bmh_receipt;
 const volatile u32*o=_gz_roh_owner;
 return !_gz_bmh_busy&&a->magic==0x424d4131u&&a->bytes==sizeof(*a)&&a->valid==1&&a->token==token&&
  a->service>=2&&a->service<=8&&a->error&&_gz_bmh_abort_seal==btr_hash(a,sizeof(*a))&&
  bmh_sealed()&&s->phase==1&&s->token==token&&s->id==a->service&&s->frame==a->frame&&
  a->receipt==_gz_bmh_seal&&a->progress==_gz_bmh_progress_seal&&
  _gz_bmh_mask==s->mask_before&&_gz_bmh_mask==((1u<<(a->service-2))-1)&&
  o[0]==ROH_MAGIC&&o[1]==240&&o[2]==2&&o[3]==ROH_FAILED_DIRTY&&o[4]==1&&o[5]==token&&
  o[6]==1&&o[7]==a->error&&o[12]==a->service&&_gz_roh_serial==token&&
  a->owner==btr_hash((const void*)o,240)&&boh_sealed()&&
  _gz_boh_journal.phase==BOH_APPLIED&&!_gz_boh_journal.error&&_gz_boh_journal.token==token;
}
static void bmh_record_abort(u32 token,u32 service,u32 error){BmhAbort*a=&_gz_bmh_abort;
 if(!error||_gz_bmh_busy||a->valid||_gz_bmh_abort_seal||!bmh_sealed()||
  _gz_bmh_receipt.phase!=1||_gz_bmh_receipt.token!=token||_gz_bmh_receipt.id!=service||
  _gz_roh_owner[3]!=ROH_FAILED_DIRTY||_gz_roh_owner[5]!=token||_gz_roh_owner[7]!=error||
  _gz_roh_owner[12]!=service||service<2||service>8)return;
 a->valid=0;a->magic=0x424d4131u;a->bytes=sizeof(*a);a->token=token;a->service=service;
 a->frame=_gz_bmh_receipt.frame;a->error=error;a->receipt=_gz_bmh_seal;
 a->progress=_gz_bmh_progress_seal;a->owner=btr_hash((const void*)_gz_roh_owner,240);
 a->valid=1;_gz_bmh_abort_seal=btr_hash(a,sizeof(*a));
}
u32 _gz_bof_scope(u32 token,u32 service){const BmhReceipt*s=&_gz_bmh_receipt;
 return bmh_sealed()&&s->phase==2&&s->token==token&&s->id==service&&
  s->frame==_gz_pbac_frame&&service>=2&&service<=8&&
  (_gz_bmh_mask&(1u<<(service-2)))&&boh_bound(token)&&
  _gz_boh_journal.phase==BOH_APPLIED&&!_gz_boh_journal.error&&
  !_gz_roh_owner[12]&&!_gz_roh_owner[7];
}
u32 _gz_bof_after_service(u32 token,u32 service){u32 request=_gz_bof_request,error;
 if(!request||request==_gz_bof_ack)return 0;
 if(request!=_gz_bof_ack+1||request>7||service!=request+1||!_gz_bof_scope(token,service))return 0;
 error=600+service;
 _gz_bof_report[2]=request;_gz_bof_report[3]=token;_gz_bof_report[4]=service;
 _gz_bof_report[5]=_gz_pbac_frame;_gz_bof_report[6]=error;_gz_bof_report[7]=_gz_bmh_mask;
 _gz_bof_report[8]=_gz_boh_journal.phase;_gz_bof_report[9]=_gz_boh_run_seal;
 __asm__ volatile("sync":::"memory");_gz_bof_ack=request;return error;
}
/* Integer/private metadata authentication only; safe in common/lane guards.
 * No source/actor/layer/SDK read and no owner/service callback occurs here. */
u32 _gz_bmh_scope(u32 token,u32 frame){const BmhReceipt*s=&_gz_bmh_receipt;const volatile u32*o=_gz_roh_owner;
 return bmh_sealed()&&boh_sealed()&&s->phase==1&&s->token==token&&s->frame==frame&&frame==_gz_pbac_frame&&
  o[0]==ROH_MAGIC&&o[1]==240&&o[2]==2&&o[3]==ROH_MUTATING&&o[4]==1&&o[5]==token&&
  o[6]==1&&!o[7]&&o[11]&&o[12]==s->id&&o[10]==frame&&_gz_roh_serial==token&&
  _gz_boh_journal.phase==BOH_APPLIED&&!_gz_boh_journal.error&&s->pin==_gz_boh_plan.pin&&
  s->source==_gz_hbp_recipe.source&&s->used==_gz_hbp_recipe.used&&
  s->pid==_gz_boh_journal.pid&&s->actor==_gz_boh_journal.actor&&s->model==_gz_btr_journal.model&&
  s->view_seal==_gz_bmv_seal&&s->plan_seal==_gz_boh_plan_seal&&s->run_seal==_gz_boh_run_seal;
}
/* Private test only: fail after one completed participant while the owning
 * service is still active. Strip together with BOF before public delivery. */
volatile u32 _gz_bif_request,_gz_bif_ack;
u32 _gz_bif_report[16]={0x42494631u,64};
u32 _gz_bif_after_part(u32 token,u32 service,u32 point){u32 request=_gz_bif_request;
 static const u32 services[12]={2,2,2,3,5,6,6,7,8,3,3,3};
 if(!request||request==_gz_bif_ack||request!=_gz_bif_ack+1||request>12||point!=request||
  services[request-1]!=service||!_gz_bmh_scope(token,_gz_pbac_frame))return 0;
 _gz_bif_report[2]=request;_gz_bif_report[3]=token;_gz_bif_report[4]=service;
 _gz_bif_report[5]=_gz_pbac_frame;_gz_bif_report[6]=700+point;_gz_bif_report[7]=_gz_bmh_mask;
 _gz_bif_report[8]=_gz_boh_journal.phase;_gz_bif_report[9]=_gz_boh_run_seal;
 _gz_bif_report[10]=_gz_roh_owner[12];
 __asm__ volatile("sync":::"memory");_gz_bif_ack=request;return 700+point;
}
u32 _gz_bmh_prepare(u32 token){BmhReceipt*s=&_gz_bmh_receipt;u32 i,z;
 if(_gz_bmh_busy||_gz_roh_owner[12]!=9)return 28;
 if(_gz_bmh_token&&(_gz_bmh_token!=token||_gz_bmh_progress_seal!=bmh_progress_hash()))return 28;
 if(_gz_bmh_mask==127)return 0;
 if((s->valid&&(!bmh_sealed()||s->phase!=2))||(!s->valid&&(_gz_bmh_mask||_gz_bmh_token)))return 28;
 z=_gz_bmc_preflight(token);if(z)return z;
 for(i=0;i<7&&(_gz_bmh_mask&(1u<<i));i++);
 if(i==7)return 28;
 s->valid=0;s->magic=0x424d4831u;s->bytes=sizeof(*s);s->token=token;s->frame=_gz_pbac_frame;
 s->id=i+2;s->phase=1;s->pin=_gz_boh_plan.pin;s->source=_gz_hbp_recipe.source;s->used=_gz_hbp_recipe.used;
 s->pid=_gz_boh_journal.pid;s->actor=_gz_boh_journal.actor;s->model=_gz_btr_journal.model;
 s->view_seal=_gz_bmv_seal;s->bank_seal=bmc_b294_hash();s->plan_seal=_gz_boh_plan_seal;
 s->run_seal=_gz_boh_run_seal;s->mask_before=_gz_bmh_mask;
 _gz_bmh_token=token;_gz_bmh_progress_seal=bmh_progress_hash();
 s->valid=1;_gz_bmh_seal=btr_hash(s,sizeof(*s));return 0;
}

/* Returned mapped pointers expire at the end of this exact service/frame.
 * Every use repeats original source checks and native actor/resource checks.
 * No caller destination/pointer is accepted as a write/replay capability. */
u32 _gz_bmv_read_service(u32 token,u32 frame,u32 part){BmvViews*v=&_gz_bmv_views;
 if(part<1||part>5||!_gz_bmh_scope(token,frame)||_gz_roh_replay_common(token,frame,1)||
  _gz_boh_service_validate(token)||_gz_bmv_entered||v->valid!=1||v->magic!=0x424d5631u||
  v->bytes!=sizeof(*v)||v->version!=1||v->token!=token||!_gz_bmv_seal||_gz_bmv_seal!=bmv_hash()||
  v->source!=_gz_hbp_recipe.source||v->source_bytes!=_gz_hbp_recipe.used||v->pin!=_gz_boh_plan.pin||
  v->source_pid!=_gz_boh_plan.source_pid||v->live_pid!=_gz_boh_journal.pid||
  v->live_actor!=_gz_boh_journal.actor||v->live_model!=_gz_btr_journal.model||
  bmv_mapped_shapes()||_gz_vws_validate(&v->world,sizeof(VrWorld))||
  _gz_ibi_bank_preflight(&v->bombs,_gz_pba_native_actor_queue,&v->bombs.rows[0].context)||bmv_live_ids()||
  _gz_bmh_receipt.bank_seal!=bmc_b294_hash()||_gz_boh_service_validate(token)||
  _gz_roh_replay_common(token,frame,0))return 0;
 if(part==1)return (u32)v->ids;
 if(part==2)return (u32)v->graph;
 if(part==3)return (u32)&v->world;
 if(part==4)return (u32)&v->bombs;
 return (u32)v;
}

u32 _gz_bmh_run(u32 token){BmhReceipt*s=&_gz_bmh_receipt;u32 z=0,end=0,i,p[5],row,mask=0,service,frame;
 if(_gz_bmh_busy)return 5;
 if(_gz_bmh_mask==127)return 0;
 if(!bmh_sealed()||s->token!=token||s->phase!=1||s->frame!=_gz_pbac_frame||
  _gz_roh_owner[12]||!boh_bound(token))return 28;
 _gz_bmh_busy=1;service=s->id;frame=s->frame;row=14+(service-2)*7;
 _gz_bmh_report[2]=token;_gz_bmh_report[4]=frame;_gz_bmh_report[row]=service;_gz_bmh_report[row+1]=frame;
 z=_gz_roh_service_begin(token,service);_gz_bmh_report[7]=z;if(z)goto done;
 for(i=0;i<5;i++){
  p[i]=_gz_bmv_read_service(token,frame,i+1);
  if(!p[i]){z=28;break;}mask|=1u<<i;
 }
 _gz_bmh_report[row+2]=mask;
 if(!z){
  BmvViews*v=(BmvViews*)p[4];
  if(p[0]!=(u32)v->ids||p[1]!=(u32)v->graph||p[2]!=(u32)&v->world||p[3]!=(u32)&v->bombs)z=28;
  else{
   i=_gz_v104_actor_registry_validate_view(BAD,v->meta,v->ids);_gz_bmh_report[row+3]=i;
   if(i!=1||_gz_identity_record_safe(p[1]))z=27;
   _gz_bmh_report[11]=v->count;
  }
 }
 /* The new READ handoff must not turn a service2..8 context into the native
  * service9 constructor/replay writer capability. */
 if(!z){
  i=_gz_roh_lifetime_common(token,0);_gz_bmh_report[row+4]=i;
  _gz_bmh_report[row+5]=_gz_roh_replay_ready();
  _gz_bmh_report[12]=_gz_boh_replay_authority(token,s->actor,R8(_gz_boh_plan.link,0x690c),frame);
  if(i!=49||_gz_bmh_report[row+5]!=1||_gz_bmh_report[12])z=28;
 }
 if(!z&&service==2)z=_gz_lpc_apply(token);
 if(!z&&service==2)z=_gz_bif_after_part(token,service,1);
 if(!z&&service==2)z=_gz_lpf_apply(token);
 if(!z&&service==2)z=_gz_bif_after_part(token,service,2);
 if(!z&&service==2)z=_gz_lct_apply(token);
 if(!z&&service==2)z=_gz_bif_after_part(token,service,3);
 if(!z&&service==3)z=_gz_ern_service(token);
 if(!z&&service==3)z=_gz_eks_service(token);
 if(!z&&service==3)z=_gz_kbs_service(token);
 if(!z&&service==3)z=_gz_kbr_apply(token);
 if(!z&&service==3)z=_gz_ekr_apply(token);
 if(!z&&service==3)z=_gz_bif_after_part(token,service,4);
 if(!z&&service==5){z=_gz_cct_apply(token);_gz_bmh_report[13]=z;}
 if(!z&&service==5)z=_gz_bif_after_part(token,service,5);
 if(!z&&service==6)z=_gz_ckr_apply(token);
 if(!z&&service==6)z=_gz_bif_after_part(token,service,6);
 if(!z&&service==6)z=_gz_prk_apply(token);
 if(!z&&service==6)z=_gz_bif_after_part(token,service,7);
 if(!z&&service==7)z=_gz_rgr_apply(token);
 if(!z&&service==7)z=_gz_bif_after_part(token,service,8);
 if(!z&&service==8)z=_gz_ppt_apply(token);
 if(!z&&service==8)z=_gz_bif_after_part(token,service,9);
 _gz_bmh_report[8]=z;end=_gz_roh_service_end(token,service,z);_gz_bmh_report[9]=end;
 if(!z)z=end;
 if(!z){
  i=_gz_bmv_read_service(token,frame,1);_gz_bmh_report[row+6]=i;_gz_bmh_report[10]=i;
  if(i||_gz_roh_owner[12]||!boh_bound(token)||_gz_boh_validate(token)){z=28;goto done;}
  _gz_bmh_mask|=1u<<(service-2);_gz_bmh_progress_seal=bmh_progress_hash();
  s->phase=2;_gz_bmh_seal=btr_hash(s,sizeof(*s));
  _gz_bmh_report[5]=_gz_bmh_mask;_gz_bmh_report[6]++;
 }
 if(!z)z=_gz_bof_after_service(token,service);
done:
 _gz_bmh_report[3]=z;_gz_bmh_busy=0;bmh_record_abort(token,service,z);return z;
}
