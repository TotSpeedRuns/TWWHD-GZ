/* Typed KRS1 source projection into the CURRENT renderer allocation. No saved
 * address is followed. KBR supplies the held owner/process/heap/archive scope,
 * all before-images and attempted-before-write accounting. Preparation copies
 * values into an immutable private plan; apply/matches/recovery need no source.
 */
enum {KRN_CONTEXT=610,KRN_SOURCE=611,KRN_NUMERIC=612,KRN_CHANGED=613,KRN_ALIAS=614};
typedef struct {
 u32 magic,bytes,valid,source_hash,body_id,layout_hash,bones,flags;
 u8 data[64*56];u32 seal;
} KrnPlan;
typedef char KrnPlanSize[sizeof(KrnPlan)==3620?1:-1];
extern u32 _gz_krn_write_boundary(void);
static u32 krn_hash(const KrnPlan*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;
 for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;return h;}
static u32 krn_packet_hash(u32 p,u32 n){u32 i,h=2166136261u;
 for(i=0;i<n;i++)h=(h^((i<4||(i>=20&&i<24))?0:R8(p,i)))*16777619u;return h;}
static u32 krn_numbers(u32 p,u32 bones){u32 i,j;
 for(i=0;i<bones;i++)for(j=4;j<56;j+=4)if(!epj_finite(R32(p,i*56+j)))return 0;return 1;}
static u32 krn_private(const KbjPlan*j,const KrjPlan*r,const KbnPlan*n,u32 p){return
 krj_private(j,p)&&epj_apart(p,sizeof(KrnPlan),(u32)r,sizeof(*r))&&
 epj_apart(p,sizeof(KrnPlan),(u32)n,sizeof(*n))&&epj_apart(p,sizeof(KrnPlan),r->buffer,r->used);}
static u32 krn_bindings(const KbjPlan*j,const KrjPlan*r,const KbnPlan*n){u32 z;
 z=_gz_krj_validate(j,r);if(z)return z;return _gz_kbn_validate(j,n);}
u32 _gz_krn_prepare(const KbjPlan*j,const KrjPlan*r,const KbnPlan*n,u32 source,KrnPlan*out){
 const EpjBounds*b;u32 z,p,bones,used,h,i;
 if(!out||!epj_descriptor((u32)out,sizeof(*out)))return KRN_CONTEXT;
 z=krn_bindings(j,r,n);if(z)return z;
 if(!krn_private(j,r,n,(u32)out))return KRN_ALIAS;
 z=_gz_krj_before_matches(j,r);if(z)return z;b=&j->layout.common.bounds;bones=j->layout.bones;
 if(!epj_ram(source,10240)||source<b->source||source-b->source>b->source_bytes||
  10240>b->source_bytes-(source-b->source)||R32(source,0)!=b->pid||R32(source,48)!=bones||
  R32(source,52)!=215||R32(source,56)!=0x3d0||R32(source,4800)!=1||R32(source,4832)!=n->body_id)return KRN_SOURCE;
 h=btr_hash((const void*)source,10240);if(h!=n->source_hash)return KRN_CHANGED;
 p=source+6600;used=24+bones*56;
 if(R32(p,0)!=0x4b525331u||R32(p,4)!=used||R32(p,8)!=bones||(R32(p,12)&~15u)||
  R32(p,16)!=n->body_id||R32(p,20)!=krn_packet_hash(p,used))return KRN_SOURCE;
 for(i=used;i<3608;i++)if(R8(p,i))return KRN_SOURCE;
 if(!krn_numbers(p+24,bones))return KRN_NUMERIC;
 eks_zero(out,sizeof(*out));out->magic=0x4b524e31u;out->bytes=sizeof(*out);out->source_hash=h;
 out->body_id=n->body_id;out->layout_hash=btr_hash(&j->layout,sizeof(j->layout));out->bones=bones;out->flags=R32(p,12);
 btr_copy(out->data,(const void*)(p+24),bones*56);
 if(btr_hash((const void*)source,10240)!=h){eks_zero(out,sizeof(*out));return KRN_CHANGED;}
 out->seal=krn_hash(out);__asm__ volatile("sync":::"memory");out->valid=1;return 0;}
u32 _gz_krn_validate(const KbjPlan*j,const KrjPlan*r,const KbnPlan*n,const KrnPlan*p){u32 z;
 if(!p||!epj_descriptor((u32)p,sizeof(*p))||p->magic!=0x4b524e31u||p->bytes!=sizeof(*p)||p->valid!=1||
  p->seal!=krn_hash(p))return KRN_CONTEXT;
 z=krn_bindings(j,r,n);if(z)return z;if(!krn_private(j,r,n,(u32)p))return KRN_ALIAS;
 if(p->source_hash!=n->source_hash||p->body_id!=n->body_id||p->layout_hash!=btr_hash(&j->layout,sizeof(j->layout))||
  p->bones!=j->layout.bones||(p->flags&~15u))return KRN_CHANGED;
 return krn_numbers((u32)p->data,p->bones)?0:KRN_NUMERIC;}
u32 _gz_krn_apply(const KbjPlan*j,const KrjPlan*r,const KbnPlan*n,const KrnPlan*p){u32 z,i,m;
 z=_gz_krn_validate(j,r,n,p);if(z)return z;m=j->layout.common.bounds.morph;
 if(R32(m,0x94)!=j->layout.target_body||R32(m,0x70)!=j->layout.target_body||
  R32(m,0x48)!=n->binding.animations[1].data)return KRN_CHANGED;
 z=_gz_kbn_matches(j,n);if(z)return z;
 for(i=0;i<p->bones;i++){
  btr_copy((void*)(r->buffer+i*56),p->data+i*56,56);
  if(!i){z=_gz_krn_write_boundary();if(z)return z;}
 }
 *(volatile u32*)(m+0x80)=p->flags;return 0;}
u32 _gz_krn_matches(const KbjPlan*j,const KrjPlan*r,const KbnPlan*n,const KrnPlan*p){u32 z;
 z=_gz_krn_validate(j,r,n,p);if(z)return z;
 return R32(j->layout.common.bounds.morph,0x80)==p->flags&&btr_equal((const void*)r->buffer,p->data,p->bones*56)?0:KRN_CHANGED;}
