static __attribute__((noinline,noclone)) SpoU32 spk_access(SpoU32 mode){SpoU32 frame=_gz_pbac_frame,gate=0,borrowed=0,z=0,i,x,root=0;
 for(i=2;i<20;i++)_gz_spk_report[i]=0;
 if(_gz_spk_entered||_gz_sck_entered||_gz_gac_root_entered||_gz_rkc_entered||_gz_dth_entered||_gz_gqk_access_entered()||_gz_dck_access_entered()||_gz_srh_probe_entered||mode<2||mode>5||
  !_gz_prk_kernel_scope(mode,frame)||!srh_sealed()||_gz_spk_serial==0xffffffffu)return 28;
 _gz_spk_serial++;_gz_spk_frame=frame;_gz_spk_thread=_gz_hg_platform_thread();_gz_spk_mode=mode;_gz_spk_entered=1;
 _gz_spk_report[2]=frame;_gz_spk_report[4]=_gz_spk_serial;_gz_spk_report[13]=mode;
 if(_gz_wct_heap_pending()){borrowed=1;gate=_gz_wct_root_borrow(WHL_ROOT_PERSIST,mode,_gz_spk_serial,frame);}
 else gate=_gz_hg_capture_begin(_gz_spk_serial);_gz_spk_report[16]=borrowed;if(!gate){z=86;goto done;}
 z=srh_wct_seal(WHL_ROOT_PERSIST,mode,_gz_spk_serial,frame,gate,borrowed);if(z)goto done;
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
  z=srh_current();if(z)goto done;z=srh_wct_seal(WHL_ROOT_PERSIST,mode,_gz_spk_serial,frame,gate,borrowed);if(z)goto done;
  if(!_gz_prk_kernel_scope(mode,frame)){z=5;goto done;}
  /* The fixed table and prevalidated private scalars are the only inputs
   * between stores. No SDK/native call, source lookup, callback or yield. */
  for(i=0;i<SPK_BYTES;i++)if(!spk_skip(i)){
   *(volatile unsigned char*)spk_address(root,i)=_gz_spk_target[i];_gz_spk_report[14]++;
  }
  spk_read(root);
  for(i=0;i<SPK_BYTES;i++)if(!spk_skip(i)&&_gz_spk_values[i]!=_gz_spk_target[i]){z=77;goto done;}
 }
 z=srh_current();if(!z)z=srh_wct_seal(WHL_ROOT_PERSIST,mode,_gz_spk_serial,frame,gate,borrowed);
done:
 if(gate){i=borrowed?_gz_wct_root_unborrow(WHL_ROOT_PERSIST,mode,_gz_spk_serial,frame,gate):_gz_hg_capture_end(gate);if(!z)z=i;_gz_spk_report[5]=i;}
 if(!z&&!_gz_prk_kernel_scope(mode,frame))z=5;
 _gz_spk_report[3]=z;_gz_spk_report[11]=_gz_hl_control.flags;_gz_spk_report[12]=_gz_hg_control[15];
 if(!z)_gz_spk_report[6]=srh_hash(_gz_spk_values,SPK_BYTES);
 _gz_spk_entered=_gz_spk_mode=0;return z;
}
