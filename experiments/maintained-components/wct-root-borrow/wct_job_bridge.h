#ifndef GZ_WCT_COUPLED_JOB_BRIDGE_H
#define GZ_WCT_COUPLED_JOB_BRIDGE_H
#include "common_admission.h"
enum {WJL_CONTEXT=0x190,WJL_STORAGE=0x191,WJL_NOT_READY=0x192,
      WJL_ZERO=0,WJL_BUSY=1,WJL_BROKEN=2};
extern GzJcaExclusive _gz_wct_job_token;
uint32_t _gz_wct_heap_context_owned(uint32_t);
uint32_t _gz_jwa_wct_try(uint32_t);
uint32_t _gz_jwa_wct_check(uint32_t);
uint32_t _gz_jwa_wct_workers_zero(uint32_t);
uint32_t _gz_jwa_wct_leave(void);
#endif
