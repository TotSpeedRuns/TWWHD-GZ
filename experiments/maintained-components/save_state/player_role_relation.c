/* Numeric relationship only, never native lifetime or replay authority.
 * The caller separately authenticates the pinned World/birth archive, official
 * current player root, unique native actor-queue membership, complete observed
 * process extent and all fresh resource/heap bindings. No old object is read.
 * A reborn player is not a surviving allocation merely because its address
 * was reused. Same-role replay must target only the newly bound current heap.
 */
#include "process.h"
u32 _gz_lpr_relation(const PoEntry*s,const PoEntry*l,u32 sf,u32 lf,u32 se,u32 le){
 u32 i;const u32*a=(const u32*)s,*b=(const u32*)l;
 if(!s||!l||!sf||!lf||!se||se>0x00ffffffu||le<se||le>0x00ffffffu||lf-sf>=0x80000000u)return 0;
 if(s->state!=PO_LIVE||l->state!=PO_LIVE||s->flags||l->flags||s->end_frame||l->end_frame||
  !s->generation||!l->generation||s->process_name!=168||l->process_name!=168||s->profile!=168||l->profile!=168||
  !s->profile_entry||s->profile_entry!=l->profile_entry||!s->object||!l->object||
  s->pid>=0xfffffffeu||l->pid>=0xfffffffeu||s->allocation_bytes<0x8284u||s->allocation_bytes>0x100000u||
  s->base_bytes+s->other_bytes<s->base_bytes||s->base_bytes+s->other_bytes!=s->allocation_bytes||
  l->base_bytes!=s->base_bytes||l->other_bytes!=s->other_bytes||l->allocation_bytes!=s->allocation_bytes||
  sf-s->born_frame>=0x80000000u||lf-l->born_frame>=0x80000000u)return 0;
 if(s->object==l->object&&s->pid==l->pid&&s->generation==l->generation){
  for(i=0;i<sizeof(*s)/4;i++)if(a[i]!=b[i])return 0;return 1;
 }
 /* Observed scene retirement/rebirth, with newer native generation and PID.
  * This intentionally excludes unrelated same-scene replacements and archives
  * from another process-observer session (checked separately by the caller). */
 return le>se&&l->generation>s->generation&&l->pid>s->pid&&
  l->born_frame-sf<0x80000000u&&l->born_frame!=sf?2:0;
}
