#include "particle_guard.h"
typedef JpgU U;
typedef unsigned long long T;
typedef T (*JpgStamp)(void);
typedef U (*JpgWaitStamp)(T);
enum {GATE=0x80000000u,COUNT=0x7fffffffu,BAD=0xffffffffu,
 LOST=1,LIMIT=2,CHANGED=4};
typedef struct {U cookie,kind,thread,manager;} JpgCall;
static JpgCall jpg_calls[JPG_CALLS];
static U jpg_admission,jpg_lock,jpg_serial,jpg_token,jpg_thread;
static U jpg_mode,jpg_capture_mode,jpg_capture_frame,jpg_captures,jpg_capture_ends;
static U jpg_gpu_entered,jpg_gpu_valid,jpg_gpu_owner_mode,jpg_gpu_owner,jpg_gpu_frame;
static U jpg_owned_calls[JPG_METHODS];
static U jpg_gpu_module;
static JpgStamp jpg_gpu_submitted,jpg_gpu_retired;
static JpgWaitStamp jpg_gpu_wait_stamp;
static U jpg_heap_open(void);
static U jpg_entered[JPG_METHODS],jpg_returned[JPG_METHODS];
static U jpg_skipped[JPG_METHODS],jpg_waited[JPG_METHODS];
U _gz_jpg_report[96]={0x4a504732u,384,2};
extern volatile U _gz_roh_owner[60],_gz_roh_serial,_gz_po_frame;
extern U _gz_hg_platform_thread(void),_gz_roh_gl_context(U,U,U);
extern U _gz_roh_lifetime_common(U,U),_gz_roh_replay_common(U,U,U),_gz_roh_recovery_common(U,U);
extern void OSSleepTicks(long long);
extern U GX2DrawDone(void);
extern int OSDynLoad_Acquire(const char*,U*);
extern int OSDynLoad_FindExport(U,U,const char*,void*);
extern volatile U _gz_hg_control[16];
extern U _gz_rkc_particle_entry(U,U),_gz_rkc_particle_identity(U,U,U,U);
extern U _gz_gac_particle_entry(U),_gz_gac_particle_identity(U,U,U);
extern U _gz_ghs_particle_entry(U,U),_gz_ghs_particle_identity(U,U,U);
extern U _gz_ghs_particle_releasing(U,U,U);
static U jpg_capture_entry(U mode,U frame){return mode==3?_gz_gac_particle_entry(frame):_gz_rkc_particle_entry(mode,frame);}
static U jpg_capture_identity(U mode,U frame,U cookie,U thread){return mode==3?_gz_gac_particle_identity(frame,cookie,thread):_gz_rkc_particle_identity(mode,frame,cookie,thread);}

/* Keep RA=0 on all reservation stores, including indexed counter accesses.
 * The emitted-code audit also checks this property, rather than trusting C. */
static __attribute__((noinline)) U jpg_add(U*p,U d){return __sync_fetch_and_add(p,d);}
static void jpg_fault(U f){__sync_fetch_and_or(_gz_jpg_report+3,f);}
static U jpg_idle(void){const volatile U*o=_gz_roh_owner;
 return o[0]==0x524f4831u&&o[1]==240&&o[2]==2&&!o[3]&&!o[4]&&!o[5]&&!o[6]&&!o[12];}
static U jpg_owned(U token){const volatile U*o=_gz_roh_owner;
 return token&&o[0]==0x524f4831u&&o[1]==240&&o[2]==2&&o[3]>=1&&o[3]<=4&&
  o[4]==1&&o[5]==token&&_gz_roh_serial==token;}
/* Capture identity is a read-only scalar query from the already entered
 * fixed renderer callback. It does not borrow a restore owner or allow any
 * native replay. Identity checks are valid from foreign writer threads. */
static U jpg_current(void){return jpg_mode==1?jpg_owned(jpg_token):
 jpg_mode==2?jpg_capture_identity(jpg_capture_mode,jpg_capture_frame,jpg_token,jpg_thread)==1:
 jpg_mode==3?_gz_ghs_particle_identity(jpg_token,jpg_capture_frame,jpg_thread)==1:0;}
static void jpg_publish(void){U i;
 _gz_jpg_report[4]=*(volatile U*)&jpg_admission;
 _gz_jpg_report[5]=jpg_token;_gz_jpg_report[6]=jpg_thread;_gz_jpg_report[7]=_gz_po_frame;
 for(i=0;i<JPG_METHODS;i++){
  _gz_jpg_report[16+i]=jpg_entered[i];_gz_jpg_report[25+i]=jpg_returned[i];
  _gz_jpg_report[34+i]=jpg_skipped[i];_gz_jpg_report[43+i]=jpg_waited[i];
  _gz_jpg_report[70+i]=jpg_owned_calls[i];
 }
 /* Deliberately unresolved: all indirect producers and GPU resource lifetime.
  * A completed DrawDone is recorded separately; it cannot clear these gaps. */
 _gz_jpg_report[52]=3;
 _gz_jpg_report[53]=jpg_mode;_gz_jpg_report[54]=jpg_capture_mode;_gz_jpg_report[55]=jpg_capture_frame;
 _gz_jpg_report[56]=jpg_captures;_gz_jpg_report[57]=jpg_capture_ends;
 _gz_jpg_report[62]=jpg_gpu_valid;_gz_jpg_report[63]=jpg_gpu_owner_mode;
 _gz_jpg_report[64]=jpg_gpu_owner;_gz_jpg_report[65]=jpg_gpu_frame;_gz_jpg_report[68]=jpg_gpu_entered;
}
/* Ordinary entry/return changes one diagnostic counter lane. Do not copy
 * every other method's unchanged counters twice per particle operation.
 * Private admission/authority state and all capture/restore checks are kept.
 * Active ownership always publishes the original complete report. */
static __attribute__((noinline)) void jpg_publish_live(U kind){
 if(jpg_mode){jpg_publish();return;}
 _gz_jpg_report[4]=*(volatile U*)&jpg_admission;
 _gz_jpg_report[7]=_gz_po_frame;
 _gz_jpg_report[16+kind]=jpg_entered[kind];
 _gz_jpg_report[25+kind]=jpg_returned[kind];
 _gz_jpg_report[52]=3;
}
void _gz_jpg_poll(void){U state;
 if(jpg_mode!=1||!jpg_token||!jpg_idle()||!__sync_bool_compare_and_swap(&jpg_lock,0,1))return;
 state=*(volatile U*)&jpg_admission;
 if(jpg_token&&jpg_idle()&&state==GATE){
  /* No admitted call remains. A new acquirer is serialized by this private
   * lock. Keep the token intact if the atomic opening unexpectedly fails. */
  __sync_synchronize();
  if(!__sync_bool_compare_and_swap(&jpg_admission,GATE,0))jpg_fault(CHANGED);
  else {jpg_token=jpg_thread=jpg_mode=jpg_gpu_valid=0;_gz_jpg_report[10]++;}
 }
 jpg_publish();__sync_synchronize();jpg_lock=0;
}
static U jpg_native_scope(U thread){U token=jpg_token,frame=_gz_po_frame;
 const volatile U*o=_gz_roh_owner;
 if(jpg_mode!=1||thread!=jpg_thread||!jpg_owned(token)||o[10]!=frame||!o[11])return 0;
 /* These queries preserve the original owner checks. They do not issue a
  * lifetime, native-call or replay permit; each existing caller still needs
  * its original typed authority before invoking a native method. */
 if(o[3]==2&&o[6]==1&&!o[7]){
  if(o[12]==9)return _gz_roh_lifetime_common(token,0)==0;
  if(o[12]>=2&&o[12]<=8)return _gz_roh_replay_common(token,frame,0)==0;
 }
 if(o[3]==4&&o[6]==1&&o[7])return _gz_roh_recovery_common(token,frame)==0;
 return 0;
}
/* Only the real collective provider may select the additional method lane.
 * An inactive collective retains the original single-bomb policy verbatim.
 * A selected but invalid collective must never fall through to that policy.
 * The provider checks the already-held HG/Gate receipts without JPG COUNT0.
 * No job/GPU wait, acquisition, native body or report grant occurs here. */
extern U _gz_lsb_jpg_native_scope(U,U,U,U,U,U);
extern U _gz_scf_has_owner(void);
static U jpg_method_scope(U kind,U manager,U emitter,U thread,U recorded,U resource,U group,U index){
 U target,token=jpg_token,frame=_gz_po_frame;const volatile U*o=_gz_roh_owner;
 if(jpg_mode!=1||thread!=jpg_thread||!jpg_owned(token)||!o[11])return 0;
 target=_gz_lsb_jpg_native_scope(token,frame,thread,kind,manager,emitter);
 if(!target)return jpg_native_scope(thread);
 /* A positive collective provider already authenticated the actual SCF
  * failed-owner ticket, current native ingress, HG and same Gate. Only that
  * selected SCF lane may use its independently bounded recovery frame.
  * Inactive and original non-SCF collective paths keep the legacy guard. */
 if(o[10]!=frame&&_gz_scf_has_owner()!=1)return 0;
 if((target!=294&&target!=467)||recorded>1||
  *(volatile U*)&jpg_admission!=(GATE|recorded))return 0;
 /* The real native467 create uses this exact resource/group and a finite
  * resource-manager index. COUNT1 is accepted only by owned_create AFTER it
  * has found the single genuine call-table entry installed by this entry. */
 if(target==467&&kind==6&&!recorded&&
  (resource!=0x2027u||group!=2||index>=8))return 0;
 return 1;
}
static U jpg_record(U kind,U manager,U thread){U i,cookie;
 for(i=0;i<JPG_CALLS;i++)if(__sync_bool_compare_and_swap(&jpg_calls[i].cookie,0,BAD))break;
 if(i==JPG_CALLS){jpg_fault(LIMIT);__sync_fetch_and_sub(&jpg_admission,1);return BAD;}
 cookie=__sync_add_and_fetch(&jpg_serial,1);
 if(!cookie||cookie==BAD){jpg_fault(LIMIT);jpg_calls[i].cookie=0;__sync_fetch_and_sub(&jpg_admission,1);return BAD;}
 jpg_calls[i].kind=kind;jpg_calls[i].thread=thread;jpg_calls[i].manager=manager;
 if((*(volatile U*)&jpg_admission)&GATE){
  jpg_gpu_valid=0;jpg_add(jpg_owned_calls+kind,1);
 }
 jpg_add(jpg_entered+kind,1);__sync_synchronize();jpg_calls[i].cookie=cookie;
 jpg_publish_live(kind);return cookie;
}
static U jpg_before(U kind,U manager,U emitter,U resource,U group,U index){U s,thread=_gz_hg_platform_thread(),attempt;
 if(kind>=JPG_METHODS||!thread){jpg_fault(CHANGED);return BAD;}
 for(;;){
  _gz_jpg_poll();s=*(volatile U*)&jpg_admission;
  if(s&GATE){
   if(!jpg_current()&&(jpg_mode!=1||!jpg_idle()))jpg_fault(CHANGED);
   if(!jpg_method_scope(kind,manager,emitter,thread,0,resource,group,index)){
    if(kind!=6&&kind!=7){jpg_add(jpg_skipped+kind,1);jpg_publish();return 0;}
    /* Creation/removal may be paired with caller-owned callback lifetimes.
     * Never return an invented removal result or lose a constructor call.
     * Wait outside the native method/JEL retirement and retry its exact ABI.
     * A same-thread unexpected lifetime is a hard diagnostic gap; it waits
     * rather than silently freeing an emitter's still-live callback. */
    if(thread==jpg_thread)jpg_fault(CHANGED);
    jpg_add(jpg_waited+kind,1);jpg_publish();OSSleepTicks(62156LL);continue;
   }
  }
  for(attempt=0;attempt<64;attempt++){
   if((s&COUNT)==COUNT){jpg_fault(LIMIT);return BAD;}
   if(__sync_bool_compare_and_swap(&jpg_admission,s,s+1))return jpg_record(kind,manager,thread);
   s=*(volatile U*)&jpg_admission;
   /* Re-evaluate owner scope whenever the gate changed under this entry. */
   if(s&GATE)break;
  }
  if(attempt==64){jpg_fault(LOST);return BAD;}
 }
}
static void jpg_after(U cookie){U i,kind,thread;
 if(!cookie||cookie==BAD)return;thread=_gz_hg_platform_thread();
 for(i=0;i<JPG_CALLS&&jpg_calls[i].cookie!=cookie;i++);
 if(i==JPG_CALLS||jpg_calls[i].thread!=thread||jpg_calls[i].kind>=JPG_METHODS||
  !((*(volatile U*)&jpg_admission)&COUNT)||!__sync_bool_compare_and_swap(&jpg_calls[i].cookie,cookie,BAD)){
  jpg_fault(CHANGED);return;
 }
 kind=jpg_calls[i].kind;jpg_add(jpg_returned+kind,1);
 __sync_synchronize();jpg_calls[i].cookie=0;__sync_synchronize();
 __sync_fetch_and_sub(&jpg_admission,1);jpg_publish_live(kind);
}
U _gz_jpg_check(U token,U frame){U i,z=0,state=*(volatile U*)&jpg_admission;
 if(jpg_mode!=1||!token||jpg_token!=token||!jpg_owned(token)||frame!=_gz_po_frame||!jpg_thread||
  _gz_hg_platform_thread()!=jpg_thread||(state&GATE)==0)return JPG_OWNER;
 if(state!=GATE)return JPG_WAIT;
 __sync_synchronize();
 for(i=0;i<JPG_CALLS;i++)if(jpg_calls[i].cookie)z=JPG_CORRUPT;
 for(i=0;i<JPG_METHODS;i++)if(jpg_entered[i]!=jpg_returned[i])z=JPG_CORRUPT;
 if(_gz_jpg_report[3])z=JPG_CORRUPT;
 if(!jpg_owned(token)||frame!=_gz_po_frame||*(volatile U*)&jpg_admission!=GATE)return JPG_OWNER;
 _gz_jpg_report[11]=z;_gz_jpg_report[12]++;jpg_publish();return z;
}
U _gz_jpg_acquire(U token,U frame){U z=JPG_OWNER,thread;
 if(!token||frame!=_gz_po_frame||!jpg_owned(token)||_gz_roh_owner[3]!=1||_gz_roh_owner[6]||
  _gz_roh_owner[12]!=1||_gz_roh_gl_context(token,frame,1)!=1)return JPG_OWNER;
 thread=_gz_hg_platform_thread();if(!thread)return JPG_OWNER;
 if(!__sync_bool_compare_and_swap(&jpg_lock,0,1))return JPG_WAIT;
 if(_gz_jpg_report[3]){z=JPG_CORRUPT;goto done;}
 if(jpg_token){z=jpg_token==token&&jpg_thread==thread?_gz_jpg_check(token,frame):JPG_OWNER;goto done;}
 /* Close only an already empty admission set. No admitted native method is
  * suspended, and a busy result leaves admission open for clean retry. */
 jpg_token=token;jpg_thread=thread;jpg_mode=1;jpg_gpu_valid=0;__sync_synchronize();
 if(!__sync_bool_compare_and_swap(&jpg_admission,0,GATE)){
  jpg_token=jpg_thread=jpg_mode=0;z=JPG_WAIT;goto done;
 }
 _gz_jpg_report[9]++;__sync_synchronize();
 if(_gz_roh_gl_context(token,frame,1)!=1){jpg_fault(CHANGED);goto done;}
 z=_gz_jpg_check(token,frame);
done:jpg_publish();__sync_synchronize();jpg_lock=0;return z;
}
/* No native data is accessed through a caller-supplied pointer. This tests
 * only the exact nine-method coordination set. Missing producers/GPU lifetime
 * ownership remain explicit even after the actual GX2 wait returns true. */
static U jpg_capture_quiet(U mode,U frame){U i;
 if(jpg_mode!=2||!jpg_token||mode!=jpg_capture_mode||frame!=jpg_capture_frame||frame!=_gz_po_frame||
  _gz_hg_platform_thread()!=jpg_thread||!jpg_current())return JPG_OWNER;
 if(*(volatile U*)&jpg_admission!=GATE)return JPG_WAIT;
 __sync_synchronize();
 for(i=0;i<JPG_CALLS;i++)if(jpg_calls[i].cookie)return JPG_CORRUPT;
 for(i=0;i<JPG_METHODS;i++)if(jpg_entered[i]!=jpg_returned[i])return JPG_CORRUPT;
 return _gz_jpg_report[3]?JPG_CORRUPT:0;
}
U _gz_jpg_capture_check(U mode,U frame){U z=jpg_capture_quiet(mode,frame);jpg_publish();return z;}
U _gz_jpg_capture_begin(U mode,U frame){U cookie,thread,z=JPG_OWNER;
 if((mode!=1&&mode!=2&&mode!=3)||frame!=_gz_po_frame||jpg_gpu_entered||!jpg_heap_open()||!jpg_idle())return JPG_OWNER;
 _gz_jpg_poll();
 cookie=jpg_capture_entry(mode,frame);thread=_gz_hg_platform_thread();
 if(!cookie||!thread)return JPG_OWNER;
 if(!__sync_bool_compare_and_swap(&jpg_lock,0,1))return JPG_WAIT;
 if(jpg_token||jpg_mode||_gz_jpg_report[3]){z=JPG_CORRUPT;goto done;}
 jpg_token=cookie;jpg_thread=thread;jpg_mode=2;jpg_capture_mode=mode;jpg_capture_frame=frame;jpg_gpu_valid=0;
 __sync_synchronize();
 if(!__sync_bool_compare_and_swap(&jpg_admission,0,GATE)){
  jpg_token=jpg_thread=jpg_mode=jpg_capture_mode=jpg_capture_frame=0;z=JPG_WAIT;goto done;
 }
 if(jpg_capture_entry(mode,frame)!=cookie){
  /* READ-only setup has not touched a native object. Cancel the exact gate
   * even if the source callback lost identity after successful acquisition. */
  if(__sync_bool_compare_and_swap(&jpg_admission,GATE,0))
   jpg_token=jpg_thread=jpg_mode=jpg_capture_mode=jpg_capture_frame=0;
  else jpg_fault(CHANGED);
  goto done;
 }
 z=jpg_capture_quiet(mode,frame);
 if(!z)jpg_captures++;
 else {
  /* Begin returns no ownership on error. Cancel a guard acquired above if
   * the final identity read changed, before the caller can mark it entered. */
  if(__sync_bool_compare_and_swap(&jpg_admission,GATE,0))
   jpg_token=jpg_thread=jpg_mode=jpg_capture_mode=jpg_capture_frame=0;
  else jpg_fault(CHANGED);
 }
done:jpg_publish();__sync_synchronize();jpg_lock=0;return z;
}
U _gz_jpg_capture_end(U mode,U frame){U i,z=0;
 if(jpg_mode!=2||!jpg_token||mode!=jpg_capture_mode||frame!=jpg_capture_frame||
  _gz_hg_platform_thread()!=jpg_thread||jpg_gpu_entered||!jpg_heap_open())return JPG_OWNER;
 if(!__sync_bool_compare_and_swap(&jpg_lock,0,1))return JPG_WAIT;
 if(*(volatile U*)&jpg_admission!=GATE){z=JPG_CORRUPT;goto done;}
 for(i=0;i<JPG_CALLS;i++)if(jpg_calls[i].cookie){z=JPG_CORRUPT;goto done;}
 /* Only this callback thread's READ-only guard is canceled. Callback/source
  * drift does not strand the gate, and no restore owner is ever released. */
 if(!__sync_bool_compare_and_swap(&jpg_admission,GATE,0)){z=JPG_CORRUPT;goto done;}
 jpg_token=jpg_thread=jpg_mode=jpg_capture_mode=jpg_capture_frame=jpg_gpu_valid=0;jpg_capture_ends++;
done:if(z)_gz_jpg_report[69]++;jpg_publish();__sync_synchronize();jpg_lock=0;return z;
}
/* Distinct GHS coordination mode3. No native method is admitted by
 * jpg_method_scope in this mode; all prior ROH/RKC/GAC rules stay intact. */
static U jpg_ghs_quiet(U req,U frame){U i;
 if(jpg_mode!=3||!req||jpg_token!=req||jpg_capture_frame!=frame||
  !jpg_thread||!_gz_ghs_particle_identity(req,frame,jpg_thread))return JPG_OWNER;
 if(*(volatile U*)&jpg_admission!=GATE)return JPG_WAIT;
 __sync_synchronize();
 for(i=0;i<JPG_CALLS;i++)if(jpg_calls[i].cookie)return JPG_CORRUPT;
 for(i=0;i<JPG_METHODS;i++)if(jpg_entered[i]!=jpg_returned[i])return JPG_CORRUPT;
 return _gz_jpg_report[3]?JPG_CORRUPT:0;
}
U _gz_jpg_ghs_pending(U req,U frame){return req&&jpg_mode==3&&jpg_token==req&&
 jpg_capture_frame==frame?1u:0u;}
U _gz_jpg_ghs_begin(U req,U frame){U z=JPG_OWNER,thread;
 if(!req||!frame||jpg_gpu_entered||!jpg_heap_open()||!jpg_idle()||
  _gz_ghs_particle_entry(req,frame)!=1)return JPG_OWNER;
 _gz_jpg_poll();thread=_gz_hg_platform_thread();if(!thread)return JPG_OWNER;
 if(!__sync_bool_compare_and_swap(&jpg_lock,0,1))return JPG_WAIT;
 if(jpg_token||jpg_mode){z=JPG_WAIT;goto done;}
 if(_gz_jpg_report[3]){z=JPG_CORRUPT;goto done;}
 jpg_token=req;jpg_thread=thread;jpg_mode=3;jpg_capture_mode=0;jpg_capture_frame=frame;jpg_gpu_valid=0;
 __sync_synchronize();
 if(!__sync_bool_compare_and_swap(&jpg_admission,0,GATE)){
  jpg_token=jpg_thread=jpg_mode=jpg_capture_mode=jpg_capture_frame=0;z=JPG_WAIT;goto done;
 }
 z=_gz_ghs_particle_entry(req,frame)==1?jpg_ghs_quiet(req,frame):JPG_OWNER;
 if(z){
  /* A failed begin either cancels its own empty gate or leaves pending1;
   * GHS then retains this exact physical acquisition for ordered cleanup. */
  if(__sync_bool_compare_and_swap(&jpg_admission,GATE,0))
   jpg_token=jpg_thread=jpg_mode=jpg_capture_mode=jpg_capture_frame=jpg_gpu_valid=0;
  else jpg_fault(CHANGED);
 }
done:jpg_publish();__sync_synchronize();jpg_lock=0;return z;
}
U _gz_jpg_ghs_end(U req,U frame){U i,z=JPG_OWNER;
 if(!_gz_jpg_ghs_pending(req,frame)||jpg_gpu_entered||!jpg_heap_open()||
  !jpg_thread||_gz_hg_platform_thread()!=jpg_thread||
  _gz_ghs_particle_releasing(req,frame,jpg_thread)!=1)return JPG_OWNER;
 if(!__sync_bool_compare_and_swap(&jpg_lock,0,1))return JPG_WAIT;
 if(*(volatile U*)&jpg_admission!=GATE){z=JPG_CORRUPT;goto done;}
 for(i=0;i<JPG_CALLS;i++)if(jpg_calls[i].cookie){z=JPG_CORRUPT;goto done;}
 if(!__sync_bool_compare_and_swap(&jpg_admission,GATE,0)){z=JPG_CORRUPT;goto done;}
 jpg_token=jpg_thread=jpg_mode=jpg_capture_mode=jpg_capture_frame=jpg_gpu_valid=0;z=0;
done:jpg_publish();__sync_synchronize();jpg_lock=0;return z;
}
static U jpg_heap_open(void){const volatile U*c=_gz_hg_control;
 return c[0]==0x48474331u&&c[1]==64&&c[2]==1&&!c[3]&&!(c[4]&GATE)&&!c[7];}
extern U _gz_scf_gpu_scope(U,U,U);
static U jpg_gpu_context(U kind,U owner,U frame){
 if(!jpg_heap_open()||jpg_gpu_entered!=1||frame!=_gz_po_frame)return 0;
 if(kind==1)return jpg_mode==1&&jpg_token==owner&&_gz_jpg_check(owner,frame)==0&&
  ((_gz_roh_owner[3]==1&&!_gz_roh_owner[6]&&_gz_roh_owner[12]==1&&_gz_roh_gl_context(owner,frame,1)==1)||
   jpg_native_scope(_gz_hg_platform_thread())||
   _gz_scf_gpu_scope(owner,frame,_gz_hg_platform_thread())==1);
 if(kind==3)return jpg_mode==3&&jpg_token==owner&&jpg_ghs_quiet(owner,frame)==0&&
  _gz_ghs_particle_entry(owner,frame)==1;
 return kind==2&&jpg_mode==2&&jpg_token==owner&&jpg_capture_quiet(jpg_capture_mode,frame)==0&&
  jpg_capture_entry(jpg_capture_mode,frame)==owner;
}
static U jpg_gpu_exports(void){int z;
 if(jpg_gpu_submitted&&jpg_gpu_retired&&jpg_gpu_wait_stamp)return 0;
 if(!jpg_gpu_module){z=OSDynLoad_Acquire("gx2.rpl",&jpg_gpu_module);
  if(z||!jpg_gpu_module){_gz_jpg_report[67]=(U)z;return 1692;}}
 z=OSDynLoad_FindExport(jpg_gpu_module,0,"GX2GetLastSubmittedTimeStamp",&jpg_gpu_submitted);
 if(!z)z=OSDynLoad_FindExport(jpg_gpu_module,0,"GX2GetRetiredTimeStamp",&jpg_gpu_retired);
 if(!z)z=OSDynLoad_FindExport(jpg_gpu_module,0,"GX2WaitTimeStamp",&jpg_gpu_wait_stamp);
 _gz_jpg_report[67]=(U)z;
 return z||!(U)jpg_gpu_submitted||!(U)jpg_gpu_retired||!(U)jpg_gpu_wait_stamp||
  (((U)jpg_gpu_submitted|(U)jpg_gpu_retired|(U)jpg_gpu_wait_stamp)&3)?1692:0;
}
static void jpg_gpu_stamp(U word,T stamp){_gz_jpg_report[word]=(U)(stamp>>32);_gz_jpg_report[word+1]=(U)stamp;}
static U jpg_gpu_wait(U kind,U owner,U frame){U z=JPG_OWNER,result,waited;T before,submitted,retired;
 if(jpg_gpu_entered||!owner||!jpg_thread||_gz_hg_platform_thread()!=jpg_thread)return JPG_OWNER;
 jpg_gpu_entered=1;
 if(!jpg_gpu_context(kind,owner,frame))goto done;
 jpg_gpu_valid=0;
 z=jpg_gpu_exports();if(z)goto done;
 before=jpg_gpu_submitted();jpg_gpu_stamp(79,before);
 if(!jpg_gpu_context(kind,owner,frame)){z=JPG_OWNER;goto done;}
 _gz_jpg_report[59]++;jpg_publish();
 result=GX2DrawDone();_gz_jpg_report[61]=result;
 /* Cemu 2.6 exports DrawDone as void, while Cafe/WUT declares BOOL. Keep
  * its raw result for diagnosis; prove completion with the actual module's
  * explicit timestamp APIs on BOTH targets, never an emulator-only success. */
 if(!jpg_gpu_context(kind,owner,frame)){z=JPG_OWNER;goto done;}
 submitted=jpg_gpu_submitted();jpg_gpu_stamp(81,submitted);
 if(!submitted||submitted<before){z=1691;goto done;}
 waited=jpg_gpu_wait_stamp(submitted);_gz_jpg_report[66]=waited;
 retired=jpg_gpu_retired();jpg_gpu_stamp(83,retired);
 if(!waited||retired<submitted){z=1691;goto done;}
 if(!jpg_gpu_context(kind,owner,frame)){z=JPG_OWNER;goto done;}
 jpg_gpu_owner_mode=kind;jpg_gpu_owner=owner;jpg_gpu_frame=frame;jpg_gpu_valid=1;
 _gz_jpg_report[60]++;z=0;
done:jpg_gpu_entered=0;jpg_publish();return z;
}
U _gz_jpg_gpu_restore(U token,U frame){return jpg_gpu_wait(1,token,frame);}
U _gz_jpg_capture_gpu(U mode,U frame){
 if(jpg_mode!=2||mode!=jpg_capture_mode||frame!=jpg_capture_frame)return JPG_OWNER;
 return jpg_gpu_wait(2,jpg_token,frame);
}
U _gz_jpg_gpu_restore_ready(U token,U frame){return
 jpg_gpu_valid&&jpg_gpu_owner_mode==1&&jpg_gpu_owner==token&&jpg_gpu_frame==frame&&
 !_gz_jpg_check(token,frame)?0:JPG_OWNER;}
U _gz_jpg_capture_gpu_ready(U mode,U frame){return
 jpg_gpu_valid&&jpg_gpu_owner_mode==2&&jpg_gpu_owner==jpg_token&&jpg_gpu_frame==frame&&
 !jpg_capture_quiet(mode,frame)?0:JPG_OWNER;}
U _gz_jpg_ghs_gpu(U req,U frame){
 if(!_gz_jpg_ghs_pending(req,frame)||jpg_ghs_quiet(req,frame))return JPG_OWNER;
 return jpg_gpu_wait(3,req,frame);
}
U _gz_jpg_ghs_check(U req,U frame){return
 jpg_gpu_valid&&jpg_gpu_owner_mode==3&&jpg_gpu_owner==req&&jpg_gpu_frame==frame&&
 jpg_ghs_quiet(req,frame)==0?0:JPG_OWNER;
}
extern U _gz_jpg_calc_original(U,U),_gz_jpg_setup_original(U,U,U),_gz_jpg_substep_original(U,U,U);
extern U _gz_jpg_after_original(U,U,U),_gz_jpg_drawstep_original(U,U,U),_gz_jpg_draw_original(U,U,U);
extern U _gz_jpg_create_original(U,U,U,U,U,U,U),_gz_jex_remove_wrapper(U,U),_gz_jpg_debug_original(U);
U _gz_jpg_calc(U m,U g){U c=jpg_before(0,m,0,0,0,0),r;if(!c)return 0;r=_gz_jpg_calc_original(m,g);jpg_after(c);return r;}
#define JPG_THREE(name,kind) U _gz_jpg_##name(U m,U i,U g){U c=jpg_before(kind,m,0,0,0,0),r;if(!c)return 0;r=_gz_jpg_##name##_original(m,i,g);jpg_after(c);return r;}
JPG_THREE(setup,1)
JPG_THREE(substep,2)
JPG_THREE(after,3)
JPG_THREE(drawstep,4)
JPG_THREE(draw,5)
U _gz_jpg_create(U m,U p,U id,U g,U rm,U ec,U pc){U c=jpg_before(6,m,0,id,g,rm),r;
 r=_gz_jpg_create_original(m,p,id,g,rm,ec,pc);jpg_after(c);return r;}
/* One composed entry, not a second overlapping native removal hook. */
U _gz_jpg_remove(U m,U e){U c=jpg_before(7,m,e,0,0,0),r;r=_gz_jex_remove_wrapper(m,e);jpg_after(c);return r;}
U _gz_jpg_debug(U m){U c=jpg_before(8,m,0,0,0,0),r;if(!c)return 0;r=_gz_jpg_debug_original(m);jpg_after(c);return r;}

/* Read-only genuine one-create witness. The recorded call was admitted
 * by jpg_before with the actual native ABI arguments; no nested collective
 * method is admitted while COUNT1. Recheck the same physical provider here. */
U _gz_jpg_owned_create(U token,U frame,U manager){U i,n=0,thread=_gz_hg_platform_thread();
 if(!token||token!=jpg_token||frame!=_gz_po_frame||jpg_mode!=1||thread!=jpg_thread||
  *(volatile U*)&jpg_admission!=(GATE|1u))return 0;
 for(i=0;i<JPG_CALLS;i++)if(jpg_calls[i].cookie){
  if(jpg_calls[i].cookie==BAD||jpg_calls[i].kind!=6||jpg_calls[i].thread!=thread||
   jpg_calls[i].manager!=manager||n++)return 0;
 }
 return n==1&&jpg_method_scope(6,manager,0,thread,1,0,0,0)&&
  *(volatile U*)&jpg_admission==(GATE|1u)&&thread==_gz_hg_platform_thread();
}
