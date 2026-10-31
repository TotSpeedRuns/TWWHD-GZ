/* Included after JEL's locked metadata helpers. These records describe call
 * causality, never native actor/effect ownership or permission to restore. */
enum {JEX_CAP=512,JEX_DEPTH=4,JEX_OVERFLOW=1,JEX_LOST=2,JEX_CONTEXT=4,JEX_WRAP=8};
typedef struct {u32 serial,kind,frame,thread,manager,manager_epoch,emitter,generation;
 u32 resource,group,index,scope,actor,pid,actor_generation,entry_procedure;} JexEvent;
typedef struct {u32 manager,emitter,generation,scope,actor,pid,actor_generation,entry_procedure,thread,frame;} JexOwner;
typedef struct {u32 cookie,actor,pid,generation,procedure,frame,thread,request;} JexScope;
typedef char JexEventSize[sizeof(JexEvent)==64?1:-1];
u32 _gz_jex_report[24]={0x4a455831u,96,1};
JexEvent _gz_jex_events[JEX_CAP];
static JexOwner _gz_jex_owners[JEL_ROWS];
static JexScope _gz_jex_scopes[JEX_DEPTH];
static u32 _gz_jex_depth,_gz_jex_cookie;
static u32 _gz_jex_returned[JEX_CAP/32];
extern u32 _gz_hg_platform_thread(void);
extern volatile u32 _gz_po_frame;
static void jex_fault(u32 bit){__sync_fetch_and_or(&_gz_jex_report[3],bit);}
static u32 jex_record(u32 kind,const JelRow*r,const JexOwner*o){JexEvent*e;u32 n=_gz_jex_report[4];
 if(n>=JEX_CAP){jex_fault(JEX_OVERFLOW);return 0;}
 e=_gz_jex_events+n;e->serial=n+1;e->kind=kind;e->frame=_gz_po_frame;
 e->thread=_gz_hg_platform_thread();e->manager=r->manager;e->manager_epoch=r->manager_epoch;
 e->emitter=r->emitter;e->generation=r->serial;e->resource=r->resource;e->group=r->group;e->index=r->index;
 e->scope=o->scope;e->actor=o->actor;e->pid=o->pid;e->actor_generation=o->actor_generation;e->entry_procedure=o->entry_procedure;
 __sync_synchronize();_gz_jex_report[4]=n+1;
 if(kind==1)_gz_jex_report[5]++;else if(kind==2)_gz_jex_report[6]++;else if(kind==3)_gz_jex_report[7]++;else if(kind==4)_gz_jex_report[13]++;
 return n+1;
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
 if(c->thread!=thread){_gz_jex_report[12]++;return;}
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
void _gz_jex_removed(u32 cookie){const JexEvent*e;JelRow r;JexOwner o;u32 index,bit;
 if(!cookie)return;
 if(!enter(0)){jex_fault(JEX_LOST);return;}
 if(cookie>_gz_jex_report[4]||cookie>JEX_CAP){jex_fault(JEX_CONTEXT);goto done;}
 e=_gz_jex_events+cookie-1;index=(cookie-1)/32;bit=1u<<((cookie-1)%32);
 if(e->serial!=cookie||e->kind!=2||e->thread!=_gz_hg_platform_thread()||
  _gz_po_frame-e->frame>=0x80000000u||(_gz_jex_returned[index]&bit)){
  jex_fault(JEX_CONTEXT);goto done;}
 r.manager=e->manager;r.manager_epoch=e->manager_epoch;r.emitter=e->emitter;r.serial=e->generation;
 r.resource=e->resource;r.group=e->group;r.index=e->index;r.alive=0;
 o.manager=e->manager;o.emitter=e->emitter;o.generation=e->generation;o.scope=e->scope;
 o.actor=e->actor;o.pid=e->pid;o.actor_generation=e->actor_generation;o.entry_procedure=e->entry_procedure;
 o.thread=e->thread;o.frame=e->frame;
 if(jex_record(4,&r,&o))_gz_jex_returned[index]|=bit;
done:leave();
}
