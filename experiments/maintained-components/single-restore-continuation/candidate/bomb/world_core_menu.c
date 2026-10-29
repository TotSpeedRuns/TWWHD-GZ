extern u32 _gz_osa_intent_match(u32,u32,u32,u32,u32),_gz_osa_raw_match(u32,u32,u32,u32);
#include "core_menu_features.h"
/* Ordinary chooser -> the exact existing no-bomb transaction. The queued
 * intent pins no snapshot while waiting. Original allocation identity and
 * bytes are requalified before the native owner acquires its real pin. */
enum {WCM_OFFSET=13216,WCM_BYTES=128};
static u32*wcm(void){return (u32*)ncb_core(WCM_OFFSET,33600-(WCM_OFFSET));}
static u32 wcm_hash(const u32*q){u32 i,h=2166136261u;const u8*b=(const u8*)q;for(i=0;i<WCM_BYTES;i++)h=(h^((i>=8&&i<12)||i>=124?0:b[i]))*16777619u;return h;}
static void wcm_seal(u32*q){q[31]=wcm_hash(q);__asm__ volatile("sync":::"memory");q[2]=1;}
static u32 wcm_valid(const u32*q){return q&&q[0]==0x57434d32u&&q[1]==WCM_BYTES&&q[2]==1&&!q[6]&&q[31]==wcm_hash(q);}
/* Authenticate the queued menu intent only. Current frame, selected
 * source, identity/hash and commit ticket remain separate worker checks.
 * This intentionally permits the next native frame and lets a changed
 * selection/source produce a terminal refusal through the normal ACK. */
u32 _gz_wcm_requested(u32 request){const u32*q=wcm();
 return GZ_CORE_MENU_RESTORE_ENABLED&&q&&wcm_valid(q)&&q[3]==1&&
  request&&request==q[4]&&q[5]<10&&_gz_wct_ack!=BAD&&request==_gz_wct_ack+1;
}
u32 _gz_wcm_choose(void){u32*q=wcm(),slot=_gz_pbac_selected,pin,p,n,id,gen,hash,z=28;
 if(!GZ_CORE_MENU_RESTORE_ENABLED||!q||slot>9||_gz_roh_user_guard()||_gz_boh_has_owner()||_gz_wct_request!=_gz_wct_ack||_gz_wct_ack==BAD||
  (!wcc_empty()||(q[2]&&(!wcm_valid(q)||q[3]==1)))||_gz_osa_acquire(slot))return 28;
 pin=_gz_osa_pin_id();p=_gz_osa_source(pin);n=_gz_osa_bytes(pin);
 if(!p||p!=_gz_pbac_slots[slot]||!archive_ok(p)||R32(p,4)!=n||R32(p,12)<30||_gz_b294m_archive_validate(p,n,0))goto done;
 if(R32(p+R32(p,208),72)||bomb_count(_gz_pba_native_actor_queue)||gz_snapshot_activity_conflict(_gz_pbac_tools,0u)||_gz_pbac_pending||_gz_pbac_event||
  *(volatile u8*)&_gz_pbac_loading||_gz_pbac_world_resume_pending)goto done;
 id=_gz_osa_pin_identity(pin);gen=_gz_osa_pin_generation(pin);if(!id||!gen)goto done;hash=btr_hash((const void*)p,n);z=5;
done:_gz_osa_release_leaf();if(z!=5)return z;
 eks_zero(q,WCM_BYTES);q[0]=0x57434d32u;q[1]=WCM_BYTES;q[3]=1;q[4]=_gz_wct_ack+1;q[5]=slot;
 q[6]=0;q[7]=n;q[8]=id;q[9]=gen;q[10]=hash;q[11]=_gz_pbac_menu;q[13]=_gz_pbac_frame;wcm_seal(q);
 _gz_pbac_menu=1;_gz_wct_commit_ticket=q[4];__asm__ volatile("sync":::"memory");_gz_wct_request=q[4];return 5;}
u32 _gz_wcm_admission(u32 request,u32 frame,u32 slot){u32*q=wcm(),pin,p,n,z=42;
 if(!q||!q[2]||q[3]!=1)return 0;
 if(!wcm_valid(q)||q[4]!=request||q[5]!=slot||slot>9||frame!=_gz_pbac_frame||_gz_roh_user_guard()||
  !_gz_osa_intent_match(slot,q[8],q[9],q[7],q[10])||_gz_osa_acquire(slot))return 42;
 pin=_gz_osa_pin_id();p=_gz_osa_source(pin);n=_gz_osa_bytes(pin);
 if(_gz_osa_raw_match(pin,slot,p,n)&&n==q[7]&&p==_gz_pbac_slots[slot]&&_gz_osa_pin_identity(pin)==q[8]&&_gz_osa_pin_generation(pin)==q[9]&&
  btr_hash((const void*)p,n)==q[10])z=0;
 _gz_osa_release_leaf();return z;}
u32 _gz_wcm_admission_held(u32 request,u32 frame,u32 slot){u32*q=wcm(),pin,p,n,z=42;
 pin=_gz_swc_admission_pin(request,frame,slot);if(!pin)return 42;
 if(!q||!q[2]||q[3]!=1)return 0;
 if(!wcm_valid(q)||q[4]!=request||q[5]!=slot||slot>9||frame!=_gz_pbac_frame||_gz_roh_user_guard()||
  !_gz_osa_intent_match(slot,q[8],q[9],q[7],q[10]))return 42;
 p=_gz_osa_source(pin);n=_gz_osa_bytes(pin);
 if(_gz_osa_raw_match(pin,slot,p,n)&&n==q[7]&&p==_gz_pbac_slots[slot]&&_gz_osa_pin_identity(pin)==q[8]&&_gz_osa_pin_generation(pin)==q[9]&&
  btr_hash((const void*)p,n)==q[10])z=0;
 if(_gz_swc_admission_pin(request,frame,slot)!=pin)return 42;return z;}
extern u32 _gz_osa_intent_reserve(u32,u32,u32,u32,u32,u32,u32);
extern u32 _gz_osa_intent_reservation_match(u32,u32,u32,u32,u32,u32,u32);
extern u32 _gz_osa_intent_release(u32,u32);
/* The new logical-operation busy guard never waives an original guard. Only
 * its own real NBD metadata cleanup or exact fresh ordinary WCT can proceed.
 * There are no callbacks/native body reads inside the metadata cleanup. */
u32 _gz_wcc_tools_blocked(void){
 if(wcc_empty())return 0;
 if(!wcc_valid()||wcc.phase==WCC_RETAINED)return 1;
 if(wcc_entered==1&&_gz_nbd_context_owned(_gz_pbac_frame))return 0;
 if(wcc.phase==WCC_RETRY&&wct_entered==1&&wcc.retry_request&&
  _gz_wct_request==wcc.retry_request&&_gz_wcm_requested(wcc.retry_request)&&
  _gz_wct_native_owned())return 0;
 return 1;
}
/* Each check below is private metadata, or an existing normal-owner predicate.
 * Neither progress report words nor an old physical capability are accepted. */
static u32 wcm_tuple(const u32*q){return wcm_valid(q)&&q[3]==1&&q[5]==wcc.slot&&
 q[7]==wcc.used&&q[8]==wcc.identity&&q[9]==wcc.generation&&q[10]==wcc.hash;}
static u32 wcm_physical_idle(void){u32 i;
 for(i=3;i<7;i++)if(_gz_roh_owner[i])return 0;
 return !_gz_roh_owner[12]&&!_gz_osa_pin_id()&&!_gz_wct_heap_pending()&&!_gz_wct_root_pending()&&
  !_gz_wct_job_token.active&&!_gz_wct_job_token.gate&&
  !_gz_swc_pending()&&!_gz_wgp_unreleased()&&!_gz_boh_has_owner();
}
static u32 wcm_lrr_allowed(u32 request,u32 t,u32 f){u32*q=wcm();
 return wcc_empty()&&!wcc_entered&&wcm_valid(q)&&q[3]==1&&q[4]==request&&request&&
  request==wct_source.request&&wcc_blocker.request==request&&wcc_blocker.token==t&&wcc_blocker.frame==f&&
  wct_source.entered&&wct_source.frame==f&&wct.token==t&&wct.phase==WCT_ROLLED&&
  wct.returned==1&&wct.error==40&&!wct.fault_group;
}
static u32 wcm_lrr_bind(u32 request,u32 t,u32 f,u32 nonce,u32 slot,u32 id,u32 gen,u32 used,u32 hash){u32*q=wcm(),cookie;
 if(!nonce||!wcm_lrr_allowed(request,t,f)||q[5]!=slot||q[7]!=used||q[8]!=id||q[9]!=gen||q[10]!=hash||
  !_gz_wct_source_worker_context(request,f,slot))return 0;
 cookie=_gz_osa_intent_reserve(request,f,slot,id,gen,used,hash);if(!cookie)return 0;
 /* No fallible work follows reservation: exact logical tuple only, no pin. */
 bcl_zero(&wcc,sizeof(wcc));wcc.magic=WCC_MAGIC;wcc.bytes=sizeof(wcc);wcc.phase=WCC_BOUND;
 wcc.request=request;wcc.token=t;wcc.frame=f;wcc.slot=slot;wcc.identity=id;wcc.generation=gen;
 wcc.used=used;wcc.hash=hash;wcc.cookie=cookie;wcc.nonce=nonce;wcc.epoch=_gz_pbac_epoch;
 wcc.stage0=_gz_pbac_stage[0];wcc.stage1=_gz_pbac_stage[1];wcc.room=_gz_pbac_room;wcc.layer=_gz_pbac_layer;
 wcc_seal();return 1;
}
/* Observation-only unused WCT report tail, after its genuine attempt receipt.
 * No runtime admission or physical owner consumes any of these words. */
static void wcm_observe(u32 result){
 _gz_wct_report[104]=WCC_MAGIC;_gz_wct_report[105]=wcc.phase;
 _gz_wct_report[106]=wcc.request;_gz_wct_report[107]=wcc.frame;_gz_wct_report[108]=wcc.token;
 _gz_wct_report[109]=40;_gz_wct_report[110]=wcc.nonce;_gz_wct_report[111]=lrr.provider_serial;
 _gz_wct_report[112]=wcc.retry_request;_gz_wct_report[113]=0;_gz_wct_report[114]=wcc.cancel;
 _gz_wct_report[115]=wcc.retry_request?1u:0u;_gz_wct_report[116]=wcc.slot;
 _gz_wct_report[117]=wcc.identity;_gz_wct_report[118]=wcc.generation;_gz_wct_report[119]=wcc.used;
 _gz_wct_report[120]=wcc.hash;_gz_wct_report[121]=_gz_pbac_frame;_gz_wct_report[122]=wcc.error;
 _gz_wct_report[123]=result;
}
static void wcm_terminal(u32*q,u32 result){u32 z;
 if(!wcc_empty()){
  if(!wcc_valid()||!wcm_tuple(q)||!wcm_physical_idle())return;
  z=_gz_osa_intent_release(wcc.cookie,wcc.request);
  if(z){
   if((u32)(_gz_pbac_frame-wcc.frame)>WCC_DEADLINE){wcc.phase=WCC_RETAINED;wcc.error=z;wcc_seal();}
   wcm_observe(5);return;
  }
  wcm_observe(result);bcl_zero(&wcc,sizeof(wcc));
 }
 q[3]=2;q[12]=result;q[14]=_gz_pbac_frame;q[15]=_gz_wct_report[8];wcm_seal(q);
 _gz_pbac_menu=q[11];_gz_status=result;_gz_start_toast(result);
}
static void wcm_continue_tick(void){u32*q,cancel,ready,failed,next,f=_gz_pbac_frame;
 /* Normal no-continuation path is fixed private metadata only. */
 if(wcc_empty()||wcc_entered||!wcc_valid()||wcc.phase!=WCC_BOUND)return;
 wcc_entered=1;q=wcm();
 if(!wcm_tuple(q)||q[4]!=wcc.request||_gz_wct_ack!=wcc.request||_gz_wct_request!=_gz_wct_ack)goto done;
 if(!lrr_valid()||lrr.request!=wcc.request||lrr.nonce!=wcc.nonce||
  lrr.slot!=wcc.slot||lrr.identity!=wcc.identity||lrr.generation!=wcc.generation||
  lrr.used!=wcc.used||lrr.hash!=wcc.hash)goto done;
 cancel=wcc_cancelled(wcc.request,wcc.nonce);
 failed=lrr.phase==LRR_HALTED&&lrr.error&&_gz_lrw_completed(lrr.nonce,lrr.provider_serial);
 ready=lrr.phase==LRR_FINISHED&&!lrr.error&&
  ((cancel&&!lrr.scheduled&&_gz_lrw_quiescent())||
   _gz_lrw_completed(lrr.nonce,lrr.provider_serial));
 if(!ready&&!failed)goto done;
 if(!wcm_physical_idle())goto done;
 if(failed&&lrr.scheduled){
  /* Producer release is not actor/CO/heap retirement. A post-issue unknown
   * or timeout remains dirty/retained with its logical SOURCE reservation. */
  wcc.phase=WCC_RETAINED;wcc.error=lrr.error;wcc_seal();wcm_observe(5);goto done;
 }
 if(failed||cancel){wcm_terminal(q,28);goto done;}
 if(lrr.scheduled!=lrr_mask(lrr.count)||lrr.retired!=lrr_mask(lrr.count))goto done;
 if(!_gz_osa_intent_reservation_match(wcc.cookie,wcc.request,wcc.slot,wcc.identity,wcc.generation,wcc.used,wcc.hash)){
  /* A foreign SOURCE writer/entered callback may be transiently busy. The
   * fixed native-frame deadline cancels; no mismatched row is ever read. */
  goto done;
 }
 if(_gz_wct_ack==BAD||_gz_wct_late_target_fault||!_gz_nbd_context_owned(f)){
  wcc.cancel=WCC_CANCEL_SOURCE;wcc_seal();wcm_terminal(q,28);goto done;
 }
 next=_gz_wct_ack+1;if(!next){wcc.cancel=WCC_CANCEL_SOURCE;wcc_seal();wcm_terminal(q,28);goto done;}
 /* Keep q's original source tuple and previous menu byte-for-byte. The
  * ordinary worker below obtains every NEW native/pin/job/owner boundary. */
 wcc.retry_request=next;wcc.phase=WCC_RETRY;wcc_seal();
 q[4]=next;wcm_seal(q);_gz_pbac_menu=1;_gz_wct_commit_ticket=next;
 __asm__ volatile("sync":::"memory");_gz_wct_request=next;
 /* The parent's earlier enter saw request==ack. Enter the actual unchanged
  * producer NOW, from this same genuine NBD callback, after the fresh intent.
  * Its normal thread/interrupt/request checks remain mandatory. */
 _gz_wct_native_enter();
done:wcc_entered=0;
}
void _gz_wcm_tick(void){u32*q=wcm(),z;
 if(!q||!wcm_valid(q)||q[3]!=1||_gz_wct_ack!=q[4])return;
 if(!wcc_empty()){
  if(!wcc_valid()||wcc.phase!=WCC_RETRY||wcc.retry_request!=q[4])return;
 }
 if(!wcc_empty()){
  if(wcc_entered||!_gz_nbd_context_owned(_gz_pbac_frame))return;
  wcc_entered=1;
  if(_gz_roh_user_guard()||!wcm_physical_idle()){wcc_entered=0;return;}
  z=!_gz_wct_report[5]&&_gz_wct_report[96]==1?15:28;
  wcm_terminal(q,z);wcc_entered=0;return;
 }
 if(_gz_roh_user_guard()||!wcm_physical_idle())return;
 z=!_gz_wct_report[5]&&_gz_wct_report[96]==1?15:28;
 wcm_terminal(q,z);
}
