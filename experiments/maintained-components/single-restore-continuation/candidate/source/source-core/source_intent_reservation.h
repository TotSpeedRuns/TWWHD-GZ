#ifndef GZ_SOURCE_INTENT_RESERVATION_H
#define GZ_SOURCE_INTENT_RESERVATION_H
#include <stdint.h>
/* Original entered WCT worker + its real OSA pin are required only at entry.
 * A logical reservation grants no READ/WORLD/native or physical pin access. */
uint32_t _gz_osa_intent_reserve(uint32_t request,uint32_t frame,uint32_t slot,
 uint32_t identity,uint32_t generation,uint32_t used,uint32_t hash);
uint32_t _gz_osa_intent_reservation_match(uint32_t cookie,uint32_t request,
 uint32_t slot,uint32_t identity,uint32_t generation,uint32_t used,uint32_t hash);
/* Validate exact own receipt, then fixed private clear. Call before a terminal
 * status is published, never from an enclosing nonfallible post-owner tail. */
uint32_t _gz_osa_intent_release(uint32_t cookie,uint32_t request);
#endif
