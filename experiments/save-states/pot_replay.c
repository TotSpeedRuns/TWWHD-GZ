/* Full PF2 pot-bank participant for surviving native allocations. Capture,
 * source, birth and live process/model/resource bindings precede every write.
 * Local dependencies remain explicit: active collision/particle/carry/BRK or
 * external-child relationships need their matching participants, not omission.
 * Normal whole-world publication remains disabled. */
#include "family_state.h"
#include "heap_owner.h"
extern u32 _gz_pfb_archive_validate(u32,u32,u32);
extern u32 _gz_bcb_archive_validate(u32,u32,u32);
extern void _gz_pn_setmatrix(u32);
extern volatile u32 _gz_ppt_native_spec[3],_gz_ppt_offset_init;
extern volatile u8 _gz_ppt_offset[12];
extern volatile GhIdentity _gz_gh_identity;
extern u32 _gz_boh_recovery_globals_authority(u32,u32);

enum {PPT_CAP=64,PPT_PREPARED=1,PPT_APPLYING=2,PPT_APPLIED=3,PPT_UNDOING=4,PPT_ROLLED=5};
typedef struct {
 u32 pid,actor,index,generation;
 PoEntry process;GhIdentity heap_binding;
 u32 heap,model,model_data,source_row,source_hash,before_hash;
 u8 model_before[60],before[2048],saved[2048];
} PptRow;
typedef char PptRowSize[sizeof(PptRow)==4308?1:-1];
typedef struct {
 u32 magic,bytes,version,valid,token,frame,count,source,used,pin;
 u32 source_bank,bank_bytes,bank_hash,world,world_hash,birth,birth_hash;
 u32 po_serial_audit,prepared_count,reserved0,reserved1,reserved2;
 u8 matrix_before[48],offset_before[12];u32 offset_init_before,spec_audit[3];
 PptRow rows[PPT_CAP];
} PptPlan;
typedef char PptPlanSize[sizeof(PptPlan)==275876?1:-1];
typedef struct {u32 magic,bytes,valid,phase,token,applied_frame,rolled_frame,applied_count,undo_count,global_undo_done,error,plan_seal;} PptRun;
static PptPlan _gz_ppt_plan;
static PptRun _gz_ppt_run;
static u32 _gz_ppt_plan_seal,_gz_ppt_run_seal,_gz_ppt_entered;
u32 _gz_ppt_report[64]={0x50505431u,256};
static void ppt_seal_run(void){_gz_ppt_run_seal=btr_hash(&_gz_ppt_run,sizeof(PptRun));}
static u32 ppt_progress(void){const PptPlan*p=&_gz_ppt_plan;const PptRun*r=&_gz_ppt_run;
 return p->magic==0x50505431u&&p->version==1&&p->valid==1&&p->count<=PPT_CAP&&
  p->bytes==164+p->count*sizeof(PptRow)&&p->prepared_count==p->count&&
  r->magic==0x50505231u&&r->bytes==sizeof(*r)&&r->valid==1&&r->token==p->token&&r->token&&
  r->phase>=PPT_PREPARED&&r->phase<=PPT_ROLLED&&r->applied_count<=p->count&&
  r->undo_count<=r->applied_count&&r->global_undo_done<=1&&
  r->plan_seal==_gz_ppt_plan_seal&&_gz_ppt_run_seal==btr_hash(r,sizeof(*r));
}
static u32 ppt_sealed(void){return ppt_progress()&&_gz_ppt_plan_seal==btr_hash(&_gz_ppt_plan,_gz_ppt_plan.bytes);}
static u32 ppt_find(u32 pid,u32*out){u32 q=_gz_pba_native_actor_queue,left=4096,a,found=0;
 *out=0;while(q){if(!left--||!span(q,16))return 27;a=R32(q,12);
  if(a){if(!span(a,16))return 27;if(R32(a,4)==pid){if(found||R16(a,8)!=453||R8(a,12)!=2)return 27;found=a;}}
  q=R32(q,8);
 }
 if(!found)return 27;*out=found;return 0;
}
/* Fresh current queue and process generation precede actor and heap reads.
 * Stored source, actor/model/heap audits are never adopted as current roots. */
static u32 ppt_process(u32 pid,u32*out,PoEntry*process){u32 a,z;const PoEntry*p;
 z=ppt_find(pid,&a);if(z)return z;
 if(_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return 27;
 p=&_gz_po_observation.entry;
 if(p->object!=a||p->pid!=pid||p->state!=PO_LIVE||!p->generation||p->flags||p->end_frame||
  p->process_name!=453||p->profile>=484||p->allocation_bytes<2364||p->allocation_bytes>0x100000u||
  p->base_bytes+p->other_bytes<p->base_bytes||p->base_bytes+p->other_bytes!=p->allocation_bytes||
  !span(a,p->allocation_bytes)||R16(a,14)!=p->profile||R32(a,16)!=p->profile_entry)return 27;
 btr_copy(process,p,sizeof(*process));*out=a;return 0;
}
static u32 ppt_binding(const PptRow*r,u32 frame){PoEntry p;u32 a,z;
 z=ppt_process(r->pid,&a,&p);if(z)return z;
 if(a!=r->actor||p.generation!=r->generation||!btr_equal(&p,&r->process,sizeof(p))||
  R32(a,0xf4)!=r->heap||R32(a,0x3b4)!=r->model)return 27;
 z=_gz_pf_preflight(a,(u32)r->before,r->pid,frame);if(z)return z;
 if(!btr_equal((const void*)&_gz_gh_identity,&r->heap_binding,sizeof(GhIdentity))||
  R32(r->model,0xac)!=r->model_data)return 226;
 return 0;
}
/* The native setter's special child-model loop reads these three globals
 * only for spec1..3. Its external actors require their own matrix journal.
 * Empty current slots permit the original loop without following a pointer. */
static u32 ppt_matrix_scope(const PptRow*r){u32 spec=(R32(r->saved,36)>>8)&63;
 if(spec>=1&&spec<=3&&(_gz_ppt_native_spec[0]||_gz_ppt_native_spec[1]||_gz_ppt_native_spec[2]))return 227;
 return 0;
}
static u32 ppt_match(const PptRow*r,const u8*packet,u32 model_before){u32 i,p=128,w,o,n,a=r->actor;
 for(i=0;i<_gz_pf_field_count();i++){w=_gz_pf_field_word(i);o=w>>16;n=w&65535;
  if(!n||p+n>1152||!btr_equal((const void*)(a+o),packet+p,n))return 226;p+=n;
 }
 if(p-128!=_gz_pf_packed_bytes()||R32(a,0x794)!=R32(packet,32))return 226;
 for(i=0;i<10;i++){
  static const u16 refs[10]={0x77c,0x780,0x784,0x788,0x8bc,0x8d8,0x8f4,0x680,0x6c4,0x70c};
  if(R32(a,refs[i])!=R32(packet,1152+i*4))return 226;
 }
 for(i=0;i<7;i++){
  u32 c=(u32)packet+1248+i*28,k=a+0x828+i*20;
  if(R32(k,0)!=R32(c,0)||R32(k,4)!=R32(c,4)||
   R32(k,8)!=(R32(c,8)?a+R32(c,12):0)||R32(k,12)!=(R32(c,16)?a+R32(c,20):0)||
   R32(k,16)!=R32(c,24))return 226;
 }
 if(R32(a,0x368)!=(R32(packet,96)?r->model:0))return 226;
 if(model_before&&!btr_equal((const void*)(r->model+0xbc),r->model_before,60))return 226;
 return 0;
}
/* Native SetMtx writes the certified primary model's60 cache bytes, type2's
 * named m700 vector, the shared matrix and its lazy offset cache. Snapshot
 * and restore shared globals around each call; no caller-selected target or
 * source model pointer is passed to the native method. */
static void ppt_set_matrix(const PptRow*r){u8 matrix[48],offset[12];u32 init;
 btr_copy(matrix,_gz_btr_shared_matrix,48);btr_copy(offset,(const void*)_gz_ppt_offset,12);init=_gz_ppt_offset_init;
 _gz_pn_setmatrix(r->actor);
 btr_copy(_gz_btr_shared_matrix,matrix,48);btr_copy((void*)_gz_ppt_offset,offset,12);_gz_ppt_offset_init=init;
}
u32 _gz_ppt_prepare(u32 token,u32 frame){PptPlan*p=&_gz_ppt_plan;PptRun*r=&_gz_ppt_run;
 u32 s,n,b,w,birth,count,i,k,a,z,pf,idx,br;PoEntry process;
 if(_gz_ppt_entered||p->valid||r->phase||_gz_ppt_plan_seal||_gz_ppt_run_seal)return 28;
 if(!token||_gz_roh_gl_context(token,frame,1)!=1||!_gz_hbp_read(token,frame))return 42;
 _gz_ppt_entered=1;s=_gz_hbp_recipe.source;n=_gz_hbp_recipe.used;
 z=_gz_pfb_archive_validate(s,n,1);if(z)goto done;
 z=_gz_bcb_archive_validate(s,n,1);if(z)goto done;
 b=s+R32(s,184);w=s+R32(s,168);birth=s+R32(s,192);count=R32(b,16);
 if(count>PPT_CAP){z=28;goto done;}
 r->phase=PPT_PREPARED;p->magic=0x50505431u;p->bytes=164+count*sizeof(PptRow);p->version=1;
 p->token=token;p->frame=frame;p->count=count;p->source=s;p->used=n;p->pin=_gz_hbp_recipe.pin;
 p->source_bank=b;p->bank_bytes=R32(s,188);p->bank_hash=btr_hash((const void*)b,p->bank_bytes);
 p->world=w;p->world_hash=R32(w,72);p->birth=birth;p->birth_hash=R32(birth,56);p->po_serial_audit=_gz_po_header.serial;
 btr_copy(p->matrix_before,_gz_btr_shared_matrix,48);btr_copy(p->offset_before,(const void*)_gz_ppt_offset,12);
 p->offset_init_before=_gz_ppt_offset_init;for(i=0;i<3;i++)p->spec_audit[i]=_gz_ppt_native_spec[i];
 for(i=0;i<count;i++){
  PptRow*row=&p->rows[i];pf=b+64+i*2048;
  if(_gz_pf_packet_check(pf)||R32(pf,68)){z=227;goto done;}
  z=ppt_process(R32(pf,20),&a,&process);if(z)goto done;
  for(idx=0;idx<R32(w,16);idx++)if(R32(w+80+idx*48,4)==process.pid)break;
  if(idx==R32(w,16)||R32(w+80+idx*48,0)!=a||R32(w+80+idx*48,8)!=453){z=27;goto done;}
  br=birth+96+idx*192;
  if(R32(br,0)!=idx||R32(br,4)!=1||R32(br,12)!=process.generation||!btr_equal((const void*)(br+16),&process,sizeof(process))){z=27;goto done;}
  for(k=0;k<i;k++)if(p->rows[k].pid==process.pid){z=27;goto done;}
  row->pid=process.pid;row->actor=a;row->index=idx;row->generation=process.generation;btr_copy(&row->process,&process,sizeof(process));
  row->source_row=pf;btr_copy(row->saved,(const void*)pf,2048);row->source_hash=btr_hash(row->saved,2048);
  z=_gz_pf_capture(a,(u32)row->before,row->pid,frame);if(z)goto done;
  if(R32(row->before,68)){z=227;goto done;}
  z=_gz_pf_preflight(a,(u32)row->before,row->pid,frame);if(z)goto done;
  row->heap=R32(a,0xf4);row->model=R32(a,0x3b4);row->model_data=R32(row->model,0xac);
  for(k=0;k<i;k++)if(p->rows[k].heap==row->heap||p->rows[k].model==row->model){z=226;goto done;}
  btr_copy(&row->heap_binding,(const void*)&_gz_gh_identity,sizeof(GhIdentity));
  row->before_hash=btr_hash(row->before,2048);btr_copy(row->model_before,(const void*)(row->model+0xbc),60);
  z=_gz_pf_preflight(a,(u32)row->saved,row->pid,frame);if(z)goto done;
  z=ppt_matrix_scope(row);if(z)goto done;z=ppt_binding(row,frame);if(z)goto done;
  z=ppt_match(row,row->before,1);if(z)goto done;p->prepared_count++;
 }
 if(_gz_roh_gl_context(token,frame,1)!=1||!_gz_hbp_read(token,frame)||
  btr_hash((const void*)b,p->bank_bytes)!=p->bank_hash){z=42;goto done;}
 p->valid=1;_gz_ppt_plan_seal=btr_hash(p,p->bytes);
 r->magic=0x50505231u;r->bytes=sizeof(*r);r->valid=1;r->token=token;r->plan_seal=_gz_ppt_plan_seal;ppt_seal_run();
 _gz_ppt_report[2]=token;_gz_ppt_report[3]=PPT_PREPARED;_gz_ppt_report[5]=frame;_gz_ppt_report[8]=count;
 _gz_ppt_report[9]=p->bytes;_gz_ppt_report[10]=_gz_ppt_plan_seal;_gz_ppt_report[11]=b;
done:
 _gz_ppt_report[4]=z;_gz_ppt_entered=0;return z;
}
static u32 ppt_apply_gate(u32 token,u32 frame){const PptPlan*p=&_gz_ppt_plan;
 if(!ppt_sealed()||p->token!=token||_gz_roh_owner[12]!=8||!_gz_bmh_scope(token,frame)||
  _gz_roh_replay_common(token,frame,1)||_gz_boh_service_validate(token)||!_gz_hbp_service_read(token,frame))return 42;
 if(p->source!=_gz_hbp_recipe.source||p->used!=_gz_hbp_recipe.used||p->pin!=_gz_hbp_recipe.pin||
  _gz_pfb_archive_validate(p->source,p->used,1)||btr_hash((const void*)p->source_bank,p->bank_bytes)!=p->bank_hash)return 42;
 return 0;
}
u32 _gz_ppt_apply(u32 token){PptPlan*p=&_gz_ppt_plan;PptRun*r=&_gz_ppt_run;u32 frame=_gz_pbac_frame,z,i;
 if(_gz_ppt_entered||!ppt_sealed()||p->token!=token||r->phase!=PPT_PREPARED)return 28;
 _gz_ppt_entered=1;z=ppt_apply_gate(token,frame);if(z)goto done;
 for(i=0;i<p->count;i++){
  z=ppt_binding(&p->rows[i],frame);if(z)goto done;
  z=ppt_match(&p->rows[i],p->rows[i].before,1);if(z)goto done;
  z=_gz_pf_preflight(p->rows[i].actor,(u32)p->rows[i].saved,p->rows[i].pid,frame);if(z)goto done;
  z=ppt_matrix_scope(&p->rows[i]);if(z)goto done;
 }
 r->phase=PPT_APPLYING;r->applied_frame=frame;ppt_seal_run();
 for(i=0;i<p->count;i++){
  PptRow*row=&p->rows[i];r->applied_count=i+1;ppt_seal_run();
  z=ppt_binding(row,frame);if(z)goto done;
  z=_gz_pf_apply_scalars(row->actor,(u32)row->saved,row->pid,frame);if(z)goto done;
  z=ppt_binding(row,frame);if(z)goto done;z=ppt_matrix_scope(row);if(z)goto done;
  ppt_set_matrix(row);
  z=ppt_binding(row,frame);if(z)goto done;z=ppt_match(row,row->saved,0);if(z)goto done;
 }
 z=ppt_apply_gate(token,frame);if(z)goto done;
 r->phase=PPT_APPLIED;ppt_seal_run();_gz_ppt_report[3]=PPT_APPLIED;_gz_ppt_report[6]=frame;_gz_ppt_report[12]=r->applied_count;
done:
 if(ppt_progress()){r->error=z;ppt_seal_run();}
 _gz_ppt_report[4]=z;_gz_ppt_entered=0;return z;
}
/* This path uses only sealed private preimages and current native roots.
 * No HBP/source/pin reader or original operation deadline is consulted. */
u32 _gz_ppt_recovery_rollback(u32 token){PptPlan*p=&_gz_ppt_plan;PptRun*r=&_gz_ppt_run;u32 frame=_gz_pbac_frame,z,i;
 if(_gz_ppt_entered||!ppt_sealed()||p->token!=token||r->phase==PPT_ROLLED)return 28;
 _gz_ppt_entered=1;
 if(!_gz_boh_recovery_authority(token,_gz_boh_journal.actor,_gz_boh_plan.link,frame)){z=40;goto done;}
 if(r->phase==PPT_PREPARED){
  for(i=0;i<p->count;i++){z=ppt_binding(&p->rows[i],frame);if(z)goto done;z=ppt_match(&p->rows[i],p->rows[i].before,1);if(z)goto done;}
 }else{
  r->phase=PPT_UNDOING;ppt_seal_run();
  while(r->undo_count<r->applied_count){
   PptRow*row=&p->rows[r->applied_count-1-r->undo_count];
   z=ppt_binding(row,frame);if(z)goto done;
   z=_gz_pf_apply_scalars(row->actor,(u32)row->before,row->pid,frame);if(z)goto done;
   btr_copy((void*)(row->model+0xbc),row->model_before,60);
   z=ppt_binding(row,frame);if(z)goto done;z=ppt_match(row,row->before,1);if(z)goto done;
   r->undo_count++;ppt_seal_run();
  }
 }
 if(!_gz_boh_recovery_authority(token,_gz_boh_journal.actor,_gz_boh_plan.link,frame)){z=40;goto done;}
 if(!ppt_sealed()){z=28;goto done;}
 r->phase=PPT_ROLLED;r->rolled_frame=frame;r->error=0;ppt_seal_run();
 _gz_ppt_report[3]=PPT_ROLLED;_gz_ppt_report[7]=frame;_gz_ppt_report[13]=r->undo_count;
done:
 _gz_ppt_report[4]=z;_gz_ppt_entered=0;return z;
}
/* Constructor matrix work predates BTR's own before-image. Restore this
 * pre-constructor shared value only AFTER native retirement and exact actor
 * baseline recovery; earlier restoration would break BTR's undo precondition. */
u32 _gz_ppt_recovery_globals(u32 token){PptPlan*p=&_gz_ppt_plan;PptRun*r=&_gz_ppt_run;u32 frame=_gz_pbac_frame,z,i;
 if(_gz_ppt_entered||!ppt_sealed()||p->token!=token||r->phase!=PPT_ROLLED||r->global_undo_done)return 28;
 _gz_ppt_entered=1;
 if(!_gz_boh_recovery_globals_authority(token,frame)){z=40;goto done;}
 if(!btr_equal((const void*)_gz_ppt_offset,p->offset_before,12)||_gz_ppt_offset_init!=p->offset_init_before){z=226;goto done;}
 for(i=0;i<3;i++)if(_gz_ppt_native_spec[i]!=p->spec_audit[i]){z=226;goto done;}
 btr_copy(_gz_btr_shared_matrix,p->matrix_before,48);
 if(!_gz_boh_recovery_globals_authority(token,frame)||!btr_equal(_gz_btr_shared_matrix,p->matrix_before,48)){z=40;goto done;}
 r->global_undo_done=1;ppt_seal_run();_gz_ppt_report[14]=1;z=0;
done:
 _gz_ppt_report[4]=z;_gz_ppt_entered=0;return z;
}
