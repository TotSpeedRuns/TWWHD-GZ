/* The per-instance derived skeletal results are a separate allocation from
 * the animation evaluator and interpolation buffers. Journal CURRENT bytes
 * only, under the enclosing KBS process/registered-heap/archive lifetime.
 * This is source-free rollback, not saved renderer replay or capture authority.
 * No address in an old snapshot is read or installed by this module.
 */
enum {KRJ_CONTEXT=590,KRJ_LAYOUT=591,KRJ_ALIAS=592,KRJ_CHANGED=593};
typedef struct {
 u32 magic,bytes,valid,body_layout_hash,buffer,bones,used,flags;
 u8 before[64*56];u32 seal;
} KrjPlan;
typedef char KrjPlanSize[sizeof(KrjPlan)==3620?1:-1];
static u32 krj_hash(const KrjPlan*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;
 for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;return h;}
/* Only immutable topology is checked here. The native flags at morph+80
 * and all result bytes may change during a held attempt and must be undone.
 * Current observed Ki instances have all four result pointers aliased. Other
 * topologies need their own contract; no guessed address is followed.
 */
static u32 krj_shape(const KbjLayout*l,u32*buffer,u32*bytes){
 const EpjBounds*b=&l->common.bounds;u32 m=b->morph,n=l->bones,p=R32(m,0x78),i;
 u32 a[7]={b->morph,b->model,b->face,l->common.buffer,l->buffer,l->transforms,l->quaternions};
 u32 sizes[7]={0xc8,0xb0,0x74,l->common.buffer_bytes,l->buffer_bytes,n*32,n*16};
 if(!n||n>64||R16(m,0x7c)!=n||R16(m,0x7e)!=n||
  R32(m,0x84)!=p||R32(m,0x88)!=p||R32(m,0x8c)!=p||!epj_allocated(b,p,n*56))return KRJ_LAYOUT;
 if(!epj_apart(p,n*56,b->source,b->source_bytes)||
  !epj_apart(p,n*56,l->resources.header,l->resources.bytes)||
  !epj_apart(p,n*56,l->resources.table,l->resources.count*4)||
  !epj_apart(p,n*56,l->resources.model_wrapper,0x3c)||
  !epj_apart(p,n*56,l->before_body,20)||!epj_apart(p,n*56,l->target_body,20)||
  !epj_apart(p,n*56,l->bound_before,20))return KRJ_ALIAS;
 for(i=0;i<7;i++)if(!epj_apart(p,n*56,a[i],sizes[i]))return KRJ_ALIAS;
 *buffer=p;*bytes=n*56;return 0;}
static u32 krj_private(const KbjPlan*j,u32 p){const KbjLayout*l=&j->layout;
 return kbj_external(&l->common.bounds,&l->resources,l->before_body,l->target_body,p,sizeof(KrjPlan))&&
  epj_apart(p,sizeof(KrjPlan),(u32)j,sizeof(*j))&&epj_apart(p,sizeof(KrjPlan),l->bound_before,20);}
u32 _gz_krj_prepare(const KbjPlan*j,KrjPlan*p){u32 z,buffer,bytes;
 if(!p||!epj_ram((u32)p,sizeof(*p)))return KRJ_CONTEXT;
 z=_gz_kbj_validate(j);if(z)return z;
 if(!krj_private(j,(u32)p))return KRJ_ALIAS;
 z=krj_shape(&j->layout,&buffer,&bytes);if(z)return z;
 eks_zero(p,sizeof(*p));p->magic=0x4b524a31u;p->bytes=sizeof(*p);
 p->body_layout_hash=btr_hash(&j->layout,sizeof(j->layout));p->buffer=buffer;p->bones=j->layout.bones;p->used=bytes;
 p->flags=R32(j->layout.common.bounds.morph,0x80);btr_copy(p->before,(const void*)buffer,bytes);
 p->seal=krj_hash(p);__asm__ volatile("sync":::"memory");p->valid=1;return 0;}
u32 _gz_krj_validate(const KbjPlan*j,const KrjPlan*p){u32 z,buffer,bytes;
 if(!p||!epj_ram((u32)p,sizeof(*p))||p->magic!=0x4b524a31u||p->bytes!=sizeof(*p)||p->valid!=1||
  p->used>sizeof(p->before)||p->seal!=krj_hash(p))return KRJ_CONTEXT;
 z=_gz_kbj_validate(j);if(z)return z;if(!krj_private(j,(u32)p))return KRJ_ALIAS;
 z=krj_shape(&j->layout,&buffer,&bytes);if(z)return z;
 return p->body_layout_hash==btr_hash(&j->layout,sizeof(j->layout))&&p->buffer==buffer&&
  p->bones==j->layout.bones&&p->used==bytes?0:KRJ_CHANGED;}
u32 _gz_krj_undo(const KbjPlan*j,const KrjPlan*p){u32 z=_gz_krj_validate(j,p);
 if(z)return z;
 /* All ownership, topology, alias and seal checks precede the first store. */
 btr_copy((void*)p->buffer,p->before,p->used);
 *(volatile u32*)(j->layout.common.bounds.morph+0x80)=p->flags;return 0;}
u32 _gz_krj_before_matches(const KbjPlan*j,const KrjPlan*p){u32 z=_gz_krj_validate(j,p);
 if(z)return z;return R32(j->layout.common.bounds.morph,0x80)==p->flags&&
  btr_equal((const void*)p->buffer,p->before,p->used)?0:KRJ_CHANGED;}
