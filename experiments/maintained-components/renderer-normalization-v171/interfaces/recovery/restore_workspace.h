#ifndef GZ_RECOVERY_RESTORE_WORKSPACE_H
#define GZ_RECOVERY_RESTORE_WORKSPACE_H
/* New sizing candidate for the published v154 HMD/HMX algorithms.
 * This supplies bytes only: no storage, owner, archive validity, native-domain
 * permission or generation-counter readiness is implied by a successful plan.
 * Counts must come from the actual proposal and fully validated saved pin.
 */
#include <stdint.h>

enum {
    GZ_RW_HEAPS = 4096, GZ_RW_ALLOCATIONS = 262144, GZ_RW_PROCESSES = 8192,
    GZ_RW_PAGE_BYTES = 65536, GZ_RW_CAPACITY_LIMIT = 0x40000000,
    GZ_RW_OK = 0, GZ_RW_ARGUMENT = 1, GZ_RW_OVERFLOW = 2, GZ_RW_SPACE = 3
};

typedef struct {
    uint32_t heaps, allocations, processes;
} GzRestoreCounts;

typedef struct {
    uint32_t journal_bytes, apply_bytes, workspace_bytes;
    uint32_t page_offset, workspace_offset, staging_capacity;
} GzRestoreWorkspace;

typedef struct {
    uint32_t workspace_bytes, tail_bytes, minimum_capture_bytes;
    uint32_t wanted_bytes, allocation_bytes, capture_capacity;
} GzRestoreReservation;

static inline uint32_t gz_rw_union(uint32_t a, uint32_t b, uint32_t limit)
{
    return a > limit - b ? limit : a + b;
}

/* Same count/stride alignment envelope used by hmd_add. The result is the
 * exact HMD/HMX requirement for these counts, not an arbitrary growth margin.
 */
static inline uint32_t gz_rw_add(uint32_t *at, uint32_t count, uint32_t stride)
{
    if (!stride || *at > 0x3fffffe0u || count > (0x3fffffe0u - *at) / stride) return 0;
    *at = (*at + count * stride + 31u) & ~31u;
    return 1;
}

static inline uint32_t gz_rw_metadata_plan(GzRestoreCounts saved,
    GzRestoreCounts current, GzRestoreWorkspace *out)
{
    uint32_t journal = 384, apply = 320, largest, classes, hash = 1, marks;
    uint32_t workspace;
    if (!out) return GZ_RW_ARGUMENT;
    *out = (GzRestoreWorkspace){0};
    if (!saved.heaps || !saved.allocations || !current.heaps || !current.allocations ||
        saved.heaps > GZ_RW_HEAPS || current.heaps > GZ_RW_HEAPS ||
        saved.allocations > GZ_RW_ALLOCATIONS || current.allocations > GZ_RW_ALLOCATIONS ||
        saved.processes > GZ_RW_PROCESSES || current.processes > GZ_RW_PROCESSES) return GZ_RW_ARGUMENT;
    largest = saved.heaps > saved.allocations ? saved.heaps : saved.allocations;
    if (saved.processes > largest) largest = saved.processes;
    classes = GZ_RW_ALLOCATIONS * 5u + largest * 4u + saved.processes * 4u;
    if (!gz_rw_add(&journal, gz_rw_union(saved.heaps, current.heaps, GZ_RW_HEAPS), 144) ||
        !gz_rw_add(&journal, gz_rw_union(saved.allocations, current.allocations, GZ_RW_ALLOCATIONS), 84) ||
        !gz_rw_add(&journal, gz_rw_union(saved.processes, current.processes, GZ_RW_PROCESSES), 76) ||
        !gz_rw_add(&journal, classes, 1)) return GZ_RW_OVERFLOW;
    while (hash < saved.allocations) hash <<= 1;
    hash <<= 1;
    marks = GZ_RW_HEAPS * 8u + hash * 4u +
            (GZ_RW_ALLOCATIONS + GZ_RW_HEAPS + GZ_RW_PROCESSES) / 8u;
    if (!gz_rw_add(&apply, saved.heaps, 132) ||
        !gz_rw_add(&apply, saved.allocations, 72) ||
        !gz_rw_add(&apply, saved.processes, 64) ||
        !gz_rw_add(&apply, marks, 1) || journal > GZ_RW_CAPACITY_LIMIT - apply)
        return GZ_RW_OVERFLOW;
    workspace = journal + apply;
    *out = (GzRestoreWorkspace){journal, apply, workspace, 0, 0, 0};
    return GZ_RW_OK;
}

/* Exact post-capture need for a known complete wire image/bound. Reservation
 * before capture uses the separate clamping-aware policy below.
 */
static inline uint32_t gz_restore_workspace_plan(GzRestoreCounts saved,
    GzRestoreCounts current, uint32_t wire_bound, GzRestoreWorkspace *out)
{
    GzRestoreWorkspace metadata;
    uint32_t result, page_offset, workspace_offset, capacity;
    if (!out) return GZ_RW_ARGUMENT;
    *out = (GzRestoreWorkspace){0};
    if (!wire_bound || wire_bound > GZ_RW_CAPACITY_LIMIT) return GZ_RW_ARGUMENT;
    result = gz_rw_metadata_plan(saved, current, &metadata);
    if (result != GZ_RW_OK) return result;
    if (wire_bound > 0xffffffe0u) return GZ_RW_OVERFLOW;
    page_offset = (wire_bound + 31u) & ~31u;
    if (page_offset > GZ_RW_CAPACITY_LIMIT - GZ_RW_PAGE_BYTES) return GZ_RW_OVERFLOW;
    workspace_offset = page_offset + GZ_RW_PAGE_BYTES;
    if (metadata.workspace_bytes > GZ_RW_CAPACITY_LIMIT - workspace_offset) return GZ_RW_OVERFLOW;
    capacity = workspace_offset + metadata.workspace_bytes;
    *out = (GzRestoreWorkspace){metadata.journal_bytes, metadata.apply_bytes, metadata.workspace_bytes,
                              page_offset, workspace_offset, capacity};
    return GZ_RW_OK;
}

/* Reserve metadata AFTER the capture's own bounded writing region. The SDK
 * may still clamp that region; the genuine encoder then decides if its data
 * fits. Complete metadata work never competes with the compressed output.
 * available_budget is the original CURRENT SDK policy's result, not a lease.
 */
static inline uint32_t gz_restore_reservation_plan(GzRestoreCounts saved,
    GzRestoreCounts current, uint32_t capture_bound, uint32_t raw_bytes,
    uint32_t available_budget, GzRestoreReservation *out)
{
    GzRestoreWorkspace metadata;
    uint32_t result, tail, minimum, overhead, pages, wanted, allocation;
    if (!out) return GZ_RW_ARGUMENT;
    *out = (GzRestoreReservation){0};
    if (!capture_bound || capture_bound > GZ_RW_CAPACITY_LIMIT || !raw_bytes ||
        (raw_bytes & 3u) || raw_bytes >= capture_bound) return GZ_RW_ARGUMENT;
    result = gz_rw_metadata_plan(saved, current, &metadata);
    if (result != GZ_RW_OK) return result;
    if (metadata.workspace_bytes > GZ_RW_CAPACITY_LIMIT - GZ_RW_PAGE_BYTES) return GZ_RW_OVERFLOW;
    tail = GZ_RW_PAGE_BYTES + metadata.workspace_bytes;
    pages = ((raw_bytes - 1u) >> 16) + 1u;
    overhead = 64u + pages * 32u;
    minimum = capture_bound - raw_bytes;
    /* The actual v154 _gz_ghc_capture entry guard adds this packet allowance
     * AFTER proposal.capacity - raw. Preserve that exact published guard,
     * even though the original capacity already contains encoder overhead.
     */
    if (minimum > GZ_RW_CAPACITY_LIMIT - overhead ||
        minimum + overhead > GZ_RW_CAPACITY_LIMIT - 31u) return GZ_RW_OVERFLOW;
    minimum = (minimum + overhead + 31u) & ~31u;
    if (minimum > GZ_RW_CAPACITY_LIMIT - tail) return GZ_RW_OVERFLOW;
    wanted = (capture_bound + 31u) & ~31u;
    wanted = wanted > GZ_RW_CAPACITY_LIMIT - tail ? GZ_RW_CAPACITY_LIMIT : wanted + tail;
    allocation = available_budget > GZ_RW_CAPACITY_LIMIT ? GZ_RW_CAPACITY_LIMIT : available_budget & ~31u;
    if (allocation > wanted) allocation = wanted;
    if (allocation < minimum + tail) return GZ_RW_SPACE;
    *out = (GzRestoreReservation){metadata.workspace_bytes, tail, minimum,
                                wanted, allocation, allocation - tail};
    return GZ_RW_OK;
}
#endif
