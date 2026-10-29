#ifndef GZ_CURRENT_ITEM_LIFECYCLE_SCALAR_H
#define GZ_CURRENT_ITEM_LIFECYCLE_SCALAR_H
/* Scalar-only policies; no API or native outcome is implemented here. */
enum {LRR_IDLE=0,LRR_ARMED=1,LRR_CALLING=2,LRR_WAIT=3,
 LRR_REPLAY_READY=4,LRR_FINISHED=5,LRR_HALTED=6};
static unsigned int lrr_mask(unsigned int n){return n&&n<=8?((1u<<n)-1u):0;}
static unsigned int lrr_release_observed(unsigned int phase,unsigned int nonce,
 unsigned int own_serial,unsigned int report_nonce,unsigned int report_serial,
 unsigned int report_phase,unsigned int first_error,unsigned int active,
 unsigned int job,unsigned int ticket){
 return (phase==LRR_REPLAY_READY||phase==LRR_HALTED)&&nonce&&own_serial&&nonce==report_nonce&&
  own_serial==report_serial&&report_phase==4&&!first_error&&!active&&!job&&!ticket;
}
static unsigned int lrr_po_completion(unsigned int state,unsigned int end,
 unsigned int flags){return state==3&&end&&!flags;}
static unsigned int lrr_co_completion(unsigned int state,unsigned int end,
 unsigned int site,unsigned int flags){return state==2&&end&&site&&!(flags&~1u);}
static unsigned int lrr_heap_completion(unsigned int state,unsigned int end,
 unsigned int site){return state==3&&end&&site;}
#endif
