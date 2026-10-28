#include "../wct_root_borrow.h"
/* Fresh real entered kernel context only; no source/root/native body read. */
SpoU32 _gz_srh_wct_root_context(SpoU32 kind,SpoU32 mode,SpoU32 serial,SpoU32 frame){SpoU32 thread;
 if(!serial||frame!=_gz_pbac_frame||mode<2||mode>5||!srh_sealed()||
  _gz_rkc_entered||_gz_dth_entered||_gz_gqk_access_entered()||
  _gz_doh_access_entered()||_gz_dck_access_entered()||_gz_srh_probe_entered)return 0;
 thread=_gz_hg_platform_thread();if(!thread)return 0;
 if(kind==WHL_ROOT_CLOCK)return _gz_sck_entered==1&&!_gz_spk_entered&&
  _gz_sck_serial==serial&&_gz_sck_frame==frame&&_gz_sck_thread==thread&&
  _gz_sck_mode==mode&&_gz_ckr_kernel_scope(mode,frame)?1u:0u;
 if(kind==WHL_ROOT_PERSIST)return _gz_spk_entered==1&&!_gz_sck_entered&&
  _gz_spk_serial==serial&&_gz_spk_frame==frame&&_gz_spk_thread==thread&&
  _gz_spk_mode==mode&&_gz_prk_kernel_scope(mode,frame)?1u:0u;
 return 0;
}
static SpoU32 srh_wct_seal(SpoU32 kind,SpoU32 mode,SpoU32 serial,SpoU32 frame,SpoU32 gate,SpoU32 borrowed){
 if(borrowed&&!_gz_wct_root_held(kind,mode,serial,frame,gate))return WHL_ROOT_RETAINED;
 return _gz_hg_capture_seal(gate);
}
