/* One Rito's recorded internal domains, ordered and reversible. An outer
 * world writer must retain real process/heap/archive/source/execution holds
 * and all unimplemented external participants. This leaf grants no native
 * replay capability or ordinary restore admission. The run is separate from
 * the immutable current-attempt row; recovery never reads historical source.
 */
enum {RTX_BAD=1470,RTX_CHANGED=1471,RTX_ALIAS=1472,RTX_PHASE=1473};
enum {RTX_PREPARED=1,RTX_WRITING=2,RTX_APPLIED=3,RTX_ROLLED=4,RTX_FAILED=5};
typedef struct {u32 magic,bytes,valid,phase,frame,row,row_seal,attempted,completed,error,undo_error,source_frame,reserved[3],seal;} RtxRun;
typedef char RtxRunSize[sizeof(RtxRun)==64?1:-1];
static u32 rtx_hash(const RtxRun*r){u32 i,h=2166136261u;const u8*q=(const u8*)r;for(i=0;i<60;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;return h;}
static void rtx_seal(RtxRun*r){r->seal=rtx_hash(r);__asm__ volatile("sync":::"memory");}
static u32 rtx_plan(const RtvRow*p){u32 i,j,z,frame,source;const KbjPlan*body;const KbjPlan*known[4];KbtPlan binding;
 if(!p||!rtv_row_member((u32)p)||!epj_ram((u32)p,sizeof(*p))||p->seal!=rtv_hash(p)||p->roles!=4||!p->source_frame||p->index>=16||
  !p->pid||p->pid!=p->process.pid||!rtf_profile(p->process.profile))return RTX_BAD;
 body=&p->controllers[0].body;frame=p->core.frame;source=p->source_frame;
 if(!frame||!_gz_nsg_retained_frame(frame)||!rbs_storage(&body->layout,(u32)p,sizeof(*p)))return RTX_CHANGED;
 for(i=0;i<4;i++){
  const RtvRole*q=p->controllers+i;u32 target=(u32)q->numeric.target;
  known[i]=&q->body;z=_gz_rbp_validate(&q->body,&q->numeric);if(z)return z;z=_gz_rrp_validate(&q->body,&q->render,&q->numeric,&q->result);if(z)return z;
  z=_gz_kbt_contract(&q->body,&binding);if(z)return z;if(!btr_equal(&binding,&q->binding,sizeof(binding)))return RTX_CHANGED;
  if(q->numeric.role!=i+1||q->numeric.frame!=frame||R32(target,24)!=source||!btr_equal(&q->numeric.process,&p->process,sizeof(PoEntry))||
   R32(target,28)!=p->process.profile||!rri_identity(&p->process,target))return RTX_CHANGED;
  if(R32(target,32)!=R32((u32)p->core.target,32)||R32(target,36)!=R32((u32)p->core.target,36)||
   R32(target,80)!=R32((u32)p->core.target,68)||R32(target,84)!=R32((u32)p->core.target,72)||R32(target,88)!=R32((u32)p->core.target,76))return RTX_CHANGED;
  if(R32((u32)p->models.target+128+i*RMS_ROW,8)!=R32(target,52)||R32((u32)p->models.target+128+i*RMS_ROW,12)!=R32(target,44))return RTX_CHANGED;
  /* Shared selected wrapper frames within this actor must agree. The outer
   * group still has to reconcile sharing with other actors/resources. */
  for(j=0;j<i;j++){
   const RtvRole*other=p->controllers+j;
   if(q->body.layout.target_body==other->body.layout.target_body&&R32(target,104)!=R32((u32)other->numeric.target,104))return RTX_CHANGED;
   if(q->body.layout.before_body==other->body.layout.before_body&&q->body.wrapper_frames[0]!=other->body.wrapper_frames[0])return RTX_CHANGED;
  }
 }
 z=_gz_rbc_validate(body,&p->core);if(z)return z;z=_gz_rmj_validate_group(body,&p->models,known);if(z)return z;z=_gz_rtj_validate_group(body,&p->texture,known);if(z)return z;z=_gz_rpj_validate(body,&p->physics);if(z)return z;
 if(p->models.frame!=frame||p->texture.frame!=frame||p->physics.frame!=frame||!btr_equal(&p->physics.process,&p->process,sizeof(PoEntry))||R32((u32)p->physics.target,24)!=source||!btr_equal(&p->core.process,&p->process,sizeof(PoEntry))||
  !btr_equal(&p->models.process,&p->process,sizeof(PoEntry))||!btr_equal(&p->texture.process,&p->process,sizeof(PoEntry))||
  R32((u32)p->core.target,24)!=source||R32((u32)p->models.target,24)!=source||R32((u32)p->texture.target,24)!=source||
  R32((u32)p->texture.target,32)!=R32((u32)p->controllers[1].numeric.target,52)||
  R32((u32)p->texture.target,108)!=rbc_value((u32)p->core.target,0x87c,1)||
  R32((u32)p->texture.target,116)!=rbc_value((u32)p->core.target,0x87e,2)||
  R32((u32)p->texture.target,112)!=rbc_value((u32)p->core.target,0xa6d,1)||
  R32((u32)p->physics.target,32)!=R32((u32)p->core.target,32)||R32((u32)p->physics.target,36)!=R32((u32)p->core.target,36)||
  R32((u32)p->physics.target,40)!=R32((u32)p->core.target,68)||R32((u32)p->physics.target,44)!=R32((u32)p->core.target,72)||
  R32((u32)p->physics.target,48)!=R32((u32)p->core.target,76))return RTX_CHANGED;
 return 0;
}
static u32 rtx_before(const RtvRow*p){u32 i,z;for(i=0;i<4;i++){
 z=_gz_kbj_before_matches(&p->controllers[i].body);if(z)return z;
 z=_gz_krj_before_matches(&p->controllers[i].body,&p->controllers[i].render);if(z)return z;}
 z=_gz_rbc_before_matches(&p->controllers[0].body,&p->core);if(z)return z;
 z=_gz_rmj_before_matches(&p->controllers[0].body,&p->models);if(z)return z;z=_gz_rtj_before_matches(&p->controllers[0].body,&p->texture);return z?z:_gz_rpj_before_matches(&p->controllers[0].body,&p->physics);
}
static u32 rtx_target(const RtvRow*p){u32 i,z;for(i=0;i<4;i++){
 const RtvRole*q=p->controllers+i;z=_gz_rbp_matches(&q->body,&q->numeric);if(z)return z;z=_gz_rrp_matches(&q->body,&q->render,&q->numeric,&q->result);if(z)return z;}
 z=_gz_rmj_matches(&p->controllers[0].body,&p->models);if(z)return z;z=_gz_rtj_matches(&p->controllers[0].body,&p->texture);if(z)return z;z=_gz_rbc_matches(&p->controllers[0].body,&p->core);return z?z:_gz_rpj_matches(&p->controllers[0].body,&p->physics);
}
static u32 rtx_validate(const RtvRow*p,const RtxRun*r){u32 z;
 if(!rtv_row_member((u32)p)||!r||(u32)r!=rtv_run_address(rtv_header(),p->index)||!epj_ram((u32)r,sizeof(*r))||r->magic!=0x52585431u||r->bytes!=64||r->valid!=1||r->seal!=rtx_hash(r)||
  r->row!=(u32)p||r->phase<1||r->phase>5||r->attempted>16||r->completed>r->attempted||r->reserved[0]!=ncb_directory_generation()||r->reserved[1]!=ncb_region_generation(NCR_RTV)||r->reserved[2]!=rtv_row_capacity(rtv_header()))return RTX_BAD;
 z=rtx_plan(p);if(z)return z;
 if(r->row_seal!=p->seal||r->frame!=p->core.frame||r->source_frame!=p->source_frame||
  !rbs_output(&p->controllers[0].body,(u32)r,sizeof(*r))||!epj_apart((u32)r,sizeof(*r),(u32)p,sizeof(*p)))return RTX_ALIAS;
 return 0;
}
u32 _gz_rtx_prepare(const RtvRow*p,RtxRun*out){u32 z=rtx_plan(p);if(z)return z;z=rtx_before(p);if(z)return z;
 if(!out||(u32)out!=rtv_run_address(rtv_header(),p->index)||!rbs_output(&p->controllers[0].body,(u32)out,sizeof(*out))||!epj_apart((u32)out,sizeof(*out),(u32)p,sizeof(*p)))return RTX_ALIAS;
 eks_zero(out,sizeof(*out));out->magic=0x52585431u;out->bytes=64;out->phase=RTX_PREPARED;out->frame=p->core.frame;
 out->row=(u32)p;out->row_seal=p->seal;out->source_frame=p->source_frame;out->reserved[0]=ncb_directory_generation();out->reserved[1]=ncb_region_generation(NCR_RTV);out->reserved[2]=rtv_row_capacity(rtv_header());rtx_seal(out);out->valid=1;return 0;
}
/* Same proven original SDK contract, without borrowing the Keese writer's
 * run ledger or failure seams. The enclosing RtxRun is already attempted. */
static u32 rtx_role_before(const RtvRow*row,u32 role,u32 completed){const RtvRole*p=row->controllers+role;const KbjPlan*j=&p->body;const KbjLayout*l=&j->layout;u32 i,k,wrapper,expected;
 if(!btr_equal((const void*)l->common.bounds.morph,j->object_before,0xc8)||!btr_equal((const void*)l->buffer,j->buffer_before,l->buffer_bytes)||
  !btr_equal((const void*)l->transforms,j->transforms_before,l->bones*32)||!btr_equal((const void*)l->quaternions,j->quaternions_before,l->bones*16))return RTX_CHANGED;
 for(k=0;k<2;k++){
  wrapper=k?l->target_body:l->before_body;expected=j->wrapper_frames[k];
  for(i=0;i<role;i++)if(completed>=i*3+2&&row->controllers[i].body.layout.target_body==wrapper)expected=R32((u32)row->controllers[i].numeric.target,104);
  if(wrapper&&R32(wrapper,0)!=expected)return RTX_CHANGED;
 }return 0;
}
static u32 rtx_bind(const RtvRow*row,u32 role,u32 completed){const RtvRole*p=row->controllers+role;KbtPlan current;u32 z,m=p->body.layout.common.bounds.morph;
 z=_gz_kbt_contract(&p->body,&current);if(z)return z;if(!btr_equal(&current,&p->binding,sizeof(current)))return RTX_CHANGED;
 z=rtx_role_before(row,role,completed);if(z)return z;
 *(volatile u32*)(m+0x70)=p->body.layout.target_body;*(volatile u32*)(m+0x94)=p->body.layout.target_body;
 _gz_kbt_native_set_resource(m+4,current.animations[1].data);
 if(_gz_kbt_native_bind_names(m+4,current.layout.skeleton)!=kbt_bind_result(&current))return KBT_NATIVE;
 if(R32(m,0x48)!=current.animations[1].data||R32(m,0x4c)!=current.animations[1].rows||R16(m,0x44)!=current.animations[1].bones||
  R32(m,0x28)!=current.animations[1].curves)return KBT_NATIVE;return _gz_kbj_validate(&p->body);
}
u32 _gz_rtx_apply(const RtvRow*p,RtxRun*r){u32 z=rtx_validate(p,r),step,role,kind;if(z)return z;
 if(r->phase!=RTX_PREPARED||r->attempted||r->completed||r->error||r->undo_error)return RTX_PHASE;
 z=rtx_before(p);if(z)return z;r->phase=RTX_WRITING;
 for(step=1;step<=16;step++){
  r->attempted=step;rtx_seal(r);
  if(step<=12){const RtvRole*q;role=(step-1)/3;kind=(step-1)%3;q=p->controllers+role;
   z=kind==0?rtx_bind(p,role,r->completed):kind==1?_gz_rbp_apply(&q->body,&q->numeric):_gz_rrp_apply(&q->body,&q->render,&q->numeric,&q->result);
  }else if(step==13)z=_gz_rmj_apply(&p->controllers[0].body,&p->models);
  else if(step==14)z=_gz_rtj_apply(&p->controllers[0].body,&p->texture);
  else if(step==15)z=_gz_rbc_apply(&p->controllers[0].body,&p->core);
  else z=_gz_rpj_apply(&p->controllers[0].body,&p->physics);
  if(z)goto failed;r->completed=step;rtx_seal(r);
 }
 z=rtx_target(p);if(z)goto failed;r->phase=RTX_APPLIED;rtx_seal(r);return 0;
failed:r->error=z;r->phase=RTX_FAILED;rtx_seal(r);return z;
}
u32 _gz_rtx_undo(const RtvRow*p,RtxRun*r){const KbjPlan*known[4];u32 z=rtx_validate(p,r),i;if(z)return z;for(i=0;i<4;i++)known[i]=&p->controllers[i].body;
 if(r->undo_error)return r->undo_error;if(r->phase==RTX_ROLLED)return rtx_before(p);
 if(r->attempted>=16){z=_gz_rpj_undo(&p->controllers[0].body,&p->physics);if(z)goto failed;}
 if(r->attempted>=15){z=_gz_rbc_undo(&p->controllers[0].body,&p->core);if(z)goto failed;}
 if(r->attempted>=14){z=_gz_rtj_undo_group(&p->controllers[0].body,&p->texture,known);if(z)goto failed;}
 if(r->attempted>=13){z=_gz_rmj_undo_group(&p->controllers[0].body,&p->models,known);if(z)goto failed;}
 for(i=4;i>0;i--)if(r->attempted>=(i-1)*3+1){const RtvRole*q=p->controllers+i-1;
  z=_gz_krj_undo(&q->body,&q->render);if(z)goto failed;z=_gz_kbj_undo(&q->body);if(z)goto failed;}
 z=rtx_before(p);if(z)goto failed;r->phase=RTX_ROLLED;rtx_seal(r);return 0;
failed:r->undo_error=z;rtx_seal(r);return z;
}
u32 _gz_rtx_matches(const RtvRow*p,const RtxRun*r){u32 z=rtx_validate(p,r);return z?z:r->phase==RTX_APPLIED?rtx_target(p):RTX_PHASE;}
u32 _gz_rtx_before_matches(const RtvRow*p,const RtxRun*r){u32 z=rtx_validate(p,r);return z?z:rtx_before(p);}
