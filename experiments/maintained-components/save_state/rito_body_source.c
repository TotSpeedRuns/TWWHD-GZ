/* Pointer-free Rito skeletal packets, one per class-defined controller.
 * Fresh current process observations precede native reads. The outer owner
 * must retain real process/heap/archive exclusion and all other participants.
 * No source pointer, callback, vtable, SDK cache or dispatch is assigned.
 * This leaf has no native runtime caller and grants no restore admission.
 */
#define RBS_W32(p,o,v) (*(volatile u32*)((p)+(o))=(v))
enum {RBS_BYTES=3232,RBS_BAD=1380,RBS_CHANGED=1381,RBS_ALIAS=1382,RBS_NUMERIC=1383,RBS_MISSING=63};
static u32 rbs_member(u32 role){return role==1?0x44cu:role==2?0x800u:role==3?0x884u:role==4?0x890u:0;}
static u32 rbs_hash(u32 p){u32 i,h=2166136261u;for(i=0;i<RBS_BYTES;i++)
 h=(h^((i>=12&&i<16)||(i>=76&&i<80)?0:R8(p,i)))*16777619u;return h;}
static u32 rbs_zero(u32 p,u32 first,u32 end){u32 i;for(i=first;i<end;i++)if(R8(p,i))return 0;return 1;}
static u32 rbs_index(const EptResources*r,u32 wrapper,u32*out){u32 i,found=0;
 for(i=0;i<r->count;i++)if(R32(r->table,i*4)==wrapper){if(++found!=1)return 0;*out=i;}return found==1;}
/* Validate only the sealed private plan and observer before reading actor
 * class fields or following morph/heap references from that actor. */
static u32 rbs_current(const KbjPlan*j,u32 role,u32 frame,PoEntry*out){const PoEntry*p;const EpjBounds*b;u32 a,z;
 if(!j||!out||!epj_ram((u32)j,sizeof(*j))||j->magic!=0x4b424a31u||j->bytes!=sizeof(*j)||j->valid!=1||
  j->seal!=kbj_hash(j)||!rbs_member(role)||!frame||!_gz_nsg_retained_frame(frame))return RBS_BAD;
 b=&j->layout.common.bounds;a=b->actor;
 if(_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return RBS_CHANGED;
 p=&_gz_po_observation.entry;
 if(p->object!=a||p->state!=PO_LIVE||!rtf_profile(p->profile)||p->process_name!=p->profile||p->allocation_bytes!=0xa7cu||
  p->base_bytes+p->other_bytes!=0xa7cu||!p->generation||p->flags||p->end_frame||!p->pid||p->pid>=0xfffffffeu||
  p->profile_entry!=_gz_po_profiles[p->profile]||!epj_ram(a,0xa7c))return RBS_CHANGED;
 if(b->actor_bytes!=0xa7cu||b->pid!=p->pid||R32(a,4)!=p->pid||R16(a,8)!=p->profile||R8(a,12)!=2||R16(a,14)!=p->profile||
  R32(a,16)!=p->profile_entry||R32(a,0xb4)!=GZ_R313_NATIVE_10017A18||R32(a,rbs_member(role))!=b->morph)return RBS_CHANGED;
 btr_copy(out,p,sizeof(*out));z=_gz_kbj_validate(j);return z?z:0;
}
/* Private output is separated from observer state, every current actor and
 * queue node, this journal's owned heap and all selected archive wrappers.
 * The caller still owns the actual private allocation and physical lifetime.
 */
static u32 rbs_storage(const KbjLayout*l,u32 out,u32 bytes){const EpjBounds*b=&l->common.bounds;
 u32 q,a,left=4096,i,w;const PoEntry*p;
 if(!kbj_external(b,&l->resources,l->before_body,l->target_body,out,bytes)||
  !epj_apart(out,bytes,l->bound_before,0x3c)||
  !epj_apart(out,bytes,(u32)&_gz_po_header,sizeof(_gz_po_header))||
  !epj_apart(out,bytes,(u32)_gz_po_rows,sizeof(_gz_po_rows))||
  !epj_apart(out,bytes,(u32)&_gz_po_observation,sizeof(_gz_po_observation))||
  !epj_apart(out,bytes,(u32)_gz_po_profiles,sizeof(_gz_po_profiles))||
  !epj_apart(out,bytes,(u32)&_gz_ncp_record,sizeof(_gz_ncp_record))||
  !epj_apart(out,bytes,(u32)&_gz_pba_native_actor_queue,4)||!epj_apart(out,bytes,(u32)&_gz_pbac_frame,4))return 0;
 for(i=0;i<l->resources.count;i++){w=R32(l->resources.table,i*4);if(w&&!epj_apart(out,bytes,w,0x3c))return 0;}
 q=_gz_pba_native_actor_queue;
 while(q){
  if(!left--||!epj_ram(q,16)||!epj_apart(out,bytes,q,16))return 0;a=R32(q,12);
  if(a){if(_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return 0;p=&_gz_po_observation.entry;
   if(p->object!=a||p->state!=PO_LIVE||p->flags||p->end_frame||!p->generation||!p->allocation_bytes||
    !epj_ram(a,p->allocation_bytes)||!epj_apart(out,bytes,a,p->allocation_bytes))return 0;}
  q=R32(q,8);
 }return 1;
}
static u32 rbs_output(const KbjPlan*j,u32 out,u32 bytes){return rbs_storage(&j->layout,out,bytes)&&epj_apart(out,bytes,(u32)j,sizeof(*j));}
u32 _gz_rbs_check(u32 p){u32 n,c,i;
 if(!epj_descriptor(p,RBS_BYTES)||R32(p,0)!=0x52425331u||R32(p,4)!=RBS_BYTES||(R32(p,8)!=1&&R32(p,8)!=2)||R32(p,12)!=1||
  !R32(p,16)||R32(p,16)>=0xfffffffeu||!R32(p,20)||!R32(p,24)||!rtf_profile(R32(p,28))||
  R32(p,36)>65535u||!rbs_member(R32(p,40))||!(n=R32(p,44))||n>64||
  !(c=R32(p,96))||c>4096||R32(p,48)>=c||R32(p,52)>=c||R32(p,56)>=c||
  R32(p,68)!=RBS_MISSING||R32(p,72)!=(R32(p,8)==2?0x4d434132u:0x4d434131u)||R32(p,76)!=rbs_hash(p)||
  R32(p,80)>255||R32(p,84)>255||R32(p,88)>255||R32(p,92)!=128||
  (R32(p,8)==1?!rbs_zero(p,100,128):(!epj_finite(R32(p,104))||!epj_finite(R32(p,108))||!epj_finite(R32(p,112))||!epj_finite(R32(p,116))||!epj_finite(R32(p,120))||R32(p,124)||(R32(p,48)==R32(p,56)&&R32(p,104)!=R32(p,108))))||!rbs_zero(p,140,144)||
  !rbs_zero(p,160+n*32,2208)||!rbs_zero(p,2208+n*16,RBS_BYTES))return RBS_BAD;
 for(i=0;i<n;i++)if(R8(p,160+i*32+18)||R8(p,160+i*32+19))return RBS_BAD;
 return kbn_numbers(p+128,p+144,p+160,p+2208,n)?0:RBS_NUMERIC;
}
u32 _gz_rbs_capture(const KbjPlan*j,u32 role,u32 frame,u32 out){
 u8 scratch[RBS_BYTES] __attribute__((aligned(4)));u32 p=(u32)scratch,z,n,i,body=0,model=0,bound=0,a;PoEntry first,last;KbtPlan binding;const KbjLayout*l;
 if(frame!=_gz_pbac_frame)return RBS_BAD;z=rbs_current(j,role,frame,&first);if(z)return z;l=&j->layout;a=l->common.bounds.actor;n=l->bones;
 z=_gz_kbj_before_matches(j);if(z)return z;
 if(l->before_body!=l->target_body||!rbs_output(j,out,RBS_BYTES)||!epj_apart(out,RBS_BYTES,p,RBS_BYTES)||
  !epj_apart(out,RBS_BYTES,(u32)&binding,sizeof(binding))||!epj_apart(out,RBS_BYTES,(u32)&first,sizeof(first))||
  !epj_apart(out,RBS_BYTES,(u32)&last,sizeof(last)))return RBS_ALIAS;
 z=_gz_kbt_contract(j,&binding);if(z)return z;
 if(!rbs_index(&l->resources,l->before_body,&body)||!rbs_index(&l->resources,l->common.model_wrapper,&model)||
  !rbs_index(&l->resources,l->bound_before,&bound))return RBS_CHANGED;
 eks_zero(scratch,sizeof(scratch));
 RBS_W32(p,0,0x52425331u);RBS_W32(p,4,RBS_BYTES);RBS_W32(p,8,2);RBS_W32(p,12,1);
 RBS_W32(p,16,first.pid);RBS_W32(p,20,first.generation);RBS_W32(p,24,frame);RBS_W32(p,28,first.profile);
 RBS_W32(p,32,R32(a,0xb0));RBS_W32(p,36,R16(a,0x2d8));RBS_W32(p,40,role);RBS_W32(p,44,n);
 RBS_W32(p,48,body);RBS_W32(p,52,model);RBS_W32(p,56,bound);
 RBS_W32(p,60,binding.animations[0].metadata);RBS_W32(p,64,binding.animations[2].metadata);RBS_W32(p,68,RBS_MISSING);RBS_W32(p,72,0x4d434132u);
 RBS_W32(p,80,R8(a,0xa75));RBS_W32(p,84,R8(a,0xa76));RBS_W32(p,88,R8(a,0xa77));RBS_W32(p,92,128);RBS_W32(p,96,l->resources.count);
 RBS_W32(p,100,R32((u32)j->object_before,0x80));RBS_W32(p,104,j->wrapper_frames[0]);RBS_W32(p,108,R32(l->bound_before,0));
 btr_copy(scratch+112,j->object_before+8,12);
 btr_copy(scratch+128,j->object_before+0xb0,12);btr_copy(scratch+144,j->object_before+0x98,16);
 for(i=0;i<n;i++){btr_copy(scratch+160+i*32,j->transforms_before+i*32,18);btr_copy(scratch+160+i*32+20,j->transforms_before+i*32+20,12);}
 btr_copy(scratch+2208,j->quaternions_before,n*16);RBS_W32(p,76,rbs_hash(p));z=_gz_rbs_check(p);if(z)return z;
 z=_gz_kbj_before_matches(j);if(z)return z;z=rbs_current(j,role,frame,&last);if(z)return z;
 if(!btr_equal(&first,&last,sizeof(first))||R32(a,0xb0)!=R32(p,32)||R16(a,0x2d8)!=R32(p,36)||
  R8(a,0xa75)!=R32(p,80)||R8(a,0xa76)!=R32(p,84)||R8(a,0xa77)!=R32(p,88))return RBS_CHANGED;
 RBS_W32(out,12,0);btr_copy((void*)out,scratch,12);btr_copy((void*)(out+16),scratch+16,RBS_BYTES-16);
 __asm__ volatile("sync":::"memory");RBS_W32(out,12,1);return 0;
}
static u32 rbs_packet(const KbjPlan*j,u32 role,u32 frame,u32 p,u32 require_bound){
 PoEntry current;KbtPlan binding;const KbjLayout*l;u32 z,index=0,model=0,a,m;
 z=rbs_current(j,role,frame,&current);if(z)return z;l=&j->layout;a=l->common.bounds.actor;m=l->common.bounds.morph;
 if(!epj_descriptor(p,RBS_BYTES))return RBS_ALIAS;
 z=_gz_rbs_check(p);if(z)return z;
 if(R32(p,28)!=current.profile||!rri_identity(&current,p)||R32(p,32)!=R32(a,0xb0)||R32(p,36)!=R16(a,0x2d8)||
  R32(p,40)!=role||R32(p,44)!=l->bones||R32(p,80)!=R8(a,0xa75)||R32(p,84)!=R8(a,0xa76)||R32(p,88)!=R8(a,0xa77)||
  R32(p,96)!=l->resources.count||!rbs_index(&l->resources,l->target_body,&index)||R32(p,48)!=index||
  !rbs_index(&l->resources,l->common.model_wrapper,&model)||R32(p,52)!=model)return RBS_CHANGED;
 z=_gz_kbt_contract(j,&binding);if(z)return z;
 /* Numeric projection does not silently collapse a source captured between
  * selecting a resource and binding it. Such a packet remains diagnostic;
  * exact partially-bound replay needs an additional transition participant. */
 if(R32(p,56)!=index||R32(p,60)!=binding.animations[1].metadata||R32(p,64)!=binding.animations[1].metadata)return RBS_CHANGED;
 if(require_bound&&(R32(m,0x94)!=l->target_body||R32(m,0x70)!=l->target_body||R32(m,0x48)!=binding.animations[1].data))return RBS_CHANGED;
 return 0;
}
static u32 rbs_source(const KbjPlan*j,u32 role,u32 frame,u32 p,u32 require_bound){PoEntry current;const EpjBounds*b;u32 z;
 z=rbs_current(j,role,frame,&current);if(z)return z;b=&j->layout.common.bounds;
 if(!epj_ram(p,RBS_BYTES)||p<b->source||p-b->source>b->source_bytes||RBS_BYTES>b->source_bytes-(p-b->source))return RBS_ALIAS;
 return rbs_packet(j,role,frame,p,require_bound);}
u32 _gz_rbs_qualify_source(const KbjPlan*j,u32 role,u32 frame,u32 p){return rbs_source(j,role,frame,p,0);}
u32 _gz_rbs_apply(const KbjPlan*j,u32 role,u32 frame,u32 p){u32 i,z=rbs_source(j,role,frame,p,1),h;const KbjLayout*l;
 if(z)return z;l=&j->layout;h=rbs_hash(p);
 if(R32(p,8)==2){btr_copy((void*)(l->common.bounds.morph+8),(const void*)(p+112),12);*(volatile u32*)l->target_body=R32(p,104);}
 btr_copy((void*)(l->common.bounds.morph+0xb0),(const void*)(p+128),12);btr_copy((void*)(l->common.bounds.morph+0x98),(const void*)(p+144),16);
 for(i=0;i<l->bones;i++){btr_copy((void*)(l->transforms+i*32),(const void*)(p+160+i*32),18);btr_copy((void*)(l->transforms+i*32+20),(const void*)(p+160+i*32+20),12);}
 btr_copy((void*)l->quaternions,(const void*)(p+2208),l->bones*16);return h==rbs_hash(p)?0:RBS_CHANGED;
}
u32 _gz_rbs_matches(const KbjPlan*j,u32 role,u32 frame,u32 p){u32 i,z=rbs_source(j,role,frame,p,1);const KbjLayout*l;
 if(z)return z;l=&j->layout;
 if(R32(p,8)==2&&(!btr_equal((const void*)(l->common.bounds.morph+8),(const void*)(p+112),12)||R32(l->target_body,0)!=R32(p,104)))return RBS_CHANGED;
 if(!btr_equal((const void*)(l->common.bounds.morph+0xb0),(const void*)(p+128),12)||
  !btr_equal((const void*)(l->common.bounds.morph+0x98),(const void*)(p+144),16)||
  !btr_equal((const void*)l->quaternions,(const void*)(p+2208),l->bones*16))return RBS_CHANGED;
 for(i=0;i<l->bones;i++)if(!btr_equal((const void*)(l->transforms+i*32),(const void*)(p+160+i*32),18)||
  !btr_equal((const void*)(l->transforms+i*32+20),(const void*)(p+160+i*32+20),12))return RBS_CHANGED;
 return 0;
}
/* The eventual root source joins four packets from one real capture scope.
 * This is structural consistency, not proof of physical exclusion. */
u32 _gz_rbs_group_check(u32 first){u32 i,j,p,z;static const u16 joins[] __attribute__((section(".text.rbs_joins"),aligned(4)))={8,16,20,24,28,32,36,80,84,88,92,96};
 if(!epj_descriptor(first,4*RBS_BYTES))return RBS_BAD;
 for(i=0;i<4;i++){p=first+i*RBS_BYTES;z=_gz_rbs_check(p);if(z)return z;if(R32(p,40)!=i+1)return RBS_CHANGED;
  for(j=0;j<sizeof(joins)/sizeof(joins[0]);j++)if(R32(p,joins[j])!=R32(first,joins[j]))return RBS_CHANGED;
 }return 0;
}

#undef RBS_W32
