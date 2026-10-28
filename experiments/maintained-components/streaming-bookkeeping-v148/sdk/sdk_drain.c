/* Scalar SDK observation optimization. Native calls/arguments are external.
 * Existing real metadata arrays and the exact old enter/locate helpers remain
 * authoritative. No callback, new state, allocation or native memory follows.
 * The first drain sweep is mandatory; only empty terminal repeats disappear.
 */
#include "observer.h"
extern u32 sk_opt_original_enter(void) __asm__("_gz_ng_observer_private_enter");
extern u32 sk_opt_original_locate(u32,u32,u32) __asm__("_gz_ng_observer_private_locate");
static void sk_opt_fault(u32 f){__sync_fetch_and_or(&_gz_sk_control.flags,f);}
static void sk_opt_leave(void){__sync_synchronize();_gz_sk_control.busy=0;}
static u32 sk_opt_serial(void){if(_gz_sk_control.serial==0xffffffffu){sk_opt_fault(SK_EXHAUSTED);return 0;}return ++_gz_sk_control.serial;}
static u32 sk_opt_kind(u32 site){u32 i;for(i=0;i<37;i++)if(_gz_sk_sites[i].site==site)return _gz_sk_sites[i].kind;return 0;}
static u32 sk_opt_allocation(u32 k){return k>=SK_ALLOC_EXP&&k<=SK_ALLOC_DEFAULT_EX;}
static u32 sk_opt_creation(u32 k){return k==SK_CREATE_EXP||k==SK_CREATE_FRM;}
static u32 sk_opt_birth(u32 k){return sk_opt_allocation(k)||sk_opt_creation(k);}
static u32 sk_opt_default_kind(u32 k){return k==SK_ALLOC_DEFAULT||k==SK_ALLOC_DEFAULT_EX;}
static u32 sk_opt_heap_index(u32 heap){u32 i;for(i=0;i<SK_HEAPS;i++)if(_gz_sk_heaps[i].live&&_gz_sk_heaps[i].heap==heap)return i;return SK_HEAPS;}
static u32 sk_opt_retire(SkAllocation*r,u32 site){u32 i,first=0xffffffffu;SkHeap*h;
 r->live=0;r->retire_site=site;_gz_sk_control.active--;_gz_sk_control.retired++;
 for(i=0;i<SK_HEAPS;i++){
  h=&_gz_sk_heaps[i];if(h->live!=1)continue;
  if((h->backing_generation==r->generation&&h->backing_pointer==r->pointer&&h->backing_heap==r->heap)||
     (!h->backing_generation&&h->buffer==r->pointer)){h->live=2;h->retire_site=site;if(first==0xffffffffu)first=(u32)h;}
 }
 return first;
}
static void sk_opt_drain(void){u32 i,j,changed;SkHeap*h;SkAllocation*r;
 do{
  changed=0;
  for(i=0;i<SK_HEAPS;i++){
   h=&_gz_sk_heaps[i];if(h->live!=2)continue;
   h->live=0;_gz_sk_control.active_heaps--;_gz_sk_control.heap_retired++;
   for(j=0;j<SK_ALLOCATIONS;j++){
    r=&_gz_sk_allocations[j];if(r->live&&r->heap==h->heap&&(!r->heap_generation||r->heap_generation==h->generation))
     if(sk_opt_retire(r,h->retire_site)<=(u32)h)changed=1;
   }
  }
 }while(changed);
}
u32 _gz_sk_begin(u32 site,u32 a0,u32 a1,u32 a2,u32 target){u32 k,i,s=0,release;SkOperation*o=0,*q;SkAllocation*r;
 if(!sk_opt_original_enter())return 0;
 _gz_sk_observation.valid=0;
 _gz_sk_heap_observation.valid=0;
 _gz_sk_control.attempts++;k=sk_opt_kind(site);
 if(!k){sk_opt_fault(SK_BAD_SITE);goto done;}
 if(_gz_sk_control.flags)goto done;
 for(i=0;i<SK_PENDING;i++)if(!_gz_sk_operations[i].live){o=&_gz_sk_operations[i];break;}
 if(!o){sk_opt_fault(SK_CAPACITY);goto done;}
 s=sk_opt_serial();if(!s)goto done;
 o->serial=s;o->site=site;o->kind=k;o->a0=a0;o->a1=a1;o->a2=a2;o->target=target;o->live=1;_gz_sk_control.pending++;
 release=k==SK_FREE_EXP||k==SK_FREE_DEFAULT||k==SK_DESTROY_EXP||k==SK_DESTROY_FRM;
 /* The result address is unknown until the API returns. A concurrent free
  * may retire it between that return and our completion observer. Refuse
  * every overlapping pending birth rather than publish a potentially freed
  * result. A birth starting during a release is refused for the same reason.
  * live3 remains an owned pending slot, but cannot publish an allocation. */
 for(i=0;i<SK_PENDING;i++){
  q=&_gz_sk_operations[i];if(q==o||!q->live)continue;
  if(release&&sk_opt_birth(q->kind))q->live=3;
  if(sk_opt_birth(k)&&(q->kind==SK_FREE_EXP||q->kind==SK_FREE_DEFAULT||q->kind==SK_DESTROY_EXP||q->kind==SK_DESTROY_FRM))o->live=3;
 }
 if(release){
  if(k==SK_FREE_EXP||k==SK_FREE_DEFAULT){
   i=sk_opt_original_locate(k==SK_FREE_EXP?a0:0,k==SK_FREE_EXP?a1:a0,0);
   if(i<SK_ALLOCATIONS){r=&_gz_sk_allocations[i];if((k==SK_FREE_EXP&&r->kind==SK_ALLOC_EXP)||(k==SK_FREE_DEFAULT&&sk_opt_default_kind(r->kind)))sk_opt_retire(r,site);}
  }else for(i=0;i<SK_ALLOCATIONS;i++){
   r=&_gz_sk_allocations[i];if(r->live&&r->heap==a0)sk_opt_retire(r,site);
  }
  if(k==SK_DESTROY_EXP||k==SK_DESTROY_FRM){
   i=sk_opt_heap_index(a0);if(i<SK_HEAPS){_gz_sk_heaps[i].live=2;_gz_sk_heaps[i].retire_site=site;}
  }
  sk_opt_drain();
 }
done:if(!s)_gz_sk_control.dropped++;sk_opt_leave();return s;
}
