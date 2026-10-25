/* Private, read-only whole-core preparation exercise. It does not authorize
 * any apply/undo, constructor, deletion, publication or native save operation.
 * Actor families outside these eight participants remain separate work. */
volatile u32 _gz_wdp_request,_gz_wdp_ack;
u32 _gz_wdp_report[96]={0x57445031u,384,1};
static u32 wdp_retiring;
static u32 wdp_blank(const void*p,u32 n){const volatile u32*q=p;u32 i;
 for(i=0;i<n/4;i++)if(q[i])return 0;return 1;
}
static u32 wdp_empty(void){
 return wdp_blank(&_gz_cct_journal,sizeof(_gz_cct_journal))&&!_gz_cct_seal&&!_gz_cct_entered&&
 wdp_blank(&_gz_ppt_plan,sizeof(_gz_ppt_plan))&&wdp_blank(&_gz_ppt_run,sizeof(_gz_ppt_run))&&!_gz_ppt_plan_seal&&!_gz_ppt_run_seal&&!_gz_ppt_entered&&
 wdp_blank(&_gz_lpc_plan,sizeof(_gz_lpc_plan))&&wdp_blank(&_gz_lpc_run,sizeof(_gz_lpc_run))&&!_gz_lpc_seal&&!_gz_lpc_run_seal&&!_gz_lpc_entered&&
 wdp_blank(&_gz_lpf_plan,sizeof(_gz_lpf_plan))&&wdp_blank(&_gz_lpf_run,sizeof(_gz_lpf_run))&&!_gz_lpf_seal&&!_gz_lpf_run_seal&&!_gz_lpf_entered&&
 wdp_blank(&_gz_lct_plan,sizeof(_gz_lct_plan))&&!_gz_lct_seal&&!_gz_lct_entered&&
 wdp_blank(&_gz_ckr_plan,sizeof(_gz_ckr_plan))&&!_gz_ckr_seal&&!_gz_ckr_entered&&!_gz_ckr_kernel_mode&&!_gz_ckr_kernel_frame&&
 wdp_blank(&_gz_prk_plan,sizeof(_gz_prk_plan))&&!_gz_prk_seal&&!_gz_prk_entered&&!_gz_prk_kernel_mode&&!_gz_prk_kernel_frame&&
 wdp_blank(&_gz_rgr_plan,sizeof(_gz_rgr_plan))&&!_gz_rgr_seal&&!_gz_rgr_entered&&fgp_empty()&&pti_empty()&&cjp_empty()&&_gz_ccr_empty()&&_gz_cgr_empty()&&_gz_wgp_idle();
}
static void wdp_clear_private(void){
 bcl_zero(&_gz_cct_journal,sizeof(_gz_cct_journal));_gz_cct_seal=0;
 bcl_zero(&_gz_ppt_plan,sizeof(_gz_ppt_plan));bcl_zero(&_gz_ppt_run,sizeof(_gz_ppt_run));_gz_ppt_plan_seal=_gz_ppt_run_seal=0;
 bcl_zero(&_gz_lpc_plan,sizeof(_gz_lpc_plan));bcl_zero(&_gz_lpc_run,sizeof(_gz_lpc_run));_gz_lpc_seal=_gz_lpc_run_seal=0;
 bcl_zero(&_gz_lpf_plan,sizeof(_gz_lpf_plan));bcl_zero(&_gz_lpf_run,sizeof(_gz_lpf_run));_gz_lpf_seal=_gz_lpf_run_seal=0;
 bcl_zero(&_gz_lct_plan,sizeof(_gz_lct_plan));_gz_lct_seal=0;
 bcl_zero(&_gz_ckr_plan,sizeof(_gz_ckr_plan));_gz_ckr_seal=0;
 bcl_zero(&_gz_prk_plan,sizeof(_gz_prk_plan));_gz_prk_seal=0;
 bcl_zero(&_gz_rgr_plan,sizeof(_gz_rgr_plan));_gz_rgr_seal=0;
 bcl_zero(&wsp_source,sizeof(wsp_source));bcl_zero(&txo_source,sizeof(txo_source));
 wsp_source_seal=wdp_retiring=0;
}
static u32 wdp_owned_or_empty(u32 p,u32 t){return !p||p==t;}
/* No preparation can write native state. Cleanup still checks every journal
 * for an actual transition or foreign token, and only follows private data. */
static u32 wdp_forget(u32 token){const volatile u32*o=_gz_roh_owner;
 if(!token||wdp_retiring!=token||wsp_source.token!=token||o[3]||o[4]||o[5]||o[6]||o[12]||
  _gz_roh_user_guard()||_gz_osa_pin_id()||_gz_boh_has_owner()||
  _gz_cct_entered||_gz_cct_journal.phase>CCT_PREPARED||_gz_cct_journal.applied_frame||_gz_cct_journal.rolled_frame||
  _gz_ppt_entered||_gz_ppt_run.phase>PPT_PREPARED||_gz_ppt_run.applied_count||_gz_ppt_run.undo_count||_gz_ppt_run.applied_frame||
  _gz_lpc_entered||_gz_lpc_run.phase>LPC_PREPARED||_gz_lpc_run.applied_frame||_gz_lpc_run.rolled_frame||
  _gz_lpf_entered||_gz_lpf_run.phase>1||_gz_lpf_run.native_calls||
  _gz_lct_entered||_gz_lct_plan.phase>LCT_PREPARED||_gz_lct_plan.applied_frame||_gz_lct_plan.rolled_frame||
  _gz_ckr_entered||_gz_ckr_kernel_mode||_gz_ckr_kernel_frame||_gz_ckr_plan.phase>CKR_PREPARED||_gz_ckr_plan.applied_frame||_gz_ckr_plan.rolled_frame||
  _gz_prk_entered||_gz_prk_kernel_mode||_gz_prk_kernel_frame||_gz_prk_plan.phase>PRK_PREPARED||_gz_prk_plan.applied_frame||_gz_prk_plan.rolled_frame||
  _gz_rgr_entered||_gz_rgr_plan.phase>RGR_PREPARED||_gz_rgr_plan.applied_frame||_gz_rgr_plan.rolled_frame||
  !wdp_owned_or_empty(_gz_cct_journal.token,token)||!wdp_owned_or_empty(_gz_ppt_plan.token,token)||
  !wdp_owned_or_empty(_gz_ppt_run.token,token)||!wdp_owned_or_empty(_gz_lpc_plan.token,token)||
  !wdp_owned_or_empty(_gz_lpc_run.token,token)||!wdp_owned_or_empty(_gz_lpf_plan.token,token)||
  !wdp_owned_or_empty(_gz_lct_plan.token,token)||!wdp_owned_or_empty(_gz_ckr_plan.token,token)||
  !wdp_owned_or_empty(_gz_prk_plan.token,token)||!wdp_owned_or_empty(_gz_rgr_plan.token,token))return 42;
 if(_gz_wgp_forget_clean(token)||_gz_cgr_forget(token)||_gz_ccr_forget(token)||cjp_forget(token)||pti_forget(token)||fgp_forget(token))return 42;
 wdp_clear_private();return wdp_empty()?0:42;
}
static u32 wdp_before_images(u32 t,u32 f){u32 z,i,seeds[3];
 z=cct_binding();if(!z)z=cct_before_links();if(!z)z=cct_match(_gz_cct_journal.before,_gz_cct_journal.before_links,_gz_cct_journal.before_force);
 _gz_wdp_report[20]=z;if(z)return z;
 if(_gz_wgp_has_token(t))z=_gz_wgp_prepared_before(t,f);else for(i=0;i<_gz_ppt_plan.count;i++){z=ppt_binding(_gz_ppt_plan.rows+i,f);if(!z)z=ppt_match(_gz_ppt_plan.rows+i,_gz_ppt_plan.rows[i].before,1);if(z)break;}
 _gz_wdp_report[21]=z;if(z)return z;
 z=lpc_binding(0);if(!z)z=lpc_match(_gz_lpc_plan.before,_gz_lpc_plan.before_bind);
 _gz_wdp_report[22]=z;if(z)return z;
 z=lpf_binding(0);if(!z)z=fr_current(&_gz_lpf_plan.leaf);if(!z)z=fr_before_matches(&_gz_lpf_plan.leaf,0);
 _gz_wdp_report[23]=z;if(z)return z;
 z=lct_bind(&_gz_lct_plan.before,0);if(!z)z=lct_match(&_gz_lct_plan.before);
 _gz_wdp_report[24]=z;if(z)return z;
 _gz_ckr_entered=1;_gz_ckr_kernel_mode=2;_gz_ckr_kernel_frame=f;z=_gz_sck_owner_read();
 _gz_ckr_entered=_gz_ckr_kernel_mode=_gz_ckr_kernel_frame=0;
 if(!z&&!btr_equal(_gz_sck_values,_gz_ckr_plan.before,4*_gz_ckr_plan.count))z=77;
 _gz_wdp_report[25]=z;if(z)return z;
 _gz_prk_entered=1;_gz_prk_kernel_mode=2;_gz_prk_kernel_frame=f;z=_gz_spk_owner_read();
 _gz_prk_entered=_gz_prk_kernel_mode=_gz_prk_kernel_frame=0;
 if(!z&&!btr_equal(_gz_spk_values,_gz_prk_plan.before,PRK_BYTES))z=77;
 _gz_wdp_report[26]=z;if(z)return z;
 rgr_read(seeds);z=btr_equal(seeds,_gz_rgr_plan.before,12)?0:77;
 _gz_wdp_report[27]=z;if(z)return z;
 return tx_observe_matches(t,f,wsp_source.source,wsp_source.used,wsp_source.pin)?0:42;
}
void _gz_wdp_step(void){u32 req=_gz_wdp_request,t=0,z=5,i,open=0,slot=_gz_pbac_selected,f=_gz_pbac_frame;
 if(!req||req==_gz_wdp_ack)return;
 for(i=3;i<96;i++)_gz_wdp_report[i]=0;
 _gz_wdp_report[3]=req;_gz_wdp_report[4]=f;
 if(req!=_gz_wdp_ack+1||_gz_roh_user_guard()||_gz_osa_pin_id()||_gz_boh_has_owner()||
  !wdp_empty()||wsp_source.valid||wsp_source_seal||wsp_retiring||wdp_retiring)goto done;
 z=_gz_slots_install(slot);_gz_wdp_report[6]=z;if(z)goto done;
 z=_gz_roh_begin(slot,60);_gz_wdp_report[7]=z;if(z)goto done;
 t=_gz_roh_owner[5];_gz_wdp_report[8]=t;
 z=_gz_roh_service_begin(t,1);_gz_wdp_report[9]=z;if(z)goto abort;
 open=1;z=wsp_source_start(t,f);_gz_wdp_report[10]=z;if(z)goto abort;
 _gz_wdp_report[11]=wsp_source.source_hash;
 for(i=12;i<28;i++)_gz_wdp_report[i]=BAD;
 _gz_wdp_report[12]=_gz_cct_prepare(t,f);
 _gz_wdp_report[13]=_gz_ppt_prepare(t,f);
 _gz_wdp_report[14]=_gz_lpc_prepare(t,f);
 if(!_gz_wdp_report[14])_gz_wdp_report[15]=_gz_lpf_prepare(t,f);
 _gz_wdp_report[16]=_gz_lct_prepare(t,f);
 _gz_wdp_report[17]=_gz_ckr_prepare(t,f);
 if(!_gz_wdp_report[17])_gz_wdp_report[18]=_gz_prk_prepare(t,f);
 _gz_wdp_report[19]=_gz_rgr_prepare(t,f);
 z=0;for(i=12;i<20;i++)if(!z&&_gz_wdp_report[i])z=_gz_wdp_report[i];
 _gz_wdp_report[40]=_gz_ppt_plan.count;_gz_wdp_report[41]=_gz_lct_plan.saved.count;
 _gz_wdp_report[42]=tx_observe_matches(t+1,f,wsp_source.source,wsp_source.used,wsp_source.pin);
 _gz_wdp_report[43]=tx_observe_matches(t,f+1,wsp_source.source,wsp_source.used,wsp_source.pin);
 if(!z)z=wdp_before_images(t,f);
abort:
 if(open){_gz_wdp_report[28]=_gz_roh_service_end(t,1,0);open=0;}
 if(t){
  _gz_wdp_report[29]=_gz_roh_abort_clean(t);
  if(!_gz_wdp_report[29]){
   if(wsp_source.token==t){wdp_retiring=t;_gz_wdp_report[30]=wdp_forget(t);}
   else _gz_wdp_report[30]=(!wdp_empty()||wsp_source.valid||wsp_source_seal||wdp_retiring)?42:0;
  }
 }
done:
 _gz_wdp_report[5]=z;_gz_wdp_report[31]=_gz_osa_pin_id();_gz_wdp_report[32]=_gz_roh_user_guard();
 _gz_wdp_report[33]=_gz_roh_owner[3];_gz_wdp_report[34]=_gz_roh_owner[4];_gz_wdp_report[35]=_gz_roh_owner[6];
 _gz_wdp_report[36]=wdp_empty();_gz_wdp_report[37]=wsp_source.valid;_gz_wdp_report[38]=_gz_pbac_frame;
 _gz_wdp_report[39]=wdp_retiring;
 __asm__ volatile("sync":::"memory");_gz_wdp_ack=req;
}
