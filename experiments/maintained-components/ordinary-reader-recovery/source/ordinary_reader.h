#ifndef GZ_RECOVERED_ORDINARY_READER_H
#define GZ_RECOVERED_ORDINARY_READER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ABI scalars are opaque 32-bit request and frame identities. Neither scalar
 * grants a token, held lifetime, READ, WORLD or saved-image ownership. */
enum {
    GZ_ORDINARY_READER_OK = 0,
    GZ_ORDINARY_READER_CONFLICT = 5,
    GZ_ORDINARY_READER_CONTEXT = 0xB72,
    GZ_ORDINARY_READER_RETAINED = 0xB73
};

typedef struct GzOrdinaryReaderExclusive16 {
    uint32_t gate;
    uint32_t thread;
    uint32_t generation;
    uint32_t active;
} GzOrdinaryReaderExclusive16;

typedef struct GzOrdinaryReaderHolder44 {
    uint32_t active;          /* +0 */
    uint32_t halted;          /* +4; sticky, release can still drain */
    uint32_t request;         /* +8 */
    uint32_t frame;           /* +12; caller scalar, distinct from +20 */
    uint32_t thread;          /* +16 */
    uint32_t nbd_serial;      /* +20; sampled NBD report[4] dispatch serial */
    uint32_t issue_serial;    /* +24; never wraps */
    GzOrdinaryReaderExclusive16 token; /* +28; fixed owner, never caller-selected */
} GzOrdinaryReaderHolder44;

#if defined(__cplusplus)
#define GZ_ORR_STATIC_ASSERT(p, m) static_assert(p, m)
#else
#define GZ_ORR_STATIC_ASSERT(p, m) _Static_assert(p, m)
#endif
GZ_ORR_STATIC_ASSERT(sizeof(GzOrdinaryReaderExclusive16) == 16, "exclusive token16");
GZ_ORR_STATIC_ASSERT(sizeof(GzOrdinaryReaderHolder44) == 44, "logical reader holder44");
GZ_ORR_STATIC_ASSERT(offsetof(GzOrdinaryReaderHolder44, frame) == 12, "frame offset12");
GZ_ORR_STATIC_ASSERT(offsetof(GzOrdinaryReaderHolder44, nbd_serial) == 20, "NBD serial offset20");
GZ_ORR_STATIC_ASSERT(offsetof(GzOrdinaryReaderHolder44, token) == 28, "fixed token offset28");
GZ_ORR_STATIC_ASSERT(offsetof(GzOrdinaryReaderExclusive16, active) == 12, "token active offset12");
#undef GZ_ORR_STATIC_ASSERT

/* New logical Source NOBITS owner only. No Native mapping or startup clear
 * is selected by this definition; later integration must close its lifetime. */
extern GzOrdinaryReaderHolder44 _gz_ordinary_reader_holder;

uint32_t _gz_ordinary_reader_current(uint32_t request, uint32_t frame);
uint32_t _gz_ordinary_reader_release(uint32_t request);
uint32_t _gz_ordinary_reader_acquire(uint32_t request, uint32_t frame);

/* Explicit external OBJECT roles. Their Source undefined symbols carry no
 * fabricated Native sizes or absolute addresses. The Gate is JWA owner+8. */
extern unsigned char _gz_ordinary_reader_jwa_owner[];
extern uint32_t _gz_ordinary_reader_report[];      /* actual96B diagnostic role */
extern uint32_t _gz_ordinary_reader_nbd_report[];  /* +16 holds dispatch serial */

/* External method roles, not fabricated Native FUNC aliases. A later join
 * must bind each genuine implementation and preserve its own admission rules. */
uint32_t _gz_ordinary_reader_view_preflight(uint32_t request, uint32_t frame);
uint32_t _gz_ordinary_reader_workers_quiet(void); /* 0 quiet, 1 workers, 2 invalid */
uint32_t _gz_ordinary_reader_current_thread(void); /* qualified wrapper role */
uint32_t _gz_ordinary_reader_gate_try_exclusive(void *gate, GzOrdinaryReaderExclusive16 *token);
uint32_t _gz_ordinary_reader_gate_leave_exclusive(void *gate, GzOrdinaryReaderExclusive16 *token);
uint32_t _gz_ordinary_reader_sticky_halt(uint32_t error);

/* These two names are genuine coreinit imports. SDK implementations, module
 * readiness, ordinary context and held-lock semantics are not supplied here. */
int32_t OSDisableInterrupts(void);
int32_t OSRestoreInterrupts(int32_t previous_state);

/* Return behavior is intentionally preserved instruction-for-instruction:
 * - current returns B72 on identity/context failure, 5 for either nonzero
 *   workers-quiet class, and 0 only with the original fixed exclusive hold.
 * - acquire returns B73 for retained/occupied/serial-exhausted state, B72 for
 *   failed context or cleaned-up failure, 5 for original conflict classes,
 *   and 0 while retaining the successful hold. Real cleanup runs on failures.
 * - release accepts the original same request even when halted. It requires
 *   ordinary IRQ context/same thread, calls real exclusive LEAVE, and clears
 *   active only when the real token is cleared. Other outcomes retain B73.
 * - sticky_halt sets halted and the first diagnostic error; it does not clear
 *   active/token/stage. This recovery never resets, rearms or models it.
 *
 * This ordinary request/frame reader differs from saved-image OIS leases,
 * Restore HELD views and the fixed-JOL-token workers-zero service.
 */

#ifdef __cplusplus
}
#endif
#endif
