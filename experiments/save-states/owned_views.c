/* Transaction-local identity views for a genuinely completed owned actor.
 * The immutable source and the installed original working banks are never
 * rebound. These copies are inputs for future replay participants, not a
 * whole-world commit, a release, or general constructor permission. */
#include "correspondence.h"
extern u32 _gz_vws_validate(const VrWorld*,u32),_gz_vr_world_hash(const VrWorld*);
enum {BMV_IDS=1,BMV_GRAPH=2,BMV_WORLD=3,BMV_BOMBS=4,BMV_HEADER=5,BMV_GRAPH_BYTES=23008};
typedef struct {
 u32 magic,bytes,version,valid,token,frame,source,source_bytes,pin;
 u32 source_pid,live_pid,live_actor,live_model,count,world_index,graph_index;
 u32 meta[4],ids[1024];
 u8 graph[BMV_GRAPH_BYTES];
 VrWorld world;
 PbBank bombs;
} BmvViews;
typedef char BmvSize[(sizeof(BmvViews)==68256)?1:-1];
static BmvViews _gz_bmv_views;
static u32 _gz_bmv_seal,_gz_bmv_entered;
u32 _gz_bmv_report[32]={0x424d5631u,128};
static u32 bmv_hash(void){u32 i,h=2166136261u;const u8*p=(const u8*)&_gz_bmv_views;
 for(i=0;i<sizeof(BmvViews);i++)h=(h^((i>=12&&i<16)?0:p[i]))*16777619u;
 return h;
}
/* No caller pointers are accepted. The private completed-constructor receipt,
 * original deep owner/source checks, typed preflight and exact native queue
 * delta all remain mandatory. Source historical actor/model values stay data. */
static __attribute__((noinline)) u32 bmv_gate(u32 token){u32 z,r,a,node;
 if(!token||!boh_bound(token)||_gz_roh_owner[12]!=9||_gz_roh_owner[10]!=_gz_pbac_frame||
  _gz_boh_journal.phase!=BOH_APPLIED||_gz_boh_journal.error||
  _gz_boh_recovery_request!=_gz_boh_recovery_ack||_gz_boh_recovery.valid||_gz_boh_recovery_seal)return 28;
 z=_gz_roh_lifetime_common(token,1);if(z)return z;
 z=_gz_boh_validate(token);if(z)return z;
 z=boh_queues(&r,&a,&node);if(z||r||!node||a!=_gz_boh_journal.actor)return z?z:28;
 z=_gz_btr_final_check(token);if(z)return z;
 return _gz_hbp_read(token,_gz_pbac_frame)?0:42;
}
static u32 bmv_component(u32 p,u32 n,u32 i,u32 wanted){u32 off;
 if(i>=25||R32(p,20+i*8)!=wanted)return 0;
 off=R32(p,16+i*8);
 return off>=216&&!(off&3)&&off<=n&&wanted<=n-off?p+off:0;
}
/* Check the three independent saved identity representations, not merely the
 * row being changed. Incoming ownership edges require their own participant;
 * this first completed placed-bomb lane has no such edge or child closure. */
static u32 bmv_source_shapes(u32 token,u32 p,u32 n,u32*meta,u32*ids,u32*graph,u32*world,u32*bombs){
 u32 i,j,k,flat,gr,wo,old=_gz_boh_plan.source_pid,count,g=0,w=0,f=0;const VrWorld*vw;const PbBank*bb;
 *meta=bmv_component(p,n,7,16);if(!*meta||R32(*meta,0)!=1)return 28;
 count=R32(*meta,4);if(!count||count>512)return 28;
 *ids=bmv_component(p,n,8,count*8);gr=bmv_component(p,n,17,0x60000);
 *world=bmv_component(p,n,19,sizeof(VrWorld));*bombs=bmv_component(p,n,24,16480);
 if(!*ids||!gr||!*world||!*bombs)return 28;
 *graph=gr+0x4a600;vw=(const VrWorld*)*world;bb=(const PbBank*)(*bombs+64);
 if(_gz_vws_validate(vw,sizeof(*vw))||vw->count!=count||vw->link_actor!=_gz_boh_plan.link||
  vw->link_pid!=_gz_boh_plan.link_pid||R32(*meta,8)!=vw->link_pid||
  vw->epoch!=_gz_roh_owner[45]||vw->stage0!=_gz_roh_owner[41]||vw->stage1!=_gz_roh_owner[42]||
  vw->room!=_gz_roh_owner[43]||vw->layer!=_gz_roh_owner[44])return 26;
 if(R32(*graph,0)!=0x49444731u||R32(*graph,4)!=count||R32(*graph,8)!=2||R32(*graph,12)>208||
  R32(*graph,16)!=vw->stage0||R32(*graph,20)!=vw->stage1||R32(*graph,24)!=vw->room||R32(*graph,28)!=vw->layer)return 28;
 if(_gz_ibi_bank_record_safe(bb)||bb->count!=1||!btr_equal(&bb->rows[0],&_gz_hbp_recipe.saved,sizeof(PbRow)))return 28;
 for(i=0;i<count;i++){
  flat=*ids+i*8;gr=*graph+32+i*40;wo=(u32)&vw->actors[i];
  if(R32(flat,0)==BAD||!R32(flat,4)||R32(flat,4)>0xffff||R32(gr,0)==BAD||R16(gr,4)==0)return 28;
  for(j=0;j<i;j++)if(R32(*ids+j*8,0)==R32(flat,0)||R32(*graph+32+j*40,0)==R32(gr,0)||vw->actors[j].pid==R32(wo,4))return 28;
  if(vw->actors[i].parent_pid==old&&vw->actors[i].pid!=old)return 28;
  for(j=0;j<count;j++)if(R32(*graph+32+j*40,0)==R32(flat,0))break;
  for(k=0;k<count;k++)if(vw->actors[k].pid==R32(flat,0))break;
  if(j==count||k==count)return 28;
  gr=*graph+32+j*40;wo=(u32)&vw->actors[k];
  if(R32(flat,4)!=R16(gr,4)||R32(flat,4)!=R32(wo,8)||R16(gr,6)!=R32(wo,16)||
   R32(gr,8)!=R32(wo,12)||R32(gr,12)!=R32(wo,20)||R32(gr,36)!=R32(wo,0)||
   !btr_equal((const void*)(gr+16),(const void*)(wo+24),19))return 28;
  if(R32(flat,0)==_gz_boh_journal.pid)return 28;
  if(R32(flat,0)==old){if(R32(flat,4)!=294||R16(gr,6)!=0xffff||R32(gr,12)!=BAD)return 28;f++;g=j;w=k;}
 }
 for(i=0;i<R32(*graph,12);i++){
  gr=*graph+20512+i*12;if(R32(gr,0)==old||R32(gr,4)==old)return 28;
 }
 if(f!=1||vw->actors[w].actor!=_gz_hbp_recipe.saved.actor||vw->actors[w].parameters!=0x80000001u)return 28;
 (void)token;_gz_bmv_views.count=count;_gz_bmv_views.world_index=w;_gz_bmv_views.graph_index=g;return 0;
}
/* Exact mapped flat-set check. The constructor's existing RLL receipt
 * independently proves unchanged unrelated actor topology and identities. */
static u32 bmv_live_ids(void){const BmvViews*v=&_gz_bmv_views;volatile u8 seen[512];u32 q=_gz_pba_native_actor_queue,left=4096,a,i,count=0;
 for(i=0;i<512;i++)seen[i]=0;
 while(q){if(!left--||!span(q,16))return 28;a=R32(q,12);
  if(a){if(!span(a,12))return 28;for(i=0;i<v->count;i++)if(v->ids[i*2]==R32(a,4)&&v->ids[i*2+1]==R16(a,8))break;
   if(i==v->count||seen[i]++)return 27;count++;}
  q=R32(q,8);
 }
 return count==v->count?0:27;
}
static u32 bmv_mapped_shapes(void){const BmvViews*v=&_gz_bmv_views;const PbRow*b=&v->bombs.rows[0];const VrActor*w;u32 g,i,seen=0;
 if(!v->count||v->count>512||v->world_index>=v->count||v->graph_index>=v->count||v->bombs.count!=1)return 28;
 w=&v->world.actors[v->world_index];g=(u32)v->graph+32+v->graph_index*40;
 if(b->pid!=v->live_pid||b->actor!=v->live_actor||b->resources[0]!=v->live_model||
  w->pid!=v->live_pid||w->actor!=v->live_actor||w->profile!=294||w->parent_pid!=BAD||
  R32(g,0)!=v->live_pid||R16(g,4)!=294||R32(g,36)!=v->live_actor)return 28;
 for(i=0;i<v->count;i++){
  if(v->ids[i*2]==v->source_pid)return 28;
  if(v->ids[i*2]==v->live_pid){if(v->ids[i*2+1]!=294)return 28;seen++;}
 }
 return seen==1?0:28;
}
u32 _gz_bmv_check(u32 token){BmvViews*v=&_gz_bmv_views;u32 z;
 if(_gz_bmv_entered||v->valid!=1||v->magic!=0x424d5631u||v->bytes!=sizeof(*v)||v->version!=1||
  v->token!=token||!_gz_bmv_seal||_gz_bmv_seal!=bmv_hash())return 28;
 z=bmv_gate(token);if(z)return z;
 if(v->source!=_gz_hbp_recipe.source||v->source_bytes!=_gz_hbp_recipe.used||v->pin!=_gz_boh_plan.pin||
  v->source_pid!=_gz_boh_plan.source_pid||v->live_pid!=_gz_boh_journal.pid||v->live_actor!=_gz_boh_journal.actor||
  v->live_model!=_gz_btr_journal.model||v->count>512||v->world_index>=v->count||v->graph_index>=v->count||
  bmv_mapped_shapes()||_gz_vws_validate(&v->world,sizeof(VrWorld))||
  _gz_ibi_bank_preflight(&v->bombs,_gz_pba_native_actor_queue,&v->bombs.rows[0].context))return 28;
 z=bmv_live_ids();if(z)return z;
 return bmv_gate(token);
}
u32 _gz_bmv_build(u32 token){BmvViews*v=&_gz_bmv_views;u32 z,p,n,meta,ids,graph,world,bombs,i;
 if(_gz_bmv_entered||v->valid||_gz_bmv_seal)return 5;
 z=bmv_gate(token);if(z)return z;
 _gz_bmv_entered=1;p=_gz_hbp_recipe.source;n=_gz_hbp_recipe.used;
 z=bmv_source_shapes(token,p,n,&meta,&ids,&graph,&world,&bombs);if(z)goto done;
 v->magic=0x424d5631u;v->bytes=sizeof(*v);v->version=1;v->valid=0;v->token=token;v->frame=_gz_pbac_frame;
 v->source=p;v->source_bytes=n;v->pin=_gz_boh_plan.pin;v->source_pid=_gz_boh_plan.source_pid;
 v->live_pid=_gz_boh_journal.pid;v->live_actor=_gz_boh_journal.actor;v->live_model=_gz_btr_journal.model;
 btr_copy(v->meta,(const void*)meta,16);for(i=0;i<1024;i++)v->ids[i]=0;
 btr_copy(v->ids,(const void*)ids,v->count*8);btr_copy(v->graph,(const void*)graph,BMV_GRAPH_BYTES);
 btr_copy(&v->world,(const void*)world,sizeof(VrWorld));btr_copy(&v->bombs,(const void*)(bombs+64),sizeof(PbBank));
 z=_gz_ibi_bind_recreated((void*)v->live_actor,&v->bombs.rows[0],&v->bombs.rows[0].context,v->ids,v->count,v->graph);if(z)goto done;
 v->world.actors[v->world_index].pid=v->live_pid;v->world.actors[v->world_index].actor=v->live_actor;
 v->world.checksum=_gz_vr_world_hash(&v->world);
 z=bmv_mapped_shapes();if(z)goto done;
 z=_gz_vws_validate(&v->world,sizeof(VrWorld));if(z)goto done;
 z=bmv_live_ids();if(z)goto done;z=bmv_gate(token);if(z)goto done;
 _gz_bmv_seal=bmv_hash();__asm__ volatile("sync":::"memory");v->valid=1;
done:
 _gz_bmv_entered=0;
 _gz_bmv_report[2]=token;_gz_bmv_report[3]=z;_gz_bmv_report[4]=_gz_pbac_frame;
 _gz_bmv_report[5]=v->valid;_gz_bmv_report[6]=v->source_pid;_gz_bmv_report[7]=v->live_pid;
 _gz_bmv_report[8]=v->live_actor;_gz_bmv_report[9]=v->live_model;_gz_bmv_report[10]=v->count;
 _gz_bmv_report[11]=_gz_bmv_seal;_gz_bmv_report[12]=sizeof(BmvViews);
 return z?z:_gz_bmv_check(token);
}
/* Frame-bounded READ-only result; never accepts a caller destination or grants
 * native issue/replay/publication. Each consuming participant must validate
 * its own resource/collision dependencies before using these mapped inputs. */
u32 _gz_bmv_read(u32 token,u32 frame,u32 part){BmvViews*v=&_gz_bmv_views;
 if(frame!=_gz_pbac_frame||part<BMV_IDS||part>BMV_HEADER||_gz_bmv_check(token))return 0;
 if(part==BMV_IDS)return (u32)v->ids;
 if(part==BMV_GRAPH)return (u32)v->graph;
 if(part==BMV_WORLD)return (u32)&v->world;
 if(part==BMV_BOMBS)return (u32)&v->bombs;
 return (u32)v;
}
u32 _gz_bmv_tick(u32 token){return _gz_bmv_views.valid?_gz_bmv_check(token):_gz_bmv_build(token);}
