/* Resolve native fast-created item children from the genuine pinned archive
 * and current observed processes. This is a read-only transaction planner,
 * not an exemption from ActorKeep guards or a raw-pointer restore. */
#include "held_item_recipe.h"
#define H32(p,o) (*(volatile const u32*)((p)+(o)))
#define H16(p,o) (*(volatile const u16*)((p)+(o)))
#define H8(p,o) (*(volatile const u8*)((p)+(o)))
extern u32 _gz_srd_read_player(u32,u32);
extern volatile u32 _gz_pbac_player,_gz_pbac_frame,_gz_pba_native_actor_queue;
extern volatile u32 _gz_roh_live_frame;
extern volatile u8 _gz_roh_live_stage[],_gz_roh_live_room,_gz_roh_live_layer;
HirPlan _gz_hir_result;
static u32 _gz_hir_seal,_gz_hir_entered;
static u32 hir_span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static void hir_copy(void*d,const void*s,u32 n){u32 i;for(i=0;i<n;i++)((u8*)d)[i]=((const volatile u8*)s)[i];}
static u32 hir_same(const void*a,const void*b,u32 n){u32 i;for(i=0;i<n;i++)if(((const volatile u8*)a)[i]!=((const volatile u8*)b)[i])return 0;return 1;}
static u32 hir_hash(const void*p,u32 n){u32 i,h=2166136261u;for(i=0;i<n;i++)h=(h^((const volatile u8*)p)[i])*16777619u;return h;}
static u32 hir_kind(u32 item,u32*profile){
 *profile=0xffffffffu;
 if(item==0x100u)return HIR_NONE;
 if(item==0x103u)return HIR_MODEL;
 /* Native Link makeItemType: fastCreate then native ActorKeep binding. */
 if(item==0x25u)*profile=446;
 else if(item==0x2fu)*profile=169;
 else if(item==0x2du)*profile=432;
 return *profile==0xffffffffu?0xffffffffu:HIR_FAST;
}
static u32 hir_source_live(u32 token,u32 frame){const HirPlan*p=&_gz_hir_result;
 /* Authenticate the original pin BEFORE reading any saved allocation. */
 if(!token||p->magic!=HIR_MAGIC||p->bytes!=sizeof(*p)||p->version!=1||p->valid!=1||p->token!=token||p->frame!=frame||
  _gz_hir_seal!=hir_hash(p,sizeof(*p))||_gz_srd_kind(token,frame)!=p->source_kind||_gz_srd_serial(token,frame)!=p->serial)return 0;
 if(_gz_srd_read_player(token,frame)!=p->source_player||_gz_srd_read_world(token,frame)!=p->source_world||
  _gz_srd_read_bank(token,frame)!=p->source_bank)return 0;
 return hir_hash((const void*)p->source_player,8192)==p->source_hash&&H32(p->source_world,72)==p->world_hash&&
  _gz_srd_kind(token,frame)==p->source_kind&&_gz_srd_serial(token,frame)==p->serial;
}
/* Fast receipt is tied to the captured process generation. No stale object,
 * profile-entry, callback, or append_audit pointer is followed. */
static u32 hir_fast(const PoEntry*e,const BcRow*r,u32 profile){return
 e->state==PO_LIVE&&!e->flags&&!e->end_frame&&e->generation&&e->process_name==profile&&e->profile==profile&&
 e->pid<0xfffffffeu&&e->allocation_bytes==e->base_bytes+e->other_bytes&&e->allocation_bytes>=e->base_bytes&&
 e->allocation_bytes>=0x3acu&&e->allocation_bytes<=0x400000u&&
 _gz_bc_row_valid(r)&&r->origin==0x025dfab8u&&r->pid==e->pid&&r->profile==profile&&r->profile_entry==e->profile_entry&&
 !r->callback&&!r->callback_arg&&r->reserved[0]==e->generation&&r->reserved[1]==e->object&&r->reserved[2]==0x46505231u;}
u32 _gz_hir_prepare_source(u32 token,u32 frame){HirPlan*p=&_gz_hir_result;const VrWorld*w;const BbBank*b;const BbEntry*e;u32 a,c,i,k,n=0,z=HIR_SOURCE;
 if(_gz_hir_entered)return HIR_CHANGED;
 _gz_hir_entered=1;_gz_hir_seal=0;p->valid=0;
 k=_gz_srd_kind(token,frame);c=_gz_srd_serial(token,frame);if(!k||!c)goto done;
 a=_gz_srd_read_player(token,frame);w=(const VrWorld*)_gz_srd_read_world(token,frame);b=(const BbBank*)_gz_srd_read_bank(token,frame);
 if(!a||!w||!b||H32(a,0)!=1||H32(a,4)!=w->link_pid||H32(a,8)!=w->link_actor)goto done;
 for(i=0;i<sizeof(*p)/4;i++)((u32*)p)[i]=0;
 p->magic=HIR_MAGIC;p->bytes=sizeof(*p);p->version=1;p->token=token;p->frame=frame;p->source_kind=k;p->serial=c;
 p->source_player=a;p->source_world=(u32)w;p->source_bank=(u32)b;p->source_hash=hir_hash((const void*)a,8192);p->world_hash=w->checksum;
 p->source_item=H32(a,108);p->source_model=H32(a,36);p->source_kind_item=hir_kind(p->source_item,&p->source_profile);
 p->source_link=w->link_actor;p->source_link_pid=w->link_pid;p->source_index=0xffffffffu;p->current_index=0xffffffffu;
 if(p->source_kind_item==0xffffffffu){z=HIR_ITEM;goto done;}
 for(i=0;i<4;i++){p->source_keeps[i]=H32(a,52+4*i);if(i&&p->source_keeps[i]!=0xffffffffu)p->disposition|=HIR_OTHER_KEEPS;}
 for(i=0;i<w->count;i++)if(w->actors[i].pid==w->link_pid){
  if(++n!=1||w->actors[i].actor!=w->link_actor||w->actors[i].profile!=168){z=HIR_SOURCE;goto done;}
  p->source_link_generation=b->rows[i].process.generation;
 }
 if(n!=1){z=HIR_SOURCE;goto done;}
 if(p->source_kind_item==HIR_FAST){
  n=0;
  for(i=0;i<w->count;i++)if(w->actors[i].pid==p->source_keeps[0]){
   e=&b->rows[i];if(++n!=1||w->actors[i].profile!=p->source_profile||!hir_fast(&e->process,&e->request,p->source_profile)){z=HIR_RECIPE;goto done;}
   p->source_index=i;hir_copy(&p->source_child,&e->process,sizeof(PoEntry));hir_copy(&p->source_request,&e->request,sizeof(BcRow));
  }
  if(n!=1){z=HIR_KEEP;goto done;}
  p->disposition|=HIR_CHILD_REPLAY;
 }else if(p->source_keeps[0]!=0xffffffffu){z=HIR_KEEP;goto done;}
 /* No consumer may turn the opaque source model address into an item model. */
 if((p->source_kind_item==HIR_NONE&&p->source_model)||(p->source_kind_item==HIR_FAST&&p->source_model)){z=HIR_ITEM;goto done;}
 if(_gz_srd_read_player(token,frame)!=a||_gz_srd_read_world(token,frame)!=(u32)w||_gz_srd_read_bank(token,frame)!=(u32)b||
  _gz_srd_kind(token,frame)!=k||_gz_srd_serial(token,frame)!=c||hir_hash((const void*)a,8192)!=p->source_hash){z=HIR_CHANGED;goto done;}
 p->valid=1;_gz_hir_seal=hir_hash(p,sizeof(*p));z=0;
done:if(z){p->valid=0;_gz_hir_seal=0;}_gz_hir_entered=0;return z;
}
static u32 hir_live_process(u32 actor,u32 minimum,PoEntry*out){u32 q=_gz_pba_native_actor_queue,n=0,steps=0;
 if(!hir_span(actor,minimum)||_gz_po_lookup(actor)||!_gz_po_observation.valid||_gz_po_observation.flags)return HIR_PROCESS;
 hir_copy(out,&_gz_po_observation.entry,sizeof(*out));
 if(out->object!=actor||out->state!=PO_LIVE||out->flags||out->end_frame||!out->generation||out->pid>=0xfffffffeu||
  out->allocation_bytes<minimum||out->allocation_bytes>0x400000u||out->allocation_bytes!=out->base_bytes+out->other_bytes||
  out->allocation_bytes<out->base_bytes||!hir_span(actor,out->allocation_bytes)||out->pid!=H32(actor,4)||
  out->process_name!=H16(actor,8)||H8(actor,12)!=2||out->profile!=H16(actor,14)||out->profile_entry!=H32(actor,16))return HIR_PROCESS;
 while(q){if(++steps>4096||!hir_span(q,16))return HIR_PROCESS;if(H32(q,12)==actor)n++;q=H32(q,8);}
 return n==1?0:HIR_PROCESS;
}
u32 _gz_hir_resolve_current(u32 token,u32 frame){HirPlan*p=&_gz_hir_result;const VrWorld*w;PoEntry e;u32 a,i,z=HIR_SOURCE,serial;
 if(_gz_hir_entered||!hir_source_live(token,frame))return HIR_SOURCE;
 _gz_hir_entered=1;p->current_valid=0;
 p->disposition&=HIR_CHILD_REPLAY|HIR_OTHER_KEEPS;
 w=(const VrWorld*)p->source_world;
 if(H32((u32)_gz_roh_live_stage,0)!=w->stage0||H32((u32)_gz_roh_live_stage,4)!=w->stage1||_gz_roh_live_room!=w->room||_gz_roh_live_layer!=w->layer){z=HIR_SCENE;goto done;}
 serial=_gz_po_header.serial;if(!_gz_po_header.serial||_gz_po_header.busy||_gz_po_header.flags){z=HIR_PROCESS;goto done;}
 a=_gz_pbac_player;z=hir_live_process(a,0x8284,&p->current_link);if(z)goto done;
 if(p->current_link.process_name!=168){z=HIR_PROCESS;goto done;}
 p->current_item=H16(a,0x69b0);p->current_model=H32(a,0x4440);p->current_kind_item=hir_kind(p->current_item,&p->current_profile);
 if(p->current_kind_item==0xffffffffu){z=HIR_ITEM;goto done;}
 for(i=0;i<4;i++){p->current_keeps[i]=H32(a,0x6590+i*8);p->current_keep_actors[i]=H32(a,0x6594+i*8);if(i&&p->current_keeps[i]!=0xffffffffu)p->disposition|=HIR_OTHER_KEEPS;}
 if(p->current_kind_item==HIR_FAST){
  if(p->current_keeps[0]==0xffffffffu||!p->current_keep_actors[0]){z=HIR_KEEP;goto done;}
  z=hir_live_process(p->current_keep_actors[0],0x3ac,&e);if(z)goto done;
  if(e.pid!=p->current_keeps[0]||_gz_bc_lookup(e.pid,p->current_profile)||!hir_fast(&e,&_gz_bc_result,p->current_profile)){z=HIR_RECIPE;goto done;}
  hir_copy(&p->current_child,&e,sizeof(e));hir_copy(&p->current_request,&_gz_bc_result,sizeof(BcRow));
 }else{
  if(p->current_keeps[0]!=0xffffffffu||p->current_keep_actors[0]){z=HIR_KEEP;goto done;}
  for(i=0;i<sizeof(p->current_child)/4;i++)((u32*)&p->current_child)[i]=0;
  for(i=0;i<sizeof(p->current_request)/4;i++)((u32*)&p->current_request)[i]=0;
 }
 if(p->source_kind_item==HIR_FAST){
  if(p->current_kind_item!=HIR_FAST||p->current_profile!=p->source_profile)p->disposition|=HIR_CREATE;
  else if(p->source_child.object!=p->current_child.object||p->source_child.pid!=p->current_child.pid||p->source_child.generation!=p->current_child.generation)p->disposition|=HIR_REBIND;
 }
 if(p->current_kind_item==HIR_FAST&&(p->source_kind_item!=HIR_FAST||p->source_profile!=p->current_profile))p->disposition|=HIR_RETIRE;
 if(p->source_kind_item!=p->current_kind_item||p->source_item!=p->current_item||p->source_model!=p->current_model)p->disposition|=HIR_MODEL_CHANGE;
 /* Recheck actual observed lifetimes and the original pin after all reads.
  * This remains serialized observation, not an allocation lease. */
 z=hir_live_process(a,0x8284,&e);if(z||!hir_same(&e,&p->current_link,sizeof(e))){z=HIR_CHANGED;goto done;}
 if(p->current_kind_item==HIR_FAST){z=hir_live_process(p->current_child.object,0x3ac,&e);if(z||!hir_same(&e,&p->current_child,sizeof(e))){z=HIR_CHANGED;goto done;}}
 if(serial!=_gz_po_header.serial||_gz_po_header.busy||_gz_po_header.flags||_gz_roh_live_frame!=frame||_gz_pbac_frame!=frame||
  _gz_srd_kind(token,frame)!=p->source_kind||_gz_srd_serial(token,frame)!=p->serial){z=HIR_CHANGED;goto done;}
 p->process_serial=serial;p->current_valid=1;z=0;
done:_gz_hir_seal=hir_hash(p,sizeof(*p));_gz_hir_entered=0;return z;
}
u32 _gz_hir_read(u32 token,u32 frame){const HirPlan*p=&_gz_hir_result;
 if(_gz_hir_entered||!hir_source_live(token,frame))return 0;
 if(p->current_valid&&(p->process_serial!=_gz_po_header.serial||_gz_po_header.busy||_gz_po_header.flags))return 0;
 return (u32)p;
}

/* Private diagnostic dispatch: one original READ pin, no installed-copy edits,
 * constructors, item changes, replay writes, or new restore admission. */
volatile u32 _gz_hir_request,_gz_hir_ack;
u32 _gz_hir_report[32]={0x48495254u,128};
void _gz_hir_step(void){u32 req=_gz_hir_request,frame=_gz_roh_live_frame,t=0,z,i;
 if(!req||req==_gz_hir_ack)return;
 for(i=2;i<32;i++)_gz_hir_report[i]=0;
 _gz_hir_report[2]=req;_gz_hir_report[3]=frame;
 if(req!=_gz_hir_ack+1){_gz_hir_report[4]=5;goto done;}
 z=_gz_bsr_begin(_gz_slots_selected,frame);_gz_hir_report[4]=z;if(z)goto done;
 t=_gz_bsr_token();_gz_hir_report[5]=t;
 _gz_hir_report[6]=_gz_hir_prepare_source(t,frame);
 if(!_gz_hir_report[6])_gz_hir_report[7]=_gz_hir_resolve_current(t,frame);
 _gz_hir_report[8]=_gz_hir_read(t,frame);
 _gz_hir_report[9]=_gz_hir_read(t+1,frame);_gz_hir_report[10]=_gz_hir_read(t,frame+1);
 {u32 rope=_gz_srd_read_grapple(t,frame);_gz_hir_report[15]=rope;
  if(rope){_gz_hir_report[16]=H32(rope,4);_gz_hir_report[17]=H32(rope,16);}}
 _gz_hir_report[11]=_gz_bsr_end(t);
 _gz_hir_report[18]=_gz_srd_read_grapple(t,frame);
 _gz_hir_report[12]=_gz_hir_read(t,frame);_gz_hir_report[13]=_gz_osa_pin_id();
done:_gz_hir_report[14]=_gz_roh_user_guard();__asm__ volatile("sync":::"memory");_gz_hir_ack=req;
}
