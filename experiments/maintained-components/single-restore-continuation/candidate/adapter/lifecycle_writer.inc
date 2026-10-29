/* Append after actual adapter.c and existing WCT bridge: SAME jwa_gate.
 * No old JWA entry/wrapper, JLC context, WHL or HG implementation is changed.
 * The future fixed consumer is deliberately unresolved until its real owner
 * and native Delete/resource call graph are separately reviewed. */
#include "lifecycle_writer.h"
#include "gate.h"
#define LRW_FN __attribute__((section(".text.gz_lrw"),noinline,noclone))
enum {LRW_MAGIC=0x4c525731u,LRW_ARMING=1,LRW_HELD=2,LRW_ENDING=3,
 LRW_ENDED=4,LRW_HALTED=5,LRW_CONTEXT=2850,LRW_BUSY=2851,
 LRW_INTEGRITY=2852,LRW_RETAINED=2853,LRW_EXHAUSTED=2854};
typedef struct {uint32_t busy,active,halted,serial,request,frame,thread,
 nbd_serial,hg_serial,hg_frame,stage,last_frame,seen;} LrwState;
static LrwState lrw __attribute__((section(".bss.gz_lrw")));
/* Permanent, fixed GZ-owned metadata keeps real failed-cleanup evidence valid.
 * This is an explicit new fixed-DATA caller of the unchanged32B HG ABI;
 * unlike the old wrapper stack ticket, it cannot dangle on a terminal return. */
static HgTicket lrw_hg __attribute__((section(".bss.gz_lrw")));
static GzJcaExclusive lrw_job __attribute__((section(".bss.gz_lrw")));
_Static_assert(sizeof(LrwState)==52,"private fixed producer identity");
_Static_assert(sizeof(HgTicket)==32,"actual HG ticket ABI");
_Static_assert(sizeof(GzJcaExclusive)==16,"actual same-Gate token ABI");
extern volatile uint32_t _gz_pbac_frame;
extern uint32_t _gz_nbd_report[16];
extern uint32_t _gz_nbd_context_owned(uint32_t);
extern uint32_t _gz_wct_heap_pending(void),_gz_wct_root_pending(void),_gz_swc_pending(void);
extern uint32_t _gz_boh_has_owner(void);
extern volatile uint32_t _gz_roh_owner[60];
extern GzJcaExclusive _gz_wct_job_token;
/* Actual observations only:3stage4serial5nonce6frame7thread8NBDserial;
 *9HGenter10jobTRY11initialZero12bodyReturn13jobLEAVE14HGexit15firstError;
 *16producerActive17jobActive18ticketLive19READ0 20WORLD0 21missing31
 *22session0. UINT_MAX means NOT CALLED, never success. */
volatile uint32_t _gz_lrw_report[32] __attribute__((section(".data.gz_lrw")))={
 LRW_MAGIC,128,1,0,0,0,0,0,0,UINT32_MAX,UINT32_MAX,UINT32_MAX,
 UINT32_MAX,UINT32_MAX,UINT32_MAX,0,0,0,0,0,0,31,0};
static LRW_FN uint32_t lrw_halt(uint32_t z){
 if(!_gz_lrw_report[15])_gz_lrw_report[15]=z;
 lrw.halted=1;lrw.stage=LRW_HALTED;_gz_lrw_report[3]=LRW_HALTED;return z;
}
static LRW_FN uint32_t lrw_native_context(void){
 uint32_t f=_gz_pbac_frame;
 return lrw.active==1&&!lrw.halted&&lrw.serial&&lrw.request&&lrw.frame==f&&
  _gz_nbd_context_owned(f)&&_gz_nbd_report[0]==0x4e424431u&&_gz_nbd_report[1]==64&&
  _gz_nbd_report[2]==1&&_gz_nbd_report[4]==lrw.nbd_serial&&_gz_nbd_report[5]==f&&
  _gz_hg_platform_thread()==lrw.thread&&lrw.thread==jwa_init_thread&&
  jwa_ordinary_context()&&_gz_lrr_requested()==lrw.request;
}
static LRW_FN uint32_t lrw_job_current(void){
 const volatile GzJcaExclusive*t=&lrw_job;volatile GzJcaGate*g=&jwa_gate;
 return _gz_jwa_lifetime_ready()&&t->active==1&&t->gate==&jwa_gate&&
  (uint32_t)(uintptr_t)t->thread==lrw.thread&&t->generation&&g->closed==1&&
  !g->broken&&!g->active&&(uint32_t)(uintptr_t)g->owner==lrw.thread&&g->epoch==t->generation;
}
static LRW_FN uint32_t lrw_hg_current(void){
 const volatile HgTicket*t=&lrw_hg;const volatile HgControl*c=&_gz_hg_control;
 return lrw.hg_serial&&t->magic==HG_TICKET&&t->self==(uint32_t)(uintptr_t)&lrw_hg&&
  t->serial==lrw.hg_serial&&t->site==0x025d57a0u&&t->live==1&&
  t->frame==lrw.hg_frame&&_gz_hg_frame==lrw.hg_frame&&
  c->magic==HG_MAGIC&&c->bytes==64&&c->version==1&&!c->flags&&
  !(c->state&HG_EXCLUSIVE)&&(c->state&HG_COUNT);
}
static LRW_FN uint32_t lrw_workers(void){uint32_t a,b,c,d;
 if(!_gz_jwa_lifetime_ready()||_gz_jwf_atomic_load(&_gz_jwf_report[3]))return 2;
 a=_gz_jwf_atomic_load(&_gz_jwf_report[16]);b=_gz_jwf_atomic_load(&_gz_jwf_report[17]);
 c=_gz_jwf_atomic_load(&_gz_jwf_report[18]);d=_gz_jwf_atomic_load(&_gz_jwf_report[19]);jwf_barrier();
 if(_gz_jwf_atomic_load(&_gz_jwf_report[3]))return 2;
 return a||b||c||d?1u:0u;
}
LRW_FN uint32_t _gz_lrw_owned(void){
 if(!lrw_native_context()||lrw.stage!=LRW_HELD||!lrw_hg_current()||!lrw_job_current())return 0;
 /* All four counters remain mandatory ZERO, including old9 bootstrap[19].
  * Even a counted caller that will wait behind CLOSED is conservatively
  * refused. No nonzero worker status is interpreted as physical closure. */
 return lrw_workers()==0u;
}
static LRW_FN uint32_t lrw_release(void){uint32_t z;
 lrw.stage=LRW_ENDING;_gz_lrw_report[3]=LRW_ENDING;
 if(lrw_job.active||lrw_job.gate){
  /* Real same-thread cleanup is independent of an expired source/NBD context.
   * No native/resource mutex or callback remains live at this call edge. */
  if(!jwa_ordinary_context())return lrw_halt(LRW_RETAINED);
  z=_gz_jca_exclusive_leave(&jwa_gate,&lrw_job);_gz_lrw_report[13]=z;
  if(z!=GZ_JCA_OK||lrw_job.active||lrw_job.gate)return lrw_halt(LRW_RETAINED);
 }
 if(lrw.hg_serial){
  z=_gz_hg_native_exit(&lrw_hg,lrw.hg_serial);_gz_lrw_report[14]=z;
  if(z!=HG_OK||lrw_hg.live)return lrw_halt(LRW_RETAINED);
  lrw.hg_serial=0;
 }
 lrw.stage=LRW_ENDED;_gz_lrw_report[3]=LRW_ENDED;return 0;
}
LRW_FN uint32_t _gz_lrw_dispatch(void){uint32_t request,f,thread,ndb,raw,i,fault=0;
 request=_gz_lrr_requested();
 if(!request&&!lrw.halted&&!lrw_job.active&&!lrw_job.gate&&!lrw.hg_serial)return 0;
 if(!__sync_bool_compare_and_swap(&lrw.busy,0,1))return 1;
 if(lrw.halted||lrw.active||lrw_job.active||lrw_job.gate||lrw.hg_serial||lrw_hg.live){lrw_halt(LRW_RETAINED);goto done;}
 f=_gz_pbac_frame;
 if(!request||!_gz_nbd_context_owned(f)||!jwa_ordinary_context()||!_gz_jwa_lifetime_ready())goto done;
 thread=(uint32_t)(uintptr_t)OSGetCurrentThread();ndb=_gz_nbd_report[4];
 if(!thread||thread!=jwa_init_thread||thread!=_gz_hg_platform_thread()||!f||
  !ndb||ndb==UINT32_MAX||_gz_nbd_report[0]!=0x4e424431u||_gz_nbd_report[1]!=64||
  _gz_nbd_report[2]!=1||_gz_nbd_report[5]!=f||(lrw.seen&&lrw.last_frame==f))goto done;
 /* No existing reader/ordinary restore may be upgraded to this writer. */
 if(_gz_wct_heap_pending()||_gz_wct_root_pending()||_gz_swc_pending()||_gz_boh_has_owner()||
  _gz_roh_owner[4]||_gz_wct_job_token.active||_gz_wct_job_token.gate)goto done;
 if(lrw.serial==UINT32_MAX){lrw_halt(LRW_EXHAUSTED);goto done;}
 if(_gz_hg_control.flags){lrw_halt(LRW_INTEGRITY);goto done;}
 if(_gz_hg_control.state&HG_EXCLUSIVE)goto done;
 lrw.request=request;lrw.frame=f;lrw.thread=thread;lrw.nbd_serial=ndb;
 lrw.seen=1;lrw.last_frame=f;++lrw.serial;lrw.active=1;lrw.stage=LRW_ARMING;jwa_barrier();
 _gz_lrw_report[3]=LRW_ARMING;_gz_lrw_report[4]=lrw.serial;_gz_lrw_report[5]=request;
 _gz_lrw_report[6]=f;_gz_lrw_report[7]=thread;_gz_lrw_report[8]=ndb;
 for(i=9;i<15;i++)_gz_lrw_report[i]=UINT32_MAX;
 /* This is the genuine shared native writer, NEVER capture_begin. It may
  * wait behind a racing reader exactly like existing native wrappers; no
  * nonwaiting/wall-time claim is made for this HG primitive. */
 lrw.hg_frame=_gz_hg_frame;
 raw=_gz_hg_native_enter(&lrw_hg,0x025d57a0u);_gz_lrw_report[9]=raw;
 if(!raw){
  /* A failed HG observation has no authentic serial. Do not guess a count
   * was acquired or call count-only exit0 to release another operation. */
  lrw_halt(LRW_RETAINED);goto expire;
 }
 lrw.hg_serial=raw;
 if(!lrw_native_context()||!lrw_hg_current()){fault=LRW_INTEGRITY;goto release;}
 raw=_gz_jca_exclusive_try(&jwa_gate,&lrw_job);_gz_lrw_report[10]=raw;
 if(raw!=GZ_JCA_OK){
  if(lrw_job.active||lrw_job.gate){lrw_halt(LRW_RETAINED);goto expire;}
  if(raw!=GZ_JCA_BUSY){fault=LRW_INTEGRITY;}
  goto release;
 }
 raw=lrw_workers();_gz_lrw_report[11]=raw;
 if(raw){if(raw!=1)fault=LRW_INTEGRITY;goto release;}
 lrw.stage=LRW_HELD;_gz_lrw_report[3]=LRW_HELD;jwa_barrier();
 if(!_gz_lrw_owned()){fault=LRW_INTEGRITY;goto release;}
 /* ONE fixed genuine consumer for this frame. Its actual typed owner and
  * native call graph are separate mandatory gates; no callback pointer or
  * fabricated success exists. Its implementation is not supplied here. */
 raw=_gz_lrr_native_step();_gz_lrw_report[12]=raw;
 if(!_gz_lrw_owned())fault=LRW_INTEGRITY;
release:
 raw=lrw_release();if(raw)fault=raw;
 if(fault)lrw_halt(fault);
expire:
 /* Successful or terminal producer authority ends WITH this native callback.
  * Permanent tokens may remain as truthful terminal evidence, never as a
  * cookie that a later callback may reuse or an input/READ grant. */
 lrw.active=0;jwa_barrier();
done:
 _gz_lrw_report[16]=lrw.active;_gz_lrw_report[17]=lrw_job.active;_gz_lrw_report[18]=lrw_hg.live;
 lrw.busy=0;return 1;
}
/* Private permanent-token observation. No report word is authority. A
 * caller still needs its own sealed nonce and a NEW ordinary world owner. */
LRW_FN uint32_t _gz_lrw_quiescent(void){const volatile LrwState*s=&lrw;
 if(s->busy||s->active||s->halted||s->hg_serial||lrw_hg.live||lrw_job.active||lrw_job.gate)return 0;
 jwa_barrier();
 return !s->busy&&!s->active&&!s->halted&&!s->hg_serial&&!lrw_hg.live&&!lrw_job.active&&!lrw_job.gate;
}
LRW_FN uint32_t _gz_lrw_completed(uint32_t nonce,uint32_t serial){const volatile LrwState*s=&lrw;
 if(!nonce||!serial||s->request!=nonce||s->serial!=serial||s->stage!=LRW_ENDED||!_gz_lrw_quiescent())return 0;
 jwa_barrier();
 return s->request==nonce&&s->serial==serial&&s->stage==LRW_ENDED&&_gz_lrw_quiescent();
}
#undef LRW_FN
