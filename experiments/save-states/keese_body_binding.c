/* Read closure for original SetResource027E0C28 + BindByName027E07DC.
 * This deliberately excludes the broad morph binder: it writes shared FSKA
 * bind caches. All native stores below fit the existing current-generation
 * skeletal journal. The outer owner still supplies process/heap/archive
 * lifetime, draw/execute hold, attempted-before-call and final publication.
 * This leaf grants none of that authority and has no normal restore caller.
 * Curve evaluation and exact saved scalar/bone projection are separate work.
 */
enum {KBT_CONTEXT=560,KBT_RESOURCE=561,KBT_MODEL=562,KBT_CAPACITY=563,KBT_NAME=564,KBT_CONSTANT=565,KBT_ALIAS=566,KBT_CHANGED=567,KBT_NATIVE=568};
typedef struct {u32 data,rows,bones,curves,frames,flags,metadata;} KbtAnimation;
typedef struct {u32 magic,bytes,valid;KbjLayout layout;u32 dictionary,bone_rows;KbtAnimation animations[3];u32 seal;} KbtPlan;
typedef char KbtPlanSize[sizeof(KbtPlan)==292?1:-1];
extern void _gz_kbt_native_set_resource(u32,u32);
extern u32 _gz_kbt_native_bind_names(u32,u32);
extern u32 _gz_kbt_attempt_boundary(void);
static u32 kbt_hash(u32 h,const void*p,u32 n){u32 i;const u8*b=(const u8*)p;for(i=0;i<n;i++)h=(h^b[i])*16777619u;return h;}
static u32 kbt_seal(const KbtPlan*p){u32 h=2166136261u;h=kbt_hash(h,p,8);h=kbt_hash(h,(const u8*)p+12,sizeof(*p)-16);return h;}
static u32 kbt_private(const KbjPlan*p,u32 out){const KbjLayout*l=&p->layout;
 return kbj_external(&l->common.bounds,&l->resources,l->before_body,l->target_body,out,sizeof(KbtPlan))&&
  epj_apart(out,sizeof(KbtPlan),(u32)p,sizeof(*p))&&epj_apart(out,sizeof(KbtPlan),l->bound_before,20);}
/* fpc_contract_dictionary first bounds every native node/reference/name.
 * Simulate the exact native descending-bit search for each input name. A
 * cycle or wrong terminal node refuses, even when a linear lookup would fit.
 * No unvalidated reference-bit value becomes an unchecked string index.
 */
static u32 kbt_name_index(u32 dict,u32 count,u32 name,u32*out){u32 index=R16(dict,12),previous=R32(dict,8),node,bit,byte,branch,steps=0,other;
 for(;;){
  if(index>count||++steps>count+1)return 0;node=dict+8+index*16;bit=R32(node,0);
  if(previous<=bit)break;
  byte=bit>>3;branch=byte<R32(name-4,0)?((R8(name,byte)>>(bit&7))&1):0;
  previous=bit;index=R16(node,branch?6:4);
 }
 if(!index||index>count)return 0;other=node+8+R32(node,8);
 if(!ept_same_name(other,name))return 0;*out=index-1;return 1;}
static u32 kbt_skeleton(const KbjLayout*l,u32*dict,u32*bones,u32*digest){const EptResources*r=&l->resources;u32 n,i,p,name,rows,d,h;
 if(!fpc_contract_in(r->header,r->bytes,l->skeleton,0x24)||R32(l->skeleton,0)!=0x46534b4cu||
  R16(l->skeleton,8)!=l->bones||!l->bones||l->bones>64||
  !ept_relative(r,l->skeleton+16,24,&d)||!d||!ept_dictionary(r,d,&n)||n!=l->bones||
  !ept_relative(r,l->skeleton+20,l->bones*64,&rows)||!rows||(rows&3))return KBT_MODEL;
 h=kbt_hash(2166136261u,(const void*)l->skeleton,0x24);h=kbt_hash(h,(const void*)d,24+n*16);h=kbt_hash(h,(const void*)rows,n*64);
 for(i=0;i<n;i++){
  if(!ept_relative(r,d+24+i*16+8,1,&name)||!name||!ept_string(r,name)||
   !ept_relative(r,d+24+i*16+12,64,&p)||p!=rows+i*64||
   !kbt_name_index(d,n,name,&p)||p!=i)return KBT_NAME;
  h=kbt_hash(h,(const void*)(name-4),R32(name-4,0)+5);
  for(p=20;p<60;p+=4)if(!epj_finite(R32(rows+i*64,p)))return KBT_CONSTANT;
 }
 *dict=d;*bones=rows;*digest=h;return 0;}
static u32 kbt_animation(const KbjLayout*l,u32 dict,u32 skeleton_hash,u32 wrapper,KbtAnimation*out){
 const EptResources*r=&l->resources;u32 z,data,n,rows,i,row,name,index,flags,constants,bytes,j,h,curves,frames;
 z=kbj_wrapper(&l->common.bounds,r,wrapper);if(z)return z;data=R32(wrapper,8);
 flags=R32(data,12);frames=R32(data,16);n=R16(data,20);curves=R32(data,24);
 if(!n||n>l->bones||curves>l->cache_capacity||!frames||frames>32767||
  !ept_relative(r,data+32,n*24,&rows)||!rows||(rows&3))return KBT_CAPACITY;
 h=kbt_hash(skeleton_hash,(const void*)data,36);h=kbt_hash(h,(const void*)rows,n*24);
 for(i=0;i<n;i++){
  row=rows+i*24;flags=R32(row,0);bytes=((flags&8)?12:0)+((flags&16)?16:0)+((flags&32)?12:0);
  if(!ept_relative(r,row+4,1,&name)||!name||!ept_string(r,name)||
   !kbt_name_index(dict,l->bones,name,&index)||index>=l->bones)return KBT_NAME;
  if(!ept_relative(r,row+20,bytes,&constants)||(!constants&&bytes)||(bytes&&(constants&3)))return KBT_RESOURCE;
  for(j=0;j<bytes;j+=4)if(!epj_finite(R32(constants,j)))return KBT_CONSTANT;
  h=kbt_hash(h,(const void*)(name-4),R32(name-4,0)+5);
  if(bytes)h=kbt_hash(h,(const void*)constants,bytes);
 }
 out->data=data;out->rows=rows;out->bones=n;out->curves=curves;out->frames=frames;out->flags=R32(data,12);out->metadata=h;return 0;}
u32 _gz_kbt_contract(const KbjPlan*p,KbtPlan*out){KbtPlan q;u32 z,h;
 if(!p||!out||!epj_descriptor((u32)out,sizeof(*out)))return KBT_CONTEXT;
 z=_gz_kbj_validate(p);if(z)return z;if(!kbt_private(p,(u32)out))return KBT_ALIAS;
 eks_zero(&q,sizeof(q));btr_copy(&q.layout,&p->layout,sizeof(q.layout));
 z=kbt_skeleton(&q.layout,&q.dictionary,&q.bone_rows,&h);if(z)return z;
 z=kbt_animation(&q.layout,q.dictionary,h,q.layout.before_body,q.animations);if(z)return z;
 z=kbt_animation(&q.layout,q.dictionary,h,q.layout.target_body,q.animations+1);if(z)return z;
 z=kbt_animation(&q.layout,q.dictionary,h,q.layout.bound_before,q.animations+2);if(z)return z;
 q.magic=0x4b425431u;q.bytes=sizeof(q);q.seal=kbt_seal(&q);btr_copy(out,&q,sizeof(q));
 __asm__ volatile("sync":::"memory");out->valid=1;return 0;}
u32 _gz_kbt_rebind(const KbjPlan*p,const KbtPlan*contract){KbtPlan now;u32 z,m,target;
 if(!contract||!epj_descriptor((u32)contract,sizeof(*contract))||contract->magic!=0x4b425431u||contract->bytes!=sizeof(*contract)||
  contract->valid!=1||contract->seal!=kbt_seal(contract))return KBT_CONTEXT;
 z=_gz_kbt_contract(p,&now);if(z)return z;
 if(!kbt_private(p,(u32)contract)||!btr_equal(&now,contract,sizeof(now)))return KBT_CHANGED;
 z=_gz_kbj_before_matches(p);if(z)return z;m=p->layout.common.bounds.morph;target=p->layout.target_body;
 /* The outer writer MUST mark this row attempted before entering this leaf:
  * the native functions below can mutate buffers before reporting a failure. */
 *(volatile u32*)(m+0x70)=target;*(volatile u32*)(m+0x94)=target;
 z=_gz_kbt_attempt_boundary();if(z)return z;
 _gz_kbt_native_set_resource(m+4,now.animations[1].data);
 if(_gz_kbt_native_bind_names(m+4,now.layout.skeleton)!=0xffff0000u)return KBT_NATIVE;
 if(R32(m,0x48)!=now.animations[1].data||R32(m,0x4c)!=now.animations[1].rows||
  R16(m,0x44)!=now.animations[1].bones||R32(m,0x28)!=now.animations[1].curves)return KBT_NATIVE;
 return _gz_kbj_validate(p);}
