#include "archive_layout.h"
/* Private metadata/hold policy only. No native API or actor/game-data stores.
 * The operation record is not a whole-world scope certificate. Native caller
 * integration, work-revision coverage, typed services and recovery are external.
 */
#include "owner.h"
#include "light_state.h"
#include "world_slot.h"
#include "sand.h"
#include "inventory_bomb.h"
extern u32 _gz_b294m_archive_validate(u32,u32,u32);
extern u32 _gz_pb_validate(u32,u32);
extern u8 _gz_roh_live_pot[];
#define U32(p,o) (*(volatile u32*)((p)+(o)))
#define U16(p,o) (*(volatile u16*)((p)+(o)))
#define U8(p,o) (*(volatile u8*)((p)+(o)))
extern volatile u32 _gz_roh_live_slots[10],_gz_roh_live_bflower,_gz_roh_live_epoch,_gz_roh_live_frame;
extern volatile u32 _gz_roh_live_link,_gz_roh_live_stage[2],_gz_roh_live_camera_table[];
extern volatile u8 _gz_roh_live_room,_gz_roh_live_layer,_gz_roh_live_loading,_gz_roh_live_event,_gz_roh_live_camera_index;
extern volatile u32 _gz_roh_live_menu,_gz_roh_live_tools,_gz_roh_live_pending,_gz_roh_live_choice;
extern volatile u32 _gz_roh_live_inv,_gz_roh_live_hover,_gz_roh_live_life,_gz_roh_live_route;
extern volatile u32 _gz_roh_live_world_pending,_gz_roh_live_world_failed,_gz_roh_live_ftravel,_gz_roh_live_fieldtravel;
extern volatile u32 _gz_roh_live_fieldphase[5];
extern volatile u32 _gz_roh_live_lights,_gz_roh_live_enemies,_gz_roh_live_bridges,_gz_roh_live_camera_pending;
extern volatile u32 _gz_roh_live_actor_head,_gz_roh_live_create_head,_gz_roh_live_delete_head;
RohOwner _gz_roh_owner={.magic=ROH_MAGIC,.bytes=sizeof(RohOwner),.version=ROH_VERSION};
u32 _gz_roh_serial;
u32 _gz_roh_registry_revision=1;
/* Private current UI context only; no archive/native ABI extension. */
static u32 _gz_roh_pinned_choice;
static u32 _gz_roh_lifetime_callback;
static u32 choice_current(u32 choice){return choice<=755u&&_gz_roh_live_choice==choice;}

static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&p+n>=p&&p+n<=0xbffe0000u;}
/* First failed Link term at the actual game-thread sampling point. Private
 * diagnostic stores only; original ROH_LINK return and read guards remain. */
volatile u32 _gz_roh_link_probe[16];
static u32 link_refusal(u32 reason,u32 p,u32 m,u32 model,u32 animation){
 if(!_gz_roh_link_probe[0]){
  _gz_roh_link_probe[1]=reason;_gz_roh_link_probe[2]=_gz_roh_live_frame;
  _gz_roh_link_probe[3]=p;_gz_roh_link_probe[4]=m;_gz_roh_link_probe[5]=model;_gz_roh_link_probe[6]=animation;
  if(reason!=1){_gz_roh_link_probe[7]=U32(p,4);_gz_roh_link_probe[8]=U16(p,8);_gz_roh_link_probe[9]=U8(p,12);}
  _gz_roh_link_probe[10]=_gz_roh_live_create_head;_gz_roh_link_probe[11]=_gz_roh_live_delete_head;
  _gz_roh_link_probe[12]=_gz_roh_live_camera_index;_gz_roh_link_probe[13]=_gz_roh_live_epoch;
  _gz_roh_link_probe[14]=_gz_roh_live_choice;_gz_roh_link_probe[15]=_gz_roh_live_link;
  __asm__ volatile("sync":::"memory");_gz_roh_link_probe[0]=0x4c4e4b31u;
 }
 return ROH_LINK;
}
static u32 hash(u32 p,u32 n){u32 h=2166136261u,i;for(i=0;i<n;i++)h=(h^U8(p,i))*16777619u;return h;}
static u32 mix(u32 h,u32 z){u32 i;for(i=0;i<4;i++){h=(h^(z>>24))*16777619u;z<<=8;}return h;}
static u32 equal_bytes(u32 a,u32 b,u32 n){u32 i;for(i=0;i<n;i++)if(U8(a,i)!=U8(b,i))return 0;return 1;}
static u32 header(void){return _gz_roh_owner.magic==ROH_MAGIC&&_gz_roh_owner.bytes==sizeof(RohOwner)&&_gz_roh_owner.version==ROH_VERSION&&!_gz_roh_owner.reserved;}
static u32 idle(void){return header()&&_gz_roh_owner.phase==ROH_IDLE&&!_gz_roh_owner.hold&&!_gz_roh_owner.token&&!_gz_roh_owner.mutated&&!_gz_roh_owner.inflight;}
static u32 owned(u32 token){return header()&&token&&_gz_roh_owner.token==token&&token==_gz_roh_serial&&_gz_roh_owner.hold==1&&_gz_roh_owner.phase>=ROH_CLEAN&&_gz_roh_owner.phase<=ROH_FAILED_DIRTY;}
static u32 failed(void){return _gz_roh_owner.phase==ROH_FAILED_CLEAN||_gz_roh_owner.phase==ROH_FAILED_DIRTY;}
static u32 fault(u32 reason){
 if(!_gz_roh_owner.error)_gz_roh_owner.error=reason?reason:ROH_INVALID;
 _gz_roh_owner.phase=_gz_roh_owner.mutated?ROH_FAILED_DIRTY:ROH_FAILED_CLEAN;
 _gz_roh_owner.hold=1;return reason?reason:ROH_INVALID;
}
static u32 pending(void){return _gz_roh_live_lights|_gz_roh_live_enemies|_gz_roh_live_bridges|_gz_roh_live_camera_pending|_gz_roh_live_ftravel|_gz_roh_live_fieldtravel;}
static u32 base_quiet(void){return _gz_roh_live_menu==1&&!_gz_roh_live_tools&&!_gz_roh_live_pending&&!_gz_roh_live_inv&&!_gz_roh_live_hover&&!_gz_roh_live_life&&!_gz_roh_live_route;}
static u32 all_quiet(void){return base_quiet()&&!_gz_roh_live_world_pending&&!_gz_roh_live_world_failed&&!_gz_roh_live_fieldphase[3]&&!pending();}
static u32 archive(u32 p,RohSample* s,u32 deep){
 u32 n,o,size,c;
 if(!span(p,216))return ROH_ARCHIVE;size=U32(p,4);c=gz_arc_shape(p,size);if(!c)return ROH_ARCHIVE;
 if(U32(p,156)!=0x60000u)return ROH_ARCHIVE;
 o=U32(p,160);n=U32(p,164);
 if(n<64||n>71744)return ROH_ARCHIVE;
 s->pot_source=p+o;s->pot_bytes=n;s->pot_work=(u32)_gz_roh_live_pot;
 if(!span(s->pot_work,n)||U32(s->pot_source,0)!=0x50544231u||U32(s->pot_source,4)!=1||
  U32(s->pot_source,8)!=n||U32(s->pot_source,12)!=1||U32(s->pot_source,16)>64||
  n!=64+1120*U32(s->pot_source,16)||U32(s->pot_source,32)!=1)return ROH_ARCHIVE;
 if(deep&&_gz_pb_validate(s->pot_source,n))return ROH_ARCHIVE;
 s->pot_hash=deep?hash(s->pot_source,n):0;
 o=U32(p,168);n=U32(p,172);if(n!=sizeof(VrWorld))return ROH_ARCHIVE;
 if(deep){
  const VrWorld*w=(const VrWorld*)(p+o);
  if(_gz_vws_validate(w,n)||U32(p,76)!=16||U32(p,84)!=w->count*8||U32(p,20)!=20||
   _gz_vws_source_matches(w,(const u32*)(p+U32(p,72)),(const u32*)(p+U32(p,80)),(const u32*)(p+U32(p,16))))return ROH_ARCHIVE;
 }
 /* Current component20 is the native Molgera sand record. The historical
  * owner9 donor's Bomb294 bank is not this archive's layout. */
 o=U32(p,176);n=U32(p,180);if(n!=SD_BYTES)return ROH_ARCHIVE;
 if(deep&&_gz_sand_record_validate((const SdRecord*)(p+o),n,(const VrWorld*)(p+U32(p,168))))return ROH_ARCHIVE;
 if(U32(p,212)!=B294_BYTES||(deep&&_gz_b294m_archive_validate(p,size,0)))return ROH_ARCHIVE;
 s->archive=p;s->archive_bytes=size;s->archive_version=c;s->components=c;
 s->directory_hash=hash(p,16+c*8);s->archive_hash=deep?hash(p,size):0;return 0;
}
extern volatile u32 _gz_roh_scene_header[16];
static u32 scene_epoch_valid(void){return _gz_roh_scene_header[0]==0x53425031u&&
 _gz_roh_scene_header[1]==1&&!(_gz_roh_scene_header[4]&0x84u)&&
 _gz_roh_live_epoch&&_gz_roh_live_epoch<=0x00ffffffu;}
/* Source format only. Saved light actor/resource bindings are not live
 * replay authority; their native preflight remains a separate participant. */
static u32 light_source(RohSample*s,u32 deep){
 const L3Packet*p=(const L3Packet*)s->bank;u32 i,j;
 if(p->magic!=L3_PACKET_MAGIC||p->bytes!=sizeof(*p)||p->version!=1||p->valid!=1||p->count>8||
  p->stage0!=s->stage0||p->stage1!=s->stage1||p->room!=s->room||p->layer!=s->layer||
  p->reserved[0]||p->reserved[1]||p->reserved[2])return 0;
 s->light_header_hash=hash((u32)p,64);s->light_hash=deep?hash((u32)p,sizeof(*p)):0;
 if(deep){
  if(p->checksum!=_gz_l3_packet_hash(p))return 0;
  for(i=0;i<p->count;i++){
   if(_gz_l3_record_valid(&p->records[i]))return 0;
   for(j=0;j<i;j++)if(p->records[j].actor==p->records[i].actor||p->records[j].pid==p->records[i].pid)return 0;
  }
 }
 return 1;
}
extern u32 _gz_doh_event_guard(void);
static u32 sample_raw(u32 slot,RohSample* s,u32 deep,u32 choice,u32 life){
 u32 i,p,m,c,index,q,n=0,h=2166136261u,a,b;
 for(i=0;i<sizeof(*s)/4;i++)((u32*)s)[i]=0;
 if(slot>9)return ROH_ARCHIVE;s->slot=slot;
 s->frame=_gz_roh_live_frame;s->epoch=_gz_roh_live_epoch;
 s->stage0=_gz_roh_live_stage[0];s->stage1=_gz_roh_live_stage[1];s->room=_gz_roh_live_room;s->layer=_gz_roh_live_layer;
 s->loading=_gz_roh_live_loading;s->event=_gz_roh_live_event;
 if(!scene_epoch_valid()||!s->epoch||!s->stage0||s->loading||(s->event&&!_gz_doh_event_guard()))return ROH_SCENE;
 if(!base_quiet()||!choice_current(choice))return ROH_QUIET;
 i=archive(_gz_roh_live_slots[slot],s,deep);if(i)return i;
 b=_gz_roh_live_bflower;if(!span(b,0x60000))return ROH_REGISTRY;
 s->working_base=b;s->bank=b+0x50000;s->registry=b+0x4a600;s->registry_revision=_gz_roh_registry_revision;
 /* Actual selected component17 must be the installed source working block.
  * This is a byte comparison, not a hash masquerading as ownership proof. */
 if(U32(s->archive,156)!=0x60000u)return ROH_ARCHIVE;
 if(deep&&!equal_bytes(b,s->archive+U32(s->archive,152),0x60000))return ROH_ARCHIVE;
 if(!s->registry_revision||!light_source(s,deep))return ROH_REGISTRY;
 if(U32(s->pot_source,48)!=s->stage0||U32(s->pot_source,52)!=s->stage1||
  U32(s->pot_source,56)!=((s->room<<8)|s->layer))return ROH_ARCHIVE;
 if(deep&&!equal_bytes(s->pot_work,s->pot_source,s->pot_bytes))return ROH_ARCHIVE;
 if(deep&&!equal_bytes((u32)&_gz_vws_source,s->archive+U32(s->archive,168),sizeof(VrWorld)))return ROH_ARCHIVE;
 if(deep&&!equal_bytes((u32)&_gz_sand_saved,s->archive+U32(s->archive,176),SD_BYTES))return ROH_ARCHIVE;
 if(deep&&!equal_bytes((u32)_gz_b294_source,s->archive+U32(s->archive,208),B294_BYTES))return ROH_ARCHIVE;
 p=s->registry;
 if(U32(p,0)!=0x49444731u||U32(p,4)>512||U32(p,8)!=2||U32(p,12)>208||U32(p,16)!=s->stage0||U32(p,20)!=s->stage1||U32(p,24)!=s->room||U32(p,28)!=s->layer)return ROH_REGISTRY;
 s->registry_header_hash=hash(p,32);s->registry_hash=deep?hash(p,23008):0;
 p=_gz_roh_live_link;
 if(!span(p,0x6b00))return link_refusal(1,p,0,0,0);
 if(U16(p,8)!=168)return link_refusal(2,p,0,0,0);
 if(U8(p,12)!=2)return link_refusal(3,p,0,0,0);
 if(U32(p,4)==0xffffffffu)return link_refusal(4,p,0,0,0);
 s->link=p;s->pid=U32(p,4);s->profile=U16(p,8);m=U32(p,0x65cc);
 if(!span(m,0xb0))return link_refusal(5,p,m,0,0);
 if(!span(U32(m,0x1c),4))return link_refusal(6,p,m,U32(m,0x1c),0);
 if(!span(U32(m,0x20),4))return link_refusal(7,p,m,U32(m,0x1c),U32(m,0x20));
 s->morf=m;s->model=U32(m,0x1c);s->animation=U32(m,0x20);s->resource444=U32(p,0x444);
 index=_gz_roh_live_camera_index;if(index>1)return ROH_CAMERA_BAD;c=_gz_roh_live_camera_table[index*13];
 if(!span(c,0xa00)||U32(c,0x248)!=c||U8(c,0x24c)!=1||U8(c,0x24d)||U32(c,0x368)!=index||U32(c,0x370)!=p)return ROH_CAMERA_BAD;
 s->camera_index=index;s->camera=c;s->camera_self=U32(c,0x248);s->camera_link=U32(c,0x370);s->camera_bound_index=U32(c,0x368);
 q=_gz_roh_live_actor_head;s->actor_head=q;
 while(q){
  if(n++==4096||!span(q,16))return ROH_REGISTRY;a=U32(q,12);h=mix(h,q);h=mix(h,U32(q,8));h=mix(h,a);
  if(a){if(!span(a,0x300))return ROH_REGISTRY;h=mix(h,U32(a,4));h=mix(h,U16(a,8));h=mix(h,U32(a,0xb0));h=mix(h,U32(a,0x2e8));}
  q=U32(q,8);
 }
 s->actor_count=n;s->actor_topology_hash=h;s->create_head=_gz_roh_live_create_head;s->delete_head=_gz_roh_live_delete_head;
 /* First scope excludes all lifecycle admission, including travel births. */
 if(!life&&(s->create_head||s->delete_head))return ROH_REGISTRY;
 if(s->frame!=_gz_roh_live_frame||s->epoch!=_gz_roh_live_epoch||s->archive!=_gz_roh_live_slots[slot]||s->working_base!=_gz_roh_live_bflower||s->link!=_gz_roh_live_link||!choice_current(choice))return ROH_OWNER;
 return 0;
}
static struct {u32 open,slot,out,deep,choice,life;} owner_sample_call;
extern u32 _gz_romain_owner_sample(u32,u32,u32);
u32 _gz_som_sample_previous(u32 slot,u32 out,u32 deep){u32 z;
 if(owner_sample_call.open!=1||slot!=owner_sample_call.slot||out!=owner_sample_call.out||deep!=owner_sample_call.deep)return ROH_ARCHIVE;
 owner_sample_call.open=2;
 z=sample_raw(slot,(RohSample*)out,deep,owner_sample_call.choice,owner_sample_call.life);
 owner_sample_call.open=1;return z;
}
static u32 sample(u32 slot,RohSample*s,u32 deep,u32 choice,u32 life){u32 z;
 if(owner_sample_call.open)return ROH_CORRUPT;
 owner_sample_call.slot=slot;owner_sample_call.out=(u32)s;owner_sample_call.deep=deep;
 owner_sample_call.choice=choice;owner_sample_call.life=life;owner_sample_call.open=1;
 z=_gz_romain_owner_sample(slot,(u32)s,deep);owner_sample_call.open=0;return z;
}
static u32 same(const RohSample* a,const RohSample* b,u32 deep,u32 life){
 u32 i;for(i=0;i<sizeof(*a)/4;i++){
  if(i==6||i==12||i==32||i==41||i==43||(life&&i>=33&&i<=37))continue; /* full hashes and advancing native frame */
  if(((const u32*)a)[i]!=((const u32*)b)[i])return ROH_OWNER;
 }
 if(deep&&(a->archive_hash!=b->archive_hash||a->registry_hash!=b->registry_hash||a->pot_hash!=b->pot_hash||a->light_hash!=b->light_hash))return ROH_ARCHIVE;
 return 0;
}
/* Only the root's fresh sealed lifetime lane may replace these five
 * original topology/empty-queue comparisons. Common source/world guards stay
 * byte-identical. Callbacks are private validation only, never owner recursion. */
static u32 life_scope(u32 token,u32* active){
 u32 z;if(_gz_roh_lifetime_callback)return ROH_CORRUPT;
 _gz_roh_lifetime_callback=1;z=_gz_roh_lifetime_scope(token);_gz_roh_lifetime_callback=0;
 if(z>1)return ROH_CORRUPT;
 if(z&&(!owned(token)||_gz_roh_owner.phase!=ROH_MUTATING||_gz_roh_owner.mutated!=1))return ROH_CORRUPT;
 *active=z;return 0;
}
static u32 life_validate(u32 token){
 u32 z;if(_gz_roh_lifetime_callback)return ROH_CORRUPT;
 _gz_roh_lifetime_callback=1;z=_gz_roh_lifetime_validate(token);_gz_roh_lifetime_callback=0;return z;
}
static u32 common_validate(u32 deep,u32 check_deadline,u32 life){
 RohSample s;u32 z,delta;
 z=sample(_gz_roh_owner.sample.slot,&s,deep,_gz_roh_pinned_choice,life);if(z)return z;
 z=same(&_gz_roh_owner.sample,&s,deep,life);if(z)return z;
 if(_gz_roh_live_world_failed)return ROH_INVALID;
 if(check_deadline){delta=s.frame-_gz_roh_owner.start_frame;if(delta>=0x80000000u||delta>_gz_roh_owner.limit)return ROH_TIMEOUT;}
 return 0;
}
static u32 validate(u32 token,u32 deep,u32 check_deadline){
 u32 z,life,again,frame;
 if(!owned(token))return ROH_BUSY;
 frame=_gz_roh_live_frame;
 z=life_scope(token,&life);if(z)return z;
 /* A malformed common world never reaches the typed backend. Scope is a
  * read-only authentication callback; it performs no native or owner calls. */
 z=common_validate(deep,check_deadline,life);if(z)return z;
 if(life&&frame!=_gz_roh_live_frame)return ROH_OWNER;
 if(life){z=life_validate(token);if(z)return z;}
 z=life_scope(token,&again);if(z||again!=life)return z?z:ROH_CORRUPT;
 /* Nothing external follows this second common pass. A private receipt or
  * scope callback cannot make changed source/player/scene state acceptable. */
 if(life){z=common_validate(deep,check_deadline,life);if(z)return z;if(frame!=_gz_roh_live_frame)return ROH_OWNER;}
 return 0;
}
u32 _gz_roh_work_replaced(void){
 if(!idle())return ROH_BUSY;
 if(_gz_roh_registry_revision==0xffffffffu||!_gz_roh_registry_revision)return ROH_EXHAUSTED;
 _gz_roh_registry_revision++;return 0;
}
u32 _gz_roh_begin(u32 slot,u32 max_frames){
 RohSample s;RohOwner before;u32 z,i,choice,old_choice;
 if(!idle())return ROH_BUSY;
 if(!max_frames||max_frames>ROH_MAX_FRAMES||_gz_roh_serial==0xffffffffu)return ROH_EXHAUSTED;
 if(!all_quiet())return ROH_QUIET;
 choice=_gz_roh_live_choice;if(choice>755u)return ROH_QUIET;
 z=sample(slot,&s,1,choice,0);if(z)return z;
 if(!choice_current(choice))return ROH_OWNER; /* before pin */
 for(i=0;i<sizeof(before)/4;i++)((volatile u32*)&before)[i]=((volatile u32*)&_gz_roh_owner)[i];
 old_choice=_gz_roh_pinned_choice;_gz_roh_pinned_choice=choice;
 _gz_roh_serial++;_gz_roh_owner.token=_gz_roh_serial;
 for(i=0;i<sizeof(s)/4;i++)((volatile u32*)&_gz_roh_owner.sample)[i]=((const u32*)&s)[i];
 _gz_roh_owner.phase=ROH_CLEAN;_gz_roh_owner.mutated=0;_gz_roh_owner.error=0;
 _gz_roh_owner.limit=max_frames;_gz_roh_owner.start_frame=s.frame;_gz_roh_owner.last_frame=s.frame;
 _gz_roh_owner.frame_open=1;_gz_roh_owner.inflight=0;_gz_roh_owner.service_calls=0;
 __asm__ volatile("sync":::"memory");
 /* No external yield exists here, but recheck at the first hold publication.
  * A rejected late context leaves the prior owner unowned, byte-for-byte;
  * serial stays consumed so a rejected receipt is never reused. */
 if(!choice_current(choice)){
  for(i=0;i<sizeof(before)/4;i++)((volatile u32*)&_gz_roh_owner)[i]=((volatile u32*)&before)[i];
  _gz_roh_pinned_choice=old_choice;return ROH_OWNER;
 }
 _gz_roh_owner.hold=1;return 0;
}
u32 _gz_roh_manager_gate(void){
 u32 z;if(idle())return 0;
 if(!owned(_gz_roh_owner.token)){_gz_roh_owner.mutated=1;fault(ROH_CORRUPT);return 1;}
 if(failed())return 1;
 z=validate(_gz_roh_owner.token,0,1);if(z)fault(z);return 1;
}
u32 _gz_roh_pre_frame(void){
 u32 z,f;if(idle())return ROH_ORDINARY;
 if(!owned(_gz_roh_owner.token)){_gz_roh_owner.mutated=1;fault(ROH_CORRUPT);return ROH_RECOVERY_ONLY;}
 if(failed())return ROH_RECOVERY_ONLY;
 z=validate(_gz_roh_owner.token,0,1);if(z){fault(z);return ROH_RECOVERY_ONLY;}
 f=_gz_roh_live_frame;if(f==_gz_roh_owner.last_frame)return ROH_DUPLICATE_FRAME;
 if(f-_gz_roh_owner.last_frame>=0x80000000u){fault(ROH_TIMEOUT);return ROH_RECOVERY_ONLY;}
 if(_gz_roh_owner.inflight){fault(ROH_REENTRY);return ROH_RECOVERY_ONLY;}
 _gz_roh_owner.last_frame=f;_gz_roh_owner.frame_open=1;return ROH_OWNED_SERVICE;
}
u32 _gz_roh_user_guard(void){return idle()?0:ROH_BUSY;}
u32 _gz_roh_mark_mutating(u32 token){
 u32 z;if(!owned(token)||_gz_roh_owner.phase!=ROH_CLEAN||_gz_roh_owner.inflight)return ROH_BUSY;
 if(!_gz_roh_owner.frame_open||_gz_roh_owner.last_frame!=_gz_roh_live_frame)return fault(ROH_SERVICE);
 z=validate(token,1,1);if(z)return fault(z);
 _gz_roh_owner.mutated=1;_gz_roh_owner.phase=ROH_MUTATING;return 0;
}
extern u32 _gz_duc_service_scope(u32,u32);
u32 _gz_roh_service_begin(u32 token,u32 id){
 u32 z;if(!owned(token)||failed())return ROH_BUSY;
 if(id<ROH_OBSERVE||id>ROH_UI)return ROH_SERVICE;
 if(id==ROH_UI&&_gz_duc_service_scope(token,_gz_roh_live_frame)!=1)return ROH_SERVICE;
 if(id==ROH_LIFETIME){u32 life;z=life_scope(token,&life);if(z)return fault(z);if(life!=1)return ROH_SERVICE;}
 if(id!=ROH_OBSERVE&&(_gz_roh_owner.phase!=ROH_MUTATING||!_gz_roh_owner.mutated))return ROH_BUSY;
 if(_gz_roh_owner.inflight)return fault(ROH_REENTRY);
 if(!_gz_roh_owner.frame_open||_gz_roh_owner.last_frame!=_gz_roh_live_frame)return fault(ROH_SERVICE);
 if(_gz_roh_owner.service_calls>=ROH_MAX_SERVICES)return fault(ROH_TIMEOUT);
 z=validate(token,0,1);if(z)return fault(z);
 _gz_roh_owner.inflight=id;_gz_roh_owner.service_calls++;return 0;
}
u32 _gz_roh_service_end(u32 token,u32 id,u32 result){
 u32 z;if(!owned(token)||failed())return ROH_BUSY;
 if(!_gz_roh_owner.inflight||_gz_roh_owner.inflight!=id)return fault(ROH_REENTRY);
 if(result&&result!=5)return fault(result);
 z=validate(token,0,1);if(z)return fault(z);
 _gz_roh_owner.inflight=0;return result;
}
u32 _gz_roh_replay_ready(void){
 if(!owned(_gz_roh_owner.token)||failed()||_gz_roh_owner.phase!=ROH_MUTATING||!_gz_roh_owner.mutated||_gz_roh_owner.inflight<ROH_LIGHT||_gz_roh_owner.inflight>ROH_POT||!_gz_roh_owner.frame_open||_gz_roh_owner.last_frame!=_gz_roh_live_frame)return 0;
 return validate(_gz_roh_owner.token,0,1)?0:1;
}
u32 _gz_roh_fail(u32 token,u32 reason){if(!owned(token))return ROH_BUSY;return fault(reason);}
u32 _gz_roh_abort_clean(u32 token){
 u32 z;if(!owned(token)||_gz_roh_owner.mutated||_gz_roh_owner.inflight)return ROH_BUSY;
 z=validate(token,1,0);if(z)return fault(z);
 if(!all_quiet())return fault(ROH_QUIET);
 _gz_roh_owner.frame_open=0;_gz_roh_owner.token=0;_gz_roh_owner.phase=ROH_IDLE;
 __asm__ volatile("sync":::"memory");_gz_roh_owner.hold=0;return 0;
}
u32 _gz_roh_final_check(u32 token){
 u32 z;if(!owned(token)||failed()||_gz_roh_owner.phase!=ROH_MUTATING||!_gz_roh_owner.mutated||_gz_roh_owner.inflight)return ROH_BUSY;
 if(!_gz_roh_owner.frame_open||_gz_roh_owner.last_frame!=_gz_roh_live_frame)return ROH_SERVICE;
 if(_gz_roh_live_world_failed)return ROH_INVALID;
 if(_gz_roh_live_world_pending!=1||pending())return ROH_PENDING;
 z=validate(token,1,1);if(z)return z;
 if(_gz_roh_live_world_failed||_gz_roh_live_world_pending!=1||pending())return ROH_PENDING;
 return 0;
}
u32 _gz_roh_runtime_admit(u32 token){(void)token;return 28;}

/* Recovery reads only pinned live context and allocation identities. Source
 * corruption must not prevent undo, but no stale/freed pointer becomes safe
 * without the external Management/allocation/revision guards being wired. */
static u32 recovery_context(void){
 const RohSample*s=&_gz_roh_owner.sample;u32 p,m,c,q,a,n=0,h=2166136261u;
 if(!scene_epoch_valid()||!base_quiet()||!choice_current(_gz_roh_pinned_choice)||_gz_roh_live_loading||_gz_roh_live_event||
  _gz_roh_live_epoch!=s->epoch||_gz_roh_live_stage[0]!=s->stage0||_gz_roh_live_stage[1]!=s->stage1||
  _gz_roh_live_room!=s->room||_gz_roh_live_layer!=s->layer||s->slot>9||
  _gz_roh_live_slots[s->slot]!=s->archive||_gz_roh_live_bflower!=s->working_base||
  (u32)_gz_roh_live_pot!=s->pot_work||_gz_roh_registry_revision!=s->registry_revision||
  _gz_roh_live_create_head||_gz_roh_live_delete_head)return 0;
 p=_gz_roh_live_link;
 if(p!=s->link||!span(p,0x6b00)||U16(p,8)!=168||U8(p,12)!=2||U32(p,4)!=s->pid||U32(p,0x444)!=s->resource444)return 0;
 m=U32(p,0x65cc);if(m!=s->morf||!span(m,0xb0)||U32(m,0x1c)!=s->model||U32(m,0x20)!=s->animation)return 0;
 if(_gz_roh_live_camera_index!=s->camera_index)return 0;
 c=_gz_roh_live_camera_table[s->camera_index*13];
 if(c!=s->camera||!span(c,0xa00)||U32(c,0x248)!=c||U8(c,0x24c)!=1||U8(c,0x24d)||
  U32(c,0x368)!=s->camera_index||U32(c,0x370)!=p)return 0;
 q=_gz_roh_live_actor_head;if(q!=s->actor_head)return 0;
 while(q){
  if(n++==4096||!span(q,16))return 0;a=U32(q,12);h=mix(h,q);h=mix(h,U32(q,8));h=mix(h,a);
  if(a){if(!span(a,0x300))return 0;h=mix(h,U32(a,4));h=mix(h,U16(a,8));h=mix(h,U32(a,0xb0));h=mix(h,U32(a,0x2e8));}
  q=U32(q,8);
 }
 return n==s->actor_count&&h==s->actor_topology_hash&&_gz_roh_live_epoch==s->epoch&&
  _gz_roh_live_frame==_gz_roh_owner.last_frame&&_gz_roh_live_link==s->link&&choice_current(_gz_roh_pinned_choice);
}
u32 _gz_roh_pot_authority(u32 token,u32 source,u32 bytes,u32 frame,u32 access){
 const RohSample*s=&_gz_roh_owner.sample;
 if(!owned(token)||access>3||!_gz_roh_owner.frame_open||frame!=_gz_roh_live_frame||
  frame!=_gz_roh_owner.last_frame||source!=s->pot_source||bytes!=s->pot_bytes)return 0;
 if(access==2){
  if(!_gz_roh_owner.mutated||(_gz_roh_owner.phase!=ROH_MUTATING&&_gz_roh_owner.phase!=ROH_FAILED_DIRTY))return 0;
  return recovery_context()?1:0;
 }
 if(failed())return 0;
 if(access==0){if(_gz_roh_owner.phase!=ROH_CLEAN||_gz_roh_owner.mutated||_gz_roh_owner.inflight!=ROH_OBSERVE)return 0;}
 else{
  if(_gz_roh_owner.phase!=ROH_MUTATING||!_gz_roh_owner.mutated)return 0;
  if(access==1&&_gz_roh_owner.inflight!=ROH_POT)return 0;
  if(access==3&&(_gz_roh_owner.inflight||_gz_roh_live_world_failed||_gz_roh_live_world_pending!=1||pending()))return 0;
 }
 return validate(token,0,1)?0:1;
}

/* Read-only same-frame pre-SDK gate. This deliberately never calls lifetime
 * scope/validation callbacks: it may be used by their private root bridge
 * without recursing. It certifies ONLY original common owner/world context;
 * fresh typed backend/key/queue/lane/closure checks remain separate. */
u32 _gz_roh_lifetime_common(u32 token,u32 deep){
 u32 z,frame;
 if(deep>1)return ROH_INVALID;
 if(!owned(token)||failed()||_gz_roh_owner.phase!=ROH_MUTATING||_gz_roh_owner.mutated!=1)return ROH_BUSY;
 frame=_gz_roh_live_frame;
 if(!_gz_roh_owner.frame_open||_gz_roh_owner.inflight!=ROH_LIFETIME||_gz_roh_owner.last_frame!=frame)return ROH_SERVICE;
 z=common_validate(deep,1,1);if(z)return z;
 if(!owned(token)||failed()||_gz_roh_owner.phase!=ROH_MUTATING||_gz_roh_owner.mutated!=1)return ROH_BUSY;
 if(frame!=_gz_roh_live_frame||!_gz_roh_owner.frame_open||_gz_roh_owner.inflight!=ROH_LIFETIME||_gz_roh_owner.last_frame!=frame)return ROH_SERVICE;
 return 0;
}

/* Explicit completed-lifetime handoff to an ordinary typed service. This
 * preserves the original common source/world checks, and never invokes the
 * lifetime callbacks recursively. It grants no constructor/SDK permission. */
extern u32 _gz_bmh_scope(u32,u32);
u32 _gz_roh_replay_common(u32 token,u32 frame,u32 deep){u32 z,id;
 if(deep>1)return ROH_INVALID;
 if(!owned(token)||failed()||_gz_roh_owner.phase!=ROH_MUTATING||_gz_roh_owner.mutated!=1)return ROH_BUSY;
 id=_gz_roh_owner.inflight;
 if(id<ROH_LIGHT||id>ROH_POT||!_gz_roh_owner.frame_open||
  frame!=_gz_roh_live_frame||_gz_roh_owner.last_frame!=frame||!_gz_bmh_scope(token,frame))return ROH_SERVICE;
 z=common_validate(deep,1,1);if(z)return z;
 if(!owned(token)||failed()||_gz_roh_owner.phase!=ROH_MUTATING||_gz_roh_owner.mutated!=1||
  _gz_roh_owner.inflight!=id||!_gz_roh_owner.frame_open||frame!=_gz_roh_live_frame||
  _gz_roh_owner.last_frame!=frame||!_gz_bmh_scope(token,frame))return ROH_SERVICE;
 return 0;
}

/* Failure-aware private ticket. It never rewrites the failed owner receipt or
 * consults the immutable source body. The independent deadline cannot renew
 * for the same owner serial; a new authenticated owner may replace a sealed
 * ticket from a strictly older serial after that prior operation retired. */
extern u32 _gz_boh_recovery_scope(u32 token);
typedef struct {u32 magic,bytes,valid,token,start,limit,owner_hash,choice;} RohRecovery;
static RohRecovery roh_recovery;
static u32 roh_recovery_seal,roh_recovery_entered;
static u32 recovery_used(void){u32 i;
 if(roh_recovery_seal)return 1;
 for(i=0;i<sizeof(roh_recovery)/4;i++)if(((const u32*)&roh_recovery)[i])return 1;
 return 0;
}
static u32 recovery_sealed(void){
 return roh_recovery.magic==0x52524331u&&roh_recovery.bytes==sizeof(RohRecovery)&&
  roh_recovery.valid==1&&roh_recovery.token&&roh_recovery.limit&&roh_recovery.limit<=ROH_MAX_FRAMES&&
  roh_recovery_seal==hash((u32)&roh_recovery,sizeof(roh_recovery));
}
/* Read-only original allocation identities. Topology and source contents are
 * deliberately absent; a separate sealed typed lane owns their comparison. */
static u32 recovery_live_context(u32 token,u32 frame){
 const RohSample*s=&_gz_roh_owner.sample;u32 p,m,c;
 if(!owned(token)||_gz_roh_owner.phase!=ROH_FAILED_DIRTY||_gz_roh_owner.mutated!=1||
  !_gz_roh_owner.error||frame!=_gz_roh_live_frame||!scene_epoch_valid()||!base_quiet()||
  !choice_current(_gz_roh_pinned_choice)||_gz_roh_live_loading||_gz_roh_live_event||
  _gz_roh_live_epoch!=s->epoch||_gz_roh_live_stage[0]!=s->stage0||_gz_roh_live_stage[1]!=s->stage1||
  _gz_roh_live_room!=s->room||_gz_roh_live_layer!=s->layer||s->slot>9||s->camera_index>1||
  _gz_roh_live_slots[s->slot]!=s->archive||_gz_roh_live_bflower!=s->working_base||
  (u32)_gz_roh_live_pot!=s->pot_work||_gz_roh_registry_revision!=s->registry_revision||
  !span(s->working_base,0x60000)||s->bank!=s->working_base+0x50000||
  s->registry!=s->working_base+0x4a600||!s->registry_revision)return 0;
 p=_gz_roh_live_link;
 if(p!=s->link||!span(p,0x6b00)||s->profile!=168||U16(p,8)!=168||U8(p,12)!=2||
  U32(p,4)!=s->pid||U32(p,0x444)!=s->resource444)return 0;
 m=U32(p,0x65cc);
 if(m!=s->morf||!span(m,0xb0)||U32(m,0x1c)!=s->model||U32(m,0x20)!=s->animation)return 0;
 if(_gz_roh_live_camera_index!=s->camera_index)return 0;
 c=_gz_roh_live_camera_table[s->camera_index*13];
 if(c!=s->camera||s->camera_self!=c||s->camera_link!=p||s->camera_bound_index!=s->camera_index||
  !span(c,0xa00)||U32(c,0x248)!=c||U8(c,0x24c)!=1||U8(c,0x24d)||
  U32(c,0x368)!=s->camera_index||U32(c,0x370)!=p)return 0;
 return frame==_gz_roh_live_frame&&owned(token)&&_gz_roh_owner.phase==ROH_FAILED_DIRTY&&
  _gz_roh_owner.mutated==1&&choice_current(_gz_roh_pinned_choice);
}
u32 _gz_roh_recovery_begin(u32 token,u32 max_frames){
 u32 frame,h,z,i;RohRecovery next;
 if(roh_recovery_entered||!max_frames||max_frames>ROH_MAX_FRAMES)return ROH_BUSY;
 if(recovery_used()&&(!recovery_sealed()||token<=roh_recovery.token))return ROH_BUSY;
 frame=_gz_roh_live_frame;
 if(!recovery_live_context(token,frame))return ROH_OWNER;
 h=hash((u32)&_gz_roh_owner,sizeof(_gz_roh_owner));
 roh_recovery_entered=1;z=_gz_boh_recovery_scope(token);roh_recovery_entered=0;
 if(z!=1||!recovery_live_context(token,frame)||h!=hash((u32)&_gz_roh_owner,sizeof(_gz_roh_owner)))return ROH_OWNER;
 next.magic=0x52524331u;next.bytes=sizeof(next);next.valid=1;next.token=token;
 next.start=frame;next.limit=max_frames;next.owner_hash=h;next.choice=_gz_roh_pinned_choice;
 for(i=0;i<sizeof(next)/4;i++)((u32*)&roh_recovery)[i]=((const u32*)&next)[i];
 roh_recovery_seal=hash((u32)&roh_recovery,sizeof(roh_recovery));
 return 0;
}
u32 _gz_roh_recovery_common(u32 token,u32 frame){
 u32 delta;
 if(roh_recovery_entered||!recovery_sealed()||roh_recovery.token!=token)return ROH_OWNER;
 delta=frame-roh_recovery.start;
 if(delta>=0x80000000u||delta>roh_recovery.limit)return ROH_TIMEOUT;
 if(roh_recovery.choice!=_gz_roh_pinned_choice||
  roh_recovery.owner_hash!=hash((u32)&_gz_roh_owner,sizeof(_gz_roh_owner))||
  !recovery_live_context(token,frame))return ROH_OWNER;
 return recovery_sealed()&&frame==_gz_roh_live_frame&&
  roh_recovery.owner_hash==hash((u32)&_gz_roh_owner,sizeof(_gz_roh_owner))?0:ROH_OWNER;
}
u32 _gz_roh_recovery_baseline(u32 token,u32 frame){
 const RohSample*s=&_gz_roh_owner.sample;u32 q,a,n=0,h=2166136261u,z;
 z=_gz_roh_recovery_common(token,frame);if(z)return z;
 if(_gz_roh_live_create_head||_gz_roh_live_delete_head)return ROH_REGISTRY;
 q=_gz_roh_live_actor_head;if(q!=s->actor_head)return ROH_REGISTRY;
 while(q){
  if(n++==4096||!span(q,16))return ROH_REGISTRY;
  a=U32(q,12);h=mix(h,q);h=mix(h,U32(q,8));h=mix(h,a);
  if(a){if(!span(a,0x300))return ROH_REGISTRY;h=mix(h,U32(a,4));h=mix(h,U16(a,8));h=mix(h,U32(a,0xb0));h=mix(h,U32(a,0x2e8));}
  q=U32(q,8);
 }
 if(n!=s->actor_count||h!=s->actor_topology_hash||_gz_roh_live_actor_head!=s->actor_head||
  _gz_roh_live_create_head||_gz_roh_live_delete_head)return ROH_REGISTRY;
 return _gz_roh_recovery_common(token,frame);
}

/* A retired FAILED_DIRTY receipt is audit history, not a new live owner. This
 * path only resumes the pre-attempt world after a complete verified rollback.
 * Runtime snapshot admission and successful commit remain separate. */
typedef struct {
 u32 magic,bytes,version,valid,token,frame,error,pin,closure_hash,owner_hash;
 RohOwner owner;
} RohRecovered;
typedef char RohRecoveredSize[sizeof(RohRecovered)==280?1:-1];
RohRecovered _gz_roh_recovered;
u32 _gz_roh_recovered_seal;
static u32 roh_finish_entered;
extern u32 _gz_boh_recovery_finish_scope(u32,u32),_gz_som_recovery_finish_check(u32);
extern u32 _gz_bmh_aborted_scope(u32);
extern u32 _gz_boh_finish_seal,_gz_osa_pin_id(void);
extern void _gz_boh_recovery_finish_leaf(void),_gz_som_recovery_finish_leaf(void),_gz_rll_recovery_finish_leaf(void);
static u32 recovered_used(void){u32 i;if(_gz_roh_recovered_seal)return 1;
 for(i=0;i<sizeof(RohRecovered)/4;i++)if(((const u32*)&_gz_roh_recovered)[i])return 1;return 0;
}
static u32 recovered_sealed(void){const RohRecovered*r=&_gz_roh_recovered;
 return r->magic==0x52464331u&&r->bytes==sizeof(*r)&&r->version==1&&r->valid==1&&r->token&&r->error&&r->pin&&
  r->closure_hash&&r->owner.token==r->token&&r->owner.error==r->error&&r->owner.phase==ROH_FAILED_DIRTY&&
  r->owner.hold==1&&r->owner.mutated==1&&r->owner_hash==hash((u32)&r->owner,sizeof(RohOwner))&&
  _gz_roh_recovered_seal==hash((u32)r,sizeof(*r));
}
u32 _gz_roh_recovery_finish(u32 token,u32 frame){RohRecovered*r=&_gz_roh_recovered;u32 z,i,h,c,pin,committed;
 if(roh_finish_entered)return ROH_BUSY;
 if(recovered_used()&&(!recovered_sealed()||token<=r->token))return ROH_OWNER;
 roh_finish_entered=1;
 z=_gz_roh_recovery_baseline(token,frame);if(z)goto denied;
 /* Keep the failed active ID intact until all physical before-images and
  * native retirement are certified. Only an authenticated returned callback
  * may retire a nonzero ID; unknown/partial constructors remain denied. */
 if(_gz_roh_owner.inflight&&_gz_bmh_aborted_scope(token)!=1){z=ROH_SERVICE;goto denied;}
 h=hash((u32)&_gz_roh_owner,sizeof(RohOwner));pin=_gz_osa_pin_id();
 if(!pin){z=ROH_OWNER;goto denied;}
 z=_gz_boh_recovery_finish_scope(token,frame);if(z!=1){z=ROH_OWNER;goto denied;}
 c=_gz_boh_finish_seal;if(!c){z=ROH_OWNER;goto denied;}
 z=_gz_som_recovery_finish_check(token);if(z)goto denied;
 z=_gz_roh_recovery_baseline(token,frame);if(z)goto denied;
 if(h!=hash((u32)&_gz_roh_owner,sizeof(RohOwner))||c!=_gz_boh_finish_seal||pin!=_gz_osa_pin_id()||
  _gz_boh_recovery_finish_scope(token,frame)!=1){z=ROH_OWNER;goto denied;}
 /* All fallible/native/source reads end above. These three leaf helpers have
  * fixed private destinations; none calls a native SDK, follows the archive,
  * frees its allocation or publishes a generic world-resume permission. */
 r->valid=0;r->magic=0x52464331u;r->bytes=sizeof(*r);r->version=1;r->token=token;r->frame=frame;
 r->error=_gz_roh_owner.error;r->pin=pin;r->closure_hash=c;r->owner_hash=h;
 for(i=0;i<sizeof(RohOwner)/4;i++)((u32*)&r->owner)[i]=((const u32*)&_gz_roh_owner)[i];
 r->valid=1;_gz_roh_recovered_seal=hash((u32)r,sizeof(*r));
 committed=_gz_roh_owner.committed_token;
 _gz_som_recovery_finish_leaf();_gz_boh_recovery_finish_leaf();_gz_rll_recovery_finish_leaf();
 for(i=0;i<sizeof(roh_recovery)/4;i++)((u32*)&roh_recovery)[i]=0;
 roh_recovery_seal=0;
 for(i=3;i<sizeof(RohOwner)/4;i++)if(i!=4)((u32*)&_gz_roh_owner)[i]=0;
 _gz_roh_owner.committed_token=committed;
 __asm__ volatile("sync":::"memory");_gz_roh_owner.hold=0;
 roh_finish_entered=0;return 0;
denied:roh_finish_entered=0;return z;
}
