#include "world_global_cut.h"
#include "owned_heap_native.h"
#include "owned_heap_process.h"
#include "owned_heap_common_domain.h"

/* PRIVATE parent offsets only; never follow a captured native pointer. */
static WgcU bank_offset(const GhcHeader *parent, WgcU bytes) {
    const unsigned char *p=(const unsigned char*)parent;
    const GhcProcessDirectory *pd;
    const GhcAllocationDirectory *d;
    WgcU at;
    if(!parent||bytes<GHC_FIXED||!parent->count||parent->count>HL_HEAPS||
       parent->reserved[0]>bytes||sizeof(*pd)>bytes-parent->reserved[0])return 0;
    pd=(const GhcProcessDirectory*)(p+parent->reserved[0]);
    if(pd->count>GHC_PROCESS_CAP)return 0;
    at=ghc_allocation_offset(parent->count,pd->count);
    if(!at||at>bytes||sizeof(*d)>bytes-at)return 0;
    d=(const GhcAllocationDirectory*)(p+at);
    if(d->magic!=GHC_ALLOC_MAGIC||d->count>HL_ALLOCS||
       d->bank_offset!=ghc_common_legacy_data(parent->count,pd->count,d->count)||
       d->bank_bytes<WGC_BYTES||d->bank_bytes>WGC_BYTES+JPC_MAX_BYTES||
       d->bank_offset>bytes||d->bank_bytes>bytes-d->bank_offset||
       parent->data!=d->bank_offset+d->bank_bytes)return 0;
    return d->bank_offset;
}

/* Derive the sole side-bank receiver from the real parent's counted data
 * offset. This reads no archived pointer and accepts no caller-chosen bank. */
static WgcHeader *receiver(WgcU request, WgcU frame, const GhcHeader *parent) {
    WgcU address = (WgcU)parent, bank;
    if (!parent || (address & 3u) || address < 0x10000000u ||
        address > 0xbffe0000u - sizeof(*parent) ||
        !_gz_ghs_scope(request, frame, 2) ||
        !_gz_ois_heap_stage_match(request, frame, address, sizeof(*parent))) return 0;
    if (parent->magic != GHC_MAGIC || parent->version != GHC_PROCESS_VERSION || parent->valid ||
        parent->diagnostic != 1 || parent->missing != 255 || parent->root_mask != 7 ||
        parent->request != request || parent->frame != frame ||
        parent->hg != _gz_ghs_hg(request, frame) || !parent->domain.heap ||
        !parent->domain.heap_generation || parent->data < GHC_FIXED + WGC_BYTES ||
        parent->data > 0xbffe0000u - address) return 0;
    bank = bank_offset(parent,parent->bytes);if(!bank)return 0;bank+=address;
    if (!_gz_ois_heap_stage_match(request, frame, bank, WGC_BYTES) ||
        !_gz_ois_heap_stage_match(request, frame, address + 128, sizeof(GhcSaveProducer))) return 0;
    return (WgcHeader *)bank;
}

WgcU _gz_wgc_capture(WgcU request, WgcU frame, const void *image) {
    const GhcHeader *parent = image;
    const GhcSaveProducer *producer;
    WgcHeader *header = receiver(request, frame, parent);
    WgcRecord *records;
    unsigned char *data = (unsigned char *)header;
    WgcU i;
    if (!header) return WGC_SCOPE;
    producer = (const GhcSaveProducer *)((const unsigned char *)parent + 128);
    for (i = 0; i < WGC_BYTES; ++i) data[i] = 0;
    header->magic = WGC_MAGIC; header->bytes = WGC_BYTES; header->version = WGC_VERSION;
    header->missing = 255; header->count = WGC_COUNT; header->table = WGC_TABLE;
    header->data = WGC_DATA; header->root = parent->domain.heap;
    header->root_generation = parent->domain.heap_generation; header->request = request;
    header->frame = frame; header->scene_hash = producer->scene_hash; header->flags = WGC_ROOTS_ONLY;
    if (_gz_fgc_capture(request, frame, data + WGC_FPC_OFFSET, FGC_BYTES) ||
        _gz_pgc_capture(request, frame, data + WGC_POSE_OFFSET, PGC_BYTES) ||
        _gz_ngc_capture(request, frame, data + WGC_CONTEXT_OFFSET, NGC_BYTES)) return WGC_CHANGED;
    records = (WgcRecord *)(data + WGC_TABLE);
    for (i = 0; i < WGC_COUNT; ++i) {
        records[i].type = i == 0 ? WGC_FPC : (i == 1 ? WGC_POSE : WGC_CONTEXT); records[i].version = 1;
        records[i].offset = i == 0 ? WGC_FPC_OFFSET : (i == 1 ? WGC_POSE_OFFSET : WGC_CONTEXT_OFFSET);
        records[i].bytes = i == 0 ? FGC_BYTES : (i == 1 ? PGC_BYTES : NGC_BYTES); records[i].flags = WGC_ROOTS_ONLY;
    }
    records[0].checksum = ((const FgcHeader *)(data + WGC_FPC_OFFSET))->checksum;
    records[1].checksum = ((const PgcHeader *)(data + WGC_POSE_OFFSET))->checksum;
    records[2].checksum = ((const NgcHeader *)(data + WGC_CONTEXT_OFFSET))->checksum;
    if (receiver(request, frame, parent) != header) return WGC_SCOPE;
    header->checksum = _gz_wgc_checksum(header, WGC_BYTES);
    __sync_synchronize(); header->valid = 1;
    if (_gz_wgc_validate(header, WGC_BYTES, producer, parent->domain.heap,
                         parent->domain.heap_generation)) { header->valid = 0; return WGC_FORMAT; }
    return WGC_OK;
}

WgcU _gz_wgc_verify_held(WgcU request, WgcU frame, const void *image) {
    const GhcHeader *parent = image;
    WgcHeader *header = receiver(request, frame, parent);
    const unsigned char *data = (const unsigned char *)header;
    const GhcSaveProducer *producer;
    if (!header) return WGC_SCOPE;
    producer = (const GhcSaveProducer *)((const unsigned char *)parent + 128);
    if (_gz_wgc_validate(header, WGC_BYTES, producer, parent->domain.heap,
                         parent->domain.heap_generation)) return WGC_FORMAT;
    if (_gz_fgc_verify_held(request, frame, data + WGC_FPC_OFFSET, FGC_BYTES) ||
        _gz_pgc_verify_held(request, frame, data + WGC_POSE_OFFSET, PGC_BYTES) ||
        _gz_ngc_verify_held(request, frame, data + WGC_CONTEXT_OFFSET, NGC_BYTES)) return WGC_CHANGED;
    return receiver(request, frame, parent) == header ? WGC_OK : WGC_SCOPE;
}

#include "owned_heap_held_view.h"
#include "held_heap_roundtrip.h"

/* Read only one actual OIS-owned parent. A historical producer identifies
 * saved DATA; it never supplies this operation's current HG/job permission.
 * The fixed CURRENT producer alone enrolls the native destination domain.
 */
const WgcHeader *_gz_wgc_held_source(WgcU request, WgcU frame, WgcU phase) {
    const OisHeldView *view = &_gz_ois_held_record;
    const GhcHeader *current, *parent;
    const GhcSaveProducer *producer;
    const WgcHeader *bank;
    WgcU address, bytes, valid, saved;
    if (!_gz_ghw_scope(request, frame, phase) ||
        !_gz_ois_held_owner(request, frame, view)) return 0;
    current = (const GhcHeader *)view->current.archive;
    if (!_gz_ghc_held_domain(request, frame, current->domain.start, current->domain.span)) return 0;
    saved = phase == GHW_APPLY || phase == GHW_VERIFY;
    parent = (const GhcHeader *)(saved ? view->saved.archive : view->current.archive);
    bytes = saved ? view->saved.bytes : view->current.bytes;
    valid = saved ? 1u : 0u;
    address = (WgcU)parent;
    if (bytes < GHC_FIXED + WGC_BYTES || bytes > 0xbffe0000u - address ||
        parent->magic != GHC_MAGIC || parent->version != GHC_PROCESS_VERSION ||
        parent->bytes != bytes || parent->valid != valid || parent->diagnostic != 1 ||
        parent->missing != 255 || parent->root_mask != 7 ||
        parent->data < GHC_FIXED + WGC_BYTES || parent->data > bytes) return 0;
    producer = (const GhcSaveProducer *)(address + 128u);
    {WgcU offset=bank_offset(parent,bytes);if(!offset)return 0;bank=(const WgcHeader*)(address+offset);}
    if (_gz_wgc_validate(bank, WGC_BYTES, producer, parent->domain.heap,
            parent->domain.heap_generation) || !_gz_ghw_scope(request, frame, phase) ||
        !_gz_ois_held_owner(request, frame, view)) return 0;
    return bank;
}

WgcU _gz_wgc_held_step(WgcU request, WgcU frame, WgcU phase, WgcU write) {
    if (write > 1 || !_gz_ghw_scope(request, frame, phase) ||
        (write && phase != GHW_APPLY && phase != GHW_INVERSE) ||
        !_gz_wgc_held_source(request, frame, phase)) return WGC_SCOPE;
    if (_gz_fgc_held_step(request, frame, phase, write) ||
        _gz_pgc_held_step(request, frame, phase, write) ||
        _gz_ngc_held_step(request, frame, phase, write)) return WGC_CHANGED;
    return _gz_ghw_scope(request, frame, phase) &&
        _gz_wgc_held_source(request, frame, phase) ? WGC_OK : WGC_SCOPE;
}
