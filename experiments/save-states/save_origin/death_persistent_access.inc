/* One fixed persistent+clock domain INSIDE the real DCK allocation lease.
 * No nested HG acquisition and no caller-selected native pointer. Existing
 * standalone SPK/SCK kernels and their authority remain unchanged. */
#define DPK_ROOT_BYTES (PP_SAVE_BYTES+3u)
extern SpoU32 _gz_dpk_kernel_scope(SpoU32,SpoU32),_gz_dpk_kernel_token(SpoU32);
extern SpoU32 _gz_dpk_kernel_value(SpoU32,SpoU32),_gz_dpk_kernel_clock(SpoU32,SpoU32);
extern SpoU32 _gz_dck_state_gate(SpoU32,SpoU32);
typedef struct {
 SpoU32 magic,bytes,valid,token,frame,binding_hash,save_hash,equip_hash,env_hash,receipt,closure,last_mode;
 SrhBinding binding;
} DpkRootRole;
typedef char DpkRootRoleSize[sizeof(DpkRootRole)==524?1:-1];
static DpkRootRole _gz_dpk_root_role;
static SpoU32 _gz_dpk_root_seal,_gz_dpk_root_live,_gz_dpk_root_entered;
unsigned char _gz_dpk_root_values[DPK_ROOT_BYTES];
SpoU32 _gz_dpk_root_clocks[5],_gz_dpk_root_report[32]={0x44505231u,128};
unsigned char _gz_dpk_save_before[SRO_SAVE_BYTES],_gz_dpk_save_applied[SRO_SAVE_BYTES],_gz_dpk_save_rolled[SRO_SAVE_BYTES];
unsigned char _gz_dpk_equip_before[32],_gz_dpk_equip_applied[32],_gz_dpk_equip_rolled[32];
unsigned char _gz_dpk_env_before[128],_gz_dpk_env_applied[128],_gz_dpk_env_rolled[128];
static unsigned char _gz_dpk_root_expected[DPK_ROOT_BYTES],_gz_dpk_root_alternate[DPK_ROOT_BYTES],_gz_dpk_root_target[DPK_ROOT_BYTES];
static void dps_seal(void){_gz_dpk_root_seal=srh_hash(&_gz_dpk_root_role,sizeof(_gz_dpk_root_role));}
static SpoU32 dps_sealed(void){const DpkRootRole*r=&_gz_dpk_root_role;
 return r->magic==0x44505232u&&r->bytes==sizeof(*r)&&r->valid==1&&r->token&&r->receipt&&
  _gz_dpk_root_seal==srh_hash(r,sizeof(*r))&&r->binding_hash==srh_hash(&r->binding,sizeof(r->binding))&&
  r->save_hash==srh_hash(_gz_dpk_save_before,SRO_SAVE_BYTES)&&r->equip_hash==srh_hash(_gz_dpk_equip_before,32)&&
  r->env_hash==srh_hash(_gz_dpk_env_before,128);
}
static SpoU32 dps_equal(const void*a,const void*b,SpoU32 n){SpoU32 i;for(i=0;i<n;i++)if(((const unsigned char*)a)[i]!=((const unsigned char*)b)[i])return 0;return 1;}
static SpoU32 dps_binding(void){SpoU32 z=srh_current();
 if(z)return z;
 return srh_hash(&_gz_srh_binding,sizeof(_gz_srh_binding))==_gz_dpk_root_role.binding_hash&&
  dps_equal(&_gz_srh_binding,&_gz_dpk_root_role.binding,sizeof(_gz_srh_binding))?0:84;
}
static void dps_values(SpoU32 root){SpoU32 i;
 for(i=0;i<DPK_ROOT_BYTES;i++)_gz_dpk_root_values[i]=*(volatile unsigned char*)spk_address(root,i);
 _gz_dpk_root_clocks[0]=*(volatile SpoU32*)(root+0x44);_gz_dpk_root_clocks[1]=*(volatile unsigned short*)(root+0x48);
 _gz_dpk_root_clocks[2]=*(volatile SpoU32*)SPO_CLOCK_A;_gz_dpk_root_clocks[3]=*(volatile unsigned short*)SPO_CLOCK_SHORT;
 _gz_dpk_root_clocks[4]=*(volatile SpoU32*)SPO_CLOCK_B;
}
static void dps_snapshot(SpoU32 root,unsigned char*save,unsigned char*equip,unsigned char*env){
 srh_copy(save,(const void*)(root+SRO_SAVE_OFFSET),SRO_SAVE_BYTES);
 srh_copy(equip,(const void*)SPO_EQUIPMENT,32);srh_copy(env,(const void*)SPO_ENVIRONMENT,128);
}
static SpoU32 dps_before(void){SpoU32 i,root=_gz_srh_binding.root;
 for(i=0;i<SRO_SAVE_BYTES;i++)if(*(volatile unsigned char*)(root+SRO_SAVE_OFFSET+i)!=_gz_dpk_save_before[i])return 77;
 for(i=0;i<32;i++)if(*(volatile unsigned char*)(SPO_EQUIPMENT+i)!=_gz_dpk_equip_before[i])return 77;
 for(i=0;i<128;i++)if(*(volatile unsigned char*)(SPO_ENVIRONMENT+i)!=_gz_dpk_env_before[i])return 77;
 return 0;
}
static SpoU32 dps_apart(SpoU32 a,SpoU32 n,SpoU32 b,SpoU32 m){return a>=b?a-b>=m:b-a>=n;}
SpoU32 _gz_dpk_root_private_apart(SpoU32 a,SpoU32 n){
 if(!n||a+n<a)return 0;
#define DPS_APART(p,m) if(!dps_apart(a,n,(SpoU32)(p),(m)))return 0
 DPS_APART(&_gz_dpk_root_role,sizeof(DpkRootRole));DPS_APART(&_gz_dpk_root_seal,4);DPS_APART(&_gz_dpk_root_live,4);DPS_APART(&_gz_dpk_root_entered,4);
 DPS_APART(_gz_dpk_root_values,DPK_ROOT_BYTES);DPS_APART(_gz_dpk_root_clocks,20);DPS_APART(_gz_dpk_root_report,128);
 DPS_APART(_gz_dpk_save_before,SRO_SAVE_BYTES);DPS_APART(_gz_dpk_save_applied,SRO_SAVE_BYTES);DPS_APART(_gz_dpk_save_rolled,SRO_SAVE_BYTES);
 DPS_APART(_gz_dpk_equip_before,32);DPS_APART(_gz_dpk_equip_applied,32);DPS_APART(_gz_dpk_equip_rolled,32);
 DPS_APART(_gz_dpk_env_before,128);DPS_APART(_gz_dpk_env_applied,128);DPS_APART(_gz_dpk_env_rolled,128);
 DPS_APART(_gz_dpk_root_expected,DPK_ROOT_BYTES);DPS_APART(_gz_dpk_root_alternate,DPK_ROOT_BYTES);DPS_APART(_gz_dpk_root_target,DPK_ROOT_BYTES);
#undef DPS_APART
 return 1;
}
/* Certificate-only final check: srh_current reads the wrapped origin/lifetime
 * observer and static root slot, not the allocated save bytes. Native byte
 * equality was established while the real outer HG lease was held. */
SpoU32 _gz_dpk_root_finish(SpoU32 token,SpoU32 frame){const DpkRootRole*r=&_gz_dpk_root_role;
 if(_gz_dpk_root_entered||_gz_dpk_root_live!=token||!dps_sealed()||r->token!=token||r->frame!=frame||
  !r->closure||(r->last_mode!=4&&r->last_mode!=5)||dps_binding())return 0;
 return r->closure;
}
SpoU32 _gz_dpk_root_receipt(SpoU32 token,SpoU32 frame){const DpkRootRole*r=&_gz_dpk_root_role;
 return !_gz_dpk_root_entered&&_gz_dpk_root_live==token&&dps_sealed()&&r->token==token&&r->frame==frame?r->receipt:0;
}
void _gz_dpk_root_forget_leaf(SpoU32 token){if(!_gz_dpk_root_entered&&_gz_dpk_root_live==token)_gz_dpk_root_live=0;}
SpoU32 _gz_dpk_root_access(SpoU32 mode){DpkRootRole*r=&_gz_dpk_root_role;
 SpoU32 frame=_gz_pbac_frame,token,gate,z=0,i,x,root,ct[5],ce[5],ca[5],key[6];
 if(_gz_dpk_root_entered||_gz_spk_entered||_gz_sck_entered||_gz_rkc_entered||_gz_dth_entered||_gz_srh_probe_entered||
  _gz_gqk_access_entered()||_gz_doh_access_entered()||!_gz_dck_access_entered()||mode<2||mode>5||
  !_gz_dpk_kernel_scope(mode,frame)||!srh_sealed())return 28;
 token=_gz_dpk_kernel_token(frame);gate=_gz_dck_state_gate(token,frame);if(!token||!gate)return 28;
 if(_gz_hg_capture_seal(gate))return 86;
 _gz_dpk_root_entered=1;for(i=2;i<32;i++)_gz_dpk_root_report[i]=0;
 _gz_dpk_root_report[2]=token;_gz_dpk_root_report[3]=frame;_gz_dpk_root_report[4]=mode;
 z=srh_current();if(z)goto done;z=spk_table();if(z)goto done;root=_gz_srh_binding.root;
 if(mode==2){
  if(_gz_dpk_root_live||(r->valid&&token<=r->token)){z=28;goto done;}
  r->valid=0;r->magic=0x44505232u;r->bytes=sizeof(*r);r->token=token;r->frame=frame;r->closure=0;r->last_mode=2;
  srh_copy(&r->binding,&_gz_srh_binding,sizeof(r->binding));r->binding_hash=srh_hash(&r->binding,sizeof(r->binding));
  dps_snapshot(root,_gz_dpk_save_before,_gz_dpk_equip_before,_gz_dpk_env_before);
  r->save_hash=srh_hash(_gz_dpk_save_before,SRO_SAVE_BYTES);r->equip_hash=srh_hash(_gz_dpk_equip_before,32);r->env_hash=srh_hash(_gz_dpk_env_before,128);
  key[0]=0x44504231u;key[1]=token;key[2]=frame;key[3]=r->binding_hash;key[4]=r->save_hash;key[5]=r->equip_hash^r->env_hash;
  r->receipt=srh_hash(key,sizeof(key));if(!r->receipt){z=77;goto done;}
  r->valid=1;_gz_dpk_root_live=token;dps_seal();
 }else if(!dps_sealed()||_gz_dpk_root_live!=token||r->token!=token||r->frame!=frame||dps_binding()){z=84;goto done;}
 dps_values(root);
 if(mode==3||mode==4){
  for(i=0;i<DPK_ROOT_BYTES;i++){
   x=_gz_dpk_kernel_value(i,0);if(x>255){z=76;goto done;}_gz_dpk_root_expected[i]=(unsigned char)x;
   x=_gz_dpk_kernel_value(i,1);if(x>255){z=76;goto done;}_gz_dpk_root_alternate[i]=(unsigned char)x;
   x=_gz_dpk_kernel_value(i,2);if(x>255){z=76;goto done;}_gz_dpk_root_target[i]=(unsigned char)x;
  }
  for(i=0;i<5;i++){ce[i]=_gz_dpk_kernel_clock(i,0);ca[i]=_gz_dpk_kernel_clock(i,1);ct[i]=_gz_dpk_kernel_clock(i,2);}
  if(!sck_values_safe(ce)||!sck_values_safe(ca)||!sck_values_safe(ct)){z=77;goto done;}
  for(i=0;i<DPK_ROOT_BYTES;i++)if(!spk_skip(i)&&_gz_dpk_root_values[i]!=_gz_dpk_root_expected[i]&&_gz_dpk_root_values[i]!=_gz_dpk_root_alternate[i]){z=77;_gz_dpk_root_report[6]=i+1;goto done;}
  for(i=0;i<5;i++)if(_gz_dpk_root_clocks[i]!=ce[i]&&_gz_dpk_root_clocks[i]!=ca[i]){z=77;_gz_dpk_root_report[7]=i+1;goto done;}
  z=dps_binding();if(z)goto done;z=_gz_hg_capture_seal(gate);if(z)goto done;
  if(!_gz_dpk_kernel_scope(mode,frame)){z=5;goto done;}
  /* All destinations are fixed. No source/native/SDK call or yield between
   * the prevalidated persistent-byte stores and five clock stores. */
  for(i=0;i<DPK_ROOT_BYTES;i++)if(!spk_skip(i)){*(volatile unsigned char*)spk_address(root,i)=_gz_dpk_root_target[i];_gz_dpk_root_report[8]++;}
  *(volatile SpoU32*)(root+0x44)=ct[0];*(volatile unsigned short*)(root+0x48)=(unsigned short)ct[1];
  *(volatile SpoU32*)SPO_CLOCK_A=ct[2];*(volatile unsigned short*)SPO_CLOCK_SHORT=(unsigned short)ct[3];*(volatile SpoU32*)SPO_CLOCK_B=ct[4];
  _gz_dpk_root_report[9]=31;dps_values(root);
  for(i=0;i<DPK_ROOT_BYTES;i++)if(!spk_skip(i)&&_gz_dpk_root_values[i]!=_gz_dpk_root_target[i]){z=77;goto done;}
  for(i=0;i<5;i++)if(_gz_dpk_root_clocks[i]!=ct[i]){z=77;goto done;}
  if(mode==3)dps_snapshot(root,_gz_dpk_save_applied,_gz_dpk_equip_applied,_gz_dpk_env_applied);
 }
 if(mode==4||mode==5){
  z=dps_before();if(z)goto done;
  if(mode==4)dps_snapshot(root,_gz_dpk_save_rolled,_gz_dpk_equip_rolled,_gz_dpk_env_rolled);
  key[0]=0x44504332u;key[1]=token;key[2]=frame;key[3]=r->receipt;key[4]=r->binding_hash;key[5]=r->save_hash^r->equip_hash^r->env_hash;
  r->closure=srh_hash(key,sizeof(key));if(!r->closure){z=77;goto done;}
 }
 r->last_mode=mode;dps_seal();z=dps_binding();if(z)goto done;
 z=_gz_hg_capture_seal(gate);if(!z&&!_gz_dpk_kernel_scope(mode,frame))z=5;
done:
 _gz_dpk_root_report[5]=z;_gz_dpk_root_report[10]=r->receipt;_gz_dpk_root_report[11]=r->closure;
 _gz_dpk_root_report[12]=srh_hash(_gz_dpk_root_values,DPK_ROOT_BYTES);_gz_dpk_root_report[13]=srh_hash(_gz_dpk_root_clocks,20);
 _gz_dpk_root_report[14]=_gz_hl_control.flags;_gz_dpk_root_report[15]=_gz_hg_control[15];_gz_dpk_root_entered=0;return z;
}
