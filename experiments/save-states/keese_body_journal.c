/* Current-allocation skeletal before-images. The outer owner must retain
 * process generation, solid heap and archive lifetime. This leaf never reads
 * a saved body, calls a native setter, or grants ordinary restore admission.
 * Undo installs only this attempt's current before-images into the exact
 * same qualified objects; historical source addresses are never followed. */
enum {KBJ_BOUNDS=540,KBJ_OBJECT=541,KBJ_ARCHIVE=542,KBJ_LAYOUT=543,KBJ_ALIAS=544,KBJ_CHANGED=545,KBJ_PRIVATE=546};
typedef struct {
 EpjLayout common;EptResources resources;
 u32 skeleton,bones,buffer,buffer_bytes,cache,cache_capacity,binding;
 u32 transforms,quaternions,before_body,target_body,bound_before;
 u32 sound,callback1,callback2;
} KbjLayout;
typedef struct {
 u32 magic,bytes,valid,used;KbjLayout layout;
 u8 object_before[0xc8],buffer_before[8192],transforms_before[2048],quaternions_before[1024];
 u32 wrapper_frames[2],seal;
} KbjPlan;
typedef char KbjLayoutSize[sizeof(KbjLayout)==184?1:-1];
typedef char KbjPlanSize[sizeof(KbjPlan)==11676?1:-1];
static u32 kbj_hash(const KbjPlan*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;
 for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;return h;
}
static u32 kbj_external(const EpjBounds*b,const EptResources*r,u32 before,u32 target,u32 p,u32 n){
 return epj_descriptor(p,n)&&epj_private(b,p,n)&&epj_apart(p,n,r->header,r->bytes)&&
  epj_apart(p,n,r->table,r->count*4)&&epj_apart(p,n,r->model_wrapper,0x3c)&&
  epj_apart(p,n,before,20)&&epj_apart(p,n,target,20);
}
static u32 kbj_wrapper(const EpjBounds*b,const EptResources*r,u32 wrapper){u32 i,found=0,data;
 if(!epj_ram(wrapper,20)||!epj_apart(wrapper,20,b->source,b->source_bytes)||
  !epj_apart(wrapper,20,b->heap,b->end-b->heap)||!epj_apart(wrapper,20,b->actor,b->actor_bytes)||
  R32(wrapper,4)!=(u32)_gz_ern_native_body_vtable||R32(wrapper,16)!=(u32)_gz_ern_native_body_name_vtable||
  !epj_finite(R32(wrapper,0)))return KBJ_ARCHIVE;
 for(i=0;i<r->count;i++)if(R32(r->table,i*4)==wrapper)found++;
 if(found!=1)return KBJ_ARCHIVE;data=R32(wrapper,8);
 return !(data&3)&&fpc_contract_in(r->header,r->bytes,data,0x30)&&R32(data,0)==0x46534b41u&&
  fpc_contract_member(r->header,r->bytes,0x28,data)?0:KBJ_ARCHIVE;
}
static u32 kbj_layout(const EpjBounds*b,const EptResources*r,u32 before,u32 target,u32 bound_before,u32 partial,KbjLayout*out){
 KbjLayout p;u32 z,m,s,n,buf,bind,cache,cap,bones,end,i,current,bound,backend,found=0;
 if(!b||!r||!out||!epj_descriptor((u32)b,sizeof(*b))||!epj_descriptor((u32)r,sizeof(*r))||
  !epj_descriptor((u32)out,sizeof(*out)))return KBJ_BOUNDS;
 z=_gz_epj_describe(b,&p.common);if(z)return z;
 if(!epj_ram(r->header,r->bytes)||!fpc_contract_root(r->header,r->bytes)||!r->count||r->count>4096||
  !epj_ram(r->table,r->count*4)||!epj_ram(r->model_wrapper,0x3c))return KBJ_ARCHIVE;
 if(!kbj_external(b,r,before,target,(u32)out,sizeof(*out))||!epj_apart((u32)out,sizeof(*out),(u32)b,sizeof(*b))||
  !epj_apart((u32)out,sizeof(*out),(u32)r,sizeof(*r))||!epj_apart(r->header,r->bytes,b->source,b->source_bytes)||
  !epj_apart(r->table,r->count*4,b->source,b->source_bytes)||!epj_apart(r->model_wrapper,0x3c,b->source,b->source_bytes))return KBJ_ALIAS;
 if(R32(b->model,0xac)!=r->model_wrapper||R32(r->model_wrapper,0)!=r->model||
  !fpc_contract_member(r->header,r->bytes,0x20,r->model)||R32(r->model,0)!=0x464d444cu||
  !fpc_contract_relative(r->header,r->bytes,r->model+12,12,&s)||!s||R32(s,0)!=0x46534b4cu)return KBJ_ARCHIVE;
 for(i=0;i<r->count;i++)if(R32(r->table,i*4)==r->model_wrapper)found++;
 if(found!=1)return KBJ_ARCHIVE;
 z=kbj_wrapper(b,r,before);if(z)return z;z=kbj_wrapper(b,r,target);if(z)return z;
 z=kbj_wrapper(b,r,bound_before);if(z)return z;
 m=b->morph;current=R32(m,0x94);bones=R16(s,8);buf=R32(m,0x34);bind=R32(m,0x3c);cache=R32(m,0x20);cap=R32(m,0x24);n=R32(m,0x50);
 if(R32(m,0)!=0x10058cb8u||R32(m,4)!=m+8||R32(m,0x38)!=0x1016d830u||R32(m,0x6c)!=s||
  (current!=before&&current!=target)||!bones||bones>64||n!=bones||R16(m,0x42)!=bones||
  R32(m,0x30)!=buf||!cap||cap>512||R16(m,0x44)>bones||R16(m,0x46)>bones||
  R32(m,0x28)>cap||R32(m,0x54)!=8)return KBJ_OBJECT;
 /* The selected morph resource and the already bound backend resource are
  * separate states. Both must belong to this current archive; do not infer
  * equality from a snapshot taken between native setter and calculation. */
 bound=R32(m,0x70);backend=R32(m,0x48);
 /* Preparation still requires a coherent native binding. During an owned
  * undo, selection and SetResource stores may be at different points. The
  * only backend resources accepted then are the recorded current binding
  * and this exact attempt's target, each already qualified above. Never
  * follow an arbitrary mismatched data pointer to decide whether it fits. */
 if((bound!=bound_before&&bound!=target)||
  (!partial&&backend!=R32(bound,8))||
  (partial&&backend!=R32(bound_before,8)&&backend!=R32(target,8))||
  R16(backend,0x14)>bones||R32(backend,0x18)>cap)return KBJ_OBJECT;
 /* Native skeletal buffer: 48-byte result rows, u32 bone bindings, then
  * 12-byte curve caches. Transform and quaternion interpolation banks are
  * separate current solid-heap allocations, sized by the current skeleton. */
 if(bind!=buf+bones*48||cache!=bind+bones*4||cache<buf)return KBJ_LAYOUT;
 end=cache+cap*12;if(end<cache||end-buf>8192||!epj_allocated(b,buf,end-buf)||
  !epj_allocated(b,R32(m,0xa8),bones*32)||!epj_allocated(b,R32(m,0xac),bones*16))return KBJ_LAYOUT;
 p.skeleton=s;p.bones=bones;p.buffer=buf;p.buffer_bytes=end-buf;p.cache=cache;p.cache_capacity=cap;p.binding=bind;
 p.transforms=R32(m,0xa8);p.quaternions=R32(m,0xac);p.before_body=before;p.target_body=target;p.bound_before=bound_before;
 p.sound=R32(m,0xbc);p.callback1=R32(m,0xc0);p.callback2=R32(m,0xc4);
 {u32 a[7]={b->morph,b->model,b->face,p.common.buffer,buf,p.transforms,p.quaternions};
  u32 sizes[7]={0xc8,0xb0,0x74,p.common.buffer_bytes,end-buf,bones*32,bones*16};u32 j;
  for(i=0;i<7;i++){
   if(!epj_apart(a[i],sizes[i],b->source,b->source_bytes)||!epj_apart(a[i],sizes[i],r->header,r->bytes)||
    !epj_apart((u32)out,sizeof(*out),a[i],sizes[i]))return KBJ_ALIAS;
   for(j=0;j<i;j++)if(!epj_apart(a[i],sizes[i],a[j],sizes[j]))return KBJ_ALIAS;
  }
 }
 btr_copy(&p.resources,r,sizeof(*r));btr_copy(out,&p,sizeof(p));return 0;
}
u32 _gz_kbj_describe(const EpjBounds*b,const EptResources*r,u32 before,u32 target,KbjLayout*out){u32 bound;
 if(!b||!epj_descriptor((u32)b,sizeof(*b))||!epj_ram(b->morph,0xc8))return KBJ_BOUNDS;
 bound=R32(b->morph,0x70);return kbj_layout(b,r,before,target,bound,0,out);}
u32 _gz_kbj_prepare(const EpjBounds*b,const EptResources*r,u32 before,u32 target,KbjPlan*p){KbjLayout q;u32 z;
 if(!p||!epj_ram((u32)p,sizeof(*p)))return KBJ_PRIVATE;
 z=_gz_kbj_describe(b,r,before,target,&q);if(z)return z;
 if(!kbj_external(b,r,before,target,(u32)p,sizeof(*p))||!epj_apart((u32)p,sizeof(*p),(u32)b,sizeof(*b))||
  !epj_apart((u32)p,sizeof(*p),(u32)r,sizeof(*r)))return KBJ_ALIAS;
 if(R32(b->morph,0x94)!=before)return KBJ_CHANGED;
 {u32 a[3]={q.buffer,q.transforms,q.quaternions},n[3]={q.buffer_bytes,q.bones*32,q.bones*16},i;
  for(i=0;i<3;i++)if(!epj_apart((u32)p,sizeof(*p),a[i],n[i]))return KBJ_ALIAS;}
 eks_zero(p,sizeof(*p));p->magic=0x4b424a31u;p->bytes=sizeof(*p);p->used=q.buffer_bytes;btr_copy(&p->layout,&q,sizeof(q));
 btr_copy(p->object_before,(const void*)b->morph,0xc8);btr_copy(p->buffer_before,(const void*)q.buffer,q.buffer_bytes);
 btr_copy(p->transforms_before,(const void*)q.transforms,q.bones*32);btr_copy(p->quaternions_before,(const void*)q.quaternions,q.bones*16);
 p->wrapper_frames[0]=R32(before,0);p->wrapper_frames[1]=R32(target,0);p->seal=kbj_hash(p);
 __asm__ volatile("sync":::"memory");p->valid=1;return 0;
}
u32 _gz_kbj_validate(const KbjPlan*p){KbjLayout q;const KbjLayout*l;u32 z;
 if(!p||!epj_ram((u32)p,sizeof(*p))||p->magic!=0x4b424a31u||p->bytes!=sizeof(*p)||p->valid!=1||
  p->used>8192||p->used!=p->layout.buffer_bytes||p->seal!=kbj_hash(p))return KBJ_PRIVATE;
 l=&p->layout;
 if(!kbj_external(&l->common.bounds,&l->resources,l->before_body,l->target_body,(u32)p,sizeof(*p)))return KBJ_ALIAS;
 z=kbj_layout(&l->common.bounds,&l->resources,l->before_body,l->target_body,l->bound_before,1,&q);if(z)return z;
 return btr_equal(&q,l,sizeof(q))?0:KBJ_CHANGED;
}
u32 _gz_kbj_undo(const KbjPlan*p){u32 z=_gz_kbj_validate(p);const KbjLayout*l;
 if(z)return z;l=&p->layout;
 btr_copy((void*)l->common.bounds.morph,p->object_before,0xc8);btr_copy((void*)l->buffer,p->buffer_before,l->buffer_bytes);
 btr_copy((void*)l->transforms,p->transforms_before,l->bones*32);btr_copy((void*)l->quaternions,p->quaternions_before,l->bones*16);
 *(volatile u32*)l->before_body=p->wrapper_frames[0];*(volatile u32*)l->target_body=p->wrapper_frames[1];return 0;
}
u32 _gz_kbj_before_matches(const KbjPlan*p){u32 z=_gz_kbj_validate(p);const KbjLayout*l;
 if(z)return z;l=&p->layout;
 return btr_equal((const void*)l->common.bounds.morph,p->object_before,0xc8)&&btr_equal((const void*)l->buffer,p->buffer_before,l->buffer_bytes)&&
  btr_equal((const void*)l->transforms,p->transforms_before,l->bones*32)&&btr_equal((const void*)l->quaternions,p->quaternions_before,l->bones*16)&&
  R32(l->before_body,0)==p->wrapper_frames[0]&&R32(l->target_body,0)==p->wrapper_frames[1]?0:KBJ_CHANGED;
}
