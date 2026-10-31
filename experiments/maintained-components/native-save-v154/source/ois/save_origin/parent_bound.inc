/* One exact accepted SRO/SRW value kernel, then additional scalar observations.
 * No native wrapper ASM/hook, native memory access, allocation or callback is
 * linked. Public events remain unauthenticated source-only future hook APIs. */
#include "observation.h"
/* Retain exact accepted bodies privately, then extend BOTH old public output
 * APIs' exclusion union to this new state. Full function-name aliases only;
 * no struct/member spelling or second SRO/SRW owner is introduced. */
static SroU32 _gz_spb_core_query(SroU32,SroU32,SroU32,SroValue*) __attribute__((noinline,noclone));
static SroU32 _gz_spb_wrapper_query(SroU32,SroU32,SroU32,SroValue*) __attribute__((noinline,noclone));
#define _gz_sro_query _gz_spb_core_query
#define _gz_srw_query _gz_spb_wrapper_query
#include "callbacks.c"
#undef _gz_sro_query
#undef _gz_srw_query
#include "parent_backing.h"
typedef struct {SpbValue v;SpbU32 entered,poison,error,scope_phase,scope_parent,scope_parent_epoch;} SpbPrivate;
static SpbPrivate _gz_spb_private={{SPB_MAGIC,SPB_VERSION,SPB_BYTES,SPB_EMPTY,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,{0,0,0,0,0,0,0,0}},0,0,0,0,0,0};
typedef char SpbPrivateSize[(sizeof(SpbPrivate)==152)?1:-1];
static SpbU32 spb_pointer(SpbU32 p){return p&&!(p&3);}
static SpbU32 spb_end(SpbU32 p,SpbU32 n,SpbU32*out){if(!p||n>0xffffffffu-p)return 0;*out=p+n;return 1;}
static SpbU32 spb_apart(SpbU32 a,SpbU32 n,SpbU32 b,SpbU32 m){SpbU32 x,y;return n&&m&&spb_end(a,n,&x)&&spb_end(b,m,&y)&&(x<=b||y<=a);}
static SpbU32 spb_overlap(SpbU32 lo,SpbU32 hi,SpbU32 p,SpbU32 n){SpbU32 end;return lo<hi&&n&&spb_end(p,n,&end)&&lo<end&&p<hi;}
static void spb_scope_clear(void){_gz_spb_private.scope_phase=0;_gz_spb_private.scope_parent=0;_gz_spb_private.scope_parent_epoch=0;}
static SpbU32 spb_header(void){const SpbPrivate*s=&_gz_spb_private;const SpbValue*v=&s->v;SpbU32 i,end,meta;
 if(v->magic!=SPB_MAGIC||v->version!=SPB_VERSION||v->bytes!=SPB_BYTES||v->phase>SPB_RETIRED||v->parent_live>1||v->backing_live>1||v->root_bound>1||s->scope_phase>2||s->poison>1||s->entered>1||v->last_event>SPB_BIRTH_MISMATCH||v->retire_reason>SPB_BIRTH_MISMATCH||v->parent_retire_reason>SPB_BIRTH_MISMATCH)return 0;
 for(i=0;i<8;i++)if(v->reserved[i])return 0;
 if(!v->parent_epoch){if(v->parent||v->parent_live||v->phase!=SPB_EMPTY||v->backing_epoch||v->scope_serial)return 0;}
 else if(!spb_pointer(v->parent)||v->phase==SPB_EMPTY||(v->parent_live?v->parent_retire_reason!=0:v->parent_retire_reason!=SPB_PARENT_DESTROY))return 0;
 if(!s->scope_phase){if(s->scope_parent||s->scope_parent_epoch)return 0;}
 else if(!v->scope_serial||!v->parent_live||s->scope_parent!=v->parent||s->scope_parent_epoch!=v->parent_epoch)return 0;
 if(!v->child){
  if(v->user||v->backing_bytes||v->direction||v->root||v->root_generation||v->root_bytes||v->initial_backing_bytes||v->backing_start||v->backing_end||v->backing_live||v->root_bound)return 0;
  if(v->phase!=SPB_EMPTY&&v->phase!=SPB_PARENT&&!(v->phase==SPB_RETIRED&&!v->parent_live))return 0;
 }else{
  if(!v->backing_epoch||!v->scope_serial||v->child!=v->user||!spb_pointer(v->child)||v->direction!=1||v->initial_backing_bytes<SPB_CHILD_BYTES||v->backing_bytes>v->initial_backing_bytes||v->backing_start!=v->user||!spb_end(v->user,v->backing_bytes,&end)||end!=v->backing_end)return 0;
  if(v->backing_live&&(!v->parent_live||v->backing_bytes<SPB_CHILD_BYTES||v->phase<SPB_BORN||v->phase>SPB_BOUND))return 0;
  if(!v->backing_live&&v->phase!=SPB_RETIRED)return 0;
 }
 if(v->root){if(!spb_pointer(v->root)||!v->root_generation||v->root_bytes!=SRO_CAPACITY||!spb_end(v->root,v->root_bytes,&end))return 0;}
 else if(v->root_generation||v->root_bytes||v->root_bound)return 0;
 if(v->root_bound){if(!v->backing_live||v->phase!=SPB_BOUND||!spb_end(v->child,SPB_CHILD_BYTES,&meta)||v->root<meta||!spb_end(v->root,v->root_bytes,&end)||end>v->backing_end)return 0;}
 if(v->phase==SPB_BOUND&&!v->root_bound)return 0;
 if(v->phase==SPB_BORN&&s->scope_phase!=2)return 0;
 if(s->scope_phase==1&&v->backing_live)return 0;
 if(s->scope_phase==2&&(!v->backing_live||v->phase!=SPB_BORN))return 0;
 return 1;
}
static SpbU32 spb_begin(void){if(_gz_spb_private.entered)return SPB_BUSY;if(_gz_spb_private.poison)return SPB_DENIED;if(!spb_header())return SPB_CORRUPT;_gz_spb_private.entered=1;return 0;}
static SpbU32 spb_return(SpbU32 z){_gz_spb_private.entered=0;return z;}
static SpbU32 spb_selected(SpbU32 p,SpbU32 u,SpbU32 pe,SpbU32 be){const SpbValue*v=&_gz_spb_private.v;
 if(pe!=v->parent_epoch||be!=v->backing_epoch)return SPB_CHANGED;
 if(!v->backing_live||p!=v->parent||u!=v->user)return SPB_NO_CHANGE;
 return 0;
}
static SpbU32 spb_retire(SpbU32 reason,SpbU32 backing,SpbU32 parent){SpbValue*v=&_gz_spb_private.v;SpbU32 bound=v->root_bound,z=0;
 /* Own eligibility is revoked BEFORE any nested core refusal. Retirement must
  * work atFFFFFFFF; no counter increment/copy/native read follows. */
 v->root_bound=0;v->last_event=reason;v->retire_reason=reason;
 if(backing){v->backing_live=0;v->phase=SPB_RETIRED;spb_scope_clear();}else if(v->backing_live)v->phase=SPB_RETURNED;
 if(parent){v->parent_live=0;v->parent_retire_reason=reason;}
 if(bound&&sro_header()&&_gz_sro_private.root==v->root&&_gz_sro_private.heap==v->child&&_gz_sro_private.generation==v->root_generation){
  z=_gz_sro_observe(SRO_DIRECT_FREE_BEGIN,v->root,v->child,v->root_generation);
  if(z==SRO_NO_CHANGE)z=0;
  if(z){_gz_spb_private.poison=1;_gz_spb_private.error=z;}
 }
 return z;
}
SpbU32 _gz_spb_outer_publish(SpbU32 parent,SpbU32 expected){SpbValue*v=&_gz_spb_private.v;SpbU32 z=spb_begin(),be,serial;
 if(z)return z;if(expected!=v->parent_epoch)return spb_return(SPB_CHANGED);
 if(!spb_pointer(parent))return spb_return(SPB_INVALID);
 if(v->parent_live||v->backing_live||_gz_spb_private.scope_phase)return spb_return(SPB_BUSY);
 if(v->parent_epoch==0xffffffffu)return spb_return(SPB_EXHAUSTED);
 be=v->backing_epoch;serial=v->scope_serial;
 v->parent_epoch++;v->parent=parent;v->parent_live=1;v->parent_retire_reason=0;
 v->backing_epoch=be;v->scope_serial=serial;v->phase=SPB_PARENT;v->child=0;v->user=0;v->backing_bytes=0;v->direction=0;v->root=0;v->root_generation=0;v->root_bytes=0;v->last_event=SPB_OUTER_PUBLISH;v->retire_reason=0;v->initial_backing_bytes=0;v->backing_start=0;v->backing_end=0;v->root_bound=0;v->backing_live=0;
 return spb_return(0);
}
SpbU32 _gz_spb_boot_begin(SpbU32 parent,SpbU32 pe){SpbValue*v=&_gz_spb_private.v;SpbU32 z=spb_begin();
 if(z)return z;if(!v->parent_live||parent!=v->parent||pe!=v->parent_epoch)return spb_return(SPB_CHANGED);
 if(v->backing_live||_gz_spb_private.scope_phase)return spb_return(SPB_BUSY);
 if(v->scope_serial==0xffffffffu)return spb_return(SPB_EXHAUSTED);
 v->scope_serial++;v->last_event=SPB_BOOT_BEGIN;_gz_spb_private.scope_parent=parent;_gz_spb_private.scope_parent_epoch=pe;_gz_spb_private.scope_phase=1;return spb_return(0);
}
SpbU32 _gz_spb_factory_post(SpbU32 child,SpbU32 bytes,SpbU32 parent,SpbU32 direction){SpbValue*v=&_gz_spb_private.v;SpbU32 z=spb_begin(),end;
 if(z)return z;if(!_gz_spb_private.scope_phase)return spb_return(SPB_NO_CHANGE);
 if(_gz_spb_private.scope_phase!=1)return spb_return(SPB_BUSY);
 if(parent!=_gz_spb_private.scope_parent||parent!=v->parent)return spb_return(SPB_NO_CHANGE);
 if(direction!=1||!spb_pointer(child)||bytes<SPB_CHILD_BYTES||(bytes&3)||!spb_end(child,bytes,&end))return spb_return(SPB_INVALID);
 if(v->backing_epoch==0xffffffffu)return spb_return(SPB_EXHAUSTED);
 v->backing_epoch++;v->child=child;v->user=child;v->backing_bytes=bytes;v->direction=direction;v->root=0;v->root_generation=0;v->root_bytes=0;v->last_event=SPB_FACTORY_POST;v->retire_reason=0;v->initial_backing_bytes=bytes;v->backing_start=child;v->backing_end=end;v->root_bound=0;v->backing_live=1;v->phase=SPB_BORN;_gz_spb_private.scope_phase=2;return spb_return(0);
}
SpbU32 _gz_spb_boot_return(SpbU32 child){SpbValue*v=&_gz_spb_private.v;SpbU32 z=spb_begin(),phase;
 if(z)return z;phase=_gz_spb_private.scope_phase;if(!phase)return spb_return(SPB_CHANGED);
 spb_scope_clear();
 if(phase==1)return spb_return(child?SPB_CHANGED:SPB_NO_CHANGE);
 if(!child||child!=v->child){spb_retire(SPB_BIRTH_MISMATCH,1,0);return spb_return(SPB_CHANGED);}
 v->phase=SPB_RETURNED;v->last_event=SPB_BOOT_RETURN;return spb_return(0);
}
SpbU32 _gz_spb_root_bind(SpbU32 root,SpbU32 child){SpbValue*v=&_gz_spb_private.v;SpbU32 z=spb_begin(),end,meta;
 if(z)return z;
 if(!v->parent_live||!v->backing_live||_gz_spb_private.scope_phase||(v->phase!=SPB_RETURNED&&v->phase!=SPB_BOUND)||child!=v->child)return spb_return(SPB_CHANGED);
 if(_gz_srw_private.entered||_gz_srw_private.poison||_gz_srw_private.stamp_active||_gz_sro_entered||!sro_header()||_gz_sro_private.phase!=SRO_CONSTRUCTED||_gz_sro_private.root!=root||_gz_sro_private.heap!=child||_gz_srw_private.slot!=SPB_ROOT_SLOT||!_gz_srw_private.ctor_events)return spb_return(SPB_CHANGED);
 if(!spb_pointer(root)||!spb_end(child,SPB_CHILD_BYTES,&meta)||root<meta||!spb_end(root,SRO_CAPACITY,&end)||end>v->backing_end)return spb_return(SPB_INVALID);
 if(v->root_generation&&_gz_sro_private.generation<=v->root_generation)return spb_return(SPB_CHANGED);
 v->root=root;v->root_generation=_gz_sro_private.generation;v->root_bytes=SRO_CAPACITY;v->root_bound=1;v->phase=SPB_BOUND;v->last_event=SPB_ROOT_BIND;v->retire_reason=0;return spb_return(0);
}
SpbU32 _gz_spb_direct_free(SpbU32 parent,SpbU32 user,SpbU32 pe,SpbU32 be){SpbU32 z=spb_begin();if(z)return z;z=spb_selected(parent,user,pe,be);if(!z)z=spb_retire(SPB_DIRECT_FREE,1,0);return spb_return(z);}
SpbU32 _gz_spb_tail_shrink(SpbU32 parent,SpbU32 user,SpbU32 old,SpbU32 next,SpbU32 pe,SpbU32 be){SpbValue*v=&_gz_spb_private.v;SpbU32 z=spb_begin(),lo,hi;
 if(z)return z;z=spb_selected(parent,user,pe,be);if(z)return spb_return(z);
 if(old!=v->backing_bytes||next>old||!spb_end(user,old,&hi)||!spb_end(user,next,&lo))return spb_return(SPB_INVALID);
 if(old-next<16)return spb_return(SPB_NO_CHANGE);
 if(spb_overlap(lo,hi,v->child,SPB_CHILD_BYTES)||(v->root_bound&&spb_overlap(lo,hi,v->root,v->root_bytes)))return spb_return(spb_retire(SPB_TAIL_SHRINK,1,0));
 v->backing_bytes=next;v->backing_end=lo;v->last_event=SPB_TAIL_SHRINK;return spb_return(0);
}
SpbU32 _gz_spb_front_shrink(SpbU32 parent,SpbU32 user,SpbU32 next_user,SpbU32 old,SpbU32 next,SpbU32 pe,SpbU32 be){SpbValue*v=&_gz_spb_private.v;SpbU32 z=spb_begin(),end,expected;
 if(z)return z;z=spb_selected(parent,user,pe,be);if(z)return spb_return(z);
 if(old!=v->backing_bytes||next>old||!spb_end(user,old,&end)||!spb_end(user,old-next,&expected)||next_user!=expected)return spb_return(SPB_INVALID);
 if(next==old)return spb_return(SPB_NO_CHANGE);
 /* Logical canonical USER movement retires the child object even when native
  * small-prefix branch keeps storage as padding rather than a free block. */
 return spb_return(spb_retire(SPB_FRONT_SHRINK,1,0));
}
SpbU32 _gz_spb_child_range(SpbU32 child,SpbU32 lo,SpbU32 hi,SpbU32 pe,SpbU32 be){SpbValue*v=&_gz_spb_private.v;SpbU32 z=spb_begin(),meta;
 if(z)return z;if(pe!=v->parent_epoch||be!=v->backing_epoch)return spb_return(SPB_CHANGED);
 if(!v->backing_live||child!=v->child)return spb_return(SPB_NO_CHANGE);
 if(!lo||!hi||lo>=hi||!spb_end(v->child,SPB_CHILD_BYTES,&meta)||lo<meta||hi>v->backing_end)return spb_return(SPB_INVALID);
 if(v->root_bound&&spb_overlap(lo,hi,v->root,v->root_bytes))return spb_return(spb_retire(SPB_CHILD_RANGE,0,0));
 return spb_return(SPB_NO_CHANGE);
}
SpbU32 _gz_spb_scope_retire(SpbU32 event,SpbU32 heap,SpbU32 pe,SpbU32 be){SpbValue*v=&_gz_spb_private.v;SpbU32 z=spb_begin(),parent;
 if(z)return z;if(event<SPB_CHILD_FULL||event>SPB_PARENT_DESTROY)return spb_return(SPB_INVALID);
 if(pe!=v->parent_epoch||be!=v->backing_epoch)return spb_return(SPB_CHANGED);
 parent=event==SPB_PARENT_FULL||event==SPB_PARENT_DESTROY;
 if((parent?heap!=v->parent||!v->parent_live:heap!=v->child||!v->backing_live))return spb_return(SPB_NO_CHANGE);
 if(event==SPB_CHILD_FULL){if(!v->root_bound)return spb_return(SPB_NO_CHANGE);z=spb_retire(event,0,0);}
 else if(event==SPB_PARENT_FULL&&!v->backing_live){spb_scope_clear();z=SPB_NO_CHANGE;}
 else z=spb_retire(event,1,event==SPB_PARENT_DESTROY);
 return spb_return(z);
}
static SpbU32 spb_output(const SpbValue*out){SpbU32 p=(SpbU32)out,end;
 return p&&!(p&3)&&spb_end(p,SPB_BYTES,&end)&&spb_apart(p,SPB_BYTES,(SpbU32)&_gz_spb_private,sizeof(_gz_spb_private))&&spb_apart(p,SPB_BYTES,(SpbU32)&_gz_sro_private,sizeof(_gz_sro_private))&&spb_apart(p,SPB_BYTES,(SpbU32)&_gz_sro_entered,sizeof(_gz_sro_entered))&&spb_apart(p,SPB_BYTES,(SpbU32)&_gz_srw_private,sizeof(_gz_srw_private));
}
static __attribute__((noinline,noclone)) SroU32 _gz_spo_sro_query_base(SroU32 root,SroU32 generation,SroU32 phase,SroValue*out){
 if(!spb_apart((SpbU32)out,SRO_BYTES,(SpbU32)&_gz_spb_private,sizeof(_gz_spb_private)))return SRO_INVALID;
 return _gz_spb_core_query(root,generation,phase,out);
}
static __attribute__((noinline,noclone)) SroU32 _gz_spo_srw_query_base(SroU32 root,SroU32 generation,SroU32 phase,SroValue*out){
 if(!spb_apart((SpbU32)out,SRO_BYTES,(SpbU32)&_gz_spb_private,sizeof(_gz_spb_private)))return SRO_INVALID;
 return _gz_spb_wrapper_query(root,generation,phase,out);
}
static __attribute__((noinline,noclone)) SpbU32 _gz_spo_spb_query_base(SpbU32 root,SpbU32 gen,SpbU32 pe,SpbU32 be,SpbValue*out){SpbValue*v=&_gz_spb_private.v;SroValue s;SpbU32 z=spb_begin(),i;SpbU32*dst=(SpbU32*)out;const SpbU32*src=(const SpbU32*)v;
 if(z)return z;if(!spb_output(out))return spb_return(SPB_INVALID);
 if(_gz_spb_private.scope_phase||!v->parent_live||!v->backing_live||!v->root_bound||v->phase!=SPB_BOUND||root!=v->root||gen!=v->root_generation||pe!=v->parent_epoch||be!=v->backing_epoch)return spb_return(SPB_CHANGED);
 z=_gz_srw_query(root,gen,SRO_BOOT_READY,&s);if(z)return spb_return(z);
 if(s.heap!=v->child||s.capacity!=v->root_bytes)return spb_return(SPB_CHANGED);
 for(i=0;i<SPB_BYTES/4;i++)dst[i]=src[i];return spb_return(0);
}
SpbU32 _gz_spb_native_admit(void){return SPB_DENIED;}
