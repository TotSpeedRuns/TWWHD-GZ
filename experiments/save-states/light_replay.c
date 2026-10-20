/* Searchlight scalar participant in the owner-held bomb/world transaction.
 * Saved addresses only identify records. Fresh current queue membership and
 * observed process generations precede every actor read. No mode initializer,
 * saved callback, model pointer, native queue or room-loader API is replayed.
 * Child Bokoblin beam-yaw capture is a separate, still missing dependency.
 */
#include "light_state.h"
#include "process.h"

enum { LCT_PREPARED=1,LCT_APPLYING=2,LCT_APPLIED=3,LCT_UNDOING=4,LCT_ROLLED=5 };
typedef struct {
 u32 magic,bytes,version,valid,phase,token,frame,applied_frame,rolled_frame;
 u32 source,used,pin,packet,link,source_hash,before_hash;
 u32 scene_epoch,link_pid;
 PoEntry processes[L3_CAPACITY];
 L3Packet saved,before;
} LctPlan;
typedef char LctSize[sizeof(LctPlan)==17096?1:-1];
static LctPlan _gz_lct_plan;
static u32 _gz_lct_seal,_gz_lct_entered;
u32 _gz_lct_report[32]={0x4c435431u,128};

static void lct_seal(void){_gz_lct_seal=btr_hash(&_gz_lct_plan,sizeof(LctPlan));}
static u32 lct_packet(const L3Packet*p){u32 i,j;
 if(p->magic!=L3_PACKET_MAGIC||p->bytes!=sizeof(*p)||p->version!=1||p->valid!=1||
  p->count>L3_CAPACITY||!p->frame||!p->epoch||!p->stage0||p->room>63||p->layer>255||
  p->link_pid||p->epoch!=p->frame||p->checksum!=_gz_l3_packet_hash(p))return 28;
 for(i=0;i<3;i++)if(p->reserved[i])return 28;
 for(i=0;i<p->count;i++){
  if(_gz_l3_record_valid(p->records+i))return 28;
  for(j=0;j<i;j++)if(p->records[i].pid==p->records[j].pid||p->records[i].actor==p->records[j].actor)return 28;
 }
 /* Native capture overwrites only active rows. Inactive rows can retain old
  * bytes; they are opaque, never validated as live records or replayed. The
  * original whole-archive pin and exact complete private copy protect them. */
 return 0;
}
static u32 lct_sealed(void){const LctPlan*p=&_gz_lct_plan;
 return p->magic==0x4c435431u&&p->bytes==sizeof(*p)&&p->version==2&&p->valid==1&&p->token&&
  p->phase>=1&&p->phase<=5&&_gz_lct_seal==btr_hash(p,sizeof(*p))&&
  p->source_hash==btr_hash(&p->saved,sizeof(L3Packet))&&p->before_hash==btr_hash(&p->before,sizeof(L3Packet));
}
static u32 lct_scene(const L3Packet*p){const LctPlan*j=&_gz_lct_plan;
 return p->stage0==_gz_pbac_stage[0]&&p->stage1==_gz_pbac_stage[1]&&
  p->room==_gz_pbac_room&&p->layer==_gz_pbac_layer&&j->scene_epoch==_gz_pbac_epoch&&
  j->link_pid==_gz_boh_plan.link_pid;
}
/* L3 v1's misleading "epoch" is the native frame counter, and link_pid is
 * written as zero. Actual scene/player identity comes from the canonical
 * source World component, authenticated under the original source pin. */
static u32 lct_source_context(u32 source,u32 used){LctPlan*j=&_gz_lct_plan;const VrWorld*w;u32 o;
 if(R32(source,172)!=sizeof(VrWorld))return 42;o=R32(source,168);
 if(o<216||(o&3)||o>used||sizeof(VrWorld)>used-o)return 42;
 w=(const VrWorld*)(source+o);
 if(_gz_vws_validate(w,sizeof(*w)))return 42;
 if(w->stage0!=j->saved.stage0||w->stage1!=j->saved.stage1||w->room!=j->saved.room||w->layer!=j->saved.layer||
  w->frame!=j->saved.epoch||w->frame!=j->saved.frame||w->epoch!=_gz_pbac_epoch||
  w->link_actor!=_gz_boh_plan.link||w->link_pid!=_gz_boh_plan.link_pid)return 26;
 j->scene_epoch=w->epoch;j->link_pid=w->link_pid;return 0;
}
/* Complete domain preflight before the first store, including empty banks.
 * An added/missing/duplicated searchlight is never treated as an empty match.
 * Once rediscovered, process extent is checked before the larger actor read.
 */
static u32 lct_bind(const L3Packet*p,u32 capture){LctPlan*j=&_gz_lct_plan;
 u32 q=_gz_pba_native_actor_queue,left=4096,a,n=0,k,z,seen=0;const PoEntry*e;
 while(q){
  if(!left--||!span(q,16))return 27;a=R32(q,12);
  if(a){
   if(!span(a,16))return 27;
   if(R16(a,8)==66){
    for(k=0;k<p->count;k++)if(p->records[k].pid==R32(a,4))break;
    if(k==p->count||(seen&(1u<<k))||a!=p->records[k].actor)return 27;
    seen|=1u<<k;n++;
    if(_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return 27;
    e=&_gz_po_observation.entry;
    if(e->object!=a||!e->generation||e->state!=PO_LIVE||e->flags||e->end_frame||
     e->allocation_bytes<0xa28||e->allocation_bytes>0x100000||!span(a,e->allocation_bytes)||
     e->base_bytes+e->other_bytes<e->base_bytes||e->base_bytes+e->other_bytes!=e->allocation_bytes||
     e->pid!=R32(a,4)||e->process_name!=66||e->profile>=484||R16(a,14)!=e->profile||
     R32(a,16)!=e->profile_entry)return 27;
    if(_gz_l3_check_actor(a,p->records+k))return 28;
    if(capture)btr_copy(j->processes+k,e,sizeof(*e));
    else if(!btr_equal(j->processes+k,e,sizeof(*e)))return 27;
   }
  }
  q=R32(q,8);
 }
 z=(1u<<p->count)-1;return n==p->count&&seen==z?0:27;
}
static u32 lct_match(const L3Packet*p){u32 i,k,j,offset,bytes,packed;const L3Record*r;
 for(i=0;i<p->count;i++){
  r=p->records+i;packed=0;
  for(k=0;_gz_l3_fields[k+1];k+=2){
   offset=_gz_l3_fields[k];bytes=_gz_l3_fields[k+1];
   for(j=0;j<bytes;j++)if(R8(r->actor,offset+j)!=r->payload[packed+j])return 27;
   packed+=bytes;
  }
 }return 0;
}
/* No fallible call remains after whole-bank preflight. Numeric dispatch mode
 * is published after the rest of that light's state, without firing initProc.
 */
static __attribute__((noinline)) void lct_write(const L3Packet*p){u32 i,k,j,offset,bytes,packed;const L3Record*r;
 for(i=0;i<p->count;i++){
  r=p->records+i;packed=0;
  for(k=0;_gz_l3_fields[k+1];k+=2){
   offset=_gz_l3_fields[k];bytes=_gz_l3_fields[k+1];
   if(offset!=0x3ac)for(j=0;j<bytes;j++)*(volatile u8*)(r->actor+offset+j)=r->payload[packed+j];
   packed+=bytes;
  }
  __asm__ volatile("sync":::"memory");*(volatile u32*)(r->actor+0x3ac)=r->mode;
 }
}
u32 _gz_lct_prepare(u32 token,u32 frame){LctPlan*j=&_gz_lct_plan;u32 s,n,o,b,z,i;
 if(_gz_lct_entered||j->phase||j->valid||_gz_lct_seal)return 28;
 if(!token||_gz_roh_gl_context(token,frame,1)!=1||!_gz_hbp_read(token,frame))return 42;
 _gz_lct_entered=1;s=_gz_hbp_recipe.source;n=_gz_hbp_recipe.used;
 if(n<216){z=42;goto done;}o=R32(s,152);b=R32(s,156);
 if(o<216||(o&3)||o>n||b>n-o||b<L3_PACKET_OFFSET+sizeof(L3Packet)){z=42;goto done;}
 j->packet=s+o+L3_PACKET_OFFSET;btr_copy(&j->saved,(const void*)j->packet,sizeof(L3Packet));
 z=lct_packet(&j->saved);if(z)goto done;z=lct_source_context(s,n);if(z)goto done;
 if(!lct_scene(&j->saved)){z=26;goto done;}
 j->magic=0x4c435431u;j->bytes=sizeof(*j);j->version=2;j->phase=LCT_PREPARED;
 j->token=token;j->frame=frame;j->source=s;j->used=n;j->pin=_gz_hbp_recipe.pin;j->link=_gz_boh_plan.link;
 z=lct_bind(&j->saved,1);if(z)goto done;
 btr_copy(&j->before,&j->saved,sizeof(L3Packet));j->before.frame=frame;j->before.epoch=frame;
 for(i=0;i<j->saved.count;i++){
  z=_gz_l3_capture_actor(j->saved.records[i].actor,j->before.records+i);if(z)goto done;
 }
 j->before.checksum=_gz_l3_packet_hash(&j->before);
 z=lct_packet(&j->before);if(z)goto done;z=lct_match(&j->before);if(z)goto done;
 if(_gz_roh_gl_context(token,frame,1)!=1||!_gz_hbp_read(token,frame)||
  !btr_equal(&j->saved,(const void*)j->packet,sizeof(L3Packet))){z=42;goto done;}
 j->source_hash=btr_hash(&j->saved,sizeof(L3Packet));j->before_hash=btr_hash(&j->before,sizeof(L3Packet));
 j->valid=1;lct_seal();_gz_lct_report[2]=token;_gz_lct_report[3]=LCT_PREPARED;
 _gz_lct_report[5]=frame;_gz_lct_report[8]=j->saved.count;
done:_gz_lct_report[4]=z;_gz_lct_entered=0;return z;
}
static u32 lct_apply_gate(u32 token,u32 frame){const LctPlan*j=&_gz_lct_plan;
 if(!lct_sealed()||j->token!=token||_gz_roh_owner[12]!=2||!_gz_bmh_scope(token,frame)||
  _gz_roh_replay_common(token,frame,1)||_gz_boh_service_validate(token)||!_gz_hbp_service_read(token,frame))return 42;
 if(j->source!=_gz_hbp_recipe.source||j->used!=_gz_hbp_recipe.used||j->pin!=_gz_hbp_recipe.pin||
  j->packet!=j->source+R32(j->source,152)+L3_PACKET_OFFSET||
  !btr_equal(&j->saved,(const void*)j->packet,sizeof(L3Packet))||!lct_scene(&j->saved))return 42;
 return lct_bind(&j->before,0);
}
u32 _gz_lct_apply(u32 token){LctPlan*j=&_gz_lct_plan;u32 z,frame=_gz_pbac_frame;
 if(_gz_lct_entered||!lct_sealed()||j->phase!=LCT_PREPARED||j->token!=token)return 28;
 _gz_lct_entered=1;z=lct_apply_gate(token,frame);if(z)goto done;
 z=lct_match(&j->before);if(z)goto done;
 j->applied_frame=frame;j->phase=LCT_APPLYING;lct_seal();lct_write(&j->saved);
 z=lct_match(&j->saved);if(z)goto done;z=lct_apply_gate(token,frame);if(z)goto done;
 j->phase=LCT_APPLIED;lct_seal();_gz_lct_report[3]=LCT_APPLIED;_gz_lct_report[6]=frame;
done:_gz_lct_report[4]=z;_gz_lct_entered=0;return z;
}
u32 _gz_lct_recovery_rollback(u32 token){LctPlan*j=&_gz_lct_plan;u32 z,frame=_gz_pbac_frame;
 if(_gz_lct_entered||!lct_sealed()||j->token!=token||j->phase==LCT_ROLLED||j->phase==LCT_UNDOING)return 28;
 _gz_lct_entered=1;
 if(!_gz_boh_recovery_authority(token,_gz_boh_journal.actor,j->link,frame)){z=40;goto done;}
 if(!lct_scene(&j->before)){z=26;goto done;}z=lct_bind(&j->before,0);if(z)goto done;
 if(j->phase==LCT_PREPARED){z=lct_match(&j->before);if(z)goto done;}
 else{j->phase=LCT_UNDOING;lct_seal();lct_write(&j->before);z=lct_match(&j->before);if(z)goto done;}
 z=lct_bind(&j->before,0);if(z)goto done;
 if(!_gz_boh_recovery_authority(token,_gz_boh_journal.actor,j->link,frame)){z=40;goto done;}
 j->rolled_frame=frame;j->phase=LCT_ROLLED;lct_seal();_gz_lct_report[3]=LCT_ROLLED;_gz_lct_report[7]=frame;
done:_gz_lct_report[4]=z;_gz_lct_entered=0;return z;
}
