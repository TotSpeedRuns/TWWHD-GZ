/* Pure captured-data relationship, never native lifetime/replay authority.
 * A REBORN result is only a logical placement candidate. The service must
 * independently bind the current queue/process, fresh heap, four controllers,
 * archives, model/texture/physics and dependent actor references. Original
 * source packets keep their original PID/generation and are never rewritten.
 */
#include "process.h"
#include "correspondence.h"
enum {RIR_NONE=0,RIR_SURVIVING=1,RIR_REBORN_CANDIDATE=2};
u32 _gz_rir_relation(const PoEntry*s,const PoEntry*l,const VrActor*a,const VrActor*b,
 u32 sf,u32 lf,u32 se,u32 le,u32 ss,u32 ls){u32 i;
 if(!s||!l||!a||!b||!sf||!lf||!ss||ss!=ls||!se||se>0x00ffffffu||le<se||le>0x00ffffffu||lf-sf>=0x80000000u)return RIR_NONE;
 if(s->state!=PO_LIVE||l->state!=PO_LIVE||s->flags||l->flags||s->end_frame||l->end_frame||
  !s->generation||!l->generation||!s->pid||!l->pid||s->pid>=0xfffffffeu||l->pid>=0xfffffffeu||
  !s->object||!l->object||(s->object&3u)||(l->object&3u)||s->profile<326||s->profile>330||l->profile!=s->profile||
  s->process_name!=s->profile||l->process_name!=l->profile||!s->profile_entry||s->profile_entry!=l->profile_entry||
  s->allocation_bytes!=0xa7cu||s->base_bytes+s->other_bytes<s->base_bytes||s->base_bytes+s->other_bytes!=0xa7cu||
  l->allocation_bytes!=s->allocation_bytes||l->base_bytes!=s->base_bytes||l->other_bytes!=s->other_bytes||
  sf-s->born_frame>=0x80000000u||lf-l->born_frame>=0x80000000u)return RIR_NONE;
 if(a->actor!=s->object||b->actor!=l->object||a->pid!=s->pid||b->pid!=l->pid||
  a->profile!=s->profile||b->profile!=l->profile||a->parameters!=b->parameters||a->set_id!=b->set_id||
  a->init_state!=2||b->init_state!=2)return RIR_NONE;
 for(i=0;i<5;i++)if(a->home[i]!=b->home[i])return RIR_NONE;
 /* A changed parent needs an independently joined dependency participant.
  * This candidate helper cannot invent the correspondence for that edge. */
 if(a->parent_pid!=b->parent_pid)return RIR_NONE;
 if(s->object==l->object&&s->pid==l->pid&&s->generation==l->generation){
  const u32*x=(const u32*)s,*y=(const u32*)l;
  for(i=0;i<sizeof(*s)/4;i++)if(x[i]!=y[i])return RIR_NONE;return RIR_SURVIVING;
 }
 return le>se&&l->generation>s->generation&&l->pid>s->pid&&l->born_frame!=sf&&l->born_frame-sf<0x80000000u?RIR_REBORN_CANDIDATE:RIR_NONE;
}
