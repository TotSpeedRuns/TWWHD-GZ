extern u32 _gz_dck_access_entered(void);
/* Reversible GameOver process quiescence, NOT a complete death restore.
 * Only the fresh, generation-bound process483 state byte at+110 is owned.
 * State5 makes its native Execute return without advancing Game Over. The
 * Link death routine, UI, camera, audio and process lifetime are separate
 * participants. They are deliberately neither cleared nor retired here.
 *
 * The diagnostic command path runs on the real frame thread under HG. It is
 * separate from root READ/writer domains and grants no caller-selected native
 * destination. Its private before-image survives every returned dirty error.
 */
typedef struct {
 u32 magic,bytes,version,valid,nonce,frame,thread,epoch,stage0,stage1,room,layer;
 PoEntry link,gameover;
 u32 before_state,before_anime,before_timer,before_delay,demo,event;
} GqPlan;
typedef struct {u32 magic,bytes,valid,phase,nonce,attempted,applied_frame,undone_frame,error,seal;} GqRun;
enum {GQ_PREPARED=1,GQ_APPLYING=2,GQ_APPLIED=3,GQ_UNDOING=4,GQ_ROLLED=5,
 GQ_CONTEXT=640,GQ_BINDING=641,GQ_VALUE=642,GQ_CHANGED=643,GQ_TIMEOUT=644,GQ_WAIT=645};
typedef char GqPlanSize[sizeof(GqPlan)==200?1:-1];
static GqPlan _gz_gqk_plan;
static GqRun _gz_gqk_run;
static u32 _gz_gqk_seal,_gz_gqk_run_seal;
static u32 _gz_gqk_entered,_gz_gqk_mode,_gz_gqk_frame,_gz_gqk_thread,_gz_gqk_serial,_gz_gqk_gate;
static u32 _gz_gqk_wait_frame,_gz_gqk_wait_nonce;
volatile u32 _gz_gqk_quiesce_request,_gz_gqk_quiesce_ack,_gz_gqk_resume_request,_gz_gqk_resume_ack;
u32 _gz_gqk_report[32]={0x47514b31u,128};
extern u32 _gz_srh_any_access_entered(void);
extern u32 _gz_hg_platform_thread(void),_gz_hg_capture_begin(u32),_gz_hg_capture_seal(u32),_gz_hg_capture_end(u32);
static u32 gq_quiet(void){return !_gz_pbac_pending&&!*(volatile u8*)&_gz_pbac_loading&&
 !_gz_pbac_tools&&!_gz_pbac_world_resume_pending&&_gz_pbac_menu<=1&&
 !_gz_roh_user_guard()&&!_gz_osa_pin_id()&&_gz_pbac_idle_for_capture();}
u32 _gz_gqk_access_entered(void){return _gz_gqk_entered;}
/* HG verifies this actual callback/thread context independently. No root
 * binding or root-byte access is authorized by this process-only domain. */
u32 _gz_gqk_context_owned(u32 cookie){u32 req;
 if(_gz_gqk_entered!=1||!cookie||cookie!=_gz_gqk_serial||_gz_gqk_frame!=_gz_pbac_frame||
  !_gz_gqk_thread||_gz_gqk_thread!=_gz_hg_platform_thread()||_gz_srh_any_access_entered()||!gq_quiet())return 0;
 if(_gz_gqk_mode==1){req=_gz_gqk_quiesce_request;return req&&req==_gz_gqk_quiesce_ack+1;}
 if(_gz_gqk_mode==2){req=_gz_gqk_resume_request;return req&&req==_gz_gqk_resume_ack+1&&req==_gz_gqk_quiesce_ack;}
 return 0;
}
static u32 gq_scope(void){return _gz_gqk_context_owned(_gz_gqk_serial)&&_gz_gqk_gate&&!_gz_hg_capture_seal(_gz_gqk_gate);}
static u32 gq_apart(u32 a,u32 n){return span(a,n)&&
 !(a<(u32)&_gz_gqk_plan+sizeof(_gz_gqk_plan)&&(u32)&_gz_gqk_plan<a+n)&&
 !(a<(u32)&_gz_gqk_run+sizeof(_gz_gqk_run)&&(u32)&_gz_gqk_run<a+n)&&
 !(a<(u32)_gz_gqk_report+128&&(u32)_gz_gqk_report<a+n);}
static void gq_run_seal(void){_gz_gqk_run_seal=btr_hash(&_gz_gqk_run,sizeof(_gz_gqk_run));}
static u32 gq_sealed(void){const GqPlan*p=&_gz_gqk_plan;const GqRun*r=&_gz_gqk_run;
 return p->magic==0x47515031u&&p->bytes==sizeof(*p)&&p->version==1&&p->valid==1&&p->nonce&&
  _gz_gqk_seal==btr_hash(p,sizeof(*p))&&r->magic==0x47515231u&&r->bytes==sizeof(*r)&&r->valid==1&&
  r->nonce==p->nonce&&r->phase>=GQ_PREPARED&&r->phase<=GQ_ROLLED&&r->attempted<=1&&
  r->seal==_gz_gqk_seal&&_gz_gqk_run_seal==btr_hash(r,sizeof(*r));}
static u32 gq_current(PoEntry*l,PoEntry*g){u32 a=_gz_pbac_player,id,z;
 if(!gq_scope())return GQ_CONTEXT;
 if(_gz_pba_native_create_queue||_gz_pba_native_delete_queue)return GQ_WAIT;
 if(!gq_apart(a,0x8284))return GQ_WAIT;
 z=dto_process(a,168,0x8284,l);if(z)return GQ_BINDING;
 if(R8(a,12)!=2||R32(a,0x65f0)!=178)return GQ_WAIT;
 if(R32(a,0x65f4)!=65535||R32(a,0x65f8)!=0x02422108u||
  R32((u32)_gz_lpc_native_procedure_table+178*12,0)!=65535||
  R32((u32)_gz_lpc_native_procedure_table+178*12,4)!=0x02422108u||R16(a,0x420)!=5)return GQ_BINDING;
 id=R32(a,0x6a80);if(id==BAD)return GQ_WAIT;
 /* The shared read helper deliberately leaves an absent result untouched.
  * Never let a previous stack value stand in for a fresh process lookup. */
 eks_zero(g,sizeof(*g));z=dto_gameover(id,g);
 if(z||!g->object||g->pid!=id||!gq_apart(g->object,0x114)||R8(g->object,12)!=2)return GQ_BINDING;
 return 0;
}
static u32 gq_binding(void){const GqPlan*p=&_gz_gqk_plan;PoEntry l,g;u32 z=gq_current(&l,&g);
 if(z)return z;
 if(!btr_equal(&l,&p->link,sizeof(l))||!btr_equal(&g,&p->gameover,sizeof(g))||p->epoch!=_gz_pbac_epoch||
  p->stage0!=_gz_pbac_stage[0]||p->stage1!=_gz_pbac_stage[1]||p->room!=_gz_pbac_room||p->layer!=_gz_pbac_layer||
  p->thread!=_gz_gqk_thread||p->event!=_gz_pbac_event)return GQ_CHANGED;
 return 0;
}
static u32 gq_prepare(void){GqPlan*p=&_gz_gqk_plan;GqRun*r=&_gz_gqk_run;PoEntry l,g;PoHeader h;u32 a,z,state;
 if(!gq_scope()||p->valid||r->valid||_gz_gqk_seal||_gz_gqk_run_seal)return GQ_CONTEXT;
 z=dto_header(&h);if(z)return GQ_BINDING;z=gq_current(&l,&g);if(z)return z;a=g.object;
 state=R8(a,0x110);if((state!=0&&state!=3&&state!=4)||R8(a,0x111)>1)return GQ_VALUE;
 eks_zero(p,sizeof(*p));p->magic=0x47515031u;p->bytes=sizeof(*p);p->version=1;
 p->nonce=_gz_gqk_quiesce_request;p->frame=_gz_gqk_frame;p->thread=_gz_gqk_thread;p->epoch=_gz_pbac_epoch;
 p->stage0=_gz_pbac_stage[0];p->stage1=_gz_pbac_stage[1];p->room=_gz_pbac_room;p->layer=_gz_pbac_layer;
 btr_copy(&p->link,&l,sizeof(l));btr_copy(&p->gameover,&g,sizeof(g));
 p->before_state=state;p->before_anime=R8(a,0x111);p->before_timer=R16(a,0x10c);p->before_delay=R16(a,0x10e);
 p->demo=5;p->event=_gz_pbac_event;z=gq_binding();if(z)return z;
 if(!btr_equal(&h,&_gz_po_header,sizeof(h))||R8(a,0x110)!=state)return GQ_CHANGED;
 p->valid=1;_gz_gqk_seal=btr_hash(p,sizeof(*p));eks_zero(r,sizeof(*r));
 r->magic=0x47515231u;r->bytes=sizeof(*r);r->valid=1;r->phase=GQ_PREPARED;r->nonce=p->nonce;r->seal=_gz_gqk_seal;gq_run_seal();
 return 0;
}
static u32 gq_apply(void){GqPlan*p=&_gz_gqk_plan;GqRun*r=&_gz_gqk_run;u32 z,a;
 if(!gq_scope()||!gq_sealed()||r->phase!=GQ_PREPARED||r->attempted)return GQ_CONTEXT;
 z=gq_binding();if(z)return z;a=p->gameover.object;if(R8(a,0x110)!=p->before_state)return GQ_CHANGED;
 /* The attempted marker is sealed BEFORE the sole native byte store. */
 r->phase=GQ_APPLYING;r->attempted=1;r->applied_frame=_gz_pbac_frame;gq_run_seal();
 *(volatile u8*)(a+0x110)=5;__asm__ volatile("sync":::"memory");
 z=gq_binding();if(z)return z;if(R8(a,0x110)!=5)return GQ_CHANGED;
 r->phase=GQ_APPLIED;gq_run_seal();return 0;
}
static u32 gq_undo(void){GqPlan*p=&_gz_gqk_plan;GqRun*r=&_gz_gqk_run;u32 z,a;
 if(!gq_scope()||!gq_sealed()||r->phase==GQ_ROLLED)return GQ_CONTEXT;
 z=gq_binding();if(z)return z;a=p->gameover.object;
 if(r->attempted){
  if(R8(a,0x110)!=5&&R8(a,0x110)!=p->before_state)return GQ_CHANGED;
  r->phase=GQ_UNDOING;gq_run_seal();*(volatile u8*)(a+0x110)=(u8)p->before_state;
  __asm__ volatile("sync":::"memory");z=gq_binding();if(z)return z;
 }
 if(R8(a,0x110)!=p->before_state)return GQ_CHANGED;
 r->phase=GQ_ROLLED;r->undone_frame=_gz_pbac_frame;r->error=0;gq_run_seal();return 0;
}
/* Single nonyielding access domain. Cookie/thread/frame and allocator
 * exclusion are independent of the saved/current journal contents. */
static u32 gq_access(u32 mode){u32 gate=0,z=GQ_CONTEXT,end;
 if(_gz_dck_access_entered()||_gz_gqk_entered||_gz_srh_any_access_entered()||!gq_quiet()||_gz_gqk_serial==BAD)return z;
 _gz_gqk_serial++;_gz_gqk_entered=1;_gz_gqk_mode=mode;_gz_gqk_frame=_gz_pbac_frame;_gz_gqk_thread=_gz_hg_platform_thread();
 if(!_gz_gqk_context_owned(_gz_gqk_serial))goto done;
 gate=_gz_hg_capture_begin(_gz_gqk_serial);if(!gate){z=86;goto done;}_gz_gqk_gate=gate;
 z=_gz_hg_capture_seal(gate);if(z)goto done;
 if(mode==1){z=gq_prepare();if(!z)z=gq_apply();}else if(mode==2)z=gq_undo();else z=GQ_CONTEXT;
 if(!z)z=_gz_hg_capture_seal(gate);
done:
 if(gate){end=_gz_hg_capture_end(gate);if(!z)z=end;}
 _gz_gqk_gate=_gz_gqk_mode=_gz_gqk_entered=0;
 if(gq_sealed()){_gz_gqk_run.error=z;gq_run_seal();}
 return z;
}
void _gz_gqk_step(void){u32 req=_gz_gqk_quiesce_request,undo=_gz_gqk_resume_request,z;
 if(_gz_gqk_entered)return;
 if(undo&&undo!=_gz_gqk_resume_ack){
  if(undo!=_gz_gqk_resume_ack+1||undo!=_gz_gqk_quiesce_ack){_gz_gqk_report[6]=GQ_CONTEXT;return;}
  if(!_gz_gqk_plan.valid&&!_gz_gqk_run.valid&&!_gz_gqk_run.attempted&&!_gz_gqk_seal&&!_gz_gqk_run_seal&&gq_quiet()){
   _gz_gqk_report[14]=1;__asm__ volatile("sync":::"memory");_gz_gqk_resume_ack=undo;_gz_gqk_report[5]=undo;return;
  }
  if(!gq_sealed()){_gz_gqk_report[6]=GQ_CONTEXT;return;}
  z=gq_access(2);_gz_gqk_report[6]=z;if(z&&!_gz_gqk_report[15])_gz_gqk_report[15]=z;
  _gz_gqk_report[3]=_gz_gqk_run.phase;_gz_gqk_report[10]=_gz_pbac_frame;
  if(!z){__asm__ volatile("sync":::"memory");_gz_gqk_resume_ack=undo;_gz_gqk_report[5]=undo;}return;
 }
 if(!req||req==_gz_gqk_quiesce_ack)return;
 if(req!=_gz_gqk_quiesce_ack+1||req!=_gz_gqk_resume_ack+1){_gz_gqk_report[6]=GQ_CONTEXT;return;}
 if(_gz_gqk_wait_nonce!=req){_gz_gqk_wait_nonce=req;_gz_gqk_wait_frame=_gz_pbac_frame;}
 if(_gz_pbac_frame-_gz_gqk_wait_frame>900){_gz_gqk_report[6]=GQ_TIMEOUT;_gz_gqk_quiesce_ack=req;return;}
 if(!gq_quiet())return;
 if(_gz_gqk_plan.valid){
  if(!gq_sealed()||_gz_gqk_run.phase!=GQ_ROLLED){_gz_gqk_report[6]=GQ_CONTEXT;return;}
  eks_zero(&_gz_gqk_plan,sizeof(_gz_gqk_plan));eks_zero(&_gz_gqk_run,sizeof(_gz_gqk_run));_gz_gqk_seal=_gz_gqk_run_seal=0;
 }
 z=gq_access(1);if(z==GQ_WAIT){_gz_gqk_report[13]=z;return;}
 _gz_gqk_report[2]=req;_gz_gqk_report[3]=_gz_gqk_run.phase;_gz_gqk_report[6]=z;if(z&&!_gz_gqk_report[15])_gz_gqk_report[15]=z;
 _gz_gqk_report[7]=_gz_pbac_frame;_gz_gqk_report[8]=_gz_gqk_plan.gameover.object;_gz_gqk_report[9]=_gz_gqk_plan.gameover.pid;
 _gz_gqk_report[11]=_gz_gqk_run.attempted;_gz_gqk_report[12]=_gz_gqk_plan.before_state;
 __asm__ volatile("sync":::"memory");_gz_gqk_quiesce_ack=req;_gz_gqk_report[4]=req;
}
