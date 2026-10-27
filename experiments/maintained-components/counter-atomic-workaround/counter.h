/* SPDX-License-Identifier: MIT
 * One aligned, caller-owned 32-bit counter. Compile counter.c separately. */
#ifndef GZ_JWA_COUNTER_H
#define GZ_JWA_COUNTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The caller must validate the index and materialize the complete address. */
void _gz_jwa_increment_counter(volatile uint32_t *counter);

#ifdef __cplusplus
}
#endif

#endif /* GZ_JWA_COUNTER_H */
