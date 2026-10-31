#include "native_context_cut.h"
#include "world_global_cut.h"
#include "owned_heap_held_view.h"
#include "held_heap_roundtrip.h"

extern const unsigned char _native_game_literal_1046f0b0[];
extern const unsigned char _native_game_literal_104753a8[];
extern NgcU _gz_ois_heap_stage_match(NgcU, NgcU, NgcU, NgcU);
extern NgcU _gz_ois_heap_apart(NgcU, NgcU, NgcU, NgcU);
extern NgcU _gz_rsa_private_apart(NgcU, NgcU), _gz_ghc_all_private_apart(NgcU, NgcU);

static NgcU initialized(void) {
    return *(const volatile NgcU *)_native_game_literal_104753a8 == 1;
}

static NgcU scope(NgcU request, NgcU frame, NgcU hg, NgcU job) {
    return hg && job && _gz_ghs_scope(request, frame, 2) == 1 &&
        _gz_ghs_hg(request, frame) == hg && _gz_jwa_ghs_check(request) == 0 &&
        _gz_jwa_ghs_generation(request) == job && _gz_jwa_ghs_workers_zero(request) != 2;
}

static NgcU apart(NgcU request, NgcU frame, NgcU held) {
    NgcU address = (NgcU)_native_game_literal_1046f0b0;
    NgcU bytes = NGC_PAYLOAD_BYTES + sizeof(NgcU);
    if (address < 0x10000000u || address >= 0xbffe0000u || bytes > 0xbffe0000u - address ||
        (NgcU)_native_game_literal_104753a8 != address + NGC_PAYLOAD_BYTES ||
        !_gz_rsa_private_apart(address, bytes) || !_gz_ghc_all_private_apart(address, bytes)) return 0;
    return held ? _gz_ois_held_apart(request, frame, &_gz_ois_held_record, address, bytes) :
        _gz_ois_heap_apart(request, frame, address, bytes);
}

static NgcU matches(const void *image) {
    const NgcU *source = (const NgcU *)((const unsigned char *)image + NGC_DATA);
    const volatile NgcU *native = (const volatile NgcU *)_native_game_literal_1046f0b0;
    NgcU i;
    for (i = 0; i < NGC_PAYLOAD_BYTES / 4; ++i)
        if (source[i] != native[i]) return 0;
    return initialized();
}

NgcU _gz_ngc_capture(NgcU request, NgcU frame, void *output, NgcU capacity) {
    NgcHeader *header = output;
    NgcU *words = output, i, address = (NgcU)output, hg, job;
    const volatile NgcU *native = (const volatile NgcU *)_native_game_literal_1046f0b0;
    if (!output || (address & 3u) || address < 0x10000000u || address >= 0xbffe0000u ||
        NGC_BYTES > 0xbffe0000u - address) return NGC_ARGUMENT;
    if (capacity < NGC_BYTES) return NGC_SPACE;
    if (!_gz_ois_heap_stage_match(request, frame, address, NGC_BYTES)) return NGC_SCOPE;
    hg = _gz_ghs_hg(request, frame); job = _gz_jwa_ghs_generation(request);
    if (!scope(request, frame, hg, job)) return NGC_SCOPE;
    if (!apart(request, frame, 0)) return NGC_PROTECTED;
    if (!initialized()) return NGC_CHANGED;
    for (i = 0; i < NGC_BYTES / 4; ++i) words[i] = 0;
    header->magic = NGC_MAGIC; header->bytes = NGC_BYTES; header->version = NGC_VERSION;
    header->missing = 255; header->address = NGC_ADDRESS; header->data = NGC_DATA;
    header->payload_bytes = NGC_PAYLOAD_BYTES; header->flags = NGC_ROOTS_ONLY; header->initialized = 1;
    if (_gz_ghs_source(request, frame, &header->producer) ||
        header->producer.hg != hg || header->producer.job_generation != job) return NGC_SCOPE;
    for (i = 0; i < NGC_PAYLOAD_BYTES / 4; ++i) words[NGC_DATA / 4 + i] = native[i];
    if (!matches(output) || !apart(request, frame, 0) || !scope(request, frame, hg, job) ||
        !_gz_ois_heap_stage_match(request, frame, address, NGC_BYTES)) return NGC_CHANGED;
    header->checksum = _gz_ngc_checksum(output, NGC_BYTES);
    __sync_synchronize(); header->valid = 1;
    if (_gz_ngc_validate(output, NGC_BYTES)) { header->valid = 0; return NGC_FORMAT; }
    return NGC_OK;
}

NgcU _gz_ngc_verify_held(NgcU request, NgcU frame, const void *image, NgcU bytes) {
    const NgcHeader *header = image;
    NgcU hg, job;
    if (!image || bytes != NGC_BYTES ||
        !_gz_ois_heap_stage_match(request, frame, (NgcU)image, bytes)) return NGC_SCOPE;
    if (_gz_ngc_validate(image, bytes)) return NGC_FORMAT;
    hg = _gz_ghs_hg(request, frame); job = _gz_jwa_ghs_generation(request);
    if (!scope(request, frame, hg, job) || header->producer.request != request ||
        header->producer.frame != frame || header->producer.hg != hg ||
        header->producer.job_generation != job ||
        header->producer.reserved != _gz_ghs_role(request, frame) ||
        header->producer.slot + 1 != _gz_ghs_target(request, frame)) return NGC_SCOPE;
    if (!apart(request, frame, 0)) return NGC_PROTECTED;
    return matches(image) && scope(request, frame, hg, job) &&
        _gz_ois_heap_stage_match(request, frame, (NgcU)image, bytes) &&
        apart(request, frame, 0) ? NGC_OK : NGC_CHANGED;
}

NgcU _gz_ngc_held_step(NgcU request, NgcU frame, NgcU phase, NgcU write) {
    const WgcHeader *bank;
    const unsigned char *image;
    const NgcU *source;
    volatile NgcU *native = (volatile NgcU *)_native_game_literal_1046f0b0;
    NgcU i;
    if (write > 1 || (write && phase != GHW_APPLY && phase != GHW_INVERSE) ||
        !_gz_ghw_scope(request, frame, phase)) return NGC_SCOPE;
    bank = _gz_wgc_held_source(request, frame, phase);
    if (!bank) return NGC_SCOPE;
    image = (const unsigned char *)bank + WGC_CONTEXT_OFFSET;
    if (_gz_ngc_validate(image, NGC_BYTES)) return NGC_FORMAT;
    if (!apart(request, frame, 1)) return NGC_PROTECTED;
    if (!initialized()) return NGC_CHANGED;
    source = (const NgcU *)(image + NGC_DATA);
    for (i = 0; i < NGC_PAYLOAD_BYTES / 4; ++i) {
        if (write) native[i] = source[i];
        else if (native[i] != source[i]) return NGC_CHANGED;
    }
    if (write) _gz_ghw_report[phase == GHW_APPLY ? 22 : 23] += NGC_PAYLOAD_BYTES;
    __sync_synchronize();
    return initialized() && _gz_ghw_scope(request, frame, phase) ? NGC_OK : NGC_SCOPE;
}
