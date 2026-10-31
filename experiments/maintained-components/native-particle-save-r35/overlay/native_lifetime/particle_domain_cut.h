#ifndef GZ_PARTICLE_DOMAIN_CUT_H
#define GZ_PARTICLE_DOMAIN_CUT_H
#include "object_binding.h"
#include "owned_heap_save.h"
#include <stddef.h>

typedef unsigned int JpcU;
enum {
    JPC_MAGIC = 0x4a504331u, JPC_VERSION = 2, JPC_HEADER = 128,
    JPC_PRODUCER = 128, JPC_LEDGER = 192, JPC_LEDGER_BYTES = 10880,
    JPC_POOLS = 11072, JPC_POOL_BYTES = 256,
    JPC_AUX_POOLS = 11328, JPC_AUX_POOL_BYTES = 256, JPC_TABLE = 11584,
    /* Eight managers and three pools each, one control allocation and two
     * deferred arrays for each of the 256 ledger rows: at most 545 owners. */
    JPC_ROW = 96, JPC_REGIONS = 545,
    JPC_MAX_ENCODED_BYTES = 0x400000, JPC_MAX_BYTES = JPC_MAX_ENCODED_BYTES,
    JPC_MAX_RAW_BYTES = 0x4000000, JPC_MAX_AUX_COUNT = 200, JPC_MAX_PARTICLES = 3000,
    JPC_MANAGER = 1, JPC_EMITTERS = 2, JPC_PARTICLES = 4,
    JPC_DEFERRED = 8, JPC_CONTROL = 16, JPC_AUX = 32, JPC_ALL_ROLES = 63,
    JPC_COVERED_BY_HEAP = 1,
    JPC_OK = 0, JPC_SCOPE = 2920, JPC_CHANGED = 2921,
    JPC_FORMAT = 2922, JPC_SPACE = 2923, JPC_LIFETIME = 2924
};

typedef struct {
    JpcU magic, bytes, version, valid, missing, regions, table, data;
    JpcU checksum, request, frame, root, root_generation;
    JpcU control, control_manager, random_seed, raw_start, raw_bytes;
    /* Sum of unique external whole-allocation bytes. Main-heap-covered rows
     * are already transported by the outer AS11 heap and contribute zero. */
    JpcU raw_total, particle_count[8], reserved[5];
} JpcHeader;

typedef struct {
    JpcU key, roles, managers, raw_bytes, offset, packed_bytes, checksum, flags;
    ObBinding allocation;
} JpcRegion;

typedef char JpcHeader128[sizeof(JpcHeader) == JPC_HEADER ? 1 : -1];
typedef char JpcRawTotal72[offsetof(JpcHeader, raw_total) == 72 ? 1 : -1];
typedef char JpcParticleCount76[offsetof(JpcHeader, particle_count) == 76 ? 1 : -1];
typedef char JpcReserved108[offsetof(JpcHeader, reserved) == 108 ? 1 : -1];
typedef char JpcRegion96[sizeof(JpcRegion) == JPC_ROW ? 1 : -1];

/* PRIVATE byte validation only. Captured pointers and generations are audit
 * identities. Neither this codec nor a valid packet grants native access.
 * Full allocation payloads include unused/active/child pool slots and array
 * cookies. Resource/device closure and lifetime adoption remain missing255. */
JpcU _gz_jpc_checksum(const void *, JpcU);
JpcU _gz_jpc_validate(const void *, JpcU, const GhcSaveProducer *,
                    JpcU root, JpcU generation, JpcU raw_start, JpcU raw_bytes);
/* Same PRIVATE validator checks; 0 means valid, otherwise a frozen reject
 * ordinal. It does not replace the ordinary 0/JPC_FORMAT result. */
JpcU _gz_jpc_diagnostic(const void *, JpcU, const GhcSaveProducer *,
                    JpcU root, JpcU generation, JpcU raw_start, JpcU raw_bytes);
#endif
