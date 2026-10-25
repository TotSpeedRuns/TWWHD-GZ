extern u32 _gz_bci_read_scope(u32,u32);
/* One owner-held clock domain. Native heap access is confined to the fixed
 * save-root kernel; this participant never accepts a native destination. */
extern u32 _gz_sck_values[8],_gz_sck_owner_read(void),_gz_sck_owner_apply(void),_gz_sck_owner_undo(void),_gz_sck_owner_check(void);
extern u32 _gz_srd_read_clock(u32,u32),_gz_ckb_archive_validate(u32,u32,u32);
enum {CKR_PREPARED=1,CKR_APPLYING=2,CKR_APPLIED=3,CKR_UNDOING=4,CKR_ROLLED=5};
typedef struct {
 u32 magic,bytes,version,valid,phase,token,frame,source,used,pin,row,row_hash;
 u32 before[8],saved[8],applied_frame,rolled_frame,error,count;
} CkrPlan;
typedef char CkrSize[sizeof(CkrPlan)==128?1:-1];
static CkrPlan _gz_ckr_plan;
static u32 _gz_ckr_seal,_gz_ckr_entered,_gz_ckr_kernel_mode,_gz_ckr_kernel_frame;
u32 _gz_ckr_report[32]={0x434b5232u,128};
static void ckr_seal(void){_gz_ckr_seal=btr_hash(&_gz_ckr_plan,sizeof(CkrPlan));}
static u32 ckr_values(const u32*p,u32 n){return (n==5||n==8)&&finite(p[0])&&p[1]<=65535&&finite(p[2])&&p[3]<=65535&&finite(p[4])&&(n==5||p[7]<=255);}
static u32 ckr_sealed(void){const CkrPlan*p=&_gz_ckr_plan;
 return p->magic==0x434b5232u&&p->bytes==sizeof(*p)&&p->version==1&&p->valid==1&&p->token&&
  p->phase>=1&&p->phase<=5&&ckr_values(p->before,p->count)&&ckr_values(p->saved,p->count)&&
  _gz_ckr_seal==btr_hash(p,sizeof(*p));
}
static void ckr_report(u32 z){u32 i;const CkrPlan*p=&_gz_ckr_plan;
 _gz_ckr_report[2]=p->token;_gz_ckr_report[3]=p->phase;_gz_ckr_report[4]=z;
 for(i=0;i<5;i++){_gz_ckr_report[5+i]=p->before[i];_gz_ckr_report[10+i]=p->saved[i];}
 _gz_ckr_report[15]=p->applied_frame;_gz_ckr_report[16]=p->rolled_frame;
 for(i=0;i<3;i++){_gz_ckr_report[17+i]=p->before[5+i];_gz_ckr_report[20+i]=p->saved[5+i];}_gz_ckr_report[23]=p->count;
}
/* Metadata-only authentication inside the heap gate. Full original source,
 * owner and recovery validations run before/after the kernel call. This query
 * follows no source, native actor, resource or saved pointer. */
/* Recovery may retain an active service ID only after the original
 * callback returned and its sealed failure receipt still matches the owner.
 * This does not admit a running callback or any forward writer. */
u32 _gz_ckr_kernel_scope(u32 mode,u32 frame){const CkrPlan*p=&_gz_ckr_plan;const volatile u32*o=_gz_roh_owner;
 if(_gz_ckr_entered!=1||mode!=_gz_ckr_kernel_mode||frame!=_gz_ckr_kernel_frame||frame!=_gz_pbac_frame||
  !ckr_sealed()||o[0]!=ROH_MAGIC||o[1]!=240||o[2]!=2||o[4]!=1||o[5]!=p->token||_gz_roh_serial!=p->token)return 0;
 if(mode==2)return p->phase==CKR_PREPARED&&o[3]==ROH_CLEAN&&!o[6]&&!o[7]&&o[11]&&o[12]==1&&o[10]==frame;
 if(tx_core_has(p->token)){
  if(mode==5&&p->phase==CKR_APPLIED&&_gz_wct_target_scope(p->token,frame))return 1;
  if(mode==3)return p->phase==CKR_APPLYING&&tx_core_forward(p->token,frame,6);
  if(mode==4)return p->phase==CKR_UNDOING&&tx_core_recovery(p->token,frame);
  if(mode==5)return (p->phase==CKR_PREPARED||p->phase==CKR_ROLLED)&&tx_core_recovery(p->token,frame);
  return 0;
 }
 if(mode==3)return p->phase==CKR_APPLYING&&o[3]==ROH_MUTATING&&o[6]==1&&!o[7]&&o[11]&&o[12]==6&&o[10]==frame&&_gz_bmh_scope(p->token,frame);
 if(mode==5&&p->phase==CKR_APPLIED&&_gz_bci_read_scope(p->token,frame))return 1;
 if(mode!=4&&mode!=5)return 0;
 if(o[3]!=ROH_FAILED_DIRTY||o[6]!=1||!o[7]||
  (o[12]&&_gz_bmh_aborted_scope(p->token)!=1&&_gz_boh_lifetime_aborted_scope(p->token)!=1)||!bor_sealed()||_gz_boh_recovery.token!=p->token)return 0;
 if(mode==4)return p->phase==CKR_UNDOING&&_gz_boh_recovery.phase==BOR_UNDO;
 return (p->phase==CKR_PREPARED||p->phase==CKR_ROLLED)&&
  (_gz_boh_recovery.phase==BOR_UNDO||_gz_boh_recovery.phase==BOR_DONE);
}
u32 _gz_ckr_kernel_count(void){return _gz_ckr_kernel_scope(_gz_ckr_kernel_mode,_gz_pbac_frame)?_gz_ckr_plan.count:0;}
u32 _gz_ckr_kernel_expected(u32 i,u32 alternate){
 if(i>=_gz_ckr_plan.count||!_gz_ckr_kernel_scope(_gz_ckr_kernel_mode,_gz_pbac_frame))return 0x7fc00000u;
 return _gz_ckr_kernel_mode==4&&alternate?_gz_ckr_plan.saved[i]:_gz_ckr_plan.before[i];
}
u32 _gz_ckr_kernel_target(u32 i){
 if(i>=_gz_ckr_plan.count||!_gz_ckr_kernel_scope(_gz_ckr_kernel_mode,_gz_pbac_frame))return 0x7fc00000u;
 return _gz_ckr_kernel_mode==3?_gz_ckr_plan.saved[i]:_gz_ckr_plan.before[i];
}
static u32 ckr_coupled(u32 source,u32 used,const u32*saved){u32 row,o,n,i,j,v[6],found=0,z;
 if(!span(source,used)||used<232||(R32(source,8)!=27&&R32(source,8)!=28&&R32(source,8)!=29&&R32(source,8)!=30)||R32(source,12)!=R32(source,8))return 42;
 o=R32(source,24);n=R32(source,28);if(o<232||o>used||n>used-o||n!=PP_ROW_BYTES)return 42;row=source+o;
 if(btr_hash(&_gz_pp_fixed,sizeof(_gz_pp_fixed))!=0xba41675cu)return 42;
 z=_gz_pp_validate(row,n);if(z)return z;
 for(i=0;i<PP_SAVE_BYTES;i++){
  o=R32(row,24+12*i);if(o>=36&&o<42){j=o-36;if(found&(1u<<j))return 42;v[j]=R32(row,28+12*i);found|=1u<<j;}
 }
 if(found!=63||((v[0]<<24)|(v[1]<<16)|(v[2]<<8)|v[3])!=saved[0]||((v[4]<<8)|v[5])!=saved[1]||
  R32(row,28+(PP_SAVE_BYTES+3)*12)!=saved[2])return 77;
 return 0;
}
static u32 ckr_source(u32 token,u32 frame,u32 service){const CkrPlan*p=&_gz_ckr_plan;u32 row;
 if(!ckr_sealed()||p->token!=token)return 42;
 if(service&&tx_core_has(token)){
  if(!tx_core_forward(token,frame,_gz_roh_owner[12]))return 42;
 }else if(service){
  if(!_gz_hbp_service_read(token,frame)||p->source!=_gz_hbp_recipe.source||
   p->used!=_gz_hbp_recipe.used||p->pin!=_gz_hbp_recipe.pin)return 42;
 }else if(!tx_observe_matches(token,frame,p->source,p->used,p->pin))return 42;
 if(_gz_ckb_archive_validate(p->source,p->used,1))return 42;
 row=p->source+R32(p->source,224);
 if(row!=p->row||btr_hash((const void*)row,128)!=p->row_hash||!btr_equal((const void*)(row+52),p->saved,20)||
  (R32(row,8)==2?8u:5u)!=p->count||(p->count==8&&!btr_equal((const void*)(row+76),p->saved+5,12)))return 42;
 return ckr_coupled(p->source,p->used,p->saved);
}
u32 _gz_ckr_prepare(u32 token,u32 frame){CkrPlan*p=&_gz_ckr_plan;u32 row,z=0,i,source,used;const TxObserveSource*v;
 if(_gz_ckr_entered||p->valid||_gz_ckr_seal)return 28;
 if(!token||_gz_roh_gl_context(token,frame,1)!=1||!(v=tx_observe_source(token,frame)))return 42;
 row=_gz_srd_read_clock(token,frame);source=v->source;used=v->used;
 if(!row||!span(source,used)||used<232||(R32(source,8)!=27&&R32(source,8)!=28&&R32(source,8)!=29&&R32(source,8)!=30)||R32(source,12)!=R32(source,8)||
  R32(source,228)!=128||row!=source+R32(source,224)||_gz_ckb_archive_validate(source,used,1))return 42;
 for(i=0;i<sizeof(*p)/4;i++)((u32*)p)[i]=0;
 p->magic=0x434b5232u;p->bytes=sizeof(*p);p->version=1;p->valid=1;p->phase=CKR_PREPARED;p->token=token;p->frame=frame;
 p->source=source;p->used=used;p->pin=v->pin;p->row=row;p->row_hash=btr_hash((const void*)row,128);
 p->count=R32(row,8)==2?8:5;
 for(i=0;i<5;i++)p->saved[i]=R32(row,52+i*4);
 if(p->count==8)for(i=0;i<3;i++)p->saved[5+i]=R32(row,76+i*4);ckr_seal();
 z=ckr_source(token,frame,0);if(z)goto done;
 _gz_ckr_entered=1;_gz_ckr_kernel_mode=2;_gz_ckr_kernel_frame=frame;
 z=_gz_sck_owner_read();if(z)goto done;
 for(i=0;i<p->count;i++)p->before[i]=_gz_sck_values[i];ckr_seal();
 if(!ckr_sealed()){z=77;goto done;}
 z=ckr_source(token,frame,0);if(!z&&_gz_roh_gl_context(token,frame,1)!=1)z=42;
done:
 ckr_report(z);_gz_ckr_entered=_gz_ckr_kernel_mode=_gz_ckr_kernel_frame=0;
 /* Preparation has issued no native stores. Its own failed private plan can
  * be discarded without clearing an owner, pin, error or foreign receipt. */
 if(z){for(i=0;i<sizeof(*p)/4;i++)((u32*)p)[i]=0;_gz_ckr_seal=0;}return z;
}
static u32 ckr_forward(u32 token,u32 frame){
 if(tx_core_has(token)){if(!tx_core_forward(token,frame,6))return 49;}
 else if(_gz_roh_owner[12]!=6||!_gz_bmh_scope(token,frame)||_gz_roh_replay_common(token,frame,0))return 49;
 return ckr_source(token,frame,1);
}
u32 _gz_ckr_apply(u32 token){CkrPlan*p=&_gz_ckr_plan;u32 frame=_gz_pbac_frame,z;
 if(_gz_ckr_entered||!ckr_sealed()||p->token!=token||p->phase!=CKR_PREPARED)return 28;
 z=ckr_forward(token,frame);if(z)return z;
 _gz_ckr_entered=1;_gz_ckr_kernel_mode=3;_gz_ckr_kernel_frame=frame;
 p->phase=CKR_APPLYING;p->applied_frame=frame;ckr_seal();
 z=_gz_sck_owner_apply();if(!z)z=ckr_forward(token,frame);
 if(!z)p->phase=CKR_APPLIED;p->error=z;ckr_seal();ckr_report(z);
 _gz_ckr_entered=_gz_ckr_kernel_mode=_gz_ckr_kernel_frame=0;return z;
}
u32 _gz_ckr_recovery_rollback(u32 token){CkrPlan*p=&_gz_ckr_plan;u32 frame=_gz_pbac_frame,z,prepared,i;
 if(_gz_ckr_entered||!ckr_sealed()||p->token!=token||p->phase==CKR_ROLLED)return 28;
 if(tx_core_has(token)){if(!tx_core_recovery(token,frame))return 40;}
 else if(!bor_original()||_gz_boh_recovery.phase!=BOR_UNDO||_gz_boh_recovery.token!=token)return 40;
 z=tx_recovery_context(token,frame);if(z)return z;prepared=p->phase==CKR_PREPARED;
 _gz_ckr_entered=1;_gz_ckr_kernel_mode=prepared?5:4;_gz_ckr_kernel_frame=frame;
 if(!prepared){p->phase=CKR_UNDOING;ckr_seal();z=_gz_sck_owner_undo();}
 else{
  z=_gz_sck_owner_check();if(!z)for(i=0;i<p->count;i++)if(_gz_sck_values[i]!=p->before[i]){z=77;break;}
 }
 if(!z)z=tx_recovery_context(token,frame);
 if(!z){p->phase=CKR_ROLLED;p->rolled_frame=frame;p->error=0;}else p->error=z;
 ckr_seal();ckr_report(z);_gz_ckr_entered=_gz_ckr_kernel_mode=_gz_ckr_kernel_frame=0;return z;
}
u32 _gz_ckr_recovery_check(u32 token,u32 frame){CkrPlan*p=&_gz_ckr_plan;u32 z,i;
 if(_gz_ckr_entered||!ckr_sealed()||p->token!=token||p->phase!=CKR_ROLLED||p->error||(tx_core_has(token)?!tx_core_recovery(token,frame):(!bor_original()||_gz_boh_recovery.phase!=BOR_DONE)))return 77;
 z=tx_recovery_context(token,frame);if(z)return z;
 _gz_ckr_entered=1;_gz_ckr_kernel_mode=5;_gz_ckr_kernel_frame=frame;z=_gz_sck_owner_check();
 if(!z)for(i=0;i<p->count;i++)if(_gz_sck_values[i]!=p->before[i]){z=77;break;}
 if(!z)z=tx_recovery_context(token,frame);
 _gz_ckr_entered=_gz_ckr_kernel_mode=_gz_ckr_kernel_frame=0;return z;
}
