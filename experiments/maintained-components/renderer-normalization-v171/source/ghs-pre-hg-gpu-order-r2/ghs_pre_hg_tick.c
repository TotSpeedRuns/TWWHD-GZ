/* CPU-order research component only. No renderer reset, hook, RPX or
 * universal GPU producer certificate. The original92B owner/report remain
 * external; public native admission/check/cleanup helpers remain unchanged. */
#include <stddef.h>
#include <coreinit/core.h>
#include <gx2/displaylist.h>
#include <gx2/event.h>
typedef unsigned int U;
enum {GHS_QUEUED=7,GHS_RESERVED=1,GHS_ARMING=2,GHS_HELD=3,GHS_ENDING=4,GHS_RELEASED=5,GHS_HALTED=6,
 GHS_COMMAND_SAVE=1,GHS_COMMAND_RESTORE=2,GHC_CONTEXT=2881,GHC_OBSERVER=2882,GHC_RETAINED=2886,
 GHW_RETAINED=2886,GHW_ROUNDTRIP=2895,GHW_CPU_COMMITTED=2897,
 GHS_COMPARE_INCOMPLETE=2891,GHS_COMPARE_REFUSED=2892,GHS_CPU_RELEASED=2898};
extern U _gz_gfxq_owned(U,U);
extern U GX2GetMainCoreId(void); /* Genuine named gx2 export; no dynamic load. */
typedef struct {U serial,phase,request,frame,epoch,slot,thread,original_return;
 U service_return,admission_return,hg,lease,job_generation,result,stage[2],room,layer,event,scene_hash,command,issued_hg,rewind;} GhsState;

_Static_assert(sizeof(GhsState)==92,"existing native owner92");
_Static_assert(offsetof(GhsState,phase)==4 && offsetof(GhsState,hg)==40 && offsetof(GhsState,lease)==44 &&
 offsetof(GhsState,job_generation)==48 && offsetof(GhsState,command)==80 && offsetof(GhsState,rewind)==88,"native fields");
_Static_assert(sizeof(void*)==4 && sizeof(BOOL)==4,"Cafe PPC32 ABI");
extern volatile GhsState _gz_pre_hg_state;
#define ghs _gz_pre_hg_state
extern volatile U _gz_ghs_report[32],_gz_status,_gz_gac_callback_service_return;
extern void _gz_pre_hg_completion(void);
extern U _gz_ghs_scope(U,U,U),_gz_ghs_context_owned(U);
extern U _gz_ghs_heap_propose(U,U),_gz_ghc_refresh(U,U),_gz_ghc_release(U,U);
extern U _gz_ois_heap_reserve(U,U,U),_gz_ois_heap_role(U,U),_gz_ois_heap_read(U,U),_gz_rns_mm_finish(U,U,U);
extern U _gz_ois_cut_drop(U,U),_gz_gpn_current_compare(U,U),_gz_ghw_roundtrip_held(U,U);
extern U _gz_jwa_ghs_pending(void),_gz_jwa_ghs_try(U),_gz_jwa_ghs_check(U),_gz_jwa_ghs_workers_zero(U),_gz_jwa_ghs_generation(U),_gz_jwa_ghs_leave(void);
extern U _gz_hg_capture_begin(U),_gz_hg_capture_end(U);

static U ghs_gpu_owner_open(U request,U frame,U job)
{
 return request && frame && job && ghs.phase==GHS_ARMING && ghs.request==request &&
  ghs.frame==frame && !ghs.hg && !ghs.issued_hg && ghs.lease==request && ghs.job_generation==job &&
  _gz_ghs_context_owned(request) && !_gz_jwa_ghs_check(request) &&
  _gz_jwa_ghs_generation(request)==job && _gz_jwa_ghs_workers_zero(request)<=1u && _gz_gfxq_owned(request,frame);
}

static U ghs_gpu_retire_open(U request,U frame,U job)
{
 U main_core;
 if(!ghs_gpu_owner_open(request,frame,job))return 0;
 main_core=GX2GetMainCoreId();
 if(main_core>2u || OSGetCoreId()!=main_core || GX2GetCurrentDisplayList(0,0))return 0;
 if(!ghs_gpu_owner_open(request,frame,job))return 0;
 /* This is the only GPU wait and HG is genuinely OPEN at this point.
  * CPU ordering does not prove all Game/SDK GPU producers are excluded. */
 if(GX2DrawDone()!=TRUE)return 0;
 if(!ghs_gpu_owner_open(request,frame,job))return 0;
 if(GX2GetMainCoreId()!=main_core || OSGetCoreId()!=main_core || GX2GetCurrentDisplayList(0,0))return 0;
 return ghs_gpu_owner_open(request,frame,job);
}
static void ghs_complete_toast(U result,U command){
 _gz_status=!result&&command==GHS_COMMAND_SAVE?14u:
   result==GHW_CPU_COMMITTED&&command==GHS_COMMAND_RESTORE?15u:16u;
 _gz_pre_hg_completion();
}
static U ghs_release_actual(void){U z;
 if(_gz_jwa_ghs_pending()){z=_gz_jwa_ghs_leave();_gz_ghs_report[18]=z;
  if(z||_gz_jwa_ghs_pending())return GHC_RETAINED;}
 if(ghs.hg){z=_gz_hg_capture_end(ghs.hg);_gz_ghs_report[19]=z;
  if(z)return GHC_RETAINED;ghs.hg=0;}
 ghs.phase=GHS_RELEASED;return _gz_ghs_scope(ghs.request,ghs.frame,3)?0:GHC_RETAINED;}
void _gz_pre_hg_tick(void){U cap,z=GHC_CONTEXT,clean,release,raw,zero,committed=0;
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
 /* Initial ZERO is acquired while the allocator remains OPEN. */
 raw=_gz_jwa_ghs_try(ghs.request);_gz_ghs_report[11]=raw;
 if(raw){z=raw==1u?GHC_CONTEXT:GHC_OBSERVER;goto finish;}
 zero=_gz_jwa_ghs_workers_zero(ghs.request);_gz_ghs_report[12]=zero;
 if(zero!=0){z=GHC_CONTEXT;goto finish;}
 ghs.job_generation=_gz_jwa_ghs_generation(ghs.request);if(!ghs.job_generation){z=GHC_CONTEXT;goto finish;}
 if(!ghs_gpu_retire_open(ghs.request,ghs.frame,ghs.job_generation)){z=GHC_CONTEXT;goto finish;}
 if(!_gz_ghs_context_owned(ghs.request)||_gz_jwa_ghs_check(ghs.request)||
  _gz_jwa_ghs_generation(ghs.request)!=ghs.job_generation||!_gz_gfxq_owned(ghs.request,ghs.frame)){z=GHC_CONTEXT;goto finish;}
 ghs.hg=_gz_hg_capture_begin(ghs.request);ghs.issued_hg=ghs.hg;_gz_ghs_report[10]=ghs.hg;
 if(!ghs.hg){z=GHC_CONTEXT;goto finish;}
 /* No second TRY: this is the same real closed Gate and generation. */
 raw=_gz_jwa_ghs_check(ghs.request);
 if(raw){z=raw==1u?GHC_CONTEXT:GHC_OBSERVER;goto finish;}
 zero=_gz_jwa_ghs_workers_zero(ghs.request);_gz_ghs_report[12]=zero;
 if(zero>1u){z=GHC_CONTEXT;goto finish;}
 raw=_gz_jwa_ghs_generation(ghs.request);
 if(!raw||raw!=ghs.job_generation||!_gz_gfxq_owned(ghs.request,ghs.frame)){z=GHC_CONTEXT;goto finish;}
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
 raw=_gz_rns_mm_finish(ghs.request,ghs.frame,!z&&!release);
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
