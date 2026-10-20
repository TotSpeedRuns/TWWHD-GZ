/* Enemy animation lookup from CURRENT resident archives. Saved actor, model,
 * morph and animation pointers are opaque audits: none is dereferenced here.
 * This is a READ participant, not enemy replay or lifetime reconstruction.
 * This reader inspects20-byte skeletal and16-byte texture-pattern prefixes.
 * Pattern backend/tail state beyond that prefix is NOT certified or replayed.
 * Owned native virtual bind027F3D6C writes wrapper+8/+16 and allocates8 bytes
 * per material. Calling it needs its own shared-resource/allocation journal;
 * resolving a resource here does not authorize that SDK operation.
 */
extern u8 _gz_ern_native_body_vtable[],_gz_ern_native_body_name_vtable[];
extern u8 _gz_fpc_native_pattern_resource_vtable[];

enum {ERN_BODY=1,ERN_PATTERN=2,ERN_CONTEXT=460,ERN_FAMILY=461,
 ERN_CONTROL=462,ERN_ARCHIVE=463,ERN_RESOURCE=464,ERN_CHANGED=465,ERN_ALIAS=466};
typedef struct {
 u32 magic,bytes,version,valid,token,frame,profile,kind,index,key;
 u32 control,node,info,table,count,container,header,header_bytes;
 u32 wrapper,wrapper_bytes,data,frame_audit,identity[4];
 u32 reserved[6];
} ErnResource;
typedef char ErnResourceSize[sizeof(ErnResource)==128?1:-1];
static ErnResource _gz_ern_resolved;
u32 _gz_ern_report[64]={0x45524e31u,256};

static u32 ern_apart(u32 a,u32 n,u32 b,u32 m){return a>=b?a-b>=m:b-a>=n;}
static u32 ern_output(u32 p,u32 n){return ern_apart(p,n,(u32)&_gz_ern_resolved,sizeof(_gz_ern_resolved))&&
 ern_apart(p,n,(u32)_gz_ern_report,sizeof(_gz_ern_report));}
static u32 ern_span(u32 p,u32 n){return span(p,n)&&p>=0x10000000u&&ern_output(p,n)&&
 ern_apart(p,n,_gz_hbp_recipe.source,0x200000u);}
static u32 ern_inside(u32 p,u32 n,u32 base,u32 bytes){return p>=base&&p-base<=bytes&&n<=bytes-(p-base);}
/* The caller must be executing the real typed enemy service in this frame.
 * No host-selected address or unowned standalone query grants native reads. */
u32 _gz_ern_scope(u32 token,u32 frame){return token&&frame==_gz_pbac_frame&&_gz_roh_owner[12]==3&&
 _gz_bmh_scope(token,frame)&&!_gz_roh_replay_common(token,frame,1)&&
 !_gz_boh_service_validate(token)&&_gz_hbp_service_read(token,frame)!=0;}
static u32 ern_family(u32 profile,u32*key,u32*name){
 if(profile==188){*key=0xadb1e169u;*name=0x4d6f3200u;return 0;}
 if(profile==189){*key=0x59f687a8u;*name=0x426b0000u;return 0;}
 if(profile==206){*key=0x4e363edbu;*name=0x43630000u;return 0;}
 if(profile==215){*key=0x663a5dcdu;*name=0x4b690000u;return 0;}
 return ERN_FAMILY;
}
static u32 ern_archive(ErnResource*out,u32 name){
 u32 c=_gz_pr_resource_control,pool,storage,node,count,steps=0,i,key,info,table,p,h,n;
 u32 seen[4]={0,0,0,0},lo=0,hi=0xffffffffu;
 if(!ern_span(c,0x2130)||R32(c,0)!=(u32)_gz_lpc_native_res_control_vtable)return ERN_CONTROL;
 pool=c+20;storage=c+40;count=R32(pool,12);
 if(R32(pool,8)!=storage||R32(pool,16)!=128||!count||count>128)return ERN_CONTROL;
 node=R32(pool,0);
 while(node){
  if(node<storage||node>=storage+128*32||((node-storage)&31)||++steps>count)return ERN_CONTROL;
  i=(node-storage)>>5;if(seen[i>>5]&(1u<<(i&31)))return ERN_CONTROL;seen[i>>5]|=1u<<(i&31);
  if(R32(node,28)!=pool||R32(node,16)!=(u32)_gz_lpc_native_res_node_vtable||R8(node,8)>1)return ERN_CONTROL;
  key=R32(node,12);if(key!=R32(node,20)||key<lo||key>hi)return ERN_CONTROL;
  if(key==out->key){
   info=R32(node,24);
   if(!ern_span(info,0x8c)||R32(info,12)!=(u32)_gz_lpc_native_res_info_vtable||
    R32(info,16)!=info+28||R32(info,20)!=(u32)_gz_lpc_native_res_string_vtable||
    R32(info,24)!=64)return ERN_ARCHIVE;
   if(name&0xff00u){if(R32(info,28)!=name)return ERN_ARCHIVE;}
   else if(R16(info,28)!=(name>>16)||R8(info,30))return ERN_ARCHIVE;
   table=R32(info,96);n=R32(info,132);p=R32(info,92);
   if(!n||n>4096||!ern_span(table,n*4)||!ern_span(p,0x24))return ERN_ARCHIVE;
   h=R32(p,20);
   if(!ern_span(h,0x6c)||R32(h,0)!=0x46524553u||R32(h,4)!=0x03040001u)return ERN_ARCHIVE;
   if(R32(h,12)<0x6c||R32(h,12)>0x4000000u||!ern_span(h,R32(h,12)))return ERN_ARCHIVE;
   out->control=c;out->node=node;out->info=info;out->table=table;out->count=n;
   out->container=p;out->header=h;out->header_bytes=R32(h,12);return 0;
  }
  if(key>out->key){if(!key)return ERN_CONTROL;hi=key-1;node=R32(node,0);}
  else{if(key==0xffffffffu)return ERN_CONTROL;lo=key+1;node=R32(node,4);}
 }
 return ERN_ARCHIVE;
}
static u32 ern_wrapper(ErnResource*out){u32 p,n,data,i;
 if(out->index>=out->count||out->index>511)return ERN_RESOURCE;
 p=R32(out->table,out->index*4);n=out->kind==ERN_BODY?20:16;
 if(!ern_span(p,n)||(R32(p,0)&0x7fffffffu)>=0x7f800000u)return ERN_RESOURCE;
 if(out->kind==ERN_BODY){
  if(R32(p,4)!=(u32)_gz_ern_native_body_vtable||R32(p,16)!=(u32)_gz_ern_native_body_name_vtable)return ERN_RESOURCE;
  data=R32(p,8);
 }else{
  if(R32(p,4)!=(u32)_gz_fpc_native_pattern_resource_vtable)return ERN_RESOURCE;
  data=R32(p,12);
 }
 if(!ern_inside(data,0x30,out->header,out->header_bytes)||!ern_span(data,0x30))return ERN_RESOURCE;
 out->wrapper=p;out->wrapper_bytes=n;out->data=data;out->frame_audit=R32(p,0);
 for(i=0;i<(n-4)/4;i++)out->identity[i]=R32(p,4+i*4);
 return 0;
}
u32 _gz_ern_resolve(u32 token,u32 profile,u32 id,u32 kind){
 ErnResource local,again;u32 frame=_gz_pbac_frame,z,i,name;
 if(!_gz_ern_scope(token,frame))return ERN_CONTEXT;
 if(!span(_gz_hbp_recipe.source,0x200000u)||!ern_output(_gz_hbp_recipe.source,0x200000u))return ERN_ALIAS;
 _gz_ern_resolved.valid=0;
 if(kind!=ERN_BODY&&kind!=ERN_PATTERN)return ERN_RESOURCE;
 for(i=0;i<sizeof(local)/4;i++)((u32*)&local)[i]=0;
 local.magic=0x45524e31u;local.bytes=sizeof(local);local.version=1;local.token=token;local.frame=frame;
 local.profile=profile;local.kind=kind;local.index=id;
 z=ern_family(profile,&local.key,&name);if(z)return z;
 z=ern_archive(&local,name);if(z)return z;
 z=ern_wrapper(&local);if(z)return z;
 if(!_gz_ern_scope(token,frame))return ERN_CHANGED;
 btr_copy(&again,&local,sizeof(local));z=ern_archive(&again,name);if(z)return z;z=ern_wrapper(&again);if(z)return z;
 if(!btr_equal(&local,&again,sizeof(local)))return ERN_CHANGED;
 btr_copy(&_gz_ern_resolved,&local,sizeof(local));
 __asm__ volatile("sync":::"memory");_gz_ern_resolved.valid=1;return 0;
}
/* Resolve saved Keese/Moblin body AND rendered face IDs. Historical pointers
 * at row4808/4812 remain audits. Other layouts need their explicit recipes;
 * this read-only service does not silently certify them as restored. Neither
 * this receipt nor _gz_ern_resolved grants permission to write an enemy. */
u32 _gz_ern_service(u32 token){u32 s,n,c,b,count,i,row,profile,id,kind,z,h=2166136261u;
 static const u16 mo2_face[]={0x82,0x83,0x84,0x86,0x87,0x88,0x85};
 if(!_gz_ern_scope(token,_gz_pbac_frame))return ERN_CONTEXT;
 s=_gz_hbp_recipe.source;n=_gz_hbp_recipe.used;
 if(!span(s,n)||n<232||R32(s,8)!=27||R32(s,12)!=27)return 42;
 if(!ern_output(s,0x200000u))return ERN_ALIAS;
 _gz_ern_report[2]=token;_gz_ern_report[3]=0;_gz_ern_report[4]=_gz_pbac_frame;
 _gz_ern_report[5]=0;_gz_ern_report[6]=0;_gz_ern_report[7]=0;
 c=R32(s,88);b=R32(s,96);
 if(c<232||c>n||4>n-c||R32(s,92)!=4||b!=c+4||b>n)return 42;
 count=R32(s+c,0);if(count>96||R32(s,100)!=count*10240||count*10240>n-b)return 42;
 _gz_ern_report[5]=count;
 for(i=0;i<count;i++){
  row=s+b+i*10240;profile=R32(row,52);_gz_ern_report[8]=i;_gz_ern_report[9]=profile;
  if((profile!=188&&profile!=215)||R32(row,4800)!=1){_gz_ern_report[3]=ERN_FAMILY;return ERN_FAMILY;}
  for(kind=ERN_BODY;kind<=ERN_PATTERN;kind++){
   if(kind==ERN_BODY)id=R32(row,4832);
   else if(profile==215){id=R32(row,4816);if(id<30||id>37)return ERN_RESOURCE;}
   else{id=R32(row,4852);if(id>6)return ERN_RESOURCE;id=mo2_face[id];}
   z=_gz_ern_resolve(token,profile,id,kind);_gz_ern_report[3]=z;if(z)return z;
   h=(h^btr_hash(&_gz_ern_resolved,sizeof(_gz_ern_resolved)))*16777619u;
   _gz_ern_report[6]++;
  }
 }
 _gz_ern_report[7]=h;
 return _gz_ern_scope(token,_gz_pbac_frame)?0:ERN_CHANGED;
}
