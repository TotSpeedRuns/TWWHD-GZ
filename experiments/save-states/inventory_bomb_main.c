#include "archive_layout.h"
#include "inventory_bomb.h"
#include "world_slot.h"
extern volatile u32 _gz_roh_live_frame,_gz_roh_live_epoch,_gz_v73_state_valid;
extern u32 _gz_roh_user_guard(void),_gz_roh_work_replaced(void);
PbContext _gz_b294m_context;
#define API __attribute__((noinline))
static u32 equal(const u8*a,const u8*b,u32 n){u32 i;for(i=0;i<n;i++)if(a[i]!=b[i])return 0;return 1;}
API u32 _gz_b294m_source_matches(u32 source,u32 bytes,const VrWorld*w){u32 i,j,count=0;const PbBank*b;
 if(_gz_b294_validate(source,bytes)||_gz_vws_validate(w,sizeof(*w)))return 28;
 if(!equal((const u8*)(source+20),(const u8*)&w->stage0,8)||*(const u32*)(source+28)!=w->room||*(const u32*)(source+32)!=w->layer||
  *(const u32*)(source+36)!=w->epoch||*(const u32*)(source+40)!=w->frame)return 28;
 b=(const PbBank*)(source+B294_HEADER);
 for(i=0;i<w->count;i++)if(w->actors[i].profile==294){const VrActor*a=&w->actors[i];const PbRow*r;
  for(j=0;j<b->count;j++)if(b->rows[j].pid==a->pid)break;
  if(j==b->count)return 28;r=&b->rows[j];
  if(r->actor!=a->actor||r->parameters!=a->parameters||r->parent!=a->parent_pid||!equal(r->home,(const u8*)a->home,20))return 28;
  count++;
 }
 return count==b->count?0:28;
}
API u32 _gz_b294m_stage_capture(void){const VrWorld*w=&_gz_vws_stage;u32 e;
 if(_gz_roh_user_guard())return 5;
 e=_gz_vws_validate_ready();if(e)return e;
 e=_gz_roh_work_replaced();if(e)return e;
 _gz_b294m_context.stage0=w->stage0;_gz_b294m_context.stage1=w->stage1;_gz_b294m_context.room=w->room;_gz_b294m_context.layer=w->layer;
 _gz_b294m_context.generation=w->epoch;_gz_b294m_context.frame=w->frame;
 e=_gz_b294_stage_capture(w->queue_head,&_gz_b294m_context);if(e)return e;
 return _gz_b294m_source_matches((u32)_gz_b294_stage,B294_BYTES,w);
}
API u32 _gz_b294m_validate_ready(void){u32 e=_gz_vws_validate_ready();if(e)return e;
 if(_gz_roh_live_frame!=_gz_b294m_context.frame||_gz_roh_live_epoch!=_gz_b294m_context.generation)return 9;
 e=_gz_b294_validate_ready(&_gz_b294m_context);if(e)return e;
 return _gz_b294m_source_matches((u32)_gz_b294_stage,B294_BYTES,&_gz_vws_stage);
}
API u32 _gz_b294m_validate_current(void){u32 e;if(_gz_v73_state_valid!=1)return 28;
 e=_gz_vws_validate_current();if(e)return e;
 return _gz_b294m_source_matches((u32)_gz_b294_source,B294_BYTES,&_gz_vws_source);
}
/* Capture/archive integration is deliberately independent of replay admission.
 * Never silently report success while an archived placed bomb is omitted. */
API u32 _gz_b294m_restore_preflight(void){u32 e=_gz_b294m_validate_current();if(e)return e;
 return _gz_b294_restore_admission((u32)_gz_b294_source,B294_BYTES);
}

API u32 _gz_b294m_archive_validate(u32 p,u32 n,u32 installed){u32 b,world;
 if(!gz_arc_shape(p,n)||*(const u32*)(p+172)!=sizeof(VrWorld)||*(const u32*)(p+212)!=B294_BYTES)return 42;
 world=p+*(const u32*)(p+168);b=p+*(const u32*)(p+208);
 if(_gz_b294m_source_matches(b,B294_BYTES,(const VrWorld*)world))return 42;
 if(installed&&!equal(_gz_b294_source,(const u8*)b,B294_BYTES))return 42;
 return 0;
}
