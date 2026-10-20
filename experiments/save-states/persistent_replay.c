/* Persistent row replay through fixed current-root operations. Independent
 * before-images support dirty undo without reading the original archive.
 * This participant alone cannot publish or release a full restored world. */
#define PRK_BYTES (PP_SAVE_BYTES+3u)
extern u8 _gz_spk_values[PRK_BYTES];
extern u32 _gz_spk_owner_read(void),_gz_spk_owner_apply(void),_gz_spk_owner_undo(void),_gz_spk_owner_check(void);
extern u32 _gz_spk_private_apart(u32,u32);
enum {PRK_PREPARED=1,PRK_APPLYING=2,PRK_APPLIED=3,PRK_UNDOING=4,PRK_ROLLED=5};
typedef struct {
 u32 magic,bytes,version,valid,phase,token,frame,source,used,pin,row,row_hash;
 u8 before[PRK_BYTES],saved[PP_OUTPUT_BYTES];
 u32 applied_frame,rolled_frame,error;
} PrkPlan;
typedef char PrkSize[sizeof(PrkPlan)==7644?1:-1];
static PrkPlan _gz_prk_plan;
static u32 _gz_prk_seal,_gz_prk_entered,_gz_prk_kernel_mode,_gz_prk_kernel_frame;
u32 _gz_prk_report[24]={0x50524b31u,96};
static void prk_seal(void){_gz_prk_seal=btr_hash(&_gz_prk_plan,sizeof(PrkPlan));}
static u32 prk_sealed(void){const PrkPlan*p=&_gz_prk_plan;
 return p->magic==0x50524b31u&&p->bytes==sizeof(*p)&&p->version==1&&p->valid==1&&p->token&&
  p->phase>=1&&p->phase<=5&&_gz_prk_seal==btr_hash(p,sizeof(*p));
}
static u32 prk_skip(u32 i){return i<PP_SAVE_BYTES&&_gz_pp_fixed.offsets[i]>=36&&_gz_pp_fixed.offsets[i]<42;}
static void prk_report(u32 z){const PrkPlan*p=&_gz_prk_plan;
 _gz_prk_report[2]=p->token;_gz_prk_report[3]=p->phase;_gz_prk_report[4]=z;
 _gz_prk_report[5]=btr_hash(p->before,PRK_BYTES);_gz_prk_report[6]=btr_hash(p->saved,PP_OUTPUT_BYTES);
 _gz_prk_report[7]=p->applied_frame;_gz_prk_report[8]=p->rolled_frame;
}
/* Recovery may retain an active service ID only after the original
 * callback returned and its sealed failure receipt still matches the owner.
 * This does not admit a running callback or any forward writer. */
u32 _gz_prk_kernel_scope(u32 mode,u32 frame){const PrkPlan*p=&_gz_prk_plan;const volatile u32*o=_gz_roh_owner;
 if(_gz_prk_entered!=1||mode!=_gz_prk_kernel_mode||frame!=_gz_prk_kernel_frame||frame!=_gz_pbac_frame||
  !prk_sealed()||o[0]!=ROH_MAGIC||o[1]!=240||o[2]!=2||o[4]!=1||o[5]!=p->token||_gz_roh_serial!=p->token)return 0;
 if(mode==2)return p->phase==PRK_PREPARED&&o[3]==ROH_CLEAN&&!o[6]&&!o[7]&&o[11]&&o[12]==1&&o[10]==frame;
 if(mode==3)return p->phase==PRK_APPLYING&&o[3]==ROH_MUTATING&&o[6]==1&&!o[7]&&o[11]&&o[12]==6&&o[10]==frame&&_gz_bmh_scope(p->token,frame);
 if(mode!=4&&mode!=5)return 0;
 if(o[3]!=ROH_FAILED_DIRTY||o[6]!=1||!o[7]||
  (o[12]&&_gz_bmh_aborted_scope(p->token)!=1)||!bor_sealed()||_gz_boh_recovery.token!=p->token)return 0;
 if(mode==4)return p->phase==PRK_UNDOING&&_gz_boh_recovery.phase==BOR_UNDO;
 return (p->phase==PRK_PREPARED||p->phase==PRK_ROLLED)&&
  (_gz_boh_recovery.phase==BOR_UNDO||_gz_boh_recovery.phase==BOR_DONE);
}
/* Called only inside a fully authenticated fixed kernel operation. Scalar
 * getters cannot disclose/accept a native/source pointer or select offsets. */
u32 _gz_prk_kernel_value(u32 i,u32 kind){
 if(i>=PRK_BYTES||kind>2||_gz_prk_entered!=1||_gz_prk_kernel_frame!=_gz_pbac_frame||
  (_gz_prk_kernel_mode!=3&&_gz_prk_kernel_mode!=4))return 256;
 if(kind==2)return _gz_prk_kernel_mode==3?_gz_prk_plan.saved[i]:_gz_prk_plan.before[i];
 return kind==1&&_gz_prk_kernel_mode==4?_gz_prk_plan.saved[i]:_gz_prk_plan.before[i];
}
static u32 prk_apart(u32 a,u32 n,u32 b,u32 m){return a>=b?a-b>=m:b-a>=n;}
static u32 prk_output_apart(u32 source){u32 q[]={(u32)&_gz_prk_seal,(u32)&_gz_prk_entered,
 (u32)&_gz_prk_kernel_mode,(u32)&_gz_prk_kernel_frame};u32 i;
 /* HBP authenticates an actual OSA slot allocation of exactly 2MiB. Exclude
  * the WHOLE retained allocation, including unused capacity, from new output. */
 if(!span(source,0x200000u)||!prk_apart(source,0x200000u,(u32)&_gz_prk_plan,sizeof(PrkPlan))||
  !prk_apart(source,0x200000u,(u32)_gz_prk_report,sizeof(_gz_prk_report))||
  !_gz_spk_private_apart(source,0x200000u))return 0;
 for(i=0;i<4;i++)if(!prk_apart(source,0x200000u,q[i],4))return 0;return 1;
}
static u32 prk_source(u32 token,u32 frame,u32 service){const PrkPlan*p=&_gz_prk_plan;
 if(!prk_sealed()||p->token!=token)return 42;
 if(service){if(!_gz_hbp_service_read(token,frame))return 42;}
 else if(!_gz_hbp_read(token,frame))return 42;
 if(p->source!=_gz_hbp_recipe.source||p->used!=_gz_hbp_recipe.used||p->pin!=_gz_hbp_recipe.pin)return 42;
 if(!span(p->source,p->used)||p->used<232||R32(p->source,8)!=27||R32(p->source,12)!=27||
  R32(p->source,24)<232||R32(p->source,24)>p->used||R32(p->source,28)!=PP_ROW_BYTES||
  PP_ROW_BYTES>p->used-R32(p->source,24)||p->row!=p->source+R32(p->source,24))return 42;
 if(btr_hash((const void*)p->row,PP_ROW_BYTES)!=p->row_hash||_gz_pp_validate(p->row,PP_ROW_BYTES))return 42;
 return 0;
}
static u32 prk_clock(u32 saved){const PrkPlan*p=&_gz_prk_plan;const u32*c=saved?_gz_ckr_plan.saved:_gz_ckr_plan.before;u32 i,o,x;
 if(!ckr_sealed()||_gz_ckr_plan.token!=p->token)return 77;
 for(i=0;i<PP_SAVE_BYTES;i++)if(prk_skip(i)){
  o=_gz_pp_fixed.offsets[i]-36;
  x=o<4?(c[0]>>(24-8*o))&255:(c[1]>>(8*(5-o)))&255;
  if((saved?p->saved[i]:p->before[i])!=x)return 77;
 }
 if(saved&&R32(p->saved,PRK_BYTES)!=c[2])return 77;return 0;
}
u32 _gz_prk_prepare(u32 token,u32 frame){PrkPlan*p=&_gz_prk_plan;u32 i,z=0,source,used,o;
 if(_gz_prk_entered||p->valid||_gz_prk_seal)return 28;
 if(!token||_gz_roh_gl_context(token,frame,1)!=1||!_gz_hbp_read(token,frame))return 42;
 source=_gz_hbp_recipe.source;used=_gz_hbp_recipe.used;
 if(!span(source,used)||used<232||R32(source,8)!=27||R32(source,12)!=27)return 42;
 o=R32(source,24);if(o<232||o>used||R32(source,28)!=PP_ROW_BYTES||PP_ROW_BYTES>used-o)return 42;
 if(!prk_output_apart(source))return 73;
 for(i=0;i<sizeof(*p)/4;i++)((u32*)p)[i]=0;
 p->magic=0x50524b31u;p->bytes=sizeof(*p);p->version=1;p->valid=1;p->phase=PRK_PREPARED;
 p->token=token;p->frame=frame;p->source=source;p->used=used;p->pin=_gz_hbp_recipe.pin;p->row=source+o;
 p->row_hash=btr_hash((const void*)p->row,PP_ROW_BYTES);
 z=_gz_pp_project(p->row,PP_ROW_BYTES,(u32)p->saved,PP_OUTPUT_BYTES);if(z)goto done;prk_seal();
 z=prk_source(token,frame,0);if(z)goto done;z=prk_clock(1);if(z)goto done;
 _gz_prk_entered=1;_gz_prk_kernel_mode=2;_gz_prk_kernel_frame=frame;
 z=_gz_spk_owner_read();if(z)goto done;
 btr_copy(p->before,_gz_spk_values,PRK_BYTES);prk_seal();
 z=prk_clock(0);if(!z)z=prk_source(token,frame,0);
 if(!z&&_gz_roh_gl_context(token,frame,1)!=1)z=42;
done:
 prk_report(z);_gz_prk_entered=_gz_prk_kernel_mode=_gz_prk_kernel_frame=0;
 if(z){for(i=0;i<sizeof(*p)/4;i++)((u32*)p)[i]=0;_gz_prk_seal=0;}return z;
}
u32 _gz_prk_apply(u32 token){PrkPlan*p=&_gz_prk_plan;u32 frame=_gz_pbac_frame,z;
 if(_gz_prk_entered||!prk_sealed()||p->token!=token||p->phase!=PRK_PREPARED)return 28;
 if(_gz_roh_owner[12]!=6||!_gz_bmh_scope(token,frame)||_gz_roh_replay_common(token,frame,0))return 49;
 z=prk_source(token,frame,1);if(z)return z;
 if(!ckr_sealed()||_gz_ckr_plan.token!=token||_gz_ckr_plan.phase!=CKR_APPLIED||prk_clock(1))return 77;
 _gz_prk_entered=1;_gz_prk_kernel_mode=3;_gz_prk_kernel_frame=frame;
 p->phase=PRK_APPLYING;p->applied_frame=frame;prk_seal();z=_gz_spk_owner_apply();
 if(!z)z=prk_source(token,frame,1);if(!z&&_gz_roh_replay_common(token,frame,0))z=42;
 if(!z)p->phase=PRK_APPLIED;p->error=z;prk_seal();prk_report(z);
 _gz_prk_entered=_gz_prk_kernel_mode=_gz_prk_kernel_frame=0;return z;
}
u32 _gz_prk_recovery_rollback(u32 token){PrkPlan*p=&_gz_prk_plan;u32 frame=_gz_pbac_frame,z,prepared,i;
 if(_gz_prk_entered||!prk_sealed()||p->token!=token||p->phase==PRK_ROLLED)return 28;
 if(!bor_original()||_gz_boh_recovery.phase!=BOR_UNDO||_gz_boh_recovery.token!=token)return 40;
 z=_gz_roh_recovery_common(token,frame);if(z)return z;prepared=p->phase==PRK_PREPARED;
 _gz_prk_entered=1;_gz_prk_kernel_mode=prepared?5:4;_gz_prk_kernel_frame=frame;
 if(!prepared){p->phase=PRK_UNDOING;prk_seal();z=_gz_spk_owner_undo();}
 else{
  z=_gz_spk_owner_check();if(!z)for(i=0;i<PRK_BYTES;i++)if(!prk_skip(i)&&_gz_spk_values[i]!=p->before[i]){z=77;break;}
 }
 if(!z)z=_gz_roh_recovery_common(token,frame);
 if(!z){p->phase=PRK_ROLLED;p->rolled_frame=frame;p->error=0;}else p->error=z;
 prk_seal();prk_report(z);_gz_prk_entered=_gz_prk_kernel_mode=_gz_prk_kernel_frame=0;return z;
}
u32 _gz_prk_recovery_check(u32 token,u32 frame){PrkPlan*p=&_gz_prk_plan;u32 z,i;
 if(_gz_prk_entered||!prk_sealed()||p->token!=token||p->phase!=PRK_ROLLED||p->error||!bor_original()||_gz_boh_recovery.phase!=BOR_DONE)return 77;
 z=_gz_roh_recovery_common(token,frame);if(z)return z;
 _gz_prk_entered=1;_gz_prk_kernel_mode=5;_gz_prk_kernel_frame=frame;z=_gz_spk_owner_check();
 if(!z)for(i=0;i<PRK_BYTES;i++)if(!prk_skip(i)&&_gz_spk_values[i]!=p->before[i]){z=77;break;}
 if(!z)z=_gz_roh_recovery_common(token,frame);
 _gz_prk_entered=_gz_prk_kernel_mode=_gz_prk_kernel_frame=0;return z;
}
