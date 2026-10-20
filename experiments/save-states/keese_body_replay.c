/* Whole-bank existing-allocation skeletal binding. Every journal and typed
 * contract is sealed before the first SDK call; each row is marked attempted
 * before any selection store. This is one held transaction participant, not
 * successful snapshot publication. Each row retains a typed saved morph,
 * frame and interpolation plan before the first native operation. */
enum {KBR_CONTEXT=570,KBR_CHANGED=571,KBR_SOURCE=572};
typedef struct {KbnPlan numeric;KrnPlan renderer;} KbrRow;
typedef struct {u32 magic,bytes,valid,token,frame,count,prepared,enemy_seal,body_seal;KbrRow rows[EKS_CAP];} KbrPlan;
typedef struct {u32 phase,token,frame,attempted,applied,undone,error,rollback_error;} KbrRun;
typedef char KbrRowSize[sizeof(KbrRow)==7036?1:-1];
typedef char KbrPlanSize[sizeof(KbrPlan)==675492?1:-1];
typedef char KbrRunSize[sizeof(KbrRun)==32?1:-1];
static KbrPlan _gz_kbr_plan;static KbrRun _gz_kbr_run;
static u32 _gz_kbr_seal,_gz_kbr_run_seal,_gz_kbr_entered;
u32 _gz_kbr_report[32]={0x4b425231u,128};
u32 _gz_kbr_private_apart(u32 p,u32 n){return
 eks_apart(p,n,(u32)&_gz_kbr_plan,sizeof(_gz_kbr_plan))&&eks_apart(p,n,(u32)&_gz_kbr_run,sizeof(_gz_kbr_run))&&
 eks_apart(p,n,(u32)&_gz_kbr_seal,4)&&eks_apart(p,n,(u32)&_gz_kbr_run_seal,4)&&
 eks_apart(p,n,(u32)&_gz_kbr_entered,4)&&eks_apart(p,n,(u32)_gz_kbr_report,sizeof(_gz_kbr_report));}
static void kbr_run_seal(void){_gz_kbr_run_seal=btr_hash(&_gz_kbr_run,sizeof(_gz_kbr_run));}
static u32 kbr_empty(void){return !_gz_kbr_plan.valid&&!_gz_kbr_run.phase&&!_gz_kbr_seal&&!_gz_kbr_run_seal;}
static u32 kbr_sealed(void){const KbrPlan*p=&_gz_kbr_plan;const KbrRun*r=&_gz_kbr_run;
 return p->magic==0x4b425231u&&p->valid==1&&p->token&&p->count<=EKS_CAP&&p->count==_gz_kbs_plan.count&&p->prepared==p->count&&
  p->bytes==36+p->count*sizeof(KbrRow)&&p->enemy_seal==_gz_eks_seal&&p->body_seal==_gz_kbs_seal&&kbs_sealed()&&
  _gz_kbr_seal==btr_hash(p,p->bytes)&&r->token==p->token&&r->frame==p->frame&&r->phase>=2&&r->phase<=5&&
  r->attempted<=p->count&&r->applied<=r->attempted&&r->undone<=r->attempted&&_gz_kbr_run_seal==btr_hash(r,sizeof(*r));}
static void kbr_report(void){const KbrRun*r=&_gz_kbr_run;
 _gz_kbr_report[2]=r->token;_gz_kbr_report[3]=r->phase;_gz_kbr_report[4]=r->error;_gz_kbr_report[5]=r->frame;
 _gz_kbr_report[6]=_gz_kbr_plan.count;_gz_kbr_report[7]=r->attempted;_gz_kbr_report[8]=r->applied;_gz_kbr_report[9]=r->undone;
 _gz_kbr_report[10]=r->rollback_error;_gz_kbr_report[11]=_gz_kbr_seal;_gz_kbr_report[12]=_gz_kbr_run_seal;}
static void kbr_clear(void){u32 n=_gz_kbr_plan.count;if(n>EKS_CAP)n=EKS_CAP;
 eks_zero(&_gz_kbr_plan,36+n*sizeof(KbrRow));eks_zero(&_gz_kbr_run,sizeof(_gz_kbr_run));_gz_kbr_seal=_gz_kbr_run_seal=0;}
/* Private cancellation witness. A leaf called outside this prepared writer
 * has no diagnostic request to consume. A genuine point11 error returns
 * normally through apply/service_end before independent recovery may begin. */
u32 _gz_kbt_attempt_boundary(void){const KbrRun*r=&_gz_kbr_run;
 if(!_gz_kbr_entered||!kbr_sealed()||r->phase!=2||r->attempted!=r->applied+1)return 0;
 return _gz_bif_after_part(r->token,3,11);}
u32 _gz_krn_write_boundary(void){const KbrRun*r=&_gz_kbr_run;
 if(!_gz_kbr_entered||!kbr_sealed()||r->phase!=2||r->attempted!=r->applied+1)return 0;
 return _gz_bif_after_part(r->token,3,12);}
static u32 kbr_binding(u32 index){KbtPlan now;u32 z;
 if(index>=_gz_kbr_plan.count||index>=_gz_kbs_plan.count)return KBR_CHANGED;
 z=kbs_binding(_gz_kbs_plan.rows+index);if(z)return z;
 z=_gz_kbt_contract(&_gz_kbs_plan.rows[index].journal,&now);if(z)return z;
 if(!btr_equal(&now,&_gz_kbr_plan.rows[index].numeric.binding,sizeof(now)))return KBR_CHANGED;
 z=_gz_kbn_validate(&_gz_kbs_plan.rows[index].journal,&_gz_kbr_plan.rows[index].numeric);if(z)return z;
 return _gz_krn_validate(&_gz_kbs_plan.rows[index].journal,&_gz_kbs_plan.rows[index].renderer,
  &_gz_kbr_plan.rows[index].numeric,&_gz_kbr_plan.rows[index].renderer);}
u32 _gz_kbr_apply(u32 token){KbrPlan*p=&_gz_kbr_plan;KbrRun*r=&_gz_kbr_run;u32 i,frame=_gz_pbac_frame,z=0;KbtPlan contract;
 if(_gz_kbr_entered||!kbr_empty()||!kbs_sealed()||_gz_kbs_plan.token!=token||_gz_kbs_plan.frame!=frame||
  _gz_kbs_run.phase!=1||!ekr_empty()||!_gz_ern_scope(token,frame))return KBR_CONTEXT;
 _gz_kbr_entered=1;p->magic=0x4b425231u;p->bytes=36+_gz_kbs_plan.count*sizeof(KbrRow);p->token=token;p->frame=frame;
 p->count=_gz_kbs_plan.count;p->enemy_seal=_gz_eks_seal;p->body_seal=_gz_kbs_seal;
 for(i=0;i<p->count;i++){
  z=kbs_binding(_gz_kbs_plan.rows+i);if(z)goto prepare_failed;
  z=kbs_before_matches(_gz_kbs_plan.rows+i);if(z)goto prepare_failed;
  z=_gz_kbt_contract(&_gz_kbs_plan.rows[i].journal,&contract);if(z)goto prepare_failed;
  z=_gz_kbn_prepare(&_gz_kbs_plan.rows[i].journal,&contract,_gz_eks_plan.rows[i].source_row,_gz_kbs_plan.rows[i].target_resource.index,&p->rows[i].numeric);if(z)goto prepare_failed;
  if(p->rows[i].numeric.source_hash!=_gz_eks_plan.rows[i].source_hash){z=KBR_SOURCE;goto prepare_failed;}
  z=_gz_krn_prepare(&_gz_kbs_plan.rows[i].journal,&_gz_kbs_plan.rows[i].renderer,&p->rows[i].numeric,
   _gz_eks_plan.rows[i].source_row,&p->rows[i].renderer);if(z)goto prepare_failed;p->prepared++;
 }
 for(i=0;i<p->count;i++){z=kbr_binding(i);if(z)goto prepare_failed;z=kbs_before_matches(_gz_kbs_plan.rows+i);if(z)goto prepare_failed;}
 if(!_gz_ern_scope(token,frame)||btr_hash((const void*)_gz_eks_plan.bank,_gz_eks_plan.bank_bytes)!=_gz_eks_plan.bank_hash){z=KBR_SOURCE;goto prepare_failed;}
 p->valid=1;_gz_kbr_seal=btr_hash(p,p->bytes);r->phase=2;r->token=token;r->frame=frame;kbr_run_seal();
 for(i=0;i<p->count;i++){
  z=kbr_binding(i);if(z)goto done;if(!_gz_ern_scope(token,frame)){z=KBR_CONTEXT;goto done;}
  r->attempted=i+1;kbr_run_seal();_gz_kbr_report[13]=i;
  z=_gz_kbt_rebind(&_gz_kbs_plan.rows[i].journal,&p->rows[i].numeric.binding);_gz_kbr_report[14]=z;if(z)goto done;
  z=_gz_kbn_apply(&_gz_kbs_plan.rows[i].journal,&p->rows[i].numeric);_gz_kbr_report[15]=z;if(z)goto done;
  z=_gz_krn_apply(&_gz_kbs_plan.rows[i].journal,&_gz_kbs_plan.rows[i].renderer,&p->rows[i].numeric,&p->rows[i].renderer);_gz_kbr_report[16]=z;if(z)goto done;
  z=_gz_kbn_matches(&_gz_kbs_plan.rows[i].journal,&p->rows[i].numeric);if(z)goto done;
  z=_gz_krn_matches(&_gz_kbs_plan.rows[i].journal,&_gz_kbs_plan.rows[i].renderer,&p->rows[i].numeric,&p->rows[i].renderer);if(z)goto done;
  r->applied++;kbr_run_seal();
  /* Diagnostic point10 is after one real row's two SDK calls. It cannot
   * manufacture a receipt, reset counters or grant ordinary admission. */
  if(!i){z=_gz_bif_after_part(token,3,10);if(z)goto done;}
 }
 for(i=0;i<p->count;i++){z=kbr_binding(i);if(z)goto done;z=_gz_kbn_matches(&_gz_kbs_plan.rows[i].journal,&p->rows[i].numeric);if(z)goto done;
  z=_gz_krn_matches(&_gz_kbs_plan.rows[i].journal,&_gz_kbs_plan.rows[i].renderer,&p->rows[i].numeric,&p->rows[i].renderer);if(z)goto done;}
 if(!_gz_ern_scope(token,frame)||btr_hash((const void*)_gz_eks_plan.bank,_gz_eks_plan.bank_bytes)!=_gz_eks_plan.bank_hash){z=KBR_SOURCE;goto done;}
 r->phase=3;
done:r->error=z;kbr_run_seal();kbr_report();_gz_kbr_entered=0;return z;
prepare_failed:kbr_clear();_gz_kbr_report[2]=token;_gz_kbr_report[4]=z;_gz_kbr_entered=0;return z;}
u32 _gz_kbr_recovery_undo(u32 token){KbrRun*r=&_gz_kbr_run;u32 i,z,frame=_gz_pbac_frame;
 if(kbr_empty())return 0;
 if(_gz_kbr_entered||!token||!bor_original()||_gz_boh_recovery.phase!=BOR_UNDO||_gz_boh_recovery.token!=token||
  !kbr_sealed()||r->token!=token||(r->phase!=2&&r->phase!=3&&r->phase!=5)||r->rollback_error)return KBR_CONTEXT;
 z=_gz_roh_recovery_common(token,frame);if(z)return z;
 for(i=0;i<_gz_kbr_plan.count;i++){z=kbr_binding(i);if(z)return z;}
 if(r->phase==5)return 0;_gz_kbr_entered=1;r->phase=4;kbr_run_seal();
 for(i=r->attempted;i;i--){const KbsRow*b=_gz_kbs_plan.rows+i-1;
  z=_gz_krj_undo(&b->journal,&b->renderer);if(z)goto done;
  z=_gz_kbj_undo(&b->journal);if(z)goto done;r->undone++;kbr_run_seal();}
 for(i=0;i<_gz_kbs_plan.count;i++){z=kbs_before_matches(_gz_kbs_plan.rows+i);if(z)goto done;}
 z=_gz_roh_recovery_common(token,frame);if(!z)r->phase=5;
done:r->rollback_error=z;kbr_run_seal();kbr_report();_gz_kbr_entered=0;return z;}
u32 _gz_kbr_recovery_check(u32 token,u32 frame){u32 z,i;
 if(kbr_empty())return 0;
 if(_gz_kbr_entered||frame!=_gz_pbac_frame||!token||!bor_original()||_gz_boh_recovery.phase!=BOR_DONE||
  _gz_boh_recovery.token!=token||!kbr_sealed()||_gz_kbr_run.token!=token||_gz_kbr_run.phase!=5||
  _gz_kbr_run.rollback_error||_gz_kbr_run.undone!=_gz_kbr_run.attempted)return KBR_CHANGED;
 z=_gz_roh_recovery_common(token,frame);if(z)return z;
 for(i=0;i<_gz_kbr_plan.count;i++){z=kbr_binding(i);if(z)return z;z=kbs_before_matches(_gz_kbs_plan.rows+i);if(z)return z;}
 return _gz_roh_recovery_common(token,frame);}
