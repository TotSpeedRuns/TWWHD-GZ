/* Direct no-lifetime Link-core participant. Uses the unchanged audited LPC
 * record/binding/copy/match kernels, with the real selected source rather than
 * an expired-bomb recipe. Face, sound, item children, UI and event cancellation
 * remain separate participants. This module never publishes a saved world.
 */
extern u32 _gz_dck_body_context(u32,u32),_gz_dck_body_gate(u32,u32);
extern u32 _gz_dfr_current_match(void),_gz_dfr_role_seal(void),_gz_dfr_partial_owned(void);
extern u32 _gz_dfr_prepare(u32,u32),_gz_dfr_apply(u32,u32),_gz_dfr_rollback(u32,u32),_gz_dfr_closure_key(u32,u32);
extern void _gz_dfr_retire_leaf(void);
extern u32 _gz_dfr_prepared_exact(void);
extern u32 _gz_roh_direct_body_common(u32,u32,u32),_gz_roh_direct_recovery_common(u32,u32);
extern volatile u32 _gz_dck_request;
extern void _gz_player_observe(void);
extern u8 _gz_player_observed[];
typedef struct {
 u32 magic,bytes,valid,token,frame,plan_seal,actor,pid,actor_bytes,actor_hash,source,pin,used,nonce,reserved0,reserved1;
} DlrRole;
typedef char DlrRoleSize[sizeof(DlrRole)==64?1:-1];
static DlrRole _gz_dlr_role;
static u32 _gz_dlr_seal,_gz_dlr_active,_gz_dlr_entered,_gz_dlr_retired_token,_gz_dlr_retired_seal;
u8 _gz_dlr_actor_before[0x8284],_gz_dlr_native_body[8192];
LpcPlan _gz_dlr_witness_plan;
LpcRun _gz_dlr_applied_run,_gz_dlr_rolled_run;
u32 _gz_dlr_witness_seals[4];
u32 _gz_dlr_report[48]={0x444c5231u,192};
static void dlr_zero(void*p,u32 n){u32 i;for(i=0;i<n;i++)((volatile u8*)p)[i]=0;}
extern volatile u32 _gz_dck_mode;
#include "death_player_sound.inc"
static u32 dlr_sealed(void){const DlrRole*d=&_gz_dlr_role;const LpcPlan*p=&_gz_lpc_plan;
 return d->magic==0x444c5031u&&d->bytes==sizeof(*d)&&d->valid==1&&d->token&&d->token==_gz_dlr_active&&
  d->frame==_gz_pbac_frame&&d->actor_bytes==0x8284&&d->nonce==_gz_dck_request&&
  d->reserved0==(_gz_dck_mode>=5?_gz_dls_seal:0)&&d->reserved1==_gz_dfr_role_seal()&&d->plan_seal==_gz_lpc_seal&&
  d->actor==p->actor&&d->pid==p->pid&&p->token==d->token&&p->frame==d->frame&&p->before_role==1&&
  d->source==p->source&&d->pin==p->pin&&d->used==p->used&&
  _gz_dlr_seal==btr_hash(d,sizeof(*d))&&d->actor_hash==btr_hash(_gz_dlr_actor_before,sizeof(_gz_dlr_actor_before))&&lpc_sealed()&&dls_bound();
}
/* Only values from the two sealed images may appear at each typed footprint
 * during APPLYING/UNDOING. Resource/procedure pointers are checked as complete
 * words. The intermediate role grants rollback, never native execution. */
static u32 dlr_union_bytes(const void*live,const u8*b,const u8*s,u32 n){u32 i;const volatile u8*v=live;
 for(i=0;i<n;i++)if(v[i]!=b[i]&&v[i]!=s[i])return 0;return 1;
}
static u32 dlr_union(void){const LpcPlan*p=&_gz_lpc_plan;u32 a=p->actor,i,j,k=3200,b,s,v,t;
 u32 partial_face=_gz_dfr_partial_owned();
 for(i=0;i<5;i++){
  if(lpc_resource(&p->archive,p->before_bind[i],&b)||lpc_resource(&p->archive,p->saved_bind[i],&s))return 0;
  v=R32(a,0x57fc+i*16);if(v!=b&&v!=s)return 0;t=128+i*200;
  if(!dlr_union_bytes((const void*)(a+0x5848+i*16),p->before+t,p->saved+t,8)||
   !dlr_union_bytes((const void*)(a+0x5898+i*16),p->before+t+8,p->saved+t+8,16)||
   !dlr_union_bytes((const void*)(a+0x57f8+i*16),p->before+t+24,p->saved+t+24,4)||
   !dlr_union_bytes((const void*)p->targets[i+3],p->before+t+28,p->saved+t+28,168))return 0;
 }
 if(!dlr_union_bytes((const void*)p->targets[0],p->before+1128,p->saved+1128,2)||
  !btr_equal((const void*)(p->targets[0]+2),p->before+1130,2)||
  !dlr_union_bytes((const void*)(p->targets[0]+4),p->before+1132,p->saved+1132,24)||
  !dlr_union_bytes((const void*)p->targets[2],p->before+1156,p->saved+1156,672))return 0;
 for(i=0;i<42;i++){
  t=1828+i*32;
  if(!dlr_union_bytes((const void*)(p->targets[1]+32*i),p->before+t,p->saved+t,18)||
   !btr_equal((const void*)(p->targets[1]+32*i+18),p->before+t+18,2)||
   !dlr_union_bytes((const void*)(p->targets[1]+32*i+20),p->before+t+20,p->saved+t+20,12))return 0;
 }
 for(i=0;i<_gz_lpc_field_count();i++){
  u32 skip=lpc_fields[i].offset==0x693c?4:0;
  if(partial_face&&lpc_fields[i].offset<=0x6980&&lpc_fields[i].offset+lpc_fields[i].bytes>=0x6984){
   u32 pre=0x6980-lpc_fields[i].offset,post=lpc_fields[i].bytes-pre-4;
   if(!dlr_union_bytes((const void*)(a+lpc_fields[i].offset),p->before+k,p->saved+k,pre)||
    !dlr_union_bytes((const void*)(a+0x6984),p->before+k+pre+4,p->saved+k+pre+4,post))return 0;
  }else if(!dlr_union_bytes((const void*)(a+lpc_fields[i].offset+skip),p->before+k+skip,p->saved+k+skip,lpc_fields[i].bytes-skip))return 0;
  k+=lpc_fields[i].bytes;
 }
 for(i=0;i<2;i++){
  v=R32(a,0x65f4+i*4);if(v!=R32(p->before,44+i*4)&&v!=R32(p->saved,44+i*4))return 0;
 }
 for(i=0;i<4;i++)if(!dlr_union_bytes((const void*)(a+lpc_motion_fields[i].offset),p->before+4612+i*4,p->saved+4612+i*4,4))return 0;
 k=4644;for(i=0;i<3;i++){
  j=lpc_pose_sizes[i];if(!dlr_union_bytes((const void*)lpc_pose_globals[i],p->before+k,p->saved+k,j))return 0;k+=j;
 }
 return 1;
}
/* Called by DOH only after its independent fresh Link process lookup. No
 * owner/common/source API is called here, so there is no owner recursion.
 * A positive role also certifies the fade value within the typed body map. */
u32 _gz_dlr_current_role(u32 actor,u32 pid){const LpcPlan*p=&_gz_lpc_plan;const LpcRun*r=&_gz_lpc_run;
 if(!dlr_sealed()||actor!=p->actor||pid!=p->pid||!_gz_dck_body_context(p->token,_gz_pbac_frame)||
  R32(actor,0x6a80)!=p->before_gameover_ID||R16(actor,0x420)!=5||_gz_pbac_event!=3||lpc_binding(0)||!_gz_dfr_current_match())return 0;
 if(r->phase==LPC_PREPARED||r->phase==LPC_ROLLED)return !lpc_match(p->before,p->before_bind)&&dls_match(0)?1:0;
 if(r->phase==LPC_APPLIED){
  if(_gz_dfr_partial_owned())return dlr_union()&&dls_match(2)?3:0;
  return !lpc_match(p->saved,p->saved_bind)&&dls_match(1)?2:0;
 }
 return (r->phase==LPC_APPLYING||r->phase==LPC_UNDOING)&&dlr_union()&&dls_match(2)?3:0;
}
static u32 dlr_source(u32 token,u32 frame,u32 s,u32 n,u32 pin){
 return _gz_dck_body_context(token,frame)&&_gz_roh_gl_context(token,frame,1)==1&&pin&&
  _gz_osa_pin_id()==pin&&!_gz_osa_validate(pin)&&_gz_osa_source(pin)==s&&_gz_osa_bytes(pin)==n&&
  _gz_roh_owner[17]==s&&_gz_roh_owner[18]==n&&n>=216;
}
u32 _gz_dlr_prepare(u32 token,u32 frame,u32 s,u32 n,u32 pin){LpcPlan*p=&_gz_lpc_plan;LpcRun*r=&_gz_lpc_run;DlrRole*d=&_gz_dlr_role;
 u32 z,off,birth,i,br;const VrWorld*w;
 if(_gz_dlr_entered||_gz_lpc_entered||p->valid||r->phase||_gz_lpc_seal||_gz_dlr_active||
  !dlr_source(token,frame,s,n,pin)||!_gz_dck_body_gate(token,frame))return 28;
 if(d->valid&&(d->token!=_gz_dlr_retired_token||token<=d->token||_gz_dlr_retired_seal!=_gz_dlr_seal||_gz_dlr_seal!=btr_hash(d,sizeof(*d))))return 28;
 _gz_dlr_entered=_gz_lpc_entered=1;for(i=2;i<48;i++)_gz_dlr_report[i]=0;
 _gz_dlr_report[2]=token;_gz_dlr_report[3]=frame;
 dlr_zero(p,sizeof(*p));dlr_zero(r,sizeof(*r));
 z=_gz_bcb_archive_validate(s,n,1);if(z)goto done;
 off=R32(s,40);if(R32(s,44)!=8192||off<216||off>n||8192>n-off){z=LPC_SOURCE;goto done;}
 p->magic=0x4c504331u;p->bytes=sizeof(*p);p->version=2;p->token=token;p->frame=frame;p->source=s;p->used=n;p->pin=pin;
 btr_copy(p->saved,(const void*)(s+off),8192);p->source_hash=btr_hash(p->saved,8192);
 z=lpc_binding(1);if(z)goto done;w=(const VrWorld*)(s+R32(s,168));birth=s+R32(s,192);
 if(w->link_pid!=p->pid||w->link_actor!=p->actor||R32(p->saved,4)!=p->pid||R32(p->saved,8)!=p->actor){z=LPC_PROCESS;goto done;}
 for(i=0;i<w->count;i++)if(w->actors[i].pid==p->pid)break;
 if(i==w->count){z=LPC_PROCESS;goto done;}br=birth+96+i*192;
 if(R32(br,0)!=i||R32(br,4)!=1||!btr_equal((const void*)(br+16),&p->process,sizeof(PoEntry))){z=LPC_PROCESS;goto done;}
 z=lpc_record(p->saved,p->saved_bind,&p->archive);if(z)goto done;
 z=lpc_capture();if(z)goto done;p->before_hash=btr_hash(p->before,8192);
 if(p->before_role!=1){z=LPC_SOURCE;goto done;}
 z=lpc_binding(0);if(z)goto done;z=lpc_match(p->before,p->before_bind);if(z)goto done;
 btr_copy(_gz_dlr_actor_before,(const void*)p->actor,sizeof(_gz_dlr_actor_before));
 if(!dlr_source(token,frame,s,n,pin)||!btr_equal((const void*)(s+off),p->saved,8192)){z=42;goto done;}
 p->valid=1;_gz_lpc_seal=btr_hash(p,sizeof(*p));
 r->magic=0x4c505231u;r->bytes=sizeof(*r);r->valid=1;r->phase=LPC_PREPARED;r->token=token;r->plan_seal=_gz_lpc_seal;lpc_run_seal();
 if(_gz_dck_mode>=5){z=dls_prepare();if(z)goto done;}
 dlr_zero(d,sizeof(*d));d->magic=0x444c5031u;d->bytes=sizeof(*d);d->valid=1;d->token=token;d->frame=frame;
 d->plan_seal=_gz_lpc_seal;d->actor=p->actor;d->pid=p->pid;d->actor_bytes=sizeof(_gz_dlr_actor_before);
 d->actor_hash=btr_hash(_gz_dlr_actor_before,sizeof(_gz_dlr_actor_before));d->source=s;d->pin=pin;d->used=n;d->nonce=_gz_dck_request;d->reserved0=_gz_dck_mode>=5?_gz_dls_seal:0;
 _gz_dlr_seal=btr_hash(d,sizeof(*d));_gz_dlr_active=token;
 btr_copy(&_gz_dlr_witness_plan,p,sizeof(*p));_gz_dlr_witness_seals[0]=_gz_lpc_seal;
 _gz_lpc_report[2]=token;_gz_lpc_report[3]=LPC_PREPARED;_gz_lpc_report[4]=0;_gz_lpc_report[5]=frame;
 _gz_dlr_report[5]=_gz_lpc_seal;_gz_dlr_report[6]=d->actor_hash;_gz_dlr_report[7]=p->actor;
 z=0;
done:
 _gz_dlr_report[4]=z;
 if(z){dlr_zero(&_gz_dls_plan,sizeof(_gz_dls_plan));_gz_dls_seal=_gz_dls_active=0;dlr_zero(p,sizeof(*p));dlr_zero(r,sizeof(*r));_gz_lpc_seal=_gz_lpc_run_seal=0;}
 _gz_dlr_entered=_gz_lpc_entered=0;return z;
}
u32 _gz_dlr_apply(u32 token,u32 frame){LpcPlan*p=&_gz_lpc_plan;LpcRun*r=&_gz_lpc_run;u32 z,gate;
 if(_gz_dlr_entered||_gz_lpc_entered||!dlr_sealed()||p->token!=token||r->phase!=LPC_PREPARED)return 28;
 _gz_dlr_entered=_gz_lpc_entered=1;gate=_gz_dck_body_gate(token,frame);
 if(!gate||_gz_roh_direct_body_common(token,frame,1)){z=40;goto done;}
 if(_gz_osa_validate(p->pin)||_gz_osa_pin_id()!=p->pin||_gz_osa_source(p->pin)!=p->source||
  _gz_osa_bytes(p->pin)!=p->used||R32(p->source,44)!=8192||
  !btr_equal((const void*)(p->source+R32(p->source,40)),p->saved,8192)){z=42;goto done;}
 z=lpc_binding(0);if(z)goto done;z=lpc_match(p->before,p->before_bind);if(z)goto done;
 r->phase=LPC_APPLYING;r->applied_frame=frame;lpc_run_seal();
 z=lpc_values(p->saved,p->saved_bind);if(z)goto done;
 z=dls_values(1);if(z)goto done;
 z=lpc_binding(0);if(z)goto done;z=lpc_match(p->saved,p->saved_bind);if(z)goto done;
 r->phase=LPC_APPLIED;lpc_run_seal();
 if(!_gz_dck_body_gate(token,frame)||_gz_roh_direct_body_common(token,frame,1)){z=40;goto done;}
 z=_gz_dfr_apply(token,frame);if(z)goto done;
 /* Original serializer is an independent read of the actual native body.
  * Its complete validity is reported honestly; face/sound remain separate. */
 _gz_player_observe();btr_copy(_gz_dlr_native_body,_gz_player_observed,8192);
 btr_copy(&_gz_dlr_applied_run,r,sizeof(*r));_gz_dlr_witness_seals[1]=_gz_lpc_run_seal;
 _gz_dlr_witness_seals[2]=btr_hash(_gz_dlr_native_body,8192);
 _gz_dlr_report[8]=1;_gz_dlr_report[9]=frame;_gz_dlr_report[10]=R32(p->actor,0x65f0);
 _gz_dlr_report[11]=R32(_gz_dlr_native_body,0);_gz_dlr_report[12]=_gz_dlr_witness_seals[2];
 z=0;
done:
 r->error=z;lpc_run_seal();_gz_lpc_report[3]=r->phase;_gz_lpc_report[4]=z;
 _gz_dlr_report[13]=z;_gz_dlr_entered=_gz_lpc_entered=0;return z;
}
static u32 dlr_before_exact(void){const LpcPlan*p=&_gz_lpc_plan;u32 i;
 if(lpc_binding(0)||lpc_match(p->before,p->before_bind)||
  !btr_equal((const void*)p->actor,_gz_dlr_actor_before,sizeof(_gz_dlr_actor_before))||
  !btr_equal((const void*)p->targets[0],p->before+1128,28)||
  !btr_equal((const void*)p->targets[1],p->before+1828,1344))return 0;
 for(i=0;i<5;i++)if(!btr_equal((const void*)p->targets[i+3],p->before+156+i*200,168))return 0;
 return 1;
}
u32 _gz_dlr_clean_before(u32 token,u32 frame){
 if(!dlr_sealed()||_gz_lpc_plan.token!=token||_gz_lpc_run.phase!=LPC_PREPARED||
  _gz_roh_owner[3]!=1||_gz_roh_owner[4]!=1||_gz_roh_owner[5]!=token||_gz_roh_owner[6]||_gz_roh_owner[7]||
  (_gz_roh_owner[12]!=0&&_gz_roh_owner[12]!=1)||!_gz_dck_body_gate(token,frame)||
  !dlr_before_exact()||!_gz_dfr_prepared_exact())return 28;
 _gz_dlr_report[18]=1;return 0;
}
u32 _gz_dlr_rollback(u32 token,u32 frame){LpcPlan*p=&_gz_lpc_plan;LpcRun*r=&_gz_lpc_run;u32 z;
 if(_gz_dlr_entered||_gz_lpc_entered||!dlr_sealed()||p->token!=token)return 28;
 _gz_dlr_entered=_gz_lpc_entered=1;
 if(!_gz_dck_body_gate(token,frame)||_gz_roh_direct_recovery_common(token,frame)||
  !_gz_dlr_current_role(p->actor,p->pid)){z=40;goto done;}
 r->phase=LPC_UNDOING;lpc_run_seal();
 z=_gz_dfr_rollback(token,frame);if(z)goto done;
 z=lpc_values(p->before,p->before_bind);if(z)goto done;
  z=dls_values(0);if(z)goto done;
 if(!dlr_before_exact()){z=LPC_CHANGED;goto done;}
 if(!_gz_dck_body_gate(token,frame)||_gz_roh_direct_recovery_common(token,frame)){z=40;goto done;}
 r->phase=LPC_ROLLED;r->rolled_frame=frame;r->error=0;lpc_run_seal();
 btr_copy(&_gz_dlr_rolled_run,r,sizeof(*r));_gz_dlr_witness_seals[3]=_gz_lpc_run_seal;
 _gz_dlr_report[14]=1;_gz_dlr_report[15]=frame;_gz_dlr_report[16]=R32(p->actor,0x65f0);z=0;
done:
 if(z){r->error=z;lpc_run_seal();}_gz_dlr_report[17]=z;_gz_dlr_entered=_gz_lpc_entered=0;return z;
}
u32 _gz_dlr_closure_key(u32 token,u32 frame,u32 camera_seal){u32 words[9],face=0;
 if(!camera_seal||_gz_dlr_entered||_gz_lpc_entered||!dlr_sealed()||_gz_lpc_plan.token!=token||
  _gz_lpc_run.phase!=LPC_ROLLED||_gz_lpc_run.error||!_gz_dck_body_context(token,frame)||!dlr_before_exact())return 0;
 words[0]=0x444c4331u;words[1]=token;words[2]=frame;words[3]=camera_seal;
 words[4]=_gz_dlr_seal;words[5]=_gz_lpc_seal;words[6]=_gz_lpc_run_seal;words[7]=_gz_dlr_role.actor_hash;
 if(_gz_dck_mode>=6){face=_gz_dfr_closure_key(token,frame);if(!face)return 0;words[0]=0x444c4332u;words[8]=face;return btr_hash(words,sizeof(words));}
 return btr_hash(words,8*4);
}
/* Called only after camera+body closure was checked by the original owner
 * release tail. Fixed private metadata cleanup, no native/source writes. */
void _gz_dlr_retire_leaf(void){
 if(_gz_dck_mode>=6)_gz_dfr_retire_leaf();
 _gz_dlr_retired_token=_gz_dlr_role.token;_gz_dlr_retired_seal=_gz_dlr_seal;
 dlr_zero(&_gz_lpc_plan,sizeof(_gz_lpc_plan));dlr_zero(&_gz_lpc_run,sizeof(_gz_lpc_run));
 _gz_lpc_seal=_gz_lpc_run_seal=_gz_lpc_entered=_gz_dls_active=0;_gz_dlr_active=0;
}
