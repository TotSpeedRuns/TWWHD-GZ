/* Included after JEL's locked metadata helpers. These records describe call
 * causality, never native actor/effect ownership or permission to restore. */
#include "jel_jex_snapshot.h"
#include "jex_window.h"
u32 _gz_jex_report[24]={0x4a455831u,96,2};
JexEvent _gz_jex_events[JEX_CAP];
static JexOwner _gz_jex_owners[JEL_ROWS];
static JexScope _gz_jex_scopes[JEX_DEPTH];
static u32 _gz_jex_depth,_gz_jex_cookie;
static u32 _gz_jex_returned[JEX_CAP/32];
extern u32 _gz_hg_platform_thread(void);
extern volatile u32 _gz_po_frame;
static void jex_fault(u32 bit){__sync_fetch_and_or(&_gz_jex_report[3],bit);}
/* All window operations execute under the original JEL lock. Event serials
 * are monotonic cookies; physical slots can recycle only after their native
 * removal has returned, or when their kind never owned a return receipt. */
static void jex_window_fault(u32 reason) {
 if(reason==JEXW_EXHAUSTED)jex_fault(JEX_OVERFLOW);
 else if(reason==JEXW_WRAP)jex_fault(JEX_WRAP);
 else jex_fault(JEX_CONTEXT);
}
static u32 jex_record(u32 kind,const JelRow*r,const JexOwner*o) {
 JexwPlan plan;JexEvent event;u32 reason;
 reason=jexw_plan(_gz_jex_report,(const u32*)_gz_jex_events,
                  _gz_jex_returned,kind,&plan);
 if(reason){jex_window_fault(reason);return 0;}
 event.serial=plan.serial;event.kind=kind;event.frame=_gz_po_frame;
 event.thread=_gz_hg_platform_thread();event.manager=r->manager;
 event.manager_epoch=r->manager_epoch;event.emitter=r->emitter;
 event.generation=r->serial;event.resource=r->resource;
 event.group=r->group;event.index=r->index;event.scope=o->scope;
 event.actor=o->actor;event.pid=o->pid;event.actor_generation=o->actor_generation;
 event.entry_procedure=o->entry_procedure;
 reason=jexw_commit(_gz_jex_report,(u32*)_gz_jex_events,_gz_jex_returned,
                    &plan,(const u32*)&event);
 if(reason){jex_window_fault(reason);return 0;}
 return event.serial;
}
/* Native C wrapper supplies no historical address. The parameters come from
 * its genuine current actor/process callback; this API dereferences none. */
u32 _gz_jex_begin(u32 actor,u32 pid,u32 generation,u32 procedure,u32 frame,u32 thread,u32 request){JexScope*c;u32 cookie=0;
 if(!actor||!pid||!generation||!frame||!thread||!request)return 0;
 if(thread!=_gz_hg_platform_thread()||frame!=_gz_po_frame){jex_fault(JEX_CONTEXT);return 0;}
 if(!enter(0)){jex_fault(JEX_LOST);return 0;}
 if(_gz_jex_depth>=JEX_DEPTH){jex_fault(JEX_CONTEXT);goto done;}
 if(_gz_jex_cookie==0xffffffffu){jex_fault(JEX_WRAP);goto done;}
 cookie=++_gz_jex_cookie;c=_gz_jex_scopes+_gz_jex_depth++;
 c->cookie=cookie;c->actor=actor;c->pid=pid;c->generation=generation;c->procedure=procedure;
 c->frame=frame;c->thread=thread;c->request=request;
 _gz_jex_report[8]++;_gz_jex_report[9]=_gz_jex_depth;_gz_jex_report[10]=request;
done:leave();return cookie;
}
void _gz_jex_end(u32 cookie,u32 frame,u32 thread){JexScope*c;
 if(!cookie)return;
 if(!enter(0)){jex_fault(JEX_LOST);return;}
 if(!_gz_jex_depth){jex_fault(JEX_CONTEXT);goto done;}
 c=_gz_jex_scopes+_gz_jex_depth-1;
 if(c->cookie!=cookie||c->thread!=thread||thread!=_gz_hg_platform_thread()||c->frame!=frame||frame!=_gz_po_frame){jex_fault(JEX_CONTEXT);goto done;}
 c->cookie=0;_gz_jex_depth--;_gz_jex_report[9]=_gz_jex_depth;_gz_jex_report[11]++;
done:leave();
}
/* Called with JEL lock held after a genuine native successful birth. An event
 * on another thread cannot borrow the main thread's active actor scope. */
static void jex_birth(JelRow*r){JexOwner*o=_gz_jex_owners+(r-_gz_jel_state.row);JexScope*c;u32 thread;
 o->generation=0;if(!_gz_jex_depth)return;
 c=_gz_jex_scopes+_gz_jex_depth-1;thread=_gz_hg_platform_thread();
 if(c->thread!=thread){if(_gz_jex_report[12]==0xffffffffu)jex_fault(JEX_WRAP);else _gz_jex_report[12]++;return;}
 if(c->frame!=_gz_po_frame||!c->cookie){jex_fault(JEX_CONTEXT);return;}
 o->manager=r->manager;o->emitter=r->emitter;o->generation=r->serial;
 o->scope=c->cookie;o->actor=c->actor;o->pid=c->pid;o->actor_generation=c->generation;
 o->entry_procedure=c->procedure;o->thread=thread;o->frame=c->frame;
 jex_record(1,r,o);
}
static u32 jex_ending(JelRow*r,u32 kind){JexOwner*o=_gz_jex_owners+(r-_gz_jel_state.row);u32 cookie;
 if(!o->generation)return 0;
 if(o->manager!=r->manager||o->emitter!=r->emitter||o->generation!=r->serial){jex_fault(JEX_CONTEXT);return 0;}
 cookie=jex_record(kind,r,o);o->generation=0;return cookie;
}
/* The C call wrapper retains the exact entry-event cookie on its own stack.
 * No actor/emitter/native list is followed after the original removal returns.
 * A returned call proves a CPU boundary, not GPU/worker completion or ownership. */
void _gz_jex_removed(u32 cookie) {
 JexwReturnPlan completed;const u32*event;JelRow row;JexOwner owner;u32 reason;
 if(!cookie)return;
 if(!enter(0)){jex_fault(JEX_LOST);return;}
 reason=jexw_plan_return(_gz_jex_report,(const u32*)_gz_jex_events,
                         _gz_jex_returned,cookie,_gz_po_frame,
                         _gz_hg_platform_thread(),&completed);
 if(reason){jex_window_fault(reason);goto done;}
 /* Preserve the exact entry locally before its slot becomes recyclable.
  * Record this real native return first, even if serial exhaustion prevents
  * a later completion event. Never set an old slot bit after append. */
 reason=jexw_mark_return(_gz_jex_report,(const u32*)_gz_jex_events,
                         _gz_jex_returned,&completed);
 if(reason){jex_window_fault(reason);goto done;}
 event=completed.event;
 row.manager=event[4];row.manager_epoch=event[5];
 row.emitter=event[6];row.serial=event[7];
 row.resource=event[8];row.group=event[9];row.index=event[10];row.alive=0;
 owner.manager=event[4];owner.emitter=event[6];owner.generation=event[7];
 owner.scope=event[11];owner.actor=event[12];owner.pid=event[13];
 owner.actor_generation=event[14];owner.entry_procedure=event[15];
 owner.thread=event[3];owner.frame=event[2];
 (void)jex_record(4,&row,&owner);
done:leave();
}
