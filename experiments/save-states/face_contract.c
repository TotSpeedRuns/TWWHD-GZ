/* Current native face SDK read/write contract. No game stores or SDK calls.
 * The owning transaction must independently certify process generation,
 * resident archive lifetimes and the registered solid-heap allocated extent.
 * These explicit current bounds are not a substitute for owner authority.
 * No saved address is followed. Output is a private proposed undo mask. */
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
#define U32(p,o) (*(volatile const u32*)((p)+(o)))
#define U16(p,o) (*(volatile const u16*)((p)+(o)))
#define U8(p,o) (*(volatile const u8*)((p)+(o)))
extern u8 _gz_face_native_pattern_vtable[],_gz_face_native_scroll_vtable[];
extern u8 _gz_face_native_frame_clamp[],_gz_face_native_frame_loop[];

typedef struct {
 u32 actor,actor_bytes,heap,head,tail,end;
 u32 animation,animation_bytes,model_file,model_bytes,model;
} FaceBounds;
typedef struct {u32 base,count,stride,width;} FaceRange;
typedef struct {
 u32 magic,bytes,valid,error,object,pattern,resource,model;
 u32 buffer,buffer_bytes,cache_count,material_count,range_count,reserved[3];
 FaceRange ranges[8];
} FacePlan;
typedef char FacePlanSize[sizeof(FacePlan)==192?1:-1];
enum {FC_BOUNDS=241,FC_OBJECT=242,FC_RESOURCE=243,FC_CAPACITY=244,FC_MODEL=245};

static u32 fpc_contract_whole(u32 p,u32 n){return p>=0x10000000u&&p<0xc0000000u&&n&&n<=0xc0000000u-p;}
static u32 fpc_contract_in(u32 base,u32 bytes,u32 p,u32 n){return fpc_contract_whole(base,bytes)&&p>=base&&p-base<=bytes&&n<=bytes-(p-base);}
static u32 fpc_contract_allocated(const FaceBounds*b,u32 p,u32 n){
 return fpc_contract_in(b->heap,b->end-b->heap,p,n)&&
 ((p>=b->heap+0x9c&&p<=b->head&&n<=b->head-p)||(p>=b->tail&&p<=b->end&&n<=b->end-p));
}
static u32 fpc_contract_relative(u32 base,u32 bytes,u32 field,u32 n,u32*out){u32 d,p;
 if(!fpc_contract_in(base,bytes,field,4)||field&3)return 0;
 d=U32(field,0);if(!d){*out=0;return 1;}
 p=field+d;if(!fpc_contract_in(base,bytes,p,n))return 0;*out=p;return 1;
}
static u32 fpc_contract_string(u32 base,u32 bytes,u32 p){u32 i,n;
 if((p&3)||!fpc_contract_in(base,bytes,p,1)||p<base+4||!fpc_contract_in(base,bytes,p-4,4))return 0;
 n=U32(p-4,0);if(n>1024||!fpc_contract_in(base,bytes,p,n+1))return 0;
 for(i=0;i<n;i++)if(!U8(p,i))return 0;
 return !U8(p,n);
}
static u32 fpc_contract_dictionary(u32 base,u32 bytes,u32 p,u32*count){u32 n,i,node,name;
 if(!p){*count=0;return 1;}
 if(p&3||!fpc_contract_in(base,bytes,p,24))return 0;
 n=U32(p,4);if(n>4096||!fpc_contract_in(base,bytes,p,24+n*16)||U32(p,0)!=24+n*16)return 0;
 for(i=0;i<=n;i++){
  node=p+8+i*16;
  if(U16(node,4)>n||U16(node,6)>n)return 0;
  if(i){if(!fpc_contract_relative(base,bytes,node+8,1,&name)||!name||!fpc_contract_string(base,bytes,name))return 0;}
 }
 *count=n;return 1;
}
static u32 fpc_contract_root(u32 base,u32 bytes){
 return !(base&3)&&fpc_contract_in(base,bytes,base,0x6c)&&U32(base,0)==0x46524553u&&
 U32(base,4)==0x03040001u&&U32(base,12)==bytes;
}
static u32 fpc_contract_member(u32 base,u32 bytes,u32 group,u32 target){u32 d,n,i,p;
 if(!fpc_contract_relative(base,bytes,base+group,24,&d)||!d||!fpc_contract_dictionary(base,bytes,d,&n))return 0;
 for(i=0;i<n;i++){
  if(!fpc_contract_relative(base,bytes,d+24+i*16+12,0x30,&p)||(p&3))return 0;
  if(p==target)return 1;
 }
 return 0;
}
static u32 fpc_contract_model(const FaceBounds*b,u32 pattern){u32 d,n,i,p,names,k;
 if(!fpc_contract_root(b->model_file,b->model_bytes)||!fpc_contract_member(b->model_file,b->model_bytes,0x20,b->model)||
 !fpc_contract_in(b->model_file,b->model_bytes,b->model,0x28)||U32(b->model,0)!=0x464d444cu||U16(b->model,0x24)!=24)return 0;
 if(!fpc_contract_relative(b->model_file,b->model_bytes,b->model+0x18,24,&d)||!d||!fpc_contract_dictionary(b->model_file,b->model_bytes,d,&n)||n!=24)return 0;
 for(i=0;i<n;i++){
  if(!fpc_contract_relative(b->model_file,b->model_bytes,d+24+i*16+12,0x3c,&p)||!p||U32(p,0)!=0x464d4154u)return 0;
  if(!fpc_contract_relative(b->model_file,b->model_bytes,p+(pattern?0x30:0x38),24,&names)||!fpc_contract_dictionary(b->model_file,b->model_bytes,names,&k))return 0;
 }
 return 1;
}
static u32 fpc_contract_resource(const FaceBounds*b,u32 r,u32 pattern,u32*count){u32 n,rows,i,row,start,tracks,values,name,j,entries;
 if(!fpc_contract_root(b->animation,b->animation_bytes)||!fpc_contract_member(b->animation,b->animation_bytes,pattern?0x38:0x34,r)||
 !fpc_contract_in(b->animation,b->animation_bytes,r,0x30)||U32(r,0)!=(pattern?0x46545850u:0x46534855u))return FC_RESOURCE;
 n=U16(r,pattern?0x16:0x14);
 if(!n||n>(pattern?5:2)||U32(r,0x1c)>380)return FC_CAPACITY;
 if(!fpc_contract_relative(b->animation,b->animation_bytes,r+0x2c,n*(pattern?28:32),&rows)||!rows)return FC_RESOURCE;
 for(i=0;i<n;i++){
  row=rows+i*(pattern?28:32);tracks=U16(row,0);start=U32(row,pattern?8:12);
  if(start>(pattern?5:16)||tracks>(pattern?5:16)-start)return FC_CAPACITY;
  if(!fpc_contract_relative(b->animation,b->animation_bytes,row+(pattern?12:16),1,&name)||!name||!fpc_contract_string(b->animation,b->animation_bytes,name))return FC_RESOURCE;
  if(!fpc_contract_relative(b->animation,b->animation_bytes,row+(pattern?16:20),tracks*(pattern?8:16),&entries)||(!entries&&tracks))return FC_RESOURCE;
  for(j=0;j<tracks;j++){
   if(!fpc_contract_relative(b->animation,b->animation_bytes,entries+j*(pattern?8:16)+(pattern?4:12),1,&name)||!name||!fpc_contract_string(b->animation,b->animation_bytes,name))return FC_RESOURCE;
  }
  if(pattern&&(!fpc_contract_relative(b->animation,b->animation_bytes,row+24,tracks*2,&values)||(!values&&tracks)))return FC_RESOURCE;
 }
 *count=n;return 0;
}
static void fpc_contract_range(FacePlan*p,u32 a,u32 n,u32 stride,u32 width){FaceRange*r=&p->ranges[p->range_count++];r->base=a;r->count=n;r->stride=stride;r->width=width;}
u32 _gz_face_contract(const FaceBounds*b,u32 object,u32 target,u32 pattern,FacePlan*out){
 u32 z,i,j,buf,cache,binding,mapping,count,fn;FacePlan p;
 if(!b||!out)return FC_BOUNDS;
 for(i=0;i<sizeof(p);i++)((volatile u8*)&p)[i]=0;
 p.magic=0x46435031u;p.bytes=sizeof(p);p.object=object;p.pattern=pattern;p.resource=target;p.model=b->model;
 z=FC_BOUNDS;
 if(!fpc_contract_whole(b->actor,b->actor_bytes)||b->actor_bytes<0x8284||b->actor_bytes>0x100000||
 !fpc_contract_whole(b->heap,b->end-b->heap)||b->heap+0x9c>b->head||b->head>b->tail||b->tail>b->end||U32(b->actor,0xf4)!=b->heap)goto done;
 z=FC_OBJECT;
 if(pattern>1||(!pattern&&object!=b->actor+0x6f4)||
 (pattern&&object!=b->actor+0x644&&object!=b->actor+0x69c)||U32(object,0)!=object+4||
 U32(object,0x34)!=(u32)(pattern?_gz_face_native_pattern_vtable:_gz_face_native_scroll_vtable))goto done;
 fn=U32(object,0x14);
 if(fn!=(u32)_gz_face_native_frame_clamp&&fn!=(u32)_gz_face_native_frame_loop)goto done;
 for(i=4;i<=16;i+=4)if((U32(object,i)&0x7fffffffu)>=0x7f800000u)goto done;
 z=FC_CAPACITY;
 if(U32(object,0x20)!=380||U16(object,0x3e)!=24||U16(object,0x42)!=24||
 U32(object,0x4c)!=(pattern?5:2)||U32(object,0x50)!=(pattern?5:16))goto done;
 z=FC_BOUNDS;buf=U32(object,0x30);cache=U32(object,0x1c);binding=U32(object,0x38);mapping=U32(object,0x54);
 if(buf&3||!fpc_contract_allocated(b,buf,8192)||!fpc_contract_in(buf,8192,cache,380*12)||cache&3||!fpc_contract_in(buf,8192,binding,96)||binding&3||
 !fpc_contract_in(buf,8192,mapping,pattern?5:32)||(!pattern&&mapping&1))goto done;
 p.buffer=buf;p.buffer_bytes=8192;p.cache_count=380;
 fpc_contract_range(&p,object+4,1,24,24);fpc_contract_range(&p,object+0x24,1,8,8);fpc_contract_range(&p,object+0x3c,1,2,2);fpc_contract_range(&p,object+0x40,1,12,12);
 fpc_contract_range(&p,binding,24,4,4);fpc_contract_range(&p,mapping,pattern?5:16,pattern?1:2,pattern?1:2);
 if(pattern){u32 results=U32(object,0x2c);if(results&1||!fpc_contract_in(buf,8192,results,10))goto done;fpc_contract_range(&p,results,5,2,2);}
 fpc_contract_range(&p,cache,380,12,4);
 /* Allocations and semantic regions must be distinct, including interleaved
  * caches whose two untouched words are intentionally absent from undo. */
 for(i=0;i<p.range_count;i++)for(j=0;j<i;j++){
  const FaceRange*a=&p.ranges[i],*c=&p.ranges[j];
  if(a->base<c->base+(c->count-1)*c->stride+c->width&&c->base<a->base+(a->count-1)*a->stride+a->width)goto done;
 }
 z=FC_MODEL;if(!fpc_contract_model(b,pattern))goto done;
 z=fpc_contract_resource(b,target,pattern,&count);if(z)goto done;
 p.material_count=count;p.valid=1;
done:p.error=z;for(i=0;i<sizeof(p);i++)((volatile u8*)out)[i]=((const volatile u8*)&p)[i];return z;
}
