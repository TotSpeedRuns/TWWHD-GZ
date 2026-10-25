/* RRS v2 preserves every bit of the bounded owned derived-result bank,
 * including stale rows of invisible models. These rows have no native
 * pointer fields and are only copied, never followed as references. Actor
 * positions/control floats, process/resource joins and exact allocation
 * bounds remain checked. Historical v1 retains its finite-result rule. */
/* Pointer-free derived skeletal records and their owning MCA flags. The
 * source links to the exact control packet, preventing mixed saved poses.
 * Current result pointers come only from the existing strict KRJ topology.
 * No model/material/particle refresh or ordinary replay permission is added.
 */
enum {RRS_BYTES=3680,RRS_HEADER=96,RRS_BAD=1440,RRS_CHANGED=1441,RRS_ALIAS=1442};
static u32 rrs_hash(u32 p){u32 i,h=2166136261u;for(i=0;i<RRS_BYTES;i++)h=(h^((i>=12&&i<16)||(i>=56&&i<60)?0:R8(p,i)))*16777619u;return h;}
static u32 rrs_numbers(u32 p,u32 bones){u32 i,j;for(i=0;i<bones;i++)for(j=4;j<56;j+=4)if(!epj_finite(R32(p+i*56,j)))return 0;return 1;}
u32 _gz_rrs_check(u32 p){u32 bones;
 if(!epj_descriptor(p,RRS_BYTES)||R32(p,0)!=0x52525331u||R32(p,4)!=RRS_BYTES||(R32(p,8)!=1&&R32(p,8)!=2)||R32(p,12)!=1||
  !R32(p,16)||R32(p,16)>=0xfffffffeu||!R32(p,20)||!R32(p,24)||!rtf_profile(R32(p,28))||!rbs_member(R32(p,32))||
  !(bones=R32(p,36))||bones>64||R32(p,40)>=4096||R32(p,44)>=4096||R32(p,48)>=4096||
  R32(p,56)!=rrs_hash(p)||R32(p,60)!=RRS_HEADER+bones*56||R32(p,68)!=0x4d434132u||
  !rbs_zero(p,72,RRS_HEADER)||!rbs_zero(p,RRS_HEADER+bones*56,RRS_BYTES)||(R32(p,8)==1&&!rrs_numbers(p+RRS_HEADER,bones)))return RRS_BAD;
 return 0;
}
static u32 rrs_join(u32 p,u32 control){u32 z=_gz_rrs_check(p);if(z)return z;z=_gz_rbs_check(control);if(z)return z;
 return R32(control,8)==2&&R32(p,28)==R32(control,28)&&R32(p,16)==R32(control,16)&&R32(p,20)==R32(control,20)&&R32(p,24)==R32(control,24)&&
  R32(p,32)==R32(control,40)&&R32(p,36)==R32(control,44)&&R32(p,40)==R32(control,52)&&R32(p,44)==R32(control,48)&&
  R32(p,48)==R32(control,56)&&R32(p,52)==R32(control,100)&&R32(p,64)==btr_hash((const void*)control,3232)?0:RRS_CHANGED;
}
static u32 rrs_encode(const KbjLayout*l,const PoEntry*process,u32 role,u32 frame,u32 control,u32 out){u32 p=out,buffer,bytes,z;
 z=_gz_rbs_check(control);if(z)return z;if(R32(control,8)!=2)return RRS_CHANGED;
 z=krj_shape(l,&buffer,&bytes);if(z)return z;
 /* Derived records can be uninitialized for a model never evaluated. */
 eks_zero((void*)p,RRS_BYTES);*(u32*)(p+0)=0x52525331u;*(u32*)(p+4)=RRS_BYTES;*(u32*)(p+8)=2;*(u32*)(p+12)=1;
 *(u32*)(p+16)=process->pid;*(u32*)(p+20)=process->generation;*(u32*)(p+24)=frame;*(u32*)(p+28)=process->profile;
 *(u32*)(p+32)=role;*(u32*)(p+36)=l->bones;*(u32*)(p+40)=R32(control,52);*(u32*)(p+44)=R32(control,48);*(u32*)(p+48)=R32(control,56);
 *(u32*)(p+52)=R32(l->common.bounds.morph,0x80);*(u32*)(p+60)=RRS_HEADER+bytes;*(u32*)(p+64)=btr_hash((const void*)control,3232);*(u32*)(p+68)=0x4d434132u;
 btr_copy((void*)(p+RRS_HEADER),(const void*)buffer,bytes);*(u32*)(p+56)=rrs_hash(p);
 return rrs_join(p,control);
}
u32 _gz_rrs_capture(const KbjPlan*j,u32 control,u32 role,u32 frame,u32 out){
 u8 current[3232] __attribute__((aligned(4))),scratch[RRS_BYTES] __attribute__((aligned(4)));
 PoEntry first,last;u32 z,buffer,bytes;
 if(frame!=_gz_pbac_frame)return RRS_BAD;z=rbs_current(j,role,frame,&first);if(z)return z;z=_gz_kbj_before_matches(j);if(z)return z;
 if(!rbs_output(j,out,RRS_BYTES)||!epj_descriptor(control,3232)||!epj_apart(out,RRS_BYTES,control,3232)||
  !epj_apart(out,RRS_BYTES,(u32)current,sizeof(current))||!epj_apart(out,RRS_BYTES,(u32)scratch,sizeof(scratch))||
  !epj_apart(out,RRS_BYTES,(u32)&first,sizeof(first))||!epj_apart(out,RRS_BYTES,(u32)&last,sizeof(last)))return RRS_ALIAS;
 z=_gz_rbs_capture(j,role,frame,(u32)current);if(z)return z;
 if(!btr_equal(current,(const void*)control,sizeof(current)))return RRS_CHANGED;
 z=rbs_packet(j,role,frame,control,1);if(z)return z;z=rrs_encode(&j->layout,&first,role,frame,control,(u32)scratch);if(z)return z;
 z=_gz_kbj_before_matches(j);if(z)return z;z=rbs_current(j,role,frame,&last);if(z)return z;
 if(!btr_equal(&first,&last,sizeof(first)))return RRS_CHANGED;
 z=krj_shape(&j->layout,&buffer,&bytes);if(z)return z;
 if(R32(j->layout.common.bounds.morph,0x80)!=R32((u32)scratch,52)||!btr_equal((const void*)buffer,scratch+RRS_HEADER,bytes))return RRS_CHANGED;
 *(volatile u32*)(out+12)=0;btr_copy((void*)out,scratch,12);btr_copy((void*)(out+16),scratch+16,RRS_BYTES-16);
 __asm__ volatile("sync":::"memory");*(volatile u32*)(out+12)=1;return 0;
}

typedef struct {u32 magic,bytes,valid,frame,role,source_hash;PoEntry process;u8 target[RRS_BYTES];u32 seal;} RrpPlan;
typedef char RrpPlanSize[sizeof(RrpPlan)==3772?1:-1];
static u32 rrp_hash(const RrpPlan*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;
 for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;return h;}
static u32 rrp_private(const KbjPlan*j,const KrjPlan*before,const RbpPlan*numeric,u32 p){return rbs_output(j,p,sizeof(RrpPlan))&&
 epj_apart(p,sizeof(RrpPlan),(u32)before,sizeof(*before))&&epj_apart(p,sizeof(RrpPlan),(u32)numeric,sizeof(*numeric));}
u32 _gz_rrp_prepare(const KbjPlan*j,const KrjPlan*before,const RbpPlan*numeric,u32 source,RrpPlan*out){PoEntry current;const EpjBounds*b;u32 z,h;
 z=_gz_rbp_validate(j,numeric);if(z)return z;z=_gz_kbj_before_matches(j);if(z)return z;z=_gz_krj_before_matches(j,before);if(z)return z;
 b=&j->layout.common.bounds;if(!epj_ram(source,RRS_BYTES)||source<b->source||source-b->source>b->source_bytes||RRS_BYTES>b->source_bytes-(source-b->source))return RRS_ALIAS;
 z=rrs_join(source,(u32)numeric->target);if(z)return z;
 if(!out||!rrp_private(j,before,numeric,(u32)out))return RRS_ALIAS;
 if(numeric->frame!=_gz_pbac_frame)return RRS_BAD;z=rbs_current(j,numeric->role,numeric->frame,&current);if(z)return z;h=btr_hash((const void*)source,RRS_BYTES);
 eks_zero(out,sizeof(*out));btr_copy(out->target,(const void*)source,RRS_BYTES);
 if(h!=btr_hash(out->target,RRS_BYTES)||h!=btr_hash((const void*)source,RRS_BYTES))return RRS_CHANGED;
 out->magic=0x52525031u;out->bytes=sizeof(*out);out->frame=numeric->frame;out->role=numeric->role;out->source_hash=h;btr_copy(&out->process,&current,sizeof(current));
 out->seal=rrp_hash(out);__asm__ volatile("sync":::"memory");out->valid=1;return 0;
}
u32 _gz_rrp_validate(const KbjPlan*j,const KrjPlan*before,const RbpPlan*numeric,const RrpPlan*p){PoEntry current;u32 z;
 if(!p||!epj_ram((u32)p,sizeof(*p))||p->magic!=0x52525031u||p->bytes!=sizeof(*p)||p->valid!=1||p->seal!=rrp_hash(p)||p->source_hash!=btr_hash(p->target,RRS_BYTES))return RRS_BAD;
 z=_gz_rbp_validate(j,numeric);if(z)return z;z=_gz_krj_validate(j,before);if(z)return z;
 if(p->frame!=numeric->frame||p->role!=numeric->role||!rrp_private(j,before,numeric,(u32)p))return RRS_CHANGED;
 z=rbs_current(j,p->role,p->frame,&current);if(z)return z;if(!btr_equal(&current,&p->process,sizeof(current)))return RRS_CHANGED;
 return rrs_join((u32)p->target,(u32)numeric->target);
}
u32 _gz_rrp_apply(const KbjPlan*j,const KrjPlan*before,const RbpPlan*numeric,const RrpPlan*p){u32 z=_gz_rrp_validate(j,before,numeric,p);if(z)return z;
 z=rbs_packet(j,p->role,p->frame,(u32)numeric->target,1);if(z)return z;
 btr_copy((void*)before->buffer,p->target+RRS_HEADER,before->used);*(volatile u32*)(j->layout.common.bounds.morph+0x80)=R32((u32)p->target,52);return 0;
}
u32 _gz_rrp_matches(const KbjPlan*j,const KrjPlan*before,const RbpPlan*numeric,const RrpPlan*p){u32 z=_gz_rrp_validate(j,before,numeric,p);if(z)return z;
 return R32(j->layout.common.bounds.morph,0x80)==R32((u32)p->target,52)&&btr_equal((const void*)before->buffer,p->target+RRS_HEADER,before->used)?0:RRS_CHANGED;
}
