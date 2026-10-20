#include "origin.h"
static void srh_boot_bind(void);
SroU32 _gz_srw_query(SroU32,SroU32,SroU32,SroValue*);
#include "parent_bound.inc"
typedef struct {
 SpoU32 magic,version,bytes,entered,poison,error,gate,serial,phase,boot_sp;
 SpoU32 parent,parent_epoch,scope_serial,child,backing_bytes,backing_epoch;
 SpoU32 root,root_generation,slot,prior_sro_generation,factory_sp,ctor_sp,completed_serial,ignored_factory;
 SpoU32 outer_events,pre_events,factory_events,return_events,ctor_events,initpre_events,initpost_events,dtor_events;
} SpoPrivate;
/* ASM alone arms gate24 before fixed C note. No setter/raw value API exposes it.
 * Hidden symbol linkage is needed by the separate ASM object, not a mapping or
 * security boundary. Serialized fixed-site/control-flow is still a premise. */
__attribute__((visibility("hidden"))) SpoPrivate _gz_spo_private={0x53504f31u,1,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
typedef char SpoSize[(sizeof(SpoPrivate)==128)?1:-1];
static void spo_count(SpoU32*p){if(*p!=0xffffffffu)(*p)++;}
static SpoU32 spo_fail(SpoU32 z){_gz_spo_private.poison=1;if(!_gz_spo_private.error)_gz_spo_private.error=z?z:SPB_DENIED;return z?z:SPB_DENIED;}
static SpoU32 spo_header(void){const SpoPrivate*s=&_gz_spo_private;
 if(s->magic!=0x53504f31u||s->version!=1||s->bytes!=128||s->entered>1||s->poison>1||s->gate>8||s->phase>SPO_RETIRED)return 0;
 if(!s->poison&&s->error)return 0;
 if(s->phase>=SPO_PENDING&&s->phase<=SPO_READY){
  if(!s->serial||!spb_pointer(s->boot_sp)||s->boot_sp>0xfffffff7u||!spb_pointer(s->parent)||!s->parent_epoch||!s->scope_serial)return 0;
  if(s->phase>=SPO_BORN&&(!spb_pointer(s->child)||s->backing_bytes<SPB_CHILD_BYTES||!s->backing_epoch||s->boot_sp<288||s->factory_sp!=s->boot_sp-288))return 0;
  if(s->phase>=SPO_CONSTRUCTED&&(!spb_pointer(s->root)||!s->root_generation||s->slot!=SPB_ROOT_SLOT||s->boot_sp<24||s->ctor_sp!=s->boot_sp-24))return 0;
  if(s->phase==SPO_READY&&s->completed_serial!=s->serial)return 0;
 }
 return 1;
}
static SpoU32 spo_start(SpoU32 site){
 if(_gz_spo_private.entered){_gz_spo_private.gate=0;return spo_fail(SPB_BUSY);}
 if(_gz_spo_private.gate!=site)return SPB_DENIED;
 if(!spo_header()){_gz_spo_private.gate=0;return spo_fail(SPB_CORRUPT);}
 _gz_spo_private.gate=0;
 if(_gz_spo_private.poison)return SPB_DENIED;
 _gz_spo_private.entered=1;return 0;
}
static SpoU32 spo_done(SpoU32 z){_gz_spo_private.entered=0;if(_gz_spo_private.poison)return _gz_spo_private.error;return z;}
SpoU32 _gz_spo_reentry_note(void){return spo_fail(SPB_BUSY);}
/* The ONLY new native-frame read. It occurs after private fixed-entry scope
 * and exact current originalSP relationship. bootSP+4 is original caller ABI
 * linkage, not a followed saved stack pointer or arbitrary object address.
 * Actual mapped current stack/nonyield/thread scope remain external premises. */
extern unsigned char _gz_hg_solid_child_factory_return[];
__attribute__((noinline,noclone)) static SpoU32 spo_frame(SpoU32 current,SpoU32 frame,SpoU32 expected){SpoU32 b=_gz_spo_private.boot_sp,a;
 if(!spb_pointer(b)||b<frame||b>0xfffffff7u||current!=b-frame||!spb_pointer(current))return 0;
 if(frame==288){
  /* Current installed wrapper saves boot's LR at B-256+172. The original
   * factory then saves the wrapper body's return at B-288+36. Both exact
   * linkage words are required; an old unwrapped or nested frame refuses. */
  if(*(volatile SpoU32*)(b-84)!=expected)return 0;
  return *(volatile SpoU32*)(b-252)==(SpoU32)_gz_hg_solid_child_factory_return;
 }
 if(frame!=24)return 0;
 a=b+4;return *(volatile SpoU32*)a==expected;
}
static SpoU32 spo_birth_current(SpoU32 phase){const SpoPrivate*s=&_gz_spo_private;const SpbValue*v=&_gz_spb_private.v;
 if(!spb_header()||_gz_spb_private.entered||_gz_spb_private.poison||!v->parent_live||v->parent!=s->parent||v->parent_epoch!=s->parent_epoch||v->scope_serial!=s->scope_serial)return 0;
 if(phase==1)return _gz_spb_private.scope_phase==1&&!v->backing_live&&_gz_spb_private.scope_parent==s->parent&&_gz_spb_private.scope_parent_epoch==s->parent_epoch;
 if(!v->backing_live||v->child!=s->child||v->backing_epoch!=s->backing_epoch||v->backing_bytes!=s->backing_bytes)return 0;
 return phase==2?(_gz_spb_private.scope_phase==2&&v->phase==SPB_BORN):(!_gz_spb_private.scope_phase&&v->phase==SPB_RETURNED);
}
static SpoU32 spo_tuple(void){const SpoPrivate*s=&_gz_spo_private;const SpbValue*v=&_gz_spb_private.v;
 return spb_header()&&sro_header()&&!_gz_spb_private.poison&&v->parent_live&&v->backing_live&&v->root_bound&&
 v->parent==s->parent&&v->parent_epoch==s->parent_epoch&&v->scope_serial==s->scope_serial&&v->child==s->child&&v->backing_epoch==s->backing_epoch&&v->root==s->root&&v->root_generation==s->root_generation&&
 _gz_sro_private.root==s->root&&_gz_sro_private.heap==s->child&&_gz_sro_private.generation==s->root_generation;
}
SpoU32 _gz_spo_outer_note(SpoU32 parent){SpoU32 z=spo_start(SPO_OUTER);if(z)return z;
 z=_gz_spb_outer_publish(parent,_gz_spb_private.v.parent_epoch);if(z)return spo_done(spo_fail(z));
 _gz_spo_private.phase=SPO_EMPTY;_gz_spo_private.boot_sp=0;_gz_spo_private.parent=parent;_gz_spo_private.parent_epoch=_gz_spb_private.v.parent_epoch;
 _gz_spo_private.scope_serial=0;_gz_spo_private.child=0;_gz_spo_private.backing_bytes=0;_gz_spo_private.backing_epoch=0;_gz_spo_private.root=0;_gz_spo_private.root_generation=0;_gz_spo_private.slot=0;_gz_spo_private.factory_sp=0;_gz_spo_private.ctor_sp=0;_gz_spo_private.completed_serial=0;
 spo_count(&_gz_spo_private.outer_events);return spo_done(0);
}
SpoU32 _gz_spo_pre_note(SpoU32 parent,SpoU32 original_sp){SpoU32 z=spo_start(SPO_PRE);if(z)return z;
 if(_gz_spo_private.phase!=SPO_EMPTY&&_gz_spo_private.phase!=SPO_RETIRED)return spo_done(spo_fail(SPB_BUSY));
 if(!spb_pointer(original_sp)||original_sp>0xfffffff7u||parent!=_gz_spo_private.parent||_gz_spo_private.parent_epoch!=_gz_spb_private.v.parent_epoch)return spo_done(spo_fail(SPB_CHANGED));
 if(_gz_spo_private.serial==0xffffffffu)return spo_done(spo_fail(SPB_EXHAUSTED));
 z=_gz_spb_boot_begin(parent,_gz_spo_private.parent_epoch);if(z)return spo_done(spo_fail(z));
 _gz_spo_private.serial++;_gz_spo_private.boot_sp=original_sp;_gz_spo_private.scope_serial=_gz_spb_private.v.scope_serial;_gz_spo_private.prior_sro_generation=_gz_sro_private.generation;
 _gz_spo_private.child=0;_gz_spo_private.backing_bytes=0;_gz_spo_private.backing_epoch=0;_gz_spo_private.root=0;_gz_spo_private.root_generation=0;_gz_spo_private.slot=0;_gz_spo_private.factory_sp=0;_gz_spo_private.ctor_sp=0;_gz_spo_private.completed_serial=0;
 _gz_spo_private.phase=SPO_PENDING;spo_count(&_gz_spo_private.pre_events);return spo_done(0);
}
SpoU32 _gz_spo_factory_note(SpoU32 child,SpoU32 bytes,SpoU32 parent,SpoU32 direction,SpoU32 original_sp){SpoU32 z=spo_start(SPO_FACTORY);if(z)return z;
 if(_gz_spo_private.phase!=SPO_PENDING||parent!=_gz_spo_private.parent||direction!=1||!spo_birth_current(1)||!spo_frame(original_sp,288,0x0203f458u)){spo_count(&_gz_spo_private.ignored_factory);return spo_done(SPB_NO_CHANGE);}
 z=_gz_spb_factory_post(child,bytes,parent,direction);if(z)return spo_done(spo_fail(z));
 _gz_spo_private.child=child;_gz_spo_private.backing_bytes=bytes;_gz_spo_private.backing_epoch=_gz_spb_private.v.backing_epoch;_gz_spo_private.factory_sp=original_sp;_gz_spo_private.phase=SPO_BORN;
 spo_count(&_gz_spo_private.factory_events);return spo_done(0);
}
SpoU32 _gz_spo_return_note(SpoU32 child,SpoU32 original_sp,SpoU32 original_lr){SpoU32 z=spo_start(SPO_RETURN);if(z)return z;
 if(original_sp!=_gz_spo_private.boot_sp||original_lr!=0x0203f458u)return spo_done(spo_fail(SPB_CHANGED));
 if(_gz_spo_private.phase==SPO_PENDING&&!child){z=_gz_spb_boot_return(0);_gz_spo_private.phase=SPO_EMPTY;return spo_done(z);}
 if(_gz_spo_private.phase!=SPO_BORN||child!=_gz_spo_private.child||!spo_birth_current(2))return spo_done(spo_fail(SPB_CHANGED));
 z=_gz_spb_boot_return(child);if(z)return spo_done(spo_fail(z));
 _gz_spo_private.phase=SPO_RETURNED;spo_count(&_gz_spo_private.return_events);return spo_done(0);
}
SpoU32 _gz_spo_ctor_note(SpoU32 root,SpoU32 child,SpoU32 slot,SpoU32 original_sp){SpoU32 z=spo_start(SPO_CTOR);if(z)return z;
 if(_gz_spo_private.phase!=SPO_RETURNED||child!=_gz_spo_private.child||slot!=SPB_ROOT_SLOT||!spo_birth_current(3)||!spo_frame(original_sp,24,0x0203f460u))return spo_done(spo_fail(SPB_CHANGED));
 z=_gz_srw_ctor_note(root,child,slot);if(z)return spo_done(spo_fail(z));
 if(_gz_sro_private.generation<=_gz_spo_private.prior_sro_generation)return spo_done(spo_fail(SPB_CHANGED));
 z=_gz_spb_root_bind(root,child);if(z)return spo_done(spo_fail(z));
 _gz_spo_private.root=root;_gz_spo_private.root_generation=_gz_sro_private.generation;_gz_spo_private.slot=slot;_gz_spo_private.ctor_sp=original_sp;_gz_spo_private.phase=SPO_CONSTRUCTED;
 spo_count(&_gz_spo_private.ctor_events);return spo_done(0);
}
SpoU32 _gz_spo_initpre_note(SpoU32 root,SpoU32 original_sp,SpoU32 original_lr){SpoU32 z=spo_start(SPO_INITPRE);if(z)return z;
 if(_gz_spo_private.phase!=SPO_CONSTRUCTED||original_sp!=_gz_spo_private.boot_sp||original_lr!=0x0203f478u||root!=_gz_spo_private.root||!spo_tuple())return spo_done(spo_fail(SPB_CHANGED));
 z=_gz_srw_preboot_note(root);if(z)return spo_done(spo_fail(z));
 _gz_spo_private.phase=SPO_INITIALIZING;spo_count(&_gz_spo_private.initpre_events);return spo_done(0);
}
SpoU32 _gz_spo_initpost_note(SpoU32 root,SpoU32 original_sp,SpoU32 original_lr){SpoU32 z=spo_start(SPO_INITPOST);if(z)return z;
 if(_gz_spo_private.phase!=SPO_INITIALIZING||original_sp!=_gz_spo_private.boot_sp||original_lr!=0x0203f480u||root!=_gz_spo_private.root||!spo_tuple())return spo_done(spo_fail(SPB_CHANGED));
 z=_gz_srw_boot_note(root);if(z)return spo_done(spo_fail(z));
 _gz_spo_private.completed_serial=_gz_spo_private.serial;_gz_spo_private.phase=SPO_READY;spo_count(&_gz_spo_private.initpost_events);srh_boot_bind();return spo_done(0);
}
static SpoU32 spo_dtor_unheld(SpoU32 root){SpoU32 z=spo_start(SPO_DTOR);if(z)return z;if(!root)return spo_done(SPB_NO_CHANGE);z=_gz_srw_dtor_note(root);
 if(z!=0&&z!=SPB_NO_CHANGE)return spo_done(spo_fail(z));
 if(root&&root==_gz_spo_private.root)_gz_spo_private.phase=SPO_RETIRED;
 spo_count(&_gz_spo_private.dtor_events);return spo_done(z);
}
/* Exclude the entire private union explicitly. Link order or adjacent DATA
 * must never make output validity depend on another record's placement. */
static SpoU32 spo_apart(const void*out,SpoU32 n){SpoU32 p=(SpoU32)out;
 return spb_apart(p,n,(SpoU32)&_gz_spo_private,sizeof(_gz_spo_private))&&
 spb_apart(p,n,(SpoU32)&_gz_spb_private,sizeof(_gz_spb_private))&&
 spb_apart(p,n,(SpoU32)&_gz_sro_private,sizeof(_gz_sro_private))&&
 spb_apart(p,n,(SpoU32)&_gz_srw_private,sizeof(_gz_srw_private))&&
 spb_apart(p,n,(SpoU32)&_gz_sro_entered,sizeof(_gz_sro_entered));}
SroU32 _gz_sro_query(SroU32 root,SroU32 gen,SroU32 phase,SroValue*out){if(!spo_apart(out,64))return SPB_INVALID;return _gz_spo_sro_query_base(root,gen,phase,out);}
SroU32 _gz_srw_query(SroU32 root,SroU32 gen,SroU32 phase,SroValue*out){if(!spo_apart(out,64))return SPB_INVALID;return _gz_spo_srw_query_base(root,gen,phase,out);}
SpbU32 _gz_spb_query(SpbU32 root,SpbU32 gen,SpbU32 pe,SpbU32 be,SpbValue*out){if(!spo_apart(out,128))return SPB_INVALID;return _gz_spo_spb_query_base(root,gen,pe,be,out);}
SpoU32 _gz_spo_query(SpoU32 root,SpoU32 gen,SpbValue*out){SpbValue value;SpoU32 z,i;SpoU32*dst=(SpoU32*)out;const SpoU32*src=(const SpoU32*)&value;
 if(_gz_spo_private.entered||_gz_spo_private.gate)return SPB_BUSY;
 if(_gz_spo_private.poison)return SPB_DENIED;
 if(!spo_header())return SPB_CORRUPT;
 if(!spo_apart(out,128)||!spb_output(out))return SPB_INVALID;
 if(_gz_spo_private.phase!=SPO_READY||root!=_gz_spo_private.root||gen!=_gz_spo_private.root_generation||!spo_tuple())return SPB_CHANGED;
 _gz_spo_private.entered=1;z=_gz_spb_query(root,gen,_gz_spo_private.parent_epoch,_gz_spo_private.backing_epoch,&value);
 if(!z)for(i=0;i<128/4;i++)dst[i]=src[i];_gz_spo_private.entered=0;return z;
}
SpoU32 _gz_spo_native_admit(void){return SPB_DENIED;}

#include "root_access.c"
