#include "archive_layout.h"
#include "owner.h"
/* Logical backings never enter the historical table. Only the one currently
 * authenticated RAW materialization appears there. Borrowers retain their
 * original exact raw pin receipt, source bytes and generation checks. */
typedef unsigned int OsaU32;
typedef struct {OsaU32 active,slot,serial,allocation,identity,generation,bytes;} OsaPin;
typedef struct {OsaPin pin;} OsaState;
OsaU32 _gz_osa_plan_snapshot[120];
static OsaState _gz_osa_state;
static SoPresentation _gz_osa_initial_presentation;
static SoInfo _gz_osa_info_copy;
static OsaU32 _gz_osa_descriptors[10][5];
static OsaU32 osa_span(OsaU32 p,OsaU32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static OsaU32 osa_apart(OsaU32 p,OsaU32 n,OsaU32 q,OsaU32 z){return n<=0xffffffffu-p&&z<=0xffffffffu-q&&(p+n<=q||q+z<=p);}
static void osa_zero(void*p,OsaU32 n){OsaU32*i=p;while(n){*i++=0;n-=4;}}
/* Fixed adapter metadata set: no READ, body hash or foreign callback. */
OsaU32 _gz_osa_adapter_private_apart(OsaU32 p,OsaU32 n){return osa_span(p,n)&&
 osa_apart(p,n,(OsaU32)&swc,sizeof(swc))&&
 osa_apart(p,n,(OsaU32)&so_intent,sizeof(so_intent))&&
 osa_apart(p,n,(OsaU32)&_gz_sfm_memo,sizeof(_gz_sfm_memo))&&
 osa_apart(p,n,(OsaU32)&_gz_osa_state,sizeof(_gz_osa_state))&&
 osa_apart(p,n,(OsaU32)&_gz_osa_initial_presentation,sizeof(_gz_osa_initial_presentation))&&
 osa_apart(p,n,(OsaU32)&_gz_osa_info_copy,sizeof(_gz_osa_info_copy))&&
 osa_apart(p,n,(OsaU32)_gz_osa_descriptors,sizeof(_gz_osa_descriptors));}
static OsaU32 osa_view_current(void){const OsaPin*p=&_gz_osa_state.pin;OsaU32 i,pin=so_pin(),raw;
 if(!pin){if(p->active)return 0;for(i=0;i<10;i++)if(_gz_slots_ptrs[i]||_gz_slot_origin_sidecars[2*i])return 0;return 1;}
 if(so_validate(pin)||p->active!=1||p->serial!=pin||p->slot!=so_lease_slot(pin)||
  p->allocation!=so_source(pin)||p->identity!=so_identity(pin)||p->generation!=so_generation(pin)||p->bytes!=so_bytes(pin))return 0;
 raw=p->allocation;
 for(i=0;i<10;i++)if(_gz_slots_ptrs[i]!=(i==p->slot?raw:0)||
  _gz_slot_origin_sidecars[2*i]!=(i==p->slot?raw:0))return 0;
 return _gz_slot_origin_sidecars[2*p->slot+1]==so_origin(pin);
}
OsaU32 _gz_osa_begin(OsaU32 slot,OsaU32 frame){
 if(!osa_view_current())return 28;
 osa_zero(&_gz_osa_initial_presentation,sizeof(_gz_osa_initial_presentation));
 return so_begin(slot,frame,(OsaU32)&_gz_osa_initial_presentation);
}
OsaU32 _gz_osa_prepare(OsaU32 slot,OsaU32 frame){return osa_view_current()?so_prepare(slot,frame):28;}
OsaU32 _gz_osa_destination(OsaU32 slot,OsaU32 frame){return osa_view_current()?so_destination(slot,frame):0;}
OsaU32 _gz_osa_publish(OsaU32 slot,OsaU32 p,OsaU32 n,OsaU32 frame){
 if(!osa_view_current()||!p||p!=so_destination(slot,frame)||n!=so_writer_bytes(slot,frame)||n<256||
  so_present_raw(slot,frame,p,n))return 28;
 return so_publish(slot,frame);
}
OsaU32 _gz_osa_finish(OsaU32 status,OsaU32 frame){return so_finish(status,frame);}
OsaU32 _gz_osa_acquire(OsaU32 slot){OsaU32 z,pin,raw;OsaPin*p=&_gz_osa_state.pin;
 if(!osa_view_current())return 28;
 z=so_acquire(slot);if(z)return z;
 pin=so_pin();raw=so_source(pin);
 p->slot=slot;p->serial=pin;p->allocation=raw;p->identity=so_identity(pin);p->generation=so_generation(pin);p->bytes=so_bytes(pin);
 _gz_slot_origin_sidecars[2*slot+1]=so_origin(pin);_gz_slot_origin_sidecars[2*slot]=raw;
 _gz_slots_ptrs[slot]=raw;__asm__ volatile("sync":::"memory");p->active=1;return 0;
}
OsaU32 _gz_osa_validate(OsaU32 pin){return !pin||pin!=so_pin()||so_validate(pin)||!osa_view_current()?28:0;}
OsaU32 _gz_osa_pin_id(void){return so_pin();}
OsaU32 _gz_osa_source(OsaU32 pin){return _gz_osa_validate(pin)?0:so_source(pin);}
OsaU32 _gz_osa_bytes(OsaU32 pin){return _gz_osa_validate(pin)?0:so_bytes(pin);}
OsaU32 _gz_osa_pin_identity(OsaU32 pin){return _gz_osa_validate(pin)?0:so_identity(pin);}
OsaU32 _gz_osa_pin_generation(OsaU32 pin){return _gz_osa_validate(pin)?0:so_generation(pin);}
OsaU32 _gz_osa_pin_hash(OsaU32 pin){return _gz_osa_validate(pin)?0:so_raw_hash(pin);}
OsaU32 _gz_osa_capacity_for(OsaU32 source){OsaU32 pin=so_pin();return source&&!_gz_osa_validate(pin)&&source==so_source(pin)?so_capacity(pin):0;}
static void osa_release_owned_leaf(void){OsaU32 i;
 /* Final leaf: no body/hash, foreign callback, codec or SDK free. */
 for(i=0;i<10;i++){_gz_slots_ptrs[i]=0;_gz_slot_origin_sidecars[2*i]=0;_gz_slot_origin_sidecars[2*i+1]=0xffffffffu;}
 _gz_osa_state.pin.active=0;so_release_leaf();
}
void _gz_osa_release_leaf(void){
 if(swc_pending()){swc_release_refused();return;}
 osa_release_owned_leaf();
}
OsaU32 _gz_osa_plan_bounds(void){return osa_view_current()?so_plan_bounds():28;}
OsaU32 _gz_osa_private_apart(OsaU32 p,OsaU32 n){return _gz_osa_adapter_private_apart(p,n)&&so_physical_apart(p,n)&&osa_view_current();}
OsaU32 _gz_osa_intent_match(OsaU32 slot,OsaU32 id,OsaU32 gen,OsaU32 used,OsaU32 hash){return osa_view_current()&&so_intent_match(slot,id,gen,used,hash);}
OsaU32 _gz_osa_raw_match(OsaU32 pin,OsaU32 slot,OsaU32 raw,OsaU32 used){return !_gz_osa_validate(pin)&&so_raw_match(pin,slot,raw,used);}
/* No source pin, decode, allocator or saved-body read while menuing. */
OsaU32 _gz_osa_occupied(OsaU32 slot){return slot<10&&!so_info(slot,(OsaU32)&_gz_osa_info_copy)&&_gz_osa_info_copy.mode!=0;}
OsaU32 _gz_osa_origin(OsaU32 slot){OsaU32 origin;
 if(!_gz_osa_occupied(slot))return 0xffffffffu;origin=_gz_osa_info_copy.origin;
 return origin<=802&&origin!=679&&origin!=680?origin:0xffffffffu;
}
OsaU32 _gz_osa_descriptor(OsaU32 slot){OsaU32*d;const SoPresentation*m;
 if(!_gz_osa_occupied(slot))return 0;d=_gz_osa_descriptors[slot];m=&_gz_osa_info_copy.presentation;
 d[0]=d[4]=0;d[1]=m->stage0;d[2]=m->stage1;d[3]=(m->room<<8)|m->layer;return(OsaU32)d;
}
