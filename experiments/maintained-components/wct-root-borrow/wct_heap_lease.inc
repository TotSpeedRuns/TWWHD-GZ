/* Include after genuine world_group_publication.c, with the frozen header
 * included before WCT/SSP definitions. No general read/world provider. */
#include "wct_heap_lease.h"
#include "wct_job_bridge.h"
#include "worker_atomic.h"
#include <coreinit/interrupts.h>
extern u32 _gz_jol_coupled_halted(void),_gz_jol_coupled_outstanding(void);
enum {WHL_MAGIC=0x57484c31u,WHL_ARMING=1,WHL_HELD=2,WHL_ENDING=3,
      WHL_ENDED=4,WHL_HALTED=5};
typedef struct {u32 magic,bytes,phase,token,frame,thread,parent,observer_phase,
 hg,identity,borrowed,error,seal;} WhlState;
typedef char WhlStateSize[sizeof(WhlState)==52?1:-1];
static WhlState whl __attribute__((section(".bss.gz_whl")));
static u32 whl_busy __attribute__((section(".bss.gz_whl")));
static u32 whl_serial __attribute__((section(".bss.gz_whl")));
/* Main-thread diagnostics, not a seqlocked cut or capability. 3phase,
 * 4acquisition identity,5token,6frame,7thread,8parent,9observerphase,
 * 10actualHG,11rawBegin,12rawHGresult,13rawEnd,14firstError,15borrowMode,
 * 16IRQprevious,17IRQrestore,18WORLD0,19READ0,20missing31,21session0,
 * 22actualJobTRY,23workerZero,24actualJobLEAVE. 8nativeNBDserial/9kind1.
 * This is a real coupled diagnostic, not an all-audio lifetime certificate. */
volatile u32 _gz_wct_heap_report[32] __attribute__((section(".data.gz_whl")))={
 WHL_MAGIC,128,2,0,0,0,0,0,0,0,0,BAD,BAD,BAD,0,0,BAD,BAD,0,0,31,0,BAD,BAD,BAD};
static void whl_seal(void){whl.seal=btr_hash(&whl,sizeof(whl)-4);__asm__ volatile("sync":::"memory");}
static u32 whl_valid(void){return whl.magic==WHL_MAGIC&&whl.bytes==sizeof(whl)&&
 whl.token&&whl.frame&&whl.thread&&whl.parent&&whl.identity&&
 whl.seal==btr_hash(&whl,sizeof(whl)-4);}
static u32 whl_fail(u32 error){if(!whl.error)whl.error=error;
 whl.phase=WHL_HALTED;whl_seal();_gz_wct_heap_report[3]=WHL_HALTED;
 if(!_gz_wct_heap_report[14])_gz_wct_heap_report[14]=error;return error;}
u32 _gz_wct_heap_pending(void){return whl.phase||whl.magic?1u:0u;}
u32 _gz_wct_heap_claims(u32 cookie){return cookie&&cookie==whl.token&&_gz_wct_heap_pending()?1u:0u;}
/* No HG API is called here: this is the fixed real provider queried by HG
 * itself, so native current metadata/IRQ checks cannot recurse through seal. */
u32 _gz_wct_heap_context_owned(u32 cookie){const volatile u32*o=_gz_roh_owner;u32 previous,restored;
 if(!whl_valid()||whl.error||cookie!=whl.token||
  (whl.phase!=WHL_ARMING&&whl.phase!=WHL_HELD&&whl.phase!=WHL_ENDING)||
  !wct_entered||whl.frame!=_gz_pbac_frame||_gz_hg_platform_thread()!=whl.thread||
  whl.observer_phase!=1||_gz_wct_native_owned()!=whl.parent||
  o[0]!=ROH_MAGIC||o[1]!=240||o[2]!=2||o[4]!=1||o[5]!=cookie||
  !o[11]||o[10]!=whl.frame||_gz_boh_has_owner())return 0;
 if(o[3]==ROH_CLEAN){if(o[6]||o[7]||(o[12]!=0&&o[12]!=1)||
   !wsp_source.valid||wsp_source.token!=cookie||wsp_source.frame!=whl.frame||
   wsp_source_seal!=btr_hash(&wsp_source,sizeof(wsp_source)))return 0;
 }else{
  if((o[3]!=ROH_MUTATING&&o[3]!=ROH_FAILED_DIRTY)||o[6]!=1||
   (o[3]==ROH_MUTATING&&o[7])||(o[3]==ROH_FAILED_DIRTY&&!o[7])||
   wct.magic!=0x57435231u||wct.bytes!=sizeof(wct)||wct.valid!=1||
   wct.token!=cookie||wct.frame!=whl.frame||wct.phase<1||wct.phase>4||
   wct_seal!=btr_hash(&wct,sizeof(wct)))return 0;
 }
 previous=(u32)OSDisableInterrupts();restored=(u32)OSRestoreInterrupts((BOOL)previous);
 if(!previous||restored)return 0;
 __asm__ volatile("sync":::"memory");
 return whl_valid()&&!whl.error&&cookie==whl.token&&
  whl.frame==_gz_pbac_frame&&_gz_hg_platform_thread()==whl.thread&&
  _gz_wct_native_owned()==whl.parent?1u:0u;
}
u32 _gz_wct_heap_held(u32 t,u32 f){return whl_valid()&&whl.phase==WHL_HELD&&
 whl.token==t&&whl.frame==f&&whl.hg&&!whl.error&&
 _gz_wct_heap_context_owned(t)&&!_gz_hg_capture_seal(whl.hg)&&
 /* A new entry waiting behind our already-CLOSED Gate is ordinary BUSY.
  * Actual check/sticky failure is not authority to touch a native object. */
 _gz_jwa_wct_workers_zero(t)!=WJL_BROKEN?1u:0u;}
u32 _gz_wct_heap_identity(u32 t,u32 f){return _gz_wct_heap_held(t,f)?whl.identity:0;}

/* Both real tokens remain caller-owned until actual cleanup succeeds. This
 * helper is also the ordinary pre-mutation BUSY path: never wait/drain or
 * clear a token by hand, and never perform native/source-body work first. */
static u32 whl_release_actual(void){u32 raw;
 if(_gz_wct_root_pending())return whl_fail(WHL_ROOT_RETAINED);
 if(_gz_wct_job_token.active||_gz_wct_job_token.gate){
  raw=_gz_jwa_wct_leave();_gz_wct_heap_report[24]=raw;
  if(raw!=GZ_JCA_OK||_gz_wct_job_token.active||_gz_wct_job_token.gate)return whl_fail(WHL_RETAINED);
 }
 if(whl.hg){raw=_gz_hg_capture_end(whl.hg);_gz_wct_heap_report[13]=raw;
  if(raw)return whl_fail(WHL_RETAINED);whl.hg=0;}
 whl.phase=WHL_ENDED;whl_seal();_gz_wct_heap_report[3]=WHL_ENDED;_gz_wct_heap_report[10]=0;return 0;
}
u32 _gz_wct_heap_begin(u32 t,u32 f){u32 parent,thread,previous,restored,raw,zero,clean,z=WHL_CONTEXT;
 if(_gz_wct_root_pending()||_gz_wct_heap_pending()||_gz_wct_job_token.active||_gz_wct_job_token.gate||
  !_gz_jwf_atomic_cas(&whl_busy,0,1))return WHL_RETAINED;
 if(!t||f!=_gz_pbac_frame||!_gz_wct_group_context(t,f,1)||
  _gz_wgp_unreleased()||!ssp_none())goto done;
 if(_gz_jol_coupled_halted()||_gz_jol_coupled_outstanding()){z=WHL_RETAINED;goto done;}
 parent=_gz_wct_native_owned();
 thread=_gz_hg_platform_thread();previous=(u32)OSDisableInterrupts();restored=(u32)OSRestoreInterrupts((BOOL)previous);
 _gz_wct_heap_report[16]=previous;_gz_wct_heap_report[17]=restored;
 if(!parent||!thread||!previous||restored)goto done;
 if(whl_serial==BAD){z=WHL_EXHAUSTED;goto done;}
 bcl_zero(&whl,sizeof(whl));whl.magic=WHL_MAGIC;whl.bytes=sizeof(whl);whl.phase=WHL_ARMING;
 whl.token=t;whl.frame=f;whl.thread=thread;whl.parent=parent;whl.observer_phase=1;whl.identity=++whl_serial;whl_seal();
 _gz_wct_heap_report[3]=WHL_ARMING;_gz_wct_heap_report[4]=whl.identity;_gz_wct_heap_report[5]=t;
 _gz_wct_heap_report[6]=f;_gz_wct_heap_report[7]=thread;_gz_wct_heap_report[8]=parent;_gz_wct_heap_report[9]=1;
 _gz_wct_heap_report[11]=_gz_wct_heap_report[12]=_gz_wct_heap_report[13]=BAD;
 _gz_wct_heap_report[22]=_gz_wct_heap_report[23]=_gz_wct_heap_report[24]=BAD;
 whl.hg=_gz_hg_capture_begin(t);_gz_wct_heap_report[10]=whl.hg;_gz_wct_heap_report[11]=whl.hg;
 /* Exact existing native HG64 ABI: result is word15. The same actual TU
  * already declares its volatile sixteen-word control in npc_service_prepare. */
 _gz_wct_heap_report[12]=_gz_hg_control[15];
 if(!whl.hg){whl.phase=WHL_ENDED;whl_seal();_gz_wct_heap_report[3]=WHL_ENDED;
  z=_gz_hg_control[15]==1u?WHL_BUSY:WHL_CONTEXT;goto done;}
 whl_seal();
 raw=_gz_jwa_wct_try(t);_gz_wct_heap_report[22]=raw;
 if(raw!=GZ_JCA_OK){
  if(_gz_wct_job_token.active||_gz_wct_job_token.gate){z=whl_fail(WHL_RETAINED);goto done;}
  z=raw==GZ_JCA_BUSY?WHL_BUSY:WHL_CONTEXT;clean=whl_release_actual();if(clean)z=clean;goto done;
 }
 zero=_gz_jwa_wct_workers_zero(t);_gz_wct_heap_report[23]=zero;
 if(zero!=WJL_ZERO){z=zero==WJL_BUSY?WHL_BUSY:WHL_INTEGRITY;clean=whl_release_actual();if(clean)z=clean;goto done;}
 whl.phase=WHL_HELD;whl_seal();_gz_wct_heap_report[3]=WHL_HELD;
 if(!_gz_wct_heap_held(t,f)){z=whl_fail(WHL_INTEGRITY);goto done;}z=0;
done:whl_busy=0;return z;}

u32 _gz_wct_heap_borrow(u32 t,u32 f,u32 mode){u32 result=0;
 if(_gz_wct_root_pending()||!whl_valid()||whl.borrowed||mode<1||mode>4||!_gz_jwf_atomic_cas(&whl_busy,0,1))return 0;
 if(!_gz_wct_heap_held(t,f)||!_gz_wgp_context_owned(t)||!wgp()||wgp()[7]!=mode)goto done;
 whl.borrowed=mode;whl_seal();_gz_wct_heap_report[15]=mode;result=whl.hg;
done:whl_busy=0;return result;}
u32 _gz_wct_heap_unborrow(u32 t,u32 f,u32 mode,u32 actual_hg){u32 z=WHL_RETAINED;
 if(!whl_valid()||!_gz_jwf_atomic_cas(&whl_busy,0,1))return z;
 if(_gz_wct_root_pending()||whl.borrowed!=mode||actual_hg!=whl.hg||!_gz_wct_heap_held(t,f)||
  !_gz_wgp_context_owned(t)||!wgp()||wgp()[7]!=mode)goto done;
 whl.borrowed=0;whl_seal();_gz_wct_heap_report[15]=0;z=0;
done:whl_busy=0;return z;}

/* Only the checked fixed commit/recovery/clean-abort sites call END. Their
 * final SSP binding/target validation MUST remain before this call. No
 * native/audio access is allowed after a successful end before private leaf. */
u32 _gz_wct_heap_end(u32 t,u32 f){const volatile u32*o=_gz_roh_owner;u32 z=WHL_RETAINED;
 if(_gz_wct_root_pending())return WHL_ROOT_RETAINED;
 if(!_gz_wct_heap_pending())return 0;
 if(!whl_valid()||whl.token!=t||whl.frame!=f||!_gz_jwf_atomic_cas(&whl_busy,0,1))return z;
 if(whl.phase==WHL_ENDED&&!whl.hg&&!whl.borrowed&&!whl.error&&
  !_gz_wct_job_token.active&&!_gz_wct_job_token.gate){z=0;goto done;}
 if(whl.borrowed||_gz_wgp_unreleased()||!_gz_wct_heap_held(t,f))goto done;
 if(o[3]==ROH_CLEAN){if(o[6]||o[7]||o[12]||!ssp_none())goto done;}
 else if(!(wct.valid==1&&wct.token==t&&wct.frame==f&&
  ((wct.phase==WCT_ROLLED&&wct.returned==1&&_gz_wct_closure_hash==wct_seal)||
   (wct.phase==WCT_FORWARD&&!wct.returned&&!wct.error&&wct_commit_arming))))goto done;
 whl.phase=WHL_ENDING;whl_seal();_gz_wct_heap_report[3]=WHL_ENDING;
 z=whl_release_actual();
done:if(z&&whl.phase!=WHL_HALTED)z=whl_fail(z);whl_busy=0;return z;}
void _gz_wct_heap_finish_leaf(u32 t){
 if(_gz_wct_root_pending()||whl_busy||!whl_valid()||whl.token!=t||whl.phase!=WHL_ENDED||whl.hg||whl.borrowed||whl.error||
  _gz_wct_job_token.active||_gz_wct_job_token.gate)return;
 bcl_zero(&whl,sizeof(whl));_gz_wct_heap_report[3]=0;
 /* whl_serial and historical diagnostic identity are deliberately retained. */
}
