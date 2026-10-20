#include "archive_layout.h"
/* Native RAM-slot allocation/read-pin adapter. Readability comes only from the
 * observed fixed MEMAllocFromExpHeapEx(2MiB,32) return in slots_store. Never
 * adopt an existing pointer/header. This certifies storage, not game coverage.
 * All seams run on the original serialized, nonyielding game lane. */
typedef unsigned int OsaU32;
extern volatile OsaU32 _gz_slots_ptrs[10];
typedef struct {OsaU32 allocation,capacity,identity,generation,used,valid;} OsaRow;
typedef struct {OsaU32 active,slot,serial,allocation,identity,generation,bytes;} OsaPin;
typedef struct {OsaU32 magic,serial,inverse,entered,writer,writer_frame,writer_slot,published;
 OsaRow rows[10];OsaPin pin;} OsaState;
static OsaState _gz_osa_state={.magic=0x4f534131u,.serial=1,.inverse=~1u};
static volatile OsaU32 _gz_osa_orphan;
static OsaU32 osa_shape(OsaU32 p,OsaU32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static OsaU32 osa_apart(OsaU32 p,OsaU32 n,OsaU32 q,OsaU32 m){return p+n<=q||q+m<=p;}
static OsaU32 osa_header(void){OsaState*s=&_gz_osa_state;
 return s->magic==0x4f534131u&&s->serial&&s->inverse==~s->serial&&s->entered<=1&&s->writer<=1&&s->published<=1&&s->pin.active<=1&&
  (s->writer?(s->writer_slot<10&&s->writer_frame&&!(s->writer_frame&7)):(!s->writer_frame&&!s->writer_slot&&!s->published))&&
  (!s->pin.active||(s->pin.slot<10&&s->pin.serial&&s->pin.identity&&s->pin.generation&&s->pin.serial<=s->serial));
}
static OsaU32 osa_issue(void){_gz_osa_state.serial++;_gz_osa_state.inverse=~_gz_osa_state.serial;return _gz_osa_state.serial;}
static OsaU32 osa_image(OsaU32 p,OsaU32 n){return gz_arc_shape(p,n)?0:28;}

OsaU32 _gz_osa_begin(OsaU32 slot,OsaU32 frame){OsaState*s=&_gz_osa_state;OsaRow*r;OsaU32 p;
 /* frame is only an invocation identity from the fixed native SP seam. It
  * is never dereferenced or used as source-memory authority. */
 if(!osa_header()||slot>9||!frame||(frame&7))return 28;
 if(s->entered||s->writer||s->pin.active)return 5;
 if(s->serial>0xfffffffcu)return 74;
 s->entered=1;r=&s->rows[slot];p=_gz_slots_ptrs[slot];
 if(p&&(!r->allocation||p!=r->allocation||r->capacity!=0x200000u||!r->identity||!r->generation)){s->entered=0;return 28;}
 /* A canonically cleared slot retires the prior metadata, never re-adopts it.
  * No pin can exist here. This operation never frees or reads that old body. */
 if(!p){r->allocation=r->capacity=r->identity=0;}
 r->generation=osa_issue();r->used=r->valid=0;
 s->writer=1;s->writer_frame=frame;s->writer_slot=slot;s->published=0;s->entered=0;return 0;
}
OsaU32 _gz_osa_allocated(OsaU32 slot,OsaU32 p,OsaU32 frame){OsaState*s=&_gz_osa_state;OsaRow*r;OsaU32 i;
 if(!osa_header()||s->entered||!s->writer||s->writer_frame!=frame||s->writer_slot!=slot||slot>9)return 28;
 r=&s->rows[slot];if(r->allocation||_gz_slots_ptrs[slot]||!osa_shape(p,0x200000u)||(p&31)||s->serial==0xffffffffu){_gz_osa_orphan=p;return 28;}
 for(i=0;i<10;i++)if(i!=slot&&s->rows[i].allocation&&!osa_apart(p,0x200000u,s->rows[i].allocation,s->rows[i].capacity)){_gz_osa_orphan=p;return 28;}
 r->allocation=p;r->capacity=0x200000u;r->identity=osa_issue();return 0;
}
OsaU32 _gz_osa_publish(OsaU32 slot,OsaU32 p,OsaU32 n,OsaU32 frame){OsaState*s=&_gz_osa_state;OsaRow*r;
 if(!osa_header()||s->entered||s->pin.active||!s->writer||s->writer_frame!=frame||slot>9||s->writer_slot!=slot)return 28;
 r=&s->rows[slot];if(p!=r->allocation||_gz_slots_ptrs[slot]!=p||r->capacity!=0x200000u||!r->identity||!r->generation||r->valid||r->used)return 28;
 if(osa_image(p,n))return 28;
 r->used=n;__asm__ volatile("sync":::"memory");r->valid=1;s->published=1;return 0;
}
OsaU32 _gz_osa_finish(OsaU32 status,OsaU32 frame){OsaState*s=&_gz_osa_state;
 if(!osa_header())return 28;
 /* A nested refused writer cannot cancel the outer writer's identity. */
 if(s->writer&&s->writer_frame==frame){
  if(status==14&&!s->published)status=28;
  if(status!=14){s->rows[s->writer_slot].used=0;s->rows[s->writer_slot].valid=0;}
  s->writer=s->writer_frame=s->writer_slot=s->published=0;
 }
 return status;
}
OsaU32 _gz_osa_acquire(OsaU32 slot){OsaState*s=&_gz_osa_state;OsaRow*r;
 if(!osa_header()||slot>9)return 28;
 if(s->entered||s->writer||s->pin.active)return 5;
 if(s->serial==0xffffffffu)return 74;
 r=&s->rows[slot];
 if(!r->valid||!r->identity||!r->generation||r->allocation!=_gz_slots_ptrs[slot]||r->capacity!=0x200000u||r->used<216||r->used>r->capacity||(r->used&3))return 28;
 s->pin.slot=slot;s->pin.serial=osa_issue();s->pin.allocation=r->allocation;s->pin.identity=r->identity;s->pin.generation=r->generation;s->pin.bytes=r->used;
 __asm__ volatile("sync":::"memory");s->pin.active=1;return 0;
}
OsaU32 _gz_osa_validate(OsaU32 pin){OsaState*s=&_gz_osa_state;OsaPin*p=&s->pin;OsaRow*r;
 if(!osa_header()||s->entered||s->writer||p->active!=1||!pin||pin!=p->serial||p->slot>9)return 28;
 r=&s->rows[p->slot];
 return r->valid==1&&r->allocation==p->allocation&&_gz_slots_ptrs[p->slot]==p->allocation&&r->capacity==0x200000u&&r->identity==p->identity&&r->generation==p->generation&&r->used==p->bytes?0:28;
}
OsaU32 _gz_osa_pin_id(void){return _gz_osa_state.pin.active?_gz_osa_state.pin.serial:0;}
OsaU32 _gz_osa_source(OsaU32 pin){return _gz_osa_validate(pin)?0:_gz_osa_state.pin.allocation;}
/* Private final leaf, called only after same-callback validate and last source
 * use. No source read, native callback, fallible check or world release. */
void _gz_osa_release_leaf(void){_gz_osa_state.pin.active=0;}

OsaU32 _gz_osa_bytes(OsaU32 pin){return _gz_osa_validate(pin)?0:_gz_osa_state.pin.bytes;}
