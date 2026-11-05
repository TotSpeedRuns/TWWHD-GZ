/* Same TU as the actual tested jwa_gate. No copied or second Gate. */
#include "wct_job_bridge.h"
#define WJL_FN __attribute__((section(".text.gz_wjl"),noinline,noclone))
GzJcaExclusive _gz_wct_job_token __attribute__((section(".bss.gz_wjl")));
WJL_FN uint32_t _gz_jwa_wct_try(uint32_t token){
 if(!_gz_wct_heap_context_owned(token)||!jwa_ordinary_context())return WJL_CONTEXT;
 if(!_gz_jwa_lifetime_ready())return WJL_NOT_READY;
 if((uint32_t)(uintptr_t)OSGetCurrentThread()!=jwa_init_thread)return WJL_CONTEXT;
 if(_gz_wct_job_token.active||_gz_wct_job_token.gate)return WJL_STORAGE;
 return _gz_jca_exclusive_try(&jwa_gate,&_gz_wct_job_token);
}
WJL_FN uint32_t _gz_jwa_wct_check(uint32_t token){
 const volatile GzJcaExclusive*t=&_gz_wct_job_token;volatile GzJcaGate*g=&jwa_gate;OSThread*thread;
 if(!_gz_wct_heap_context_owned(token)||!jwa_ordinary_context())return WJL_CONTEXT;
 if(!_gz_jwa_lifetime_ready())return WJL_NOT_READY;
 thread=OSGetCurrentThread();jwa_barrier();
 return thread&&(uint32_t)(uintptr_t)thread==jwa_init_thread&&t->active==1&&t->gate==&jwa_gate&&
  t->thread==thread&&t->generation&&g->closed==1&&!g->broken&&!g->active&&g->owner==thread&&g->epoch==t->generation?GZ_JCA_OK:GZ_JCA_BAD;
}
WJL_FN uint32_t _gz_jwa_wct_workers_zero(uint32_t token){uint32_t a,b,c,d;
 if(_gz_jwa_wct_check(token)!=GZ_JCA_OK||_gz_jwf_atomic_load(&_gz_jwf_report[3]))return WJL_BROKEN;
 a=_gz_jwf_atomic_load(&_gz_jwf_report[16]);b=_gz_jwf_atomic_load(&_gz_jwf_report[17]);
 c=_gz_jwf_atomic_load(&_gz_jwf_report[18]);d=_gz_jwf_atomic_load(&_gz_jwf_report[19]);jwf_barrier();
 if(_gz_jwf_atomic_load(&_gz_jwf_report[3]))return WJL_BROKEN;
 return a||b||c||d?WJL_BUSY:WJL_ZERO;
}
WJL_FN uint32_t _gz_jwa_wct_leave(void){
 if(!jwa_ordinary_context())return WJL_CONTEXT;
 /* Cleanup remains genuine and independent of an expired producer. */
 return _gz_jca_exclusive_leave(&jwa_gate,&_gz_wct_job_token);
}
#undef WJL_FN
