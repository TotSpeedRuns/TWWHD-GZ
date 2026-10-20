/* Private CLEAN/OBSERVE death hold, not death restoration. The one exception
 * to the ordinary event0 owner sampler is exact current event3, procedure178,
 * demo5 and a fresh unique LIVE GameOver483. The saved archive role stays
 * unchanged. No actor, UI, camera, source or native process byte is written.
 *
 * Initial identity capture owns the real nonyielding frame/HG exclusion.
 * Subsequent owner queries use the original sampler's nonyielding frame read
 * boundary, revalidate both full process generations before native reads and
 * permit only CLEAN/OBSERVE. No SDK/lifetime/mutating permission is granted.
 */
typedef struct {
 u32 magic,bytes,valid,nonce,frame,thread,epoch,stage0,stage1,room,layer,event;
 u32 slot,expected_token,UI_root,reserved;
 PoEntry link,gameover;
 u32 fade,timer,delay,state,anime;
} DohPlan;
typedef char DohPlanSize[sizeof(DohPlan)==212?1:-1];
enum {DOH_CONTEXT=660,DOH_BINDING=661,DOH_CHANGED=662,DOH_WAIT=663,DOH_TIMEOUT=664};
extern u32 _gz_dck_step(u32),_gz_dck_owner_role(u32,u32,u32),_gz_dck_access_entered(void);
static DohPlan _gz_doh_plan;
static u32 _gz_doh_seal,_gz_doh_active,_gz_doh_token,_gz_doh_claiming,_gz_doh_wait;
static u32 _gz_doh_entered,_gz_doh_thread,_gz_doh_frame,_gz_doh_serial,_gz_doh_gate;
volatile u32 _gz_doh_request,_gz_doh_ack;
u32 _gz_doh_report[32]={0x444f4831u,128};
extern volatile u32 _gz_roh_serial,_gz_ps_ui_state_root;
extern u32 _gz_roh_gl_context(u32,u32,u32);
extern u32 _gz_lpc_probe_current(u32,u32);
static u32 doh_idle_owner(void){return _gz_roh_owner[0]==0x524f4831u&&_gz_roh_owner[1]==240&&
 _gz_roh_owner[2]==2&&!_gz_roh_owner[3]&&!_gz_roh_owner[4]&&!_gz_roh_owner[5]&&
 !_gz_roh_owner[6]&&!_gz_roh_owner[7]&&!_gz_roh_owner[12]&&!_gz_roh_owner[15];}
u32 _gz_doh_access_entered(void){return _gz_doh_entered;}
u32 _gz_doh_context_owned(u32 cookie){return _gz_doh_entered==1&&cookie&&cookie==_gz_doh_serial&&
 _gz_doh_frame==_gz_pbac_frame&&_gz_doh_thread&&_gz_doh_thread==_gz_hg_platform_thread()&&
 !_gz_srh_any_access_entered()&&!_gz_gqk_access_entered()&&!_gz_dck_access_entered()&&gq_quiet()&&
 _gz_doh_active&&_gz_doh_active==_gz_doh_request&&_gz_doh_request==_gz_doh_ack+1;}
static u32 doh_scope(void){return _gz_doh_context_owned(_gz_doh_serial)&&_gz_doh_gate&&
 !_gz_hg_capture_seal(_gz_doh_gate);}
static u32 doh_header(const DohPlan*p){return p->magic==0x444f5031u&&p->bytes==sizeof(*p)&&
 p->valid==1&&p->nonce==_gz_doh_active&&p->nonce==_gz_doh_request&&p->nonce==_gz_doh_ack+1&&
 p->slot<=9&&p->expected_token&&p->event==3&&!p->reserved&&_gz_doh_seal==btr_hash(p,sizeof(*p));}
static u32 doh_current(PoEntry*l,PoEntry*g,u32*role){u32 a=_gz_pbac_player,id,z;
 *role=0;
 if(_gz_pba_native_create_queue||_gz_pba_native_delete_queue)return DOH_WAIT;
 z=dto_process(a,168,0x8284,l);if(z)return DOH_BINDING;
 if(R8(a,12)!=2)return DOH_WAIT;
 *role=_gz_dlr_current_role(a,l->pid);
 if(!*role){
 if(R32(a,0x65f0)!=178)return DOH_WAIT;
 if(R32(a,0x65f4)!=65535||R32(a,0x65f8)!=0x02422108u||
  R32((u32)_gz_lpc_native_procedure_table+178*12,0)!=65535||
  R32((u32)_gz_lpc_native_procedure_table+178*12,4)!=0x02422108u||R16(a,0x420)!=5)return DOH_BINDING;
 }
 id=R32(a,0x6a80);if(id==BAD)return DOH_WAIT;
 eks_zero(g,sizeof(*g));z=dto_gameover(id,g);
 if(z||!g->object||g->pid!=id||R8(g->object,12)!=2)return DOH_BINDING;
 if((R8(g->object,0x110)!=0&&R8(g->object,0x110)!=3&&R8(g->object,0x110)!=4)||R8(g->object,0x111)>1)return DOH_BINDING;
 return 0;
}
static u32 doh_binding(void){const DohPlan*p=&_gz_doh_plan;PoHeader h;PoEntry l,g;u32 z,a,role;
 if(!doh_header(p)||p->thread!=_gz_hg_platform_thread()||p->epoch!=_gz_pbac_epoch||
  p->stage0!=_gz_pbac_stage[0]||p->stage1!=_gz_pbac_stage[1]||p->room!=_gz_pbac_room||p->layer!=_gz_pbac_layer||
  p->event!=_gz_pbac_event||p->UI_root!=_gz_ps_ui_state_root||p->slot!=_gz_pbac_selected)return DOH_CHANGED;
 z=dto_header(&h);if(z)return DOH_BINDING;z=doh_current(&l,&g,&role);if(z)return z;
 if(!btr_equal(&l,&p->link,sizeof(l))||!btr_equal(&g,&p->gameover,sizeof(g)))return DOH_CHANGED;
 a=g.object;
 if((!role&&R32(l.object,0x69f8)!=p->fade)||R16(a,0x10c)!=p->timer||R16(a,0x10e)!=p->delay||
  R8(a,0x110)!=p->state||R8(a,0x111)!=p->anime||!btr_equal(&h,&_gz_po_header,sizeof(h)))return DOH_CHANGED;
 return 0;
}
u32 _gz_doh_event_guard(void){const DohPlan*p=&_gz_doh_plan;
 if(!_gz_doh_active||!doh_header(p)||_gz_pbac_event!=3||_gz_pbac_menu!=1||
  _gz_pbac_pending||*(volatile u8*)&_gz_pbac_loading||_gz_pbac_tools||_gz_pbac_world_resume_pending)return 0;
 if(_gz_doh_claiming){if(!doh_idle_owner()||_gz_doh_token||p->expected_token!=_gz_roh_serial+1||_gz_roh_serial==BAD)return 0;}
 else if(!_gz_doh_token||_gz_doh_token!=p->expected_token||_gz_roh_serial!=_gz_doh_token||
  _gz_roh_owner[0]!=0x524f4831u||_gz_roh_owner[1]!=240||_gz_roh_owner[2]!=2||
  _gz_roh_owner[4]!=1||_gz_roh_owner[5]!=_gz_doh_token||_gz_roh_owner[15])return 0;
 else if(_gz_roh_owner[3]!=1||_gz_roh_owner[6]||_gz_roh_owner[7]||_gz_roh_owner[12]>1){
  if(_gz_roh_owner[3]!=2||_gz_roh_owner[7]||!_gz_dck_owner_role(_gz_doh_token,_gz_pbac_frame,2))return 0;
 }
 return !doh_binding();
}
/* Exact current death only, independent of archive contents and camera mode.
 * The no-lifetime kernel owns its distinct sealed FAILED_DIRTY role. */
u32 _gz_doh_recovery_scope(u32 token,u32 frame){return token&&token==_gz_doh_token&&
 frame==_gz_pbac_frame&&doh_header(&_gz_doh_plan)&&_gz_pbac_event==3&&_gz_pbac_menu==1&&
 !_gz_pbac_pending&&!*(volatile u8*)&_gz_pbac_loading&&!_gz_pbac_tools&&!_gz_pbac_world_resume_pending&&
 _gz_dck_owner_role(token,frame,4)&&!doh_binding();}
/* Current-player read role only. Never a saved procedure, mutation, SDK,
 * lifecycle, recovery or release authorization. */
u32 _gz_doh_player_scope(u32 token,u32 frame,u32 actor,u32 pid){return token&&token==_gz_doh_token&&
 frame==_gz_pbac_frame&&actor==_gz_doh_plan.link.object&&pid==_gz_doh_plan.link.pid&&
 _gz_roh_owner[12]==1&&_gz_doh_event_guard();}
static u32 doh_prepare(void){DohPlan*p=&_gz_doh_plan;PoHeader h;PoEntry l,g;u32 z,a,role;
 if(!doh_scope()||!doh_idle_owner()||_gz_doh_token||_gz_pbac_menu!=1||_gz_pbac_event!=3||_gz_roh_serial==BAD)return DOH_CONTEXT;
 z=dto_header(&h);if(z)return DOH_BINDING;z=doh_current(&l,&g,&role);if(z)return z;if(role)return DOH_CONTEXT;
 eks_zero(p,sizeof(*p));p->magic=0x444f5031u;p->bytes=sizeof(*p);p->nonce=_gz_doh_active;
 p->frame=_gz_pbac_frame;p->thread=_gz_doh_thread;p->epoch=_gz_pbac_epoch;p->stage0=_gz_pbac_stage[0];
 p->stage1=_gz_pbac_stage[1];p->room=_gz_pbac_room;p->layer=_gz_pbac_layer;p->event=3;
 p->slot=_gz_pbac_selected;p->expected_token=_gz_roh_serial+1;p->UI_root=_gz_ps_ui_state_root;
 btr_copy(&p->link,&l,sizeof(l));btr_copy(&p->gameover,&g,sizeof(g));a=g.object;
 p->fade=R32(l.object,0x69f8);p->timer=R16(a,0x10c);p->delay=R16(a,0x10e);p->state=R8(a,0x110);p->anime=R8(a,0x111);
 if(!finite(p->fade)||p->fade>0x3f800000u||!btr_equal(&h,&_gz_po_header,sizeof(h)))return DOH_CHANGED;
 p->valid=1;_gz_doh_seal=btr_hash(p,sizeof(*p));return doh_binding();
}
static u32 doh_access(void){u32 z=DOH_CONTEXT,end,gate=0;
 if(_gz_doh_entered||_gz_dck_access_entered()||_gz_srh_any_access_entered()||_gz_gqk_access_entered()||!gq_quiet()||_gz_doh_serial==BAD)return z;
 _gz_doh_serial++;_gz_doh_entered=1;_gz_doh_thread=_gz_hg_platform_thread();_gz_doh_frame=_gz_pbac_frame;
 if(!_gz_doh_context_owned(_gz_doh_serial))goto done;
 gate=_gz_hg_capture_begin(_gz_doh_serial);if(!gate){z=86;goto done;}_gz_doh_gate=gate;
 z=_gz_hg_capture_seal(gate);if(z)goto done;z=doh_prepare();if(!z)z=_gz_hg_capture_seal(gate);
done:
 if(gate){end=_gz_hg_capture_end(gate);if(!z)z=end;}
 _gz_doh_gate=_gz_doh_entered=0;return z;
}
static void doh_finish(u32 z){u32 a=_gz_doh_token?_gz_roh_abort_clean(_gz_doh_token):0;
 _gz_doh_report[5]=z;_gz_doh_report[7]=a;_gz_doh_report[4]=_gz_pbac_frame;
 _gz_doh_report[17]=_gz_roh_owner[3];_gz_doh_report[18]=_gz_roh_owner[4];
 if(a)return; /* Keep a failed owner and its role; never fabricate release. */
 _gz_doh_token=0;__asm__ volatile("sync":::"memory");_gz_doh_ack=_gz_doh_active;_gz_doh_active=0;
}
void _gz_doh_step(void){u32 req=_gz_doh_request,frame=_gz_pbac_frame,z,end,i;
 if(!req||req==_gz_doh_ack)return;
 if(!_gz_doh_active){
  if(req!=_gz_doh_ack+1||_gz_pbac_selected>9||!gq_quiet()||_gz_pbac_menu!=1||_gz_pbac_event!=3)return;
  _gz_doh_active=req;_gz_doh_wait=frame;for(i=2;i<32;i++)_gz_doh_report[i]=0;
  _gz_doh_report[2]=req;_gz_doh_report[8]=_gz_pbac_selected;
 }
 if(!_gz_doh_token){
  if(frame-_gz_doh_wait>120){doh_finish(DOH_TIMEOUT);return;}
  z=doh_access();if(z==DOH_WAIT)return;if(z){doh_finish(z);return;}
  z=_gz_slots_install(_gz_pbac_selected);if(z){doh_finish(z);return;}
  _gz_doh_claiming=1;z=_gz_roh_begin(_gz_pbac_selected,60);_gz_doh_claiming=0;
  if(z){doh_finish(z);return;}
  _gz_doh_token=_gz_roh_owner[5];_gz_doh_report[6]=_gz_doh_token;_gz_doh_report[9]=_gz_osa_pin_id();_gz_doh_report[3]=frame;
  _gz_doh_report[13]=_gz_doh_plan.gameover.object;_gz_doh_report[14]=_gz_doh_plan.state;_gz_doh_report[15]=_gz_doh_plan.event;return;
 }
 if(_gz_roh_owner[10]==frame&&_gz_roh_owner[11]&&_gz_dck_step(_gz_doh_token))return;
 if(req!=_gz_doh_active||_gz_roh_owner[7]){doh_finish(_gz_roh_owner[7]?_gz_roh_owner[7]:28);return;}
 if(_gz_roh_owner[10]!=frame||!_gz_roh_owner[11]||_gz_doh_report[10]==frame)return;
 z=_gz_roh_service_begin(_gz_doh_token,1);_gz_doh_report[11]=z;if(z){doh_finish(z);return;}
 if(!_gz_doh_report[16]){
  z=_gz_lpc_probe_current(_gz_doh_token,frame);_gz_doh_report[19]=z;
  if(z){end=_gz_roh_service_end(_gz_doh_token,1,0);_gz_doh_report[12]=end;doh_finish(end?end:z);return;}
 }
 z=_gz_roh_gl_context(_gz_doh_token,frame,1)==1?0:42;
 end=_gz_roh_service_end(_gz_doh_token,1,z);_gz_doh_report[12]=end;if(z||end){doh_finish(z?z:end);return;}
 _gz_doh_report[10]=frame;_gz_doh_report[16]++;
 if(frame-_gz_doh_report[3]>=30)doh_finish(0);
}
