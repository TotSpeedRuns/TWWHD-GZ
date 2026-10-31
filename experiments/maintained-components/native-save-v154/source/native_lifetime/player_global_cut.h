#ifndef GZ_PLAYER_GLOBAL_CUT_H
#define GZ_PLAYER_GLOBAL_CUT_H

#include "owned_heap_save.h"

typedef unsigned int PgcU;
enum {
    PGC_MAGIC = 0x50474331u, PGC_VERSION = 1,
    PGC_BYTES = 192, PGC_HEADER = 96, PGC_COUNT = 3,
    PGC_DATA = 144, PGC_PAYLOAD_BYTES = 24, PGC_ROOTS_ONLY = 1,
    PGC_OK = 0, PGC_ARGUMENT = 1, PGC_SCOPE = 2,
    PGC_SPACE = 3, PGC_CHANGED = 4, PGC_FORMAT = 5, PGC_PROTECTED = 6
};

typedef struct {
    PgcU magic, bytes, version, valid, missing, count, flags, checksum;
    GhcSaveProducer producer;
} PgcHeader;

typedef struct { PgcU id, address, bytes, data; } PgcRecord;
typedef char PgcHeader96[sizeof(PgcHeader) == 96 ? 1 : -1];
typedef char PgcRecord16[sizeof(PgcRecord) == 16 ? 1 : -1];
extern const PgcRecord _gz_pgc_layout[PGC_COUNT];

/* The three keys identify native position/angle globals proved by original
 * Link Execute instructions. Keys and captured values are never followed by
 * these pure validators. Capture resolves separate real ELF DATA aliases.
 * No function here restores globals or grants world-writer permission. */
PgcU _gz_pgc_checksum(const void *image, PgcU bytes);
PgcU _gz_pgc_validate(const void *image, PgcU bytes);
PgcU _gz_pgc_capture(PgcU request, PgcU frame, void *output, PgcU capacity);
PgcU _gz_pgc_verify_held(PgcU request, PgcU frame, const void *image, PgcU bytes);

#endif
