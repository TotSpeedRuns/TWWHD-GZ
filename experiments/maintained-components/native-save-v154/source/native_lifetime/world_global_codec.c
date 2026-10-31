#include "world_global_cut.h"
#include <stdint.h>

static WgcU equal(const void *a, const void *b, WgcU bytes) {
    const unsigned char *left = a, *right = b;
    WgcU i;
    for (i = 0; i < bytes; ++i) if (left[i] != right[i]) return 0;
    return 1;
}

WgcU _gz_wgc_checksum(const void *image, WgcU bytes) {
    const unsigned char *data = image;
    WgcU i, hash = 2166136261u;
    if (!image || bytes != WGC_BYTES) return 0;
    for (i = 0; i < bytes; ++i) {
        WgcU byte = ((i >= 12 && i < 16) || (i >= 32 && i < 36)) ? 0 : data[i];
        hash = (hash ^ byte) * 16777619u;
    }
    return hash;
}

WgcU _gz_wgc_validate(const void *image, WgcU bytes, const GhcSaveProducer *producer,
                     WgcU root, WgcU root_generation) {
    const unsigned char *data = image;
    const WgcHeader *header = image;
    const WgcRecord *records;
    const FgcHeader *fpc;
    const PgcHeader *pose;
    const NgcHeader *context;
    WgcU i;
    if (!image || !producer || ((uintptr_t)image & 3u) || ((uintptr_t)producer & 3u) ||
        bytes != WGC_BYTES || !root || !root_generation ||
        header->magic != WGC_MAGIC || header->bytes != WGC_BYTES ||
        header->version != WGC_VERSION || header->valid != 1 || header->missing != 255 ||
        header->count != WGC_COUNT || header->table != WGC_TABLE || header->data != WGC_DATA ||
        header->root != root || header->root_generation != root_generation ||
        header->request != producer->request || header->frame != producer->frame ||
        header->scene_hash != producer->scene_hash || header->flags != WGC_ROOTS_ONLY ||
        header->reserved) return WGC_FORMAT;
    records = (const WgcRecord *)(data + WGC_TABLE);
    for (i = 0; i < WGC_COUNT; ++i) {
        const WgcRecord *record = records + i;
        WgcU offset = i == 0 ? WGC_FPC_OFFSET : (i == 1 ? WGC_POSE_OFFSET : WGC_CONTEXT_OFFSET);
        WgcU size = i == 0 ? FGC_BYTES : (i == 1 ? PGC_BYTES : NGC_BYTES);
        if (record->type != (WgcU)(i == 0 ? WGC_FPC : (i == 1 ? WGC_POSE : WGC_CONTEXT)) || record->version != 1 ||
            record->offset != offset || record->bytes != size || record->flags != WGC_ROOTS_ONLY ||
            record->reserved[0] || record->reserved[1]) return WGC_FORMAT;
    }
    fpc = (const FgcHeader *)(data + WGC_FPC_OFFSET);
    pose = (const PgcHeader *)(data + WGC_POSE_OFFSET);
    context = (const NgcHeader *)(data + WGC_CONTEXT_OFFSET);
    if (_gz_fgc_validate(fpc, FGC_BYTES) || _gz_pgc_validate(pose, PGC_BYTES) || _gz_ngc_validate(context, NGC_BYTES) ||
        !equal(&fpc->producer, producer, sizeof(*producer)) ||
        !equal(&pose->producer, producer, sizeof(*producer)) ||
        !equal(&context->producer, producer, sizeof(*producer)) ||
        records[2].checksum != context->checksum ||
        records[0].checksum != fpc->checksum || records[1].checksum != pose->checksum)
        return WGC_FORMAT;
    return header->checksum == _gz_wgc_checksum(image, bytes) ? WGC_OK : WGC_FORMAT;
}
