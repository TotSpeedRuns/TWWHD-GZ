/* Transaction-wide Keese pattern preparation. Every current-generation
 * journal is captured before a future shared-resource writer may run.
 * This stage issues no SDK call and changes no actor/resource bytes.
 * Recovery checks are source-free; normal restore admission stays disabled.
 */
enum {EKS_CAP=96,EKS_CONTEXT=520,EKS_PROCESS=521,EKS_HEAP=522,
 EKS_ARCHIVE=523,EKS_SOURCE=524,EKS_ALIAS=525,EKS_CHANGED=526};
typedef struct {
 u32 pid,actor,index,generation,source_row,source_hash;
 PoEntry process;GhIdentity heap;
 ErnResource before_resource,target_resource;
 EptPlan target;
 EpjPlan journal;
} EksRow;
typedef struct {
 u32 magic,bytes,version,valid,token,frame,count,source,used,pin;
 u32 bank,bank_bytes,bank_hash,world,world_hash,birth,birth_hash,prepared;
 EksRow rows[EKS_CAP];
} EksPlan;
typedef struct {u32 phase,token,prepared_frame,rolled_frame,error;} EksRun;
static EksPlan _gz_eks_plan;
static EksRun _gz_eks_run;
static u32 _gz_eks_seal,_gz_eks_run_seal,_gz_eks_entered;
u32 _gz_eks_report[32]={0x454b5331u,128};
typedef char EksRowSize[sizeof(EksRow)==9040?1:-1];
typedef char EksPlanSize[sizeof(EksPlan)==867912?1:-1];
static void eks_run_seal(void){_gz_eks_run_seal=btr_hash(&_gz_eks_run,sizeof(EksRun));}
static u32 eks_sealed(void){const EksPlan*p=&_gz_eks_plan;const EksRun*r=&_gz_eks_run;
 return p->magic==0x454b5331u&&p->version==1&&p->valid==1&&p->token&&p->count<=EKS_CAP&&
  p->prepared==p->count&&p->bytes==72+p->count*sizeof(EksRow)&&
  _gz_eks_seal==btr_hash(p,p->bytes)&&r->token==p->token&&(r->phase==1||r->phase==5)&&
  _gz_eks_run_seal==btr_hash(r,sizeof(*r));
}
static u32 eks_apart(u32 a,u32 n,u32 b,u32 m){return a>=b?a-b>=m:b-a>=n;}
static u32 eks_external(u32 p,u32 n){return span(p,n)&&_gz_ekr_private_apart(p,n)&&_gz_kbs_private_apart(p,n)&&_gz_kbr_private_apart(p,n)&&_gz_krc_private_apart(p,n)&&
 eks_apart(p,n,(u32)&_gz_eks_plan,sizeof(_gz_eks_plan))&&
 eks_apart(p,n,(u32)&_gz_eks_run,sizeof(_gz_eks_run))&&
 eks_apart(p,n,(u32)_gz_eks_report,sizeof(_gz_eks_report));}
static u32 eks_find(u32 pid,u32*out,PoEntry*process){u32 q=_gz_pba_native_actor_queue,left=4096,a,found=0;const PoEntry*p;
 *out=0;
 while(q){if(!left--||!span(q,20))return EKS_PROCESS;a=R32(q,12);
  if(a){if(!span(a,16))return EKS_PROCESS;if(R32(a,4)==pid){if(found)return EKS_PROCESS;found=a;}}
  q=R32(q,8);
 }
 if(!found||_gz_po_lookup(found)||!_gz_po_observation.valid||_gz_po_observation.flags)return EKS_PROCESS;
 p=&_gz_po_observation.entry;
 if(p->object!=found||p->pid!=pid||p->process_name!=215||p->state!=PO_LIVE||!p->generation||p->flags||p->end_frame||
  p->profile>=484||p->base_bytes+p->other_bytes<p->base_bytes||p->allocation_bytes!=p->base_bytes+p->other_bytes||
  p->allocation_bytes<0x10bc||p->allocation_bytes>0x100000||!eks_external(found,p->allocation_bytes)||
  R16(found,8)!=215||R8(found,12)!=2||R16(found,14)!=p->profile||R32(found,16)!=p->profile_entry)return EKS_PROCESS;
 btr_copy(process,p,sizeof(*p));*out=found;return 0;
}
/* Each complete current object is qualified through the real registered
 * solid heap before its payload is read. The helper's output is copied;
 * subsequent helper calls cannot silently replace this row's heap binding.
 */
static u32 eks_heap(const GhIdentity*expected,u32 heap,u32 p,u32 n){u32 z;
 z=_gz_gh_solid(heap,p,n);if(z||!_gz_gh_identity.valid)return EKS_HEAP;
 if(expected&&!btr_equal((const void*)&_gz_gh_identity,expected,40))return EKS_CHANGED;
 return 0;
}
static u32 eks_resource(const ErnResource*r){ErnResource now;u32 z;
 if(!_gz_ekr_private_apart(r->wrapper,r->wrapper_bytes))return EKS_ALIAS;
 if(r->magic!=0x45524e31u||r->bytes!=sizeof(*r)||r->valid!=1||r->profile!=215||r->kind!=ERN_PATTERN||
  r->key!=0x663a5dcdu||r->wrapper_bytes!=16||!r->count||r->count>4096||!eks_external(r->header,r->header_bytes)||
  !eks_external(r->table,r->count*4)||!eks_external(r->wrapper,16))return EKS_ARCHIVE;
 btr_copy(&now,r,sizeof(now));z=ern_archive(&now,0x4b690000u);if(z)return z;
 z=ern_wrapper(&now);if(z)return z;
 /* Shared frame+0 is reversible state, not archive identity. */
 return btr_equal(&now,r,84)&&btr_equal(now.identity,r->identity,16)?0:EKS_CHANGED;
}
static u32 eks_logical(const EksRow*r){ErnResource now;u32 p=R32(r->actor,0x4e0),i,found=0,z;
 if(!p)return 0;
 if(!_gz_ekr_private_apart(p,16))return EKS_ALIAS;
 btr_copy(&now,&r->target_resource,sizeof(now));
 for(i=30;i<38&&i<now.count;i++)if(R32(now.table,i*4)==p){if(found)return EKS_ARCHIVE;found=i;}
 if(!found)return EKS_ARCHIVE;now.index=found;
 z=ern_archive(&now,0x4b690000u);if(z)return z;z=ern_wrapper(&now);if(z)return z;
 return now.wrapper==p?0:EKS_CHANGED;
}
static u32 eks_binding(const EksRow*r){PoEntry process;const EpjBounds*b=&r->journal.layout.bounds;u32 a,z;
 z=eks_find(r->pid,&a,&process);if(z)return z;
 if(a!=r->actor||process.generation!=r->generation||!btr_equal(&process,&r->process,sizeof(process))||
  R32(a,0xf4)!=r->heap.heap||R32(a,0x3d0)!=b->morph||R32(a,0x4e4)!=b->face)return EKS_CHANGED;
 z=eks_heap(&r->heap,r->heap.heap,b->morph,0xc8);if(z)return z;
 if(R32(b->morph,0x90)!=b->model)return EKS_CHANGED;
 z=eks_heap(&r->heap,r->heap.heap,b->model,0xb0);if(z)return z;
 z=eks_heap(&r->heap,r->heap.heap,b->face,0x74);if(z)return z;
 z=eks_heap(&r->heap,r->heap.heap,r->journal.layout.buffer,r->journal.layout.buffer_bytes);if(z)return z;
 z=eks_resource(&r->before_resource);if(z)return z;z=eks_resource(&r->target_resource);if(z)return z;
 z=eks_logical(r);if(z)return z;
 if(R32(b->model,0xac)!=r->target.resources.model_wrapper||
  R32(r->target.resources.model_wrapper,0)!=r->target.resources.model)return EKS_CHANGED;
 return _gz_epj_validate(&r->journal);
}
static void eks_zero(void*p,u32 n){u32 i;for(i=0;i<n;i++)((volatile u8*)p)[i]=0;}
static void eks_clear(void){eks_zero(&_gz_eks_plan,sizeof(_gz_eks_plan));eks_zero(&_gz_eks_run,sizeof(_gz_eks_run));_gz_eks_seal=_gz_eks_run_seal=0;}
u32 _gz_eks_service(u32 token){EksPlan*p=&_gz_eks_plan;EksRun*run=&_gz_eks_run;
 u32 frame=_gz_pbac_frame,s,n,c,b,count,w,birth,i,j,k,a,z=0,pf,old_index;EpjBounds bounds;EpjLayout layout;EptResources resources;PoEntry process;
 if(_gz_eks_entered||p->valid||run->phase||_gz_eks_seal||_gz_eks_run_seal)return EKS_CONTEXT;
 if(!_gz_ern_scope(token,frame))return EKS_CONTEXT;
 _gz_eks_entered=1;s=_gz_hbp_recipe.source;n=_gz_hbp_recipe.used;
 if(!eks_external(s,0x200000u)||n<232||n>0x200000u||R32(s,8)!=27||R32(s,12)!=27){z=EKS_SOURCE;goto done;}
 c=R32(s,88);b=R32(s,96);
 if(c<232||c>n||4>n-c||R32(s,92)!=4||b!=c+4||b>n){z=EKS_SOURCE;goto done;}
 count=R32(s+c,0);if(count>EKS_CAP||R32(s,100)!=count*10240||count*10240>n-b){z=EKS_SOURCE;goto done;}
 z=_gz_bcb_archive_validate(s,n,1);if(z)goto done;
 w=s+R32(s,168);birth=s+R32(s,192);
 p->magic=0x454b5331u;p->bytes=72+count*sizeof(EksRow);p->version=1;p->token=token;p->frame=frame;
 p->count=count;p->source=s;p->used=n;p->pin=_gz_hbp_recipe.pin;p->bank=s+b;p->bank_bytes=count*10240;
 p->bank_hash=btr_hash((const void*)p->bank,p->bank_bytes);p->world=w;p->world_hash=R32(w,72);p->birth=birth;p->birth_hash=R32(birth,56);
 for(i=0;i<count;i++){
  EksRow*r=p->rows+i;pf=p->bank+i*10240;_gz_eks_report[6]=i;
  if(R32(pf,52)!=215||R32(pf,4800)!=1){z=ERN_FAMILY;goto done;}
  z=eks_find(R32(pf,0),&a,&process);if(z)goto done;
  for(j=0;j<R32(w,16);j++)if(R32(w+80+j*48,4)==process.pid)break;
  if(j>=R32(w,16)||R32(w+80+j*48,0)!=a||R32(w+80+j*48,8)!=215){z=EKS_PROCESS;goto done;}
  k=birth+96+j*192;
  if(R32(k,0)!=j||R32(k,4)!=1||R32(k,12)!=process.generation||!btr_equal((const void*)(k+16),&process,sizeof(process))){z=EKS_PROCESS;goto done;}
  for(k=0;k<i;k++)if(p->rows[k].pid==process.pid||p->rows[k].actor==a){z=EKS_PROCESS;goto done;}
  r->pid=process.pid;r->actor=a;r->index=j;r->generation=process.generation;r->source_row=pf;r->source_hash=btr_hash((const void*)pf,10240);btr_copy(&r->process,&process,sizeof(process));
  bounds.actor=a;bounds.pid=process.pid;bounds.actor_bytes=0x10bc;bounds.heap=R32(a,0xf4);bounds.morph=R32(a,0x3d0);bounds.face=R32(a,0x4e4);
  z=eks_heap(0,bounds.heap,bounds.morph,0xc8);if(z)goto done;btr_copy(&r->heap,(const void*)&_gz_gh_identity,sizeof(r->heap));
  bounds.head=r->heap.head;bounds.tail=r->heap.tail;bounds.end=r->heap.start+r->heap.bytes;
  if(bounds.end<r->heap.start||!eks_external(bounds.heap,bounds.end-bounds.heap)){z=EKS_HEAP;goto done;}
  bounds.model=R32(bounds.morph,0x90);z=eks_heap(&r->heap,bounds.heap,bounds.model,0xb0);if(z)goto done;
  z=eks_heap(&r->heap,bounds.heap,bounds.face,0x74);if(z)goto done;
  bounds.before_wrapper=R32(bounds.face,16);bounds.source=s;bounds.source_bytes=0x200000u;
  z=_gz_ern_resolve(token,215,R32(pf,4816),ERN_PATTERN);if(z)goto done;btr_copy(&r->target_resource,&_gz_ern_resolved,sizeof(ErnResource));
  bounds.target_wrapper=r->target_resource.wrapper;old_index=0xffffffffu;
  for(k=0;k<r->target_resource.count;k++)if(R32(r->target_resource.table,k*4)==bounds.before_wrapper){if(old_index!=0xffffffffu){z=EKS_ARCHIVE;goto done;}old_index=k;}
  if(old_index==0xffffffffu){z=EKS_ARCHIVE;goto done;}
  z=_gz_ern_resolve(token,215,old_index,ERN_PATTERN);if(z)goto done;btr_copy(&r->before_resource,&_gz_ern_resolved,sizeof(ErnResource));
  if(r->before_resource.table!=r->target_resource.table||r->before_resource.header!=r->target_resource.header){z=EKS_ARCHIVE;goto done;}
  z=_gz_epj_describe(&bounds,&layout);if(z)goto done;
  z=eks_heap(&r->heap,bounds.heap,layout.buffer,layout.buffer_bytes);if(z)goto done;
  resources.header=r->target_resource.header;resources.bytes=r->target_resource.header_bytes;resources.model_wrapper=R32(bounds.model,0xac);
  if(!eks_external(resources.model_wrapper,4)){z=EKS_ARCHIVE;goto done;}
  resources.model=R32(resources.model_wrapper,0);resources.table=r->target_resource.table;resources.count=r->target_resource.count;
  z=_gz_ept_contract(&bounds,&resources,&r->target);if(z)goto done;
  z=_gz_epj_prepare(&bounds,&r->journal);if(z)goto done;
  z=eks_binding(r);if(z)goto done;z=_gz_epj_before_matches(&r->journal);if(z)goto done;p->prepared++;
 }
 /* All journals and shared wrapper before-images exist before publication.
  * A second pass rejects a changed earlier actor or shared binding. */
 for(i=0;i<count;i++){z=eks_binding(p->rows+i);if(z)goto done;z=_gz_epj_before_matches(&p->rows[i].journal);if(z)goto done;}
 if(!_gz_ern_scope(token,frame)||btr_hash((const void*)p->bank,p->bank_bytes)!=p->bank_hash){z=EKS_CHANGED;goto done;}
 p->valid=1;_gz_eks_seal=btr_hash(p,p->bytes);run->phase=1;run->token=token;run->prepared_frame=frame;eks_run_seal();
done:
 _gz_eks_report[2]=token;_gz_eks_report[3]=z?0:1;_gz_eks_report[4]=z;_gz_eks_report[5]=frame;
 _gz_eks_report[7]=p->count;_gz_eks_report[8]=p->prepared;_gz_eks_report[9]=_gz_eks_seal;
 if(z)eks_clear();_gz_eks_entered=0;return z;
}
/* No SDK writer is enabled in this stage. Recovery must nevertheless prove
 * that the prepared current actor/resource state is still its before-image.
 * No original saved packet or source-body pointer is consulted here. */
static u32 eks_empty(void){return !_gz_eks_plan.valid&&!_gz_eks_run.phase&&!_gz_eks_seal&&!_gz_eks_run_seal;}
u32 _gz_eks_recovery_rollback(u32 token){u32 frame=_gz_pbac_frame,z,i;
 if(_gz_eks_entered||!token||!bor_original()||_gz_boh_recovery.phase!=BOR_UNDO||_gz_boh_recovery.token!=token)return EKS_CONTEXT;
 z=_gz_roh_recovery_common(token,frame);if(z)return z;
 if(eks_empty()){z=_gz_ekr_recovery_undo(token);if(z)return z;z=_gz_kbr_recovery_undo(token);return z?z:_gz_kbs_recovery_rollback(token);}
 if(!eks_sealed()||_gz_eks_plan.token!=token||_gz_eks_run.phase!=1)return EKS_CHANGED;
 z=_gz_ekr_recovery_undo(token);if(z)return z;
 z=_gz_kbr_recovery_undo(token);if(z)return z;
 z=_gz_kbs_recovery_rollback(token);if(z)return z;
 _gz_eks_entered=1;
 for(i=0;i<_gz_eks_plan.count;i++){z=eks_binding(_gz_eks_plan.rows+i);if(z)goto done;z=_gz_epj_before_matches(&_gz_eks_plan.rows[i].journal);if(z)goto done;}
 z=_gz_roh_recovery_common(token,frame);
 if(!z){_gz_eks_run.phase=5;_gz_eks_run.rolled_frame=frame;eks_run_seal();}
done:_gz_eks_report[10]=z;_gz_eks_entered=0;return z;
}
u32 _gz_eks_recovery_check(u32 token,u32 frame){u32 z,i;
 if(_gz_eks_entered||!token||frame!=_gz_pbac_frame||!bor_original()||_gz_boh_recovery.phase!=BOR_DONE||_gz_boh_recovery.token!=token)return EKS_CONTEXT;
 z=_gz_roh_recovery_common(token,frame);if(z)return z;
 if(eks_empty()){z=_gz_ekr_recovery_check(token,frame);if(z)return z;z=_gz_kbr_recovery_check(token,frame);return z?z:_gz_kbs_recovery_check(token,frame);}
 if(!eks_sealed()||_gz_eks_plan.token!=token||_gz_eks_run.phase!=5||_gz_eks_run.error)return EKS_CHANGED;
 for(i=0;i<_gz_eks_plan.count;i++){z=eks_binding(_gz_eks_plan.rows+i);if(z)return z;z=_gz_epj_before_matches(&_gz_eks_plan.rows[i].journal);if(z)return z;}
 z=_gz_ekr_recovery_check(token,frame);if(z)return z;
 z=_gz_kbr_recovery_check(token,frame);if(z)return z;
 z=_gz_kbs_recovery_check(token,frame);return z?z:_gz_roh_recovery_common(token,frame);
}
