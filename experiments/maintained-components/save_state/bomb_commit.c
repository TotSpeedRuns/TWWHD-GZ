/* Forward publication for the completed placed-bomb transaction only.
 * Generic runtime admission remains denied. Native lifetime, source, every
 * participating domain and the original bounded owner are checked first. */
extern u32 _gz_roh_bomb_commit(u32,u32);
static u32 bci_entered,bci_arming,bci_token,bci_frame,bci_source_hash;
u32 _gz_bci_report[32]={0x42434931u,128,1};
u32 _gz_bci_read_scope(u32 token,u32 frame){
 return bci_entered==1&&bci_token==token&&bci_frame==frame&&frame==_gz_pbac_frame&&
  bmh_sealed()&&_gz_bmh_token==token&&_gz_bmh_mask==127&&!_gz_bmh_busy&&
  _gz_bmh_receipt.phase==2&&boh_sealed()&&_gz_boh_plan.token==token&&
  _gz_boh_journal.phase==BOH_APPLIED&&!_gz_boh_journal.error&&
  _gz_roh_owner[3]==ROH_MUTATING&&_gz_roh_owner[4]==1&&_gz_roh_owner[5]==token&&
  _gz_roh_owner[6]==1&&!_gz_roh_owner[7]&&!_gz_roh_owner[12]&&_gz_roh_owner[10]==frame;
}
u32 _gz_boh_commit_scope(u32 token,u32 frame){
 return bci_arming==1&&_gz_bci_read_scope(token,frame)&&bci_source_hash&&
  bci_source_hash==_gz_roh_owner[22]&&_gz_bci_report[4]==4095&&_gz_bcf_commit_scope(token,frame)==1;
}
void _gz_boh_commit_leaf(void){
 /* Fixed private cleanup only. All calls with native/source inputs ended
  * before the owner enters this nonfallible publication tail. */
 _gz_btr_commit_leaf();_gz_boh_recovery_finish_leaf();
}
static u32 bci_finish(u32 token){u32 z,i,frame=_gz_pbac_frame,values[3];
 if(bci_entered)return 5;bci_entered=1;bci_token=token;bci_frame=frame;bci_source_hash=0;
 _gz_bci_report[3]=token;_gz_bci_report[4]=0;_gz_bci_report[5]=0;_gz_bci_report[6]=frame;
 if(!_gz_bci_read_scope(token,frame)||!boh_bound(token)||_gz_boh_validate(token)){z=40;goto done;}
 if(_gz_osa_validate(_gz_boh_plan.pin)){z=42;goto done;}
 bci_source_hash=btr_hash((const void*)_gz_hbp_recipe.source,_gz_hbp_recipe.used);
 if(bci_source_hash!=_gz_roh_owner[22]){z=42;goto done;}
 _gz_bci_report[4]|=1;
 if(!lpc_sealed()||_gz_lpc_run.phase!=LPC_APPLIED||_gz_lpc_run.error||
  lpc_binding(0)||lpc_match(_gz_lpc_plan.saved,_gz_lpc_plan.saved_bind)){z=236;goto done;}
 _gz_bci_report[4]|=2;
 if(!lpf_sealed()||_gz_lpf_run.phase!=3||_gz_lpf_run.error||lpf_binding(0)||
  fr_current(&_gz_lpf_plan.leaf)||fr_saved_matches(&_gz_lpf_plan.leaf)){z=248;goto done;}
 _gz_bci_report[4]|=4;
 if(!cct_sealed()||_gz_cct_journal.phase!=CCT_APPLIED||cct_binding()||
  cct_match(_gz_cct_journal.saved,_gz_cct_journal.after_links,_gz_cct_journal.after_force)){z=45;goto done;}
 _gz_bci_report[4]|=8;
 if(!ppt_sealed()||_gz_ppt_run.phase!=PPT_APPLIED||_gz_ppt_run.error||
  _gz_ppt_run.applied_count!=_gz_ppt_plan.count){z=226;goto done;}
 if(_gz_boh_plan.role==BOH_CREATED){z=_gz_bcf_final_check(token,frame);if(z)goto done;}else{
 for(i=0;i<_gz_ppt_plan.count;i++)if(ppt_binding(_gz_ppt_plan.rows+i,frame)||
  ppt_match(_gz_ppt_plan.rows+i,_gz_ppt_plan.rows[i].saved,0)){z=226;goto done;}
 }
 _gz_bci_report[4]|=16;
 if(!lct_sealed()||_gz_lct_plan.phase!=LCT_APPLIED||!lct_scene(&_gz_lct_plan.saved)||
  lct_bind(&_gz_lct_plan.saved,0)||lct_match(&_gz_lct_plan.saved)){z=26;goto done;}
 _gz_bci_report[4]|=32;
 if(_gz_btr_suspended_check(token)||!btr_source()){z=28;goto done;}_gz_bci_report[4]|=64;
 if(!ckr_sealed()||_gz_ckr_plan.phase!=CKR_APPLIED||_gz_ckr_plan.error){z=77;goto done;}
 _gz_ckr_entered=1;_gz_ckr_kernel_mode=5;_gz_ckr_kernel_frame=frame;z=_gz_sck_owner_check();
 _gz_ckr_entered=_gz_ckr_kernel_mode=_gz_ckr_kernel_frame=0;if(z)goto done;
 for(i=0;i<_gz_ckr_plan.count;i++)if(_gz_sck_values[i]!=_gz_ckr_plan.saved[i]){z=77;goto done;}
 _gz_bci_report[4]|=128;
 if(!prk_sealed()||_gz_prk_plan.phase!=PRK_APPLIED||_gz_prk_plan.error){z=77;goto done;}
 _gz_prk_entered=1;_gz_prk_kernel_mode=5;_gz_prk_kernel_frame=frame;z=_gz_spk_owner_check();
 _gz_prk_entered=_gz_prk_kernel_mode=_gz_prk_kernel_frame=0;if(z)goto done;
 for(i=0;i<PRK_BYTES;i++)if(!prk_skip(i)&&_gz_spk_values[i]!=_gz_prk_plan.saved[i]){z=77;goto done;}
 _gz_bci_report[4]|=256;
 if(!rgr_sealed()||_gz_rgr_plan.phase!=RGR_APPLIED||_gz_rgr_plan.error){z=77;goto done;}
 rgr_read(values);if(!btr_equal(values,_gz_rgr_plan.saved,12)){z=77;goto done;}
 /* Enemy behavior/children still lack a complete forward certificate. Keep
  * that boundary explicit; render replay alone cannot authorize gameplay. */
 if(!(eks_empty()||(eks_sealed()&&!_gz_eks_plan.count))||
  !(kbs_empty()||(kbs_sealed()&&!_gz_kbs_plan.count))||
  !(bks_empty()||(bks_sealed()&&!_gz_bks_plan.count))){z=461;goto done;}
 _gz_bci_report[4]|=512;
 z=fgp_saved_check(token,frame);if(z)goto done;
 _gz_bci_report[4]|=1024;
 if(_gz_boh_validate(token)||_gz_osa_validate(_gz_boh_plan.pin)||
  !_gz_bci_read_scope(token,frame)){z=40;goto done;}
 z=_gz_bcf_final_check(token,frame);if(z)goto done;_gz_bci_report[4]|=2048;
 z=_gz_jpg_check(token,frame);if(z)goto done;
 bci_arming=1;z=_gz_roh_bomb_commit(token,frame);
 if(!z)_gz_snapshot_ui_arm();bci_arming=0;
done:
 _gz_bci_report[5]=z;_gz_bci_report[7]=_gz_pbac_frame;
 bci_entered=bci_arming=bci_token=bci_frame=bci_source_hash=0;return z;
}
