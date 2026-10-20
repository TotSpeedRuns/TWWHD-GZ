/* Captured GanonK Morth Hover snapshot domain. Current native actors are
 * rediscovered on every tick; no captured process/heap/model address is used.
 * Normal initialization owns all models, collision links and allocations.
 * This is not admission of the unfinished whole-game snapshot transaction.
 */
#include "morth_setup.h"
typedef unsigned int U32;typedef unsigned short U16;typedef unsigned char U8;
#define R32(a,o) (*(volatile U32*)((a)+(o)))
#define R16(a,o) (*(volatile U16*)((a)+(o)))
#define R8(a,o) (*(volatile U8*)((a)+(o)))
extern const U8 _gz_mht_rows[3][208],_gz_mht_packet[400],_gz_mht_keep[6],_gz_mht_descriptor[];
extern const U32 _gz_mht_pose[3],_gz_mht_facing;
extern volatile U32 _gz_mht_actor_head,_gz_mht_create_head,_gz_mht_delete_head,_gz_mht_frame;
extern volatile U8 _gz_mht_event,_gz_loading_store,_gz_room_store;
extern volatile U32 _gz_stage_store[2],_gz_link_slot,_gz_heap_slot,_gz_pending,_gz_route_active,_gz_route_choice;
extern volatile U32 _gz_state[],_gz_page,_gz_status,_gz_world_resume_pending,_gz_life_phase;
extern volatile U32 _gz_position_store[3];extern volatile U16 _gz_angle_store[3],_gz_shape_store[3];
extern U8 _gz_keep_equipment[8];
extern U32 _gz_route_heap_check(void),_gz_route_load(void);
extern void _gz_route_finish(void),_gz_route_apply(const void*),_gz_if_apply_c(void),_gz_start_toast(void),_gz_clear_damage_restart(void);
extern U32 _gz_mht_native_create(U32,U32,const void*,int,const void*,const void*,U32,U32);
extern U32 _gz_mht_native_delete(void*);
volatile U32 _gz_mht_phase,_gz_mht_error;
U32 _gz_mht_report[40]={0x4d485431u,160};
static U32 mht_serial,mht_start,mht_last,mht_link,mht_link_pid,mht_issued,mht_delete_pid,mht_route;
static U32 mht_born[3],mht_born_count,mht_cleanup_error;
static const U32 mht_fields[12][2]={{0x2ec,20},{0x300,20},{0x314,20},{0x328,6},{0x330,24},{0x370,12},{0x37c,12},{0x3a0,2},{0x3a4,4},{0x3a8,2},{0x544,12},{0x558,72}};
static U32 mht_span(U32 a,U32 n){return !(a&3)&&a>=0x10000000u&&n&&a+n>a&&a+n<0xbffe0000u;}
static void mht_copy(void*d,const void*s,U32 n){U32 i;for(i=0;i<n;i++)((volatile U8*)d)[i]=((const U8*)s)[i];}
static U32 mht_hash(const void*p,U32 n){U32 i,h=2166136261u;for(i=0;i<n;i++)h=(h^((const U8*)p)[i])*16777619u;return h;}
static U32 mht_scene(void){return _gz_stage_store[0]==0x47616e6fu&&_gz_stage_store[1]==0x6e4b0000u&&_gz_room_store==0;}
static U32 mht_live(void){U32 a=_gz_link_slot;
 return !_gz_loading_store&&!_gz_mht_event&&mht_span(a,0x8100)&&R16(a,8)==168&&R8(a,12)==2&&
  R32(a,0x65f0)>=4&&R32(a,0x65f0)<=5;
}
/* Bounded current-list query. Output actor addresses expire before any native
 * callback. The current process PID, class and constructed marker are checked
 * again before scalar writes or a native delete call. */
static U32 mht_find(U32 pid,U32*out){U32 node=_gz_mht_actor_head,n=0,prev=0,a;*out=0;
 while(node){
  if(++n>4096||!mht_span(node,20)||node==prev)return 1;
  a=R32(node,12);if(a){if(!mht_span(a,0x3ac)||R8(a,12)>3)return 1;
   if(R32(a,4)==pid){if(*out)return 1;*out=a;}}
  prev=node;node=R32(node,8);
 }
 return 0;
}
static U32 mht_class(U32 a){return mht_span(a,0xf08)&&R16(a,8)==205&&R8(a,12)==2&&
 R32(a,0xb0)==2&&R32(a,0xb4)==0x100136c0u&&R32(a,0xa8)==0x101f3bdcu&&R8(a,0x3aa)==1;
}
static U32 mht_morths(U32*pids,U32*actors,U32*count){U32 node=_gz_mht_actor_head,n=0,a,i,j,x,y;*count=0;
 while(node){
  if(++n>4096||!mht_span(node,20))return 1;
  a=R32(node,12);if(a){
   if(!mht_span(a,0x3ac))return 1;
   if(R16(a,8)==205){
    if(!mht_class(a)||*count==3)return 2;
    i=(*count)++;pids[i]=R32(a,4);actors[i]=a;
   }
  }
  node=R32(node,8);
 }
 for(i=1;i<*count;i++)for(j=i;j&&pids[j]<pids[j-1];j--){x=pids[j];pids[j]=pids[j-1];pids[j-1]=x;y=actors[j];actors[j]=actors[j-1];actors[j-1]=y;}
 for(i=1;i<*count;i++)if(pids[i]==pids[i-1]||actors[i]==actors[i-1])return 1;
 return 0;
}
/* None of these transient actors existed in the captured source scene. Let
 * their native destructors remove collision/resources; do not clear tags or
 * restore their old pointers. This also removes the preceding attempt's heart.
 */
static U32 mht_ephemeral(U32*pid,U32*out){U32 node=_gz_mht_actor_head,n=0,a,p;*pid=*out=0;
 while(node){if(++n>4096||!mht_span(node,20))return 1;a=R32(node,12);
  if(a){if(!mht_span(a,0x3ac))return 1;p=R16(a,8);
   if(p==255||p==294||p==472||p==473||p==474){
    if(R8(a,12)!=2||R8(a,0x3aa)!=1)return 2;*pid=R32(a,4);*out=a;return 0;}}
  node=R32(node,8);
 }return 0;
}
static void mht_menu(void){_gz_state[0]=1;_gz_page=50;_gz_state[2]=0;}
static void mht_done(U32 error){_gz_mht_error=error;_gz_mht_report[4]=error;_gz_mht_report[5]=_gz_mht_frame;
 _gz_mht_report[6]=mht_route;_gz_mht_phase=0;
 if(!error){_gz_state[0]=0;_gz_page=0;_gz_status=15;}
 else{mht_menu();_gz_status=5;}
 _gz_start_toast();
}
static void mht_fail(U32 error){mht_cleanup_error=error;_gz_mht_report[4]=error;
 if(mht_born_count||mht_issued){_gz_mht_phase=7;mht_start=_gz_mht_frame;}
 else mht_done(error);
}
void _gz_mht_choose(void){U32 i;
 if(_gz_mht_phase)return;
 if(_gz_state[0]!=1||_gz_page!=50||_gz_pending||_gz_route_active||_gz_world_resume_pending||_gz_life_phase||!mht_live()){
  _gz_mht_error=1;_gz_status=5;_gz_start_toast();return;}
 for(i=2;i<40;i++)_gz_mht_report[i]=0;
 mht_serial++;if(!mht_serial)mht_serial++;
 _gz_mht_report[2]=mht_serial;mht_start=_gz_mht_frame;mht_last=~mht_start;
 mht_link=_gz_link_slot;mht_link_pid=R32(mht_link,4);mht_issued=mht_delete_pid=mht_route=mht_born_count=mht_cleanup_error=0;
 for(i=0;i<3;i++)mht_born[i]=0;
 _gz_mht_error=0;_gz_mht_phase=1;
}
static U32 mht_publish(const U32*pids,const U32*actors){U32 i,j,k,at,a,found;
 if(_gz_mhs_validate(_gz_mht_packet,400)||_gz_route_heap_check()||!mht_scene()||!mht_live()||
  _gz_mht_create_head||_gz_mht_delete_head||_gz_pending||_gz_route_active)return 10;
 /* Recheck all three identities before the first fixed scalar store. */
 for(i=0;i<3;i++)if(mht_find(pids[i],&found)||found!=actors[i]||!mht_class(found))return 11;
 for(i=0;i<6;i++)_gz_keep_equipment[i]=_gz_mht_keep[i];
 _gz_route_apply(_gz_mht_descriptor);_gz_if_apply_c();
 for(i=0;i<3;i++){
  at=0;a=actors[i];
  for(j=0;j<12;j++){for(k=0;k<mht_fields[j][1];k++)R8(a,mht_fields[j][0]+k)=_gz_mht_rows[i][at+k];at+=mht_fields[j][1];}
  /* The captured platform setup has no live joint/parent attachment. */
  R32(a,0x550)=0;R32(a,0x554)=0;R32(a,0x7a4)=0;
  _gz_mht_report[16+i]=pids[i];_gz_mht_report[20+i]=a;
 }
 a=_gz_link_slot;
 for(i=0;i<3;i++){_gz_position_store[i]=_gz_mht_pose[i];R32(a,0x314+4*i)=_gz_mht_pose[i];R32(a,0x300+4*i)=_gz_mht_pose[i];R32(a,0x33c+4*i)=0;}
 _gz_angle_store[0]=_gz_angle_store[2]=_gz_shape_store[0]=_gz_shape_store[2]=0;
 _gz_angle_store[1]=_gz_shape_store[1]=(U16)_gz_mht_facing;
 R16(a,0x320)=R16(a,0x324)=R16(a,0x328)=R16(a,0x32c)=0;
 R16(a,0x322)=R16(a,0x32a)=R16(a,0x30e)=(U16)_gz_mht_facing;
 R32(a,0x370)=R32(a,0x6a14)=0;
 R32(a,0x7294)=R32(a,0x6a48)=R32(a,0x6a4c)=_gz_mht_pose[1];
 _gz_clear_damage_restart();
 _gz_mht_report[24]=a;_gz_mht_report[25]=R32(a,4);_gz_mht_report[26]=mht_hash(_gz_mht_rows,624);
 _gz_mht_report[27]=_gz_mht_frame;return 0;
}
U32 _gz_mht_tick(void){U32 phase=_gz_mht_phase,i,z,n,pids[3],actors[3],a,pid,sc[3],angles[2]={0,0};
 if(!phase)return 0;
 if(mht_last==_gz_mht_frame)return 1;mht_last=_gz_mht_frame;_gz_mht_report[3]=phase;
 if(_gz_mht_frame-mht_start>900){_gz_mht_error=90;_gz_mht_report[4]=90;_gz_mht_phase=8;return 1;}
 if(phase==8)return 1;
 mht_menu();
 if(phase==1){
  if(_gz_mhs_validate(_gz_mht_packet,400)){mht_fail(3);return 1;}
  if(_gz_mht_create_head||_gz_mht_delete_head)return 1;
  if(!mht_scene()){
   _gz_route_choice=56;z=_gz_route_load();mht_menu();
   if(z!=10){mht_fail(4);return 1;}
   mht_route=1;_gz_mht_phase=2;return 1;
  }
  _gz_mht_phase=3;return 1;
 }
 if(phase==2){_gz_route_finish();mht_menu();if(_gz_route_active)return 1;
  if(!mht_scene()){mht_fail(5);return 1;}if(!mht_live())return 1;
  mht_link=_gz_link_slot;mht_link_pid=R32(mht_link,4);_gz_mht_phase=3;return 1;
 }
 if(phase==7){
  if(_gz_mht_create_head||_gz_mht_delete_head)return 1;
  for(i=0;i<mht_born_count;i++){
   if(mht_find(mht_born[i],&a)){_gz_mht_phase=8;return 1;}
   if(a){if(!mht_scene()||!mht_class(a)||_gz_mht_native_delete((void*)a)!=1){_gz_mht_phase=8;return 1;}return 1;}
  }
  mht_done(mht_cleanup_error);return 1;
 }
 if(!mht_scene()||!mht_live()||_gz_link_slot!=mht_link||R32(mht_link,4)!=mht_link_pid){mht_fail(6);return 1;}
 if(_gz_mht_create_head||_gz_mht_delete_head)return 1;
 if(phase==5){
  if(mht_find(mht_delete_pid,&a)){mht_fail(7);return 1;}if(a)return 1;
  mht_delete_pid=0;_gz_mht_phase=3;return 1;
 }
 if(phase==4){
  if(mht_find(mht_issued,&a)||!a||!mht_class(a)){mht_fail(8);return 1;}
  mht_issued=0;_gz_mht_phase=3;return 1;
 }
 if(phase!=3){mht_fail(9);return 1;}
 z=mht_ephemeral(&pid,&a);if(z){mht_fail(12+z);return 1;}
 if(a){
  /* Call the current actor's native deletion lane once, then rediscover. */
  if(_gz_mht_native_delete((void*)a)!=1){mht_fail(15);return 1;}
  mht_delete_pid=pid;_gz_mht_report[9]++;_gz_mht_phase=5;return 1;
 }
 z=mht_morths(pids,actors,&n);if(z){mht_fail(16+z);return 1;}
 if(n<3){
  if(mht_born_count==3){mht_fail(20);return 1;}
  mht_copy(angles,_gz_mht_rows[n]+12,6);mht_copy(sc,_gz_mht_rows[n]+66,12);
  pid=_gz_mht_native_create(205,2,_gz_mht_rows[n],(signed char)_gz_mht_rows[n][18],angles,sc,0xffffffffu,0);
  if(!pid||pid==0xffffffffu){mht_fail(21);return 1;}
  mht_issued=pid;mht_born[mht_born_count++]=pid;_gz_mht_report[8]++;_gz_mht_phase=4;return 1;
 }
 z=mht_publish(pids,actors);if(z){mht_fail(z);return 1;}
 _gz_mht_report[7]++;mht_done(0);return 1;
}
