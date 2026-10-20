/* Actual owner-held constructor participant. Normal full-world restore stays
 * denied. The diagnostic stops with the world held after typed replay: only
 * the future complete transaction may publish/release that operation. */
/* Match lane.h's comparison ABI without redeclaring the actual owner's
 * established volatile-word view in this existing translation unit. */
enum {ROH_MAGIC=0x524f4831u,ROH_CLEAN=1,ROH_MUTATING=2,ROH_FAILED_DIRTY=4,RLL_CREATE=1,RLL_EITHER=0,RLL_PRESENT=1,RLL_ABSENT=2};
typedef struct {u32 token,kind,profile,source_pid,source_actor,live_pid,live_actor,actor_queue_node,key,key_bytes,observation_frame,expected_presence;} RllTarget;
extern volatile u32 _gz_rll_current_layer,_gz_rll_priority_head;
extern u32 _gz_rll_prepare(u32,u32,u32,u32,u32,u32,u32),_gz_rll_check_unissued(u32,u32,u32);
extern u32 _gz_rll_check_actor_delta(const RllTarget*),_gz_rll_poll_actor_delta(const RllTarget*);
extern u32 _gz_rll_recovery_actor_delta(const RllTarget*);
extern u32 _gz_roh_recovery_begin(u32,u32),_gz_roh_recovery_common(u32,u32),_gz_roh_recovery_baseline(u32,u32);
extern u32 _gz_btr_recovery_rollback(u32),_gz_boh_native_delete_tag_do(u32,u32);
extern u32 _gz_boh_native_create_do(u32);
extern u32 _gz_bmv_tick(u32);
extern u32 _gz_bmc_preflight(u32);
extern u32 _gz_bmh_scope(u32,u32),_gz_bmh_prepare(u32),_gz_bmh_run(u32);
extern u32 _gz_ern_service(u32);
extern u32 _gz_cct_prepare(u32,u32),_gz_cct_apply(u32),_gz_cct_recovery_rollback(u32);
extern u32 _gz_ppt_prepare(u32,u32),_gz_ppt_apply(u32),_gz_ppt_recovery_rollback(u32),_gz_ppt_recovery_globals(u32);
extern u32 _gz_lpc_prepare(u32,u32),_gz_lpc_apply(u32),_gz_lpc_recovery_rollback(u32);
extern u32 _gz_lpf_prepare(u32,u32),_gz_lpf_recovery_rollback(u32);
extern u32 _gz_lct_prepare(u32,u32),_gz_lct_recovery_rollback(u32);
extern u32 _gz_rll_replay_actor_delta(const RllTarget*);
extern u32 _gz_roh_mark_mutating(u32),_gz_roh_lifetime_common(u32,u32),_gz_roh_fail(u32,u32);
extern volatile u32 _gz_roh_serial,_gz_roh_live_frame;
u32 _gz_boh_ready_callback(void*,void*);
extern u32 _gz_ckr_prepare(u32,u32),_gz_ckr_apply(u32),_gz_ckr_recovery_rollback(u32);
extern u32 _gz_prk_prepare(u32,u32),_gz_prk_apply(u32),_gz_prk_recovery_rollback(u32);
extern u32 _gz_rgr_prepare(u32,u32),_gz_rgr_apply(u32),_gz_rgr_recovery_rollback(u32);
extern u32 _gz_eks_service(u32),_gz_eks_recovery_rollback(u32),_gz_eks_recovery_check(u32,u32);
extern u32 _gz_ekr_apply(u32),_gz_ekr_recovery_undo(u32),_gz_ekr_recovery_check(u32,u32),_gz_ekr_private_apart(u32,u32);
extern u32 _gz_kbs_service(u32),_gz_kbs_recovery_rollback(u32),_gz_kbs_recovery_check(u32,u32),_gz_kbs_private_apart(u32,u32);
extern u32 _gz_kbr_apply(u32),_gz_kbr_recovery_undo(u32),_gz_kbr_recovery_check(u32,u32),_gz_kbr_private_apart(u32,u32);
static u32 bcl_step(void);
enum {BOH_PREPARED=1,BOH_ISSUING=2,BOH_ACTIVE=3,BOH_READY=4,BOH_REPLAYING=5,BOH_APPLIED=6,BOH_FAILED=7};
typedef struct {u32 magic,bytes,valid,token,frame,link,link_pid,pin,hbp_seal,source_pid;PbaKey recipe;} BohPlan;
typedef struct {u32 phase,token,pid,actor,ready,armed,issue_frame,last_pump,pumps,callback_frame,error;} BohJournal;
static BohPlan _gz_boh_plan;
static BohJournal _gz_boh_journal;
static PbaTracker _gz_boh_tracker;
static PbaKey _gz_boh_issued_key;
static u32 _gz_boh_plan_seal,_gz_boh_run_seal,_gz_boh_tracker_seal,_gz_boh_key_seal,_gz_boh_pumping;
volatile u32 _gz_boh_request,_gz_boh_ack;
u32 _gz_boh_report[64]={0x424f4831u,256};
static void boh_seal_run(void){_gz_boh_run_seal=btr_hash(&_gz_boh_journal,sizeof(BohJournal));}
static u32 boh_sealed(void){return _gz_boh_plan.magic==0x424f5031u&&_gz_boh_plan.bytes==sizeof(BohPlan)&&_gz_boh_plan.valid==1&&
 _gz_boh_plan_seal==btr_hash(&_gz_boh_plan,sizeof(BohPlan))&&_gz_boh_run_seal==btr_hash(&_gz_boh_journal,sizeof(BohJournal))&&
 _gz_boh_journal.token==_gz_boh_plan.token&&_gz_boh_journal.phase>=BOH_PREPARED&&_gz_boh_journal.phase<=BOH_FAILED;}
u32 _gz_boh_has_owner(void){return _gz_boh_plan.valid!=0;}
/* Authentication only, before original common owner guards. Never follows a
 * source, actor, request, layer or resource allocation. */
u32 _gz_boh_scope(u32 token){
 if(!_gz_boh_plan.valid)return 0;
 if(!boh_sealed()||!token||_gz_boh_plan.token!=token||_gz_boh_journal.error)return 2;
 if(_gz_roh_owner[3]==ROH_CLEAN)return 0;
 return _gz_roh_owner[3]==ROH_MUTATING&&_gz_roh_owner[4]==1&&_gz_roh_owner[5]==token&&_gz_roh_owner[6]==1?1:2;
}
static u32 boh_bound(u32 token){const BohPlan*p=&_gz_boh_plan;const volatile u32*o=_gz_roh_owner;
 return boh_sealed()&&token&&token==p->token&&o[0]==ROH_MAGIC&&o[1]==240&&o[2]==2&&o[3]==ROH_MUTATING&&
  o[4]==1&&o[5]==token&&o[6]==1&&!o[7]&&o[11]&&(!o[12]||o[12]==9)&&_gz_roh_serial==token&&
  _gz_pbac_frame-o[9]<=o[8]&&o[8]&&o[8]<=120&&_gz_pbac_frame-o[10]<0x80000000u&&
  _gz_osa_pin_id()==p->pin&&!_gz_osa_validate(p->pin)&&p->link==_gz_pbac_player&&span(p->link,0x6b00)&&
  R32(p->link,4)==p->link_pid&&R16(p->link,8)==168&&R8(p->link,12)==2;
}
static u32 boh_service_bound(u32 token){const BohPlan*p=&_gz_boh_plan;const volatile u32*o=_gz_roh_owner;
 return _gz_bmh_scope(token,_gz_pbac_frame)&&boh_sealed()&&token==p->token&&
  _gz_boh_journal.phase==BOH_APPLIED&&!_gz_boh_journal.error&&o[3]==ROH_MUTATING&&o[4]==1&&
  o[5]==token&&o[6]==1&&!o[7]&&o[11]&&o[12]>=2&&o[12]<=8&&o[10]==_gz_pbac_frame&&
  _gz_roh_serial==token&&o[8]&&o[8]<=120&&_gz_pbac_frame-o[9]<=o[8]&&
  _gz_osa_pin_id()==p->pin&&!_gz_osa_validate(p->pin)&&p->link==_gz_pbac_player&&
  span(p->link,0x6b00)&&R32(p->link,4)==p->link_pid&&R16(p->link,8)==168&&R8(p->link,12)==2;
}
static u32 boh_tracker_valid(void){return _gz_boh_journal.armed&&header(&_gz_boh_tracker)&&
 _gz_boh_tracker_seal==btr_hash(&_gz_boh_tracker,sizeof(PbaTracker))&&_gz_boh_key_seal==btr_hash(&_gz_boh_issued_key,sizeof(PbaKey))&&
 btr_equal(&_gz_boh_issued_key,&_gz_boh_tracker.key,sizeof(PbaKey))&&_gz_boh_issued_key.pid==_gz_boh_journal.pid&&same_heap(&_gz_boh_tracker);}
static u32 boh_target_node(u32 pid,u32*actor){u32 q=_gz_pba_native_actor_queue,n=4096,node=0,a;
 *actor=0;while(q){if(!n--||!span(q,16))return BAD;a=R32(q,12);
  if(a){if(!span(a,12))return BAD;if(R32(a,4)==pid){if(node||R16(a,8)!=294)return BAD;node=q;*actor=a;}}
  q=R32(q,8);}return node;
}
/* Fresh queue walk first. Stored request/actor audits are comparison values.
 * Native Do may free its request; no caller retains a dereference across Do. */
static u32 boh_queues(u32*request,u32*actor,u32*node){u32 r,a=0,n;
 if(!boh_tracker_valid()||_gz_pba_native_delete_queue||_gz_rll_priority_head)return 60;
 r=find_request(_gz_pba_native_create_queue,_gz_boh_journal.pid);if(r==BAD)return 61;
 if(r){if(_gz_pba_native_create_queue!=r||R32(r,8)||!request_owned(&_gz_boh_tracker,r))return 61;
  a=R32(r,0x40);if(a&&(!span(a,0xb50)||R32(a,4)!=_gz_boh_journal.pid||R16(a,8)!=294))return 64;
 }else if(_gz_pba_native_create_queue)return 61;
 n=boh_target_node(_gz_boh_journal.pid,actor);if(n==BAD)return 61;
 if(a&&*actor&&a!=*actor)return 64;
 if(!*actor)*actor=a;
 *request=r;*node=n;
 if(_gz_boh_journal.ready){if(!*actor||*actor!=_gz_boh_journal.actor||!actor_owned(&_gz_boh_tracker,*actor)||R8(*actor,12)!=2)return 64;}
 return 0;
}
static u32 boh_delta(u32 r,u32 a,u32 node){RllTarget t;u32 i;
 for(i=0;i<sizeof(t)/4;i++)((u32*)&t)[i]=0;
 t.token=_gz_boh_plan.token;t.kind=RLL_CREATE;t.profile=294;t.source_pid=_gz_boh_plan.source_pid;
 t.live_pid=_gz_boh_journal.pid;t.live_actor=a;t.actor_queue_node=node;t.key=(u32)&_gz_boh_plan.recipe;
 t.key_bytes=sizeof(PbaKey);t.observation_frame=_gz_pbac_frame;t.expected_presence=r?RLL_EITHER:RLL_PRESENT;
 if(_gz_roh_owner[12]>=2&&_gz_roh_owner[12]<=8)return _gz_rll_replay_actor_delta(&t);
 return _gz_roh_owner[12]?_gz_rll_check_actor_delta(&t):_gz_rll_poll_actor_delta(&t);
}
__attribute__((noinline)) u32 _gz_boh_service_validate(u32 token){u32 r,a,node,z;
 if(!boh_service_bound(token))return 60;
 if(!_gz_hbp_service_read(token,_gz_pbac_frame)||_gz_hbp_seal!=_gz_boh_plan.hbp_seal)return 62;
 z=boh_queues(&r,&a,&node);if(z)return z;
 if(r||!node||a!=_gz_boh_journal.actor)return 64;
 z=boh_delta(r,a,node);if(z)return z;
 return _gz_btr_suspended_check(token);
}
u32 _gz_boh_validate(u32 token){u32 r,a,node,z,p;
 if(!_gz_boh_plan.valid)return 60;
 if(_gz_bmh_scope(token,_gz_pbac_frame))return _gz_boh_service_validate(token);
 if(!boh_bound(token)||_gz_boh_journal.error||_gz_boh_journal.phase==BOH_FAILED)return 60;
 p=_gz_hbp_read(token,_gz_pbac_frame);if(!p||_gz_hbp_seal!=_gz_boh_plan.hbp_seal)return 62;
 if(_gz_boh_journal.phase==BOH_PREPARED)return _gz_rll_check_unissued(token,(u32)&_gz_boh_plan.recipe,sizeof(PbaKey));
 if(_gz_boh_journal.phase==BOH_ISSUING||_gz_boh_pumping)return 5;
 z=boh_queues(&r,&a,&node);if(z)return z;z=boh_delta(r,a,node);if(z)return z;
 if(!r&&!_gz_boh_journal.ready)return 64;
 if(_gz_boh_journal.ready&&_gz_boh_journal.phase<=BOH_READY&&(!born_free(a)||R16(a,0xa7c)!=150||R8(_gz_boh_plan.link,0x690c)))return 69;
 if(_gz_boh_journal.phase==BOH_APPLIED)return _gz_btr_suspended_check(token);
 return 0;
}
u32 _gz_boh_freeze_authority(u32 token,u32 actor,u32 link,u32 frame){u32 r,a,node;
 return frame==_gz_pbac_frame&&(boh_bound(token)||boh_service_bound(token))&&link==_gz_boh_plan.link&&_gz_boh_journal.ready&&
  !boh_queues(&r,&a,&node)&&!r&&node&&a==actor&&actor==_gz_boh_journal.actor;
}
u32 _gz_boh_replay_authority(u32 token,u32 actor,u32 count,u32 frame){
 return _gz_roh_owner[12]==9&&_gz_roh_owner[10]==frame&&
  _gz_boh_freeze_authority(token,actor,_gz_boh_plan.link,frame)&&R8(_gz_boh_plan.link,0x690c)==count&&
  !_gz_roh_lifetime_common(token,0);
}
u32 _gz_boh_pause(void*actor){u32 a=(u32)actor,r,p,n;
 if(!_gz_boh_plan.valid||!span(a,12)||R16(a,8)!=294)return 0;
 if(!boh_sealed()||!_gz_boh_journal.armed)return 1;
 if(R32(a,4)!=_gz_boh_journal.pid)return 0;
 /* Holding a failed operation never grants more reads or native calls. */
 if(_gz_boh_journal.error||_gz_roh_owner[7])return 1;
 if(boh_queues(&r,&p,&n)||p!=a)return 1;
 _gz_boh_report[30]++;return 1;
}
u32 _gz_boh_ready_callback(void*actor,void*user){u32 a=(u32)actor,r,z=0;
 _gz_boh_report[19]++;
 if(user)z=1;else if(!boh_bound(_gz_boh_plan.token))z=2;else if(!_gz_boh_pumping)z=3;
 else if(!boh_tracker_valid())z=4;else if(_gz_boh_journal.ready)z=5;else if(_gz_boh_report[19]!=1)z=6;
 if(z){_gz_boh_report[18]=z;return 1;}
 r=find_request(_gz_pba_native_create_queue,_gz_boh_journal.pid);
 if(!r||r==BAD){_gz_boh_report[18]=7;return 1;}
 if(!request_owned(&_gz_boh_tracker,r)){_gz_boh_report[18]=8;return 1;}
 if(R32(r,0x40)!=a){_gz_boh_report[18]=9;return 1;}
 {PbaQueues q=current_queues();z=_gz_pba_constructor_ready(&_gz_boh_tracker,actor,&q);}
 _gz_boh_report[20]=z;if(z){_gz_boh_report[18]=10;return 1;}
 _gz_boh_tracker_seal=btr_hash(&_gz_boh_tracker,sizeof(PbaTracker));
 _gz_boh_journal.actor=a;_gz_boh_journal.ready=1;_gz_boh_journal.callback_frame=_gz_pbac_frame;boh_seal_run();
 _gz_boh_report[21]=a;_gz_boh_report[22]=R16(a,0xa7c);return 1;
}
static u32 boh_failure(u32 z){if(!_gz_boh_report[5])_gz_boh_report[5]=z;
 _gz_boh_report[4]=BOH_FAILED;_gz_boh_report[6]=_gz_pbac_frame;
 if(boh_sealed()){_gz_boh_journal.error=z;_gz_boh_journal.phase=BOH_FAILED;boh_seal_run();}
 if(_gz_roh_owner[4]&&_gz_roh_owner[5])_gz_roh_fail(_gz_roh_owner[5],z);
 __asm__ volatile("sync":::"memory");_gz_boh_ack=_gz_boh_request;return z;
}
static u32 boh_prepare(u32 token,u32 frame){BohPlan*p=&_gz_boh_plan;u32 i,z;
 z=_gz_hbp_prepare(token,frame);if(z)return z;
 if(!_gz_hbp_read(token,frame)||_gz_rll_current_layer<0x10000000u)return 62;
 for(i=0;i<sizeof(*p)/4;i++)((u32*)p)[i]=0;
 p->magic=0x424f5031u;p->bytes=sizeof(*p);p->token=token;p->frame=frame;p->link=_gz_pbac_player;p->link_pid=R32(p->link,4);
 p->pin=_gz_osa_pin_id();p->hbp_seal=_gz_hbp_seal;p->source_pid=_gz_hbp_recipe.saved.pid;btr_copy(&p->recipe,&_gz_hbp_recipe.recipe,sizeof(PbaKey));
 p->recipe.callback=(u32)_gz_boh_ready_callback;
 z=_gz_rll_prepare(token,RLL_CREATE,294,_gz_hbp_recipe.saved.pid,0,(u32)&p->recipe,sizeof(PbaKey));if(z)return z;
 _gz_boh_journal.phase=BOH_PREPARED;_gz_boh_journal.token=token;boh_seal_run();
 p->valid=1;_gz_boh_plan_seal=btr_hash(p,sizeof(*p));return 0;
}
static u32 boh_issue(void){BohJournal*j=&_gz_boh_journal;u32 z,pid,i,angles[2]={0},scale[3]={0x3f800000u,0x3f800000u,0x3f800000u};PbaQueues q;
 z=_gz_boh_validate(j->token);if(z)return z;z=_gz_roh_lifetime_common(j->token,0);if(z)return z;
 if(R8(_gz_boh_plan.link,0x690c)||bomb_count(_gz_pba_native_actor_queue))return 69;
 btr_copy(&_gz_boh_issued_key,&_gz_boh_plan.recipe,sizeof(PbaKey));for(i=0;i<6;i++)((u8*)angles)[i]=_gz_boh_issued_key.home[12+i];
 j->phase=BOH_ISSUING;j->issue_frame=_gz_pbac_frame;boh_seal_run();
 pid=_gz_pbac_native_create(294,0x80000001u,(u32)_gz_boh_issued_key.home,_gz_boh_issued_key.home[18],(u32)angles,(u32)scale,BAD,_gz_boh_issued_key.callback);
 if(pid==BAD||pid==_gz_hbp_recipe.saved.pid)return 64;
 j->pid=pid;_gz_boh_issued_key.pid=pid;_gz_boh_key_seal=btr_hash(&_gz_boh_issued_key,sizeof(PbaKey));
 boh_seal_run(); /* Record the issued PID before the first fallible arm. */
 _gz_pba_append_cleanup_enabled=1;q=current_queues();z=_gz_pba_arm(&_gz_boh_tracker,&_gz_boh_issued_key,&q);if(z)return z;
 _gz_boh_tracker_seal=btr_hash(&_gz_boh_tracker,sizeof(PbaTracker));j->armed=1;j->phase=BOH_ACTIVE;boh_seal_run();
 _gz_boh_report[15]=pid;return _gz_boh_validate(j->token);
}
static u32 boh_pump(void){BohJournal*j=&_gz_boh_journal;u32 r,a,node,z,result;
 z=_gz_boh_validate(j->token);if(z)return z;z=boh_queues(&r,&a,&node);if(z)return z;
 if(!r)return 0;
 if(j->last_pump==_gz_pbac_frame)return 5;
 z=_gz_roh_lifetime_common(j->token,0);if(z)return z;
 j->last_pump=_gz_pbac_frame;j->pumps++;boh_seal_run();_gz_boh_pumping=1;
 result=_gz_boh_native_create_do(r);
 _gz_boh_pumping=0;
 /* r may now be freed: only rediscovered current queues are examined. */
 _gz_boh_report[16]=j->pumps;_gz_boh_report[17]=result;
 z=_gz_boh_validate(j->token);if(z)return z;
 z=boh_queues(&r,&a,&node);if(z)return z;
 return r?5:0;
}
/* Source-free failure recovery for the ONE completed, replayed constructor.
 * The failed owner and original operation receipts remain immutable. There is
 * no source-body validation, owner error reset, generic constructor cancel,
 * full-world commit or release in this diagnostic. An independent private
 * ticket authorizes typed undo and the exact owned native delete lane only. */
enum {BOR_ARMED=1,BOR_UNDO=2,BOR_ROLLED=3,BOR_DELETE=4,BOR_RETIRING=5,BOR_DONE=6,BOR_FAILED=7};
typedef struct {
 u32 magic,bytes,valid,phase,token,request,start,limit,pid,actor,link,link_pid;
 u32 plan_seal,journal_seal,tracker_seal,key_seal,owner_error,last_pump,delete_calls,pumps,error;
} BorJournal;
static BorJournal _gz_boh_recovery;
static u32 _gz_boh_recovery_seal,_gz_boh_recovery_arming,_gz_boh_recovery_entered;
volatile u32 _gz_boh_recovery_request,_gz_boh_recovery_ack;
u32 _gz_boh_recovery_report[64]={0x424f5231u,256};
static void bor_seal(void){_gz_boh_recovery_seal=btr_hash(&_gz_boh_recovery,sizeof(BorJournal));}
static u32 bor_used(void){u32 i;
 if(_gz_boh_recovery_seal)return 1;
 for(i=0;i<sizeof(_gz_boh_recovery)/4;i++)if(((const u32*)&_gz_boh_recovery)[i])return 1;
 return 0;
}
static u32 bor_sealed(void){const BorJournal*j=&_gz_boh_recovery;
 return j->magic==0x424f5231u&&j->bytes==sizeof(*j)&&j->valid==1&&
  j->phase>=BOR_ARMED&&j->phase<=BOR_FAILED&&j->limit==120&&j->token&&j->request&&
  _gz_boh_recovery_seal==btr_hash(j,sizeof(*j));
}
/* No original source or borrowed HBP access. The recipe, tracker and receipts
 * were sealed before mutation, and continue to identify this issued process. */
static u32 bor_original(void){const BorJournal*j=&_gz_boh_recovery;
 return bor_sealed()&&j->request==_gz_boh_recovery_request&&boh_sealed()&&_gz_boh_journal.phase==BOH_APPLIED&&!_gz_boh_journal.error&&
  _gz_boh_journal.ready==1&&_gz_boh_journal.armed==1&&
  j->plan_seal==_gz_boh_plan_seal&&j->journal_seal==_gz_boh_run_seal&&
  j->tracker_seal==_gz_boh_tracker_seal&&j->key_seal==_gz_boh_key_seal&&boh_tracker_valid()&&
  j->token==_gz_boh_plan.token&&j->token==_gz_boh_journal.token&&
  j->pid==_gz_boh_journal.pid&&j->pid!=BAD&&j->pid!=_gz_boh_plan.source_pid&&
  j->actor==_gz_boh_journal.actor&&j->actor==_gz_boh_tracker.actor_audit&&
  j->link==_gz_boh_plan.link&&j->link_pid==_gz_boh_plan.link_pid&&
  _gz_roh_owner[0]==ROH_MAGIC&&_gz_roh_owner[1]==240&&_gz_roh_owner[2]==2&&
  _gz_roh_owner[3]==ROH_FAILED_DIRTY&&_gz_roh_owner[4]==1&&_gz_roh_owner[5]==j->token&&
  _gz_roh_owner[6]==1&&_gz_roh_owner[7]==j->owner_error&&j->owner_error&&_gz_roh_serial==j->token;
}
static u32 bor_empty(u32 address){return !R32(address,0)&&!R32(address,4)&&!R32(address,8);}
/* Read-only phase-specific certificate; it grants no owner or native API.
 * The native completed Delete request removes mLyTg from its original layer
 * before destruction. fpcLyTg_QueueTo clears its three node links, used byte
 * and mpLayer, preserves tag data, and sets both list IDs to FFFF. The separate
 * DeleteTag retains the original layer, and the actor-registration tag+C8
 * remains linked until native destruction. Do not interpret this native cut
 * as an allocation change, and do not accept an attached tag in RETIRING. */
u32 _gz_boh_retirement_layer_certificate(u32 a,u32 phase,u32 request_layer){
 if(!span(a,0x34)||!span(request_layer,4)||_gz_rll_current_layer!=request_layer)return 0;
 if(phase>=BOR_ARMED&&phase<=BOR_DELETE)
  return R8(a,12)==2&&R32(a,0x2c)==request_layer?1:0;
 if(phase!=BOR_RETIRING)return 0;
 return R8(a,12)==3&&!R32(a,0x18)&&!R32(a,0x1c)&&!R32(a,0x20)&&
  R32(a,0x24)==a&&!R8(a,0x28)&&!R32(a,0x2c)&&R16(a,0x30)==0xffff&&R16(a,0x32)==0xffff?1:0;
}
/* Current actor and tag addresses are obtained from current queue heads. The
 * private actor/tag values are comparisons only; no freed allocation is read. */
static u32 bor_queues(u32*actor,u32*node,u32*tag){const BorJournal*j=&_gz_boh_recovery;u32 a,n,t,d;
 *actor=*node=*tag=0;
 if(!bor_original()||!bor_empty((u32)&_gz_pba_native_create_queue)||_gz_rll_priority_head)return 81;
 n=boh_target_node(j->pid,&a);if(n==BAD)return 82;
 t=_gz_pba_native_delete_queue;
 if(t){
  if(!span(t,28)||R32((u32)&_gz_pba_native_delete_queue,4)!=t||
   R32((u32)&_gz_pba_native_delete_queue,8)!=1||R32(t,0)||
   R32(t,4)!=(u32)&_gz_pba_native_delete_queue||R32(t,8)||R8(t,16)!=1)return 83;
  d=R32(t,12);
  if(!a||d!=a||t!=a+0x4c||R32(t,20)!=_gz_boh_tracker.request_layer)return 83;
 }else if(!bor_empty((u32)&_gz_pba_native_delete_queue))return 83;
 if(a){
  if(a!=j->actor||n!=a+0xc8||!span(n,20)||R8(n,16)!=1||
   !actor_owned(&_gz_boh_tracker,a)||R32(a,0x10)!=_gz_boh_tracker.profile_audit||
   R32(a,0xa8)!=_gz_boh_tracker.methods_audit||R32(a,0x3bc)!=_gz_boh_tracker.model_audit||
   !_gz_boh_retirement_layer_certificate(a,j->phase,_gz_boh_tracker.request_layer)||
   R32(a,0xac))return 84;
  if(j->phase<=BOR_DELETE){if(t)return 84;}
  else if(j->phase==BOR_RETIRING){if(!t||j->delete_calls!=1)return 84;}
  else return 84;
 }else if(t||n||j->phase!=BOR_RETIRING||j->delete_calls!=1)return 85;
 *actor=a;*node=n;*tag=t;return 0;
}
static u32 bor_delta(u32 a,u32 node,u32 presence){RllTarget t;u32 i;
 for(i=0;i<sizeof(t)/4;i++)((u32*)&t)[i]=0;
 t.token=_gz_boh_recovery.token;t.kind=RLL_CREATE;t.profile=294;t.source_pid=_gz_boh_plan.source_pid;
 t.live_pid=_gz_boh_recovery.pid;t.live_actor=a;t.actor_queue_node=node;
 t.key=(u32)&_gz_boh_plan.recipe;t.key_bytes=sizeof(PbaKey);t.observation_frame=_gz_pbac_frame;t.expected_presence=presence;
 return _gz_rll_recovery_actor_delta(&t);
}
static u32 bor_live(u32*actor,u32*node,u32*tag){u32 z;
 if(!bor_original()||_gz_boh_recovery.error)return 80;
 z=_gz_roh_recovery_common(_gz_boh_recovery.token,_gz_pbac_frame);if(z)return z;
 z=bor_queues(actor,node,tag);if(z)return z;
 z=bor_delta(*actor,*node,*actor?RLL_PRESENT:RLL_ABSENT);if(z)return z;
 return _gz_roh_recovery_common(_gz_boh_recovery.token,_gz_pbac_frame);
}
/* The owner may open a recovery ticket only inside this private arming call.
 * Other callers, reentrancy, partial constructors and old ticket renewal deny. */
u32 _gz_boh_recovery_scope(u32 token){
 return _gz_boh_recovery_arming==1&&_gz_boh_recovery_entered==1&&bor_original()&&
  _gz_boh_recovery.phase==BOR_ARMED&&_gz_boh_recovery.token==token&&
  _gz_boh_recovery.start==_gz_pbac_frame?1:0;
}
/* Strict boolean closure used exclusively by typed rollback. It permits the
 * participant's sealed count1->0 undo; its own before/after guards bind exact
 * values. Native Delete is impossible until the successful undo is recorded. */
u32 _gz_boh_recovery_authority(u32 token,u32 actor,u32 link,u32 frame){u32 a,n,t;
 if(!_gz_boh_recovery_entered||!bor_sealed()||_gz_boh_recovery.phase!=BOR_UNDO||
  _gz_boh_recovery.token!=token||_gz_boh_recovery.actor!=actor||_gz_boh_recovery.link!=link||
  frame!=_gz_pbac_frame||bor_live(&a,&n,&t)||a!=actor||!n||t)return 0;
 return 1;
}
/* Shared constructor scratch predates the bomb replay preimage. Only the
 * independently authenticated, source-free retirement ticket may restore it,
 * after native deletion and exact original actor topology are both proven. */
u32 _gz_boh_recovery_globals_authority(u32 token,u32 frame){u32 a,n,t;
 if(!_gz_boh_recovery_entered||!bor_sealed()||_gz_boh_recovery.phase!=BOR_RETIRING||
  _gz_boh_recovery.token!=token||_gz_boh_recovery.delete_calls!=1||frame!=_gz_pbac_frame||
  bor_live(&a,&n,&t)||a||n||t||_gz_roh_recovery_baseline(token,frame))return 0;
 return 1;
}
static void bor_fail(u32 z){BorJournal*j=&_gz_boh_recovery;
 if(!_gz_boh_recovery_report[4])_gz_boh_recovery_report[4]=z;
 _gz_boh_recovery_report[3]=BOR_FAILED;_gz_boh_recovery_report[6]=_gz_pbac_frame;
 if(bor_sealed()){j->phase=BOR_FAILED;j->error=z;bor_seal();}
 /* Do not call original fail/reset APIs: their original error, inflight and
  * hold receipt are the very preimage certified by this recovery ticket. */
 __asm__ volatile("sync":::"memory");_gz_boh_recovery_ack=_gz_boh_recovery_request;
}
static u32 bor_begin(u32 request){BorJournal*j=&_gz_boh_recovery;u32 i,z;
 if(!boh_sealed()||_gz_boh_journal.phase!=BOH_APPLIED||_gz_boh_journal.error||
  !_gz_boh_journal.ready||!_gz_boh_journal.armed||!boh_tracker_valid()||
  _gz_roh_owner[3]!=ROH_FAILED_DIRTY||!_gz_roh_owner[7]||
  _gz_boh_request!=_gz_boh_ack||bor_used())return 80;
 for(i=0;i<sizeof(*j)/4;i++)((u32*)j)[i]=0;
 j->magic=0x424f5231u;j->bytes=sizeof(*j);j->valid=1;j->phase=BOR_ARMED;j->request=request;
 j->token=_gz_boh_plan.token;j->start=_gz_pbac_frame;j->limit=120;j->pid=_gz_boh_journal.pid;j->actor=_gz_boh_journal.actor;
 j->link=_gz_boh_plan.link;j->link_pid=_gz_boh_plan.link_pid;j->plan_seal=_gz_boh_plan_seal;j->journal_seal=_gz_boh_run_seal;
 j->tracker_seal=_gz_boh_tracker_seal;j->key_seal=_gz_boh_key_seal;j->owner_error=_gz_roh_owner[7];bor_seal();
 _gz_boh_recovery_report[5]=j->start;_gz_boh_recovery_report[7]=j->token;_gz_boh_recovery_report[8]=j->pid;
 _gz_boh_recovery_report[9]=j->actor;_gz_boh_recovery_report[10]=j->link;_gz_boh_recovery_report[11]=j->owner_error;
 _gz_boh_recovery_report[20]=_gz_boh_plan.pin;_gz_boh_recovery_report[21]=_gz_boh_journal.phase;
 _gz_boh_recovery_arming=1;z=_gz_roh_recovery_begin(j->token,120);_gz_boh_recovery_arming=0;
 _gz_boh_recovery_report[22]=z;return z;
}
static void bor_step_inner(void){BorJournal*j=&_gz_boh_recovery;u32 req=_gz_boh_recovery_request,z,a,n,t,result;
 if(!req||req==_gz_boh_recovery_ack)return;
 if(!j->valid){
  for(z=2;z<64;z++)_gz_boh_recovery_report[z]=0;
  _gz_boh_recovery_report[2]=req;
  if(req!=_gz_boh_recovery_ack+1){bor_fail(80);return;}
  z=bor_begin(req);if(z){bor_fail(z);return;}
 }
 if(!bor_sealed()||j->request!=req||j->phase==BOR_FAILED||j->phase==BOR_DONE){bor_fail(80);return;}
 z=bor_live(&a,&n,&t);_gz_boh_recovery_report[23]=z;if(z){bor_fail(z);return;}
 _gz_boh_recovery_report[6]=_gz_pbac_frame;
 if(j->phase==BOR_ARMED){
  if(!a||!n||t||R8(j->link,0x690c)!=1||R8(a,0xa70)!=1){bor_fail(86);return;}
  j->phase=BOR_UNDO;bor_seal();_gz_boh_recovery_report[18]=R8(j->link,0x690c);
  z=_gz_prk_recovery_rollback(j->token);_gz_boh_recovery_report[34]=z;if(z){bor_fail(z);return;}
  z=_gz_ckr_recovery_rollback(j->token);_gz_boh_recovery_report[33]=z;if(z){bor_fail(z);return;}
  z=_gz_lct_recovery_rollback(j->token);_gz_boh_recovery_report[32]=z;if(z){bor_fail(z);return;}
  z=_gz_ppt_recovery_rollback(j->token);_gz_boh_recovery_report[28]=z;if(z){bor_fail(z);return;}
  z=_gz_cct_recovery_rollback(j->token);_gz_boh_recovery_report[27]=z;if(z){bor_fail(z);return;}
  z=_gz_eks_recovery_rollback(j->token);_gz_boh_recovery_report[36]=z;if(z){bor_fail(z);return;}
  z=_gz_lpf_recovery_rollback(j->token);_gz_boh_recovery_report[31]=z;if(z){bor_fail(z);return;}
  z=_gz_lpc_recovery_rollback(j->token);_gz_boh_recovery_report[30]=z;if(z){bor_fail(z);return;}
  z=_gz_btr_recovery_rollback(j->token);_gz_boh_recovery_report[12]=z;if(z){bor_fail(z);return;}
  z=bor_live(&a,&n,&t);if(z||!a||t||R8(j->link,0x690c)||!born_free(a)){bor_fail(z?z:87);return;}
  _gz_boh_recovery_report[19]=R8(j->link,0x690c);j->phase=BOR_ROLLED;bor_seal();
 }else if(j->phase==BOR_ROLLED){
  if(!a||t||j->delete_calls||R8(j->link,0x690c)||!born_free(a)){bor_fail(87);return;}
  j->phase=BOR_DELETE;j->delete_calls=1;bor_seal();
  result=_gz_pbac_native_delete_actor_entry((void*)a);
  _gz_boh_recovery_report[13]=result;_gz_boh_recovery_report[14]=j->delete_calls;
  j->phase=BOR_RETIRING;bor_seal();
  /* Delete only schedules. Rediscover the current allocation and owned tag;
   * no cached actor/tag is followed across either native call. */
  z=bor_live(&a,&n,&t);if(z||result!=1||!a||!t){bor_fail(z?z:88);return;}
 }else if(j->phase==BOR_RETIRING){
  if(!a||!t||!n||j->delete_calls!=1||R8(j->link,0x690c)||!born_free(a)){bor_fail(89);return;}
  if(j->last_pump==_gz_pbac_frame)return;
  j->last_pump=_gz_pbac_frame;j->pumps++;bor_seal();
  result=_gz_boh_native_delete_tag_do(t,0x025ddf68u);
  /* t/a may now be freed. Only rediscovered queues may supply a new pointer.
   * Keep the numeric audit values in the sealed journal for comparisons. */
  _gz_boh_recovery_report[15]=j->pumps;_gz_boh_recovery_report[16]=result;
  z=bor_live(&a,&n,&t);if(z||result>1){bor_fail(z?z:90);return;}
  if(result==0){if(!a||!n||!t){bor_fail(91);return;}}
  else{
   if(a||n||t||R8(j->link,0x690c)){bor_fail(92);return;}
   z=_gz_roh_recovery_baseline(j->token,_gz_pbac_frame);_gz_boh_recovery_report[24]=z;
   if(z){bor_fail(z);return;}
   z=_gz_ppt_recovery_globals(j->token);_gz_boh_recovery_report[29]=z;if(z){bor_fail(z);return;}
   z=_gz_rgr_recovery_rollback(j->token);_gz_boh_recovery_report[35]=z;if(z){bor_fail(z);return;}
   j->phase=BOR_DONE;bor_seal();_gz_boh_recovery_report[3]=BOR_DONE;_gz_boh_recovery_report[26]=_gz_pbac_frame;
   /* Root remains FAILED_DIRTY/held, with its source pin and error intact.
    * A future full participant transaction must own final publication. */
   __asm__ volatile("sync":::"memory");_gz_boh_recovery_ack=req;
  }
 }else{bor_fail(80);return;}
 _gz_boh_recovery_report[3]=j->phase;
}
static void bor_step(void){
 if(_gz_boh_recovery_entered)return;
 _gz_boh_recovery_entered=1;bor_step_inner();_gz_boh_recovery_entered=0;
}
void _gz_boh_step(void){u32 req=_gz_boh_request,t,z,end,view_error=0,frame=_gz_pbac_frame,slot=_gz_pbac_selected,i;
 bor_step();
 if(bcl_step())return;
 if(_gz_boh_recovery_request!=_gz_boh_recovery_ack||_gz_boh_recovery.valid||_gz_boh_recovery_seal)return;
 if(!req||req==_gz_boh_ack)return;
 if(!_gz_boh_plan.valid){
  if(req!=_gz_boh_ack+1||_gz_roh_user_guard()||_gz_pbac_active||_gz_pbac_issued.valid||_gz_osa_pin_id())return;
  for(i=2;i<64;i++)_gz_boh_report[i]=0;
  _gz_boh_report[2]=req;_gz_boh_report[3]=frame;
  if(_gz_pba_native_create_queue||_gz_pba_native_delete_queue||_gz_rll_priority_head||bomb_count(_gz_pba_native_actor_queue))return;
  z=_gz_slots_install(slot);_gz_boh_report[8]=z;if(z){boh_failure(z);return;}
  z=_gz_roh_begin(slot,120);_gz_boh_report[9]=z;if(z){boh_failure(z);return;}
  t=_gz_roh_owner[5];_gz_boh_report[10]=t;
  z=_gz_roh_service_begin(t,1);_gz_boh_report[11]=z;if(z){boh_failure(z);return;}
  z=boh_prepare(t,frame);_gz_boh_report[12]=z;
  if(!z){z=_gz_cct_prepare(t,frame);_gz_boh_report[35]=z;}
  if(!z){z=_gz_ppt_prepare(t,frame);_gz_boh_report[36]=z;}
  if(!z){z=_gz_lpc_prepare(t,frame);_gz_boh_report[37]=z;}
  if(!z){z=_gz_lpf_prepare(t,frame);_gz_boh_report[38]=z;}
  if(!z){z=_gz_lct_prepare(t,frame);_gz_boh_report[39]=z;}
  if(!z){z=_gz_ckr_prepare(t,frame);_gz_boh_report[40]=z;}
  if(!z){z=_gz_prk_prepare(t,frame);_gz_boh_report[41]=z;}
  if(!z){z=_gz_rgr_prepare(t,frame);_gz_boh_report[42]=z;}
  end=_gz_roh_service_end(t,1,z);if(z||end){boh_failure(z?z:end);return;}
  z=_gz_roh_mark_mutating(t);_gz_boh_report[13]=z;if(z){boh_failure(z);return;}
  z=_gz_roh_service_begin(t,9);_gz_boh_report[14]=z;if(z){boh_failure(z);return;}
  z=boh_issue();end=_gz_roh_service_end(t,9,z);if(z||end){boh_failure(z?z:end);return;}
  _gz_boh_report[4]=BOH_ACTIVE;return;
 }
 t=_gz_boh_plan.token;
 if(_gz_roh_owner[7]||!boh_sealed()){boh_failure(_gz_roh_owner[7]?_gz_roh_owner[7]:51);return;}
 if(_gz_roh_owner[10]!=frame)return;
 z=_gz_roh_service_begin(t,9);if(z){boh_failure(z);return;}
 z=boh_pump();
 if(!z&&_gz_boh_journal.ready&&_gz_boh_journal.phase==BOH_ACTIVE){
  _gz_boh_journal.phase=BOH_READY;boh_seal_run();
  z=_gz_btr_prepare(t,_gz_boh_journal.actor,_gz_boh_plan.link,&_gz_hbp_recipe.saved,0);_gz_boh_report[23]=z;
  if(!z){_gz_boh_journal.phase=BOH_REPLAYING;boh_seal_run();z=_gz_btr_apply(t);_gz_boh_report[24]=z;}
  if(!z){_gz_boh_journal.phase=BOH_APPLIED;boh_seal_run();_gz_boh_report[25]=frame;}
 }
 if(!z&&_gz_boh_journal.phase==BOH_APPLIED){
  view_error=_gz_bmv_tick(t);
  if(!view_error)view_error=_gz_bmc_preflight(t);
  if(!view_error)view_error=_gz_bmh_prepare(t);
 }
 end=_gz_roh_service_end(t,9,z);if((z&&z!=5)||(end&&end!=5)){boh_failure(z&&z!=5?z:end);return;}
 if(!view_error&&_gz_boh_journal.phase==BOH_APPLIED)view_error=_gz_bmh_run(t);
 if(view_error){
  /* Camera service5 may have written. Preserve its preimage and the completed
   * constructor receipt so independent recovery can undo camera first, then
   * the bomb, before native retirement. Do not clear the original failure. */
  _gz_boh_report[34]=view_error;_gz_roh_fail(t,view_error);
  __asm__ volatile("sync":::"memory");_gz_boh_ack=req;return;
 }
 _gz_boh_report[4]=_gz_boh_journal.phase;_gz_boh_report[26]=frame;_gz_boh_report[27]++;
 if(_gz_boh_journal.phase==BOH_APPLIED&&frame-_gz_boh_report[25]>=30){
  _gz_boh_report[28]=_gz_boh_validate(t);_gz_boh_report[29]=_gz_osa_pin_id();
  if(_gz_boh_report[28]){boh_failure(_gz_boh_report[28]);return;}
  /* Deliberately no owner/source/quarantine release here. Full transaction
   * undo, remaining participants and final publication are not implemented. */
  __asm__ volatile("sync":::"memory");_gz_boh_ack=req;
 }
}
