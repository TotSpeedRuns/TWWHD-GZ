/* Pure placement relation for a collision-background owner. A candidate is
 * not a lifetime, geometry equivalence, constructor or replay capability.
 * Callers must independently rediscover the native actor queue, current PO,
 * BG registry, resource and enclosing held scene/source transaction.
 */
#include "process.h"
#include "correspondence.h"
u32 _gz_bgo_relation(const PoEntry*s,const PoEntry*l,const VrActor*a,const VrActor*b,
 u32 sf,u32 lf,u32 se,u32 le){u32 i;
 if(!s||!l||!a||!b||!sf||!lf||!se||se>le||le>0xffffffu||lf-sf>=0x80000000u||
  s->state!=PO_LIVE||l->state!=PO_LIVE||s->flags||l->flags||s->end_frame||l->end_frame||
  !s->generation||!l->generation||!s->pid||!l->pid||s->pid>=0xfffffffeu||l->pid>=0xfffffffeu||
  !s->object||!l->object||(s->object&3u)||(l->object&3u)||s->profile>=484||l->profile!=s->profile||
  s->process_name!=s->profile||l->process_name!=s->profile||!s->profile_entry||s->profile_entry!=l->profile_entry||
  s->allocation_bytes<0x300u||s->allocation_bytes>=0x1000000u||l->allocation_bytes!=s->allocation_bytes||
  s->base_bytes+s->other_bytes<s->base_bytes||l->base_bytes+l->other_bytes<l->base_bytes||
  s->base_bytes+s->other_bytes!=s->allocation_bytes||l->base_bytes!=s->base_bytes||l->other_bytes!=s->other_bytes||
  sf-s->born_frame>=0x80000000u||lf-l->born_frame>=0x80000000u||
  a->actor!=s->object||b->actor!=l->object||a->pid!=s->pid||b->pid!=l->pid||
  a->profile!=s->profile||b->profile!=l->profile||a->parameters!=b->parameters||a->set_id!=b->set_id||
  a->parent_pid!=b->parent_pid||a->init_state!=2||b->init_state!=2)return 0;
 for(i=0;i<5;i++)if(a->home[i]!=b->home[i])return 0;
 if(s->object==l->object&&s->pid==l->pid&&s->generation==l->generation){
  const u32*x=(const u32*)s,*y=(const u32*)l;for(i=0;i<16;i++)if(x[i]!=y[i])return 0;return 1;}
 return le>se&&l->pid>s->pid&&l->generation>s->generation&&l->born_frame!=sf&&l->born_frame-sf<0x80000000u?2:0;
}
