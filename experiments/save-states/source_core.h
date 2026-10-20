#ifndef GZ_TYPE1_POT_SOURCE_CORE_H
#define GZ_TYPE1_POT_SOURCE_CORE_H
#include "integration.h"
/* Pure source correspondence only. A nonzero result points into the owner's
 * already pinned bank. It is not a layer/resource/lifetime certificate.
 * Original APP bytes remain separate from post-Create actor parameters. */
u32 _gz_rip_source_row(const RiPlan*,const NbClock*);
/* Existing typed scalar/self/child preflight, using the fresh live PID.
 * No native calls, actor stores, publication, or lifetime permission. */
u32 _gz_rip_ready(const RiPlan*,const NbClock*,u32 actor,u32 live_pid);
#endif
