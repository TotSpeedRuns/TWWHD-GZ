static __attribute__((noinline,noclone)) SpoU32 sck_access(SpoU32 mode){SpoU32 frame=_gz_pbac_frame,gate=0,borrowed=0,z=0,i,root,n=0,target[8],expected[8],alternate[8];
 for(i=2;i<16;i++)_gz_sck_report[i]=0;
 if(_gz_sck_entered||_gz_spk_entered||_gz_gac_root_entered||_gz_rkc_entered||_gz_dth_entered||_gz_gqk_access_entered()||_gz_doh_access_entered()||_gz_dck_access_entered()||_gz_srh_probe_entered||!sck_scope(mode,frame)||!srh_sealed()||
  _gz_sck_serial==0xffffffffu)return 28;
 _gz_sck_serial++;_gz_sck_frame=frame;_gz_sck_thread=_gz_hg_platform_thread();_gz_sck_mode=mode;_gz_sck_entered=1;
 _gz_sck_report[2]=frame;_gz_sck_report[4]=_gz_sck_serial;_gz_sck_report[13]=mode;
 if(_gz_wct_heap_pending()){borrowed=1;gate=_gz_wct_root_borrow(WHL_ROOT_CLOCK,mode,_gz_sck_serial,frame);}
 else gate=_gz_hg_capture_begin(_gz_sck_serial);_gz_sck_report[15]=borrowed;if(!gate){z=86;goto done;}
 z=srh_wct_seal(WHL_ROOT_CLOCK,mode,_gz_sck_serial,frame,gate,borrowed);if(z)goto done;
 z=srh_current();if(z)goto done;root=_gz_srh_binding.root;
 sck_read(root);
 n=mode==1?8:_gz_ckr_kernel_count();if(n!=5&&n!=8){z=28;goto done;}
 if(mode==3||mode==4){
  for(i=0;i<n;i++){
   target[i]=_gz_ckr_kernel_target(i);expected[i]=_gz_ckr_kernel_expected(i,0);
   alternate[i]=mode==4?_gz_ckr_kernel_expected(i,1):expected[i];
  }
  if(!sck_values_safe(target,n)||!sck_values_safe(expected,n)||!sck_values_safe(alternate,n)){z=77;goto done;}
  for(i=0;i<n;i++)if(_gz_sck_values[i]!=expected[i]&&_gz_sck_values[i]!=alternate[i]){z=77;goto done;}
  z=srh_current();if(z)goto done;z=srh_wct_seal(WHL_ROOT_CLOCK,mode,_gz_sck_serial,frame,gate,borrowed);if(z)goto done;
  if(!sck_scope(mode,frame)){z=5;goto done;}
  /* Fixed targets only. No native call, callback, source lookup or yield
   * occurs between these five stores under the exact allocation exclusion. */
  *(volatile SpoU32*)(root+0x44)=target[0];_gz_sck_report[14]|=1;
  *(volatile unsigned short*)(root+0x48)=(unsigned short)target[1];_gz_sck_report[14]|=2;
  *(volatile SpoU32*)SPO_CLOCK_A=target[2];_gz_sck_report[14]|=4;
  *(volatile unsigned short*)SPO_CLOCK_SHORT=(unsigned short)target[3];_gz_sck_report[14]|=8;
  *(volatile SpoU32*)SPO_CLOCK_B=target[4];_gz_sck_report[14]|=16;
  /* v1 archives never write the absent air fields. Fixed static native
   * targets join the same entered frame/thread/root/heap transaction. */
  if(n==8){*(volatile SpoU32*)SCK_AIR=target[5];_gz_sck_report[14]|=32;
   *(volatile SpoU32*)(SCK_AIR+4)=target[6];_gz_sck_report[14]|=64;
   *(volatile unsigned char*)(SCK_AIR+0x64)=(unsigned char)target[7];_gz_sck_report[14]|=128;}
  sck_read(root);for(i=0;i<n;i++)if(_gz_sck_values[i]!=target[i]){z=77;goto done;}
 }
 z=srh_current();if(!z)z=srh_wct_seal(WHL_ROOT_CLOCK,mode,_gz_sck_serial,frame,gate,borrowed);
done:
 if(gate){i=borrowed?_gz_wct_root_unborrow(WHL_ROOT_CLOCK,mode,_gz_sck_serial,frame,gate):_gz_hg_capture_end(gate);if(!z)z=i;_gz_sck_report[5]=i;}
 if(!z&&!sck_scope(mode,frame))z=5;
 if(!z)for(i=0;i<5;i++)_gz_sck_report[6+i]=_gz_sck_values[i];
 _gz_sck_report[3]=z;_gz_sck_report[11]=_gz_hl_control.flags;
 _gz_sck_report[12]=_gz_hg_control[15];_gz_sck_entered=_gz_sck_mode=0;return z;
}
