/* Typed resource/capacity check for the native reused-buffer Keese path.
 * Archive/controller/process/heap lifetime authority belongs to the outer
 * held transaction. No native writes, SDK calls or restore admission here.
 * The curve evaluator is certified only for the owned Ki step-integer format;
 * other families/formats require their own complete read/write contracts.
 */
enum {EPT_RESOURCE=510,EPT_MODEL=511,EPT_CAPACITY=512,EPT_CURVE=513,EPT_ALIAS=514};
typedef struct {u32 header,bytes,model,model_wrapper,table,count;} EptResources;
typedef struct {u32 data,materials,textures,curves,max_frame,results,cache;} EptAnimation;
typedef struct {u32 magic,bytes,valid,error;EpjLayout layout;EptResources resources;EptAnimation animations[2];} EptPlan;
typedef char EptPlanSize[sizeof(EptPlan)==196?1:-1];

static u32 ept_relative(const EptResources*r,u32 field,u32 bytes,u32*out){
 return fpc_contract_relative(r->header,r->bytes,field,bytes,out);
}
static u32 ept_dictionary(const EptResources*r,u32 dictionary,u32*count){
 return fpc_contract_dictionary(r->header,r->bytes,dictionary,count);
}
static u32 ept_string(const EptResources*r,u32 value){return fpc_contract_string(r->header,r->bytes,value);}
static u32 ept_same_name(u32 a,u32 b){u32 i,n=R32(a-4,0);
 if(n!=R32(b-4,0))return 0;for(i=0;i<n;i++)if(R8(a,i)!=R8(b,i))return 0;return 1;
}
static u32 ept_named(const EptResources*r,u32 dictionary,u32 name,u32*target){u32 i,n,s,p;
 if(!dictionary||!ept_dictionary(r,dictionary,&n))return 0;
 for(i=0;i<n;i++){
  if(!ept_relative(r,dictionary+24+i*16+8,1,&s)||!s||!ept_string(r,s))return 0;
  if(ept_same_name(s,name)){
   if(!ept_relative(r,dictionary+24+i*16+12,4,&p)||!p)return 0;*target=p;return 1;
  }
 }
 return 0;
}
/* Read closure of native027DF584 for this owned format: float frame keys,
 * uint32 step results. No other key/curve format is implicitly admitted. */
static u32 ept_curve(const EptResources*r,u32 curve,u32 textures,u32 maxframe){u32 n,frames,values,i,bits,last=0;
 u32 exponent=0,upper=maxframe,maximum;
 if(!fpc_contract_in(r->header,r->bytes,curve,36)||R16(curve,0)!=0x40)return 0;
 n=R16(curve,2);if(!n||n>4096)return 0;
 while(upper>1){upper>>=1;exponent++;}
 maximum=((127+exponent)<<23)|((maxframe-(1u<<exponent))<<(23-exponent));
 for(i=8;i<=24;i+=4)if(!epj_finite(R32(curve,i)))return 0;
 if((R32(curve,8)|R32(curve,12))&0x80000000u||R32(curve,12)<R32(curve,8)||R32(curve,12)>maximum)return 0;
 if(!ept_relative(r,curve+28,n*4,&frames)||!frames||!ept_relative(r,curve+32,n*4,&values)||!values)return 0;
 for(i=0;i<n;i++){
  bits=R32(frames,i*4);
  /* Nonnegative IEEE frame keys are integer-orderable. The upper bound is
   * the exact float representation of maxframe, supplied from native s16. */
  if(!epj_finite(bits)||(bits&0x80000000u)||bits>maximum||(i&&bits<=last)||R32(values,i*4)>=textures)return 0;
  last=bits;
 }
 return 1;
}
static u32 ept_animation(const EptResources*r,const EpjLayout*l,u32 wrapper,EptAnimation*out){
 u32 data=R32(wrapper,12),n,textures,rows,i,row,tracks,start,name,material,dict,entries,j,s,curve_count,curves,cache_index,constants,tex,table,total=0,maxframe;
 if(!fpc_contract_root(r->header,r->bytes)||!fpc_contract_member(r->header,r->bytes,0x38,data)||
  !fpc_contract_in(r->header,r->bytes,data,0x34)||R32(data,0)!=0x46545850u)return EPT_RESOURCE;
 n=R16(data,0x16);textures=R16(data,0x14);maxframe=R16(data,0x12);
 if(!n||n>l->animation_capacity||!textures||textures>4096||!maxframe||maxframe>32767||
  R32(data,0x18)>l->result_capacity||R32(data,0x1c)>l->cache_capacity)return EPT_CAPACITY;
 if(!ept_relative(r,data+0x30,textures*8,&table)||!table)return EPT_RESOURCE;
 for(i=0;i<textures;i++){
  if(!ept_relative(r,table+8*i,1,&name)||!name||!ept_string(r,name)||
   !ept_relative(r,table+8*i+4,4,&tex)||!tex||R32(tex,0)!=0x46544558u||
   !fpc_contract_member(r->header,r->bytes,0x24,tex))return EPT_RESOURCE;
 }
 if(!ept_relative(r,data+0x2c,n*28,&rows)||!rows||
  !ept_relative(r,r->model+0x18,24,&dict)||!dict)return EPT_RESOURCE;
 for(i=0;i<n;i++){
  row=rows+i*28;tracks=R16(row,0);curve_count=R16(row,2);cache_index=R32(row,4);start=R32(row,8);
  if(!tracks||start>l->result_capacity||tracks>l->result_capacity-start||
   cache_index>l->cache_capacity||curve_count>l->cache_capacity-cache_index||
   cache_index>R32(data,0x1c)||curve_count>R32(data,0x1c)-cache_index)return EPT_CAPACITY;
  if(!ept_relative(r,row+12,1,&name)||!name||!ept_string(r,name)||!ept_named(r,dict,name,&material)||
   !fpc_contract_in(r->header,r->bytes,material,0x34)||R32(material,0)!=0x464d4154u)return EPT_MODEL;
  if(!ept_relative(r,material+0x30,24,&material)||!material||!ept_dictionary(r,material,&j))return EPT_MODEL;
  if(!ept_relative(r,row+16,tracks*8,&entries)||!entries||!ept_relative(r,row+24,tracks*2,&constants)||!constants)return EPT_RESOURCE;
  if(!ept_relative(r,row+20,curve_count*36,&curves)||(!curves&&curve_count))return EPT_RESOURCE;
  for(j=0;j<tracks;j++){
   s=R8(entries,j*8);
   if(s!=255&&s>=curve_count)return EPT_CURVE;
   if(R16(constants,j*2)>=textures)return EPT_CURVE;
   if(!ept_relative(r,entries+j*8+4,1,&name)||!name||!ept_string(r,name)||!ept_named(r,material,name,&tex))return EPT_MODEL;
  }
  for(j=0;j<curve_count;j++)if(!ept_curve(r,curves+j*36,textures,maxframe))return EPT_CURVE;
  total+=curve_count;
 }
 out->data=data;out->materials=n;out->textures=textures;out->curves=total;out->max_frame=maxframe;
 out->results=R32(data,0x18);out->cache=R32(data,0x1c);return 0;
}
u32 _gz_ept_contract(const EpjBounds*b,const EptResources*r,EptPlan*out){EptPlan p;u32 z,i,n,found=0;
 if(!b||!r||!out||!epj_descriptor((u32)r,sizeof(*r))||!epj_ram((u32)out,sizeof(*out)))return EPT_RESOURCE;
 z=_gz_epj_describe(b,&p.layout);if(z)return z;
 if(!epj_private(b,(u32)out,sizeof(*out))||!epj_apart((u32)out,sizeof(*out),(u32)b,sizeof(*b))||
  !epj_apart((u32)out,sizeof(*out),(u32)r,sizeof(*r)))return EPT_ALIAS;
 if(!epj_ram(r->header,r->bytes)||!fpc_contract_root(r->header,r->bytes)||!r->count||r->count>4096||
  !epj_ram(r->table,r->count*4)||!epj_ram(r->model_wrapper,4))return EPT_RESOURCE;
 if(!epj_apart((u32)out,sizeof(*out),r->header,r->bytes)||!epj_apart((u32)out,sizeof(*out),r->table,r->count*4)||
  !epj_apart((u32)out,sizeof(*out),r->model_wrapper,4)||!epj_apart(r->header,r->bytes,b->source,b->source_bytes)||
  !epj_apart(r->table,r->count*4,b->source,b->source_bytes)||!epj_apart(r->model_wrapper,4,b->source,b->source_bytes))return EPT_ALIAS;
 if(R32(b->model,0xac)!=r->model_wrapper||R32(r->model_wrapper,0)!=r->model||
  !fpc_contract_member(r->header,r->bytes,0x20,r->model)||
  !fpc_contract_in(r->header,r->bytes,r->model,0x28)||R32(r->model,0)!=0x464d444cu||
  R16(r->model,0x24)!=p.layout.materials)return EPT_MODEL;
 for(i=0;i<r->count;i++){
  n=R32(r->table,i*4);if(n==r->model_wrapper)found|=1;
  if(n==b->before_wrapper)found|=2;if(n==b->target_wrapper)found|=4;
 }
 if(found!=7)return EPT_RESOURCE;
 n=R32(b->face,16);i=R32(n,12);
 if(!fpc_contract_in(r->header,r->bytes,i,0x34)||R32(b->face+0x1c,0x44)!=i||
  !ept_relative(r,i+0x2c,1,&n)||!n||R32(b->face+0x1c,0x48)!=n)return EPT_RESOURCE;
 if(!ept_relative(r,r->model+0x18,24,&n)||!n||!ept_dictionary(r,n,&i)||i!=p.layout.materials)return EPT_MODEL;
 z=ept_animation(r,&p.layout,b->before_wrapper,&p.animations[0]);if(z)return z;
 z=ept_animation(r,&p.layout,b->target_wrapper,&p.animations[1]);if(z)return z;
 p.magic=0x45505431u;p.bytes=sizeof(p);p.valid=1;p.error=0;btr_copy(&p.resources,r,sizeof(*r));btr_copy(out,&p,sizeof(p));return 0;
}
