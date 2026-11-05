#ifndef GZ_JOB_OWNER_CONTEXT_H
#define GZ_JOB_OWNER_CONTEXT_H
#include <stdint.h>

/* A fixed private producer seam, never a public context-cookie setter. */
void _gz_jol_after_native_frame(uint32_t parent, uint32_t thread,
                                uint32_t frame);
uint32_t _gz_jol_context_owned(uint32_t cookie);
extern volatile uint32_t _gz_jol_context_report[32];
#endif
