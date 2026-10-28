/* Retain the complete8192-row preflight. Cache at most32 matching rows
 * for sparse retirement, with an overflow fallback. The selected parent
 * already bounds its retirement scan by first/last matches; this is not
 * removal of a previously unconditional zero-match scan. No game body
 * reads, omitted lifetime events, capacity or gameplay changes. */
#include "members.h"
extern volatile u32 _gz_co_frame;
enum {CM_FAST_INDICES=32};
/* Single fixed private scratch. Existing CMbusy is acquired before any write
 * and held through every use; failed/reentrant admissions never touch it. */
static CoEntry *cm_fast_rows[CM_FAST_INDICES] __attribute__((section(".bss.gz_cm_fast")));
static u32 cm_fast_span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xc0000000u&&n&&n<=0xc0000000u-p;}
static void cm_fast_flag(u32 f){__sync_fetch_and_or(&_gz_cm_control.flags,f);}
static void cm_fast_co_flag(u32 f){__sync_fetch_and_or(&_gz_co_header.flags,f);}
static u32 cm_fast_header(void){const CmControl*c=&_gz_cm_control;return c->magic==CM_MAGIC&&c->bytes==64&&c->version==1&&!c->session&&!c->reserved;}
static __attribute__((noinline)) u32 cm_fast_co_header(void){const CoHeader*h=&_gz_co_header;return h->magic==CO_MAGIC&&h->bytes==64&&h->version==1&&h->capacity==CO_CAP&&h->rows==(u32)_gz_co_rows&&h->active<=CO_CAP;}
static void cm_fast_bump(u32*p){if(*p==0xffffffffu)cm_fast_flag(CM_COUNTER_EXHAUSTED);else ++*p;}
static u32 cm_fast_active(u32 s){return s==CO_LIVE||s==CO_UNOBSERVED;}
u32 _gz_cm_invalidate(u32 p,u32 n,u32 reason){CmControl*c=&_gz_cm_control;CoHeader*h=&_gz_co_header;
 u32 i,remaining,active=0,count=0,frame,flag,matched=0;CoEntry*e;
 if(!cm_fast_header()){cm_fast_flag(CM_BAD_CO_HEADER);return CM_UNAVAILABLE;}
 if(!__sync_bool_compare_and_swap(&c->busy,0,1)){cm_fast_flag(CM_REENTRY);return CM_UNAVAILABLE;}
 cm_fast_bump(&c->attempts);
 if(!cm_fast_span(p,n)||(reason!=CM_CREATED&&reason!=CM_ENDING)){cm_fast_flag(CM_BAD_SPAN);goto done;}
 if(!cm_fast_co_header()){cm_fast_flag(CM_BAD_CO_HEADER);cm_fast_co_flag(CO_BAD_HEADER);goto done;}
 if(!__sync_bool_compare_and_swap(&h->busy,0,1)){cm_fast_flag(CM_CO_BUSY);cm_fast_co_flag(CO_REENTRY);goto done;}
 frame=_gz_co_frame;
 /* Exactly the parent's whole-table preflight and active-count proof.
  * Extra stores go only to owned scratch; no CO row mutates before success. */
 for(remaining=CO_CAP,e=_gz_co_rows;remaining;--remaining,++e){
  if(e->state==CO_EMPTY){if(e->object|e->generation)goto corrupt;continue;}
  if(e->state!=CO_LIVE&&e->state!=CO_DEAD&&e->state!=CO_UNOBSERVED)goto corrupt;
  if(!cm_fast_span(e->object,0x50)||e->generation>h->serial||
   (e->state==CO_LIVE&&!e->generation)||(e->state==CO_UNOBSERVED&&e->generation))goto corrupt;
  if(cm_fast_active(e->state)){
   ++active;
   if(p<e->object+0x50&&e->object<p+n){
    if(matched<CM_FAST_INDICES)cm_fast_rows[matched]=e;
    ++matched;
    if(matched==CM_FAST_INDICES+1){--remaining;++e;break;}
   }
  }
 }
 /* Overflow enters the exact original validation-only tail: no further
  * overlap tests or scratch writes; every remaining row is still checked. */
 for(;remaining;--remaining,++e){
  if(e->state==CO_EMPTY){if(e->object|e->generation)goto corrupt;continue;}
  if(e->state!=CO_LIVE&&e->state!=CO_DEAD&&e->state!=CO_UNOBSERVED)goto corrupt;
  if(!cm_fast_span(e->object,0x50)||e->generation>h->serial||
   (e->state==CO_LIVE&&!e->generation)||(e->state==CO_UNOBSERVED&&e->generation))goto corrupt;
  if(cm_fast_active(e->state))++active;
 }
 if(active!=h->active)goto corrupt;
 flag=reason==CM_CREATED?CM_ROW_CREATED:CM_ROW_ENDING;
 if(matched<=CM_FAST_INDICES){
  for(i=0;i<matched;i++){
   e=cm_fast_rows[i];
   e->dead_frame=frame;e->dtor_site=0;e->flags|=flag;
   __sync_synchronize();e->state=CO_DEAD;++count;
  }
 }else{
  /* Overflow scans all rows without truncation or partial retirement.
   * The selected parent instead bounds retirement by first/last matches;
   * the exact selected-parent comparisons are documented in README.md. */
  for(remaining=CO_CAP,e=_gz_co_rows;remaining;--remaining,++e){
   if(cm_fast_active(e->state)&&p<e->object+0x50&&e->object<p+n){
    e->dead_frame=frame;e->dtor_site=0;e->flags|=flag;
    __sync_synchronize();e->state=CO_DEAD;++count;
   }
  }
 }
 h->active-=count;c->process=p;c->allocation_bytes=n;c->reason=reason;c->frame=frame;
 c->count=count;c->co_serial=h->serial;cm_fast_bump(&c->commits);
 if(c->retired>0xffffffffu-count){c->retired=0xffffffffu;cm_fast_flag(CM_COUNTER_EXHAUSTED);}else c->retired+=count;
 if(h->flags)cm_fast_flag(CM_SOURCE_GAP);
 if(_gz_co_frame!=frame)cm_fast_flag(CM_FRAME_CHANGED);
 __sync_synchronize();*(volatile u32*)&h->busy=0;
 __sync_synchronize();*(volatile u32*)&c->busy=0;
 return c->flags?CM_UNAVAILABLE:CM_OK;
corrupt:cm_fast_flag(CM_BAD_ROW);cm_fast_co_flag(CO_BAD_HEADER);
 __sync_synchronize();*(volatile u32*)&h->busy=0;
done:__sync_synchronize();*(volatile u32*)&c->busy=0;return CM_UNAVAILABLE;
}
