#include "player_global_cut.h"
#include <stdint.h>

const PgcRecord _gz_pgc_layout[PGC_COUNT] = {
    {1, 0x1046cd48u, 12, PGC_DATA},
    {2, 0x1046cd10u, 6, PGC_DATA + 12},
    {3, 0x1046cd08u, 6, PGC_DATA + 18}
};

PgcU _gz_pgc_checksum(const void *image, PgcU bytes) {
    const unsigned char *data = image;
    PgcU i, hash = 2166136261u;
    if (!image || bytes != PGC_BYTES) return 0;
    for (i = 0; i < bytes; ++i) {
        PgcU byte = ((i >= 12 && i < 16) || (i >= 28 && i < 32)) ? 0 : data[i];
        hash = (hash ^ byte) * 16777619u;
    }
    return hash;
}

PgcU _gz_pgc_validate(const void *image, PgcU bytes) {
    const PgcHeader *header = image;
    const PgcRecord *records;
    PgcU i;
    if (!image || ((uintptr_t)image & 3u) || bytes != PGC_BYTES ||
        header->magic != PGC_MAGIC || header->bytes != PGC_BYTES ||
        header->version != PGC_VERSION || header->valid != 1 ||
        header->missing != 255 || header->count != PGC_COUNT ||
        header->flags != PGC_ROOTS_ONLY || header->producer.version != 2 ||
        !ghs_provenance_format(&header->producer, header->producer.request,
            header->producer.frame, header->producer.hg, header->producer.lease,
            header->producer.original_return, header->producer.thread)) return PGC_FORMAT;
    records = (const PgcRecord *)((const unsigned char *)image + PGC_HEADER);
    for (i = 0; i < PGC_COUNT; ++i) {
        const PgcRecord *expected = _gz_pgc_layout + i;
        if (records[i].id != expected->id || records[i].address != expected->address ||
            records[i].bytes != expected->bytes || records[i].data != expected->data)
            return PGC_FORMAT;
    }
    for (i = PGC_DATA + PGC_PAYLOAD_BYTES; i < PGC_BYTES; ++i)
        if (((const unsigned char *)image)[i]) return PGC_FORMAT;
    return header->checksum == _gz_pgc_checksum(image, bytes) ? PGC_OK : PGC_FORMAT;
}
