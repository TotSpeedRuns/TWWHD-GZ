#include "emitter_lifetime.h"
/* Private native observation starts before the first real manager constructor.
 * This supplies identities only; it grants no particle replay permission. */
JelState _gz_jel_state={.magic=0x4a454c31u,.bytes=sizeof(JelState),.version=1,.enabled=1};
u32 _gz_jel_native_audit[16];
JelPool _gz_jel_pools[JEL_MANAGERS];
JelAuxPool _gz_jel_aux_pools[JEL_MANAGERS];
typedef char JelStateStill10880[sizeof(JelState)==10880?1:-1];
typedef char JelPoolsStill256[sizeof(_gz_jel_pools)==256?1:-1];
typedef char JelAuxPools256[sizeof(_gz_jel_aux_pools)==256?1:-1];
static JelPool*pool(u32 p){u32 i;for(i=0;i<JEL_MANAGERS;i++)if(_gz_jel_pools[i].manager==p)return _gz_jel_pools+i;return 0;}
static JelAuxPool*auxiliary_pool(JelPool*q){return _gz_jel_aux_pools+(q-_gz_jel_pools);}
/* Pure range exclusion for coupled generic metadata-output guards. The
 * caller must still apply every original exclusion and admission predicate. */
u32 _gz_jel_aux_private_apart(u32 p,u32 n){u32 a=(u32)_gz_jel_aux_pools;
 if(!n||n>0xffffffffu-p)return 0;
 return p>=a?p-a>=sizeof(_gz_jel_aux_pools):a-p>=n;
}
static void poison(u32 x){__sync_fetch_and_or(&_gz_jel_state.flags,x);}
/* Never wait in a hook, including a same-core preemption. A dropped event
 * invalidates the ledger permanently; the native operation still continues.
 * This lock protects metadata only and supplies no JPA producer exclusion. */
static u32 enter(u32 event){
 JelState*s=&_gz_jel_state;
 if(!*(volatile u32*)&s->enabled)return 0;
 if(!__sync_bool_compare_and_swap(&s->busy,0,1)){if(event)poison(JEL_LOST);return 0;}
 if(s->magic!=0x4a454c31u||s->bytes!=sizeof(*s)||s->version!=1||s->enabled!=1){poison(JEL_BAD);s->busy=0;return 0;}
 if(s->flags){s->busy=0;return 0;}
 return 1;
}
static void leave(void){__sync_synchronize();*(volatile u32*)&_gz_jel_state.busy=0;}
static u32 next(void){JelState*s=&_gz_jel_state;
 if(s->serial==0xffffffffu){poison(JEL_WRAP);return 0;}
 s->events++;return ++s->serial;
}
static JelManager*manager(u32 p){u32 i;for(i=0;i<JEL_MANAGERS;i++)if(_gz_jel_state.manager[i].pointer==p)return _gz_jel_state.manager+i;return 0;}
static JelRow*row(u32 m,u32 e){u32 i;for(i=0;i<JEL_ROWS;i++)if(_gz_jel_state.row[i].manager==m&&_gz_jel_state.row[i].emitter==e)return _gz_jel_state.row+i;return 0;}
#include "emitter_attribution.inc"
#include "auxiliary_manager_event.inc"
void _gz_jel_birth(u32 p,u32 e,u32 resource,u32 group,u32 index){JelManager*m;JelRow*r;u32 i,n,active=0;
 if(!enter(1))return;
 _gz_jel_native_audit[3]++;_gz_jel_native_audit[4]=p;_gz_jel_native_audit[5]=e;
 /* The native return epilogue is also reached after an unsuccessful create.
  * Null does not name a pool slot and cannot mint an identity. */
 if(!e)goto done;
 m=manager(p);
 if(!m){poison(JEL_UNSEEN);goto done;}
 if((e&3)||resource>0xffffu||group>=16||index>=8){poison(JEL_BAD);goto done;}
 r=row(p,e);
 if(r&&r->alive){poison(JEL_BAD);goto done;}
 for(i=0;i<JEL_ROWS;i++)if(_gz_jel_state.row[i].manager==p&&_gz_jel_state.row[i].alive)active++;
 if(active>=m->capacity){poison(JEL_LIMIT);goto done;}
 if(!r){for(i=0;i<JEL_ROWS;i++)if(!_gz_jel_state.row[i].alive){r=_gz_jel_state.row+i;break;}}
 if(!r){poison(JEL_LIMIT);goto done;}
 n=next();if(!n)goto done;
 r->manager=p;r->manager_epoch=m->epoch;r->emitter=e;r->serial=n;
 r->resource=resource;r->group=group;r->index=index;r->alive=1;
 jex_birth(r);
done:leave();
}
u32 _gz_jel_remove_begin(u32 p,u32 e){JelRow*r;u32 n,cookie=0;
 if(!enter(1))return 0;
 _gz_jel_native_audit[6]++;_gz_jel_native_audit[7]=p;_gz_jel_native_audit[8]=e;
 r=row(p,e);
 if(!r||!r->alive){poison(JEL_UNSEEN);goto done;}
 /* Invalidate at native removal ENTRY, before the first teardown callback or
  * free-list edit. A returned failure must not resurrect this generation. */
 cookie=jex_ending(r,2);r->alive=0;n=next();(void)n;
done:leave();return cookie;
}
/* Existing metadata-only callers keep their entry-only contract. */
void _gz_jel_remove(u32 p,u32 e){(void)_gz_jel_remove_begin(p,e);}
extern u32 _gz_jex_original_remove(u32,u32);
u32 _gz_jex_remove_wrapper(u32 p,u32 e){u32 cookie,result;
 cookie=_gz_jel_remove_begin(p,e);
 result=_gz_jex_original_remove(p,e);
 _gz_jex_removed(cookie);return result;
}
u32 _gz_jel_lookup(u32 p,u32 e){JelManager*m;JelRow*r;u32 n=0;
 if(!enter(0))return 0;
 m=manager(p);r=row(p,e);
 if(m&&r&&r->alive&&r->manager_epoch==m->epoch)n=r->serial;
 leave();return n;
}
u32 _gz_jel_match(u32 p,u32 e,u32 serial,u32 resource,u32 group,u32 index){JelManager*m;JelRow*r;u32 ok=0;
 if(!serial||!enter(0))return 0;
 m=manager(p);r=row(p,e);
 if(m&&r&&r->alive&&r->manager_epoch==m->epoch&&r->serial==serial&&r->resource==resource&&r->group==group&&r->index==index)ok=1;
 leave();return ok;
}
static JelChild*child(u32 p){u32 i;for(i=0;i<JEL_CHILDREN;i++)if(_gz_jel_state.child[i].pointer==p)return _gz_jel_state.child+i;return 0;}
void _gz_jel_child_born(u32 p){JelChild*c;u32 i,n;
 if(!enter(1))return;
 if(!p)goto done;
 if(p&3){poison(JEL_BAD);goto done;}
 c=child(p);
 if(!c){for(i=0;i<JEL_CHILDREN;i++)if(!_gz_jel_state.child[i].alive){c=_gz_jel_state.child+i;break;}}
 if(!c){poison(JEL_LIMIT);goto done;}
 n=next();if(!n)goto done;
 c->pointer=p;c->epoch=n;c->alive=1;c->reserved=0;
 for(i=0;i<2;i++)c->emitter[i]=c->generation[i]=c->manager[i]=0;
done:leave();
}
void _gz_jel_child_ending(u32 p){JelChild*c;
 if(!enter(1))return;
 if(!p)goto done;
 c=child(p);if(!c||!c->alive){poison(JEL_UNSEEN);goto done;}
 c->alive=0;(void)next();
done:leave();
}
void _gz_jel_edge(u32 p,u32 e,u32 edge){JelChild*c;JelRow*r=0;JelManager*m;u32 i;
 if(!enter(1))return;
 c=child(p);
 if(!c||!c->alive){poison(JEL_UNSEEN);goto done;}
 if(edge!=0x58&&edge!=0x5c){poison(JEL_BAD);goto done;}
 edge=(edge-0x58)>>2;
 if(!e){c->emitter[edge]=c->generation[edge]=c->manager[edge]=0;(void)next();goto done;}
 for(i=0;i<JEL_ROWS;i++)if(_gz_jel_state.row[i].emitter==e&&_gz_jel_state.row[i].alive){
  if(r){poison(JEL_BAD);goto done;}r=_gz_jel_state.row+i;
 }
 if(!r){poison(JEL_UNSEEN);goto done;}
 m=manager(r->manager);
 if(!m||m->epoch!=r->manager_epoch){poison(JEL_UNSEEN);goto done;}
 if(!next())goto done;
 c->emitter[edge]=e;c->generation[edge]=r->serial;c->manager[edge]=r->manager;
done:leave();
}
u32 _gz_jel_child_epoch(u32 p){JelChild*c;u32 n=0;
 if(!enter(0))return 0;
 c=child(p);if(c&&c->alive)n=c->epoch;
 leave();return n;
}
u32 _gz_jel_edge_generation(u32 p,u32 epoch,u32 edge,u32 e){JelChild*c;JelRow*r;JelManager*m;u32 n=0;
 if(!epoch||(edge!=0x58&&edge!=0x5c)||!enter(0))return 0;
 edge=(edge-0x58)>>2;c=child(p);
 if(!c||!c->alive||c->epoch!=epoch||c->emitter[edge]!=e||!c->generation[edge])goto done;
 r=row(c->manager[edge],e);m=manager(c->manager[edge]);
 if(r&&m&&r->alive&&r->manager_epoch==m->epoch&&r->serial==c->generation[edge])n=r->serial;
done:leave();return n;
}

/* Native manager constructor entry after a non-null this has been obtained,
 * before its lists and arrays are initialized. Retire prior receipts before
 * any same-address construction can overwrite the old manager. */
void _gz_jel_constructing(u32 p){JelPool*q;JelAuxPool*a;JelManager*m;u32 i,n;
 if(!enter(1))return;
 _gz_jel_native_audit[9]++;_gz_jel_native_audit[10]=p;
 if(!p||(p&3)){poison(JEL_BAD);goto done;}
 q=pool(p);
 if(!q){for(i=0;i<JEL_MANAGERS;i++)if(!_gz_jel_pools[i].manager){q=_gz_jel_pools+i;break;}}
 if(!q){poison(JEL_LIMIT);goto done;}
 a=auxiliary_pool(q);
 /* Invalidate immediately, even if serial exhaustion refuses the event.
  * Native construction resumes only after this observer hook returns. */
 a->manager=p;a->construction=0;a->base=a->count=a->serial=a->epoch=0;
 a->phase=JEL_AUX_CONSTRUCTING;a->reserved=0;
 n=next();if(!n)goto done;
 m=manager(p);if(m)m->epoch=0;
 for(i=0;i<JEL_ROWS;i++)if(_gz_jel_state.row[i].manager==p){if(_gz_jel_state.row[i].alive)jex_ending(_gz_jel_state.row+i,3);_gz_jel_state.row[i].alive=0;}
 q->manager=p;q->construction=n;q->base=0;q->capacity=0;q->serial=0;q->epoch=0;q->phase=1;q->reserved=0;
 a->construction=n;
done:leave();
}
/* Native new-array runtime has returned its first constructed object. This
 * hook observes r3 before mr. r30,r3 destroys the original capacity in r30. */
void _gz_jel_pool(u32 p,u32 e,u32 count){JelPool*q;u32 n;
 if(!enter(1))return;
 _gz_jel_native_audit[11]++;_gz_jel_native_audit[12]=p;_gz_jel_native_audit[13]=e;_gz_jel_native_audit[14]=count;
 q=pool(p);if(!q||q->phase!=1||!q->construction){poison(JEL_UNSEEN);goto done;}
 if(!e&&count){q->phase=3;goto done;}
 if((e&3)||count>JEL_ROWS||count*0x3fcu>0xffffffffu-e){poison(JEL_BAD);goto done;}
 n=next();if(!n)goto done;
 q->base=e;q->capacity=count;q->serial=n;q->epoch=0;q->phase=2;
done:leave();
}
u32 _gz_jel_pool_match(u32 p,u32 e,u32 generation){JelPool*q;JelManager*m;JelRow*r;u32 n=0,d;
 if(!generation||!enter(0))return 0;
 q=pool(p);m=manager(p);r=row(p,e);
 if(!q||q->phase!=2||!q->construction||!q->serial||!q->base||!q->capacity||q->capacity>JEL_ROWS||
  !m||!m->epoch||q->epoch!=m->epoch||q->capacity!=m->capacity||!r||!r->alive||r->manager_epoch!=m->epoch||r->serial!=generation||e<q->base)goto done;
 d=e-q->base;if(d<q->capacity*0x3fcu&&d%0x3fcu==0)n=q->serial;
done:leave();return n;
}

#include "jex_quiet_query.inc"
