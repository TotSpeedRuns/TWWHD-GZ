#include "observation.h"
static SroValue _gz_sro_private={SRO_MAGIC,SRO_VERSION,SRO_BYTES,SRO_EMPTY,0,0,0,0,0,0,0,0,{0,0,0,0}};
static SroU32 _gz_sro_entered;
static SroU32 sro_root_shape(SroU32 root,SroU32 heap){
 return root&&heap&&!(root&3)&&!(heap&3)&&root<=0xffffffffu-SRO_CAPACITY;
}
static SroU32 sro_header(void){
 const SroValue*s=&_gz_sro_private;SroU32 i;
 if(s->magic!=SRO_MAGIC||s->version!=SRO_VERSION||s->bytes!=SRO_BYTES||s->phase>SRO_RETIRED)return 0;
 for(i=0;i<4;i++)if(s->reserved[i])return 0;
 if(s->phase==SRO_EMPTY)return !s->generation&&!s->root&&!s->heap&&!s->capacity&&!s->save&&!s->save_bytes&&!s->last_event&&!s->retire_reason;
 if(!s->generation||!sro_root_shape(s->root,s->heap)||s->capacity!=SRO_CAPACITY||s->save!=s->root+SRO_SAVE_OFFSET||s->save_bytes!=SRO_SAVE_BYTES)return 0;
 if(s->phase==SRO_CONSTRUCTED)return s->last_event==SRO_CTOR_PUBLISHED&&!s->retire_reason;
 if(s->phase==SRO_BOOT_READY)return s->last_event==SRO_BOOT_COMPLETED&&!s->retire_reason;
 return (s->retire_reason==SRO_ROOT_DTOR_BEGIN||s->retire_reason==SRO_DIRECT_FREE_BEGIN)&&s->last_event==s->retire_reason;
}
static SroU32 sro_apart(SroU32 p,SroU32 n,SroU32 q,SroU32 m){
 return n&&m&&p+n>=p&&q+m>=q&&(p+n<=q||q+m<=p);
}
static SroU32 sro_output(const SroValue*out){SroU32 p=(SroU32)out;
 return p&&!(p&3)&&p<=0xffffffffu-SRO_BYTES&&
 sro_apart(p,SRO_BYTES,(SroU32)&_gz_sro_private,sizeof(_gz_sro_private))&&
 sro_apart(p,SRO_BYTES,(SroU32)&_gz_sro_entered,sizeof(_gz_sro_entered));
}
static SroU32 sro_event(SroU32 event,SroU32 root,SroU32 heap,SroU32 generation){
 SroValue*s=&_gz_sro_private;
 if(!sro_header())return SRO_CORRUPT;
 if(event<SRO_CTOR_PUBLISHED||event>SRO_DIRECT_FREE_BEGIN)return SRO_DENIED;
 if(generation!=s->generation)return SRO_CHANGED;
 if(event==SRO_CTOR_PUBLISHED){
  if(!sro_root_shape(root,heap))return SRO_INVALID;
  if(s->phase!=SRO_EMPTY&&s->phase!=SRO_RETIRED)return SRO_BUSY;
  if(s->generation==0xffffffffu)return SRO_EXHAUSTED;
  /* No input capacity, save pointer, completion flag or authority is used. */
  s->generation++;s->root=root;s->heap=heap;s->capacity=SRO_CAPACITY;
  s->save=root+SRO_SAVE_OFFSET;s->save_bytes=SRO_SAVE_BYTES;
  s->last_event=SRO_CTOR_PUBLISHED;s->retire_reason=0;
  s->phase=SRO_CONSTRUCTED;
  return SRO_OK;
 }
 if(s->phase==SRO_EMPTY||s->root!=root||s->heap!=heap)return SRO_CHANGED;
 if(event==SRO_REUSE_RETURN){
  if(s->phase==SRO_RETIRED)return SRO_CHANGED;
  return SRO_NO_CHANGE;
 }
 if(event==SRO_BOOT_COMPLETED){
  if(s->phase==SRO_BOOT_READY)return SRO_NO_CHANGE;
  if(s->phase!=SRO_CONSTRUCTED)return SRO_CHANGED;
  s->last_event=SRO_BOOT_COMPLETED;s->phase=SRO_BOOT_READY;
  return SRO_OK;
 }
 /* Revoke before any following native destructor/free. No counter needs to
  * increment here, even at generationFFFFFFFF; old queries are now denied. */
 if(s->phase==SRO_RETIRED)return SRO_NO_CHANGE;
 s->phase=SRO_RETIRED;s->last_event=event;s->retire_reason=event;
 return SRO_OK;
}
SroU32 _gz_sro_observe(SroU32 event,SroU32 root,SroU32 heap,SroU32 generation){SroU32 z;
 if(_gz_sro_entered)return SRO_BUSY;
 _gz_sro_entered=1;z=sro_event(event,root,heap,generation);_gz_sro_entered=0;return z;
}
SroU32 _gz_sro_query(SroU32 root,SroU32 generation,SroU32 required,SroValue*out){
 SroU32 z=SRO_OK,i;const SroU32*from=(const SroU32*)&_gz_sro_private;SroU32*to=(SroU32*)out;
 if(_gz_sro_entered)return SRO_BUSY;
 if(!sro_output(out))return SRO_INVALID;
 _gz_sro_entered=1;
 if(!sro_header())z=SRO_CORRUPT;
 else if(required!=SRO_CONSTRUCTED&&required!=SRO_BOOT_READY)z=SRO_DENIED;
 else if(_gz_sro_private.phase==SRO_EMPTY||_gz_sro_private.phase==SRO_RETIRED||
 _gz_sro_private.root!=root||_gz_sro_private.generation!=generation||
 (required==SRO_BOOT_READY&&_gz_sro_private.phase!=SRO_BOOT_READY))z=SRO_CHANGED;
 if(!z)for(i=0;i<SRO_BYTES/4;i++)to[i]=from[i];
 _gz_sro_entered=0;return z;
}
SroU32 _gz_sro_native_admit(void){return SRO_DENIED;}
