#ifndef GZ_JWA_FRAME_INIT_H
#define GZ_JWA_FRAME_INIT_H
#include <stdint.h>
/* Logical Source-only cold initializer; no read/world or Native caller grant. */
uint32_t _gz_jwa_init_after_native_frame(uint32_t parent, uint32_t thread, uint32_t frame);
#endif
