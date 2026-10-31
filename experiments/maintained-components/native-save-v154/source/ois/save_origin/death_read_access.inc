/* Fixed death transition READ domain. No source/native destination argument.
 * The same real allocator exclusion/root generation remains mandatory. This
 * domain grants no clock/persistent write and no lifecycle or restore API.
 */
extern SpoU32 _gz_dto_kernel_scope(SpoU32),_gz_dto_read_under_root(SpoU32);
static SpoU32 dth_context_owned(SpoU32 cookie){return _gz_dth_entered==1&&cookie&&
 cookie==_gz_dth_serial&&_gz_dth_frame==_gz_pbac_frame&&_gz_dth_thread&&
 _gz_dth_thread==_gz_hg_platform_thread()&&srh_sealed()&&_gz_dto_kernel_scope(_gz_dth_frame);}
SpoU32 _gz_dth_read_scope(SpoU32 frame){return frame==_gz_dth_frame&&dth_context_owned(_gz_dth_serial)&&
 _gz_dth_gate&&!_gz_hg_capture_seal(_gz_dth_gate);}
SpoU32 _gz_dth_health(void){return _gz_dth_read_scope(_gz_pbac_frame)?*(volatile unsigned short*)(_gz_srh_binding.root+0x22):0xffffffffu;}
SpoU32 _gz_dth_capture_access(void){SpoU32 frame=_gz_pbac_frame,gate=0,z,end,health;
 if(_gz_dth_entered||_gz_gqk_access_entered()||_gz_dck_access_entered()||_gz_doh_access_entered()||_gz_spk_entered||_gz_sck_entered||_gz_gac_root_entered||_gz_rkc_entered||_gz_srh_probe_entered||
  !_gz_dto_kernel_scope(frame)||!srh_sealed()||_gz_dth_serial==0xffffffffu)return 28;
 _gz_dth_serial++;_gz_dth_entered=1;_gz_dth_frame=frame;_gz_dth_thread=_gz_hg_platform_thread();
 gate=_gz_hg_capture_begin(_gz_dth_serial);if(!gate){z=86;goto done;}
 _gz_dth_gate=gate;z=_gz_hg_capture_seal(gate);if(z)goto done;z=srh_current();if(z)goto done;
 health=_gz_dth_health();if(health==0xffffffffu){z=5;goto done;}
 z=_gz_dto_read_under_root(frame);if(z)goto done;
 z=srh_current();if(!z)z=_gz_hg_capture_seal(gate);
 if(!z&&health!=_gz_dth_health())z=5;
done:
 if(gate){end=_gz_hg_capture_end(gate);if(!z)z=end;}
 if(!z&&!_gz_dto_kernel_scope(frame))z=5;
 _gz_dth_gate=_gz_dth_entered=0;return z;}
