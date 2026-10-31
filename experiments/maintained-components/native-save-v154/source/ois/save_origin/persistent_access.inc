/* Fixed persistent byte domain under the genuine current save-root lease.
 * The six canonical clock bytes belong exclusively to the clock participant.
 * The exact frozen table selects every other save byte and three fixed equips.
 * No archive offset or caller pointer can select a native destination. */
#include "../persistent_projection.h"
#define SPK_BYTES (PP_SAVE_BYTES+3u)
unsigned char _gz_spk_values[SPK_BYTES];
static unsigned char _gz_spk_target[SPK_BYTES],_gz_spk_expected[SPK_BYTES],_gz_spk_alternate[SPK_BYTES];
SpoU32 _gz_spk_report[20]={0x53504b31u,80};

SpoU32 _gz_spk_private_apart(SpoU32 a,SpoU32 n){SpoU32 p[]={
 (SpoU32)_gz_spk_values,(SpoU32)_gz_spk_target,(SpoU32)_gz_spk_expected,(SpoU32)_gz_spk_alternate};SpoU32 i;
 SpoU32 q[]={(SpoU32)_gz_spk_report,(SpoU32)&_gz_spk_entered,(SpoU32)&_gz_spk_frame,
  (SpoU32)&_gz_spk_thread,(SpoU32)&_gz_spk_serial,(SpoU32)&_gz_spk_mode};
 if(!n||a+n<a)return 0;
 for(i=0;i<4;i++)if(a<p[i]+SPK_BYTES&&p[i]<a+n)return 0;
 for(i=0;i<6;i++)if(a<q[i]+(i?4:80)&&q[i]<a+n)return 0;
 return 1;
}
static SpoU32 spk_skip(SpoU32 i){return i<PP_SAVE_BYTES&&_gz_pp_fixed.offsets[i]>=36&&_gz_pp_fixed.offsets[i]<42;}
static SpoU32 spk_address(SpoU32 root,SpoU32 i){return i<PP_SAVE_BYTES?root+0x20+_gz_pp_fixed.offsets[i]:SPO_EQUIPPED_ITEMS+i-PP_SAVE_BYTES;}
static SpoU32 spk_table(void){SpoU32 i,last=0,count=0,o;
 if(srh_hash(&_gz_pp_fixed,sizeof(_gz_pp_fixed))!=0xba41675cu)return 75;
 for(i=0;i<PP_SAVE_BYTES;i++){
  o=_gz_pp_fixed.offsets[i];if((i&&o<=last)||o>=0x1258)return 75;last=o;
  if(o>=36&&o<42)count++;
 }return count==6?0:75;
}
static void spk_read(SpoU32 root){SpoU32 i;
 for(i=0;i<SPK_BYTES;i++)_gz_spk_values[i]=*(volatile unsigned char*)spk_address(root,i);
}
static __attribute__((noinline,noclone)) SpoU32 spk_access(SpoU32 mode){SpoU32 frame=_gz_pbac_frame,gate=0,z=0,i,x,root=0;
 for(i=2;i<20;i++)_gz_spk_report[i]=0;
 if(_gz_spk_entered||_gz_sck_entered||_gz_gac_root_entered||_gz_rkc_entered||_gz_dth_entered||_gz_gqk_access_entered()||_gz_dck_access_entered()||_gz_srh_probe_entered||mode<2||mode>5||
  !_gz_prk_kernel_scope(mode,frame)||!srh_sealed()||_gz_spk_serial==0xffffffffu)return 28;
 _gz_spk_serial++;_gz_spk_frame=frame;_gz_spk_thread=_gz_hg_platform_thread();_gz_spk_mode=mode;_gz_spk_entered=1;
 _gz_spk_report[2]=frame;_gz_spk_report[4]=_gz_spk_serial;_gz_spk_report[13]=mode;
 gate=_gz_hg_capture_begin(_gz_spk_serial);if(!gate){z=86;goto done;}
 z=_gz_hg_capture_seal(gate);if(z)goto done;
 z=srh_current();if(z)goto done;z=spk_table();if(z)goto done;root=_gz_srh_binding.root;
 spk_read(root);
 if(mode==3||mode==4){
  /* Materialize and validate ALL fixed scalar inputs before any store. The
   * participant authenticates the full private plan at each scope boundary. */
  for(i=0;i<SPK_BYTES;i++){
   x=_gz_prk_kernel_value(i,0);if(x>255){z=76;goto done;}_gz_spk_expected[i]=(unsigned char)x;
   x=_gz_prk_kernel_value(i,1);if(x>255){z=76;goto done;}_gz_spk_alternate[i]=(unsigned char)x;
   x=_gz_prk_kernel_value(i,2);if(x>255){z=76;goto done;}_gz_spk_target[i]=(unsigned char)x;
  }
  for(i=0;i<SPK_BYTES;i++)if(!spk_skip(i)&&_gz_spk_values[i]!=_gz_spk_expected[i]&&
    _gz_spk_values[i]!=_gz_spk_alternate[i]){z=77;_gz_spk_report[15]=i+1;goto done;}
  z=srh_current();if(z)goto done;z=_gz_hg_capture_seal(gate);if(z)goto done;
  if(!_gz_prk_kernel_scope(mode,frame)){z=5;goto done;}
  /* The fixed table and prevalidated private scalars are the only inputs
   * between stores. No SDK/native call, source lookup, callback or yield. */
  for(i=0;i<SPK_BYTES;i++)if(!spk_skip(i)){
   *(volatile unsigned char*)spk_address(root,i)=_gz_spk_target[i];_gz_spk_report[14]++;
  }
  spk_read(root);
  for(i=0;i<SPK_BYTES;i++)if(!spk_skip(i)&&_gz_spk_values[i]!=_gz_spk_target[i]){z=77;goto done;}
 }
 z=srh_current();if(!z)z=_gz_hg_capture_seal(gate);
done:
 if(gate){i=_gz_hg_capture_end(gate);if(!z)z=i;_gz_spk_report[5]=i;}
 if(!z&&!_gz_prk_kernel_scope(mode,frame))z=5;
 _gz_spk_report[3]=z;_gz_spk_report[11]=_gz_hl_control.flags;_gz_spk_report[12]=_gz_hg_control[15];
 if(!z)_gz_spk_report[6]=srh_hash(_gz_spk_values,SPK_BYTES);
 _gz_spk_entered=_gz_spk_mode=0;return z;
}
SpoU32 _gz_spk_owner_read(void){return spk_access(2);}
SpoU32 _gz_spk_owner_apply(void){return spk_access(3);}
SpoU32 _gz_spk_owner_undo(void){return spk_access(4);}
SpoU32 _gz_spk_owner_check(void){return spk_access(5);}
