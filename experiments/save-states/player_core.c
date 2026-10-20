/* Typed Link body/track/old-pose participant, inside the held transaction.
 * Item children, actor keeps, face/material animation, sound and UI have
 * separate participants. This does not admit or publish a full restore. */
#include "heap_owner.h"
#include "correspondence.h"
typedef struct {u16 offset,bytes;} LpcField;
static const LpcField lpc_fields[]={
 {0x300,12},{0x314,12},{0x322,12},{0x33c,12},{0x370,4},{0x3ac,24},
 {0x65f0,4},{0x6916,32},{0x693c,52},{0x6972,60},{0x69b0,6},
 {0x69c0,24},{0x69f0,120},{0x6a70,16},{0x6a98,72},{0x7290,84},
 {0x72f0,108},{0x68dc,2},{0x3d0,6},{0,0}};
u32 _gz_lpc_field_count(void){return sizeof(lpc_fields)/sizeof(lpc_fields[0])-1;}
u32 _gz_lpc_field_word(u32 i){return i<_gz_lpc_field_count()?((u32)lpc_fields[i].offset<<16)|lpc_fields[i].bytes:0;}
typedef struct {u32 control,node,info,table,count,key;} LpcArc;
typedef struct {
 u32 magic,bytes,version,valid,token,frame,source,used,pin,actor,pid,heap,source_hash,before_hash;
 PoEntry process;u32 targets[8],spans[8];GhIdentity heaps[8];LpcArc archive;
 u32 before_bind[5],saved_bind[5];u8 before[8192],saved[8192];
 /* Private current-role audits. These are never copied into a saved archive
  * or used as native addresses. GameOver/demo/event replay is separate. */
 u32 before_role,before_gameover_ID,before_demo,before_event;
} LpcPlan;
typedef char LpcPlanSize[sizeof(LpcPlan)==17032?1:-1];
typedef struct {u32 magic,bytes,valid,phase,token,applied_frame,rolled_frame,error,plan_seal;} LpcRun;
enum {LPC_PREPARED=1,LPC_APPLYING=2,LPC_APPLIED=3,LPC_UNDOING=4,LPC_ROLLED=5,
 LPC_SOURCE=231,LPC_PROCESS=232,LPC_RESOURCE=233,LPC_HEAP=234,LPC_VALUE=235,LPC_CHANGED=236};
static LpcPlan _gz_lpc_plan;static LpcRun _gz_lpc_run;
static u32 _gz_lpc_seal,_gz_lpc_run_seal,_gz_lpc_entered;
u32 _gz_lpc_report[32]={0x4c504331u,128};
extern volatile u32 _gz_pr_resource_control;
extern u8 _gz_lpc_native_res_control_vtable[];
extern u8 _gz_lpc_native_res_node_vtable[];
extern u8 _gz_lpc_native_res_info_vtable[];
extern u8 _gz_lpc_native_res_string_vtable[];
extern u8 _gz_lpc_native_procedure_table[];
extern u8 _gz_lpc_native_sound_string_vtable[];
extern u32 _gz_doh_player_scope(u32,u32,u32,u32);

extern u32 _gz_bcb_archive_validate(u32,u32,u32);
static u32 lpc_finite(u32 x){return (x&0x7fffffffu)<0x7f800000u;}
static void lpc_run_seal(void){_gz_lpc_run_seal=btr_hash(&_gz_lpc_run,sizeof(LpcRun));}
static u32 lpc_sealed(void){const LpcPlan*p=&_gz_lpc_plan;const LpcRun*r=&_gz_lpc_run;
 return p->magic==0x4c504331u&&p->bytes==sizeof(*p)&&p->version==2&&p->valid==1&&p->token&&p->before_role<=1&&
  (!p->before_role||(R32(p->before,40)==178&&R32(p->before,44)==65535&&R32(p->before,48)==0x02422108u&&
   p->before_gameover_ID!=BAD&&p->before_demo==5&&p->before_event==3))&&
  r->magic==0x4c505231u&&r->bytes==sizeof(*r)&&r->valid==1&&r->token==p->token&&r->phase>=1&&r->phase<=5&&
  r->plan_seal==_gz_lpc_seal&&_gz_lpc_seal==btr_hash(p,sizeof(*p))&&_gz_lpc_run_seal==btr_hash(r,sizeof(*r));
}
/* Exact current resident archive topology, as used by native23DE720. No saved
 * resource pointer is followed. Normal body lookup has no heap reconstruction. */
static u32 lpc_archive(LpcArc*out){u32 c=_gz_pr_resource_control,pool,storage,node,count,steps=0,key,info,table,j;u32 seen[4]={0,0,0,0};
 if(!span(c,0x2130)||R32(c,0)!=(u32)_gz_lpc_native_res_control_vtable)return LPC_RESOURCE;
 pool=c+0x14;storage=c+0x28;count=R32(pool,12);
 if(R32(pool,8)!=storage||R32(pool,16)!=128||!count||count>128)return LPC_RESOURCE;
 node=R32(pool,0);
 while(node){
  if(node<storage||node>=storage+128*32||((node-storage)&31)||++steps>count)return LPC_RESOURCE;
  j=(node-storage)>>5;if(seen[j>>5]&(1u<<(j&31)))return LPC_RESOURCE;seen[j>>5]|=1u<<(j&31);
  if(R32(node,28)!=pool||R32(node,16)!=(u32)_gz_lpc_native_res_node_vtable||R8(node,8)>1)return LPC_RESOURCE;
  key=R32(node,12);if(R32(node,20)!=key)return LPC_RESOURCE;
  if(key==237211244u){
   info=R32(node,24);
   if(!span(info,0x8c)||R32(info,12)!=(u32)_gz_lpc_native_res_info_vtable||R32(info,16)!=info+28||R32(info,20)!=(u32)_gz_lpc_native_res_string_vtable||R32(info,24)!=64)return LPC_RESOURCE;
   if(R32(info,28)!=0x4c6b416eu||R16(info,32)!=0x6d00)return LPC_RESOURCE;
   table=R32(info,96);count=R32(info,132);
   if(!span(R32(info,92),0x24)||!count||count>4096||!span(table,count*4))return LPC_RESOURCE;
   out->control=c;out->node=node;out->info=info;out->table=table;out->count=count;out->key=key;return 0;
  }
  node=R32(node,key>237211244u?0:4);
 }
 return LPC_RESOURCE;
}
static u32 lpc_resource(const LpcArc*c,u32 id,u32*out){u32 p;
 if(id==BAD){*out=0;return 0;}
 if(id>=c->count)return LPC_RESOURCE;p=R32(c->table,id*4);
 if(!span(p,20))return LPC_RESOURCE;*out=p;return 0;
}
static u32 lpc_process(PoEntry*out){u32 a=_gz_pbac_player,q=_gz_pba_native_actor_queue,left=4096,n=0,p;const PoEntry*e;
 while(q){if(!left--||!span(q,16))return LPC_PROCESS;p=R32(q,12);if(p==a)n++;q=R32(q,8);}
 if(n!=1||!span(a,0x8284)||_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return LPC_PROCESS;
 e=&_gz_po_observation.entry;
 if(e->object!=a||e->state!=PO_LIVE||!e->generation||e->flags||e->end_frame||e->allocation_bytes<0x8284||e->allocation_bytes>0x100000||
  e->base_bytes+e->other_bytes!=e->allocation_bytes||e->base_bytes+e->other_bytes<e->base_bytes||!span(a,e->allocation_bytes)||
  e->pid!=R32(a,4)||e->process_name!=168||e->profile>=484||R16(a,8)!=168||R8(a,12)!=2||R16(a,14)!=e->profile||R32(a,16)!=e->profile_entry)return LPC_PROCESS;
 btr_copy(out,e,sizeof(*out));return 0;
}
static u32 lpc_procedure(u32 id,u32 sentinel,u32 function){u32 t=(u32)_gz_lpc_native_procedure_table;
 return id<=168&&sentinel==65535&&R32(t+12*id,0)==65535&&R32(t+12*id,4)==function;
}
/* The saved role above remains<=168. Only an independently authenticated
 * CURRENT dying actor can produce this private before-image role. */
static u32 lpc_current_procedure(LpcPlan*p){u32 a=p->actor,id=R32(a,0x65f0),s=R32(a,0x65f4),f=R32(a,0x65f8),t=(u32)_gz_lpc_native_procedure_table;
 p->before_role=0;p->before_gameover_ID=BAD;p->before_demo=p->before_event=0;
 if(lpc_procedure(id,s,f))return 0;
 if(id!=178||s!=65535||f!=0x02422108u||R32(t+178*12,0)!=65535||R32(t+178*12,4)!=f||
  !_gz_doh_player_scope(p->token,_gz_pbac_frame,a,p->pid))return LPC_SOURCE;
 p->before_role=1;p->before_gameover_ID=R32(a,0x6a80);p->before_demo=R16(a,0x420);p->before_event=_gz_pbac_event;
 return p->before_gameover_ID!=BAD&&p->before_demo==5&&p->before_event==3?0:LPC_SOURCE;
}
/* Native execute begins by reading the debug pose globals. Keeping only
 * actor.current would snap it back on the next Execute. These are scalar
 * vectors/angles, never owner or collision pointers. Record markers keep
 * older captures from silently fabricating unrecorded native state. */
static const LpcField lpc_motion_fields[]={{0x374,4},{0x378,4},{0x834,4},{0x8a0,4}};
extern u8 _gz_lpc_native_pose_position[],_gz_lpc_native_pose_shape[],_gz_lpc_native_pose_current[];
static const u32 lpc_pose_globals[]={(u32)_gz_lpc_native_pose_position,(u32)_gz_lpc_native_pose_shape,(u32)_gz_lpc_native_pose_current};
static const u32 lpc_pose_sizes[]={12,6,6};
static u32 lpc_motion_valid(const u8*s){u32 i;
 if(R32(s,4608)!=0x41495231u||R32(s,4640)!=0x44504d31u)return LPC_SOURCE;
 for(i=4612;i<=4624;i+=4)if(i!=4620&&!lpc_finite(R32(s,i)))return LPC_VALUE;
 for(i=4644;i<4656;i+=4)if(!lpc_finite(R32(s,i)))return LPC_VALUE;
 return 0;
}
static void lpc_motion_capture(u8*s,u32 a){u32 i,k=4644;
 *(u32*)(s+4608)=0x41495231u;*(u32*)(s+4640)=0x44504d31u;
 for(i=0;i<4;i++)btr_copy(s+4612+4*i,(const void*)(a+lpc_motion_fields[i].offset),4);
 for(i=0;i<3;i++){btr_copy(s+k,(const void*)lpc_pose_globals[i],lpc_pose_sizes[i]);k+=lpc_pose_sizes[i];}
}
static void lpc_motion_write(const u8*s,u32 a){u32 i,k=4644;
 for(i=0;i<4;i++)btr_copy((void*)(a+lpc_motion_fields[i].offset),s+4612+4*i,4);
 for(i=0;i<3;i++){btr_copy((void*)lpc_pose_globals[i],s+k,lpc_pose_sizes[i]);k+=lpc_pose_sizes[i];}
}
static u32 lpc_motion_match(const u8*s,u32 a){u32 i,k=4644;
 for(i=0;i<4;i++)if(!btr_equal((const void*)(a+lpc_motion_fields[i].offset),s+4612+4*i,4))return LPC_CHANGED;
 for(i=0;i<3;i++){if(!btr_equal((const void*)lpc_pose_globals[i],s+k,lpc_pose_sizes[i]))return LPC_CHANGED;k+=lpc_pose_sizes[i];}
 return 0;
}
static u32 lpc_value(const u8*s,u32 offset,u32*out){u32 i,k=3200,j,v;
 for(i=0;i<_gz_lpc_field_count();i++){
  if(offset>=lpc_fields[i].offset&&offset-lpc_fields[i].offset+4<=lpc_fields[i].bytes){
   v=0;for(j=0;j<4;j++)v=(v<<8)|s[k+offset-lpc_fields[i].offset+j];*out=v;return 1;
  }k+=lpc_fields[i].bytes;
 }return 0;
}
static u32 lpc_record(const u8*s,u32*bindings,const LpcArc*arc){u32 i,j,k=3200,proc=BAD,t,id,ref,value;
 if(R32(s,0)!=1||!lpc_procedure(R32(s,40),R32(s,44),R32(s,48)))return LPC_SOURCE;
 {u32 z=lpc_motion_valid(s);if(z)return z;}
 for(i=0;i<_gz_lpc_field_count();i++){if(lpc_fields[i].offset==0x65f0)proc=R32(s,k);k+=lpc_fields[i].bytes;}
 if(k>4500||proc!=R32(s,40)||R8(s,1128)>1||R16(s,1152)>42||R16(s,1154)>42||R16(s,1152)>R16(s,1154))return LPC_SOURCE;
 {static const LpcField floats[]={{0x300,12},{0x314,12},{0x33c,12},{0x370,4},{0x69f0,40},{0x6a1c,76},{0x6a9c,36},{0x6ac0,12},{0x6ad4,12},{0x7290,24}};
  for(i=0;i<sizeof(floats)/sizeof(floats[0]);i++)for(j=0;j<floats[i].bytes;j+=4)
   if(!lpc_value(s,floats[i].offset+j,&value)||!lpc_finite(value))return LPC_VALUE;
 }
 for(i=1132;i<1152;i+=4)if(!lpc_finite(R32(s,i)))return LPC_VALUE;
 for(i=1156;i<1828;i+=4)if(!lpc_finite(R32(s,i)))return LPC_VALUE;
 for(i=0;i<42;i++)for(j=0;j<3;j++)if(!lpc_finite(R32(s,1828+i*32+j*4))||!lpc_finite(R32(s,1828+i*32+20+j*4)))return LPC_VALUE;
 for(i=0;i<5;i++){
  t=128+i*200;id=R16(s,t);ref=R32(s,88+i*4);
  /* Demo-archive lifetimes remain a named future participant. */
  if(R16(s,t+2)!=65535||R16(s,t+4)!=65535||!lpc_finite(R32(s,t+8))||!lpc_finite(R32(s,t+12))||!lpc_finite(R32(s,t+24)))return LPC_SOURCE;
  for(j=0;j<42;j++)if(!lpc_finite(R32(s,t+28+j*4)))return LPC_VALUE;
  if(id!=65535){if(ref!=i)return LPC_SOURCE;bindings[i]=id;}
  else if(ref==BAD)bindings[i]=BAD;
  else{if(ref>=5)return LPC_SOURCE;bindings[i]=R16(s,128+ref*200);if(bindings[i]==65535)return LPC_SOURCE;}
  if(lpc_resource(arc,bindings[i],&value))return LPC_RESOURCE;
 }
 return 0;
}
static u32 lpc_current_numeric(const u8*s){u32 i,j,value;
 static const LpcField floats[]={{0x300,12},{0x314,12},{0x33c,12},{0x370,4},{0x69f0,40},{0x6a1c,76},{0x6a9c,36},{0x6ac0,12},{0x6ad4,12},{0x7290,24}};
 if(lpc_motion_valid(s)||R8(s,1128)>1||R16(s,1152)>42||R16(s,1154)>42||R16(s,1152)>R16(s,1154))return LPC_VALUE;
 for(i=0;i<sizeof(floats)/sizeof(floats[0]);i++)for(j=0;j<floats[i].bytes;j+=4)
  if(!lpc_value(s,floats[i].offset+j,&value)||!lpc_finite(value))return LPC_VALUE;
 for(i=1132;i<1152;i+=4)if(!lpc_finite(R32(s,i)))return LPC_VALUE;
 for(i=1156;i<1828;i+=4)if(!lpc_finite(R32(s,i)))return LPC_VALUE;
 for(i=0;i<42;i++)for(j=0;j<3;j++)if(!lpc_finite(R32(s,1828+i*32+j*4))||!lpc_finite(R32(s,1828+i*32+20+j*4)))return LPC_VALUE;
 for(i=0;i<5;i++){
  u32 t=128+i*200;
  if(R16(s,t+2)!=65535||R16(s,t+4)!=65535||!lpc_finite(R32(s,t+8))||!lpc_finite(R32(s,t+12))||!lpc_finite(R32(s,t+24)))return LPC_SOURCE;
  for(j=0;j<42;j++)if(!lpc_finite(R32(s,t+28+j*4)))return LPC_VALUE;
 }
 return 0;
}
static u32 lpc_binding(u32 capture){LpcPlan*p=&_gz_lpc_plan;PoEntry e;LpcArc arc;u32 a,z,i,j,t[8],n[8]={36,1344,672,168,168,168,168,168},heap;
 z=lpc_process(&e);if(z)return z;a=e.object;heap=R32(a,0xf4);
 /* +693C is the current SafeString vtable, not a numeric body value.
  * Capture retains its old packed word as an opaque audit, but neither
  * normal replay nor undo may install the source/before address. */
 if(R32(a,0x693c)!=(u32)_gz_lpc_native_sound_string_vtable)return LPC_RESOURCE;
 if(!capture&&(a!=p->actor||e.pid!=p->pid||heap!=p->heap||!btr_equal(&e,&p->process,sizeof(e))))return LPC_PROCESS;
 t[0]=R32(a,0x65cc);
 if(_gz_gh_solid(heap,t[0],36))return LPC_HEAP;
 t[1]=R32(t[0],28);t[2]=R32(t[0],32);
 for(i=0;i<5;i++){if(R32(a,0x5800+i*16)!=42)return LPC_HEAP;t[i+3]=R32(a,0x5804+i*16);}
 for(i=0;i<8;i++){
  for(j=0;j<i;j++)if(t[i]<t[j]+n[j]&&t[j]<t[i]+n[i])return LPC_HEAP;
  if(t[i]<a+e.allocation_bytes&&a<t[i]+n[i])return LPC_HEAP;
  if(_gz_gh_solid(heap,t[i],n[i]))return LPC_HEAP;
  if(capture){p->targets[i]=t[i];p->spans[i]=n[i];btr_copy(&p->heaps[i],(const void*)&_gz_gh_identity,sizeof(GhIdentity));}
  else if(t[i]!=p->targets[i]||n[i]!=p->spans[i]||!btr_equal(&p->heaps[i],(const void*)&_gz_gh_identity,sizeof(GhIdentity)))return LPC_HEAP;
 }
 z=lpc_archive(&arc);if(z)return z;
 if(capture){p->actor=a;p->pid=e.pid;p->heap=heap;btr_copy(&p->process,&e,sizeof(e));btr_copy(&p->archive,&arc,sizeof(arc));}
 else if(!btr_equal(&arc,&p->archive,sizeof(arc)))return LPC_RESOURCE;
 return 0;
}
/* Current resource pointer -> current resident index. No source pointer is
 * used, and a resource that has no active alias still gets a reversible ID. */
static u32 lpc_capture(void){LpcPlan*p=&_gz_lpc_plan;u8*s=p->before;u32 a=p->actor,i,j,k=3200,t,ptr,id;
 {u32 z=lpc_current_procedure(p);if(z)return z;}
 for(i=0;i<8192;i++)s[i]=0;
 *(u32*)(s)=1;*(u32*)(s+4)=p->pid;*(u32*)(s+8)=a;
 for(i=0;i<3;i++)*(u32*)(s+12+i*4)=p->targets[i==1?1:i==2?2:0];
 *(u32*)(s+40)=R32(a,0x65f0);*(u32*)(s+44)=R32(a,0x65f4);*(u32*)(s+48)=R32(a,0x65f8);
 for(i=0;i<5;i++){
  t=128+i*200;btr_copy(s+t,(const void*)(a+0x5848+i*16),8);btr_copy(s+t+8,(const void*)(a+0x5898+i*16),16);
  *(u32*)(s+t+24)=R32(a,0x57f8+i*16);btr_copy(s+t+28,(const void*)p->targets[i+3],168);
  ptr=R32(a,0x57fc+i*16);id=BAD;
  if(ptr){for(j=0;j<p->archive.count;j++)if(R32(p->archive.table,j*4)==ptr){id=j;break;}if(id==BAD)return LPC_RESOURCE;}
  p->before_bind[i]=id;
 }
 btr_copy(s+1128,(const void*)p->targets[0],28);btr_copy(s+1156,(const void*)p->targets[2],672);btr_copy(s+1828,(const void*)p->targets[1],1344);
 for(i=0;i<_gz_lpc_field_count();i++){btr_copy(s+k,(const void*)(a+lpc_fields[i].offset),lpc_fields[i].bytes);k+=lpc_fields[i].bytes;}
 lpc_motion_capture(s,a);
 {u32 z=lpc_current_numeric(s);if(z)return z;}
 if(p->before_role){
  if(R32(s,40)!=178||R32(s,44)!=65535||R32(s,48)!=0x02422108u||
   !_gz_doh_player_scope(p->token,_gz_pbac_frame,a,p->pid)||R32(a,0x6a80)!=p->before_gameover_ID||
   R16(a,0x420)!=p->before_demo||_gz_pbac_event!=p->before_event)return LPC_CHANGED;
  return 0;
 }
 return lpc_procedure(R32(s,40),R32(s,44),R32(s,48))?0:LPC_SOURCE;
}
static u32 lpc_values(const u8*s,const u32*ids){LpcPlan*p=&_gz_lpc_plan;u32 a=p->actor,i,k=3200,ptr,z;
 for(i=0;i<5;i++){
  z=lpc_resource(&p->archive,ids[i],&ptr);if(z)return z;*(u32*)(a+0x57fc+i*16)=ptr;
  btr_copy((void*)(a+0x5848+i*16),s+128+i*200,8);btr_copy((void*)(a+0x5898+i*16),s+136+i*200,16);
  *(u32*)(a+0x57f8+i*16)=R32(s,152+i*200);btr_copy((void*)p->targets[i+3],s+156+i*200,168);
 }
 btr_copy((void*)p->targets[0],s+1128,2);btr_copy((void*)(p->targets[0]+4),s+1132,24);
 btr_copy((void*)p->targets[2],s+1156,672);
 for(i=0;i<42;i++){btr_copy((void*)(p->targets[1]+32*i),s+1828+32*i,18);btr_copy((void*)(p->targets[1]+32*i+20),s+1848+32*i,12);}
 for(i=0;i<_gz_lpc_field_count();i++){
  u32 skip=lpc_fields[i].offset==0x693c?4:0;
  btr_copy((void*)(a+lpc_fields[i].offset+skip),s+k+skip,lpc_fields[i].bytes-skip);k+=lpc_fields[i].bytes;
 }
 *(u32*)(a+0x65f4)=R32(s,44);*(u32*)(a+0x65f8)=R32(s,48);
 lpc_motion_write(s,a);
 return 0;
}
static u32 lpc_match(const u8*s,const u32*ids){LpcPlan*p=&_gz_lpc_plan;u32 a=p->actor,i,k=3200,ptr;
 for(i=0;i<5;i++){
  if(lpc_resource(&p->archive,ids[i],&ptr)||R32(a,0x57fc+i*16)!=ptr||R32(a,0x57f8+i*16)!=R32(s,152+i*200)||
   !btr_equal((const void*)(a+0x5848+i*16),s+128+i*200,8)||!btr_equal((const void*)(a+0x5898+i*16),s+136+i*200,16)||
   !btr_equal((const void*)p->targets[i+3],s+156+i*200,168))return LPC_CHANGED;
 }
 if(!btr_equal((const void*)p->targets[0],s+1128,2)||!btr_equal((const void*)(p->targets[0]+4),s+1132,24)||!btr_equal((const void*)p->targets[2],s+1156,672))return LPC_CHANGED;
 for(i=0;i<42;i++)if(!btr_equal((const void*)(p->targets[1]+32*i),s+1828+32*i,18)||!btr_equal((const void*)(p->targets[1]+32*i+20),s+1848+32*i,12))return LPC_CHANGED;
 for(i=0;i<_gz_lpc_field_count();i++){
  u32 skip=lpc_fields[i].offset==0x693c?4:0;
  if(!btr_equal((const void*)(a+lpc_fields[i].offset+skip),s+k+skip,lpc_fields[i].bytes-skip))return LPC_CHANGED;k+=lpc_fields[i].bytes;
 }
 if(lpc_motion_match(s,a))return LPC_CHANGED;
 return R32(a,0x65f4)==R32(s,44)&&R32(a,0x65f8)==R32(s,48)?0:LPC_CHANGED;
}
u32 _gz_lpc_prepare(u32 token,u32 frame){LpcPlan*p=&_gz_lpc_plan;LpcRun*r=&_gz_lpc_run;u32 z,s,n,off,birth,i,br;const VrWorld*w;
 if(_gz_lpc_entered||p->valid||r->phase||_gz_lpc_seal)return 28;
 if(!token||_gz_roh_gl_context(token,frame,1)!=1||!_gz_hbp_read(token,frame))return 42;
 _gz_lpc_entered=1;s=_gz_hbp_recipe.source;n=_gz_hbp_recipe.used;
 z=_gz_bcb_archive_validate(s,n,1);if(z)goto done;
 off=R32(s,40);if(R32(s,44)!=8192||off>n||8192>n-off){z=LPC_SOURCE;goto done;}
 p->magic=0x4c504331u;p->bytes=sizeof(*p);p->version=2;p->token=token;p->frame=frame;p->source=s;p->used=n;p->pin=_gz_hbp_recipe.pin;
 btr_copy(p->saved,(const void*)(s+off),8192);p->source_hash=btr_hash(p->saved,8192);
 z=lpc_binding(1);if(z)goto done;
 w=(const VrWorld*)(s+R32(s,168));birth=s+R32(s,192);
 if(w->link_pid!=p->pid||w->link_actor!=p->actor||R32(p->saved,4)!=p->pid||R32(p->saved,8)!=p->actor){z=LPC_PROCESS;goto done;}
 for(i=0;i<w->count;i++)if(w->actors[i].pid==p->pid)break;
 if(i==w->count){z=LPC_PROCESS;goto done;}br=birth+96+i*192;
 if(R32(br,0)!=i||R32(br,4)!=1||!btr_equal((const void*)(br+16),&p->process,sizeof(PoEntry))){z=LPC_PROCESS;goto done;}
 z=lpc_record(p->saved,p->saved_bind,&p->archive);if(z)goto done;
 z=lpc_capture();if(z)goto done;p->before_hash=btr_hash(p->before,8192);
 z=lpc_binding(0);if(z)goto done;z=lpc_match(p->before,p->before_bind);if(z)goto done;
 if(_gz_roh_gl_context(token,frame,1)!=1||!_gz_hbp_read(token,frame)){z=42;goto done;}
 p->valid=1;_gz_lpc_seal=btr_hash(p,sizeof(*p));r->magic=0x4c505231u;r->bytes=sizeof(*r);r->valid=1;r->phase=LPC_PREPARED;r->token=token;r->plan_seal=_gz_lpc_seal;lpc_run_seal();
 _gz_lpc_report[2]=token;_gz_lpc_report[3]=LPC_PREPARED;_gz_lpc_report[5]=frame;_gz_lpc_report[8]=p->actor;_gz_lpc_report[9]=p->pid;
done:_gz_lpc_report[4]=z;_gz_lpc_entered=0;return z;
}
static u32 lpc_gate(u32 token,u32 frame){const LpcPlan*p=&_gz_lpc_plan;
 if(!lpc_sealed()||p->token!=token||_gz_roh_owner[12]!=2||!_gz_bmh_scope(token,frame)||_gz_roh_replay_common(token,frame,1)||
  _gz_boh_service_validate(token)||!_gz_hbp_service_read(token,frame)||p->source!=_gz_hbp_recipe.source||p->used!=_gz_hbp_recipe.used||p->pin!=_gz_hbp_recipe.pin||
  R32(p->source,44)!=8192||!btr_equal((const void*)(p->source+R32(p->source,40)),p->saved,8192))return 42;
 return 0;
}
u32 _gz_lpc_apply(u32 token){LpcPlan*p=&_gz_lpc_plan;LpcRun*r=&_gz_lpc_run;u32 frame=_gz_pbac_frame,z;
 if(_gz_lpc_entered||!lpc_sealed()||p->token!=token||r->phase!=LPC_PREPARED)return 28;
 _gz_lpc_entered=1;z=lpc_gate(token,frame);if(z)goto done;z=lpc_binding(0);if(z)goto done;z=lpc_match(p->before,p->before_bind);if(z)goto done;
 r->phase=LPC_APPLYING;r->applied_frame=frame;lpc_run_seal();z=lpc_values(p->saved,p->saved_bind);if(z)goto done;
 z=lpc_binding(0);if(z)goto done;z=lpc_match(p->saved,p->saved_bind);if(z)goto done;z=lpc_gate(token,frame);if(z)goto done;
 r->phase=LPC_APPLIED;lpc_run_seal();_gz_lpc_report[3]=LPC_APPLIED;_gz_lpc_report[6]=frame;
done:
 if(r->magic==0x4c505231u&&r->bytes==sizeof(*r)&&r->valid==1&&r->token==token&&r->phase>=1&&r->phase<=5&&
  r->plan_seal==_gz_lpc_seal&&_gz_lpc_seal==btr_hash(p,sizeof(*p))){r->error=z;lpc_run_seal();}
 _gz_lpc_report[4]=z;_gz_lpc_entered=0;return z;
}
u32 _gz_lpc_recovery_rollback(u32 token){LpcPlan*p=&_gz_lpc_plan;LpcRun*r=&_gz_lpc_run;u32 frame=_gz_pbac_frame,z;
 if(_gz_lpc_entered||!lpc_sealed()||p->token!=token||r->phase==LPC_ROLLED)return 28;
 _gz_lpc_entered=1;if(!_gz_boh_recovery_authority(token,_gz_boh_journal.actor,_gz_boh_plan.link,frame)){z=40;goto done;}
 z=lpc_binding(0);if(z)goto done;
 if(r->phase!=LPC_PREPARED){r->phase=LPC_UNDOING;lpc_run_seal();z=lpc_values(p->before,p->before_bind);if(z)goto done;}
 z=lpc_binding(0);if(z)goto done;z=lpc_match(p->before,p->before_bind);if(z)goto done;
 if(!_gz_boh_recovery_authority(token,_gz_boh_journal.actor,_gz_boh_plan.link,frame)){z=40;goto done;}
 if(_gz_lpc_seal!=btr_hash(p,sizeof(*p))){z=28;goto done;}
 r->phase=LPC_ROLLED;r->rolled_frame=frame;r->error=0;lpc_run_seal();_gz_lpc_report[3]=LPC_ROLLED;_gz_lpc_report[7]=frame;
done:_gz_lpc_report[4]=z;_gz_lpc_entered=0;return z;
}
