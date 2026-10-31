#ifndef GZ_OWNED_HEAP_PACKED_STORAGE_POLICY_H
#define GZ_OWNED_HEAP_PACKED_STORAGE_POLICY_H
/* Only allocation sizing. The caller supplies the genuine SDK largest-block
 * result under existing OIS/GHS ownership. This creates no storage lease or
 * native access grant. Roles are asserted against the real OIS enum in its TU.
 * SAVED leaves space for its exact-sized unpublished copy. CURRENT is short
 * lived: it keeps one region and reserves the actual workspace upper bound. */
static inline unsigned int ghc_packed_staging_budget(unsigned int requested,
 unsigned int largest,unsigned int role,unsigned int work_bytes){
 unsigned int available,reserve;
 if(!requested)return 0;
 if(role==1u)available=(largest/2u)&~31u;
 else if(role==2u){
  if(work_bytes>0xffffff60u)return 0;
  reserve=(work_bytes+128u+31u)&~31u;
  available=largest>reserve?(largest-reserve)&~31u:0;
 }else return 0;
 return requested<available?requested:available;
}
#endif
