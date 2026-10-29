#include "core_menu_features.h"
extern u32 _gz_wcm_requested(u32);
/* Private core rewind -> before-image undo. No ordinary snapshot commit,
 * actor creation/deletion, room loader or lifetime exceptions are available. */
enum {WCT_PREPARED=1,WCT_FORWARD=2,WCT_UNDO=3,WCT_ROLLED=4};
typedef struct {
 u32 magic,bytes,valid,token,frame,phase,source,used,pin,identity,generation,revision,source_hash;
 u32 link,camera,link_hash,camera_hash,face_hash[3],pot_hash[64],model_hash[64];
 u32 returned,error,services,fault_group,undo_stage;
} WctReceipt;
static WctReceipt wct;
static u32 wct_seal,wct_entered,wct_target_entered,wct_commit_arming;
volatile u32 _gz_wct_commit_ticket;
extern volatile u32 _gz_wct_request,_gz_wct_ack;
u32 _gz_wct_source_worker_context(u32 request,u32 frame,u32 slot){u32 parent;
 if(wct_source.entered!=1||wct_entered!=1||!request||request!=wct_source.request||
  frame!=wct_source.frame||slot!=wct_source.slot||frame!=_gz_pbac_frame||slot!=_gz_pbac_selected||
  slot>9||request!=_gz_wct_request||_gz_wct_ack==BAD||request!=_gz_wct_ack+1||
  !_gz_wcm_requested(request)||_gz_boh_has_owner())return 0;
 parent=_gz_wct_native_owned();return parent&&_gz_hg_platform_thread()?parent:0;
}
u32 _gz_wct_closure_hash;
volatile u32 _gz_wct_request,_gz_wct_ack;
/* Private selector is latched and consumed before preparation. */
volatile u32 _gz_wct_late_target_fault;
u32 _gz_wct_report[128]={0x57435431u,512,1};
u32 _gz_wct_source_digest_scope(void){u32 request=_gz_wct_request;
 return wct_entered==1&&request&&request!=_gz_wct_ack&&request==_gz_wct_ack+1&&_gz_wcm_requested(request);
}
extern u32 _gz_roh_core_context(u32,u32,u32,u32),_gz_roh_core_roundtrip_finish(u32,u32);
static void wct_seal_now(void){wct_seal=btr_hash(&wct,sizeof(wct));}
static u32 wct_plans(u32 t){
 return cct_sealed()&&ppt_sealed()&&lpc_sealed()&&lpf_sealed()&&lct_sealed()&&ckr_sealed()&&prk_sealed()&&rgr_sealed()&&fgp_sealed(t)&&pti_sealed(t)&&cjp_sealed(t)&&_gz_ccr_sealed(t)&&_gz_cgr_sealed(t)&&
  _gz_cct_journal.token==t&&_gz_ppt_plan.token==t&&_gz_lpc_plan.token==t&&_gz_lpf_plan.token==t&&
  _gz_lct_plan.token==t&&_gz_ckr_plan.token==t&&_gz_prk_plan.token==t&&_gz_rgr_plan.token==t;
}
static u32 wct_sealed(u32 t,u32 f){
 return t&&wct.magic==0x57435231u&&wct.bytes==sizeof(wct)&&wct.valid==1&&wct.token==t&&
  wct.frame==f&&f==_gz_pbac_frame&&wct.phase>=1&&wct.phase<=4&&
  wct_seal==btr_hash(&wct,sizeof(wct))&&wct_plans(t);
}
/* Metadata only: the real frame-owned callback establishes entry before
 * the independent native HG gate may be acquired. No actor/source body is
 * followed here, and group receipts cannot create an owner. */
u32 _gz_wct_group_context(u32 t,u32 f,u32 mode){const volatile u32*o=_gz_roh_owner;
 if(!wct_entered||!t||f!=_gz_pbac_frame||o[0]!=ROH_MAGIC||o[1]!=240||o[2]!=2||o[4]!=1||o[5]!=t||o[10]!=f||!o[11]||_gz_boh_has_owner())return 0;
 if(mode==1)return o[3]==ROH_CLEAN&&!o[6]&&!o[7]&&o[12]==1&&wsp_source.valid==1&&wsp_source.token==t&&
  wsp_source.frame==f&&wsp_source_seal==btr_hash(&wsp_source,sizeof(wsp_source))&&wsp_source.pin==_gz_osa_pin_id();
 if(!wct_sealed(t,f)||o[6]!=1)return 0;
 if(mode==2)return wct.phase==WCT_FORWARD&&!wct.returned&&!wct.error&&o[3]==ROH_MUTATING&&!o[7]&&o[12]==8;
 if(mode==3)return (wct.phase==WCT_UNDO||wct.phase==WCT_ROLLED)&&wct.returned==1&&
  ((!wct.error&&o[3]==ROH_MUTATING&&!o[7]&&!o[12])||(wct.error&&o[3]==ROH_FAILED_DIRTY&&o[7]==wct.error));
 if(mode==4)return wct_target_entered==1&&wct.phase==WCT_FORWARD&&!wct.returned&&!wct.error&&o[3]==ROH_MUTATING&&!o[7]&&
  (o[12]==WCT_FINAL_SERVICE||!o[12])&&o[13]==WCT_FINAL_CALLS;return 0;}
u32 _gz_wct_has_token(u32 t){return t&&wct.valid&&wct.token==t;}
u32 _gz_wct_forward_scope(u32 t,u32 f,u32 id){const volatile u32*o=_gz_roh_owner;
 if(!wct_sealed(t,f)||wct.phase!=WCT_FORWARD||wct.returned||wct.error||_gz_boh_has_owner()||
  !_gz_roh_core_context(t,f,id,0)||o[17]!=wct.source||o[18]!=wct.used||o[26]!=wct.revision||
  o[29]!=wct.link||o[37]!=wct.camera||_gz_osa_pin_id()!=wct.pin||_gz_osa_validate(wct.pin)||
  _gz_osa_source(wct.pin)!=wct.source||_gz_osa_bytes(wct.pin)!=wct.used||
  _gz_osa_pin_identity(wct.pin)!=wct.identity||_gz_osa_pin_generation(wct.pin)!=wct.generation||
  spd_changed(wct.pin,wct.source,wct.used,wct.identity,wct.generation,wct.revision,wct.source_hash,SVD_WCT_FORWARD_FNV,0))return 0;
 return _gz_cct_journal.source==wct.source&&_gz_cct_journal.source_used==wct.used&&_gz_cct_journal.pin==wct.pin&&
  _gz_ppt_plan.source==wct.source&&_gz_ppt_plan.used==wct.used&&_gz_ppt_plan.pin==wct.pin&&
  _gz_lpc_plan.source==wct.source&&_gz_lpc_plan.used==wct.used&&_gz_lpc_plan.pin==wct.pin&&
  _gz_lct_plan.source==wct.source&&_gz_lct_plan.used==wct.used&&_gz_lct_plan.pin==wct.pin&&
  _gz_ckr_plan.source==wct.source&&_gz_ckr_plan.used==wct.used&&_gz_ckr_plan.pin==wct.pin&&
  _gz_prk_plan.source==wct.source&&_gz_prk_plan.used==wct.used&&_gz_prk_plan.pin==wct.pin&&
  _gz_rgr_plan.source==wct.source&&_gz_rgr_plan.used==wct.used&&_gz_rgr_plan.pin==wct.pin;
}
/* No source-body API or saved allocation is touched from this entry. The
 * current owner, ordered actor topology and private typed plans remain live. */
u32 _gz_wct_recovery_scope(u32 t,u32 f){const volatile u32*o=_gz_roh_owner;
 if(!wct_sealed(t,f)||(wct.phase!=WCT_UNDO&&wct.phase!=WCT_ROLLED)||wct.returned!=1||
  _gz_boh_has_owner()||o[5]!=t||o[13]!=wct.services||
  !((!wct.error&&o[3]==ROH_MUTATING&&!o[7]&&!o[12])||
    (wct.error&&o[3]==ROH_FAILED_DIRTY&&o[7]==wct.error&&(o[12]==2||o[12]==5||o[12]==6||o[12]==8||(!o[12]&&wct.services==WCT_FINAL_CALLS)))))return 0;
 return _gz_roh_core_context(t,f,0,1);
}
static u32 wct_globals(u32 t,u32 f){const PptPlan*p=&_gz_ppt_plan;u32 i;
 if(!_gz_wct_recovery_scope(t,f)||_gz_ppt_run.phase!=PPT_ROLLED||
  !btr_equal(_gz_btr_shared_matrix,p->matrix_before,48)||
  !btr_equal((const void*)_gz_ppt_offset,p->offset_before,12)||_gz_ppt_offset_init!=p->offset_init_before)return 226;
 for(i=0;i<3;i++)if(_gz_ppt_native_spec[i]!=p->spec_audit[i])return 226;
 _gz_ppt_run.global_undo_done=1;ppt_seal_run();return 0;
}
static u32 wct_before(u32 t,u32 f){u32 z,i;
 if(!_gz_wct_recovery_scope(t,f)||_gz_lpc_run.phase!=LPC_ROLLED||_gz_lpf_run.phase!=5||
  _gz_cct_journal.phase!=CCT_ROLLED||_gz_lct_plan.phase!=LCT_ROLLED||_gz_ppt_run.phase!=PPT_ROLLED||
  !_gz_ppt_run.global_undo_done||_gz_ppt_run.applied_count!=_gz_ppt_run.undo_count)return 80;
 z=lpc_binding(0);if(!z)z=lpc_match(_gz_lpc_plan.before,_gz_lpc_plan.before_bind);if(!z)z=hms_journal_match(&_gz_lpc_plan);if(z)return z;
 z=_gz_ssp_before(t,f);if(z)return z;
 z=lpf_binding(0);if(!z)z=fr_current(&_gz_lpf_plan.leaf);if(!z)z=fr_before_matches(&_gz_lpf_plan.leaf,0);if(z)return z;
 z=cct_binding();if(!z)z=cct_before_links();if(!z)z=cct_match(_gz_cct_journal.before,_gz_cct_journal.before_links,_gz_cct_journal.before_force);if(z)return z;
 z=lct_bind(&_gz_lct_plan.before,0);if(!z)z=lct_match(&_gz_lct_plan.before);if(z)return z;
 if(_gz_wgp_has_token(t)){z=_gz_wgp_before(t,f);if(z)return z;}else for(i=0;i<_gz_ppt_plan.count;i++){const PptRow*p=_gz_ppt_plan.rows+i;
  z=ppt_binding(p,f);if(!z)z=ppt_match(p,p->before,1);if(z)return z;
  if(btr_hash((const void*)p->actor,2364)!=wct.pot_hash[i]||btr_hash((const void*)p->model,0xf8)!=wct.model_hash[i])return 226;
 }
 z=_gz_ckr_recovery_check(t,f);if(z)return z;z=_gz_prk_recovery_check(t,f);if(z)return z;z=_gz_rgr_recovery_check(t,f);if(z)return z;
 z=fgp_before(t);if(z)return z;z=pti_before(t);if(z)return z;z=cjp_before(t);if(z)return z;z=_gz_ccr_before(t,f);if(z)return z;
 /* Repeat contact before-image verification through its actual WGP owner.
  * The earlier group check returned after releasing its physical HG hold. */
 z=_gz_wgp_has_token(t)?_gz_wgp_before(t,f):_gz_cgr_before(t,f);if(z)return z;
 if(btr_hash((const void*)wct.link,0x8284)!=wct.link_hash||btr_hash((const void*)wct.camera,0xb28)!=wct.camera_hash)return 236;
 for(i=0;i<3;i++)if(btr_hash((const void*)_gz_lpf_plan.leaf.before_plans[i].buffer,8192)!=wct.face_hash[i])return 248;
 return 0;
}
u32 _gz_wct_finish_scope(u32 t,u32 f){
 return wct_sealed(t,f)&&wct.phase==WCT_ROLLED&&wct.returned==1&&
  ((!wct.error&&wct.services==7)||(wct.error&&wct.services>=2&&wct.services<=7))&&
  _gz_wct_closure_hash&&_gz_wct_closure_hash==wct_seal&&!wct_before(t,f);
}
void _gz_wct_finish_leaf(void){if(_gz_wct_root_pending()||_gz_ipk_admission_outstanding())return;_gz_wgp_finish_leaf(wct.token);_gz_cgr_finish(wct.token);_gz_ccr_finish(wct.token);cjp_finish(wct.token);pti_finish_leaf(wct.token);fgp_finish_leaf(wct.token);_gz_ssp_finish_leaf(wct.token);_gz_wct_heap_finish_leaf(wct.token);wdp_clear_private();bcl_zero(&wct,sizeof(wct));wct_seal=_gz_wct_closure_hash=0;}
#include "world_core_target.inc"
static u32 wct_group(u32 t,u32 f,u32 g){u32 id=g==1?5:g==3?8:g==5?WCT_FINAL_SERVICE:g==4?6:2,z;
 if(f!=_gz_pbac_frame)return 42;
 z=_gz_roh_service_begin(t,id);if(z)return z;
 if(g==0){z=_gz_lpc_apply(t);if(!z)z=_gz_lpf_apply(t);}
 else if(g==1)z=_gz_cct_apply(t);
 else if(g==2)z=_gz_lct_apply(t);
 else if(g==3)z=_gz_wgp_forward(t,f);
 else if(g==4){z=_gz_ckr_apply(t);if(!z)z=_gz_prk_apply(t);}
 else {z=_gz_rgr_apply(t);if(!z){z=_gz_wct_target_check(t,f);_gz_wct_report[90]=z;}}
 if(!z&&wct.fault_group==g+1)z=981+g;
 if(z){_gz_wct_report[29]=id;_gz_wct_report[30]=_gz_roh_fail(t,z);return z;}
 return _gz_roh_service_end(t,id,0);
}
u32 _gz_wct_menu_available(void){return GZ_CORE_MENU_RESTORE_ENABLED;}
void _gz_wct_step(void){u32 req=_gz_wct_request,t=0,z=5,open=0,dirty=0,i,late_fault=0,commit=0,f=_gz_pbac_frame,slot=_gz_pbac_selected;
 /* Only a sealed intent issued by the ordinary chooser reaches this
  * worker. In particular, a direct private request/ticket does nothing.
  * Selection/source changes are handled below as acknowledged refusals;
  * they must not leave a genuine chooser command waiting forever. */
 if(!GZ_CORE_MENU_RESTORE_ENABLED||!req||req==_gz_wct_ack||wct_entered||!_gz_wcm_requested(req))return;
 wct_entered=1;
 for(i=3;i<128;i++)_gz_wct_report[i]=0;_gz_wct_report[3]=req;_gz_wct_report[4]=f;
 /* Bind a private fault ticket to one exact command. Even a clean refusal
  * consumes it; a later command cannot inherit a stale actor-write intent. */
 if(req==_gz_wct_ack+1){late_fault=_gz_wct_late_target_fault;_gz_wct_late_target_fault=0;commit=_gz_wct_commit_ticket;_gz_wct_commit_ticket=0;}
 /* Public ordinary commits cannot arm the private fault/roundtrip
  * path. Refuse and acknowledge a mismatched ticket before any install,
  * owner acquisition, source body preparation or native write. */
 if(commit!=req||(late_fault&&(!GZ_CORE_MENU_FAULT_TEST_ENABLED||
  req>65535u||(late_fault&65535u)!=req||!(late_fault>>16)||(late_fault>>16)>7))){z=42;goto done;}
 _gz_wct_report[98]=late_fault>>16;
 _gz_wct_report[94]=commit==req;
 if(req!=_gz_wct_ack+1||_gz_roh_user_guard()||_gz_osa_pin_id()||_gz_boh_has_owner()||!wdp_empty()||
  wsp_source.valid||wsp_source_seal||wsp_retiring||wdp_retiring||wct.valid||wct_seal||!ssp_none())goto done;
 wct_source.request=req;wct_source.frame=f;wct_source.slot=slot;
 __asm__ volatile("sync":::"memory");wct_source.entered=1;
 z=_gz_swc_begin(req,f,slot);_gz_wct_report[100]=z;if(z)goto done;
 z=_gz_swc_admission(req,f,slot);_gz_wct_report[101]=z;if(z)goto done;
 z=_gz_swc_install(req,f,slot);_gz_wct_report[6]=z;if(z)goto done;
 z=_gz_swc_roh_begin(req,f,slot,60);_gz_wct_report[7]=z;if(z)goto done;t=_gz_roh_owner[5];_gz_wct_report[8]=t;
 z=_gz_roh_service_begin(t,1);_gz_wct_report[9]=z;if(z)goto abort;open=1;
 z=wsp_source_start(t,f);_gz_wct_report[10]=z;if(z)goto abort;
 _gz_wct_report[11]=wsp_source.source_hash;
 _gz_wct_report[12]=_gz_cct_prepare(t,f);_gz_wct_report[13]=0;
 _gz_wct_report[14]=_gz_lpc_prepare(t,f);
 _gz_wct_report[15]=_gz_wct_report[14]?BAD:_gz_lpf_prepare(t,f);
 _gz_wct_report[16]=_gz_lct_prepare(t,f);_gz_wct_report[17]=_gz_ckr_prepare(t,f);
 _gz_wct_report[18]=_gz_wct_report[17]?BAD:_gz_prk_prepare(t,f);_gz_wct_report[19]=_gz_rgr_prepare(t,f);
 z=0;for(i=12;i<20;i++)if(!z&&_gz_wct_report[i])z=_gz_wct_report[i];if(z)goto abort;
 z=fgp_prepare(t,f);_gz_wct_report[76]=z;if(z)goto abort;
 z=pti_prepare(t,f);_gz_wct_report[80]=z;if(z)goto abort;
 z=cjp_prepare(t,f);_gz_wct_report[97]=z;if(z)goto abort;
 /* A skeletal subparticipant is not a complete Chu restore. Refuse the
  * private forward-commit request while still CLEAN, before any native write. */
 if(commit==req&&!cjp_complete(t)){z=1209;goto abort;}
 z=_gz_ccr_prepare(t,f);_gz_wct_report[85]=z;if(z)goto abort;
 z=_gz_wgp_prepare(t,f);_gz_wct_report[87]=z;if(z)goto abort;
 z=wdp_before_images(t,f);if(z)goto abort;
 /* Conditional real SSP need adds no hold to unrelated restores. */
 if(_gz_wct_heap_pending()){if(!_gz_wct_heap_held(t,f)){z=WHL_INTEGRITY;goto abort;}}
 else if(_gz_ssp_heap_needed(t,f)){z=_gz_wct_heap_begin(t,f);if(z)goto abort;}
 /* Only the sealed same-allocation quiet stow below can reconcile a
  * source-holstered/current-drawn mismatch. Other transitions still refuse. */
 z=_gz_ssp_prepare(t,f);if(z)goto abort;
 if((R32(_gz_lpc_plan.saved,108)!=R16(_gz_lpc_plan.actor,0x69b0)||
  R32(_gz_lpc_plan.saved,36)!=R32(_gz_lpc_plan.actor,0x4440))&&!_gz_ssp_prepared(t,f)){z=28;goto abort;}
 wct.magic=0x57435231u;wct.bytes=sizeof(wct);wct.valid=1;wct.token=t;wct.frame=f;wct.phase=WCT_PREPARED;
 wct.source=wsp_source.source;wct.used=wsp_source.used;wct.pin=wsp_source.pin;wct.identity=wsp_source.identity;
 wct.generation=wsp_source.generation;wct.revision=wsp_source.revision;wct.source_hash=wsp_source.source_hash;
 wct.link=_gz_lpc_plan.actor;wct.camera=_gz_cct_journal.camera;
 if(!wrt_link_camera_hashes(wct.link,wct.camera)){z=44;goto abort;}
 wct.link_hash=btr_hash((const void*)wct.link,0x8284);wct.camera_hash=btr_hash((const void*)wct.camera,0xb28);
 for(i=0;i<3;i++)wct.face_hash[i]=btr_hash((const void*)_gz_lpf_plan.leaf.before_plans[i].buffer,8192);
 for(i=0;i<_gz_ppt_plan.count;i++){wct.pot_hash[i]=btr_hash((const void*)_gz_ppt_plan.rows[i].actor,2364);wct.model_hash[i]=btr_hash((const void*)_gz_ppt_plan.rows[i].model,0xf8);}
 for(i=0;i<3;i++){_gz_wct_report[56+i]=R32(wct.link,0x314+4*i);if(!lpc_value(_gz_lpc_plan.saved,0x314+4*i,&_gz_wct_report[60+i])){z=231;goto abort;}}
 wct.fault_group=late_fault?late_fault>>16:0;wct_seal_now();
 _gz_wct_report[20]=_gz_roh_service_end(t,1,0);open=0;if(_gz_wct_report[20]){z=_gz_wct_report[20];goto abort;}
 z=_gz_roh_mark_mutating(t);_gz_wct_report[21]=z;if(z)goto abort;dirty=1;
 wct.phase=WCT_FORWARD;wct_seal_now();
 for(i=0;i<6;i++){z=wct_group(t,f,i);_gz_wct_report[22+i]=z;if(z)break;_gz_wct_report[28]++;}
 for(i=0;i<3;i++)_gz_wct_report[64+i]=R32(wct.link,0x314+4*i);
 /* Private fault after the final service has returned. Every typed target
  * has already passed; this exercises source-free terminal recovery. */
 if(!z&&wct.fault_group==7)z=1210;
 if(!z&&commit==req){z=wct_commit_finish(t,f);_gz_wct_report[95]=z;
  if(!z){_gz_wct_report[96]=1;goto done;}}
 if(z&&!_gz_roh_owner[7])_gz_roh_fail(t,z);
 wct.error=_gz_roh_owner[7]?_gz_roh_owner[7]:z;wct.services=_gz_roh_owner[13];wct.returned=1;wct.phase=WCT_UNDO;wct_seal_now();
 _gz_wct_report[31]=_gz_lpc_recovery_rollback(t);if(_gz_wct_report[31])goto retained;
 _gz_wct_report[32]=_gz_lpf_recovery_rollback(t);if(_gz_wct_report[32])goto retained;
 _gz_wct_report[33]=_gz_cct_recovery_rollback(t);if(_gz_wct_report[33])goto retained;
 _gz_wct_report[34]=_gz_lct_recovery_rollback(t);if(_gz_wct_report[34])goto retained;
 _gz_wct_report[88]=_gz_wgp_recover(t,f);if(_gz_wct_report[88])goto retained;
 _gz_wct_report[39]=wct_globals(t,f);if(_gz_wct_report[39])goto retained;
 _gz_wct_report[36]=_gz_prk_recovery_rollback(t);if(_gz_wct_report[36])goto retained;
 _gz_wct_report[37]=_gz_ckr_recovery_rollback(t);if(_gz_wct_report[37])goto retained;
 _gz_wct_report[38]=_gz_rgr_recovery_rollback(t);if(_gz_wct_report[38])goto retained;
 _gz_wct_report[40]=wct_before(t,f);if(_gz_wct_report[40])goto retained;
 for(i=0;i<3;i++)_gz_wct_report[68+i]=R32(wct.link,0x314+4*i);
 _gz_wct_report[72]=wct.link_hash;_gz_wct_report[73]=btr_hash((const void*)wct.link,0x8284);
 _gz_wct_report[74]=wct.camera_hash;_gz_wct_report[75]=btr_hash((const void*)wct.camera,0xb28);
 wct.phase=WCT_ROLLED;wct_seal_now();_gz_wct_closure_hash=wct_seal;
 _gz_wct_report[41]=_gz_roh_core_roundtrip_finish(t,f);if(_gz_wct_report[41])goto retained;
 _gz_wct_report[42]=1;goto done;
abort:
 if(dirty||_gz_wgp_unreleased())goto retained;
 if(open){_gz_wct_report[43]=_gz_roh_service_end(t,1,0);open=0;}
 if(t){i=_gz_ssp_forget_clean(t);if(i){z=i;goto retained;}i=_gz_wct_heap_end(t,f);if(i){z=i;goto retained;}_gz_wct_report[44]=_gz_roh_abort_clean(t);if(!_gz_wct_report[44]){
  if(wsp_source.token==t){wdp_retiring=t;_gz_wct_report[45]=wdp_forget(t);}
  else _gz_wct_report[45]=!wdp_empty()||wsp_source.valid||wsp_source_seal?42:0;
  if(!_gz_wct_report[45]){_gz_wct_heap_finish_leaf(t);bcl_zero(&wct,sizeof(wct));wct_seal=0;}
 }}goto done;
retained:_gz_wct_report[46]=1;
done:
 if(_gz_swc_pending()){i=_gz_swc_cancel(req,f,slot);_gz_wct_report[102]=i;
  if(i){_gz_wct_report[46]=1;if(!z)z=i;}}
 _gz_wct_report[103]=_gz_swc_pending();
 wct_source.entered=0;wct_source.request=wct_source.frame=wct_source.slot=0;
 _gz_wct_report[5]=z;_gz_wct_report[47]=_gz_osa_pin_id();_gz_wct_report[48]=_gz_roh_user_guard();
 _gz_wct_report[49]=_gz_roh_owner[3];_gz_wct_report[50]=_gz_roh_owner[4];_gz_wct_report[51]=_gz_roh_owner[6];
 _gz_wct_report[52]=wdp_empty();_gz_wct_report[53]=wct.valid;_gz_wct_report[54]=wsp_source.valid;_gz_wct_report[55]=_gz_pbac_frame;
 __asm__ volatile("sync":::"memory");_gz_wct_ack=req;wct_entered=0;
}
