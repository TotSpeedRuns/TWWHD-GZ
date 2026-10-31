#include "player_global_cut.h"

extern const unsigned char _native_game_literal_1046cd48[];
extern const unsigned char _native_game_literal_1046cd10[];
extern const unsigned char _native_game_literal_1046cd08[];
extern PgcU _gz_ois_heap_stage_match(PgcU, PgcU, PgcU, PgcU);
extern PgcU _gz_ois_heap_apart(PgcU, PgcU, PgcU, PgcU);
extern PgcU _gz_rsa_private_apart(PgcU, PgcU), _gz_ghc_all_private_apart(PgcU, PgcU);

static const unsigned char *const native_pose[PGC_COUNT] = {
    _native_game_literal_1046cd48,
    _native_game_literal_1046cd10,
    _native_game_literal_1046cd08
};

static PgcU buffer(PgcU address, PgcU bytes) {
    return address >= 0x10000000u && address < 0xbffe0000u &&
        bytes && bytes <= 0xbffe0000u - address;
}

static PgcU scope(PgcU request, PgcU frame, PgcU hg, PgcU job) {
    return hg && job && _gz_ghs_scope(request, frame, 2) == 1 &&
        _gz_ghs_hg(request, frame) == hg && _gz_jwa_ghs_check(request) == 0 &&
        _gz_jwa_ghs_generation(request) == job && _gz_jwa_ghs_workers_zero(request) != 2;
}

static PgcU sources_apart(PgcU request, PgcU frame) {
    PgcU i;
    for (i = 0; i < PGC_COUNT; ++i) {
        PgcU address = (PgcU)native_pose[i], bytes = _gz_pgc_layout[i].bytes;
        if (!buffer(address, bytes) || !_gz_rsa_private_apart(address, bytes) ||
            !_gz_ghc_all_private_apart(address, bytes) ||
            !_gz_ois_heap_apart(request, frame, address, bytes)) return 0;
    }
    return 1;
}

static PgcU matches(const void *image, PgcU request, PgcU frame, PgcU hg, PgcU job) {
    const unsigned char *saved = image;
    PgcU i, k;
    for (i = 0; i < PGC_COUNT; ++i) {
        const PgcRecord *layout = _gz_pgc_layout + i;
        const volatile unsigned char *native = native_pose[i];
        if (!scope(request, frame, hg, job)) return 0;
        for (k = 0; k < layout->bytes; ++k)
            if (saved[layout->data + k] != native[k]) return 0;
    }
    return 1;
}

PgcU _gz_pgc_capture(PgcU request, PgcU frame, void *output, PgcU capacity) {
    PgcHeader *header = output;
    PgcRecord *records;
    PgcU i, k, address = (PgcU)output, hg, job;
    if (!output || (address & 3u) || !buffer(address, PGC_BYTES)) return PGC_ARGUMENT;
    if (capacity < PGC_BYTES) return PGC_SPACE;
    if (!_gz_ois_heap_stage_match(request, frame, address, PGC_BYTES)) return PGC_SCOPE;
    hg = _gz_ghs_hg(request, frame); job = _gz_jwa_ghs_generation(request);
    if (!scope(request, frame, hg, job)) return PGC_SCOPE;
    if (!sources_apart(request, frame)) return PGC_PROTECTED;
    for (i = 0; i < PGC_BYTES; ++i) ((unsigned char *)output)[i] = 0;
    header->magic = PGC_MAGIC; header->bytes = PGC_BYTES; header->version = PGC_VERSION;
    header->missing = 255; header->count = PGC_COUNT; header->flags = PGC_ROOTS_ONLY;
    if (_gz_ghs_source(request, frame, &header->producer)) return PGC_SCOPE;
    if (header->producer.hg != hg || header->producer.job_generation != job) return PGC_SCOPE;
    records = (PgcRecord *)(address + PGC_HEADER);
    for (i = 0; i < PGC_COUNT; ++i) {
        const PgcRecord *layout = _gz_pgc_layout + i;
        if (!scope(request, frame, hg, job)) return PGC_SCOPE;
        records[i] = *layout;
        for (k = 0; k < layout->bytes; ++k)
            ((unsigned char *)output)[layout->data + k] = ((const volatile unsigned char *)native_pose[i])[k];
    }
    if (!matches(output, request, frame, hg, job) || !sources_apart(request, frame) ||
        !scope(request, frame, hg, job) ||
        !_gz_ois_heap_stage_match(request, frame, address, PGC_BYTES)) return PGC_CHANGED;
    header->checksum = _gz_pgc_checksum(output, PGC_BYTES);
    __sync_synchronize(); header->valid = 1;
    if (_gz_pgc_validate(output, PGC_BYTES)) { header->valid = 0; return PGC_FORMAT; }
    return PGC_OK;
}

PgcU _gz_pgc_verify_held(PgcU request, PgcU frame, const void *image, PgcU bytes) {
    const PgcHeader *header = image;
    PgcU hg, job;
    if (!image || bytes != PGC_BYTES || !_gz_ois_heap_stage_match(request, frame, (PgcU)image, bytes))
        return PGC_SCOPE;
    if (_gz_pgc_validate(image, bytes)) return PGC_FORMAT;
    hg = _gz_ghs_hg(request, frame); job = _gz_jwa_ghs_generation(request);
    if (!scope(request, frame, hg, job) || header->producer.request != request ||
        header->producer.frame != frame || header->producer.hg != hg ||
        header->producer.job_generation != job ||
        header->producer.reserved != _gz_ghs_role(request, frame) ||
        header->producer.slot + 1 != _gz_ghs_target(request, frame)) return PGC_SCOPE;
    if (!sources_apart(request, frame)) return PGC_PROTECTED;
    return matches(image, request, frame, hg, job) && scope(request, frame, hg, job) &&
        _gz_ois_heap_stage_match(request, frame, (PgcU)image, bytes) &&
        sources_apart(request, frame) ? PGC_OK : PGC_CHANGED;
}

#include "world_global_cut.h"
#include "owned_heap_held_view.h"
#include "held_heap_roundtrip.h"

PgcU _gz_pgc_held_step(PgcU request, PgcU frame, PgcU phase, PgcU write) {
    const WgcHeader *bank;
    const unsigned char *image;
    PgcU i, k, address, bytes;
    if (write > 1 || (write && phase != GHW_APPLY && phase != GHW_INVERSE) ||
        !_gz_ghw_scope(request, frame, phase)) return PGC_SCOPE;
    bank = _gz_wgc_held_source(request, frame, phase);
    if (!bank) return PGC_SCOPE;
    image = (const unsigned char *)bank + WGC_POSE_OFFSET;
    if (_gz_pgc_validate(image, PGC_BYTES)) return PGC_FORMAT;
    for (i = 0; i < PGC_COUNT; ++i) {
        address = (PgcU)native_pose[i]; bytes = _gz_pgc_layout[i].bytes;
        if (!buffer(address, bytes) || !_gz_rsa_private_apart(address, bytes) ||
            !_gz_ghc_all_private_apart(address, bytes) ||
            !_gz_ois_held_apart(request, frame, &_gz_ois_held_record, address, bytes)) return PGC_PROTECTED;
    }
    for (i = 0; i < PGC_COUNT; ++i) {
        const PgcRecord *layout = _gz_pgc_layout + i;
        volatile unsigned char *target = (volatile unsigned char *)native_pose[i];
        if (!_gz_ghw_scope(request, frame, phase)) return PGC_SCOPE;
        for (k = 0; k < layout->bytes; ++k) {
            if (write) target[k] = image[layout->data + k];
            else if (target[k] != image[layout->data + k]) return PGC_CHANGED;
        }
        if (write) _gz_ghw_report[phase == GHW_APPLY ? 22 : 23] += layout->bytes;
    }
    __sync_synchronize();
    return _gz_ghw_scope(request, frame, phase) ? PGC_OK : PGC_SCOPE;
}
