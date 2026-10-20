/* Current-generation Keese pattern before-images. This leaf is NOT restore
 * authority. Its caller must hold the actor/process generation, registered
 * solid heap and current archive lifetimes across capture, SDK and undo.
 * No saved address or snapshot byte is adopted as an undo destination.
 * Native SDK entry remains unwired until its owner/recovery integration is
 * validated. These functions add no normal slot/menu admission.
 */
enum {EPJ_BOUNDS=500,EPJ_OBJECT=501,EPJ_LAYOUT=502,EPJ_ALIAS=503,
 EPJ_PRIVATE=504,EPJ_CHANGED=505};
typedef struct {
 u32 actor,pid,actor_bytes,morph,heap,head,tail,end,face,model;
 u32 before_wrapper,target_wrapper,source,source_bytes;
} EpjBounds;
typedef struct {
 EpjBounds bounds;
 u32 buffer,buffer_bytes,cache,cache_capacity,binding,materials,results;
 u32 animation_capacity,result_capacity,mapping,model_wrapper;
} EpjLayout;
typedef struct {
 u32 magic,bytes,valid,used;
 EpjLayout layout;
 u8 face_before[0x74],buffer_before[8192];
 u32 actor_wrapper;u8 actor_cursor[8];
 u32 model_attachment,wrapper_frames[2],seal;
} EpjPlan;
typedef char EpjBoundsSize[sizeof(EpjBounds)==56?1:-1];
typedef char EpjLayoutSize[sizeof(EpjLayout)==100?1:-1];
typedef char EpjPlanSize[sizeof(EpjPlan)==8452?1:-1];

/* Normalize the publication marker so the seal can be completed before the
 * final release-store. Private plan access remains exclusive to its owner. */
static u32 epj_plan_hash(const EpjPlan*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;
 for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;
 return h;
}

static u32 epj_apart(u32 a,u32 n,u32 b,u32 m){return a>=b?a-b>=m:b-a>=n;}
static u32 epj_ram(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xc0000000u&&n&&n<=0xc0000000u-p;}
extern u32 _gz_hg_platform_thread(void);
/* Temporary C descriptors may live on Default Core1's low native stack.
 * This exception never qualifies an actor, resource, plan or undo target.
 * The genuine current-thread API and documented OSThread/OSContext tags and
 * bounds identify only the currently executing stack, not a saved pointer. */
static u32 epj_descriptor(u32 p,u32 n){u32 t,lo,hi;
 if(epj_ram(p,n))return 1;
 if((p&3)||p<0x0e000000u||p>=0x10000000u||!n||n>0x10000000u-p)return 0;
 t=_gz_hg_platform_thread();
 if((t&7)||t<0x0e000000u||t>0xc0000000u-0x39c||
  R32(t,0)!=0x4f53436fu||R32(t,4)!=0x6e747874u||R32(t,0x320)!=0x74487244u)return 0;
 lo=R32(t,0x398);hi=R32(t,0x394);
 return !(lo&7)&&!(hi&7)&&lo>=0x0e000000u&&hi<=0x10000000u&&hi>lo&&p>=lo&&p<hi&&n<=hi-p;
}
static u32 epj_allocated(const EpjBounds*b,u32 p,u32 n){
 return epj_ram(p,n)&&p>=b->heap+0x9c&&p<=b->end&&n<=b->end-p&&
  ((p<=b->head&&n<=b->head-p)||(p>=b->tail&&n<=b->end-p));
}
static u32 epj_finite(u32 value){return (value&0x7fffffffu)<0x7f800000u;}
static u32 epj_private(const EpjBounds*b,u32 p,u32 n){
 u32 wrapper;if(!epj_ram(b->model,0xb0))return 0;wrapper=R32(b->model,0xac);
 return epj_ram(wrapper,0x3c)&&epj_descriptor(p,n)&&epj_apart(p,n,wrapper,0x3c)&&epj_apart(p,n,b->actor,b->actor_bytes)&&
  epj_apart(p,n,b->heap,b->end-b->heap)&&epj_apart(p,n,b->morph,0xc8)&&
  epj_apart(p,n,b->model,0xb0)&&epj_apart(p,n,b->before_wrapper,16)&&
  epj_apart(p,n,b->target_wrapper,16)&&epj_apart(p,n,b->source,b->source_bytes);
}
/* Only invariants are compared on the undo path. Native reusable init may
 * change frame/controller/resource selection, active counts and caches.
 * A valid inactive cache sentinel is +infinity, not a rejected camera state.
 */
static u32 epj_layout(const EpjBounds*b,EpjLayout*out){
 EpjLayout q;u32 a,o,w,buf,cache,cap,mat,bind,results,ac,rc,map,end,fn,i,p;
 if(!b||!out||!epj_descriptor((u32)b,sizeof(*b))||!epj_descriptor((u32)out,sizeof(*out)))return EPJ_BOUNDS;
 if(!epj_ram(b->actor,b->actor_bytes)||b->actor_bytes!=0x10bc||
  !epj_ram(b->heap,b->end-b->heap)||b->end<=b->heap||b->heap+0x9c>b->head||
  b->head>b->tail||b->tail>b->end||!epj_ram(b->morph,0xc8)||!epj_ram(b->model,0xb0)||
  !epj_ram(b->before_wrapper,16)||!epj_ram(b->target_wrapper,16)||
  !epj_ram(b->source,b->source_bytes)||b->source_bytes!=0x200000u)return EPJ_BOUNDS;
 if(!epj_private(b,(u32)out,sizeof(*out))||!epj_apart((u32)out,sizeof(*out),(u32)b,sizeof(*b)))return EPJ_ALIAS;
 /* Require every current target, including complete objects, to be outside
  * the immutable source allocation; this also applies during source-free undo. */
 if(!epj_apart(b->actor,b->actor_bytes,b->source,b->source_bytes)||
  !epj_apart(b->heap,b->end-b->heap,b->source,b->source_bytes)||
  !epj_apart(b->before_wrapper,16,b->source,b->source_bytes)||
  !epj_apart(b->target_wrapper,16,b->source,b->source_bytes))return EPJ_ALIAS;
 a=b->actor;o=b->face;
 if(!epj_allocated(b,o,0x74)||!epj_allocated(b,b->morph,0xc8)||!epj_allocated(b,b->model,0xb0))return EPJ_BOUNDS;
 if(R32(a,4)!=b->pid||R16(a,8)!=215||R8(a,12)!=2||R32(a,0xf4)!=b->heap||
  R32(a,0x3d0)!=b->morph||R32(a,0x4e4)!=o||R32(b->morph,0x90)!=b->model||
  R32(b->heap,0x1c)!=b->heap||R32(b->heap,0x20)!=b->end-b->heap||
  R32(b->heap,0x94)!=b->head||R32(b->heap,0x98)!=b->tail)return EPJ_OBJECT;
 p=R32(o,16);
 /* Logical pattern is reversible actor state, not allocation identity. It
  * may differ from the last rendered pattern between Execute and Draw. The
  * outer participant qualifies it through the current Ki archive; this leaf
  * only copies the current before-image and never follows that pointer. */
 if(p!=b->before_wrapper&&p!=b->target_wrapper)return EPJ_OBJECT;
 for(i=0;i<2;i++){
  p=i?b->target_wrapper:b->before_wrapper;
  if(R32(p,4)!=(u32)_gz_fpc_native_pattern_resource_vtable||!epj_finite(R32(p,0)))return EPJ_OBJECT;
 }
 w=o+0x1c;buf=R32(w,0x30);cache=R32(w,0x1c);cap=R32(w,0x20);
 mat=R16(w,0x3e);bind=R32(w,0x38);results=R32(w,0x2c);
 ac=R32(w,0x4c);rc=R32(w,0x50);map=R32(w,0x54);fn=R32(w,0x14);
 if(R32(w,0)!=w+4||R32(w,0x34)!=(u32)_gz_face_native_pattern_vtable||
  (fn!=(u32)_gz_face_native_frame_clamp&&fn!=(u32)_gz_face_native_frame_loop))return EPJ_OBJECT;
 for(i=4;i<=16;i+=4)if(!epj_finite(R32(w,i)))return EPJ_OBJECT;
 if(!cap||cap>512||!mat||mat>64||!ac||ac>64||!rc||rc>64||
  R16(w,0x42)!=mat||R16(w,0x40)>ac||R32(w,0x24)>cap)return EPJ_LAYOUT;
 /* Layout emitted by native pattern-buffer init: result u16 array,
  * aligned material bindings, byte mapping, aligned 12-byte cache rows. */
 if(results!=buf||bind!=buf+((ac*2+3)&~3u)||map!=bind+mat*4||
  cache!=((map+rc+3)&~3u)||cache<buf)return EPJ_LAYOUT;
 end=cache+cap*12;if(end<cache||end-buf>8192||!epj_allocated(b,buf,end-buf))return EPJ_LAYOUT;
 if(!epj_apart(buf,end-buf,o,0x74)||!epj_apart(buf,end-buf,b->morph,0xc8)||
  !epj_apart(buf,end-buf,b->model,0xb0)||!epj_apart(o,0x74,b->morph,0xc8)||
  !epj_apart(o,0x74,b->model,0xb0)||!epj_apart(b->morph,0xc8,b->model,0xb0))return EPJ_ALIAS;
 for(i=0;i<sizeof(q)/4;i++)((u32*)&q)[i]=0;
 btr_copy(&q.bounds,b,sizeof(*b));q.buffer=buf;q.buffer_bytes=end-buf;q.cache=cache;q.cache_capacity=cap;
 q.binding=bind;q.materials=mat;q.results=results;q.animation_capacity=ac;
 q.result_capacity=rc;q.mapping=map;q.model_wrapper=R32(b->model,0xac);btr_copy(out,&q,sizeof(q));return 0;
}
u32 _gz_epj_describe(const EpjBounds*b,EpjLayout*out){return epj_layout(b,out);}
u32 _gz_epj_prepare(const EpjBounds*b,EpjPlan*p){EpjLayout q;u32 i,z;
 if(!b||!p||!epj_descriptor((u32)b,sizeof(*b))||!epj_ram((u32)p,sizeof(*p)))return EPJ_PRIVATE;
 z=epj_layout(b,&q);if(z)return z;
 if(!epj_private(b,(u32)p,sizeof(*p))||!epj_apart((u32)p,sizeof(*p),(u32)b,sizeof(*b)))return EPJ_ALIAS;
 if(R32(b->face,16)!=b->before_wrapper)return EPJ_CHANGED;
 for(i=0;i<sizeof(*p)/4;i++)((u32*)p)[i]=0;
 p->magic=0x45504a32u;p->bytes=sizeof(*p);p->used=q.buffer_bytes;btr_copy(&p->layout,&q,sizeof(q));
 btr_copy(p->face_before,(const void*)b->face,0x74);btr_copy(p->buffer_before,(const void*)q.buffer,q.buffer_bytes);
 p->actor_wrapper=R32(b->actor,0x4e0);btr_copy(p->actor_cursor,(const void*)(b->actor+0x4e8),6);
 p->model_attachment=R32(q.model_wrapper,0x38);p->wrapper_frames[0]=R32(b->before_wrapper,0);p->wrapper_frames[1]=R32(b->target_wrapper,0);
 p->seal=epj_plan_hash(p);__asm__ volatile("sync":::"memory");p->valid=1;return 0;
}
u32 _gz_epj_validate(const EpjPlan*p){EpjLayout q;u32 z;
 if(!p||!epj_ram((u32)p,sizeof(*p))||p->magic!=0x45504a32u||p->bytes!=sizeof(*p)||p->valid!=1||
  p->used>8192||p->used!=p->layout.buffer_bytes||p->seal!=epj_plan_hash(p))return EPJ_PRIVATE;
 if(!epj_private(&p->layout.bounds,(u32)p,sizeof(*p)))return EPJ_ALIAS;
 z=epj_layout(&p->layout.bounds,&q);if(z)return z;
 /* The current shared model wrapper is identity. Its +0x38 attachment is a
  * reversible shared write of native entry025E7AFC, not model-instance data.
  * Loop/clamp callback is validated independently and restored in face bytes. */
 return btr_equal(&q,&p->layout,sizeof(q))?0:EPJ_CHANGED;
}
u32 _gz_epj_undo(const EpjPlan*p){const EpjBounds*b;u32 z=_gz_epj_validate(p);
 if(z)return z;b=&p->layout.bounds;
 /* All refusals precede native stores. This restores this attempt's CURRENT
  * before-images while lifetimes are held; it never copies saved pointers. */
 btr_copy((void*)b->face,p->face_before,0x74);btr_copy((void*)p->layout.buffer,p->buffer_before,p->used);
 *(volatile u32*)(b->actor+0x4e0)=p->actor_wrapper;btr_copy((void*)(b->actor+0x4e8),p->actor_cursor,6);
 *(volatile u32*)(p->layout.model_wrapper+0x38)=p->model_attachment;
 *(volatile u32*)b->before_wrapper=p->wrapper_frames[0];*(volatile u32*)b->target_wrapper=p->wrapper_frames[1];return 0;
}
u32 _gz_epj_before_matches(const EpjPlan*p){const EpjBounds*b;u32 z=_gz_epj_validate(p);
 if(z)return z;b=&p->layout.bounds;
 return btr_equal((const void*)b->face,p->face_before,0x74)&&
  btr_equal((const void*)p->layout.buffer,p->buffer_before,p->used)&&R32(b->actor,0x4e0)==p->actor_wrapper&&
  btr_equal((const void*)(b->actor+0x4e8),p->actor_cursor,6)&&R32(p->layout.model_wrapper,0x38)==p->model_attachment&&
  R32(b->before_wrapper,0)==p->wrapper_frames[0]&&R32(b->target_wrapper,0)==p->wrapper_frames[1]?0:EPJ_CHANGED;
}
