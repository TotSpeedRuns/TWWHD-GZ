#ifndef GZ_WCT_FIXED_ROOT_BORROW_H
#define GZ_WCT_FIXED_ROOT_BORROW_H
#include <stdint.h>
enum { WHL_ROOT_CLOCK=1,WHL_ROOT_PERSIST=2,WHL_ROOT_RETAINED=2790 };
/* Fixed real kernel metadata producer. No callback/pointer/result parameter. */
uint32_t _gz_srh_wct_root_context(uint32_t kind,uint32_t mode,uint32_t serial,uint32_t frame);
/* Private entered CLOCK/PERSIST tuple only; not an owner/heap/read grant. */
uint32_t _gz_srh_wct_root_member(uint32_t kind,uint32_t mode,uint32_t serial,uint32_t frame);
uint32_t _gz_wct_root_borrow(uint32_t kind,uint32_t mode,uint32_t serial,uint32_t frame);
uint32_t _gz_wct_root_held(uint32_t kind,uint32_t mode,uint32_t serial,uint32_t frame,uint32_t actual_hg);
uint32_t _gz_wct_root_unborrow(uint32_t kind,uint32_t mode,uint32_t serial,uint32_t frame,uint32_t actual_hg);
uint32_t _gz_wct_root_pending(void);
uint32_t _gz_wct_heap_pending(void);
#endif
