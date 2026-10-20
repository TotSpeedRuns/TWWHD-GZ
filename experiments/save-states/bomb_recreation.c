#include "archive_layout.h"
/* Private expired-inventory-bomb participant. No ordinary restore admission.
 * Archive and scene are derived inside the engine callback. Retired saved
 * actors/models are comparison values; this code never dereferences them. */
#include "abort_state_diagnostic.c"
#include "created_replay.c"
u32 _gz_boh_has_owner(void),_gz_boh_replay_authority(u32,u32,u32,u32),_gz_boh_freeze_authority(u32,u32,u32,u32);
u32 _gz_boh_pause(void*);
/* Reuse the actual current main allocator ledger; no shadow source owner. */
extern u32 _gz_osa_acquire(u32),_gz_osa_pin_id(void),_gz_osa_source(u32),_gz_osa_bytes(u32),_gz_osa_validate(u32);
extern void _gz_osa_release_leaf(void);
extern u32 _gz_b294m_archive_validate(u32,u32,u32),_gz_roh_user_guard(void);
u32 _gz_krc_private_apart(u32,u32);
extern volatile u32 _gz_pbac_selected,_gz_pbac_epoch,_gz_roh_owner[60];
extern u32 _gz_osa_native_store(u32);
extern u32 _gz_pbac_native_create(u32,u32,u32,u32,u32,u32,u32,u32);
extern u32 _gz_pbac_native_delete_actor_entry(void*);
extern volatile u32 _gz_pbac_player,_gz_pbac_menu,_gz_pbac_pending,_gz_pbac_loading,_gz_pbac_frame;
extern volatile u32 _gz_pbac_tools,_gz_pbac_world_resume_pending,_gz_pbac_main_bomb_create_phase;
extern volatile u32 _gz_pbac_stage[2],_gz_pbac_slots[10];
extern volatile u8 _gz_pbac_room,_gz_pbac_layer,_gz_pbac_event;
volatile u32 _gz_pbac_request,_gz_pbac_ack;
/* Private diagnostic handoff only. Zero keeps the previously tested immediate
 * component commit. A matching request defers commit until its release nonce;
 * these words never grant normal whole-world restoration admission. */
volatile u32 _gz_pbac_hold_request,_gz_pbac_hold_release;
u32 _gz_pbac_report[128]={0x42414332,512};
PbaTracker _gz_pbac_tracker;
PbaKey _gz_pbac_issued_key;
typedef struct {u32 magic,bytes,valid,token,pid,inverse_pid,actor,source_actor,source_pid,source_player,start_frame,creating,ready,fault,completed;} Issued;
Issued _gz_pbac_issued;
u32 _gz_pbac_quarantine_emergency,_gz_pbac_witness[18];
u32 _gz_pbac_framework_witness[4]; /* owned base Execute,Draw,last frame,actor */
static u32 _gz_pbac_source_pin,_gz_pbac_source_slot,_gz_pbac_start_epoch;
static u32 _gz_pbac_active,_gz_pbac_fatal,_gz_pbac_source_row,_gz_pbac_source_hash,_gz_pbac_player_pid;
static u32 _gz_pbac_applied;
u32 _gz_pbac_idle_for_capture(void){return !_gz_pbac_active&&!_gz_pbac_fatal&&!_gz_pbac_quarantine_emergency&&!_gz_pbac_issued.valid&&!_gz_pbac_issued.creating;}
/* Only our acquired pin may be released by a pre-native clean refusal. */
static u32 _gz_pbac_pin_owned,_gz_pbac_native_issued;
static u32 _gz_pbac_wait_request,_gz_pbac_wait_frame,_gz_pbac_wait_polls,_gz_pbac_wait_seen;

static PbaQueues current_queues(void){PbaQueues q;q.create_head=_gz_pba_native_create_queue;q.actor_head=_gz_pba_native_actor_queue;q.delete_head=_gz_pba_native_delete_queue;return q;}
static u32 archive_ok(u32 p){return span(p,216)&&gz_arc_shape(p,R32(p,4));}
static u32 source_row(u32 archive){u32 p,n;
 if(_gz_osa_validate(_gz_pbac_source_pin)||_gz_osa_source(_gz_pbac_source_pin)!=archive||!archive_ok(archive))return 0;
 n=_gz_osa_bytes(_gz_pbac_source_pin);
 if(_gz_b294m_archive_validate(archive,n,0)||R32(archive,212)!=16480)return 0;
 p=archive+R32(archive,208);
 if(R32(p,72)!=1)return 0;
 p+=96;return _gz_ibi_record_safe((const PbRow*)p)?0:p;
}
static u32 bomb_count(u32 head){u32 n=4096,count=0,a;
 while(head){if(!n--||!span(head,16))return BAD;a=R32(head,12);
  if(a){if(!span(a,12))return BAD;if(R16(a,8)==294)count++;}head=R32(head,8);}
 return count;
}
/* A recipe has no process identity until native Create returns. Do not pass
 * the BAD sentinel to the ownership validator or invent a temporary PID. */
static u32 recipe_safe(const PbaKey*k){u32 i;
 if(k->pid!=BAD||!k->token||k->slot>9||!archive_ok(k->archive)||k->room>255||k->layer>255||
  k->callback!=(u32)_gz_pbac_ready_callback)return 0;
 for(i=0;i<3;i++)if(!finite(R32(k,40+i*4)))return 0;
 return span(k->family[0],4);
}
static void fault(u32 reason,u32 emergency){_gz_pbac_fatal=1;_gz_pbac_issued.fault=reason;if(!_gz_pbac_report[60])_gz_pbac_report[60]=reason;if(emergency)_gz_pbac_quarantine_emergency=1;}
static u32 issued_ok(void){Issued*s=&_gz_pbac_issued;PbaKey*k=&_gz_pbac_issued_key;
 return s->magic==0x42414931u&&s->bytes==60&&s->valid==1&&s->token==_gz_pbac_active&&s->token&&
  s->pid!=BAD&&s->inverse_pid==~s->pid&&s->pid!=s->source_pid&&s->source_actor==0&&
  k->token==s->token&&k->pid==s->pid&&key_safe(k);
}
static u32 tracker_matches(void){u32 i;if(!header(&_gz_pbac_tracker))return 0;for(i=0;i<sizeof(PbaKey);i++)if(((const u8*)&_gz_pbac_tracker.key)[i]!=((const u8*)&_gz_pbac_issued_key)[i])return 0;return 1;}
static u32 context_reason(u32 count){Issued*s=&_gz_pbac_issued;PbaKey*k=&_gz_pbac_issued_key;u32 l=_gz_pbac_player;
 /* Same ordered short-circuit guards as the native-tested predecessor. A
  * diagnostic code explains a refusal; it never makes that refusal pass. */
 if(_gz_osa_validate(_gz_pbac_source_pin))return 1;
 if(l!=s->source_player)return 2;if(!span(l,0x690d))return 3;
 if(R16(l,8)!=168)return 4;if(R32(l,4)!=_gz_pbac_player_pid)return 5;
 if(R8(l,0x690c)!=count)return 6;if(_gz_pbac_menu!=1)return 7;
 if(_gz_pbac_tools)return 8;if(_gz_pbac_world_resume_pending)return 9;
 if(_gz_pbac_main_bomb_create_phase)return 10;if(_gz_pbac_pending)return 11;
 if(*(volatile u8*)&_gz_pbac_loading)return 12;if(_gz_pbac_event)return 13;
 if(_gz_pbac_stage[0]!=k->stage0)return 14;if(_gz_pbac_stage[1]!=k->stage1)return 15;
 if(_gz_pbac_room!=k->room)return 16;if(_gz_pbac_layer!=k->layer)return 17;
 if(_gz_pbac_selected!=k->slot)return 18;if(_gz_pbac_slots[k->slot]!=k->archive)return 19;
 if(!archive_ok(k->archive))return 20;if(!_gz_pbac_start_epoch)return 21;
 if(_gz_pbac_epoch!=_gz_pbac_start_epoch)return 22;
 if(_gz_roh_owner[3])return 23;if(_gz_roh_owner[4])return 24;
 if(_gz_roh_owner[5])return 25;if(_gz_roh_owner[6])return 26;
 return _gz_roh_user_guard()?27:0;
}
static u32 context(u32 count){return context_reason(count)==0;}
static u32 bind_actor(u32 a,const PbaQueues*q){u32 req;
 if(!issued_ok()||!tracker_matches()||!span(a,0xb50)||R32(a,4)!=_gz_pbac_issued.pid||R16(a,8)!=294)return 0;
 if(_gz_pbac_issued.actor)return _gz_pbac_issued.actor==a;
 req=find_request(q->create_head,_gz_pbac_issued.pid);
 if(!req||req==BAD||!request_owned(&_gz_pbac_tracker,req)||R32(req,0x40)!=a)return 0;
 _gz_pbac_issued.actor=a;_gz_pbac_report[64]=a;return 1;
}
static u32 authority_reason(u32 token,u32 actor,u32 count,u32 frame){PbaQueues q=current_queues();u32 z;
 if(token!=_gz_pbac_active)return 1;if(frame!=_gz_pbac_frame)return 2;if(_gz_pbac_fatal)return 3;
 if(!issued_ok())return 4;if(!tracker_matches())return 5;if(!same_heap(&_gz_pbac_tracker))return 6;
 if(_gz_pbac_tracker.phase!=PBA_OWNED)return 7;if(!_gz_pbac_issued.ready)return 8;
 if(actor!=_gz_pbac_issued.actor)return 9;if(find_actor(q.actor_head,_gz_pbac_issued.pid)!=actor)return 10;
 if(q.create_head)return 11;if(q.delete_head)return 12;if(R8(actor,0xc)!=2)return 13;
 if(!actor_owned(&_gz_pbac_tracker,actor))return 14;z=context_reason(count);return z?100+z:0;
}
u32 _gz_btr_authority(u32 token,u32 actor,u32 count,u32 frame){u32 z;
 if(_gz_boh_has_owner())return _gz_boh_replay_authority(token,actor,count,frame);
 z=authority_reason(token,actor,count,frame);
 if(z&&!_gz_pbac_report[110]){
  _gz_pbac_report[110]=z;_gz_pbac_report[111]=z>=100?z-100:0;
  _gz_pbac_report[112]=_gz_pbac_frame;_gz_pbac_report[113]=token;_gz_pbac_report[114]=count;
  _gz_pbac_report[115]=_gz_pba_native_create_queue;_gz_pbac_report[116]=_gz_pba_native_delete_queue;
  _gz_pbac_report[117]=_gz_pba_native_heap;_gz_pbac_report[118]=_gz_pbac_tracker.allocation_heap;
  _gz_pbac_report[119]=_gz_pbac_event;_gz_pbac_report[120]=_gz_pbac_menu;
  _gz_pbac_report[121]=*(volatile u8*)&_gz_pbac_loading;_gz_pbac_report[122]=_gz_pbac_source_pin;
  _gz_pbac_report[123]=_gz_roh_owner[3];_gz_pbac_report[124]=span(_gz_pbac_player,0x690d)?R8(_gz_pbac_player,0x690c):BAD;
  _gz_pbac_report[125]=span(actor,0x3c0)?R32(actor,0x3bc):0;_gz_pbac_report[126]=frame;_gz_pbac_report[127]=0x42444731u;
 }
 return z==0;
}
u32 _gz_btr_freeze_authority(u32 token,u32 actor,u32 link,u32 frame){PbaQueues q=current_queues();
 if(_gz_boh_has_owner())return _gz_boh_freeze_authority(token,actor,link,frame);
 /* Actual current constructor ownership precedes all model/field reads. This
  * permission only suppresses execution of this one actor. It does not accept
  * a foreign request as ours, pump any queue, or admit any state publication. */
 return token==_gz_pbac_active&&frame==_gz_pbac_frame&&!_gz_pbac_fatal&&issued_ok()&&tracker_matches()&&
  _gz_pbac_tracker.phase==PBA_OWNED&&_gz_pbac_issued.ready&&actor==_gz_pbac_issued.actor&&
  link==_gz_pbac_issued.source_player&&link==_gz_pbac_player&&span(link,0x690d)&&
  R32(link,4)==_gz_pbac_player_pid&&R16(link,8)==168&&!_gz_osa_validate(_gz_pbac_source_pin)&&
  find_actor(q.actor_head,_gz_pbac_issued.pid)==actor&&R8(actor,0xc)==2&&actor_owned(&_gz_pbac_tracker,actor);
}
u32 _gz_pba_native_delete_actor(void*actor){u32 a=(u32)actor,z;
 if(!issued_ok()||!tracker_matches()||!same_heap(&_gz_pbac_tracker)||!_gz_pbac_issued.ready||a!=_gz_pbac_issued.actor||!actor_owned(&_gz_pbac_tracker,a)||!born_free(a))return 0;
 z=_gz_pbac_native_delete_actor_entry(actor);_gz_pbac_report[81]=z;_gz_pbac_report[82]++;return z==1;
}
static u32 pause_owned_actor(void*actor,u32 draw){u32 a=(u32)actor;PbaQueues q;
 if(_gz_pbac_quarantine_emergency){_gz_pbac_report[71]++;return 1;}
 if(_gz_pbac_issued.creating){fault(50,1);return 1;}
 if(!_gz_pbac_issued.valid)return 0;
 if(!issued_ok()){fault(51,1);return 1;}
 if(!span(a,12)){fault(52,1);return 1;}
 if(R32(a,4)!=_gz_pbac_issued.pid)return 0;
 q=current_queues();_gz_pbac_report[67]++;
 _gz_pbac_framework_witness[draw?1:0]++;_gz_pbac_framework_witness[2]=_gz_pbac_frame;_gz_pbac_framework_witness[3]=a;
 if(!bind_actor(a,&q)){fault(53,0);_gz_pbac_report[68]++;return 1;}
 if(_gz_pbac_issued.ready){if(!draw){_gz_pbac_witness[10]++;_gz_pbac_witness[11]=_gz_pbac_frame;_gz_pbac_witness[12]=R16(a,0xa7c);}
  if(_gz_btr_journal.phase==BTR_APPLIED){
   if(_gz_btr_suspended_check(_gz_pbac_active))fault(67,0);
   else _gz_pbac_report[109]++;
  }else if(!born_free(a)||R16(a,0xa7c)!=150)fault(66,0);}
 else _gz_pbac_report[72]++;
 _gz_pbac_report[68]++;return 1;
}
u32 _gz_pbac_pause_before_execute(void*actor){if(_gz_boh_has_owner())return _gz_boh_pause(actor);return pause_owned_actor(actor,0);}
/* The native actor base updates old pose/NOEXEC before the profile Execute,
 * and NODRAW before profile Draw. Quarantine must precede both. Retain the
 * existing profile hook as a fallback; never mask these saved scalar bytes.
 * Unrelated actors and the idle ordinary game take the original native path. */
u32 _gz_pbac_pause_framework_execute(void*actor){u32 a=(u32)actor;
 if(_gz_boh_has_owner())return _gz_boh_pause(actor);
 if(!_gz_pbac_issued.valid&&!_gz_pbac_issued.creating&&!_gz_pbac_quarantine_emergency)return 0;
 if(!span(a,12)||R16(a,8)!=294)return 0;
 return pause_owned_actor(actor,0);
}
extern u32 _gz_roh_render_hold(void);
u32 _gz_pbac_pause_framework_draw(void*actor){u32 a=(u32)actor;
 if(_gz_roh_render_hold())return 1;
 if(_gz_boh_has_owner())return _gz_boh_pause(actor);
 if(!_gz_pbac_issued.valid&&!_gz_pbac_issued.creating&&!_gz_pbac_quarantine_emergency)return 0;
 if(!span(a,12)||R16(a,8)!=294)return 0;
 return pause_owned_actor(actor,1);
}
u32 _gz_pbac_ready_callback(void*actor,void*user){PbaQueues q=current_queues();u32 z;_gz_pbac_report[57]++;
 if(_gz_pbac_issued.creating||user||_gz_pbac_fatal||_gz_pbac_report[57]!=1||!bind_actor((u32)actor,&q)){fault(54,0);return 1;}
 z=_gz_pba_constructor_ready(&_gz_pbac_tracker,actor,&q);_gz_pbac_report[65]=z;if(z){fault(z,0);return 1;}
 _gz_pbac_issued.ready=1;_gz_pbac_report[66]=R16(actor,0xa7c);_gz_pbac_witness[0]=3;_gz_pbac_witness[8]=_gz_pbac_frame;
 _gz_pbac_witness[9]=R16(actor,0xa7c);_gz_pbac_witness[15]=2;return 1;
}
/* No native request, replay or actor store has occurred on these paths.
 * A bad source / busy owner is a clean refusal, not a dirty lifetime fault.
 * Source corruption is not read during release; the original ledger must
 * independently authenticate this exact acquired pin first. Protocol faults
 * and every post-issue failure retain the original fatal quarantine path. */
static __attribute__((noinline,noclone)) u32 clean_refusal(u32 reason){u32 token=_gz_pbac_active,pin=_gz_pbac_source_pin,had_pin=_gz_pbac_pin_owned;
 if((reason!=41&&reason!=45&&reason!=80)||!token||token!=_gz_pbac_ack+1||
  _gz_pbac_native_issued||_gz_pbac_applied||_gz_pbac_fatal||_gz_pbac_quarantine_emergency||
  _gz_pbac_issued.valid||_gz_pbac_issued.creating)return 0;
 if(had_pin){
  if(!pin||_gz_osa_pin_id()!=pin||_gz_osa_validate(pin))return 0;
  _gz_osa_release_leaf();
 }
 _gz_pbac_pin_owned=0;_gz_pbac_source_pin=0;_gz_pbac_source_row=0;_gz_pbac_source_hash=0;
 _gz_pbac_wait_request=0;
 _gz_pbac_report[4]=3;_gz_pbac_report[5]=reason;_gz_pbac_report[7]=_gz_pbac_frame;
 _gz_pbac_report[47]=0x434c4e31u;_gz_pbac_report[48]=had_pin;_gz_pbac_report[49]=pin;
 _gz_pbac_report[50]=_gz_pbac_native_issued;
 _gz_pbac_report[3]=token;_gz_pbac_active=0;
 __asm__ volatile("sync":::"memory");_gz_pbac_ack=token;return 1;
}
static void failed(u32 reason){u32 i;if(clean_refusal(reason))return;fault(reason,0);_gz_pbac_report[4]=3;_gz_pbac_report[5]=reason;_gz_pbac_report[7]=_gz_pbac_frame;
 for(i=0;i<18;i++)_gz_pbac_report[83+i]=_gz_pbac_witness[i];
 __asm__ volatile("sync":::"memory");_gz_pbac_report[3]=_gz_pbac_active;_gz_pbac_ack=_gz_pbac_active;_gz_pbac_active=0;
}
/* All fallible ownership/readback checks precede this publication. No native
 * API, source read, yield or error branch follows the final quarantine release. */
static void committed(void){u32 i,token=_gz_pbac_active;
 _gz_pbac_witness[0]=6;_gz_pbac_report[4]=2;_gz_pbac_report[5]=0;_gz_pbac_report[7]=_gz_pbac_frame;
 _gz_pbac_report[15]=1;_gz_pbac_report[54]=PBA_DONE;_gz_pbac_report[77]=_gz_btr_report[8];
 for(i=0;i<18;i++)_gz_pbac_report[83+i]=_gz_pbac_witness[i];
 _gz_pbac_issued.completed=1;_gz_pbac_report[3]=token;_gz_pbac_active=0;
 __asm__ volatile("sync":::"memory");_gz_pbac_ack=token;
 __asm__ volatile("sync":::"memory");_gz_pbac_issued.valid=0;
}
void _gz_pbac_step(void){u32 req=_gz_pbac_request,i,z,pid,angles[2]={0},scale[3]={0x3f800000u,0x3f800000u,0x3f800000u};
 PbaQueues q=current_queues();PbaKey*k=&_gz_pbac_issued_key;Issued*s=&_gz_pbac_issued;const PbRow*row;
 if(!_gz_pbac_active){u32 p,l;
  if(!req||req==_gz_pbac_ack)return;
  _gz_pbac_active=req;
  _gz_pbac_applied=0;
  if(_gz_pbac_fatal||req!=_gz_pbac_ack+1||s->valid||s->creating){failed(55);return;}
  _gz_pbac_native_issued=0;_gz_pbac_pin_owned=0;_gz_pbac_source_pin=0;
  for(i=2;i<128;i++)_gz_pbac_report[i]=0;
  _gz_pbac_report[2]=req;_gz_pbac_report[4]=1;_gz_pbac_report[6]=_gz_pbac_frame;
  z=bomb_count(q.actor_head);if(z==BAD){failed(40);return;}
  if(q.create_head||q.delete_head||z){
   /* No source/lifecycle owner or native request exists yet. Ordinary native
    * Management must remain free to drain its work; never classify a transient
    * queue as a dirty restore or poison subsequent user operations. */
   if(_gz_pbac_wait_request!=req){_gz_pbac_wait_request=req;_gz_pbac_wait_frame=_gz_pbac_frame;_gz_pbac_wait_polls=0;_gz_pbac_wait_seen=0;}
   _gz_pbac_wait_polls++;_gz_pbac_wait_seen|=(q.create_head?1u:0u)|(q.delete_head?2u:0u)|(z?4u:0u);
   _gz_pbac_report[4]=4;_gz_pbac_report[5]=5;
   _gz_pbac_report[41]=q.create_head;_gz_pbac_report[42]=q.delete_head;_gz_pbac_report[43]=z;
   _gz_pbac_report[44]=_gz_pbac_wait_frame;_gz_pbac_report[45]=_gz_pbac_wait_polls;_gz_pbac_report[46]=_gz_pbac_wait_seen;
   _gz_pbac_active=0;
   if(_gz_pbac_frame-_gz_pbac_wait_frame>120u){
    _gz_pbac_report[4]=3;_gz_pbac_report[5]=43;_gz_pbac_report[7]=_gz_pbac_frame;_gz_pbac_report[3]=req;
    _gz_pbac_wait_request=0;__asm__ volatile("sync":::"memory");_gz_pbac_ack=req;
   }
   return;
  }
  if(_gz_pbac_wait_request==req){_gz_pbac_report[44]=_gz_pbac_wait_frame;_gz_pbac_report[45]=_gz_pbac_wait_polls;_gz_pbac_report[46]=_gz_pbac_wait_seen;_gz_pbac_wait_request=0;}
  _gz_pbac_source_slot=_gz_pbac_selected;_gz_pbac_start_epoch=_gz_pbac_epoch;
  if(_gz_pbac_source_slot>9||!_gz_pbac_start_epoch||_gz_roh_user_guard()||_gz_osa_acquire(_gz_pbac_source_slot)){failed(80);return;}
  _gz_pbac_source_pin=_gz_osa_pin_id();_gz_pbac_pin_owned=1;
  p=_gz_osa_source(_gz_pbac_source_pin);if(!p){failed(81);return;}
  _gz_pbac_report[104]=_gz_osa_native_store(_gz_pbac_source_slot);
  if(_gz_pbac_report[104]!=5||_gz_osa_validate(_gz_pbac_source_pin)){failed(82);return;}
  _gz_pbac_report[105]=_gz_pbac_source_pin;
  p=source_row(p);l=_gz_pbac_player;
  if(!p||!span(l,0x690d)||R16(l,8)!=168||R8(l,0x690c)!=0){failed(41);return;}
  row=(const PbRow*)p;
  for(i=0;i<sizeof(*k);i++)((u8*)k)[i]=0;
  k->token=req;k->slot=_gz_pbac_source_slot;k->archive=_gz_pbac_slots[_gz_pbac_source_slot];k->stage0=row->context.stage0;k->stage1=row->context.stage1;
  k->room=row->context.room;k->layer=row->context.layer;k->generation=req;k->pid=BAD;k->callback=(u32)_gz_pbac_ready_callback;
  for(i=0;i<20;i++)k->home[i]=row->home[i];
  for(i=0;i<5;i++)k->family[i]=row->resources[i+1];
  for(i=0;i<sizeof(*s);i++)((u8*)s)[i]=0;
  s->magic=0x42414931u;s->bytes=60;s->token=req;s->source_actor=0;s->source_pid=row->pid;s->source_player=l;s->start_frame=_gz_pbac_frame;
  _gz_pbac_player_pid=R32(l,4);_gz_pbac_source_row=p;_gz_pbac_source_hash=btr_hash((const void*)p,sizeof(PbRow));
  if(!context(0)||!recipe_safe(k)){failed(45);return;}
  for(i=0;i<18;i++)_gz_pbac_witness[i]=0;
  _gz_pbac_witness[0]=2;_gz_pbac_witness[1]=_gz_pbac_frame;
  _gz_pbac_report[8]=row->pid;_gz_pbac_report[14]=0;_gz_pbac_report[76]=btr_saved(row,0xa7c,2);
  for(i=0;i<6;i++)((u8*)angles)[i]=k->home[12+i];
  _gz_pbac_native_issued=1;s->creating=1;pid=_gz_pbac_native_create(294,0x80000001u,(u32)k->home,k->home[18],(u32)angles,(u32)scale,BAD,k->callback);
  s->pid=pid;s->inverse_pid=~pid;k->pid=pid;s->valid=1;__asm__ volatile("sync":::"memory");s->creating=0;
  _gz_pbac_report[10]=pid;
  if(pid==BAD||pid==s->source_pid){fault(42,1);failed(42);return;}
  _gz_pba_append_cleanup_enabled=1;q=current_queues();z=_gz_pba_arm(&_gz_pbac_tracker,k,&q);_gz_pbac_report[58]=z;
  if(z){failed(z);return;}
  return;
 }
 if(req!=_gz_pbac_active||_gz_pbac_fatal||!issued_ok()||!tracker_matches()||!context(_gz_pbac_applied?1:0)||!same_heap(&_gz_pbac_tracker)){failed(57);return;}
 if(_gz_pbac_frame-s->start_frame>120){failed(43);return;}
 if(_gz_pbac_applied){
  if(q.create_head||q.delete_head){
   if(_gz_btr_suspended_check(_gz_pbac_active)){failed(67);return;}
   _gz_pbac_report[73]++;return;
  }
  z=_gz_btr_final_check(_gz_pbac_active);if(z){failed(z);return;}
  _gz_pbac_report[108]=_gz_pbac_frame;
  if(_gz_pbac_hold_release!=_gz_pbac_active)return;
  goto finish;
 }
 if(!s->ready)return;
 q=current_queues();
 if(q.create_head||q.delete_head)return;
 if(find_actor(q.actor_head,s->pid)!=s->actor||bomb_count(q.actor_head)!=1||!actor_owned(&_gz_pbac_tracker,s->actor)||R8(s->actor,0xc)!=2||!born_free(s->actor)){failed(72);return;}
 if(_gz_pbac_frame-_gz_pbac_witness[8]<2||!_gz_pbac_witness[10])return;
 if(btr_hash((const void*)_gz_pbac_source_row,sizeof(PbRow))!=_gz_pbac_source_hash){failed(73);return;}
 z=_gz_btr_prepare(_gz_pbac_active,s->actor,s->source_player,(const void*)_gz_pbac_source_row,0);_gz_pbac_report[101]=z;
 if(z){failed(z);return;}
 z=_gz_btr_apply(_gz_pbac_active);_gz_pbac_report[102]=z;
 if(z){_gz_btr_rollback(_gz_pbac_active);failed(z);return;}
 z=_gz_btr_final_check(_gz_pbac_active);_gz_pbac_report[103]=z;
 if(z){_gz_btr_rollback(_gz_pbac_active);failed(z);return;}
 if(_gz_pbac_hold_request==_gz_pbac_active){
  _gz_pbac_applied=1;_gz_pbac_report[106]=1;_gz_pbac_report[107]=_gz_pbac_frame;return;
 }
finish:
 z=_gz_pba_commit(&_gz_pbac_tracker,_gz_pbac_active);_gz_pbac_report[74]=z;
 if(z){_gz_btr_rollback(_gz_pbac_active);failed(z);return;}
 _gz_btr_commit_leaf();_gz_osa_release_leaf();_gz_pbac_pin_owned=0;committed();
}

#include "held_bomb_prepare.c"
#include "held_bomb_probe.c"
#include "held_constructor.c"
#include "owned_views.c"
#include "mapped_preflight.c"
#include "mapped_service.c"
#include "camera_replay.c"
#include "pot_replay.c"
#include "player_core.c"
#include "player_current_probe.c"
#include "death_player_replay.c"
#include "player_face_capture.c"
#include "player_face_replay.c"
#include "death_player_face.inc"
#include "light_replay.c"
#include "persistent_projection.c"
#include "clock_replay.c"
#include "persistent_replay.c"
#include "death_persistent.c"
#include "rng_replay.c"
#include "enemy_resource.c"
#include "enemy_pattern_journal.c"
#include "enemy_pattern_target.c"
#include "enemy_pattern_service.c"
#include "enemy_pattern_replay.c"
#include "keese_body_journal.c"
#include "keese_body_binding.c"
#include "keese_body_numeric.c"
#include "keese_render_journal.c"
#include "keese_private_pool.c"
#include "keese_render_capture.c"
#include "keese_render_replay.c"
#include "keese_body_service.c"
#include "keese_body_replay.c"
#include "recovery_finish.c"
#include "death_observer.c"
#include "gameover_quiesce.c"
#include "death_hold.c"

#include "death_ui_cancel.c"
#include "death_camera_transition.c"
