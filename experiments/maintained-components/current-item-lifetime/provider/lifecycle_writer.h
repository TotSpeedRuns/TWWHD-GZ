#ifndef GZ_NATIVE_LIFECYCLE_WRITER_H
#define GZ_NATIVE_LIFECYCLE_WRITER_H
#include <stdint.h>
/* Fixed NBD dispatcher only.0 means no lifecycle request;1 consumes this
 * NBD service slot while the separate lifecycle owner is pending/terminal.
 * It does not report restore success or authorize a source-world commit. */
uint32_t _gz_lrw_dispatch(void);
/* Right-now lexical writer identity; never a cross-frame cookie or READ grant. */
uint32_t _gz_lrw_owned(void);
/* Required real consumer contract, NOT implemented/simulated here:
 * requested reads only its genuine private intent nonce, with no API calls;
 * step independently authenticates source/current actor/resource ownership.
 * No caller-supplied callback pointer, native EA, context or result exists. */
uint32_t _gz_lrr_requested(void);
uint32_t _gz_lrr_native_step(void);
#endif
