#ifndef GZ_SOURCE_WORKER_CUSTODIAN_H
#define GZ_SOURCE_WORKER_CUSTODIAN_H
#include <stdint.h>
enum {SWC_READY=1,SWC_ADMITTED=2,SWC_INSTALLED=3,SWC_HALTED=4,
      SWC_ADMISSION=1,SWC_INSTALL=2,SWC_ADOPT=3,SWC_RETAINED=2801,
      SWC_WORK_SLOT=1,SWC_WORK_MCA=2,SWC_WORK_BFLOWER=3};
/* Fixed worker-only SOURCE custody. No pointer, digest or result is supplied.
 * These calls cannot retain a chooser/queued-frame or create READ/WORLD. */
uint32_t _gz_swc_begin(uint32_t request,uint32_t frame,uint32_t slot);
uint32_t _gz_swc_admission(uint32_t request,uint32_t frame,uint32_t slot);
uint32_t _gz_swc_install(uint32_t request,uint32_t frame,uint32_t slot);
uint32_t _gz_swc_roh_begin(uint32_t request,uint32_t frame,uint32_t slot,uint32_t frames);
uint32_t _gz_swc_cancel(uint32_t request,uint32_t frame,uint32_t slot);
uint32_t _gz_swc_pending(void);
/* Fixed retained GZ wrapper purposes, never native/API outcome providers. */
uint32_t _gz_swc_slot_work_replaced(void);
uint32_t _gz_swc_mca_work_replaced(void);
uint32_t _gz_swc_bflower_work_replaced(void);
/* Actual pin only for the fixed held WCM body during its direct call. */
uint32_t _gz_swc_admission_pin(uint32_t request,uint32_t frame,uint32_t slot);
uint32_t _gz_wct_source_worker_context(uint32_t request,uint32_t frame,uint32_t slot);
#endif
