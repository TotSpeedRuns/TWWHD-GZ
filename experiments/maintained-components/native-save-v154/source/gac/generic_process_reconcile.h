#ifndef GZ_GENERIC_PROCESS_RECONCILE_H
#define GZ_GENERIC_PROCESS_RECONCILE_H

#include "process.h"
#include "lifetime.h"
#include "owned_heap_save.h"
#include "owned_heap_process.h"

/* Scheduling metadata only. No function below calls a game API, follows a
 * recorded native pointer, establishes a hold, or authorizes actor writes.
 * The native consumer must obtain both views from its real owned cut. */
enum {
    GRC_MAGIC = 0x47524331u, GRC_VERSION = 1, GRC_LIMIT = PO_CAP,
    GRC_OK = 0, GRC_ARGUMENT = 1, GRC_FORMAT = 2, GRC_STALE = 3,
    GRC_DOMAIN_CHANGED = 4, GRC_CAPACITY = 5, GRC_AMBIGUOUS = 6,
    GRC_UNRESOLVED = 7, GRC_IDENTITY_CONFLICT = 8, GRC_CAPTURE_ROLE = 9,
    GRC_BIND_NONE = GHCP_BIND_NONE, GRC_BIND_LIVE = GHCP_BIND_LIVE,
    GRC_BIND_OUTSIDE = GHCP_BIND_OUTSIDE,
    GRC_MATCHING = 1, GRC_CURRENT_ONLY = 2, GRC_SAVED_ONLY = 3,
    GRC_REUSED_ADDRESS = 4, GRC_TOMBSTONE = 5,
    GRC_OUTSIDE_DOMAIN = 6, GRC_NOT_RESOLVED = 7,
    GRC_MISSING_ALL = 255
};
#define GRC_NO_ROW 0xffffffffu

/* All four identity records use the actual selected observer declarations.
 * A missing binding has zero allocation/owner/scope; it is not a wildcard.
 * The indices are private registry indices, never native addresses. */
typedef GhcProcessRow GrcProcess;
typedef char GrcProcess256[sizeof(GrcProcess) == 256 ? 1 : -1];

/* Native adapter projection, not a new wire format or certificate. The image
 * and census must already have been independently validated and joined. Old
 * AS v5 images contain no process census and cannot supply this structure.
 * Full snapshot tags are compared before emitting a valid plan. Hash zero is
 * valid. Addresses/labels are opaque numerical keys and are never followed. */
typedef struct {
    GhcSaveProducer producer;
    GhcSaveScene scene;
    HlHeap selected_domain, sdk_domain;
    PoHeader processes;
    HlControl allocations;
    u32 archive_identity, archive_generation;
    u32 image_type, image_version, image_bytes, image_checksum, image_plan_checksum;
    u32 row_count, row_checksum, missing, census_flags, reserved;
} GrcCutStamp;
typedef char GrcStamp400[sizeof(GrcCutStamp) == 400 ? 1 : -1];

typedef struct {
    const GrcCutStamp *stamp;
    const GhcProcessDirectory *directory;
    const GrcProcess *rows;
    u32 rows_bytes, directory_offset;
} GrcCutView;

typedef struct {
    u32 saved_index, current_index, classification, reserved;
} GrcPlanRow;
typedef struct {
    u32 magic, bytes, version, valid, missing, status;
    u32 count, matching, current_only, saved_only, reused, tombstones;
    u32 outside, unresolved, reserved[2];
    GrcCutStamp saved, current;
    GhcProcessDirectory saved_directory, current_directory;
} GrcPlan;

/* Caller supplies bounded private output/workspace, apart from both input
 * views. Work has one u32 index per input row; no allocator or global BSS.
 * expected_current is the consumer's independently pinned current cut tag,
 * not a frame/report used as permission. The native caller must recheck its
 * real Source/purpose/leases and both cut identities before acting on a plan.
 * Success returns classifications only, with missing255 permanently set.
 * GRC_UNRESOLVED may return a valid diagnostic plan containing unbound LIVE
 * or ENDING rows. It is never a retire/create/replay authorization. */
u32 grc_plan(const GrcCutView *saved, const GrcCutView *current,
             const GrcCutStamp *expected_current,
             u32 *work, u32 work_words,
             GrcPlan *plan, GrcPlanRow *rows, u32 row_capacity);
/* Explicit legacy/offline inspection only. It cannot claim a fresh native
 * Restore cut. The ordinary grc_plan API below now requires HSPv2 Save1 as
 * saved and a distinct, later, genuine Restore2 current cut. */
u32 grc_plan_diagnostic(const GrcCutView *saved, const GrcCutView *current,
                        const GrcCutStamp *expected_current,
                        u32 *work, u32 work_words,
                        GrcPlan *plan, GrcPlanRow *rows, u32 row_capacity);
u32 grc_row_checksum(const GrcProcess *rows, u32 count);
u32 grc_validate_cut(const GrcCutView *view);
/* Private AS6 parser/projection. It validates archive arithmetic and hashes;
 * it neither acquires an OIS pin nor makes an old archive a fresh world cut.
 * Native integration must retain and validate the real OIS read leases and
 * call the actual _gz_ghc_validate before/after this diagnostic operation. */
u32 grc_project_as8_private(const void *image, u32 bytes,
                            u32 archive_identity, u32 archive_generation,
                            GrcCutStamp *stamp, GrcCutView *view);

#endif
