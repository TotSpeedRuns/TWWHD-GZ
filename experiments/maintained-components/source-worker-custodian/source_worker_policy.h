#ifndef GZ_SOURCE_WORKER_POLICY_H
#define GZ_SOURCE_WORKER_POLICY_H
#include <stdint.h>
#include "source_worker_custodian.h"
typedef struct {uint32_t request,frame,slot,thread,parent;} GzSwcContext;
/* Scalar equality only; never establishes a native producer or issues a pin.
 * The implementation obtains b from real current native metadata each time. */
static inline uint32_t gz_swc_context_same(const GzSwcContext*a,const GzSwcContext*b){
 return a->request&&a->frame&&a->slot<10&&a->thread&&a->parent&&
  a->request==b->request&&a->frame==b->frame&&a->slot==b->slot&&
  a->thread==b->thread&&a->parent==b->parent;
}
static inline uint32_t gz_swc_stage(uint32_t phase,uint32_t borrower,uint32_t stage){
 return stage>=1&&stage<=3&&phase==stage&&!borrower;
}
/* Scalar order only; real entry points still authenticate the live lease. */
static inline uint32_t gz_swc_work_next(uint32_t count,uint32_t purpose){
 return count<SWC_WORK_BFLOWER&&purpose>=SWC_WORK_SLOT&&
  purpose<=SWC_WORK_BFLOWER&&purpose==count+1;
}
#endif
