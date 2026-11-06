#ifndef GZ_OPEN_JOB_WRITER_ADAPTER_R3_H
#define GZ_OPEN_JOB_WRITER_ADAPTER_R3_H

#include <stdint.h>

/* Private admission diagnostic. Mode1 is an empty exclusive TRY followed by
 * genuine metadata LEAVE, which may block; no game reader is granted.
 * Initialization OPEN is one-time success, not current logical admission. */
uint32_t _gz_jwa_init_open(void);
uint32_t _gz_jwa_enter(uint32_t kind, uint32_t incoming_sp,
                     uint32_t caller_lr, uint32_t original_r3,
                     uint32_t original_r5, uint32_t original_r6);
void _gz_jwa_after(uint32_t incoming_sp, uint32_t output[2]);
uint32_t _gz_jwa_world_ready(void);

/* First rows are immutable after state==2: state, thread, SP, LR, r3, r5,
 * r6, invocation serial. Every native EA is an opaque scalar label. */
extern volatile uint32_t _gz_jwa_report[64];
extern volatile uint32_t _gz_jwa_first[9][8];

/* JWA report v3: word25 unwind restriction1, word26 OPEN-only0, word27 WORLD0,
 * word28 MISSING15, word63 mode1 TRY/immediate potentially blocking LEAVE.
 * Existing counter and immutable first-row offsets are unchanged.
 * Logical admission probe only; no world/read-ready grant. */
void _gz_jwa_exclusive_probe(uint32_t serial, uint32_t expected_thread);
extern volatile uint32_t _gz_jwa_exclusive_probe_report[32];

#endif
