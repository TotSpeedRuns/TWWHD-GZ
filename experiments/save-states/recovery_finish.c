/* Complete before-image closure and retirement of the actual failed attempt.
 * No successful snapshot publication is granted here. Original source/body
 * readers, native actor APIs and the ordinary deadline are never consulted.
 * Only a separately requested, fully rolled-back owned lane may return IDLE.
 */
volatile u32 _gz_boh_finish_request,_gz_boh_finish_ack;
u32 _gz_boh_finish_seal;
u32 _gz_boh_finish_report[32]={0x42434631u,128};
typedef struct {u32 magic,bytes,valid,token,frame,request,owner_hash,mask;
 u32 boh_plan,boh_run,recovery,body,body_run,face,face_run,camera,pot,pot_run,light,bomb,clock,persistent,rng,enemy,enemy_run,enemy_replay,keese_body,keese_body_run,keese_rebind,keese_rebind_run;} BclCertificate;
static BclCertificate _gz_boh_finished;
static u32 bcl_entered,bcl_arming;
extern u32 _gz_roh_recovery_finish(u32,u32);
static u32 bcl_sealed(void){const BclCertificate*c=&_gz_boh_finished;
 return c->magic==0x42464331u&&c->bytes==sizeof(*c)&&c->valid==1&&c->token&&c->request&&c->mask==2047&&
  _gz_boh_finish_seal==btr_hash(c,sizeof(*c));
}
static u32 bcl_current(u32 token,u32 frame){u32 a=0,node,z;
 if(!bor_original()||_gz_boh_recovery.phase!=BOR_DONE||_gz_boh_recovery.error||
  _gz_boh_recovery.token!=token||_gz_boh_recovery.delete_calls!=1||!_gz_boh_recovery.pumps||
  _gz_boh_recovery_request!=_gz_boh_recovery_ack||frame!=_gz_pbac_frame||
  _gz_pbac_active||_gz_pbac_fatal||_gz_pbac_quarantine_emergency||_gz_pbac_issued.valid)return 80;
 z=_gz_roh_recovery_baseline(token,frame);if(z)return z;
 node=boh_target_node(_gz_boh_recovery.pid,&a);
 if(node||a||_gz_pba_native_create_queue||_gz_pba_native_delete_queue||_gz_rll_priority_head||
  R8(_gz_boh_plan.link,0x690c))return 81;
 return bor_delta(0,0,RLL_ABSENT);
}
/* Every actual native writer is rechecked after Delete_Do completed. The bomb
 * actor/model are already gone: only their sealed audit receipt is read. */
static u32 bcl_before_images(u32 token,u32 frame){u32 z,i;const PptPlan*p=&_gz_ppt_plan;
 if(!lpc_sealed()||_gz_lpc_plan.token!=token||_gz_lpc_run.phase!=LPC_ROLLED||_gz_lpc_run.error||
  _gz_lpc_entered)return 231;
 z=lpc_binding(0);if(z)return z;z=lpc_match(_gz_lpc_plan.before,_gz_lpc_plan.before_bind);if(z)return z;
 _gz_boh_finish_report[6]|=2;
 if(!lpf_sealed()||_gz_lpf_plan.token!=token||_gz_lpf_run.phase!=5||_gz_lpf_run.error||_gz_lpf_entered)return 248;
 z=lpf_binding(0);if(z)return z;z=fr_current(&_gz_lpf_plan.leaf);if(z)return z;
 z=fr_before_matches(&_gz_lpf_plan.leaf,0);if(z)return z;_gz_boh_finish_report[6]|=4;
 if(!cct_sealed()||_gz_cct_journal.token!=token||_gz_cct_journal.phase!=CCT_ROLLED||_gz_cct_entered)return 45;
 z=cct_binding();if(z)return z;z=cct_before_links();if(z)return z;
 z=cct_match(_gz_cct_journal.before,_gz_cct_journal.before_links,_gz_cct_journal.before_force);if(z)return z;
 _gz_boh_finish_report[6]|=8;
 if(!ppt_sealed()||p->token!=token||_gz_ppt_run.phase!=PPT_ROLLED||_gz_ppt_run.error||
  !_gz_ppt_run.global_undo_done||_gz_ppt_entered)return 226;
 for(i=0;i<p->count;i++){z=ppt_binding(p->rows+i,frame);if(z)return z;z=ppt_match(p->rows+i,p->rows[i].before,1);if(z)return z;}
 if(!btr_equal(_gz_btr_shared_matrix,p->matrix_before,48)||!btr_equal((const void*)_gz_ppt_offset,p->offset_before,12)||
  _gz_ppt_offset_init!=p->offset_init_before)return 226;
 for(i=0;i<3;i++)if(_gz_ppt_native_spec[i]!=p->spec_audit[i])return 226;
 _gz_boh_finish_report[6]|=16;
 if(!lct_sealed()||_gz_lct_plan.token!=token||_gz_lct_plan.phase!=LCT_ROLLED||_gz_lct_entered||
  !lct_scene(&_gz_lct_plan.before))return 26;
 z=lct_bind(&_gz_lct_plan.before,0);if(z)return z;z=lct_match(&_gz_lct_plan.before);if(z)return z;
 _gz_boh_finish_report[6]|=32;
 if(_gz_btr_journal.phase!=BTR_ROLLED||_gz_btr_journal.token!=token||
  _gz_btr_seal!=btr_hash((const u8*)&_gz_btr_journal+4,sizeof(BtrJournal)-4)||
  _gz_btr_progress[0]!=token||_gz_btr_progress[3]!=5||_gz_btr_progress[4]!=btr_hash(_gz_btr_progress,16))return 28;
 _gz_boh_finish_report[6]|=64;
 z=_gz_ckr_recovery_check(token,frame);if(z)return z;
 _gz_boh_finish_report[6]|=128;
 z=_gz_prk_recovery_check(token,frame);if(z)return z;
 _gz_boh_finish_report[6]|=256;
 z=_gz_rgr_recovery_check(token,frame);if(z)return z;
 _gz_boh_finish_report[6]|=512;
 z=_gz_eks_recovery_check(token,frame);if(z)return z;
 _gz_boh_finish_report[6]|=1024;return 0;
}
u32 _gz_boh_recovery_finish_scope(u32 token,u32 frame){const BclCertificate*c=&_gz_boh_finished;
 return bcl_entered==1&&bcl_arming==1&&bcl_sealed()&&c->token==token&&c->frame==frame&&frame==_gz_pbac_frame&&
  c->request==_gz_boh_finish_request&&_gz_boh_finish_request==_gz_boh_finish_ack+1&&bor_original()&&
  _gz_boh_recovery.phase==BOR_DONE&&c->owner_hash==btr_hash((const void*)_gz_roh_owner,240)&&
  c->boh_plan==_gz_boh_plan_seal&&c->boh_run==_gz_boh_run_seal&&c->recovery==_gz_boh_recovery_seal&&
  c->body==_gz_lpc_seal&&c->body_run==_gz_lpc_run_seal&&c->face==_gz_lpf_seal&&c->face_run==_gz_lpf_run_seal&&
  c->camera==_gz_cct_seal&&c->pot==_gz_ppt_plan_seal&&c->pot_run==_gz_ppt_run_seal&&
  c->light==_gz_lct_seal&&c->bomb==_gz_btr_seal&&c->clock==_gz_ckr_seal&&c->persistent==_gz_prk_seal&&c->rng==_gz_rgr_seal&&
  c->enemy==_gz_eks_seal&&c->enemy_run==_gz_eks_run_seal&&c->enemy_replay==_gz_ekr_seal&&
  c->keese_body==_gz_kbs_seal&&c->keese_body_run==_gz_kbs_run_seal&&
  c->keese_rebind==_gz_kbr_seal&&c->keese_rebind_run==_gz_kbr_run_seal;
}
/* Fixed private destinations. No source, actor, heap, archive or SDK target is
 * accepted. Keep command counters and the retired certificate for monotonic
 * replay protection; all live participant plans return to canonical empty. */
static void bcl_zero(void*p,u32 n){u32 i;for(i=0;i<n;i++)((volatile u8*)p)[i]=0;}
void _gz_boh_recovery_finish_leaf(void){u32 i;
 bcl_zero(&_gz_lpc_plan,sizeof(_gz_lpc_plan));bcl_zero(&_gz_lpc_run,sizeof(_gz_lpc_run));_gz_lpc_seal=_gz_lpc_run_seal=_gz_lpc_entered=0;
 bcl_zero(&_gz_lpf_plan,sizeof(_gz_lpf_plan));bcl_zero(&_gz_lpf_run,sizeof(_gz_lpf_run));_gz_lpf_seal=_gz_lpf_run_seal=_gz_lpf_entered=0;
 bcl_zero(&_gz_cct_journal,sizeof(_gz_cct_journal));_gz_cct_seal=_gz_cct_entered=0;
 bcl_zero(&_gz_ppt_plan,sizeof(_gz_ppt_plan));bcl_zero(&_gz_ppt_run,sizeof(_gz_ppt_run));_gz_ppt_plan_seal=_gz_ppt_run_seal=_gz_ppt_entered=0;
 bcl_zero(&_gz_lct_plan,sizeof(_gz_lct_plan));_gz_lct_seal=_gz_lct_entered=0;
 bcl_zero(&_gz_rgr_plan,sizeof(_gz_rgr_plan));_gz_rgr_seal=_gz_rgr_entered=0;
 bcl_zero(&_gz_ern_resolved,sizeof(_gz_ern_resolved));
 kbr_clear();_gz_kbr_entered=0;kbs_clear();_gz_kbs_entered=0;ekr_clear();eks_clear();_gz_eks_entered=0;
 bcl_zero(&_gz_prk_plan,sizeof(_gz_prk_plan));_gz_prk_seal=_gz_prk_entered=_gz_prk_kernel_mode=_gz_prk_kernel_frame=0;
 bcl_zero(&_gz_ckr_plan,sizeof(_gz_ckr_plan));_gz_ckr_seal=_gz_ckr_entered=_gz_ckr_kernel_mode=_gz_ckr_kernel_frame=0;
 bcl_zero(&_gz_btr_journal,sizeof(_gz_btr_journal));bcl_zero(_gz_btr_progress,sizeof(_gz_btr_progress));_gz_btr_seal=0;
 bcl_zero(&_gz_hbp_recipe,sizeof(_gz_hbp_recipe));_gz_hbp_seal=0;
 bcl_zero(&_gz_bmv_views,sizeof(_gz_bmv_views));_gz_bmv_seal=_gz_bmv_entered=0;
 bcl_zero(_gz_bmc_b294,sizeof(_gz_bmc_b294));_gz_bmc_entered=0;
 bcl_zero(&_gz_bmh_receipt,sizeof(_gz_bmh_receipt));_gz_bmh_seal=_gz_bmh_busy=_gz_bmh_token=_gz_bmh_mask=_gz_bmh_progress_seal=0;
 bcl_zero(&_gz_bmh_abort,sizeof(_gz_bmh_abort));_gz_bmh_abort_seal=0;
 for(i=2;i<64;i++)_gz_bmh_report[i]=0;
 bcl_zero(&_gz_boh_plan,sizeof(_gz_boh_plan));bcl_zero(&_gz_boh_journal,sizeof(_gz_boh_journal));
 bcl_zero(&_gz_boh_tracker,sizeof(_gz_boh_tracker));bcl_zero(&_gz_boh_issued_key,sizeof(_gz_boh_issued_key));
 _gz_boh_plan_seal=_gz_boh_run_seal=_gz_boh_tracker_seal=_gz_boh_key_seal=_gz_boh_pumping=0;
 bcl_zero(&_gz_boh_recovery,sizeof(_gz_boh_recovery));_gz_boh_recovery_seal=_gz_boh_recovery_arming=_gz_boh_recovery_entered=0;
}
/* Default request0 preserves the independently tested held diagnostic. A new
 * request must already be queued when its bounded recovery reaches DONE. */
static u32 bcl_step(void){BclCertificate*c=&_gz_boh_finished;u32 req=_gz_boh_finish_request,frame=_gz_pbac_frame,token,z;
 if(!req||req==_gz_boh_finish_ack)return 0;
 if(bcl_entered)return 1;
 if(!bor_sealed()||_gz_boh_recovery.phase!=BOR_DONE)return 1;
 bcl_entered=1;token=_gz_boh_recovery.token;
 _gz_boh_finish_report[2]=req;_gz_boh_finish_report[4]=token;_gz_boh_finish_report[5]=frame;_gz_boh_finish_report[6]=0;
 if(req!=_gz_boh_finish_ack+1){z=80;goto done;}
 z=bcl_current(token,frame);if(z)goto done;_gz_boh_finish_report[6]=1;
 z=bcl_before_images(token,frame);if(z)goto done;
 z=bcl_current(token,frame);if(z)goto done;
 c->magic=0x42464331u;c->bytes=sizeof(*c);c->valid=1;c->token=token;c->frame=frame;c->request=req;
 c->owner_hash=btr_hash((const void*)_gz_roh_owner,240);c->mask=_gz_boh_finish_report[6];
 c->boh_plan=_gz_boh_plan_seal;c->boh_run=_gz_boh_run_seal;c->recovery=_gz_boh_recovery_seal;
 c->body=_gz_lpc_seal;c->body_run=_gz_lpc_run_seal;c->face=_gz_lpf_seal;c->face_run=_gz_lpf_run_seal;
 c->camera=_gz_cct_seal;c->pot=_gz_ppt_plan_seal;c->pot_run=_gz_ppt_run_seal;c->light=_gz_lct_seal;c->bomb=_gz_btr_seal;c->clock=_gz_ckr_seal;c->persistent=_gz_prk_seal;c->rng=_gz_rgr_seal;
 c->enemy=_gz_eks_seal;c->enemy_run=_gz_eks_run_seal;c->enemy_replay=_gz_ekr_seal;
 c->keese_body=_gz_kbs_seal;c->keese_body_run=_gz_kbs_run_seal;
 c->keese_rebind=_gz_kbr_seal;c->keese_rebind_run=_gz_kbr_run_seal;
 _gz_boh_finish_seal=btr_hash(c,sizeof(*c));
 _gz_boh_finish_report[7]=_gz_roh_owner[7];_gz_boh_finish_report[8]=_gz_osa_pin_id();
 _gz_boh_finish_report[11]=_gz_boh_plan.source_pid;_gz_boh_finish_report[12]=_gz_boh_recovery.pid;
 _gz_boh_finish_report[13]=_gz_ppt_plan.count;_gz_boh_finish_report[14]=_gz_lct_plan.saved.count;
 bcl_arming=1;z=_gz_roh_recovery_finish(token,frame);bcl_arming=0;
 _gz_boh_finish_report[9]=_gz_osa_pin_id();_gz_boh_finish_report[10]=_gz_roh_owner[3];
done:_gz_boh_finish_report[3]=z;__asm__ volatile("sync":::"memory");_gz_boh_finish_ack=req;bcl_entered=0;return 1;
}
