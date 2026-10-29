#include "observer_native_addresses.h"
/* Private fast-child constructor/retirement exercise. The real owner holds
 * Management and Draw. A created child is bound/replayed and unbound before retirement; it is never
 * executed or drawn in this private transaction exercise.
 * This proves one lifetime participant, not a whole-world restore commit. */
#include "held_item_recipe.h"
#include "held_rope_replay.h"
enum {HIF_PREPARED=1,HIF_ISSUING=2,HIF_CREATED=3,HIF_DELETE=4,HIF_DEAD=5};
typedef struct {
 u32 magic,bytes,valid,token,frame,phase,pin,source,used,source_hash;
 u32 link,camera,link_hash,camera_hash,source_pid,actor,pid,generation,node,layer;
 u32 returned,error,recovery_arming,closure,deleted,pumped,request,last_pump;
 u32 layer_before[11],list_pointer,list_count,lists_before[96];
 u32 globals[3],hio[9],rng[3],matrix[12];
 u32 player_mode,player_started,player_returned,lpc_seal,lpf_seal,face_hash[3];
 u32 world_group,world_returned,pot_hash[64],model_hash[64];
 BcRow recipe;HrrPlan rope;
} HifReceipt;
static HifReceipt hif;
static u32 hif_seal,hif_entered;
volatile u32 _gz_hif_request,_gz_hif_ack;
u32 _gz_hif_report[64]={0x48494631u,256,1};
extern u32 _gz_hif_native_fast_create(u32,u32,u32,u32,u32,u32,u32,u32,u32);
extern u32 _gz_roh_item_roundtrip_finish(u32,u32);
extern u32 _gz_po_lookup(u32),_gz_rll_private_empty(void);
extern void _gz_rll_recovery_finish_leaf(void);
static u32 hit_plans(u32);
extern u32 _gz_hit_typed_meta_scope(u32,u32),_gz_hit_before_item_scope(u32,u32);
extern u32 _gz_rll_item_replay_actor_delta(const RllTarget*);
extern u32 _gz_roh_item_player_context(u32,u32,u32);
static void hif_seal_now(void){hif_seal=btr_hash(&hif,sizeof(hif));}
static u32 hif_sealed(u32 t){return t&&hif.magic==0x48494652u&&hif.bytes==sizeof(hif)&&hif.valid==1&&
 hif.token==t&&_gz_pbac_frame-hif.frame<=60&&hif.rope.valid==1&&hif.rope.token==t&&
 hif.rope.phase>=HRR_PREPARED&&hif.rope.phase<=HRR_UNDONE&&hif.phase>=HIF_PREPARED&&hif.phase<=HIF_DEAD&&
 hif_seal==btr_hash(&hif,sizeof(hif));}
u32 _gz_hif_has_owner(void){return hif.valid!=0;}
/* Pure metadata dispatch: no source, actor or native callback is followed. */
u32 _gz_hif_scope(u32 t){const volatile u32*o=_gz_roh_owner;
 if(!hif.valid)return 0;
 if(!hif_sealed(t)||hif.error)return 2;
 if(o[3]==ROH_CLEAN)return 0;
 return o[3]==ROH_MUTATING&&o[4]==1&&o[5]==t&&o[6]==1?1:2;
}
static u32 hif_node(u32 pid,u32*actor){u32 q=_gz_pba_native_actor_queue,n=0,found=0,a;
 *actor=0;while(q){if(++n>4096||!span(q,16))return BAD;a=R32(q,12);
  if(a){if(!span(a,0x300))return BAD;if(R32(a,4)==pid){if(found||R16(a,8)!=446)return BAD;found=q;*actor=a;}}
  q=R32(q,8);
 }return found;
}
static u32 hif_request(void){u32 r=_gz_pba_native_create_queue;
 if(!r)return 0;
 if(!span(r,0x50)||R32(r,0)||R32(r,8)||R32(r,12)!=r||R8(r,16)!=1||
  R32((u32)&_gz_pba_native_create_queue,4)!=r||R32((u32)&_gz_pba_native_create_queue,8)!=1||
  R8(r,0x15)||R32(r,0x34)!=GZ_R313_NATIVE_101F3AB8||R32(r,0x3c)!=hif.pid||
  R32(r,0x40)!=hif.actor||R32(r,0x44)!=hif.layer||R32(r,0x48)||R32(r,0x4c))return BAD;
 return r;
}
static u32 hif_delta(u32 recovery){RllTarget t;u32 a,node,r=hif_request();
 if(r==BAD)return 61;
 node=hif_node(hif.pid,&a);if(node==BAD)return 61;
 if(hif.phase==HIF_DEAD){if(a||node)return 62;}
 else if(r){if(recovery||hif.phase!=HIF_CREATED||(a&&a!=hif.actor)||node!=hif.node)return 62;}
 else if(!a||a!=hif.actor||node!=hif.node)return 62;
 bcl_zero(&t,sizeof(t));t.token=hif.token;t.kind=RLL_CREATE;t.profile=446;t.source_pid=hif.source_pid;
 t.live_pid=hif.pid;t.live_actor=a;t.actor_queue_node=node;t.key=(u32)&hif.recipe;t.key_bytes=sizeof(BcRow);
 t.observation_frame=_gz_pbac_frame;t.expected_presence=hif.phase==HIF_DEAD?RLL_ABSENT:(r?RLL_EITHER:RLL_PRESENT);
 return recovery?_gz_rll_recovery_actor_delta(&t):
  ((_gz_roh_owner[12]==2||_gz_roh_owner[12]==5||_gz_roh_owner[12]==6||_gz_roh_owner[12]==8)?_gz_rll_item_replay_actor_delta(&t):
   (_gz_roh_owner[12]?_gz_rll_check_actor_delta(&t):_gz_rll_poll_actor_delta(&t)));
}
u32 _gz_hif_validate(u32 t){const volatile u32*o=_gz_roh_owner;
 if(!hif_sealed(t)||hif.error||o[3]!=ROH_MUTATING||o[4]!=1||o[5]!=t||o[6]!=1||o[7]||
  _gz_pbac_frame-o[10]>=0x80000000u||!o[11]||(o[12]&&o[12]!=9&&!(_gz_hit_typed_meta_scope(t,_gz_pbac_frame)))||_gz_osa_pin_id()!=hif.pin||_gz_osa_validate(hif.pin)||
  _gz_osa_source(hif.pin)!=hif.source||_gz_osa_bytes(hif.pin)!=hif.used||
  btr_hash((const void*)hif.source,hif.used)!=hif.source_hash)return 60;
 if(hif.phase==HIF_PREPARED)return _gz_rll_check_unissued(t,(u32)&hif.recipe,sizeof(BcRow));
 if(hif.phase==HIF_ISSUING||!hif.returned)return 5;
 return hif_delta(0);
}
u32 _gz_hif_recovery_scope(u32 t){return hif_sealed(t)&&hif.error&&hif.returned==1&&
 hif.phase==HIF_CREATED&&hif.recovery_arming==1&&_gz_roh_owner[7]==hif.error&&
 (((hif.error==990||hif.error==991||hif.error==992)&&_gz_roh_owner[12]==9)||
  ((_gz_roh_owner[12]==2||_gz_roh_owner[12]==5||_gz_roh_owner[12]==6||_gz_roh_owner[12]==8)&&
   hif.player_started==1&&hif.player_returned==1&&hif.player_mode==3&&
   (!hif.world_group||hif.world_returned==1)&&hit_plans(t)))?1:0;}
static u32 hif_context(void){u32 z;
 if(!hif_sealed(hif.token)||!hif.returned)return 60;
 if(hif.error)z=_gz_roh_recovery_common(hif.token,_gz_pbac_frame);
 else if(hif.player_mode&&_gz_roh_owner[12]==0)z=_gz_roh_item_player_context(hif.token,_gz_pbac_frame,1)?0:60;
 else z=_gz_roh_lifetime_common(hif.token,0);
 return z?z:hif_delta(hif.error?1:0);
}
#include "held_rope_replay.c"
#include "held_player_handoff.c"
static u32 hif_bind_new(u32 returned){PoEntry e;BcRow b;u32 a,node,r,z,result;
 /* FastCreate returns a current creation-request member before ExecuteQ.
  * The request, not its returned pointer alone, supplies the allocation. */
 r=_gz_pba_native_create_queue;
 if(!span(r,0x50)||R32(r,0)||R32(r,8)||R32(r,12)!=r||R8(r,16)!=1||
  R32((u32)&_gz_pba_native_create_queue,4)!=r||R32((u32)&_gz_pba_native_create_queue,8)!=1||
  R8(r,0x15)||R32(r,0x34)!=GZ_R313_NATIVE_101F3AB8||R32(r,0x44)!=hif.layer||R32(r,0x48)||R32(r,0x4c))return 63;
 a=R32(r,0x40);
 if(a!=returned||!span(a,0x2c68)||R16(a,8)!=446||R8(a,12)!=0||
  R32(a,0x14)!=r||R32(a,4)!=R32(r,0x3c)||_gz_po_lookup(a)||!_gz_po_observation.valid)return 64;
 btr_copy(&e,&_gz_po_observation.entry,sizeof(e));
 if(e.object!=a||e.state!=PO_LIVE||e.flags||e.end_frame||!e.generation||e.pid>=0xfffffffeu||
  e.profile!=446||e.process_name!=446||e.allocation_bytes<0x2c68||
  e.allocation_bytes!=e.base_bytes+e.other_bytes||!span(a,e.allocation_bytes)||
  e.pid!=R32(a,4)||e.profile_entry!=R32(a,16)||_gz_bc_lookup(e.pid,446))return 64;
 btr_copy(&b,&_gz_bc_result,sizeof(b));
 if(!_gz_bc_row_valid(&b)||b.origin!=0x025dfab8u||b.callback||b.callback_arg||b.pid!=e.pid||
  b.profile_entry!=e.profile_entry||b.reserved[0]!=e.generation||b.reserved[1]!=a||
  b.reserved[2]!=0x46505231u||b.layer!=hif.layer)return 65;
 hif.actor=a;hif.pid=e.pid;hif.generation=e.generation;
 hif.node=hif_node(e.pid,&a);if(hif.node==BAD||(a&&a!=returned))return 61;
 _gz_hif_report[20]=returned;_gz_hif_report[21]=e.pid;_gz_hif_report[22]=e.generation;
 hif.phase=HIF_CREATED;hif_seal_now();z=_gz_hif_validate(hif.token);if(z)return z;
 z=_gz_roh_lifetime_common(hif.token,0);if(z)return z;
 r=hif_request();if(!r||r==BAD)return 63;
 result=_gz_boh_native_create_do(r);_gz_hif_report[37]=result;
 /* r may now be freed. Rediscover, require exact completion; do not execute
  * the new actor or pump any foreign creation request. */
 if(_gz_pba_native_create_queue||result!=1)return 74;
 node=hif_node(hif.pid,&a);if(node==BAD||!node||a!=hif.actor||R32(a,0x14)||R8(a,12)!=2||
  _gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.entry.state!=PO_LIVE||
  _gz_po_observation.entry.pid!=hif.pid||_gz_po_observation.entry.generation!=hif.generation)return 75;
 hif.node=node;hif_seal_now();return _gz_hif_validate(hif.token);
}
/* Recheck both current queues and the typed delete tag around native calls.
 * Saved/current audit pointers are never dereferenced after DeleteTagDo. */
static u32 hif_delete(void){u32 z,a,node,tag,result;
 z=hif_context();if(z)return z;
 if(_gz_po_lookup(hif.actor)||!_gz_po_observation.valid||_gz_po_observation.entry.state!=PO_LIVE||
  _gz_po_observation.entry.pid!=hif.pid||_gz_po_observation.entry.generation!=hif.generation)return 70;
 node=hif_node(hif.pid,&a);
 if(!a||a!=hif.actor||node!=hif.node||_gz_pba_native_create_queue||_gz_rll_priority_head||
  hif.rope.phase!=HRR_UNDONE||hif.rope.bound||hif.rope.undo_done!=1)return 66;
 if(hif.phase==HIF_CREATED){
  if(hif.deleted||_gz_pba_native_delete_queue||R32(a,0x2c)!=hif.layer)return 66;
  hif.phase=HIF_DELETE;hif.deleted=1;hif_seal_now();
  result=_gz_pbac_native_delete_actor_entry((void*)a);_gz_hif_report[25]=result;
  z=hif_context();if(z||result!=1)return z?z:67;
 }else if(hif.phase!=HIF_DELETE||hif.deleted!=1)return 66;
 node=hif_node(hif.pid,&a);tag=_gz_pba_native_delete_queue;
 if(!a||a!=hif.actor||node!=hif.node||!span(tag,28)||tag!=a+0x4c||
  R32((u32)&_gz_pba_native_delete_queue,4)!=tag||R32((u32)&_gz_pba_native_delete_queue,8)!=1||
  R32(tag,0)||R32(tag,4)!=(u32)&_gz_pba_native_delete_queue||R32(tag,8)||
  R32(tag,12)!=a||R8(tag,16)!=1||R32(tag,20)!=hif.layer)return 68;
 if(hif.last_pump==_gz_pbac_frame)return 5;
 hif.last_pump=_gz_pbac_frame;hif.pumped++;hif_seal_now();
 result=_gz_boh_native_delete_tag_do(tag,GZ_R317_NATIVE_025DDF68);_gz_hif_report[26]=result;
 _gz_hif_report[38]=hif.last_pump;_gz_hif_report[39]=hif.pumped;
 /* The retail tag has a one-frame timer. A pending return retains the owned
  * actor/tag and the same sealed operation; it never means successful undo. */
 if(result==0){z=hif_context();return z?z:5;}
 if(result!=1||_gz_pba_native_create_queue||_gz_pba_native_delete_queue||_gz_rll_priority_head)return 69;
 hif.phase=HIF_DEAD;hif_seal_now();z=hif_context();if(z)return z;
 if(_gz_po_lookup(hif.actor)||!_gz_po_observation.valid)return 70;
 if(_gz_po_observation.entry.generation!=hif.generation||_gz_po_observation.entry.pid!=hif.pid||
  _gz_po_observation.entry.state!=PO_ENDED||!_gz_po_observation.entry.end_frame)return 70;
 return 0;
}
static u32 hif_before(void){u32 i,z;
 z=hit_before();if(z)return z;
 /* Query current registries without following the old, now freed heap. */
 if(!hif.rope.heap||_gz_gh_absent(hif.rope.heap))return 355;
 if(!hif_sealed(hif.token)||hif.phase!=HIF_DEAD||!hif.deleted||!hif.pumped||
  hif.rope.phase!=HRR_UNDONE||hif.rope.bound||hif.rope.undo_done!=1||
  btr_hash((const void*)hif.link,0x8284)!=hif.link_hash||
  btr_hash((const void*)hif.camera,0xb28)!=hif.camera_hash||
  !btr_equal((const void*)GZ_R313_NATIVE_10464674,hif.globals,12)||
  !btr_equal((const void*)GZ_R313_NATIVE_1046469C,hif.hio,36)||
  !btr_equal(_gz_btr_shared_matrix,hif.matrix,48)||
  !btr_equal((const void*)hif.layer,hif.layer_before,44)||
  R32(hif.layer,16)!=hif.list_pointer||R32(hif.layer,20)!=hif.list_count||
  !btr_equal((const void*)hif.list_pointer,hif.lists_before,hif.list_count*12))return 71;
 for(i=0;i<3;i++)if(R32(GZ_OB_RNG,4*i)!=hif.rng[i])return 71;
 return 0;
}
u32 _gz_hif_finish_scope(u32 t,u32 f){return hif_sealed(t)&&f==_gz_pbac_frame&&hif.closure==1&&!hif_before();}
void _gz_hif_finish_leaf(void){fgp_finish_leaf(hif.token);bcl_zero(&hif,sizeof(hif));hif_seal=0;
 wdp_clear_private();wsp_retiring=0;
}
void _gz_hif_step(void){/* User test: private fault/roundtrip command disabled. */return;u32 req=_gz_hif_request,t=0,f=_gz_pbac_frame,z=5,i,p,dirty=0,fault=0;
 const HirPlan*r;
 if(!req||req==_gz_hif_ack||hif_entered)return;hif_entered=1;
 if(hif.valid){
  t=hif.token;dirty=1;
  if(!hif_sealed(t)||req!=hif.request||req!=_gz_hif_ack+1||hif.phase!=HIF_DELETE){z=76;goto fail;}
  if(hif.last_pump==f){hif_entered=0;return;}
  if(!hif.error){z=_gz_roh_service_begin(t,9);if(z)goto fail;}
  goto cleanup;
 }
 for(i=3;i<64;i++)_gz_hif_report[i]=0;for(i=3;i<128;i++)_gz_hit_report[i]=0;_gz_hif_report[3]=req;_gz_hif_report[4]=f;
 if(req!=_gz_hif_ack+1||_gz_roh_user_guard()||_gz_osa_pin_id()||_gz_boh_has_owner()||
  !_gz_rll_private_empty()||!wdp_empty()||!fgp_empty()||wsp_source.valid||wsp_source_seal||hif.valid||hif_seal)goto done;
 z=_gz_slots_install(_gz_pbac_selected);_gz_hif_report[6]=z;if(z)goto done;
 z=_gz_roh_begin(_gz_pbac_selected,60);_gz_hif_report[7]=z;if(z)goto done;
 t=_gz_roh_owner[5];_gz_hif_report[8]=t;
 z=_gz_roh_service_begin(t,1);_gz_hif_report[9]=z;if(z)goto abort;
 z=wsp_source_start(t,f);_gz_hif_report[10]=z;if(z)goto abort;
 z=_gz_hir_prepare_source(t,f);_gz_hif_report[11]=z;if(z)goto abort;
 z=_gz_hir_resolve_current(t,f);_gz_hif_report[12]=z;if(z)goto abort;
 r=(const HirPlan*)_gz_hir_read(t,f);if(!r){z=72;goto abort;}
 if(r->source_kind_item!=HIR_FAST||r->source_item!=0x25||r->source_profile!=446||
  r->current_kind_item!=HIR_NONE||r->current_item!=0x100||r->current_model||
  r->disposition&HIR_OTHER_KEEPS){z=73;goto abort;}
 bcl_zero(&hif,sizeof(hif));hif.magic=0x48494652u;hif.bytes=sizeof(hif);hif.valid=0;
 hif.token=t;hif.frame=f;hif.request=req;hif.phase=HIF_PREPARED;hif.pin=wsp_source.pin;
 hif.source=wsp_source.source;hif.used=wsp_source.used;hif.source_hash=wsp_source.source_hash;
 hif.link=_gz_pbac_player;hif.camera=_gz_roh_owner[37];hif.layer=_gz_rll_current_layer;
 if(!span(hif.layer,44)){z=77;goto abort;}
 hif.list_pointer=R32(hif.layer,16);hif.list_count=R32(hif.layer,20);
 if(!hif.list_count||hif.list_count>32||!span(hif.list_pointer,hif.list_count*12)){z=77;goto abort;}
 btr_copy(hif.layer_before,(const void*)hif.layer,44);
 btr_copy(hif.lists_before,(const void*)hif.list_pointer,hif.list_count*12);
 hif.link_hash=btr_hash((const void*)hif.link,0x8284);hif.camera_hash=btr_hash((const void*)hif.camera,0xb28);
 hif.source_pid=r->source_child.pid;btr_copy(&hif.recipe,&r->source_request,sizeof(BcRow));
 btr_copy(hif.globals,(const void*)GZ_R313_NATIVE_10464674,12);btr_copy(hif.hio,(const void*)GZ_R313_NATIVE_1046469C,36);
 btr_copy(hif.rng,(const void*)GZ_OB_RNG,12);btr_copy(hif.matrix,_gz_btr_shared_matrix,48);
 z=hrr_prepare(r);_gz_hif_report[57]=z;if(z)goto abort;
 z=hit_prepare(t,f);if(z)goto abort;
 if((req&15)==15){z=369;goto abort;}
 hif.valid=1;hif_seal_now();
 z=_gz_rll_prepare(t,RLL_CREATE,446,hif.source_pid,0,(u32)&hif.recipe,sizeof(BcRow));_gz_hif_report[13]=z;if(z)goto abort;
 z=_gz_roh_service_end(t,1,0);_gz_hif_report[14]=z;if(z)goto abort;
 z=_gz_roh_mark_mutating(t);_gz_hif_report[15]=z;if(z)goto abort;dirty=1;
 z=_gz_roh_service_begin(t,9);_gz_hif_report[16]=z;if(z)goto done;
 z=_gz_roh_lifetime_common(t,1);_gz_hif_report[17]=z;if(z)goto fail;
 hif.phase=HIF_ISSUING;hif_seal_now();
 p=_gz_hif_native_fast_create(446,0,(u32)(hif.recipe.append+4),BAD,0,0,BAD,0,0);
 hif.returned=1;hif_seal_now();_gz_hif_report[18]=p;
 z=hif_bind_new(p);_gz_hif_report[19]=z;if(z)goto fail;
 /* Exercise binding, a partial scalar replay and full replay failures. The
  * native actor remains exclusively owned throughout source-free undo. */
 fault=(req&15)>=2&&(req&15)<=12?(req&15)+989:0;
 if((req&15)==2)fault=991;else if((req&15)==3)fault=992;else if((req&15)==4)fault=990;
 _gz_hit_report[3]=req;_gz_hit_report[4]=f;_gz_hit_report[5]=fault;
 z=hrr_apply(fault);_gz_hif_report[51]=z;
 if(!z)z=hit_forward(t,f,fault);
 if(z&&z!=990&&z!=991&&z!=992&&!hif.player_returned)goto fail;
 hif.player_mode=HIT_UNDO;hif.player_returned=1;hif_seal_now();
 if(z){
  hif.error=z;hif.recovery_arming=1;hif_seal_now();
  _gz_hif_report[23]=_gz_roh_fail(t,z);
  z=_gz_roh_recovery_begin(t,60);_gz_hif_report[24]=z;
  hif.recovery_arming=0;hif_seal_now();if(z)goto done;
 }
 z=hit_undo(t,f);if(z)goto fail;
cleanup:
 if(hif.rope.bound){z=hrr_undo();_gz_hif_report[53]=z;if(z)goto fail;}
 z=hif_delete();_gz_hif_report[27]=z;
 if(z==5){
  if(!hif.error){z=_gz_roh_service_end(t,9,5);if(z!=5)goto fail;}
  _gz_hif_report[5]=5;_gz_hif_report[34]=_gz_roh_owner[3];_gz_hif_report[35]=_gz_roh_owner[7];
  hif_entered=0;return;
 }
 if(z)goto fail;
 /* Native HIO create/delete are attested retail no-ops; Create only writes
  * mNo plus these three rope globals. Restore them after actual retirement.
  * Native PID/process generations deliberately remain monotonic. */
 z=hif_context();if(z)goto fail;
 for(i=0;i<3;i++)*(volatile u32*)(GZ_R313_NATIVE_10464674+4*i)=hif.globals[i];
 *(volatile u8*)GZ_R313_NATIVE_1046469C=((const u8*)hif.hio)[0];
 btr_copy(_gz_btr_shared_matrix,hif.matrix,48);
 for(i=0;i<3;i++)*(volatile u32*)(GZ_OB_RNG+4*i)=hif.rng[i];
 z=hif_before();_gz_hif_report[28]=z;if(z)goto fail;
 if(!hif.error){z=_gz_roh_service_end(t,9,0);_gz_hif_report[29]=z;if(z)goto fail;}
 hif.closure=1;hif_seal_now();z=_gz_roh_item_roundtrip_finish(t,f);_gz_hif_report[30]=z;
 if(!z){_gz_hif_report[31]=1;_gz_hif_report[40]=f;_gz_hif_report[41]=f-_gz_hif_report[4];}goto done;
abort:
 if(!dirty){if(_gz_roh_owner[12]==1&&_gz_roh_owner[3]==ROH_CLEAN&&!_gz_roh_owner[7])_gz_roh_service_end(t,1,0);
  _gz_hif_report[32]=_gz_roh_abort_clean(t);if(!_gz_hif_report[32]){
   if(wsp_source.token==t){wdp_retiring=t;_gz_hit_report[40]=wdp_forget(t);}
   else _gz_hit_report[40]=wdp_empty()?0:42;
   if(!_gz_hit_report[40])_gz_hit_report[77]=fgp_forget(t);
   if(!_gz_hit_report[40]&&!_gz_hit_report[77]){_gz_rll_recovery_finish_leaf();_gz_hif_finish_leaf();}}}
 goto done;
fail:if(!hif.error)_gz_hif_report[33]=_gz_roh_fail(t,z);
done:_gz_hif_report[5]=z;_gz_hif_report[34]=_gz_roh_owner[3];_gz_hif_report[35]=_gz_roh_owner[7];
 _gz_hif_report[36]=_gz_osa_pin_id();__asm__ volatile("sync":::"memory");_gz_hif_ack=req;hif_entered=0;
}
