/* Final verification of every participant in this typed core transaction.
 * This is not a whole-game coverage certificate: actors not handled by this
 * core still need their own participants before ordinary UI admission. */
extern u32 _gz_roh_core_target_context(u32,u32),_gz_roh_core_commit(u32,u32);
u32 _gz_ccr_target_check(u32,u32),_gz_cgr_target_check(u32,u32);
u32 _gz_wct_target_scope(u32 t,u32 f){const volatile u32*o=_gz_roh_owner;
 if(!wct_sealed(t,f)||wct.phase!=WCT_FORWARD||wct.returned||wct.error||_gz_boh_has_owner()||
  !_gz_roh_core_target_context(t,f)||o[17]!=wct.source||o[18]!=wct.used||o[26]!=wct.revision||
  o[29]!=wct.link||o[37]!=wct.camera||_gz_osa_pin_id()!=wct.pin||_gz_osa_validate(wct.pin)||
  _gz_osa_source(wct.pin)!=wct.source||_gz_osa_bytes(wct.pin)!=wct.used||
  _gz_osa_pin_identity(wct.pin)!=wct.identity||_gz_osa_pin_generation(wct.pin)!=wct.generation||
  btr_hash((const void*)wct.source,wct.used)!=wct.source_hash)return 0;
 return 1;
}
u32 _gz_wct_target_check(u32 t,u32 f){u32 z=42,i,values[3];
 _gz_wct_report[91]=0;_gz_wct_report[92]=42;_gz_wct_report[93]=0;
 if(wct_target_entered||!_gz_wct_target_scope(t,f))return 42;
 wct_target_entered=1;
 if(_gz_lpc_run.phase!=LPC_APPLIED||_gz_lpc_run.error||lpc_binding(0)||
  lpc_match(_gz_lpc_plan.saved,_gz_lpc_plan.saved_bind)){z=236;goto done;}
 _gz_wct_report[91]|=1;_gz_wct_report[93]=1;
 if(_gz_lpf_run.phase!=3||_gz_lpf_run.error||lpf_binding(0)||
  fr_current(&_gz_lpf_plan.leaf)||fr_saved_matches(&_gz_lpf_plan.leaf)){z=248;goto done;}
 _gz_wct_report[91]|=2;_gz_wct_report[93]=2;
 if(_gz_cct_journal.phase!=CCT_APPLIED||cct_binding()||
  cct_match(_gz_cct_journal.saved,_gz_cct_journal.after_links,_gz_cct_journal.after_force)){z=45;goto done;}
 _gz_wct_report[91]|=4;_gz_wct_report[93]=3;
 if(_gz_ppt_run.phase!=PPT_APPLIED||_gz_ppt_run.error||_gz_ppt_run.applied_count!=_gz_ppt_plan.count){z=226;goto done;}
 if(_gz_wgp_has_token(t)){z=_gz_wgp_target(t,f);if(z)goto done;}else for(i=0;i<_gz_ppt_plan.count;i++)if(ppt_binding(_gz_ppt_plan.rows+i,f)||
  ppt_match(_gz_ppt_plan.rows+i,_gz_ppt_plan.rows[i].saved,0)){z=226;goto done;}
 _gz_wct_report[91]|=8;_gz_wct_report[93]=4;
 if(_gz_lct_plan.phase!=LCT_APPLIED||!lct_scene(&_gz_lct_plan.saved)||
  lct_bind(&_gz_lct_plan.saved,0)||lct_match(&_gz_lct_plan.saved)){z=26;goto done;}
 _gz_wct_report[91]|=16;_gz_wct_report[93]=5;
 z=fgp_core_target(t,f);if(z)goto done;
 _gz_wct_report[91]|=32;_gz_wct_report[93]=6;
 z=_gz_pti_target_check(t,f);if(z)goto done;
 _gz_wct_report[91]|=64;_gz_wct_report[93]=7;
 z=_gz_ccr_target_check(t,f);if(z)goto done;
 _gz_wct_report[91]|=128;_gz_wct_report[93]=8;
 z=_gz_cgr_target_check(t,f);if(z)goto done;
 _gz_wct_report[91]|=256;_gz_wct_report[93]=9;
 if(_gz_ckr_entered||_gz_ckr_plan.phase!=CKR_APPLIED||_gz_ckr_plan.error){z=77;goto done;}
 _gz_ckr_entered=1;_gz_ckr_kernel_mode=5;_gz_ckr_kernel_frame=f;z=_gz_sck_owner_check();
 _gz_ckr_entered=_gz_ckr_kernel_mode=_gz_ckr_kernel_frame=0;if(z)goto done;
 for(i=0;i<_gz_ckr_plan.count;i++)if(_gz_sck_values[i]!=_gz_ckr_plan.saved[i]){z=77;goto done;}
 _gz_wct_report[91]|=512;_gz_wct_report[93]=10;
 if(_gz_prk_entered||_gz_prk_plan.phase!=PRK_APPLIED||_gz_prk_plan.error){z=77;goto done;}
 _gz_prk_entered=1;_gz_prk_kernel_mode=5;_gz_prk_kernel_frame=f;z=_gz_spk_owner_check();
 _gz_prk_entered=_gz_prk_kernel_mode=_gz_prk_kernel_frame=0;if(z)goto done;
 for(i=0;i<PRK_BYTES;i++)if(!prk_skip(i)&&_gz_spk_values[i]!=_gz_prk_plan.saved[i]){z=77;goto done;}
 _gz_wct_report[91]|=1024;_gz_wct_report[93]=11;
 if(_gz_rgr_entered||_gz_rgr_plan.phase!=RGR_APPLIED||_gz_rgr_plan.error){z=77;goto done;}
 rgr_read(values);if(!btr_equal(values,_gz_rgr_plan.saved,12)){z=77;goto done;}
 _gz_wct_report[91]|=2048;_gz_wct_report[93]=12;
 z=cjp_target(t,f);if(z)goto done;
 _gz_wct_report[91]|=4096;_gz_wct_report[93]=13;
 z=_gz_wgp_target(t,f);if(z)goto done;_gz_wct_report[91]|=8192;_gz_wct_report[93]=14;
 z=_gz_wct_target_scope(t,f)?0:42;
done:_gz_wct_report[92]=z;wct_target_entered=0;return z;
}
u32 _gz_wct_commit_scope(u32 t,u32 f){
 return wct_commit_arming==1&&wct_entered==1&&!wct_target_entered&&wct_sealed(t,f)&&
  wct.phase==WCT_FORWARD&&!wct.returned&&!wct.error&&wct.services==WCT_FINAL_CALLS&&
  _gz_wct_report[91]==16383&&!_gz_wct_report[92]&&_gz_wct_report[93]==14&&cjp_complete(t)&&_gz_wgp_commit_scope(t,f)&&
  _gz_wct_closure_hash&&_gz_wct_closure_hash==wct_seal&&_gz_wct_target_scope(t,f);
}
static u32 wct_commit_finish(u32 t,u32 f){u32 z;
 if(wct_commit_arming||!wct_sealed(t,f)||_gz_roh_owner[12]||_gz_roh_owner[13]!=WCT_FINAL_CALLS)return 42;
 wct.services=WCT_FINAL_CALLS;wct_seal_now();z=_gz_wct_target_check(t,f);if(z)return z;
 _gz_wct_closure_hash=wct_seal;wct_commit_arming=1;
 z=_gz_roh_core_commit(t,f);wct_commit_arming=0;
 if(!z)_gz_snapshot_ui_arm();
 return z;
}
