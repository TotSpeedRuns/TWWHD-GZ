/* Immutable private Rito numeric target. A real outer owner supplies source
 * pinning and physical lifetimes. Historical source memory is not reread by
 * validation, application or recovery after preparation. No normal caller.
 */
enum {RBP_BAD=1420,RBP_CHANGED=1421,RBP_ALIAS=1422};
typedef struct {u32 magic,bytes,valid,frame,role,source_hash;PoEntry process;u8 target[3232];u32 seal;} RbpPlan;
typedef char RbpPlanSize[sizeof(RbpPlan)==3324?1:-1];
static u32 rbp_hash(const RbpPlan*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;
 for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;return h;}
u32 _gz_rbp_prepare(const KbjPlan*j,u32 source,u32 role,u32 frame,RbpPlan*out){PoEntry before,after;u32 z,h;
 if(frame!=_gz_pbac_frame)return RBP_BAD;z=rbs_current(j,role,frame,&before);if(z)return z;z=_gz_kbj_before_matches(j);if(z)return z;
 z=rbs_source(j,role,frame,source,0);if(z)return z;
 if(!out||!rbs_output(j,(u32)out,sizeof(*out)))return RBP_ALIAS;
 h=btr_hash((const void*)source,3232);eks_zero(out,sizeof(*out));btr_copy(out->target,(const void*)source,3232);
 if(h!=btr_hash(out->target,3232)||h!=btr_hash((const void*)source,3232))return RBP_CHANGED;
 z=rbs_packet(j,role,frame,(u32)out->target,0);if(z)return z;
 z=_gz_kbj_before_matches(j);if(z)return z;z=rbs_current(j,role,frame,&after);if(z)return z;
 if(!btr_equal(&before,&after,sizeof(before)))return RBP_CHANGED;
 out->magic=0x52425031u;out->bytes=sizeof(*out);out->frame=frame;out->role=role;out->source_hash=h;btr_copy(&out->process,&before,sizeof(before));
 out->seal=rbp_hash(out);__asm__ volatile("sync":::"memory");out->valid=1;return 0;
}
u32 _gz_rbp_validate(const KbjPlan*j,const RbpPlan*p){PoEntry current;u32 z;
 if(!p||!epj_ram((u32)p,sizeof(*p))||p->magic!=0x52425031u||p->bytes!=sizeof(*p)||p->valid!=1||p->seal!=rbp_hash(p)||
  p->source_hash!=btr_hash(p->target,3232))return RBP_BAD;
 z=rbs_current(j,p->role,p->frame,&current);if(z)return z;
 if(!btr_equal(&p->process,&current,sizeof(current))||!rbs_output(j,(u32)p,sizeof(*p)))return RBP_CHANGED;
 return rbs_packet(j,p->role,p->frame,(u32)p->target,0);
}
u32 _gz_rbp_apply(const KbjPlan*j,const RbpPlan*p){u32 z=_gz_rbp_validate(j,p),i;const KbjLayout*l;
 if(z)return z;z=rbs_packet(j,p->role,p->frame,(u32)p->target,1);if(z)return z;l=&j->layout;
 if(R32((u32)p->target,8)==2){btr_copy((void*)(l->common.bounds.morph+8),p->target+112,12);*(volatile u32*)l->target_body=R32((u32)p->target,104);}
 btr_copy((void*)(l->common.bounds.morph+0xb0),p->target+128,12);btr_copy((void*)(l->common.bounds.morph+0x98),p->target+144,16);
 for(i=0;i<l->bones;i++){btr_copy((void*)(l->transforms+i*32),p->target+160+i*32,18);btr_copy((void*)(l->transforms+i*32+20),p->target+180+i*32,12);}
 btr_copy((void*)l->quaternions,p->target+2208,l->bones*16);return 0;
}
u32 _gz_rbp_matches(const KbjPlan*j,const RbpPlan*p){u32 z=_gz_rbp_validate(j,p),i;const KbjLayout*l;
 if(z)return z;z=rbs_packet(j,p->role,p->frame,(u32)p->target,1);if(z)return z;l=&j->layout;
 if(R32((u32)p->target,8)==2&&(!btr_equal((const void*)(l->common.bounds.morph+8),p->target+112,12)||R32(l->target_body,0)!=R32((u32)p->target,104)))return RBP_CHANGED;
 if(!btr_equal((const void*)(l->common.bounds.morph+0xb0),p->target+128,12)||!btr_equal((const void*)(l->common.bounds.morph+0x98),p->target+144,16)||
  !btr_equal((const void*)l->quaternions,p->target+2208,l->bones*16))return RBP_CHANGED;
 for(i=0;i<l->bones;i++)if(!btr_equal((const void*)(l->transforms+i*32),p->target+160+i*32,18)||!btr_equal((const void*)(l->transforms+i*32+20),p->target+180+i*32,12))return RBP_CHANGED;
 return 0;
}
