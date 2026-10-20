/* Owner-held identity lane scaffold only. No native API or game stores.
 * A successful comparison never grants issue/pump/destruction authority. */
#include "lane.h"
#define R32(p,o) (*(volatile const u32*)((u32)(p)+(o)))
#define R16(p,o) (*(volatile const u16*)((u32)(p)+(o)))
#define R8(p,o) (*(volatile const u8*)((u32)(p)+(o)))
extern volatile u32 _gz_rll_actor_head,_gz_rll_create_head,_gz_rll_delete_head,_gz_rll_priority_head,_gz_rll_current_layer;
extern volatile u32 _gz_roh_live_frame,_gz_roh_live_epoch,_gz_roh_live_choice,_gz_roh_live_link,_gz_roh_live_stage[2];
extern volatile u32 _gz_roh_live_slots[10],_gz_roh_live_bflower;
extern volatile u8 _gz_roh_live_room,_gz_roh_live_layer,_gz_roh_live_loading,_gz_roh_live_event;
RllBaseline _gz_rll_baseline,_gz_rll_stage;
volatile RllReport _gz_rll_report;
u32 _gz_rll_fault;
/* Separate trusted-length corruption seal. A mismatch never rebaselines. */
typedef struct {u32 magic,bytes,valid,used,hash,inverse;} RllSeal;
RllSeal _gz_rll_seal;
static u32 sealed(void);

static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static u32 input(u32 p,u32 n){return !(p&3)&&p>=0x01800000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static u32 overlap(u32 a,u32 an,u32 b,u32 bn){return a<b+bn&&b<a+an;}
/* Inputs must never alias ANY publication/error/private staging role. An
 * invalid alias returns without error publication because even a diagnostic
 * store might otherwise overwrite the caller's alleged immutable input. */
static u32 private_alias(u32 p,u32 n){return
 overlap(p,n,(u32)&_gz_rll_baseline,sizeof(_gz_rll_baseline))||
 overlap(p,n,(u32)&_gz_rll_stage,sizeof(_gz_rll_stage))||
 overlap(p,n,(u32)&_gz_rll_report,sizeof(_gz_rll_report))||
 overlap(p,n,(u32)&_gz_rll_fault,sizeof(_gz_rll_fault))||
 overlap(p,n,(u32)&_gz_rll_seal,sizeof(_gz_rll_seal));}
static u32 equal(u32 a,u32 b,u32 n){u32 i;for(i=0;i<n;i++)if(R8(a,i)!=R8(b,i))return 0;return 1;}
static u32 mix(u32 h,u32 z){u32 i;for(i=0;i<4;i++){h=(h^(z>>24))*16777619u;z<<=8;}return h;}
static u32 prefix_hash(u32 p,u32 n){u32 h=2166136261u,i;for(i=0;i<n;i++)h=(h^R8(p,i))*16777619u;return h;}
/* No baseline-derived length/count/key value is read before this bounded
 * exact-prefix check. The separate seal's used length is range-checked first. */
static u32 sealed(void){const RllSeal*s=&_gz_rll_seal;u32 used;
 if(s->magic!=0x524c5332u||s->bytes!=sizeof(*s)||s->valid!=1||s->inverse!=~s->hash)return 0;
 used=s->used;
 if(used<(u32)&((RllBaseline*)0)->rows[0]||used>sizeof(RllBaseline)||(used-(u32)&((RllBaseline*)0)->rows[0])%sizeof(RllRow))return 0;
 return prefix_hash((u32)&_gz_rll_baseline,used)==s->hash;
}
static void copy(u32 a,u32 b,u32 n){u32 i;for(i=0;i<n;i++)*(volatile u8*)(a+i)=R8(b,i);}
static u32 idle(void){const RohOwner*o=&_gz_roh_owner;return o->magic==ROH_MAGIC&&o->bytes==240&&o->version==2&&!o->reserved&&o->phase==ROH_IDLE&&!o->hold&&!o->token&&!o->mutated&&!o->inflight;}
/* Basic receipt binding only. The integrated new owner MUST run its original
 * deep/shallow nonqueue checks too; this helper does not stand in for them. */
static u32 owner(u32 token,u32 mode){const RohOwner*o=&_gz_roh_owner;const RohSample*s=&o->sample;u32 f=_gz_roh_live_frame,delta=f-o->start_frame;
 if(mode==3)return _gz_roh_recovery_common(token,f)==0;
 if(o->magic!=ROH_MAGIC||o->bytes!=240||o->version!=2||o->reserved||!token||o->token!=token||_gz_roh_serial!=token||o->hold!=1||!o->frame_open||delta>=0x80000000u||delta>o->limit||f-o->last_frame>=0x80000000u)return 0;
 if(mode==1){if(o->phase!=ROH_CLEAN||o->mutated||o->inflight!=ROH_OBSERVE||o->last_frame!=f)return 0;}
 else if(mode==4){
  extern u32 _gz_bmh_scope(u32,u32);
  if(o->phase!=ROH_MUTATING||o->mutated!=1||o->inflight<2||o->inflight>8||o->last_frame!=f||!_gz_bmh_scope(token,f))return 0;
 }
 else if(o->phase!=ROH_MUTATING||o->mutated!=1||(mode==2?(o->inflight!=RLL_FUTURE_OWNER_SERVICE||o->last_frame!=f):o->inflight!=0))return 0;
 if(s->slot>9||s->archive!=_gz_roh_live_slots[s->slot]||s->working_base!=_gz_roh_live_bflower||s->registry_revision!=_gz_roh_registry_revision||s->link!=_gz_roh_live_link||s->epoch!=_gz_roh_live_epoch||!s->epoch||s->stage0!=_gz_roh_live_stage[0]||s->stage1!=_gz_roh_live_stage[1]||s->room!=_gz_roh_live_room||s->layer!=_gz_roh_live_layer||_gz_roh_live_loading||_gz_roh_live_event||_gz_roh_live_choice>755)return 0;
 if(!span(s->link,0x6b00)||s->profile!=168||R32(s->link,4)!=s->pid||R16(s->link,8)!=168||R8(s->link,12)!=2)return 0;
 return f==_gz_roh_live_frame;
}
static u32 row(u32 q,RllRow*r){u32 a=R32(q,12),i;r->node=q;r->actor=a;
 if(!a){for(i=2;i<sizeof(*r)/4;i++)((u32*)r)[i]=0;return 1;}
 if(!span(a,0x300))return 0;
 r->pid=R32(a,4);r->profile=R16(a,8);r->parameters=R32(a,0xb0);r->parent=R32(a,0x2e8);r->init=R8(a,12);r->word10=R32(a,0x10);r->wordA8=R32(a,0xa8);r->wordF4=R32(a,0xf4);
 for(i=0;i<5;i++)r->home[i]=R32(a,0x2ec+4*i);return 1;
}
static u32 fail(u32 z,u32 q){_gz_rll_fault=z;_gz_rll_report.result=z;_gz_rll_report.first_bad_node=q;return z;}
static u32 compare_fail(u32 z,u32 q,u32 mode){
 if(mode!=3)return fail(z,q);
 if(!_gz_rll_fault)_gz_rll_fault=z;
 _gz_rll_report.result=z;_gz_rll_report.first_bad_node=q;return z;
}
u32 _gz_rll_prepare(u32 token,u32 kind,u32 profile,u32 source_pid,u32 source_actor,u32 key,u32 key_bytes){
 RllBaseline*s=&_gz_rll_stage;u32 q,n=0,i,target=0,frame,h=2166136261u,a,next,used;
 if(_gz_rll_seal.valid||_gz_rll_baseline.valid||_gz_rll_fault)return RLL_BUSY;
 if(!owner(token,1))return RLL_CONTEXT;
 if((kind!=RLL_CREATE&&kind!=RLL_DELETE)||profile>=0x7fffu||source_pid==0xffffffffu||!key_bytes||key_bytes>RLL_KEY_BYTES||!input(key,key_bytes)||
  private_alias(key,key_bytes))return RLL_INVALID;
 /* The current operation borrows this exact live player allocation. Replacing
  * that owner is a different protocol, never an ordinary actor-lane target. */
 if(profile==_gz_roh_owner.sample.profile||(kind==RLL_DELETE&&source_actor==_gz_roh_owner.sample.link))return RLL_INVALID;
 if(_gz_rll_create_head||_gz_rll_delete_head||_gz_rll_priority_head||!span(_gz_rll_current_layer,4)||_gz_rll_actor_head!=_gz_roh_owner.sample.actor_head)return RLL_FOREIGN;
 s->valid=0;s->magic=RLL_MAGIC;s->bytes=sizeof(*s);s->version=1;s->token=token;s->kind=kind;s->profile=profile;s->source_pid=source_pid;s->source_actor=source_actor;
 s->choice=_gz_roh_live_choice;s->current_layer=_gz_rll_current_layer;s->captured_frame=frame=_gz_roh_live_frame;s->head=q=_gz_rll_actor_head;s->key_bytes=key_bytes;
 copy((u32)&s->sample,(u32)&_gz_roh_owner.sample,sizeof(RohSample));copy((u32)s->key,key,key_bytes);
 while(q){
  if(n==RLL_CAPACITY||!span(q,16))return RLL_LIMIT;
  for(i=0;i<n;i++)if(s->rows[i].node==q)return RLL_FOREIGN;
  if(!row(q,&s->rows[n]))return RLL_FOREIGN;
  if(kind==RLL_DELETE&&s->rows[n].actor==source_actor){if(s->rows[n].pid!=source_pid||s->rows[n].profile!=profile||s->rows[n].init!=2||target++)return RLL_FOREIGN;}
  next=R32(q,8);a=s->rows[n].actor;h=mix(mix(mix(h,q),next),a);
  if(a){h=mix(h,s->rows[n].pid);h=mix(h,s->rows[n].profile);h=mix(h,s->rows[n].parameters);h=mix(h,s->rows[n].parent);}
  q=next;n++;
 }
 if((kind==RLL_DELETE&&target!=1)||(kind==RLL_CREATE&&source_actor))return RLL_INVALID;
 if(n!=_gz_roh_owner.sample.actor_count||h!=_gz_roh_owner.sample.actor_topology_hash)return RLL_CHANGED;
 s->count=n;
 if(!owner(token,1)||frame!=_gz_roh_live_frame||s->choice!=_gz_roh_live_choice||s->head!=_gz_rll_actor_head||s->current_layer!=_gz_rll_current_layer||_gz_rll_create_head||_gz_rll_delete_head||_gz_rll_priority_head||!equal(key,(u32)s->key,key_bytes)||!equal((u32)&s->sample,(u32)&_gz_roh_owner.sample,sizeof(RohSample)))return RLL_CHANGED;
 used=((u32)&s->rows[n])-(u32)s;s->valid=1;
 copy((u32)&_gz_rll_baseline,(u32)s,used);
 _gz_rll_seal.valid=0;_gz_rll_seal.magic=0x524c5332u;_gz_rll_seal.bytes=sizeof(RllSeal);_gz_rll_seal.used=used;
 _gz_rll_seal.hash=prefix_hash((u32)&_gz_rll_baseline,used);_gz_rll_seal.inverse=~_gz_rll_seal.hash;
 __asm__ volatile("sync":::"memory");_gz_rll_seal.valid=1;return 0;
}
static u32 compare(const RllTarget*t,u32 mode){
 const RllBaseline*b=&_gz_rll_baseline;RllRow r;u32 q,n=0,j=0,seen=0,unowned=0,frame=_gz_roh_live_frame,i;
 /* Reject alias roles before even publishing an existing sticky error. */
 if(!input((u32)t,sizeof(*t))||private_alias((u32)t,sizeof(*t)))return RLL_INVALID;
 if(!input(t->key,t->key_bytes)||private_alias(t->key,t->key_bytes))return RLL_INVALID;
 if(_gz_rll_fault&&mode!=3)return _gz_rll_fault;
 if(!sealed())return compare_fail(RLL_CHANGED,0,mode);
 if(!b->valid||b->magic!=RLL_MAGIC||b->bytes!=sizeof(*b)||b->version!=1||b->count>RLL_CAPACITY||b->key_bytes>RLL_KEY_BYTES||!b->key_bytes||_gz_rll_seal.used!=(u32)&b->rows[b->count]-(u32)b||!owner(t->token,mode)||t->token!=b->token||t->kind!=b->kind||t->profile!=b->profile||t->source_pid!=b->source_pid||t->source_actor!=b->source_actor||t->observation_frame!=frame||t->expected_presence>2||t->live_pid==0xffffffffu||t->key_bytes!=b->key_bytes||!input(t->key,t->key_bytes)||!equal(t->key,(u32)b->key,b->key_bytes)||(mode==3&&(b->kind!=RLL_CREATE||b->source_actor)))return compare_fail(RLL_CONTEXT,0,mode);
 if(b->choice!=_gz_roh_live_choice||b->current_layer!=_gz_rll_current_layer||_gz_rll_priority_head||!equal((u32)&b->sample,(u32)&_gz_roh_owner.sample,sizeof(RohSample)))return compare_fail(RLL_CHANGED,0,mode);
 if(t->kind==RLL_DELETE&&(t->live_pid!=b->source_pid||t->live_actor!=b->source_actor))return compare_fail(RLL_FOREIGN,0,mode);
 if(t->kind==RLL_CREATE)for(i=0;i<b->count;i++)if(b->rows[i].actor&&(b->rows[i].pid==t->live_pid||b->rows[i].actor==t->live_actor))return compare_fail(RLL_FOREIGN,b->rows[i].node,mode);
 q=_gz_rll_actor_head;_gz_rll_report.head=q;_gz_rll_report.first_bad_node=0;
 while(q){
  if(n++==RLL_CAPACITY+1u||!span(q,16)||!row(q,&r))return compare_fail(RLL_LIMIT,q,mode);
  if(r.actor&&(r.pid==t->live_pid||r.actor==t->live_actor)){
   if(seen++||!t->live_actor||!t->actor_queue_node||r.node!=t->actor_queue_node||r.actor!=t->live_actor||r.pid!=t->live_pid||r.profile!=t->profile)return compare_fail(RLL_FOREIGN,q,mode);
   if(t->kind==RLL_DELETE){if(j>=b->count||b->rows[j].actor!=b->source_actor||b->rows[j].node!=q)return compare_fail(RLL_FOREIGN,q,mode);j++;}
  }else{
   if(t->kind==RLL_DELETE&&j<b->count&&b->rows[j].actor==b->source_actor)j++;
   if(j>=b->count||!equal((u32)&r,(u32)&b->rows[j],sizeof(r)))return compare_fail(RLL_CHANGED,q,mode);j++;unowned++;
  }
  q=R32(q,8);
 }
 if(t->kind==RLL_DELETE&&j<b->count&&b->rows[j].actor==b->source_actor)j++;
 if(j!=b->count||(t->expected_presence==RLL_PRESENT&&!seen)||(t->expected_presence==RLL_ABSENT&&seen))return compare_fail(RLL_CHANGED,0,mode);
 if(!sealed()||!owner(t->token,mode)||frame!=_gz_roh_live_frame||b->choice!=_gz_roh_live_choice||b->current_layer!=_gz_rll_current_layer||_gz_rll_priority_head||_gz_rll_actor_head!=_gz_rll_report.head||!equal(t->key,(u32)b->key,b->key_bytes))return compare_fail(RLL_CHANGED,0,mode);
 _gz_rll_report.result=0;_gz_rll_report.frame=frame;_gz_rll_report.target_seen=seen;_gz_rll_report.unowned_count=unowned;_gz_rll_report.count=n;return 0;
}
u32 _gz_rll_check_actor_delta(const RllTarget*t){return compare(t,2);}
u32 _gz_rll_poll_actor_delta(const RllTarget*t){return compare(t,0);}
u32 _gz_rll_recovery_actor_delta(const RllTarget*t){return compare(t,3);}
/* Read-only topology witness inside the exact sealed typed handoff. The
 * original service9 issue/pump and source-free recovery entries are unchanged. */
u32 _gz_rll_replay_actor_delta(const RllTarget*t){return compare(t,4);}
u32 _gz_rll_check_unissued(u32 token,u32 key,u32 key_bytes){
 const RllBaseline*b=&_gz_rll_baseline;RllRow r;u32 q,i=0,frame=_gz_roh_live_frame,mode=_gz_roh_owner.inflight?2:0;
 if(!input(key,key_bytes)||private_alias(key,key_bytes))return RLL_INVALID;
 if(_gz_rll_fault)return _gz_rll_fault;
 if(!sealed())return fail(RLL_CHANGED,0);
 if(!owner(token,mode)||b->kind!=RLL_CREATE||b->token!=token||b->source_actor||b->key_bytes!=key_bytes||
  !equal(key,(u32)b->key,key_bytes)||b->count>RLL_CAPACITY||b->choice!=_gz_roh_live_choice||
  b->current_layer!=_gz_rll_current_layer||_gz_rll_create_head||_gz_rll_delete_head||_gz_rll_priority_head||
  !equal((u32)&b->sample,(u32)&_gz_roh_owner.sample,sizeof(RohSample)))return fail(RLL_CONTEXT,0);
 q=_gz_rll_actor_head;
 while(q){if(i>=b->count||!span(q,16)||!row(q,&r)||!equal((u32)&r,(u32)&b->rows[i],sizeof(r)))return fail(RLL_CHANGED,q);i++;q=R32(q,8);}
 if(i!=b->count||!sealed()||!owner(token,mode)||frame!=_gz_roh_live_frame||_gz_rll_create_head||_gz_rll_delete_head||_gz_rll_priority_head||
  !equal(key,(u32)b->key,key_bytes))return fail(RLL_CHANGED,0);
 return 0;
}
u32 _gz_rll_forget(void){if(!idle())return RLL_BUSY;_gz_rll_baseline.valid=0;_gz_rll_stage.valid=0;_gz_rll_seal.valid=0;_gz_rll_fault=0;return 0;}
void _gz_rll_recovery_finish_leaf(void){_gz_rll_baseline.valid=0;_gz_rll_stage.valid=0;_gz_rll_seal.valid=0;_gz_rll_fault=0;}
u32 _gz_rll_native_admit(void){return 28;}
