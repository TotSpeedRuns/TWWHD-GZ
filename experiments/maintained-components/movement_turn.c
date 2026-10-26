/* Movement utility. Input queues authored intent; the existing completed
 * native game-frame callback resolves the current player independently.
 * There is no saved actor pointer, remote native call or continuous writer.
 */
#include "process.h"
typedef unsigned int U;
extern volatile U _gz_state[],_gz_page,_gz_pending,_gz_link_slot,_gz_game_counter;
extern volatile U _gz_life_phase,_gz_mht_phase,_gz_pbac_world_resume_pending;
extern volatile U _gz_pba_native_actor_queue,_gz_pba_native_create_queue,_gz_pba_native_delete_queue;
extern volatile unsigned char _gz_loading_store,_gz_pbac_event;
extern unsigned char _gz_angle_store[],_gz_shape_store[];
extern U _gz_bind_masks[],_gz_roh_user_guard(void),_gz_osa_pin_id(void);
extern void _gz_game_text(U,U,U,const char*);
enum {MVT_PAGE=54,MVT_ROOT=8,MVT_BAD=1,MVT_CHANGED=2};
volatile U _gz_mvt_command;
U _gz_mvt_report[16]={0x4d565431u,64};
static U mvt_result;
static U mvt_span(U a,U n){return !(a&3u)&&a>=0x10000000u&&a<0xc0000000u&&n&&n<=0xc0000000u-a;}
U _gz_mvt_quarter(U angle,U direction){return angle<=65535u&&(direction==1||direction==2)?(angle+(direction==1?49152u:16384u))&65535u:0xffffffffu;}
static U mvt_match(U edge,U held,U mask){return mask&&(edge&mask)&&(held&mask)==mask;}
void _gz_mvt_open(void){_gz_page=MVT_PAGE;_gz_state[2]=0;mvt_result=0;}
U _gz_mvt_input(U edge,U held,U repeat){U keys=edge|repeat;
 if(!_gz_state[0]||_gz_page!=MVT_PAGE)return 0;
 if(mvt_match(edge,held,_gz_bind_masks[6])){_gz_page=58;_gz_state[2]=1;return 1;}
 if(keys&(_gz_bind_masks[3]|_gz_bind_masks[4]))_gz_state[2]^=1;
 if(_gz_state[2]>1)_gz_state[2]=0;
 if(mvt_match(edge,held,_gz_bind_masks[5])){
  if(!__sync_bool_compare_and_swap(&_gz_mvt_command,0,_gz_state[2]+1))mvt_result=3;
 }
 return 1;
}
static U mvt_quiet(void){return _gz_state[0]&&_gz_page==MVT_PAGE&&!_gz_pending&&!_gz_loading_store&&!_gz_pbac_event&&
 !_gz_life_phase&&!_gz_mht_phase&&!_gz_pbac_world_resume_pending&&!_gz_pba_native_create_queue&&!_gz_pba_native_delete_queue&&
 !_gz_roh_user_guard()&&!_gz_osa_pin_id();}
static U mvt_same(const PoEntry*a,const PoEntry*b){U i;const U*x=(const U*)a,*y=(const U*)b;for(i=0;i<16;i++)if(x[i]!=y[i])return 0;return 1;}
static U mvt_current(U a,PoEntry*out){U q,head,left=4096,next,found=0;const PoEntry*e;
 if(!mvt_quiet()||!mvt_span(a,0x8284)||_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return 0;e=&_gz_po_observation.entry;
 if(e->object!=a||e->state!=PO_LIVE||e->flags||e->end_frame||!e->generation||!e->pid||e->pid>=0xfffffffeu||
  e->profile!=168||e->process_name!=168||e->allocation_bytes<0x8284||e->allocation_bytes>0x100000||
  e->base_bytes+e->other_bytes<e->base_bytes||e->base_bytes+e->other_bytes!=e->allocation_bytes||!mvt_span(a,e->allocation_bytes)||
  e->profile_entry!=_gz_po_profiles[168]||*(volatile U*)(a+4)!=e->pid||*(volatile u16*)(a+8)!=168||
  *(volatile u16*)(a+14)!=168||*(volatile u8*)(a+12)!=2||*(volatile U*)(a+16)!=e->profile_entry)return 0;
 for(U i=0;i<16;i++)((U*)out)[i]=((const volatile U*)e)[i];head=q=_gz_pba_native_actor_queue;
 while(q){if(!left--||!mvt_span(q,16))return 0;next=*(volatile U*)(q+8);if(*(volatile U*)(q+12)==a)found++;q=next;}
 return found==1&&head==_gz_pba_native_actor_queue&&a==_gz_link_slot&&mvt_quiet();
}
void _gz_mvt_tick(void){U command,a,old,current,shape,global_current,global_shape;PoEntry first,last;
 if(!_gz_mvt_command)return;
 command=__sync_lock_test_and_set(&_gz_mvt_command,0);if(!command)return;
 _gz_mvt_report[2]++;_gz_mvt_report[3]=command;_gz_mvt_report[4]=_gz_game_counter;_gz_mvt_report[5]=MVT_BAD;
 mvt_result=3;a=_gz_link_slot;if(command>2||!mvt_current(a,&first))return;
 old=*(volatile u16*)(a+0x30e);current=*(volatile u16*)(a+0x322);shape=*(volatile u16*)(a+0x32a);
 global_current=*(volatile u16*)(_gz_angle_store+2);global_shape=*(volatile u16*)(_gz_shape_store+2);
 if(current!=global_current||shape!=global_shape||!mvt_current(a,&last)||!mvt_same(&first,&last)||
  old!=*(volatile u16*)(a+0x30e)||current!=*(volatile u16*)(a+0x322)||shape!=*(volatile u16*)(a+0x32a)||
  global_current!=*(volatile u16*)(_gz_angle_store+2)||global_shape!=*(volatile u16*)(_gz_shape_store+2)){
  _gz_mvt_report[5]=MVT_CHANGED;return;
 }
 _gz_mvt_report[6]=a;_gz_mvt_report[7]=first.pid;_gz_mvt_report[8]=first.generation;
 _gz_mvt_report[9]=shape;_gz_mvt_report[10]=_gz_mvt_quarter(shape,command);
 /* Fixed current scalar destinations only. X/Z rotation, position, velocity,
  * scalar speed, input bearings, actor pointers and all save bytes are retained. */
 *(volatile u16*)(_gz_angle_store+2)=(u16)_gz_mvt_quarter(current,command);
 *(volatile u16*)(_gz_shape_store+2)=(u16)_gz_mvt_quarter(shape,command);
 *(volatile u16*)(a+0x30e)=(u16)_gz_mvt_quarter(old,command);
 *(volatile u16*)(a+0x322)=(u16)_gz_mvt_quarter(current,command);
 *(volatile u16*)(a+0x32a)=(u16)_gz_mvt_quarter(shape,command);
 __asm__ volatile("sync":::"memory");_gz_mvt_report[5]=0;_gz_mvt_report[11]++;mvt_result=command;
}
static void mvt_text(U x,U y,const char*s){_gz_game_text(0,x,y,s);}
void _gz_mvt_root_draw(void){mvt_text(4,11,"Movement");}
void _gz_mvt_draw(void){
 mvt_text(2,1,"Movement");mvt_text(4,4,"Turn Right 90 Degrees");mvt_text(4,5,"Turn Left 90 Degrees");
 mvt_text(2,4+(_gz_state[2]&1),">");
 if(mvt_result)mvt_text(4,8,mvt_result==1?"Turned right 90 degrees":mvt_result==2?"Turned left 90 degrees":"Try again after the scene settles");
 mvt_text(4,19,"A: Turn   B: Back");
}
