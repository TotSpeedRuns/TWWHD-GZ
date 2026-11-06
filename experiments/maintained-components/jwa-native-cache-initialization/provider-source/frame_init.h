#ifndef GZ_JWA_FRAME_INIT_H
#define GZ_JWA_FRAME_INIT_H
#include <stdint.h>
/* Logical Source-only initializers; neither grants Read/world authority. */
uint32_t _gz_jwa_init_after_native_frame(uint32_t parent, uint32_t thread, uint32_t frame);
/* Returns accepted initialization, including a checked same-thread OPEN.
 * This does not distinguish fresh construction and is not a lifetime lease. */
uint32_t _gz_jwa_init_for_native_jwt(void);
#endif
