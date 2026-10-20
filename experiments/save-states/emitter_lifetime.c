#include "emitter_lifetime.h"
JelState _gz_jel_state={.magic=0x4a454c31u,.bytes=sizeof(JelState),.version=1};
JelPool _gz_jel_pools[JEL_MANAGERS];
static JelPool*pool(u32 p){u32 i;for(i=0;i<JEL_MANAGERS;i++)if(_gz_jel_pools[i].manager==p)return _gz_jel_pools+i;return 0;}
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
void _gz_jel_manager(u32 p,u32 capacity){JelManager*m;u32 i,n;
 if(!enter(1))return;
 if(!p)goto done;
 if((p&3)||capacity>JEL_ROWS){poison(JEL_BAD);goto done;}
 m=manager(p);
 if(!m){for(i=0;i<JEL_MANAGERS;i++)if(!_gz_jel_state.manager[i].pointer){m=_gz_jel_state.manager+i;break;}}
 if(!m){poison(JEL_LIMIT);goto done;}
 n=next();if(!n)goto done;
 for(i=0;i<JEL_ROWS;i++)if(_gz_jel_state.row[i].manager==p)_gz_jel_state.row[i].alive=0;
 m->pointer=p;m->epoch=n;m->capacity=capacity;
 {JelPool*q=pool(p);
  if(q&&q->phase==2&&q->capacity==capacity&&q->construction&&q->serial)q->epoch=n;
 }
done:leave();
}
void _gz_jel_birth(u32 p,u32 e,u32 resource,u32 group,u32 index){JelManager*m;JelRow*r;u32 i,n,active=0;
 if(!enter(1))return;
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
done:leave();
}
void _gz_jel_remove(u32 p,u32 e){JelRow*r;u32 n;
 if(!enter(1))return;
 r=row(p,e);
 if(!r||!r->alive){poison(JEL_UNSEEN);goto done;}
 /* Invalidate at native removal ENTRY, before the first teardown callback or
  * free-list edit. A returned failure must not resurrect this generation. */
 r->alive=0;n=next();(void)n;
done:leave();
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
void _gz_jel_constructing(u32 p){JelPool*q;JelManager*m;u32 i,n;
 if(!enter(1))return;
 if(!p||(p&3)){poison(JEL_BAD);goto done;}
 q=pool(p);
 if(!q){for(i=0;i<JEL_MANAGERS;i++)if(!_gz_jel_pools[i].manager){q=_gz_jel_pools+i;break;}}
 if(!q){poison(JEL_LIMIT);goto done;}
 n=next();if(!n)goto done;
 m=manager(p);if(m)m->epoch=0;
 for(i=0;i<JEL_ROWS;i++)if(_gz_jel_state.row[i].manager==p)_gz_jel_state.row[i].alive=0;
 q->manager=p;q->construction=n;q->base=0;q->capacity=0;q->serial=0;q->epoch=0;q->phase=1;q->reserved=0;
done:leave();
}
/* Native new-array runtime has returned its first constructed object. This
 * hook observes r3 before mr. r30,r3 destroys the original capacity in r30. */
void _gz_jel_pool(u32 p,u32 e,u32 count){JelPool*q;u32 n;
 if(!enter(1))return;
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
