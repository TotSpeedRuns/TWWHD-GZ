#ifndef GZ_WCT_COUPLED_HEAP_LEASE_H
#define GZ_WCT_COUPLED_HEAP_LEASE_H
#include <stdint.h>
enum {WHL_OK=0,WHL_BUSY=1850,WHL_CONTEXT=1851,WHL_INTEGRITY=1852,
      WHL_RETAINED=1853,WHL_EXHAUSTED=1854};
/* Fixed native NBD/HBP producer. These are not public context setters and
 * cannot set GFO/JOL entered, completion, WORLD or READ readiness. */
void _gz_wct_native_enter(void);
void _gz_wct_native_exit(void);
uint32_t _gz_wct_native_owned(void);
/* One fixed WCT transaction: actual HG begin FIRST, then immediate real
 * sameGate exclusive TRY and actual worker-zero before any SSP binding.
 * Keep BOTH actual tokens through final target/inverse verification.
 * End releases real job token before HG. Failure retains genuine tokens.
 * No end/rebegin while the same transaction journal exists. */
uint32_t _gz_wct_heap_begin(uint32_t token,uint32_t frame);
uint32_t _gz_wct_heap_end(uint32_t token,uint32_t frame);
void _gz_wct_heap_finish_leaf(uint32_t token);
/* Metadata-only actual two-hold predicates. Identity is monotonic and cannot
 * be reused across frames/transactions. This covers only enrolled native
 * writers/retirement routes, never untracked audio/resource destruction. */
uint32_t _gz_wct_heap_held(uint32_t token,uint32_t frame);
uint32_t _gz_wct_heap_identity(uint32_t token,uint32_t frame);
uint32_t _gz_wct_heap_claims(uint32_t cookie);
uint32_t _gz_wct_heap_context_owned(uint32_t cookie);
/* Fixed WGP enter/leave only. Actual token borrowing, never nested HG/job
 * admission. An ended/failed pending journal forbids legacy fallback. */
uint32_t _gz_wct_heap_pending(void);
uint32_t _gz_wct_heap_borrow(uint32_t token,uint32_t frame,uint32_t mode);
uint32_t _gz_wct_heap_unborrow(uint32_t token,uint32_t frame,uint32_t mode,uint32_t actual_hg);
extern volatile uint32_t _gz_wct_heap_report[32];
#endif
