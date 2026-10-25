/* Small immutable source/current identity receipt. Pure integer metadata:
 * neither this record nor a matching PID grants a native lifetime or writer.
 * The service separately holds the current native queue/heap/resource scope.
 */
#include "process.h"
typedef struct {
 u32 magic,relation,source_pid,source_generation,source_frame,current_frame;
 u32 source_epoch,current_epoch,actor,pid,generation,seal;
} RriBinding;
typedef char RriBindingSize[sizeof(RriBinding)==48?1:-1];
static u32 rri_hash(const RriBinding*b){const u32*p=(const u32*)b;u32 i,h=2166136261u;
 for(i=0;i<11;i++)h=(h^p[i])*16777619u;return h;}
u32 _gz_rri_record_match(const RriBinding*b,const PoEntry*e,u32 pid,u32 gen,u32 frame,u32 epoch){
 if(!b||!e||b->magic!=0x52524931u||b->relation!=2||b->seal!=rri_hash(b)||
  !b->source_pid||!b->source_generation||!b->source_frame||!b->current_frame||
  b->source_pid>=0xfffffffeu||b->pid>=0xfffffffeu||b->pid<=b->source_pid||
  b->generation<=b->source_generation||b->current_frame-b->source_frame>=0x80000000u||
  !b->source_epoch||b->source_epoch>=b->current_epoch||b->current_epoch!=epoch||epoch>0xffffffu||
  b->actor!=e->object||b->pid!=e->pid||b->generation!=e->generation||
  e->state!=PO_LIVE||e->flags||e->end_frame||e->allocation_bytes!=0xa7cu||
  pid!=b->source_pid||gen!=b->source_generation||frame!=b->source_frame)return 0;
 return 1;
}
