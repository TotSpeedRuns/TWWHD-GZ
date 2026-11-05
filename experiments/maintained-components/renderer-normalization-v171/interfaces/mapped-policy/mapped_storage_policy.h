#ifndef GZ_MAPPED_STORAGE_POLICY_H
#define GZ_MAPPED_STORAGE_POLICY_H

#include <stdint.h>
#include "restore_workspace.h"

/* Bounded requests, not measurements or allocation authority. Version0.2.7's
 * largest mapped pool has23,855,104 gross bytes; the video pool15,073,280.
 * Leave room for their actual heap metadata. Every real allocation can still
 * fail due to shared occupancy, fragmentation or an unavailable backend.
 * Do not feed these constants to a diagnostic as a measured SDK largest.
 */
enum {
    GZ_MM_CAPTURE_REQUEST_LIMIT = 22u * 1024u * 1024u,
    GZ_MM_SAVED_REQUEST_LIMIT = 14u * 1024u * 1024u,
    GZ_MM_ROLE_SAVED = 1,
    GZ_MM_ROLE_CURRENT = 2
};

static inline uint32_t gz_mm_stage_request(uint32_t requested, uint32_t role)
{
    uint32_t bounded;
    if (requested < 192u || requested > GZ_RW_CAPACITY_LIMIT ||
        (role != GZ_MM_ROLE_SAVED && role != GZ_MM_ROLE_CURRENT)) return 0;
    bounded = requested < GZ_MM_CAPTURE_REQUEST_LIMIT ? requested : GZ_MM_CAPTURE_REQUEST_LIMIT;
    return (bounded + 31u) & ~31u;
}

/* The compactor calls the explicit video allocator. A refusal leaves the
 * already validated, published staging image owned by the original OIS path.
 */
static inline uint32_t gz_mm_tight_request(uint32_t used, uint32_t capacity)
{
    uint32_t rounded;
    if (used < 192u || (used & 3u) || used > capacity ||
        (capacity & 31u) || capacity > GZ_RW_CAPACITY_LIMIT || used > 0xffffffe0u) return 0;
    rounded = (used + 31u) & ~31u;
    return rounded <= capacity && rounded <= GZ_MM_SAVED_REQUEST_LIMIT ? rounded : 0;
}

/* Preserve the real HMD2 journal/apply/page reservation and encoder minimum.
 * Allocation permission still comes only from the actual mapped allocator,
 * complete backend ownership and all existing OIS private/phase checks.
 */
static inline uint32_t gz_mm_current_plan(GzRestoreCounts saved,
    GzRestoreCounts current, uint32_t capture_bound, uint32_t raw_bytes,
    GzRestoreReservation *out)
{
    return gz_restore_reservation_plan(saved, current, capture_bound, raw_bytes,
                                      GZ_MM_CAPTURE_REQUEST_LIMIT, out);
}

#endif
