/* Private saved-camera replay with complete death-camera rollback. This is
 * NOT full death restoration.
 * Exact original EndEventCamera and dCamParam::Change execute under the real
 * MUTATING/camera service. Failure/rollback uses a no-lifetime owner ticket.
 * Only their audited flags/parameter-cache footprints are undone. Whole native
 * camera bytes and CameraInfo must equal the original before release.
 * Mode3 replays audited saved scalars/work/view/angles and fresh actor links.
 * No Link body, UI, event owner or process lifetime is replayed. */
extern u32 _gz_dck_native_end(u32,u32),_gz_dck_native_change(u32,int);
extern u32 _gz_roh_direct_recovery_begin(u32,u32),_gz_roh_direct_recovery_common(u32,u32);
extern u32 _gz_roh_direct_recovery_finish(u32,u32);
static u32 _gz_dck_code_valid(void){return btr_hash((const void*)0x0253e864u,60)==0x4b70d263u&&btr_hash((const void*)0x024f7280u,64)==0x46d8098bu;}
enum {DCK_PREPARED=1,DCK_ENDING=2,DCK_CHANGING=3,DCK_UNDOING=4,DCK_ROLLED=5,DCK_COPYING=6};
enum {DCK_CONTEXT=670,DCK_BINDING=671,DCK_SOURCE=672,DCK_IMAGE=673,DCK_API=674,DCK_TEST_FAILURE=679};
typedef struct {
 u32 magic,bytes,valid,phase,nonce,token,frame,thread,camera,body,index,link,link_pid;
 u32 before_style,saved_style,saved_algorithm,pin,source,source_bytes,groups;
 PoEntry process;
 u8 before[0xb28],info[12],saved[2048],after[0xb28],after_info[12];
 PoEntry before_refs[6],saved_refs[6];
} DckPlan;
typedef char DckPlanSize[sizeof(DckPlan)==8696?1:-1];
static DckPlan _gz_dck_plan;
DckPlan _gz_dck_clean_abort_plan;
static u32 _gz_dck_seal,_gz_dck_active,_gz_dck_entered,_gz_dck_serial,_gz_dck_thread,_gz_dck_frame,_gz_dck_gate;
static u32 _gz_dck_retired_token,_gz_dck_retired_seal;
volatile u32 _gz_dck_request,_gz_dck_ack,_gz_dck_mode=2;
u32 _gz_dck_report[48]={0x44434b31u,192};
u32 _gz_dck_closure_seal;
static void dck_seal(void){_gz_dck_seal=btr_hash(&_gz_dck_plan,sizeof(_gz_dck_plan));}
static u32 dck_sealed(void){const DckPlan*p=&_gz_dck_plan;
 return p->magic==0x44435031u&&p->bytes==sizeof(*p)&&p->valid==1&&p->token&&
  p->phase>=DCK_PREPARED&&p->phase<=DCK_COPYING&&p->nonce==_gz_dck_active&&
  p->nonce==_gz_dck_request&&p->nonce==_gz_dck_ack+1&&p->frame==_gz_dck_frame&&
  p->thread==_gz_dck_thread&&p->body==p->camera+0x248&&p->index<2&&(p->groups==0||p->groups==1||p->groups==3||p->groups==7)&&
  p->before_style<=144&&p->saved_style<=144&&p->saved_style!=p->before_style&&
  _gz_dck_seal==btr_hash(p,sizeof(*p));
}
u32 _gz_dck_access_entered(void){return _gz_dck_entered;}
/* Callback metadata only. Native allocations are validated AFTER real HG
 * entry; a cookie never grants pointer/lifetime or world-write authority. */
u32 _gz_dck_context_owned(u32 cookie){return _gz_dck_entered==1&&cookie&&cookie==_gz_dck_serial&&
 _gz_dck_frame==_gz_pbac_frame&&_gz_dck_thread&&_gz_dck_thread==_gz_hg_platform_thread()&&
 !_gz_srh_any_access_entered()&&!_gz_gqk_access_entered()&&!_gz_doh_access_entered()&&
 _gz_dck_active&&_gz_dck_active==_gz_dck_request&&_gz_dck_request==_gz_dck_ack+1&&
 _gz_doh_token&&_gz_doh_active&&doh_header(&_gz_doh_plan);}
/* Authentication only: original DOH independently checks current generations,
 * native dying-body tuple and GameOver/UI unchanged values around this scope. */
u32 _gz_dck_owner_role(u32 token,u32 frame,u32 phase){return dck_sealed()&&
 _gz_dck_context_owned(_gz_dck_serial)&&token==_gz_dck_plan.token&&frame==_gz_dck_frame&&
 (phase==2||phase==4)&&_gz_roh_owner[3]==phase&&_gz_roh_owner[4]==1&&
 _gz_roh_owner[5]==token&&_gz_roh_owner[6]==1&&_gz_roh_owner[12]<=10&&
 (_gz_roh_owner[12]==0||_gz_roh_owner[12]==5||(_gz_dck_mode>=4&&_gz_roh_owner[12]==2)||(_gz_dck_mode>=7&&_gz_roh_owner[12]==6)||(_gz_dck_mode>=8&&_gz_roh_owner[12]==10))&&!_gz_roh_owner[15];}
/* Fixed callback metadata; the body participant checks its own phase,
 * real typed service, source pin, fresh bindings and HG lease separately. */
__attribute__((noinline,noclone)) u32 _gz_dck_body_context(u32 token,u32 frame){return _gz_dck_mode>=4&&dck_sealed()&&
 _gz_dck_context_owned(_gz_dck_serial)&&token==_gz_dck_plan.token&&frame==_gz_dck_frame;}
__attribute__((noinline,noclone)) u32 _gz_dck_body_gate(u32 token,u32 frame){return _gz_dck_body_context(token,frame)&&
 _gz_dck_gate&&!_gz_hg_capture_seal(_gz_dck_gate)?_gz_dck_gate:0;}
u32 _gz_dck_state_context(u32 token,u32 frame){return _gz_dck_mode>=7&&_gz_dck_body_context(token,frame);}
u32 _gz_dck_state_gate(u32 token,u32 frame){return _gz_dck_mode>=7?_gz_dck_body_gate(token,frame):0;}
u32 _gz_dck_ui_context(u32 token,u32 frame){return _gz_dck_mode>=8&&_gz_dck_body_context(token,frame);}
u32 _gz_dck_ui_gate(u32 token,u32 frame){return _gz_dck_mode>=8?_gz_dck_body_gate(token,frame):0;}
static u32 dck_closure_value(void){u32 key[3],base=_gz_dck_mode>=4?
 _gz_dlr_closure_key(_gz_dck_plan.token,_gz_dck_frame,_gz_dck_seal):_gz_dck_seal;
 if(_gz_dck_mode<7)return base;if(!base)return 0;
 key[0]=0x44504331u;key[1]=base;key[2]=_gz_dpk_closure_key(_gz_dck_plan.token,_gz_dck_frame);
 if(!key[2])return 0;base=btr_hash(key,sizeof(key));if(_gz_dck_mode<8)return base;
 key[0]=0x44554343u;key[1]=base;key[2]=_gz_duc_closure_key(_gz_dck_plan.token,_gz_dck_frame);
 return key[2]?btr_hash(key,sizeof(key)):0;
}
static u32 dck_binding(void){const DckPlan*p=&_gz_dck_plan;PoEntry now;u32 c,index,z;
 if(!dck_sealed()||doh_binding())return DCK_BINDING;
 z=cct_current(&now,&c,&index);if(z||c!=p->camera||index!=p->index||
  !btr_equal(&now,&p->process,sizeof(now)))return DCK_BINDING;
 return 0;
}
#include "death_camera_typed.inc"
/* All bytes outside the TWO original native API footprints must be exact.
 * A returned partial failure may match either complete before/after value at
 * each written footprint, never arbitrary fields or a partially forged cache. */
static u32 dck_image(u32 exact,u32 groups){const DckPlan*p=&_gz_dck_plan;u32 i,n,flags,base=0x248;
 if(groups==7)return dct_after_image();
 const u8*b=p->before;u32 old_flags=R32(b,base+0x510),new_flags=old_flags&~0x20000000u;
 for(i=0;i<0xb28;i++){
  if(groups&1&&i>=base+0x510&&i<base+0x514)continue;
  if(groups&2&&i>=base+0x8a8&&i<base+0x8b0)continue;
  if(R8(p->camera,i)!=b[i])return DCK_IMAGE;
 }
 flags=R32(p->body,0x510);
 if(flags!=old_flags&&(!(groups&1)||flags!=new_flags))return DCK_IMAGE;
 if(exact==1&&flags!=new_flags)return DCK_IMAGE;
 if(exact==2&&flags!=old_flags)return DCK_IMAGE;
 n=R32(p->body,0x8ac);
 if(n!=p->before_style&&(!(groups&2)||n!=p->saved_style))return DCK_IMAGE;
 if(R32(p->body,0x8a8)!=0x1004487cu+n*132)return DCK_IMAGE;
 if(exact==1&&(groups&2)&&n!=p->saved_style)return DCK_IMAGE;
 if(exact==2&&n!=p->before_style)return DCK_IMAGE;
 if(!btr_equal((const void*)((u32)_gz_cct_native_info+p->index*52),p->info,12))return DCK_IMAGE;
 return 0;
}
static u32 dck_prepare(u32 token,u32 frame){DckPlan*p=&_gz_dck_plan;PoEntry now;u32 c,index,pin,s,n,off,z,i,v;
 if(_gz_dck_seal||p->valid||_gz_roh_gl_context(token,frame,1)!=1||!_gz_dck_gate)return DCK_CONTEXT;
 if(_gz_hg_capture_seal(_gz_dck_gate)||!_gz_dck_code_valid())return DCK_CONTEXT;
 pin=_gz_osa_pin_id();if(!pin||_gz_osa_validate(pin))return DCK_SOURCE;
 s=_gz_osa_source(pin);n=_gz_osa_bytes(pin);
 if(!s||s!=_gz_roh_owner[17]||n!=_gz_roh_owner[18]||n<216)return DCK_SOURCE;
 off=R32(s,48);if(R32(s,52)!=2048||off<216||(off&3)||off>n||2048>n-off)return DCK_SOURCE;
 z=cct_current(&now,&c,&index);if(z)return DCK_BINDING;
 eks_zero(p,sizeof(*p));p->magic=0x44435031u;p->bytes=sizeof(*p);p->phase=DCK_PREPARED;
 p->nonce=_gz_dck_active;p->token=token;p->frame=frame;p->thread=_gz_dck_thread;
 p->camera=c;p->body=c+0x248;p->index=index;p->link=_gz_doh_plan.link.object;p->link_pid=_gz_doh_plan.link.pid;
 p->pin=pin;p->source=s;p->source_bytes=n;btr_copy(&p->process,&now,sizeof(now));
 btr_copy(p->before,(const void*)c,sizeof(p->before));
 btr_copy(p->info,(const void*)((u32)_gz_cct_native_info+index*52),12);
 btr_copy(p->saved,(const void*)(s+off),2048);
 p->before_style=R32(p->body,0x514);p->saved_style=R32(p->saved,20);p->saved_algorithm=R32(p->saved,24);
 if(p->before_style!=4||_gz_cct_native_styles[4*33]!=11||
  p->saved_style>144||p->saved_style==4||!cct_work(p->saved_algorithm)||
  _gz_cct_native_styles[p->saved_style*33]!=p->saved_algorithm||
  R32(p->saved,0)!=1||R32(p->saved,8)!=c||R32(p->saved,12)!=now.pid||R32(p->saved,16)!=index||
  _gz_camera_tower_safe(p->saved)||R32(p->body,0x8a4)!=0x10044840u||
  R32(p->body,0x8a8)!=0x1004487cu+4*132||R32(p->body,0x8ac)!=4||
  !(R32(p->body,0x510)&0x20000000u)||R32(p->body,0x410)!=p->link_pid||
  R32(p->body,0x414)!=9)return DCK_BINDING;
 v=R32(p->body,0x37c);if(v>5&&(v<0x32||v>0x34))return DCK_BINDING;
 if(R32(p->body,0x384)>1)return DCK_BINDING;
 for(i=0;i<6;i++)if(!finite(R32(p->body,0x388+i*4)))return DCK_BINDING;
 for(i=0;i<13;i++)if(!finite(R32(p->saved,1400+i*4)))return DCK_SOURCE;
 if(_gz_roh_gl_context(token,frame,1)!=1||_gz_osa_validate(pin)||_gz_osa_pin_id()!=pin||
  !btr_equal(p->saved,(const void*)(s+off),2048)||_gz_hg_capture_seal(_gz_dck_gate))return DCK_SOURCE;
 z=dct_prepare();if(z)return z;
 p->valid=1;dck_seal();return dck_binding();
}
u32 _gz_dck_recovery_scope(u32 token){return dck_sealed()&&token==_gz_dck_plan.token&&
 _gz_dck_owner_role(token,_gz_pbac_frame,4)&&!dck_binding()&&!dck_image(0,_gz_dck_plan.groups);}
u32 _gz_dck_finish_scope(u32 token,u32 frame){return dck_sealed()&&_gz_dck_plan.phase==DCK_ROLLED&&
 token==_gz_dck_plan.token&&frame==_gz_dck_plan.frame&&_gz_dck_closure_seal&&_gz_dck_closure_seal==dck_closure_value()&&
 _gz_dck_owner_role(token,frame,4)&&!dck_binding()&&!dck_image(2,0);}
static u32 dck_undo(u32 token,u32 frame){DckPlan*p=&_gz_dck_plan;u32 z;
 z=_gz_roh_direct_recovery_common(token,frame);if(z)return z;
 if(!_gz_dck_recovery_scope(token)||!_gz_dck_gate||_gz_hg_capture_seal(_gz_dck_gate)||!_gz_dck_code_valid())return DCK_CONTEXT;
 if(p->groups==7&&(dct_before_refs()||dct_saved_refs()))return DCK_BINDING;
 p->phase=DCK_UNDOING;dck_seal();
 if(p->groups&2){if(_gz_dck_native_change(p->body+0x8a4,p->before_style)!=1)return DCK_API;}
 if(p->groups&4)dct_undo();
 if(p->groups&1)*(volatile u32*)(p->body+0x510)=R32(p->before,0x248+0x510);
 z=dck_binding();if(z)return z;z=dck_image(2,0);if(z)return z;
 z=_gz_hg_capture_seal(_gz_dck_gate);if(z)return z;
 z=_gz_roh_direct_recovery_common(token,frame);if(z)return z;
 p->phase=DCK_ROLLED;dck_seal();_gz_dck_closure_seal=dck_closure_value();return _gz_dck_closure_seal?0:DCK_CONTEXT;
}
/* Driver is called only by the already-owned fixed death-frame dispatcher.
 * No service result doubles, metadata-forced owner or fake lifetime receipt. */
u32 _gz_dck_step(u32 token){DckPlan*p=&_gz_dck_plan;u32 frame=_gz_pbac_frame,z=0,end=0,dirty=0,service=0,i,clean_blocked=0;
 if(!_gz_dck_request||_gz_dck_request==_gz_dck_ack)return 0;
 if(_gz_dck_active||_gz_dck_entered||_gz_dck_request!=_gz_dck_ack+1||!token||
  token!=_gz_doh_token||_gz_roh_owner[3]!=1||_gz_roh_owner[7]||_gz_dck_serial==BAD||
  (_gz_dck_mode<1||_gz_dck_mode>8))return 1;
 _gz_dck_active=_gz_dck_request;_gz_dck_entered=1;_gz_dck_serial++;_gz_dck_frame=frame;_gz_dck_thread=_gz_hg_platform_thread();
 for(i=2;i<48;i++)_gz_dck_report[i]=0;
 _gz_dck_report[2]=_gz_dck_active;_gz_dck_report[3]=token;_gz_dck_report[4]=frame;_gz_dck_report[24]=_gz_dck_mode;
 if(_gz_dck_seal||p->valid){
  if(p->phase!=DCK_ROLLED||p->token!=_gz_dck_retired_token||token<=p->token||
   _gz_dck_retired_seal!=_gz_dck_seal||_gz_dck_seal!=btr_hash(p,sizeof(*p))){z=DCK_CONTEXT;goto done;}
  eks_zero(p,sizeof(*p));_gz_dck_seal=_gz_dck_closure_seal=0;
 }
 if(!_gz_dck_context_owned(_gz_dck_serial)){z=DCK_CONTEXT;goto done;}
 _gz_dck_gate=_gz_hg_capture_begin(_gz_dck_serial);if(!_gz_dck_gate){z=86;goto done;}
 z=_gz_hg_capture_seal(_gz_dck_gate);if(z)goto done;
 z=_gz_roh_service_begin(token,1);if(z)goto done;service=1;
 z=dck_prepare(token,frame);_gz_dck_report[6]=z;
 if(!z&&_gz_dck_mode>=4){z=_gz_dlr_prepare(token,frame,p->source,p->source_bytes,p->pin);_gz_dck_report[27]=z;}
 if(!z&&_gz_dck_mode>=6){z=_gz_dfr_prepare(token,frame);_gz_dck_report[30]=z;}
 if(!z&&_gz_dck_mode>=7){z=_gz_dpk_prepare(token,frame,p->source,p->source_bytes,p->pin);_gz_dck_report[33]=z;}
 if(!z&&_gz_dck_mode>=8){z=_gz_duc_prepare(token,frame);_gz_dck_report[36]=z;}
 end=_gz_roh_service_end(token,1,0);service=0;if(z||end){if(!z)z=end;goto done;}
 z=_gz_roh_mark_mutating(token);_gz_dck_report[7]=z;if(z)goto done;dirty=1;
 if(_gz_dck_mode>=8){
  z=_gz_roh_service_begin(token,10);if(z)goto recover;service=10;
  z=_gz_duc_apply(token,frame);_gz_dck_report[37]=z;if(z)goto recover;
  z=_gz_roh_service_end(token,10,0);if(z)goto recover;service=0;
 }
 if(_gz_dck_mode>=7){
  z=_gz_roh_service_begin(token,6);if(z)goto recover;service=6;
  z=_gz_dpk_apply(token,frame);_gz_dck_report[34]=z;if(z)goto recover;
  z=_gz_roh_service_end(token,6,0);if(z)goto recover;service=0;
 }
 if(_gz_dck_mode>=4){
  z=_gz_roh_service_begin(token,2);if(z)goto recover;service=2;
  z=_gz_dlr_apply(token,frame);_gz_dck_report[28]=z;if(z)goto recover;
  z=_gz_roh_service_end(token,2,0);if(z)goto recover;service=0;
 }
 z=_gz_roh_service_begin(token,5);_gz_dck_report[8]=z;if(z)goto recover;service=5;
 z=dck_binding();if(!z)z=dck_image(2,0);if(z)goto recover;
 p->phase=DCK_ENDING;p->groups=1;dck_seal();
 _gz_dck_report[9]=_gz_dck_native_end(p->body,p->link_pid);
 if(_gz_dck_report[9]!=1){z=DCK_API;goto recover;}
 z=dck_image(1,1);if(z)goto recover;_gz_dck_report[10]=1;
 if(_gz_dck_mode>=2){
  if(_gz_hg_capture_seal(_gz_dck_gate)||dck_binding()||!_gz_dck_code_valid()){z=DCK_CONTEXT;goto recover;}
  p->phase=DCK_CHANGING;p->groups=3;dck_seal();
  _gz_dck_report[11]=_gz_dck_native_change(p->body+0x8a4,p->saved_style);
  if(_gz_dck_report[11]!=1){z=DCK_API;goto recover;}
  z=dck_image(1,3);if(z)goto recover;_gz_dck_report[12]=1;
 }
 if(_gz_dck_mode>=3){z=dct_apply(token,frame);if(z)goto recover;}
 _gz_dck_report[13]=1;z=DCK_TEST_FAILURE;
recover:
 _gz_dck_report[5]=z;
 end=_gz_roh_fail(token,z?z:DCK_TEST_FAILURE);_gz_dck_report[14]=end;
 end=_gz_roh_direct_recovery_begin(token,10);_gz_dck_report[15]=end;if(end){z=end;goto done;}
 if(_gz_dck_mode>=7){end=_gz_dpk_rollback(token,frame);_gz_dck_report[35]=end;if(end){z=end;goto done;}}
 if(_gz_dck_mode>=4){end=_gz_dlr_rollback(token,frame);_gz_dck_report[29]=end;if(end){z=end;goto done;}}
 if(_gz_dck_mode>=8){end=_gz_duc_rollback(token,frame);_gz_dck_report[38]=end;if(end){z=end;goto done;}}
 end=dck_undo(token,frame);_gz_dck_report[16]=end;if(end){z=end;goto done;}
 z=0;
done:
 if(!dirty&&_gz_dlr_active){end=_gz_dlr_clean_before(token,frame);if(!end&&_gz_dck_mode>=7&&!_gz_dpk_prepared_exact(token,frame))end=28;if(!end&&_gz_dck_mode>=8&&!_gz_duc_prepared_exact(token,frame))end=28;_gz_dck_report[31]=end;if(end){z=end;clean_blocked=1;}}
 if(_gz_dck_gate){end=_gz_hg_capture_end(_gz_dck_gate);if(!z)z=end;_gz_dck_gate=0;_gz_dck_report[17]=end;if(end)clean_blocked=1;}
 /* HG failures keep the failed owner held. A successful returned exclusion
  * and exact fresh source-free closure precede the separate metadata tail. */
 if(!z&&dirty){z=_gz_roh_direct_recovery_finish(token,frame);_gz_dck_report[18]=z;}
 _gz_dck_report[19]=z;_gz_dck_report[20]=_gz_roh_owner[3];_gz_dck_report[21]=_gz_roh_owner[4];
 _gz_dck_report[22]=_gz_dck_seal;_gz_dck_report[23]=_gz_dck_closure_seal;
 if(!z&&dirty&&doh_idle_owner()){
  if(_gz_dck_mode>=8)_gz_duc_retire_leaf();
  if(_gz_dck_mode>=7)_gz_dpk_retire_leaf();
  if(_gz_dck_mode>=4)_gz_dlr_retire_leaf();
  _gz_dck_retired_token=token;_gz_dck_retired_seal=_gz_dck_seal;
  _gz_dck_ack=_gz_dck_active;_gz_dck_active=0;_gz_doh_report[20]=1;
  _gz_doh_report[4]=frame;_gz_doh_report[5]=0;_gz_doh_report[7]=0;
  _gz_doh_token=0;_gz_doh_ack=_gz_doh_active;_gz_doh_active=0;
 }else if(!dirty&&!clean_blocked){
  if(service==1)_gz_roh_service_end(token,1,0);
  doh_finish(z);if(!_gz_doh_token){
   if(_gz_duc_active)_gz_duc_retire_leaf();
   if(_gz_dpk_active)_gz_dpk_retire_leaf();
   if(_gz_dlr_active)_gz_dlr_retire_leaf();
   btr_copy(&_gz_dck_clean_abort_plan,p,sizeof(*p));_gz_dck_report[32]=btr_hash(p,sizeof(*p));
   eks_zero(p,sizeof(*p));_gz_dck_seal=_gz_dck_closure_seal=0;
   _gz_dck_retired_token=_gz_dck_retired_seal=0;
   _gz_dck_ack=_gz_dck_active;_gz_dck_active=0;
  }
 }
 _gz_dck_entered=0;return 1;
}
