/* Whole-bank current-generation skeletal before-images. All rows finish
 * before the first existing pattern SDK write. This stage performs no body
 * SDK operation. Source-free recovery checks every original body image. */
enum {KBS_CONTEXT=550,KBS_RESOURCE=551,KBS_CHANGED=552,KBS_ALIAS=553};
typedef struct {u32 index,pid,actor,generation;ErnResource before_resource,target_resource;KbjPlan journal;KrjPlan renderer;} KbsRow;
typedef struct {u32 magic,bytes,version,valid,token,frame,count,prepared,enemy_seal;KbsRow rows[EKS_CAP];} KbsPlan;
typedef struct {u32 phase,token,prepared_frame,rolled_frame,error;} KbsRun;
typedef char KbsRowSize[sizeof(KbsRow)==15568?1:-1];
typedef char KbsPlanSize[sizeof(KbsPlan)==1494564?1:-1];
static KbsPlan _gz_kbs_plan;static KbsRun _gz_kbs_run;
static u32 _gz_kbs_seal,_gz_kbs_run_seal,_gz_kbs_entered;
u32 _gz_kbs_report[32]={0x4b425331u,128};
u32 _gz_kbs_private_apart(u32 p,u32 n){return
 eks_apart(p,n,(u32)&_gz_kbs_plan,sizeof(_gz_kbs_plan))&&eks_apart(p,n,(u32)&_gz_kbs_run,sizeof(_gz_kbs_run))&&
 eks_apart(p,n,(u32)&_gz_kbs_seal,4)&&eks_apart(p,n,(u32)&_gz_kbs_run_seal,4)&&
 eks_apart(p,n,(u32)&_gz_kbs_entered,4)&&eks_apart(p,n,(u32)_gz_kbs_report,sizeof(_gz_kbs_report));}
static void kbs_run_seal(void){_gz_kbs_run_seal=btr_hash(&_gz_kbs_run,sizeof(_gz_kbs_run));}
static u32 kbs_empty(void){return !_gz_kbs_plan.valid&&!_gz_kbs_run.phase&&!_gz_kbs_seal&&!_gz_kbs_run_seal;}
static u32 kbs_sealed(void){const KbsPlan*p=&_gz_kbs_plan;const KbsRun*r=&_gz_kbs_run;
 return p->magic==0x4b425331u&&p->version==1&&p->valid==1&&p->token&&p->count<=EKS_CAP&&p->prepared==p->count&&
  p->bytes==36+p->count*sizeof(KbsRow)&&p->enemy_seal==_gz_eks_seal&&eks_sealed()&&
  _gz_kbs_seal==btr_hash(p,p->bytes)&&r->token==p->token&&(r->phase==1||r->phase==5)&&
  _gz_kbs_run_seal==btr_hash(r,sizeof(*r));}
static void kbs_clear(void){u32 n=_gz_kbs_plan.count;
 /* Every active row is scrubbed on both refusal and retirement. Previously
  * inactive rows are never published or read as authority. */
 if(n>EKS_CAP)n=EKS_CAP;eks_zero(&_gz_kbs_plan,36+n*sizeof(KbsRow));eks_zero(&_gz_kbs_run,sizeof(_gz_kbs_run));
 _gz_kbs_seal=_gz_kbs_run_seal=0;}
static u32 kbs_resource(const ErnResource*r){ErnResource now;u32 z;
 if(r->magic!=0x45524e31u||r->bytes!=sizeof(*r)||r->version!=1||r->valid!=1||r->profile!=215||r->kind!=ERN_BODY||
  r->token!=_gz_kbs_plan.token||r->frame!=_gz_kbs_plan.frame||r->key!=0x663a5dcdu||r->wrapper_bytes!=20||
  !r->count||r->count>4096||!eks_external(r->header,r->header_bytes)||!eks_external(r->table,r->count*4)||
  !eks_external(r->wrapper,20))return KBS_RESOURCE;
 btr_copy(&now,r,sizeof(now));z=ern_archive(&now,0x4b690000u);if(z)return z;z=ern_wrapper(&now);if(z)return z;
 return btr_equal(&now,r,84)&&btr_equal(now.identity,r->identity,16)?0:KBS_CHANGED;}
static u32 kbs_targets(const KbjLayout*l){const EpjBounds*b=&l->common.bounds;const EptResources*r=&l->resources;
 return eks_external(b->actor,b->actor_bytes)&&eks_external(b->heap,b->end-b->heap)&&
  eks_external(r->header,r->bytes)&&eks_external(r->table,r->count*4)&&eks_external(r->model_wrapper,0x3c)&&
  eks_external(l->before_body,20)&&eks_external(l->target_body,20)&&eks_external(l->bound_before,20)?0:KBS_ALIAS;}
static u32 kbs_heaps(const EksRow*e,const KbjLayout*l){u32 z,buffer,bytes;
 z=eks_heap(&e->heap,e->heap.heap,l->buffer,l->buffer_bytes);if(z)return z;
 z=eks_heap(&e->heap,e->heap.heap,l->transforms,l->bones*32);if(z)return z;
 z=eks_heap(&e->heap,e->heap.heap,l->quaternions,l->bones*16);if(z)return z;
 z=krj_shape(l,&buffer,&bytes);if(z)return z;
 if(!eks_external(buffer,bytes))return KBS_ALIAS;
 return eks_heap(&e->heap,e->heap.heap,buffer,bytes);}
static u32 kbs_before_matches(const KbsRow*r){u32 z=_gz_kbj_before_matches(&r->journal);
 return z?z:_gz_krj_before_matches(&r->journal,&r->renderer);}
static u32 kbs_binding(const KbsRow*r){const EksRow*e;u32 z;
 if(r->index>=_gz_eks_plan.count)return KBS_CHANGED;e=_gz_eks_plan.rows+r->index;
 if(e->pid!=r->pid||e->actor!=r->actor||e->generation!=r->generation||
  !btr_equal(&r->journal.layout.common.bounds,&e->journal.layout.bounds,sizeof(EpjBounds))||
  !btr_equal(&r->journal.layout.resources,&e->target.resources,sizeof(EptResources))||
  r->journal.layout.before_body!=r->before_resource.wrapper||r->journal.layout.target_body!=r->target_resource.wrapper)return KBS_CHANGED;
 z=eks_binding(e);if(z)return z;z=kbs_resource(&r->before_resource);if(z)return z;z=kbs_resource(&r->target_resource);if(z)return z;
 z=kbs_targets(&r->journal.layout);if(z)return z;z=kbs_heaps(e,&r->journal.layout);if(z)return z;
 return _gz_krj_validate(&r->journal,&r->renderer);}
u32 _gz_kbs_service(u32 token){KbsPlan*p=&_gz_kbs_plan;u32 frame=_gz_pbac_frame,i,j,body,z=0;KbjLayout layout;KbtPlan contract;
 if(_gz_kbs_entered||!kbs_empty()||!eks_sealed()||_gz_eks_plan.token!=token||_gz_eks_run.phase!=1||
  _gz_eks_plan.frame!=frame||!ekr_empty()||!_gz_ern_scope(token,frame))return KBS_CONTEXT;
 _gz_kbs_entered=1;p->magic=0x4b425331u;p->bytes=36+_gz_eks_plan.count*sizeof(KbsRow);p->version=1;
 p->token=token;p->frame=frame;p->count=_gz_eks_plan.count;p->enemy_seal=_gz_eks_seal;
 for(i=0;i<p->count;i++){
  const EksRow*e=_gz_eks_plan.rows+i;KbsRow*r=p->rows+i;_gz_kbs_report[6]=i;
  z=eks_binding(e);if(z)goto done;
  if(!eks_external(e->source_row,10240)||btr_hash((const void*)e->source_row,10240)!=e->source_hash){z=KBS_CHANGED;goto done;}
  r->index=i;r->pid=e->pid;r->actor=e->actor;r->generation=e->generation;
  body=R32(e->journal.layout.bounds.morph,0x94);
  for(j=0;j<e->target_resource.count;j++)if(R32(e->target_resource.table,j*4)==body)break;
  if(j==e->target_resource.count){z=KBS_RESOURCE;goto done;}
  z=_gz_ern_resolve(token,215,j,ERN_BODY);if(z)goto done;btr_copy(&r->before_resource,&_gz_ern_resolved,sizeof(ErnResource));
  z=_gz_ern_resolve(token,215,R32(e->source_row,4832),ERN_BODY);if(z)goto done;btr_copy(&r->target_resource,&_gz_ern_resolved,sizeof(ErnResource));
  z=kbs_resource(&r->before_resource);if(z)goto done;z=kbs_resource(&r->target_resource);if(z)goto done;
  z=_gz_kbj_describe(&e->journal.layout.bounds,&e->target.resources,r->before_resource.wrapper,r->target_resource.wrapper,&layout);if(z)goto done;
  z=kbs_targets(&layout);if(z)goto done;z=kbs_heaps(e,&layout);if(z)goto done;
  z=_gz_kbj_prepare(&e->journal.layout.bounds,&e->target.resources,r->before_resource.wrapper,r->target_resource.wrapper,&r->journal);if(z)goto done;
  z=_gz_krj_prepare(&r->journal,&r->renderer);if(z)goto done;
  z=_gz_kbt_contract(&r->journal,&contract);if(z)goto done;
  z=kbs_binding(r);if(z)goto done;z=kbs_before_matches(r);if(z)goto done;p->prepared++;
 }
 for(i=0;i<p->count;i++){z=kbs_binding(p->rows+i);if(z)goto done;z=kbs_before_matches(p->rows+i);if(z)goto done;}
 if(!_gz_ern_scope(token,frame)||btr_hash((const void*)_gz_eks_plan.bank,_gz_eks_plan.bank_bytes)!=_gz_eks_plan.bank_hash){z=KBS_CHANGED;goto done;}
 p->valid=1;_gz_kbs_seal=btr_hash(p,p->bytes);_gz_kbs_run.phase=1;_gz_kbs_run.token=token;_gz_kbs_run.prepared_frame=frame;kbs_run_seal();
done:
 _gz_kbs_report[2]=token;_gz_kbs_report[3]=z?0:1;_gz_kbs_report[4]=z;_gz_kbs_report[5]=frame;
 _gz_kbs_report[7]=p->count;_gz_kbs_report[8]=p->prepared;_gz_kbs_report[9]=_gz_kbs_seal;
 if(z)kbs_clear();_gz_kbs_entered=0;return z;}
u32 _gz_kbs_recovery_rollback(u32 token){u32 frame=_gz_pbac_frame,z,i;
 if(_gz_kbs_entered||!token||!bor_original()||_gz_boh_recovery.phase!=BOR_UNDO||_gz_boh_recovery.token!=token)return KBS_CONTEXT;
 z=_gz_roh_recovery_common(token,frame);if(z)return z;if(kbs_empty())return 0;
 if(!kbs_sealed()||_gz_kbs_plan.token!=token||_gz_kbs_run.phase!=1)return KBS_CHANGED;
 _gz_kbs_entered=1;
 for(i=0;i<_gz_kbs_plan.count;i++){z=kbs_binding(_gz_kbs_plan.rows+i);if(z)goto done;z=kbs_before_matches(_gz_kbs_plan.rows+i);if(z)goto done;}
 z=_gz_roh_recovery_common(token,frame);if(!z){_gz_kbs_run.phase=5;_gz_kbs_run.rolled_frame=frame;kbs_run_seal();}
done:_gz_kbs_report[10]=z;_gz_kbs_entered=0;return z;}
u32 _gz_kbs_recovery_check(u32 token,u32 frame){u32 z,i;
 if(_gz_kbs_entered||!token||frame!=_gz_pbac_frame||!bor_original()||_gz_boh_recovery.phase!=BOR_DONE||_gz_boh_recovery.token!=token)return KBS_CONTEXT;
 z=_gz_roh_recovery_common(token,frame);if(z)return z;if(kbs_empty())return 0;
 if(!kbs_sealed()||_gz_kbs_plan.token!=token||_gz_kbs_run.phase!=5||_gz_kbs_run.error)return KBS_CHANGED;
 for(i=0;i<_gz_kbs_plan.count;i++){z=kbs_binding(_gz_kbs_plan.rows+i);if(z)return z;z=kbs_before_matches(_gz_kbs_plan.rows+i);if(z)return z;}
 return _gz_roh_recovery_common(token,frame);}
