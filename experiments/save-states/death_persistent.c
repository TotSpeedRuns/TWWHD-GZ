/* Direct selected-source persistent/clock participant. This is a typed
 * service6 under the existing death-frame owner, never an expired-bomb lane.
 * Native addresses stay inside the separate fixed current-root kernel. */
extern u32 _gz_dck_state_context(u32,u32),_gz_dck_state_gate(u32,u32);
extern u32 _gz_roh_direct_state_common(u32,u32,u32);
extern u32 _gz_dpk_root_access(u32),_gz_dpk_root_receipt(u32,u32),_gz_dpk_root_finish(u32,u32),_gz_dpk_root_private_apart(u32,u32);
extern void _gz_dpk_root_forget_leaf(u32);
extern u8 _gz_dpk_root_values[PP_SAVE_BYTES+3];
extern u32 _gz_dpk_root_clocks[5];
enum {DPK_PREPARED=1,DPK_APPLYING=2,DPK_APPLIED=3,DPK_UNDOING=4,DPK_ROLLED=5};
typedef struct {
 u32 magic,bytes,version,valid,phase,token,frame,source,used,pin,row,row_hash,clock_row,clock_hash;
 u8 before[PP_SAVE_BYTES+3],saved[PP_OUTPUT_BYTES];
 u32 before_clock[5],saved_clock[5],root_receipt,applied_frame,rolled_frame,error;
} DpkPlan;
typedef char DpkPlanSize[sizeof(DpkPlan)==7696?1:-1];
static DpkPlan _gz_dpk_plan;
static u32 _gz_dpk_seal,_gz_dpk_active,_gz_dpk_entered,_gz_dpk_mode,_gz_dpk_frame;
DpkPlan _gz_dpk_prepared_plan,_gz_dpk_applied_plan,_gz_dpk_rolled_plan;
u32 _gz_dpk_report[32]={0x44504b31u,128};
static void dpk_seal(void){_gz_dpk_seal=btr_hash(&_gz_dpk_plan,sizeof(_gz_dpk_plan));}
static u32 dpk_values(const u32*p){return finite(p[0])&&p[1]<=65535&&finite(p[2])&&p[3]<=65535&&finite(p[4]);}
static u32 dpk_coupled(const u8*s,const u32*c){u32 i,o,v;
 for(i=0;i<PP_SAVE_BYTES;i++){
  o=_gz_pp_fixed.offsets[i];if(o<36||o>=42)continue;o-=36;
  v=o<4?(c[0]>>(24-8*o))&255:(c[1]>>(8*(5-o)))&255;
  if(s[i]!=v)return 0;
 }
 return 1;
}
static u32 dpk_sealed(void){const DpkPlan*p=&_gz_dpk_plan;
 return p->magic==0x44505031u&&p->bytes==sizeof(*p)&&p->version==1&&p->valid==1&&p->token&&
  p->phase>=1&&p->phase<=5&&p->frame==_gz_pbac_frame&&_gz_dpk_seal==btr_hash(p,sizeof(*p))&&
  dpk_values(p->before_clock)&&dpk_values(p->saved_clock)&&dpk_coupled(p->saved,p->saved_clock)&&
  R32(p->saved,PP_SAVE_BYTES+3)==p->saved_clock[2];
}
/* Scope is metadata only: no source/native pointer lookup, caller-selected
 * offset or BOH/HBP receipt. The root kernel owns fresh lifetime and HG checks. */
u32 _gz_dpk_kernel_scope(u32 mode,u32 frame){const DpkPlan*p=&_gz_dpk_plan;const volatile u32*o=_gz_roh_owner;
 if(_gz_dpk_entered!=1||mode!=_gz_dpk_mode||frame!=_gz_dpk_frame||frame!=_gz_pbac_frame||
  !dpk_sealed()||!_gz_dck_state_context(p->token,frame)||o[0]!=ROH_MAGIC||o[1]!=240||o[2]!=2||
  o[4]!=1||o[5]!=p->token||_gz_roh_serial!=p->token||o[10]!=frame||!o[11])return 0;
 if(mode==2)return p->phase==DPK_PREPARED&&o[3]==1&&!o[6]&&!o[7]&&o[12]==1;
 if(mode==3)return _gz_dpk_active==p->token&&p->phase==DPK_APPLYING&&o[3]==2&&o[6]==1&&!o[7]&&o[12]==6;
 if(mode==4)return _gz_dpk_active==p->token&&p->phase==DPK_UNDOING&&o[3]==4&&o[6]==1&&o[7]&&(o[12]==2||o[12]==5||o[12]==6);
 if(mode!=5||_gz_dpk_active!=p->token)return 0;
 return (p->phase==DPK_PREPARED&&o[3]==1&&!o[6]&&!o[7]&&(o[12]==0||o[12]==1))||
  (p->phase==DPK_ROLLED&&o[3]==4&&o[6]==1&&o[7]&&(o[12]==2||o[12]==5||o[12]==6));
}
u32 _gz_dpk_kernel_token(u32 frame){return _gz_dpk_entered==1&&frame==_gz_dpk_frame&&frame==_gz_pbac_frame?_gz_dpk_plan.token:0;}
u32 _gz_dpk_kernel_value(u32 i,u32 kind){const DpkPlan*p=&_gz_dpk_plan;
 if(i>=PP_SAVE_BYTES+3||kind>2||_gz_dpk_entered!=1||_gz_dpk_frame!=_gz_pbac_frame||(_gz_dpk_mode!=3&&_gz_dpk_mode!=4))return 256;
 if(kind==2)return _gz_dpk_mode==3?p->saved[i]:p->before[i];
 return kind==1&&_gz_dpk_mode==4?p->saved[i]:p->before[i];
}
u32 _gz_dpk_kernel_clock(u32 i,u32 kind){const DpkPlan*p=&_gz_dpk_plan;
 if(i>=5||kind>2||_gz_dpk_entered!=1||_gz_dpk_frame!=_gz_pbac_frame||(_gz_dpk_mode!=3&&_gz_dpk_mode!=4))return 0x7fc00000u;
 if(kind==2)return _gz_dpk_mode==3?p->saved_clock[i]:p->before_clock[i];
 return kind==1&&_gz_dpk_mode==4?p->saved_clock[i]:p->before_clock[i];
}
static u32 dpk_source(u32 token,u32 frame,u32 forward){const DpkPlan*p=&_gz_dpk_plan;
 if(!dpk_sealed()||p->token!=token||!_gz_dck_state_gate(token,frame)||_gz_osa_validate(p->pin)||
  _gz_osa_pin_id()!=p->pin||_gz_osa_source(p->pin)!=p->source||_gz_osa_bytes(p->pin)!=p->used||
  p->source!=_gz_roh_owner[17]||p->used!=_gz_roh_owner[18])return 42;
 if(forward){if(_gz_roh_direct_state_common(token,frame,1))return 40;}
 else if(_gz_roh_gl_context(token,frame,1)!=1)return 42;
 if(R32(p->source,24)+p->source!=p->row||R32(p->source,224)+p->source!=p->clock_row||
  btr_hash((const void*)p->row,PP_ROW_BYTES)!=p->row_hash||btr_hash((const void*)p->clock_row,128)!=p->clock_hash||
  !btr_equal((const void*)(p->clock_row+52),p->saved_clock,20)||_gz_pp_validate(p->row,PP_ROW_BYTES)||
  _gz_ckb_archive_validate(p->source,p->used,1))return 42;
 return 0;
}
static u32 dpk_apart(u32 s){
 if(!span(s,0x200000u)||!prk_output_apart(s)||!_gz_dpk_root_private_apart(s,0x200000u))return 0;
#define DPK_APART(p,n) if(!prk_apart(s,0x200000u,(u32)(p),(n)))return 0
 DPK_APART(&_gz_dpk_plan,sizeof(DpkPlan));DPK_APART(&_gz_dpk_prepared_plan,sizeof(DpkPlan));
 DPK_APART(&_gz_dpk_applied_plan,sizeof(DpkPlan));DPK_APART(&_gz_dpk_rolled_plan,sizeof(DpkPlan));
 DPK_APART(&_gz_dpk_seal,4);DPK_APART(&_gz_dpk_active,4);DPK_APART(&_gz_dpk_entered,4);
 DPK_APART(&_gz_dpk_mode,4);DPK_APART(&_gz_dpk_frame,4);DPK_APART(_gz_dpk_report,128);
#undef DPK_APART
 return 1;
}
static void dpk_report(u32 index,u32 z){const DpkPlan*p=&_gz_dpk_plan;
 _gz_dpk_report[2]=p->token;_gz_dpk_report[3]=p->frame;_gz_dpk_report[index]=z;
 _gz_dpk_report[7]=p->root_receipt;_gz_dpk_report[8]=_gz_dpk_seal;
 _gz_dpk_report[9]=btr_hash(p->before,PP_SAVE_BYTES+3);_gz_dpk_report[10]=btr_hash(p->saved,PP_OUTPUT_BYTES);
 _gz_dpk_report[11]=btr_hash(p->before_clock,20);_gz_dpk_report[12]=btr_hash(p->saved_clock,20);
}
u32 _gz_dpk_prepare(u32 token,u32 frame,u32 source,u32 used,u32 pin){DpkPlan*p=&_gz_dpk_plan;u32 i,o,c,z;
 if(_gz_dpk_entered||_gz_dpk_active||p->valid||_gz_dpk_seal||!token||!_gz_dck_state_gate(token,frame)||
  _gz_roh_gl_context(token,frame,1)!=1||!pin||_gz_osa_pin_id()!=pin||_gz_osa_validate(pin)||
  _gz_osa_source(pin)!=source||_gz_osa_bytes(pin)!=used||!span(source,used)||used<232||
  R32(source,8)!=27||R32(source,12)!=27)return 28;
 if(!dpk_apart(source))return 73;
 o=R32(source,24);c=R32(source,224);
 if(o<232||o>used||R32(source,28)!=PP_ROW_BYTES||PP_ROW_BYTES>used-o||
  c<232||c>used||R32(source,228)!=128||128>used-c||_gz_ckb_archive_validate(source,used,1))return 42;
 dlr_zero(p,sizeof(*p));for(i=2;i<32;i++)_gz_dpk_report[i]=0;
 p->magic=0x44505031u;p->bytes=sizeof(*p);p->version=1;p->valid=1;p->phase=DPK_PREPARED;
 p->token=token;p->frame=frame;p->source=source;p->used=used;p->pin=pin;p->row=source+o;p->clock_row=source+c;
 p->row_hash=btr_hash((const void*)p->row,PP_ROW_BYTES);p->clock_hash=btr_hash((const void*)p->clock_row,128);
 for(i=0;i<5;i++)p->saved_clock[i]=R32(p->clock_row,52+i*4);
 z=_gz_pp_project(p->row,PP_ROW_BYTES,(u32)p->saved,PP_OUTPUT_BYTES);if(z)goto done;dpk_seal();
 z=dpk_source(token,frame,0);if(z)goto done;
 _gz_dpk_entered=1;_gz_dpk_mode=2;_gz_dpk_frame=frame;z=_gz_dpk_root_access(2);if(z)goto done;
 btr_copy(p->before,_gz_dpk_root_values,PP_SAVE_BYTES+3);btr_copy(p->before_clock,_gz_dpk_root_clocks,20);
 p->root_receipt=_gz_dpk_root_receipt(token,frame);dpk_seal();
 if(!p->root_receipt||!dpk_sealed()||!dpk_coupled(p->before,p->before_clock)){z=77;goto done;}
 z=dpk_source(token,frame,0);if(z)goto done;
 _gz_dpk_active=token;btr_copy(&_gz_dpk_prepared_plan,p,sizeof(*p));z=0;
done:
 dpk_report(4,z);_gz_dpk_entered=_gz_dpk_mode=_gz_dpk_frame=0;
 if(z){_gz_dpk_root_forget_leaf(token);dlr_zero(p,sizeof(*p));_gz_dpk_seal=_gz_dpk_active=0;}return z;
}
u32 _gz_dpk_apply(u32 token,u32 frame){DpkPlan*p=&_gz_dpk_plan;u32 z;
 if(_gz_dpk_entered||!dpk_sealed()||_gz_dpk_active!=token||p->token!=token||p->phase!=DPK_PREPARED)return 28;
 z=dpk_source(token,frame,1);if(z)return z;
 if(_gz_dpk_root_receipt(token,frame)!=p->root_receipt)return 84;
 _gz_dpk_entered=1;_gz_dpk_mode=3;_gz_dpk_frame=frame;p->phase=DPK_APPLYING;p->applied_frame=frame;dpk_seal();
 z=_gz_dpk_root_access(3);if(!z)z=dpk_source(token,frame,1);
 if(!z)p->phase=DPK_APPLIED;p->error=z;dpk_seal();btr_copy(&_gz_dpk_applied_plan,p,sizeof(*p));dpk_report(5,z);
 _gz_dpk_entered=_gz_dpk_mode=_gz_dpk_frame=0;return z;
}
u32 _gz_dpk_rollback(u32 token,u32 frame){DpkPlan*p=&_gz_dpk_plan;u32 z;
 if(_gz_dpk_entered||!dpk_sealed()||_gz_dpk_active!=token||p->token!=token||p->phase==DPK_ROLLED)return 28;
 if(!_gz_dck_state_gate(token,frame)||_gz_roh_direct_recovery_common(token,frame)||_gz_dpk_root_receipt(token,frame)!=p->root_receipt)return 40;
 _gz_dpk_entered=1;_gz_dpk_mode=4;_gz_dpk_frame=frame;p->phase=DPK_UNDOING;dpk_seal();
 z=_gz_dpk_root_access(4);if(!z)z=_gz_roh_direct_recovery_common(token,frame);
 if(!z){p->phase=DPK_ROLLED;p->rolled_frame=frame;}p->error=z;dpk_seal();btr_copy(&_gz_dpk_rolled_plan,p,sizeof(*p));dpk_report(6,z);
 _gz_dpk_entered=_gz_dpk_mode=_gz_dpk_frame=0;return z;
}
u32 _gz_dpk_prepared_exact(u32 token,u32 frame){DpkPlan*p=&_gz_dpk_plan;u32 z;
 if(!_gz_dpk_active)return !p->valid&&!_gz_dpk_seal;
 if(_gz_dpk_entered||!dpk_sealed()||p->token!=token||p->phase!=DPK_PREPARED||!_gz_dck_state_gate(token,frame))return 0;
 _gz_dpk_entered=1;_gz_dpk_mode=5;_gz_dpk_frame=frame;z=_gz_dpk_root_access(5);
 _gz_dpk_entered=_gz_dpk_mode=_gz_dpk_frame=0;return z==0;
}
u32 _gz_dpk_closure_key(u32 token,u32 frame){const DpkPlan*p=&_gz_dpk_plan;u32 key[8],root;
 if(_gz_dpk_entered||!dpk_sealed()||_gz_dpk_active!=token||p->token!=token||p->frame!=frame||p->phase!=DPK_ROLLED||p->error)return 0;
 root=_gz_dpk_root_finish(token,frame);if(!root)return 0;
 key[0]=0x44505331u;key[1]=token;key[2]=frame;key[3]=_gz_dpk_seal;key[4]=p->root_receipt;key[5]=root;
 key[6]=btr_hash(p->before,PP_SAVE_BYTES+3);key[7]=btr_hash(p->before_clock,20);_gz_dpk_report[13]=root;
 return btr_hash(key,sizeof(key));
}
void _gz_dpk_retire_leaf(void){_gz_dpk_root_forget_leaf(_gz_dpk_plan.token);dlr_zero(&_gz_dpk_plan,sizeof(_gz_dpk_plan));
 _gz_dpk_seal=_gz_dpk_active=_gz_dpk_entered=_gz_dpk_mode=_gz_dpk_frame=0;}
