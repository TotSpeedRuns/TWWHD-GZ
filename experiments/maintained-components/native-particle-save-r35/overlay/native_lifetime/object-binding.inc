/* Included after rebind.inc: use the same observer lock and private-span
 * checks, without racing the older shared _gz_hl_observation output. */
#include "object_binding.h"
typedef char ObSize[sizeof(ObBinding)==64?1:-1];
/* Completed native mutation births and explicit image rebinds are provenance,
 * not a stale lifetime. The current parent generation/epoch/scope still has
 * to match; unknown or release/retirement flags remain rejected. */
#define OB_PROVENANCE (HM_BORN_DURING|256u)
static u32 hrp_ob_private_original(u32 p,u32 n){u32 i;HlHeap*h;
 /* The fixed protected-set predicates do not depend on h. Keep them at
  * both boundaries; each current heap still gets BOTH original disjointness
  * tests under the same actual observer lock. No protected range is omitted. */
 if(!hr_private(p,n,0))return 0;
 for(i=0;i<HL_HEAPS;i++){
  h=_gz_hl_heaps+i;
  if(h->heap&&(h->state==HL_LIVE||h->state==HL_MUTATING||h->state==HL_ENDING)&&
   (!hr_apart(p,n,h->start,h->bytes)||!hr_apart(p,n,h->heap,header_bytes(h->kind))))return 0;
 }
 return hr_private(p,n,0);
}
static u32 ob_private(u32 p,u32 n){HrpTicks started=0;HrpInvocation invocation;u32 active=hrp_window(&invocation),result;
 if(active)started=hrp_clock();result=hrp_ob_private_original(p,n);
 if(active)hrp_note(1,&invocation,started);return result;
}
static void ob_fill(ObBinding*b,u32 p,u32 n,const HlStoredAllocation*a,const HlHeap*h){
 zero(b,sizeof(*b));b->magic=0x4f424431u;b->bytes=sizeof(*b);b->version=1;
 b->query=p;b->query_bytes=n;b->pointer=a->pointer;b->capacity=a->capacity;b->generation=a->generation;
 b->heap=h->heap;b->heap_generation=h->generation;b->epoch=h->epoch;b->kind=a->kind;b->block=a->block;
 b->missing=HL_MISSING_ALL; /* This record never manufactures a held/session capability. */
}
ObU32 _gz_ob_capture(u32 p,u32 n,ObBinding*out){HlStoredAllocation*a;HlHeap*h;ObBinding b;u32 z=OB_OBSERVER,ok;
 /* Existing report words only; preserve the actual short-circuit predicates. */
 _gz_ghc_report[11]=0x4f424731u;_gz_ghc_report[12]=p;_gz_ghc_report[13]=n;
 _gz_ghc_report[14]=(u32)out;_gz_ghc_report[16]=_gz_ghc_report[17]=_gz_ghc_report[18]=_gz_ghc_report[19]=0xffffffffu;
 ok=span(p,n);_gz_ghc_report[15]=ok;if(!ok)return OB_ARGUMENT;
 ok=_gz_image_write_allowed((u32)out,sizeof(*out));_gz_ghc_report[16]=ok;if(!ok)return OB_ARGUMENT;
 if(!lock())return OB_OBSERVER;
 ok=hr_healthy();_gz_ghc_report[17]=ok;if(!ok)goto done;
 ok=ob_private((u32)out,sizeof(*out));_gz_ghc_report[18]=ok;if(!ok){z=OB_ARGUMENT;goto done;}
 ok=owner_index_locked(p,n,&a,&h);_gz_ghc_report[19]=ok;if(ok){z=OB_ABSENT;goto done;}
 if(!a->generation||a->missing!=HL_MISSING_ALL||a->reserved||a->release_token||a->state!=HL_LIVE||
  a->kind!=h->kind||a->heap!=h->heap||(a->flags&~OB_PROVENANCE)){z=OB_OBSERVER;goto done;}
 ob_fill(&b,p,n,a,h);copy(out,&b,sizeof(b));z=OB_OK;
done:unlock();return z;
}
static ObU32 ob_validate(const ObBinding*in,u32 before,u32 source_frame){ObBinding b,now;HlStoredAllocation*a;HlHeap*h;u32 z=OB_OBSERVER,frame=0;
 if(!lock())return OB_OBSERVER;
 if(!hr_healthy())goto done;
 if(before){frame=_gz_hl_frame;
  if(!source_frame||frame-source_frame>=0x80000000u){z=OB_ARGUMENT;goto done;}}

 if(!ob_private((u32)in,sizeof(*in))){z=OB_ARGUMENT;goto done;}
 copy(&b,in,sizeof(b));
 if(b.magic!=0x4f424431u||b.bytes!=sizeof(b)||b.version!=1||!span(b.query,b.query_bytes)||
  !b.generation||!b.heap_generation||b.missing!=HL_MISSING_ALL||b.session||b.reserved){z=OB_ARGUMENT;goto done;}
 /* Resolve current metadata afresh. Never dereference b.query/b.pointer or
  * use a historical row address as a current lookup result. */
 if(owner_index_locked(b.query,b.query_bytes,&a,&h)){z=OB_CHANGED;goto done;}
 if((a->flags&~OB_PROVENANCE)||a->reserved||a->release_token||a->missing!=HL_MISSING_ALL||a->kind!=h->kind||a->heap!=h->heap){z=OB_CHANGED;goto done;}
 ob_fill(&now,b.query,b.query_bytes,a,h);z=OB_OK;
 {u32 i;for(i=0;i<sizeof(b)/4;i++)if(((u32*)&b)[i]!=((u32*)&now)[i]){z=OB_CHANGED;break;}}
 if(before&&z==OB_OK){u32 allocation_age=source_frame-a->born_frame,heap_age=source_frame-h->born_frame;
  if(!allocation_age||allocation_age>=0x80000000u||!heap_age||heap_age>=0x80000000u)z=OB_NEWER;
  if(frame!=_gz_hl_frame)z=OB_OBSERVER;
 }
done:unlock();return z;
}
ObU32 _gz_ob_validate(const ObBinding*in){return ob_validate(in,0,0);}
ObU32 _gz_ob_validate_before(const ObBinding*in,ObU32 source_frame){return ob_validate(in,1,source_frame);}

