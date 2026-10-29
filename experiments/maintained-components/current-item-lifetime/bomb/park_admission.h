#ifndef GZ_CURRENT_ONLY_PARK_ADMISSION_H
#define GZ_CURRENT_ONLY_PARK_ADMISSION_H
#include <stdint.h>
#include "worker_atomic.h"
#include "wct_heap_lease.h"
enum { IPK_OK=0,IPK_BUSY=1,IPK_CONTEXT=2,IPK_BROKEN=3 };
/* Private lexical borrower of the SAME transaction HG and job tokens.
 * No begin/end invokes Gate TRY/LEAVE or HG begin/end. The genuine WHL/NBD
 * producer must already own both tokens. No GFO or WORLD/READ grant. */
uint32_t _gz_ipk_admission_begin(uint32_t token,uint32_t frame,uint32_t mode);
uint32_t _gz_ipk_admission_check(void);
uint32_t _gz_ipk_admission_end(void);
uint32_t _gz_ipk_admission_outstanding(void);
uint32_t _gz_ipk_context_owned(uint32_t token);
uint32_t _gz_ipk_idle(void);
uint32_t _gz_ipk_private_apart(uint32_t pointer,uint32_t bytes);
/* Checked before actual WHL END; private finish follows successful END. */
uint32_t _gz_ipk_before_end(uint32_t token,uint32_t frame);
void _gz_ipk_finish_leaf(uint32_t token);
extern volatile uint32_t _gz_ipk_admission_report[32];
/* No caller hooks here. Future CLEAN prepare needs a genuine earlier source
 * handoff and coupled hold; moving the current late SSP begin is insufficient. */
uint32_t _gz_ipk_prepare(uint32_t token,uint32_t frame);
uint32_t _gz_ipk_native_roundtrip(uint32_t token,uint32_t frame);
uint32_t _gz_ipk_native_park(uint32_t token,uint32_t frame);
uint32_t _gz_ipk_native_unpark(uint32_t token,uint32_t frame);
#endif
