/* RMS v2 preserves the exact owned derived-matrix bank as opaque bits.
 * Actual native Bm5 wing inputs contain unused non-finite words while the
 * actor/heap remain stable. This bank has no pointer fields, is never used
 * for addressing, and is copied only to the newly qualified same model's
 * bounded allocation. Position/scale and structural checks remain strict.
 * Historical v1 packets retain their finite-matrix rule. */
/* Pointer-free model transforms/joint matrices for all eight Rito model
 * roles, including explicitly absent accessories. Native pointers are only
 * fresh current topology; none is serialized or assigned from old source.
 * Material/GPU/event/physics ownership and complete world replay are separate.
 */
enum {RMS_BYTES=25728,RMS_ROW=3200,RMS_BAD=1450,RMS_CHANGED=1451,RMS_ALIAS=1452};
typedef struct {u32 model,wrapper,skeleton,header,matrices,bones,index,flags_fixed;} RmsModel;
typedef struct {u32 actor,presence;RmsModel models[8];} RmsLayout;
typedef char RmsLayoutSize[sizeof(RmsLayout)==264?1:-1];
static u32 rms_hash(u32 p){u32 i,h=2166136261u;for(i=0;i<RMS_BYTES;i++)h=(h^((i>=12&&i<16)||(i>=64&&i<68)?0:R8(p,i)))*16777619u;return h;}
static u32 rms_numbers(u32 p,u32 n){u32 i;for(i=0;i<n;i+=4)if(!epj_finite(R32(p,i)))return 0;return 1;}
static u32 rms_packet(u32 p,u32 published){u32 i,q,n,mask=0;
 if(!epj_descriptor(p,RMS_BYTES)||R32(p,0)!=0x524d5331u||R32(p,4)!=RMS_BYTES||(R32(p,8)!=1&&R32(p,8)!=2)||R32(p,12)!=published||
  !R32(p,16)||R32(p,16)>=0xfffffffeu||!R32(p,20)||!R32(p,24)||!rtf_profile(R32(p,28))||R32(p,36)>65535||R32(p,40)>255||R32(p,44)!=8||
  R32(p,48)>255||R32(p,52)>255||R32(p,56)>255||!R32(p,60)||R32(p,60)>4096||R32(p,64)!=rms_hash(p)||
  R32(p,68)!=0x524d4431u||R32(p,72)!=64||!rbs_zero(p,76,128))return RMS_BAD;
 for(i=0;i<8;i++){
  q=p+128+i*RMS_ROW;if(R32(q,0)>1)return RMS_BAD;
  if(!R32(q,0)){if(!rbs_zero(q,0,RMS_ROW))return RMS_BAD;continue;}
  mask|=1u<<i;n=R32(q,12);
  if(R32(q,4)!=i+1||R32(q,8)>=R32(p,60)||!n||n>64||R32(q,16)>65535||!rbs_zero(q,20,64)||
   (!rms_numbers(q+64,60)||(R32(p,8)==1&&!rms_numbers(q+124,n*48)))||!rbs_zero(q,124+n*48,RMS_ROW))return RMS_BAD;
 }
 return (mask&15u)==15u&&mask==R32(p,40)?0:RMS_BAD;
}
u32 _gz_rms_check(u32 p){return rms_packet(p,1);}
static u32 rms_span_apart(const EpjBounds*b,const EptResources*r,u32 p,u32 n){return
 epj_apart(p,n,b->actor,b->actor_bytes)&&epj_apart(p,n,b->source,b->source_bytes)&&
 epj_apart(p,n,r->header,r->bytes)&&epj_apart(p,n,r->table,r->count*4);}
static u32 rms_layout_mode(const KbjLayout*base,RmsLayout*out,const KbjPlan*const*known){
 const EpjBounds*owner=&base->common.bounds;const EptResources*archive=&base->resources;
 EpjBounds b;EptResources r;KbjLayout controllers[4];RmsLayout q;u32 i,j,k,z,p,m,h,s,n,selected;
 if(!epj_descriptor((u32)out,sizeof(*out))||!rtf_profile(R16(owner->actor,8)))return RMS_BAD;
 eks_zero(&q,sizeof(q));q.actor=owner->actor;
 /* Requalify every morph and its independent evaluator/buffer allocation.
  * This leaf never assumes the other three models fit the body skeleton. */
 for(i=0;i<4;i++){
  btr_copy(&b,owner,sizeof(b));b.morph=R32(owner->actor,rbs_member(i+1));
  if(!epj_allocated(owner,b.morph,0xc8))return RMS_CHANGED;b.model=R32(b.morph,0x90);
  if(!epj_allocated(owner,b.model,0xf8))return RMS_CHANGED;
  btr_copy(&r,archive,sizeof(r));r.model_wrapper=R32(b.model,0xac);
  if(!rbs_index(archive,r.model_wrapper,&k)||!epj_ram(r.model_wrapper,0x3c))return RMS_CHANGED;r.model=R32(r.model_wrapper,0);
  if(known){const KbjLayout*l;
   if(!epj_descriptor((u32)known,16)||!known[i])return RMS_BAD;z=_gz_kbj_validate(known[i]);if(z)return z;l=&known[i]->layout;
   if(l->common.bounds.actor!=owner->actor||l->common.bounds.pid!=owner->pid||l->common.bounds.morph!=b.morph||l->common.bounds.model!=b.model||
    l->common.bounds.heap!=owner->heap||l->common.bounds.head!=owner->head||l->common.bounds.tail!=owner->tail||l->common.bounds.end!=owner->end||
    l->common.bounds.source!=owner->source||l->common.bounds.source_bytes!=owner->source_bytes||l->resources.header!=archive->header||
    l->resources.bytes!=archive->bytes||l->resources.table!=archive->table||l->resources.count!=archive->count)return RMS_CHANGED;
   btr_copy(controllers+i,l,sizeof(*l));continue;
  }
  selected=R32(b.morph,0x94);
  z=kbj_layout(&b,&r,selected,selected,R32(b.morph,0x70),1,owner->source?0:1,controllers+i);if(z)return z;
 }
 for(i=0;i<8;i++){
  RmsModel*t=q.models+i;
  p=i<4?controllers[i].common.bounds.model:R32(owner->actor,0x7f0+(i-4)*4);
  if(!p){if(i<4)return RMS_CHANGED;continue;}
  if(!epj_allocated(owner,p,0xf8)||R32(p,0xc)!=GZ_R313_NATIVE_1016E8BC)return RMS_CHANGED;
  m=R32(p,0xac);if(!rbs_index(archive,m,&t->index)||!epj_ram(m,0x3c))return RMS_CHANGED;
  s=R32(m,0);if(!fpc_contract_member(archive->header,archive->bytes,0x20,s)||R32(s,0)!=0x464d444cu||
   !fpc_contract_relative(archive->header,archive->bytes,s+12,12,&s)||!s||R32(s,0)!=0x46534b4cu)return RMS_CHANGED;
  n=R16(s,8);h=R32(p,0x2c);if(!n||n>64||!epj_allocated(owner,h,0x14)||R32(h,0)!=s)return RMS_CHANGED;
  t->model=p;t->wrapper=m;t->skeleton=s;t->header=h;t->matrices=R32(h,0x10);t->bones=n;t->flags_fixed=R16(h,4)&~0x10u;
  if(!epj_allocated(owner,t->matrices,n*48))return RMS_CHANGED;q.presence|=1u<<i;
 }
 /* Matrices must not overlap any current model/header, other matrix bank,
  * morph, evaluator, interpolation or result allocation. */
 for(i=0;i<8;i++)if(q.presence&(1u<<i)){
  const RmsModel*t=q.models+i;u32 a[3]={t->model,t->header,t->matrices},bytes[3]={0xf8,0x14,t->bones*48};
  for(j=0;j<3;j++){
   if(!rms_span_apart(owner,archive,a[j],bytes[j])||!epj_apart((u32)out,sizeof(*out),a[j],bytes[j]))return RMS_ALIAS;
   for(k=0;k<j;k++)if(!epj_apart(a[j],bytes[j],a[k],bytes[k]))return RMS_ALIAS;
   for(k=0;k<i;k++)if(q.presence&(1u<<k)){
    const RmsModel*v=q.models+k;
    if(!epj_apart(a[j],bytes[j],v->model,0xf8)||!epj_apart(a[j],bytes[j],v->header,0x14)||!epj_apart(a[j],bytes[j],v->matrices,v->bones*48))return RMS_ALIAS;
   }
   for(k=0;k<4;k++){
    const KbjLayout*l=controllers+k;u32 buffer,used;
    z=krj_shape(l,&buffer,&used);if(z)return z;
    if((j!=0||i!=k)&&!epj_apart(a[j],bytes[j],l->common.bounds.model,0xf8))return RMS_ALIAS;
    if(!epj_apart(a[j],bytes[j],l->common.bounds.morph,0xc8)||!epj_apart(a[j],bytes[j],l->buffer,l->buffer_bytes)||
     !epj_apart(a[j],bytes[j],l->transforms,l->bones*32)||!epj_apart(a[j],bytes[j],l->quaternions,l->bones*16)||
     !epj_apart(a[j],bytes[j],buffer,used))return RMS_ALIAS;
   }
  }
 }
 btr_copy(out,&q,sizeof(q));return 0;
}
static u32 rms_layout(const KbjLayout*base,RmsLayout*out){return rms_layout_mode(base,out,0);}
static u32 rms_join(const KbjLayout*l,const RmsLayout*models,const PoEntry*process,u32 p,u32 published){u32 i,q,z=rms_packet(p,published),a=models->actor;
 if(z)return z;
 if(R32(p,28)!=process->profile||!rri_identity(process,p)||R32(p,32)!=R32(a,0xb0)||R32(p,36)!=R16(a,0x2d8)||
  R32(p,40)!=models->presence||R32(p,48)!=R8(a,0xa75)||R32(p,52)!=R8(a,0xa76)||R32(p,56)!=R8(a,0xa77)||R32(p,60)!=l->resources.count)return RMS_CHANGED;
 for(i=0;i<8;i++)if(models->presence&(1u<<i)){
  q=p+128+i*RMS_ROW;if(R32(q,8)!=models->models[i].index||R32(q,12)!=models->models[i].bones||
   (R32(q,16)&~0x10u)!=models->models[i].flags_fixed)return RMS_CHANGED;
 }return 0;
}
static u32 rms_encode(const KbjLayout*l,const RmsLayout*models,const PoEntry*process,u32 frame,u32 p,u32 published){u32 i,q,a=models->actor;
 eks_zero((void*)p,RMS_BYTES);*(u32*)(p+0)=0x524d5331u;*(u32*)(p+4)=RMS_BYTES;*(u32*)(p+8)=2;*(u32*)(p+12)=published;
 *(u32*)(p+16)=process->pid;*(u32*)(p+20)=process->generation;*(u32*)(p+24)=frame;*(u32*)(p+28)=process->profile;
 *(u32*)(p+32)=R32(a,0xb0);*(u32*)(p+36)=R16(a,0x2d8);*(u32*)(p+40)=models->presence;*(u32*)(p+44)=8;
 *(u32*)(p+48)=R8(a,0xa75);*(u32*)(p+52)=R8(a,0xa76);*(u32*)(p+56)=R8(a,0xa77);*(u32*)(p+60)=l->resources.count;
 *(u32*)(p+68)=0x524d4431u;*(u32*)(p+72)=64;
 for(i=0;i<8;i++)if(models->presence&(1u<<i)){
  const RmsModel*m=models->models+i;q=p+128+i*RMS_ROW;
  *(u32*)(q+0)=1;*(u32*)(q+4)=i+1;*(u32*)(q+8)=m->index;*(u32*)(q+12)=m->bones;*(u32*)(q+16)=R16(m->header,4);
  btr_copy((void*)(q+64),(const void*)(m->model+0xbc),60);btr_copy((void*)(q+124),(const void*)m->matrices,m->bones*48);
 }*(u32*)(p+64)=rms_hash(p);return rms_join(l,models,process,p,published);
}
static u32 rms_match(const RmsLayout*l,u32 p){u32 i,q;for(i=0;i<8;i++)if(l->presence&(1u<<i)){
 const RmsModel*m=l->models+i;q=p+128+i*RMS_ROW;
 if(R16(m->header,4)!=R32(q,16)||!btr_equal((const void*)(m->model+0xbc),(const void*)(q+64),60)||
  !btr_equal((const void*)m->matrices,(const void*)(q+124),m->bones*48))return RMS_CHANGED;
 }return 0;}
u32 _gz_rms_capture(const KbjPlan*j,u32 frame,u32 out){PoEntry first,last;RmsLayout l,again;u32 z;
 if(frame!=_gz_pbac_frame)return RMS_BAD;z=rbs_current(j,1,frame,&first);if(z)return z;z=_gz_kbj_before_matches(j);if(z)return z;
 if(!rbs_output(j,out,RMS_BYTES)||!epj_apart(out,RMS_BYTES,(u32)&first,sizeof(first))||!epj_apart(out,RMS_BYTES,(u32)&last,sizeof(last))||
  !epj_apart(out,RMS_BYTES,(u32)&l,sizeof(l))||!epj_apart(out,RMS_BYTES,(u32)&again,sizeof(again)))return RMS_ALIAS;
 z=rms_layout(&j->layout,&l);if(z)return z;z=rms_encode(&j->layout,&l,&first,frame,out,0);if(z)return z;
 z=_gz_kbj_before_matches(j);if(z)return z;z=rbs_current(j,1,frame,&last);if(z)return z;
 z=rms_layout(&j->layout,&again);if(z)return z;if(!btr_equal(&first,&last,sizeof(first))||!btr_equal(&l,&again,sizeof(l)))return RMS_CHANGED;
 z=rms_match(&l,out);if(z)return z;__asm__ volatile("sync":::"memory");*(volatile u32*)(out+12)=1;return 0;
}
typedef struct {u32 magic,bytes,valid,frame,before_hash,target_hash;PoEntry process;RmsLayout layout;
 u8 before[RMS_BYTES],target[RMS_BYTES];u32 seal;} RmjPlan;
typedef char RmjPlanSize[sizeof(RmjPlan)==51812?1:-1];
static u32 rmj_hash(const RmjPlan*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;return h;}
u32 _gz_rmj_prepare(const KbjPlan*j,u32 source,u32 frame,RmjPlan*out){PoEntry first,last;RmsLayout again;const EpjBounds*b;u32 z,h;
 if(frame!=_gz_pbac_frame)return RMS_BAD;z=rbs_current(j,1,frame,&first);if(z)return z;z=_gz_kbj_before_matches(j);if(z)return z;b=&j->layout.common.bounds;
 if(!epj_ram(source,RMS_BYTES)||source<b->source||source-b->source>b->source_bytes||RMS_BYTES>b->source_bytes-(source-b->source))return RMS_ALIAS;
 z=_gz_rms_check(source);if(z)return z;if(!out||!rbs_output(j,(u32)out,sizeof(*out)))return RMS_ALIAS;
 h=btr_hash((const void*)source,RMS_BYTES);eks_zero(out,sizeof(*out));btr_copy(out->target,(const void*)source,RMS_BYTES);
 if(h!=btr_hash(out->target,RMS_BYTES)||h!=btr_hash((const void*)source,RMS_BYTES))return RMS_CHANGED;
 z=rms_layout(&j->layout,&out->layout);if(z)return z;z=rms_join(&j->layout,&out->layout,&first,(u32)out->target,1);if(z)return z;
 z=rms_encode(&j->layout,&out->layout,&first,frame,(u32)out->before,1);if(z)return z;
 z=_gz_kbj_before_matches(j);if(z)return z;z=rbs_current(j,1,frame,&last);if(z)return z;
 if(!btr_equal(&first,&last,sizeof(first)))return RMS_CHANGED;z=rms_layout(&j->layout,&again);if(z)return z;
 if(!btr_equal(&again,&out->layout,sizeof(again)))return RMS_CHANGED;z=rms_match(&out->layout,(u32)out->before);if(z)return z;
 out->magic=0x524d4a31u;out->bytes=sizeof(*out);out->frame=frame;out->before_hash=btr_hash(out->before,RMS_BYTES);out->target_hash=h;
 btr_copy(&out->process,&first,sizeof(first));out->seal=rmj_hash(out);__asm__ volatile("sync":::"memory");out->valid=1;return 0;
}
static u32 rmj_validate_mode(const KbjPlan*j,const RmjPlan*p,const KbjPlan*const*known){RmsLayout current;PoEntry process;u32 z;
 if(!p||!epj_ram((u32)p,sizeof(*p))||p->magic!=0x524d4a31u||p->bytes!=sizeof(*p)||p->valid!=1||p->seal!=rmj_hash(p)||
  p->before_hash!=btr_hash(p->before,RMS_BYTES)||p->target_hash!=btr_hash(p->target,RMS_BYTES))return RMS_BAD;
 z=rbs_current(j,1,p->frame,&process);if(z)return z;if(!rbs_output(j,(u32)p,sizeof(*p))||!btr_equal(&process,&p->process,sizeof(process)))return RMS_CHANGED;
 z=rms_layout_mode(&j->layout,&current,known);if(z)return z;if(!btr_equal(&current,&p->layout,sizeof(current)))return RMS_CHANGED;
 z=rms_join(&j->layout,&current,&process,(u32)p->before,1);if(z)return z;
 if(R32((u32)p->before,24)!=p->frame)return RMS_CHANGED;return rms_join(&j->layout,&current,&process,(u32)p->target,1);
}
u32 _gz_rmj_validate(const KbjPlan*j,const RmjPlan*p){return rmj_validate_mode(j,p,0);}
u32 _gz_rmj_validate_group(const KbjPlan*j,const RmjPlan*p,const KbjPlan*const*known){return known?rmj_validate_mode(j,p,known):RMS_BAD;}
static void rms_write(const RmsLayout*l,u32 p){u32 i,q;for(i=0;i<8;i++)if(l->presence&(1u<<i)){
 const RmsModel*m=l->models+i;q=p+128+i*RMS_ROW;btr_copy((void*)(m->model+0xbc),(const void*)(q+64),60);
 btr_copy((void*)m->matrices,(const void*)(q+124),m->bones*48);*(volatile u16*)(m->header+4)=(u16)R32(q,16);
 }}
u32 _gz_rmj_apply(const KbjPlan*j,const RmjPlan*p){u32 z=_gz_rmj_validate(j,p);if(z)return z;rms_write(&p->layout,(u32)p->target);return 0;}
u32 _gz_rmj_undo(const KbjPlan*j,const RmjPlan*p){u32 z=_gz_rmj_validate(j,p);if(z)return z;rms_write(&p->layout,(u32)p->before);return 0;}
u32 _gz_rmj_undo_group(const KbjPlan*j,const RmjPlan*p,const KbjPlan*const*known){u32 z=_gz_rmj_validate_group(j,p,known);if(z)return z;rms_write(&p->layout,(u32)p->before);return 0;}
u32 _gz_rmj_matches(const KbjPlan*j,const RmjPlan*p){u32 z=_gz_rmj_validate(j,p);return z?z:rms_match(&p->layout,(u32)p->target);}
u32 _gz_rmj_before_matches(const KbjPlan*j,const RmjPlan*p){u32 z=_gz_rmj_validate(j,p);return z?z:rms_match(&p->layout,(u32)p->before);}
