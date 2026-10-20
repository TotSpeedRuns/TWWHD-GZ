/* Typed saved morph state. Historical source pointers never become targets.
 * The enclosing KBR participant supplies original owner/generation/heap/archive
 * authority and a complete current before-image before this leaf may write.
 * SDK result/cache evaluation and final saved-world publication are separate.
 */
enum {KBN_CONTEXT=580,KBN_SOURCE=581,KBN_NUMERIC=582,KBN_CHANGED=583,KBN_ALIAS=584};
typedef struct {
 u32 magic,bytes,valid,source_hash,body_id;KbtPlan binding;
 u8 blend[12],frame[16],transforms[2048],quaternions[1024];u32 seal;
} KbnPlan;
typedef char KbnPlanSize[sizeof(KbnPlan)==3416?1:-1];
static u32 kbn_hash(const KbnPlan*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;
 for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;return h;}
static u32 kbn_private(const KbjPlan*j,u32 p){const KbjLayout*l=&j->layout;
 return kbj_external(&l->common.bounds,&l->resources,l->before_body,l->target_body,p,sizeof(KbnPlan))&&
  epj_apart(p,sizeof(KbnPlan),(u32)j,sizeof(*j))&&epj_apart(p,sizeof(KbnPlan),l->bound_before,20)&&
  epj_apart(p,sizeof(KbnPlan),l->buffer,l->buffer_bytes)&&
  epj_apart(p,sizeof(KbnPlan),l->transforms,l->bones*32)&&epj_apart(p,sizeof(KbnPlan),l->quaternions,l->bones*16);}
/* Every floating field is checked as a bit pattern. Signed rotations and the
 * native signed16 frame bounds retain their exact bits; mode/state are bytes.
 * Native -1 blend sentinels and negative playback are valid finite states.
 */
static u32 kbn_numbers(u32 blend,u32 frame,u32 transforms,u32 quaternions,u32 bones){u32 i,j,p;
 for(i=0;i<12;i+=4)if(!epj_finite(R32(blend,i)))return 0;
 if(!epj_finite(R32(frame,0))||!epj_finite(R32(frame,4)))return 0;
 for(i=0;i<bones;i++){
  p=transforms+i*32;
  for(j=0;j<12;j+=4)if(!epj_finite(R32(p,j))||!epj_finite(R32(p,20+j)))return 0;
  for(j=0;j<16;j+=4)if(!epj_finite(R32(quaternions+i*16,j)))return 0;
 }
 return 1;}
u32 _gz_kbn_prepare(const KbjPlan*j,const KbtPlan*binding,u32 source,u32 body_id,KbnPlan*out){
 KbtPlan now;const EpjBounds*b;u32 z,h,n,i;
 if(!j||!binding||!out||!epj_descriptor((u32)binding,sizeof(*binding))||!epj_descriptor((u32)out,sizeof(*out)))return KBN_CONTEXT;
 z=_gz_kbj_validate(j);if(z)return z;b=&j->layout.common.bounds;n=j->layout.bones;
 if(!kbn_private(j,(u32)out)||!epj_apart((u32)out,sizeof(*out),(u32)binding,sizeof(*binding)))return KBN_ALIAS;
 if(!epj_ram(source,10240)||source<b->source||source-b->source>b->source_bytes||10240>b->source_bytes-(source-b->source)||
  R32(source,0)!=b->pid||R32(source,48)!=n||R32(source,56)!=0x3d0||R32(source,4800)!=1||R32(source,4832)!=body_id||
  body_id>=j->layout.resources.count||R32(j->layout.resources.table,body_id*4)!=j->layout.target_body)return KBN_SOURCE;
 z=_gz_kbt_contract(j,&now);if(z)return z;
 if(!btr_equal(&now,binding,sizeof(now)))return KBN_CHANGED;
 h=btr_hash((const void*)source,10240);
 if(!kbn_numbers(source+20,source+4864,source+64,source+2240,n))return KBN_NUMERIC;
 eks_zero(out,sizeof(*out));out->magic=0x4b424e31u;out->bytes=sizeof(*out);out->source_hash=h;out->body_id=body_id;
 btr_copy(&out->binding,binding,sizeof(*binding));btr_copy(out->blend,(const void*)(source+20),12);
 btr_copy(out->frame,(const void*)(source+4864),16);
 for(i=0;i<n;i++){
  /* Two padding bytes between signed rotations and positions are not state. */
  btr_copy(out->transforms+i*32,(const void*)(source+64+i*32),18);
  btr_copy(out->transforms+i*32+20,(const void*)(source+64+i*32+20),12);
 }
 btr_copy(out->quaternions,(const void*)(source+2240),n*16);
 if(btr_hash((const void*)source,10240)!=h){eks_zero(out,sizeof(*out));return KBN_CHANGED;}
 out->seal=kbn_hash(out);__asm__ volatile("sync":::"memory");out->valid=1;return 0;}
u32 _gz_kbn_validate(const KbjPlan*j,const KbnPlan*p){KbtPlan now;u32 z,n;
 if(!p||!epj_descriptor((u32)p,sizeof(*p))||p->magic!=0x4b424e31u||p->bytes!=sizeof(*p)||p->valid!=1||
  p->seal!=kbn_hash(p))return KBN_CONTEXT;
 z=_gz_kbj_validate(j);if(z)return z;if(!kbn_private(j,(u32)p))return KBN_ALIAS;
 z=_gz_kbt_contract(j,&now);if(z)return z;
 if(!btr_equal(&now,&p->binding,sizeof(now))||p->body_id>=now.layout.resources.count||
  R32(now.layout.resources.table,p->body_id*4)!=now.layout.target_body)return KBN_CHANGED;
 n=now.layout.bones;
 return kbn_numbers((u32)p->blend,(u32)p->frame,(u32)p->transforms,(u32)p->quaternions,n)?0:KBN_NUMERIC;}
u32 _gz_kbn_apply(const KbjPlan*j,const KbnPlan*p){const KbjLayout*l;u32 z,i,m;
 z=_gz_kbn_validate(j,p);if(z)return z;l=&j->layout;m=l->common.bounds.morph;
 if(R32(m,0x94)!=l->target_body||R32(m,0x70)!=l->target_body||R32(m,0x48)!=p->binding.animations[1].data)return KBN_CHANGED;
 btr_copy((void*)(m+0xb0),p->blend,12);btr_copy((void*)(m+0x98),p->frame,16);
 for(i=0;i<l->bones;i++){
  btr_copy((void*)(l->transforms+i*32),p->transforms+i*32,18);
  btr_copy((void*)(l->transforms+i*32+20),p->transforms+i*32+20,12);
 }
 btr_copy((void*)l->quaternions,p->quaternions,l->bones*16);return 0;}
u32 _gz_kbn_matches(const KbjPlan*j,const KbnPlan*p){const KbjLayout*l;u32 z,i,m;
 z=_gz_kbn_validate(j,p);if(z)return z;l=&j->layout;m=l->common.bounds.morph;
 if(!btr_equal((const void*)(m+0xb0),p->blend,12)||!btr_equal((const void*)(m+0x98),p->frame,16)||
  !btr_equal((const void*)l->quaternions,p->quaternions,l->bones*16))return KBN_CHANGED;
 for(i=0;i<l->bones;i++)if(!btr_equal((const void*)(l->transforms+i*32),p->transforms+i*32,18)||
  !btr_equal((const void*)(l->transforms+i*32+20),p->transforms+i*32+20,12))return KBN_CHANGED;
 return 0;}
