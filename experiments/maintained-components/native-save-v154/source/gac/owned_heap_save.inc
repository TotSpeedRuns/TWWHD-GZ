/* Genuine native Save/Restore coordinator. Snapshot capture and CPU
 * replay use separate compiled providers under the real current hold.
 * No GZS/WSP/ROH source or per-actor/profile/camera admission list. */
#include "owned_heap_save.h"
#include "owned_heap_held_view.h"
#include "held_heap_roundtrip.h"
/* Exact existing coreinit interrupt ABI: BOOL is signed32, no FP args. */
extern int OSDisableInterrupts(void),OSRestoreInterrupts(int);
extern const B _gz_ghs_save_request_return[],_gz_ghs_restore_request_return[];
extern U _gz_ois_heap_role(U,U),_gz_ois_cut_occupied(U),_gz_osa_occupied(U);
extern U _gz_gpn_current_compare(U,U);
extern U _gz_ois_cut_drop(U,U);
enum {GHS_COMPARE_INCOMPLETE=2891,GHS_COMPARE_REFUSED=2892,GHS_CPU_RELEASED=2898};
extern volatile U _gz_status,_gz_slots_selected;
extern void _gz_start_toast(void);
/* Existing native message table:14 Saved,15 Restored,16 Unsupported/kept.
 * Publication/retained CPU commit alone may select a success message. */
static void ghs_complete_toast(U result,U command){
 _gz_status=!result&&command==GHS_COMMAND_SAVE?14u:
   result==GHW_CPU_COMMITTED&&command==GHS_COMMAND_RESTORE?15u:16u;
 _gz_start_toast();
}
extern U _gz_wct_heap_pending(void),_gz_wct_root_pending(void),_gz_ipk_admission_outstanding(void);
extern U _gz_jol_coupled_halted(void),_gz_jol_coupled_outstanding(void);
extern U _gz_ghs_heap_propose(U,U),_gz_ghc_refresh(U,U),_gz_ghc_release(U,U);
extern U _gz_ois_heap_reserve(U,U,U),_gz_ois_heap_read(U,U),_gz_ois_heap_finish(U,U,U);
extern U _gz_ois_heap_stage_match(U,U,U,U);
extern U _gz_hg_capture_begin(U),_gz_hg_capture_seal(U),_gz_hg_capture_end(U);
typedef struct {U serial,phase,request,frame,epoch,slot,thread,original_return;
 U service_return,admission_return,hg,lease,job_generation,result,stage[2],room,layer,event,scene_hash,command,issued_hg,rewind;} GhsState;
typedef char GhsState92[sizeof(GhsState)==92?1:-1];
static GhsState ghs __attribute__((section(".bss.gz_ghs")));
volatile U _gz_ghs_report[32] __attribute__((section(".data.gz_ghs")))={0x47485331u,128,1};
static U ghs_scene_hash(const GhsState*s){U h=2166136261u,i;
 const U v[6]={s->epoch,s->stage[0],s->stage[1],s->room,s->layer,s->event};
 for(i=0;i<6;i++)h=(h^v[i])*16777619u;return h;}
static U ghs_scene(void){return ghs.epoch&&ghs.epoch==_gz_pbac_epoch&&
 ghs.stage[0]==_gz_roh_live_stage[0]&&ghs.stage[1]==_gz_roh_live_stage[1]&&
 ghs.room==_gz_roh_live_room&&ghs.layer==_gz_roh_live_layer&&
 !_gz_pbac_loading&&ghs.event==_gz_roh_live_event&&!_gz_pbac_world_resume_pending;}
static U ghs_foreign_idle(void){return !_gz_pbac_pending&&!(_gz_pbac_tools&~8u)&&
 !_gz_osa_pin_id()&&!_gz_boh_has_owner()&&_gz_wgp_idle()&&!_gz_wgp_unreleased()&&
 !_gz_wct_heap_pending()&&!_gz_wct_root_pending()&&!_gz_ipk_admission_outstanding()&&
 !_gz_jol_coupled_halted()&&!_gz_jol_coupled_outstanding()&&!gac_entered&&!gac_heap.phase;}
U _gz_ghs_busy(void){return ghs.phase?1u:0u;}
U _gz_ghs_claims(U cookie){return cookie&&ghs.phase&&cookie==ghs.request?1u:0u;}
U _gz_ghs_identity(U cookie){return _gz_ghs_claims(cookie)&&
 ghs.phase!=GHS_QUEUED&&ghs.phase!=GHS_HALTED&&callback()&&
 ghs.frame&&ghs.frame==_gz_pbac_frame&&ghs.thread&&ghs.thread==_gz_hg_platform_thread()&&
 ghs.original_return==(U)_gz_gac_native_frame_return&&
 ghs.service_return==(U)_gz_frame_done+8u&&
 ((ghs.command==GHS_COMMAND_SAVE&&ghs.admission_return==(U)_gz_ghs_save_request_return)||
  (ghs.command==GHS_COMMAND_RESTORE&&ghs.admission_return==(U)_gz_ghs_restore_request_return))&&
 ghs.slot<10&&ghs.slot==_gz_slots_selected&&ghs_scene();}
U _gz_ghs_role(U req,U f){return _gz_ghs_identity(req)&&f==ghs.frame?ghs.command:0;}
U _gz_ghs_target(U req,U f){return _gz_ghs_identity(req)&&f==ghs.frame?ghs.slot+1:0;}
U _gz_ghs_image_version(U req,U f){return _gz_ghs_identity(req)&&f==ghs.frame?GHC_PROCESS_VERSION:0;}
U _gz_ghs_picker_present(U slot){U hint;
 if(slot>=10)return 0;hint=_gz_ois_cut_occupied(slot);return hint?1u:_gz_osa_occupied(slot);}
U _gz_ghs_context_owned(U cookie){U previous,restored;
 if(!_gz_ghs_identity(cookie)||ghs.phase<GHS_ARMING||ghs.phase>GHS_ENDING||
  !_gz_ghs_owner_idle(cookie)||!ghs_foreign_idle()||_gz_ois_heap_role(cookie,ghs.frame)!=ghs.command)return 0;
 previous=(U)OSDisableInterrupts();restored=(U)OSRestoreInterrupts((int)previous);
 return previous&&!restored&&_gz_ghs_identity(cookie)&&_gz_ghs_owner_idle(cookie)&&ghs_foreign_idle()&&
  _gz_ois_heap_role(cookie,ghs.frame)==ghs.command;}
U _gz_ghs_scope(U req,U f,U phase){
 if(!_gz_ghs_identity(req)||f!=ghs.frame||!_gz_ghs_owner_idle(req)||!ghs_foreign_idle())return 0;
 if(phase==1)return ghs.phase==GHS_RESERVED&&!ghs.hg&&!_gz_jwa_ghs_pending();
 if(phase==2)return ghs.phase==GHS_HELD&&ghs.hg&&ghs.lease&&ghs.job_generation&&
  !_gz_hg_capture_seal(ghs.hg)&&_gz_jwa_ghs_workers_zero(req)!=2u&&
  _gz_jwa_ghs_check(req)==0&&_gz_jwa_ghs_generation(req)==ghs.job_generation&&
  _gz_ois_heap_role(req,f)==ghs.command;
 if(phase==3)return ghs.phase==GHS_RELEASED&&!ghs.hg&&!_gz_jwa_ghs_pending();
 return 0;}
U _gz_ghs_hg(U req,U f){return _gz_ghs_scope(req,f,2)?ghs.hg:0;}
#include "owned_heap_held_issuer.inc"
U _gz_ghs_apply_scope(U req,U f){return ghs.rewind==1u&&
 ghs.command==GHS_COMMAND_RESTORE&&_gz_ghs_scope(req,f,2);}
U _gz_ghs_source(U req,U f,GhcSaveProducer*out){
 if(!_gz_ghs_scope(req,f,2)||ghs.issued_hg!=ghs.hg||!_gz_ois_heap_stage_match(req,f,(U)out,sizeof(*out)))return GHC_CONTEXT;
 out->magic=GHS_MAGIC;out->bytes=64;out->version=2;out->request=req;out->frame=f;
 out->epoch=ghs.epoch;out->slot=ghs.slot;out->thread=ghs.thread;
 out->original_return=ghs.original_return;out->service_return=ghs.service_return;
 out->save_return=ghs.admission_return;out->hg=ghs.hg;out->lease=ghs.lease;
 out->job_generation=ghs.job_generation;out->scene_hash=ghs.scene_hash;out->reserved=ghs.command;
 return _gz_ghs_scope(req,f,2)?0:GHC_CHANGED;}
/* Fixed receiver is a real64B BSS object owned by the separate native
 * coordinator. No image/stack/arbitrary caller pointer can receive this.
 * Prior issuedHG is metadata; outstanding ghs.hg is already genuinely zero. */
U _gz_ghs_completed_source(U req,U f,GhcSaveProducer*out){U p=(U)out;
 if(out!=&_gz_gpn_completed_issuer||(p&3u)||p<0x10000000u||p>0xbffe0000u-sizeof(*out)||
  !_gz_ghs_private_apart(p,sizeof(*out))||!_gz_ghs_scope(req,f,3)||
  !ghs.issued_hg||!ghs.lease||!ghs.job_generation)return GHC_CONTEXT;
 out->magic=GHS_MAGIC;out->bytes=64;out->version=2;out->request=req;out->frame=f;
 out->epoch=ghs.epoch;out->slot=ghs.slot;out->thread=ghs.thread;
 out->original_return=ghs.original_return;out->service_return=ghs.service_return;
 out->save_return=ghs.admission_return;out->hg=ghs.issued_hg;out->lease=ghs.lease;
 out->job_generation=ghs.job_generation;out->scene_hash=ghs.scene_hash;out->reserved=ghs.command;
 return _gz_ghs_scope(req,f,3)?0:GHC_CHANGED;}
U _gz_ghs_process_scene(U req,U f,GhcSaveScene*out){
 if(!_gz_ghs_scope(req,f,2)||!_gz_ois_heap_stage_match(req,f,(U)out,sizeof(*out)))return GHC_CONTEXT;
 out->epoch=ghs.epoch;out->stage[0]=ghs.stage[0];out->stage[1]=ghs.stage[1];
 out->room=ghs.room;out->layer=ghs.layer;out->event=ghs.event;out->hash=ghs.scene_hash;out->reserved=0;
 return _gz_ghs_scope(req,f,2)?0:GHC_CHANGED;}
U _gz_ghs_private_apart(U p,U n){return ghc_policy_apart(p,n,(U)&ghs,sizeof(ghs))&&
 ghc_policy_apart(p,n,(U)_gz_ghs_report,sizeof(_gz_ghs_report));}
/* Each public wrapper captures its own ACTUAL native BL return before this
 * private helper. No caller role/target/archive field can supply authority. */
static __attribute__((noinline)) U ghs_admit(U command,U native_return,U actual_return){U f=_gz_pbac_frame,i;
 if(native_return!=(U)_gz_gac_native_frame_return||
  !((command==GHS_COMMAND_SAVE&&actual_return==(U)_gz_ghs_save_request_return)||
    (command==GHS_COMMAND_RESTORE&&actual_return==(U)_gz_ghs_restore_request_return)))return 0;
 if(ghs.phase)return 1;
 /* Absence alone preserves the original typed Restore fallback. A generic
  * presence/BUSY/corrupt hint is NOT a grant: actual reserve will validate it. */
 if(command==GHS_COMMAND_RESTORE&&_gz_slots_selected<10&&!_gz_ois_cut_occupied(_gz_slots_selected))return 0;
 _gz_status=28;
 if(ghs.hg||_gz_jwa_ghs_pending()){ghs.result=GHC_RETAINED;
  _gz_ghs_report[9]=GHC_RETAINED;_gz_ghs_report[22]=0;_gz_ghs_report[3]=GHS_HALTED;
  __sync_synchronize();ghs.phase=GHS_HALTED;return 1;}
 if(ghs.serial&&f==ghs.frame)return 1; /* Same consumed chooser edge. */
 if(ghs.serial==0xffffffffu){_gz_ghs_report[9]=GHS_REQUEST_EXHAUSTED;_gz_ghs_report[22]=0;return 1;}
 ++ghs.serial;ghs.request=ghs.serial;ghs.frame=f;ghs.epoch=_gz_pbac_epoch;
 ghs.slot=_gz_slots_selected;ghs.thread=_gz_hg_platform_thread();ghs.command=command;
 ghs.original_return=native_return;ghs.admission_return=actual_return;
 ghs.service_return=ghs.hg=ghs.lease=ghs.job_generation=ghs.result=ghs.issued_hg=ghs.rewind=0;
 ghs.stage[0]=_gz_roh_live_stage[0];ghs.stage[1]=_gz_roh_live_stage[1];
 ghs.room=_gz_roh_live_room;ghs.layer=_gz_roh_live_layer;ghs.event=_gz_roh_live_event;ghs.scene_hash=ghs_scene_hash(&ghs);
 for(i=3;i<32;i++)_gz_ghs_report[i]=0;
 _gz_ghs_report[3]=GHS_QUEUED;_gz_ghs_report[4]=ghs.request;_gz_ghs_report[5]=f;
 _gz_ghs_report[6]=ghs.epoch;_gz_ghs_report[7]=ghs.slot;_gz_ghs_report[8]=ghs.thread;
 _gz_ghs_report[21]=255;_gz_ghs_report[23]=ghs.event;_gz_ghs_report[24]=command;
 if(!f||!ghs.thread||ghs.slot>=10||!ghs_scene()||!ghs_foreign_idle()||_gz_roh_user_guard()){
  ghs.result=GHC_CONTEXT;_gz_ghs_report[9]=GHC_CONTEXT;_gz_ghs_report[3]=0;return 1;}
 __sync_synchronize();ghs.phase=GHS_QUEUED;return 1;}
U _gz_ghs_save_request(U native_return){return ghs_admit(GHS_COMMAND_SAVE,native_return,(U)__builtin_return_address(0));}
U _gz_ghs_restore_request(U native_return){return ghs_admit(GHS_COMMAND_RESTORE,native_return,(U)__builtin_return_address(0));}
static U ghs_release_actual(void){U z;
 if(_gz_jwa_ghs_pending()){z=_gz_jwa_ghs_leave();_gz_ghs_report[18]=z;
  if(z||_gz_jwa_ghs_pending())return GHC_RETAINED;}
 if(ghs.hg){z=_gz_hg_capture_end(ghs.hg);_gz_ghs_report[19]=z;
  if(z)return GHC_RETAINED;ghs.hg=0;}
 ghs.phase=GHS_RELEASED;return _gz_ghs_scope(ghs.request,ghs.frame,3)?0:GHC_RETAINED;}
void _gz_ghs_tick(void){U cap,z=GHC_CONTEXT,clean,release,raw,zero,committed=0;
 if(ghs.phase!=GHS_QUEUED)return;
 ghs.service_return=_gz_gac_callback_service_return;
 ghs.phase=GHS_RESERVED;
 if(!_gz_ghs_scope(ghs.request,ghs.frame,1))goto halt;
 cap=_gz_ghs_heap_propose(ghs.request,ghs.frame);
 if(!cap){z=GHC_OBSERVER;goto finish;}
 z=_gz_ois_heap_reserve(ghs.request,ghs.frame,cap);if(z)goto finish;
 raw=_gz_ois_heap_role(ghs.request,ghs.frame);_gz_ghs_report[25]=raw;
 if(raw!=ghs.command){z=GHC_CONTEXT;goto finish;}
 z=_gz_ghc_refresh(ghs.request,ghs.frame);if(z)goto finish;
 ghs.phase=GHS_ARMING;ghs.lease=ghs.request;
 ghs.hg=_gz_hg_capture_begin(ghs.request);ghs.issued_hg=ghs.hg;_gz_ghs_report[10]=ghs.hg;
 if(!ghs.hg){z=GHC_CONTEXT;goto finish;}
 raw=_gz_jwa_ghs_try(ghs.request);_gz_ghs_report[11]=raw;
 if(raw){z=raw==1u?GHC_CONTEXT:GHC_OBSERVER;goto finish;}
 zero=_gz_jwa_ghs_workers_zero(ghs.request);_gz_ghs_report[12]=zero;
 if(zero!=0){z=GHC_CONTEXT;goto finish;}
 ghs.job_generation=_gz_jwa_ghs_generation(ghs.request);if(!ghs.job_generation){z=GHC_CONTEXT;goto finish;}
 ghs.phase=GHS_HELD;
 if(!_gz_ghs_scope(ghs.request,ghs.frame,2)){z=GHC_CONTEXT;goto finish;}
 _gz_ghs_report[13]=ghs.job_generation;_gz_ghs_report[14]=ghs.lease;
 z=_gz_ois_heap_read(ghs.request,ghs.frame);
 if(!z&&ghs.command==GHS_COMMAND_RESTORE){
  ghs.rewind=1;__sync_synchronize();
  raw=_gz_ghw_roundtrip_held(ghs.request,ghs.frame);_gz_ghs_report[27]=raw;
  if(raw==GHW_RETAINED)goto halt;
  ghs.rewind=0;__sync_synchronize();
  if(raw==GHW_CPU_COMMITTED)committed=1;
  else if(raw!=GHW_ROUNDTRIP)z=raw;
 }
finish:
 if(ghs.hg||_gz_jwa_ghs_pending())ghs.phase=GHS_ENDING;
 clean=ghs_release_actual();if(clean)goto halt;
 release=_gz_ghc_release(ghs.request,ghs.frame);if(release==GHC_RETAINED)goto halt;
 raw=_gz_ois_heap_finish(ghs.request,ghs.frame,!z&&!release);
 if(raw==GHC_RETAINED)goto halt;
 if(!z)z=release?release:raw;
 if(!z&&ghs.command==GHS_COMMAND_RESTORE){
  if(committed){
   /* All native CPU targets and actual lifetime rows already verified.
    * The former GPN path only reparsed PRIVATE before/source archives,
    * allocated a classification workspace, then released this same pair.
    * A committed restore needs the real unleased publication retired, not
    * another diagnostic classification. Drop authenticates its actual
    * request/frame, releases the exact saved pin and retires only CURRENT.
    * It neither overwrites the user slot nor grants native memory access. */
   raw=_gz_ois_cut_drop(ghs.request,ghs.frame);
   _gz_ghs_report[26]=raw?raw:GHS_CPU_RELEASED;
   if(raw)goto halt;
   z=GHW_CPU_COMMITTED;
  }else{
   raw=_gz_gpn_current_compare(ghs.request,ghs.frame);_gz_ghs_report[26]=raw;
   if(raw!=GHS_COMPARE_INCOMPLETE&&raw!=GHS_COMPARE_REFUSED)goto halt;
   z=raw;
  }
 }
 ghs.result=z;_gz_ghs_report[9]=z;_gz_ghs_report[20]=release;
 _gz_ghs_report[22]=!z?1u:0u;_gz_ghs_report[3]=0;__sync_synchronize();ghs.phase=0;ghs_complete_toast(z,ghs.command);return;
halt:
 ghs.result=GHC_RETAINED;_gz_ghs_report[9]=GHC_RETAINED;
 _gz_ghs_report[3]=GHS_HALTED;__sync_synchronize();ghs.phase=GHS_HALTED;ghs_complete_toast(GHC_RETAINED,ghs.command);
}
