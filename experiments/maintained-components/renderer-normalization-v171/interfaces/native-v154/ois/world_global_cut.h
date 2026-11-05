#ifndef GZ_WORLD_GLOBAL_CUT_H
#define GZ_WORLD_GLOBAL_CUT_H

#include "fpc_global_cut.h"
#include "player_global_cut.h"
#include "native_context_cut.h"

typedef unsigned int WgcU;
enum {
    WGC_MAGIC = 0x57474331u, WGC_VERSION = 2, WGC_BYTES = 27072,
    WGC_TABLE = 64, WGC_DATA = 160, WGC_COUNT = 3,
    WGC_FPC_OFFSET = 160, WGC_POSE_OFFSET = 1408, WGC_CONTEXT_OFFSET = 1600,
    WGC_FPC = 0x46504331u, WGC_POSE = 0x504f5345u, WGC_CONTEXT = 0x4e474331u, WGC_ROOTS_ONLY = 1,
    WGC_OK = 0, WGC_SCOPE = 2, WGC_SPACE = 3, WGC_CHANGED = 4, WGC_FORMAT = 5
};

typedef struct {
    WgcU magic, bytes, version, valid, missing, count, table, data;
    WgcU checksum, root, root_generation, request, frame, scene_hash, flags, reserved;
} WgcHeader;

typedef struct {
    WgcU type, version, offset, bytes, checksum, flags, reserved[2];
} WgcRecord;
typedef char WgcHeader64[sizeof(WgcHeader) == 64 ? 1 : -1];
typedef char WgcRecord32[sizeof(WgcRecord) == 32 ? 1 : -1];
typedef char WgcAllBanks27072[WGC_CONTEXT_OFFSET + NGC_BYTES == WGC_BYTES ? 1 : -1];

/* This is an explicit AS10 side bank. It retains the full FPC root cut and
 * the three player globals under the same genuine producer as the heap.
 * Includes the entire proved native game-context object. Dynamic references, audio/text and device state still
 * need closure. missing255 and the roots-only restriction remain explicit. */
WgcU _gz_wgc_checksum(const void *image, WgcU bytes);
WgcU _gz_wgc_validate(const void *image, WgcU bytes, const GhcSaveProducer *producer,
                     WgcU root, WgcU root_generation);
WgcU _gz_wgc_capture(WgcU request, WgcU frame, const void *parent);
WgcU _gz_wgc_verify_held(WgcU request, WgcU frame, const void *parent);

/* Private held CPU-bank replay; all missing/device obligations remain. */
const WgcHeader *_gz_wgc_held_source(WgcU request, WgcU frame, WgcU phase);
WgcU _gz_wgc_held_step(WgcU request, WgcU frame, WgcU phase, WgcU write);
FgcU _gz_fgc_held_step(FgcU request, FgcU frame, FgcU phase, FgcU write);
PgcU _gz_pgc_held_step(PgcU request, PgcU frame, PgcU phase, PgcU write);

#endif
