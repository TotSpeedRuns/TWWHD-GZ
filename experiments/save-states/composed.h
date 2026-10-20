#ifndef GZ_TYPE1_SOURCE_RESOURCE_COMPOSED_H
#define GZ_TYPE1_SOURCE_RESOURCE_COMPOSED_H
#include "source_core.h"
#include "resource_checks.h"
/* Zero means exact source/APP/birth correspondence and current Type1 WAIT
 * scalar/self/child/callback/registered-heap/Always resource checks passed.
 * Caller must already own a stable Management boundary and genuine live
 * actor identity. This is not _gz_ri_typed and grants no lifetime admission. */
u32 _gz_ripc_ready(const RiPlan*,const NbClock*,u32 actor,u32 live_pid);
/* Generation, shared resource lifetime, global particle reverse references
 * and destructor receipts remain unresolved here; currently returns15. */
u32 _gz_ripc_missing(void);
#endif
