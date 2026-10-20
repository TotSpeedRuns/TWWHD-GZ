/* Full normal face-frame capture and read-only live binding observation.
 * No face replay, SDK setter, owner admission or normal world release is
 * added by this capture successor. Context is an opaque audit, never a
 * pointer to follow or a field admitted for restoration. */
#include "face_contract.c"
extern u8 _gz_fpc_native_pattern_resource_vtable[],_gz_fpc_native_scroll_resource_vtable[];
static const u16 fpc_object_offsets[]={0x644,0x69c,0x6f4};
u32 _gz_fpc_binding_report[64]={0x46434231u,256};
static FaceBounds _gz_fpc_observed_bounds;
static FacePlan _gz_fpc_observed_plans[3];
static GhIdentity _gz_fpc_observed_heaps[3];

/* Actual ResourceArchiveInfo+5C -> ResourceArchiveHD+14 -> BFRES header.
 * Every traversal starts from the current native resource controller. The
 * saved archive/model addresses in a snapshot are never followed. */
static u32 fpc_archive(u32 key,LpcArc*out,u32*container,u32*header,u32*bytes){
 u32 c=_gz_pr_resource_control,pool,storage,node,count,steps=0,i,info,table,seen[4]={0,0,0,0},p,h,n;
 if(!span(c,0x2130)||R32(c,0)!=(u32)_gz_lpc_native_res_control_vtable)return 401;
 pool=c+0x14;storage=c+0x28;count=R32(pool,12);
 if(R32(pool,8)!=storage||R32(pool,16)!=128||!count||count>128)return 401;
 node=R32(pool,0);
 while(node){
  if(node<storage||node>=storage+128*32||((node-storage)&31)||++steps>count)return 401;
  i=(node-storage)>>5;if(seen[i>>5]&(1u<<(i&31)))return 401;seen[i>>5]|=1u<<(i&31);
  if(R32(node,28)!=pool||R32(node,16)!=(u32)_gz_lpc_native_res_node_vtable||R8(node,8)>1||R32(node,12)!=R32(node,20))return 401;
  if(R32(node,12)==key){
   info=R32(node,24);
   if(!span(info,0x8c)||R32(info,12)!=(u32)_gz_lpc_native_res_info_vtable||R32(info,16)!=info+28||
    R32(info,20)!=(u32)_gz_lpc_native_res_string_vtable||R32(info,24)!=64)return 401;
   if(key==237211244u){if(R32(info,28)!=0x4c6b416eu||R16(info,32)!=0x6d00)return 401;}
   else if(key==0x969e36cfu){if(R32(info,28)!=0x4c696e6bu||R8(info,32))return 401;}
   else return 401;
   table=R32(info,96);count=R32(info,132);p=R32(info,92);
   if(!count||count>4096||!span(table,count*4)||!span(p,0x24))return 401;
   h=R32(p,20);if(!span(h,0x6c)||R32(h,0)!=0x46524553u||R32(h,4)!=0x03040001u)return 402;
   n=R32(h,12);if(n<0x6c||n>0x4000000u||!span(h,n))return 402;
   out->control=c;out->node=node;out->info=info;out->table=table;out->count=count;out->key=key;
   *container=p;*header=h;*bytes=n;return 0;
  }
  node=R32(node,R32(node,12)>key?0:4);
 }
 return 401;
}
static u32 fpc_index(u32 recipe,u32*index){u32 id;
 if(R16(recipe,4)!=65535||R16(recipe,6)!=65535)return 403;
 id=R16(recipe,2);if(id==65535)id=R16(recipe,0);if(id==65535)return 403;
 *index=id;return 0;
}
static u32 fpc_wrapper(const LpcArc*arc,u32 id,u32 kind,u32*wrapper,u32*data){u32 p;
 if(id>=arc->count)return 403;p=R32(arc->table,id*4);
 if(!span(p,16)||R32(p,4)!=(u32)(kind?_gz_fpc_native_pattern_resource_vtable:_gz_fpc_native_scroll_resource_vtable))return 403;
 *wrapper=p;*data=R32(p,12);return span(*data,0x30)?0:403;
}
static u32 fpc_observe(u32 actor){LpcArc anim,models,again;FaceBounds*b=&_gz_fpc_observed_bounds;
 u32 ac,ah,an,mc,mh,mn,cc,hh,nn,z,i,id,p,data,model,heap,model_index=BAD;
 _gz_fpc_binding_report[2]=0;_gz_fpc_binding_report[3]=0;
 _gz_fpc_binding_report[4]=_gz_pbac_frame;_gz_fpc_binding_report[5]=actor;_gz_fpc_binding_report[6]=R32(actor,4);
 z=fpc_archive(237211244u,&anim,&ac,&ah,&an);if(z)goto done;
 z=fpc_archive(0x969e36cfu,&models,&mc,&mh,&mn);if(z)goto done;
 p=R32(actor,0x444);if(!span(p,4)){z=404;goto done;}
 for(i=0;i<models.count;i++)if(R32(models.table,i*4)==p){model_index=i;break;}
 if(model_index==BAD){z=404;goto done;}model=R32(p,0);heap=R32(actor,0xf4);
 b->actor=actor;b->actor_bytes=0x8284;b->heap=heap;b->animation=ah;b->animation_bytes=an;
 b->model_file=mh;b->model_bytes=mn;b->model=model;
 for(i=0;i<3;i++){
  u32 o=actor+fpc_object_offsets[i],buf=R32(o,0x30);
  if(_gz_gh_solid(heap,buf,8192)){z=405;goto done;}
  btr_copy(&_gz_fpc_observed_heaps[i],(const void*)&_gz_gh_identity,sizeof(GhIdentity));
  if(i&&!btr_equal(&_gz_fpc_observed_heaps[0],&_gz_fpc_observed_heaps[i],40)){z=405;goto done;}
  if(_gz_fpc_observed_heaps[i].model!=buf||_gz_fpc_observed_heaps[i].model_bytes!=8192){z=405;goto done;}
  {u32 j;for(j=0;j<i;j++)if(buf<_gz_fpc_observed_heaps[j].model+8192&&_gz_fpc_observed_heaps[j].model<buf+8192){z=405;goto done;}}
  b->head=_gz_fpc_observed_heaps[i].head;b->tail=_gz_fpc_observed_heaps[i].tail;
  b->end=_gz_fpc_observed_heaps[i].start+_gz_fpc_observed_heaps[i].bytes;
  z=fpc_index(actor+(i<2?0x65d0:0x65e0),&id);if(z)goto done;
  z=fpc_wrapper(&anim,id,i<2,&p,&data);if(z)goto done;
  if(R32(actor,i<2?0x5f4:0x5fc)!=p||R32(o,0x44)!=data){z=403;goto done;}
  z=_gz_face_contract(b,o,data,i<2,&_gz_fpc_observed_plans[i]);_gz_fpc_binding_report[23+i]=z;if(z)goto done;
  _gz_fpc_binding_report[26+i]=buf;
 }
 if(R32(actor,0x444)!=R32(models.table,model_index*4)||R32(actor,0xf4)!=heap){z=406;goto done;}
 z=fpc_archive(237211244u,&again,&cc,&hh,&nn);if(z)goto done;
 if(!btr_equal(&anim,&again,sizeof(anim))||cc!=ac||hh!=ah||nn!=an){z=406;goto done;}
 z=fpc_archive(0x969e36cfu,&again,&cc,&hh,&nn);if(z)goto done;
 if(!btr_equal(&models,&again,sizeof(models))||cc!=mc||hh!=mh||nn!=mn){z=406;goto done;}
 _gz_fpc_binding_report[7]=heap;_gz_fpc_binding_report[8]=anim.info;_gz_fpc_binding_report[9]=ac;
 _gz_fpc_binding_report[10]=ah;_gz_fpc_binding_report[11]=an;_gz_fpc_binding_report[12]=anim.table;_gz_fpc_binding_report[13]=anim.count;
 _gz_fpc_binding_report[14]=models.info;_gz_fpc_binding_report[15]=mc;_gz_fpc_binding_report[16]=mh;_gz_fpc_binding_report[17]=mn;
 _gz_fpc_binding_report[18]=models.table;_gz_fpc_binding_report[19]=models.count;_gz_fpc_binding_report[20]=R32(actor,0x444);
 _gz_fpc_binding_report[21]=model_index;_gz_fpc_binding_report[22]=model;_gz_fpc_binding_report[2]=1;
done:_gz_fpc_binding_report[3]=z;return z;
}

/* 80-byte FCR2 extension: header8 + three {four scalar floats,
 * canonical loop/clamp selector, opaque context audit}. Context is not a
 * replay permission and never gets dereferenced. Marker publishes last. */
u32 _gz_fpc_capture_record(u32 actor,u8*record){u32 i,j,o,fn,mode,k;
 *(u32*)(record+4704)=0;*(u32*)(record+4708)=80;
 for(i=0;i<3;i++){
  o=actor+fpc_object_offsets[i];k=4712+i*24;
  if(R32(o,0)!=o+4||R32(o,0x34)!=(u32)(i<2?_gz_face_native_pattern_vtable:_gz_face_native_scroll_vtable))return 0;
  fn=R32(o,0x14);
  if(fn==(u32)_gz_face_native_frame_clamp)mode=0;
  else if(fn==(u32)_gz_face_native_frame_loop)mode=1;else return 0;
  if(!btr_equal((const void*)(o+4),record+4528+i*12,12))return 0;
  for(j=0;j<16;j+=4){u32 v=R32(o,4+j);if((v&0x7fffffffu)>=0x7f800000u)return 0;*(u32*)(record+k+j)=v;}
  *(u32*)(record+k+16)=mode;*(u32*)(record+k+20)=R32(o,0x18);
 }
 *(u32*)(record+4704)=0x46435232u;
 /* This observation intentionally does not gate existing capture/replay or
  * confer SDK authority. Its exact native failure remains visible. */
 (void)fpc_observe(actor);return 1;
}
