/* Passive fixed-event observations only. Native pointer integers are never followed. */
#include "observation.c"
typedef struct {
 SroU32 entered,poison,error,slot,ctor_events,boot_events,dtor_events,ignored_dtors;
 SroU32 preboot_events,stamp_active,stamp_root,stamp_heap,stamp_generation,stamp_slot;
} SrwPrivate;
typedef char SrwSize[(sizeof(SrwPrivate)==56)?1:-1];
static SrwPrivate _gz_srw_private;
static SroU32 srw_fail(SroU32 z){
 _gz_srw_private.poison=1;
 if(!_gz_srw_private.error)_gz_srw_private.error=z?z:SRO_DENIED;
 return z?z:SRO_DENIED;
}
static SroU32 srw_begin(void){
 if(_gz_srw_private.entered)return srw_fail(SRO_BUSY);
 _gz_srw_private.entered=1;
 if(_gz_sro_entered){_gz_srw_private.entered=0;return srw_fail(SRO_BUSY);}
 if(!sro_header()){_gz_srw_private.entered=0;return srw_fail(SRO_CORRUPT);}
 return 0;
}
static SroU32 srw_end(SroU32 z){_gz_srw_private.entered=0;return z;}
SroU32 _gz_srw_ctor_note(SroU32 root,SroU32 heap,SroU32 slot){SroU32 z=srw_begin();
 if(z)return z;if(_gz_srw_private.poison)return srw_end(SRO_DENIED);
 if(!slot||(slot&3))return srw_end(srw_fail(SRO_INVALID));
 z=_gz_sro_observe(SRO_CTOR_PUBLISHED,root,heap,_gz_sro_private.generation);
 if(z)return srw_end(srw_fail(z));
 /* Do not replace an active pre-DCC stamp. A new construct may have the same
  * address, but its new generation must fail the retained post-DCC comparison. */
 _gz_srw_private.slot=slot;_gz_srw_private.ctor_events++;return srw_end(0);
}
SroU32 _gz_srw_preboot_note(SroU32 root){SroU32 z=srw_begin();
 if(z)return z;if(_gz_srw_private.poison)return srw_end(SRO_DENIED);
 if(_gz_srw_private.stamp_active)return srw_end(srw_fail(SRO_BUSY));
 if(!root||root!=_gz_sro_private.root||_gz_sro_private.phase!=SRO_CONSTRUCTED||
    !_gz_srw_private.slot||(_gz_srw_private.slot&3))return srw_end(srw_fail(SRO_CHANGED));
 _gz_srw_private.stamp_root=root;
 _gz_srw_private.stamp_heap=_gz_sro_private.heap;
 _gz_srw_private.stamp_generation=_gz_sro_private.generation;
 _gz_srw_private.stamp_slot=_gz_srw_private.slot;
 _gz_srw_private.preboot_events++;
 /* Publish pending stamp last. No native read, callback or output is involved. */
 _gz_srw_private.stamp_active=1;
 return srw_end(0);
}
SroU32 _gz_srw_boot_note(SroU32 root){
 SroU32 z=srw_begin(),old_root,old_heap,old_generation,old_slot;
 if(z)return z;if(_gz_srw_private.poison)return srw_end(SRO_DENIED);
 if(_gz_srw_private.stamp_active!=1)return srw_end(srw_fail(SRO_CHANGED));
 old_root=_gz_srw_private.stamp_root;old_heap=_gz_srw_private.stamp_heap;
 old_generation=_gz_srw_private.stamp_generation;old_slot=_gz_srw_private.stamp_slot;
 /* Consume once before any validation failure. Never resample a generation
  * from the possibly replaced current root to authenticate this completion. */
 _gz_srw_private.stamp_active=0;
 if(!old_generation||!old_slot||root!=old_root||
    _gz_sro_private.phase!=SRO_CONSTRUCTED||_gz_sro_private.root!=old_root||
    _gz_sro_private.heap!=old_heap||_gz_sro_private.generation!=old_generation||
    _gz_srw_private.slot!=old_slot)return srw_end(srw_fail(SRO_CHANGED));
 z=_gz_sro_observe(SRO_BOOT_COMPLETED,old_root,old_heap,old_generation);
 if(z!=SRO_OK)return srw_end(srw_fail(z));
 _gz_srw_private.boot_events++;return srw_end(0);
}
SroU32 _gz_srw_dtor_note(SroU32 root){SroU32 z=srw_begin();
 if(z)return z;
 if(root&&root!=_gz_sro_private.root){_gz_srw_private.ignored_dtors++;return srw_end(SRO_NO_CHANGE);}
 if(_gz_srw_private.poison)return srw_end(SRO_DENIED);
 if(!root)return srw_end(srw_fail(SRO_INVALID));
 z=_gz_sro_observe(SRO_ROOT_DTOR_BEGIN,root,_gz_sro_private.heap,_gz_sro_private.generation);
 if(z!=SRO_OK&&z!=SRO_NO_CHANGE)return srw_end(srw_fail(z));
 /* Keep any old stamp intact: retirement/new construction cannot bless it. */
 _gz_srw_private.dtor_events++;return srw_end(z);
}
SroU32 _gz_srw_query(SroU32 root,SroU32 generation,SroU32 phase,SroValue*out){SroU32 z,p=(SroU32)out;
 if(_gz_srw_private.entered)return SRO_BUSY;
 if(_gz_srw_private.poison)return SRO_DENIED;
 if(_gz_srw_private.stamp_active)return SRO_BUSY;
 if(!sro_apart(p,SRO_BYTES,(SroU32)&_gz_srw_private,sizeof(_gz_srw_private)))return SRO_INVALID;
 if(!sro_header())return srw_fail(SRO_CORRUPT);
 /* The exported core is an unauthenticated value-event API. A direct core
  * BOOT event cannot substitute for this fixed pre/post path's private receipt. */
 if(_gz_sro_private.phase==SRO_BOOT_READY&&
    (!_gz_srw_private.boot_events||_gz_srw_private.stamp_root!=_gz_sro_private.root||
     _gz_srw_private.stamp_heap!=_gz_sro_private.heap||
     _gz_srw_private.stamp_generation!=_gz_sro_private.generation||
     _gz_srw_private.stamp_slot!=_gz_srw_private.slot))return srw_fail(SRO_CHANGED);
 _gz_srw_private.entered=1;z=_gz_sro_query(root,generation,phase,out);_gz_srw_private.entered=0;
 return z;
}
SroU32 _gz_srw_native_admit(void){return SRO_DENIED;}
