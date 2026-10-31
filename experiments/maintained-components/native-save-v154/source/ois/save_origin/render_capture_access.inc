/* Fixed renderer READ domain. No actor, native buffer, root or game pointer
 * is supplied by a caller. Only the genuine staged capture callbacks may
 * enter; the actual common allocator exclusion must be acquired and sealed
 * before the fixed renderer reader can run. Clock/persistent writers retain
 * their original enumerated targets and never gain renderer access.
 */
extern SpoU32 _gz_krc_kernel_scope(SpoU32,SpoU32),_gz_krc_read_under_root(SpoU32,SpoU32);
extern SpoU32 _gz_jpg_capture_begin(SpoU32,SpoU32),_gz_jpg_capture_gpu(SpoU32,SpoU32);
extern SpoU32 _gz_jpg_capture_gpu_ready(SpoU32,SpoU32),_gz_jpg_capture_end(SpoU32,SpoU32);
SpoU32 _gz_rkc_report[16]={0x524b4331u,64};
static SpoU32 rkc_context_owned(SpoU32 cookie){return _gz_rkc_entered==1&&cookie&&
 cookie==_gz_rkc_serial&&_gz_rkc_frame==_gz_pbac_frame&&
 (_gz_rkc_mode==1||_gz_rkc_mode==2)&&_gz_rkc_thread&&
 _gz_rkc_thread==_gz_hg_platform_thread()&&srh_sealed()&&
 _gz_krc_kernel_scope(_gz_rkc_mode,_gz_rkc_frame);}
/* Entry is issued only by the genuine fixed renderer callback, before its
 * heap exclusion. Identity is a scalar-only observer usable by a foreign JPA
 * writer while it waits: it grants neither read access nor native replay. */
SpoU32 _gz_rkc_particle_entry(SpoU32 mode,SpoU32 frame){return
 mode==_gz_rkc_mode&&frame==_gz_rkc_frame&&!_gz_rkc_gate&&
 rkc_context_owned(_gz_rkc_serial)?_gz_rkc_serial:0;}
SpoU32 _gz_rkc_particle_identity(SpoU32 mode,SpoU32 frame,SpoU32 cookie,SpoU32 thread){return
 _gz_rkc_entered==1&&cookie&&cookie==_gz_rkc_serial&&thread&&thread==_gz_rkc_thread&&
 mode==_gz_rkc_mode&&(mode==1||mode==2)&&frame==_gz_rkc_frame&&frame==_gz_pbac_frame&&
 srh_sealed()&&_gz_krc_kernel_scope(mode,frame)?1:0;}
SpoU32 _gz_rkc_read_scope(SpoU32 mode,SpoU32 frame){return
 mode==_gz_rkc_mode&&frame==_gz_rkc_frame&&rkc_context_owned(_gz_rkc_serial)&&
 _gz_rkc_gate&&!_gz_hg_capture_seal(_gz_rkc_gate)&&!_gz_jpg_capture_gpu_ready(mode,frame);}
SpoU32 _gz_rkc_private_apart(SpoU32 a,SpoU32 n){SpoU32 i,p[]={
 (SpoU32)&_gz_rkc_entered,(SpoU32)&_gz_rkc_frame,(SpoU32)&_gz_rkc_thread,
 (SpoU32)&_gz_rkc_serial,(SpoU32)&_gz_rkc_mode,(SpoU32)&_gz_rkc_gate,(SpoU32)_gz_rkc_report};
 if(!n||a+n<a)return 0;
 for(i=0;i<7;i++)if(a<p[i]+(i==6?64:4)&&p[i]<a+n)return 0;return 1;}
SpoU32 _gz_rkc_capture_access(SpoU32 mode){SpoU32 frame=_gz_pbac_frame,gate=0,particle=0,z=0,end;
 if((mode!=1&&mode!=2)||_gz_gac_root_entered||_gz_rkc_entered||_gz_spk_entered||_gz_sck_entered||_gz_dth_entered||_gz_gqk_access_entered()||_gz_dck_access_entered()||_gz_srh_probe_entered||
  !_gz_krc_kernel_scope(mode,frame)||!srh_sealed()||_gz_rkc_serial==0xffffffffu)return 28;
 _gz_rkc_serial++;_gz_rkc_entered=1;_gz_rkc_mode=mode;_gz_rkc_frame=frame;_gz_rkc_thread=_gz_hg_platform_thread();
 _gz_rkc_report[2]=mode;_gz_rkc_report[3]=frame;_gz_rkc_report[4]=_gz_rkc_serial;
 _gz_rkc_report[9]=_gz_rkc_report[10]=_gz_rkc_report[11]=0;
 z=_gz_jpg_capture_begin(mode,frame);_gz_rkc_report[9]=z;if(z)goto done;particle=1;
 /* A GPU wait must finish while the allocator is OPEN. Waiting after the
  * heap lock could deadlock graphics work that needs that same allocator. */
 z=_gz_jpg_capture_gpu(mode,frame);_gz_rkc_report[10]=z;if(z)goto done;
 gate=_gz_hg_capture_begin(_gz_rkc_serial);if(!gate){z=86;goto done;}
 _gz_rkc_gate=gate;z=_gz_hg_capture_seal(gate);if(z)goto done;
 z=srh_current();if(z)goto done;
 if(!_gz_rkc_read_scope(mode,frame)){z=5;goto done;}
 z=_gz_krc_read_under_root(mode,frame);if(z)goto done;
 z=srh_current();if(!z)z=_gz_hg_capture_seal(gate);
done:
 if(gate){end=_gz_hg_capture_end(gate);if(!z)z=end;_gz_rkc_report[6]=end;}
 if(particle){end=_gz_jpg_capture_end(mode,frame);if(!z)z=end;_gz_rkc_report[11]=end;}
 if(!z&&!_gz_krc_kernel_scope(mode,frame))z=5;
 _gz_rkc_report[5]=z;_gz_rkc_report[7]=_gz_hl_control.flags;_gz_rkc_report[8]=_gz_hg_control[15];
 _gz_rkc_gate=_gz_rkc_entered=_gz_rkc_mode=0;return z;}
