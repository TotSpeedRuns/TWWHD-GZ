/* Two linear passes over private metadata under ONE actual HL trylock.
 * First validate/count and reserve all remaining copy work/capacity. Only
 * then copy, without another mutable-source hash, check, failure or native
 * body read. No allocation-sized scratch and no per-allocation owner lookup.
 */
#include "object_directory.h"
#define OBD_FN static __attribute__((noinline, section(".text.gz_ob_directory")))
#define OBD_API __attribute__((noinline, section(".text.gz_ob_directory")))

OBD_FN u32 obd_span(u32 p, u32 n) {
    return !(p & 3) && p >= 0x10000000u && p < 0xbffe0000u &&
        n >= 64 && !(n & 15) && n <= OB_DIRECTORY_MAX_BYTES && n <= 0xbffe0000u - p;
}

OBD_FN u32 obd_reserve(ObTryWork *work, u32 units) {
    if (units > work->left) { work->exhausted = 1; return 0; }
    work->left -= units;
    return 1;
}

OBD_FN u32 obd_active(const HlHeap *h) {
    return h->heap && (h->state == HL_LIVE || h->state == HL_MUTATING || h->state == HL_ENDING);
}

OBD_FN u32 obd_private(u32 p, u32 n, ObTryWork *work) {
    u32 i;
    HlHeap *h;
    if (!hr_private(p, n, 0)) return 0;
    for (i = 0; i < HL_HEAPS; ++i) {
        if (!obt_take(work)) return 0;
        h = _gz_hl_heaps + i;
        if (obd_active(h) && (!hr_apart(p, n, h->start, h->bytes) ||
            !hr_apart(p, n, h->heap, header_bytes(h->kind)))) return 0;
    }
    return 1;
}

/* The index authenticates membership before any node dereference. Previous
 * links and a nil tail rule out repeated/cyclic rows. A row cannot belong to
 * two distinct heap lists because hi_member requires its exact owner slot.
 * Global counts must match both retained controls. No counter is modified.
 */
OBD_FN u32 obd_census(u32 *heaps, u32 *allocations, ObTryWork *work) {
    u32 i, j, node, previous, hc = 0, ac = 0;
    HlHeap *h;
    HiHeap *index;
    HlStoredAllocation *a;
    HiLink *link;
    for (i = 0; i < HL_HEAPS; ++i) {
        if (!obt_take(work)) return 0;
        h = _gz_hl_heaps + i;
        index = _gz_hi_heaps + i;
        if (!h->heap) {
            if (index->head || index->count || index->generation || index->reserved) return 0;
            continue;
        }
        if (!h->generation || h->kind < HL_EXP || h->kind > HL_CUSTOM ||
            !span(h->heap, header_bytes(h->kind)) || !managed(h->kind, h->start, h->bytes) ||
            h->state < HL_LIVE || h->state > HL_MUTATING || index->generation != h->generation ||
            index->reserved || index->count > HL_ALLOCS - ac || (!index->head) != (!index->count)) return 0;
        if (obd_active(h)) ++hc;
        else if (index->count) return 0;
        node = index->head;
        previous = 0;
        for (j = 0; j < index->count; ++j) {
            if (!obt_take(work) || !hi_member(node, i + 1, h->generation)) return 0;
            a = _gz_hl_allocations + node - 1;
            link = _gz_hi_links + node - 1;
            if (link->previous != previous || a->kind != h->kind || a->heap_epoch != h->epoch ||
                a->missing != HL_MISSING_ALL || a->reserved ||
                !within(h->start, h->bytes, a->pointer, a->capacity)) return 0;
            if (a->kind == HL_EXP && a->block &&
                (a->block > a->pointer || a->pointer - a->block < 16)) return 0;
            previous = node;
            node = link->next;
        }
        if (node) return 0;
        ac += index->count;
    }
    if (hc != _gz_hl_control.active_heaps || ac != _gz_hl_control.active_allocations ||
        ac != _gz_hi_control.nodes) return 0;
    *heaps = hc;
    *allocations = ac;
    return 1;
}

OBD_FN u32 obd_bytes(u32 heaps, u32 allocations) {
    if (heaps > HL_HEAPS || allocations > HL_ALLOCS) return 0;
    return 64 + heaps * 80 + allocations * 96;
}

/* Charged before any output. One second-pass heap slot/node unit, every
 * output word, and the header words. Census/private scans are charged as
 * they run. This is a descriptive bounded-work unit, not CPU cycles/time.
 */
OBD_FN u32 obd_copy_units(u32 heaps, u32 allocations) {
    if (heaps > HL_HEAPS || allocations > HL_ALLOCS) return 0;
    return HL_HEAPS + heaps * 20 + allocations * 25 + 16;
}

OBD_FN void obd_export(HlAllocation *d, const HlStoredAllocation *s) {
    d->pointer = s->pointer; d->generation = s->generation; d->state = s->state; d->kind = s->kind;
    d->heap = s->heap; d->heap_generation = s->heap_generation; d->requested = s->requested; d->alignment = s->alignment;
    d->block = s->block; d->capacity = s->capacity;
    d->padding = s->kind == HL_EXP && s->block ? s->pointer - s->block - 16u : 0;
    d->born_frame = s->born_frame; d->end_frame = s->end_frame; d->birth_site = s->birth_site; d->end_site = s->end_site;
    d->release_token = s->release_token; d->flags = s->flags; d->heap_epoch = s->heap_epoch;
    d->missing = s->missing; d->reserved = s->reserved;
}

/* No failure is possible after entry: all counts, nodes, spans and copy work
 * have been checked while the SAME metadata lock remained held. The caller
 * owns output and excludes other readers/writers until this call returns.
 */
OBD_FN void obd_copy(void *output, const ObDirectoryHeader *header) {
    ObDirectoryHeap *heap_out = (ObDirectoryHeap *)((unsigned char *)output + 64);
    ObDirectoryAllocation *allocation_out = (ObDirectoryAllocation *)((unsigned char *)heap_out + header->heap_count * 80);
    HlHeap *h;
    HiHeap *index;
    u32 i, j, k, node;
    for (i = 0; i < HL_HEAPS; ++i) {
        h = _gz_hl_heaps + i;
        if (!obd_active(h)) continue;
        heap_out->index = i + 1;
        for (k = 0; k < 16; ++k) ((u32 *)&heap_out->heap)[k] = ((const u32 *)h)[k];
        for (k = 0; k < 3; ++k) heap_out->reserved[k] = 0;
        ++heap_out;
        index = _gz_hi_heaps + i;
        node = index->head;
        for (j = 0; j < index->count; ++j) {
            allocation_out->index = node;
            allocation_out->heap_index = i + 1;
            obd_export(&allocation_out->allocation, _gz_hl_allocations + node - 1);
            allocation_out->reserved[0] = allocation_out->reserved[1] = 0;
            ++allocation_out;
            node = _gz_hi_links[node - 1].next;
        }
    }
    /* Header last, and only after every row copy completed. */
    for (k = 0; k < 16; ++k) ((u32 *)output)[k] = ((const u32 *)header)[k];
}

OBD_FN void obd_header(ObDirectoryHeader *header, u32 heaps, u32 allocations, u32 mode) {
    header->magic = OB_DIRECTORY_MAGIC; header->bytes = obd_bytes(heaps, allocations); header->version = 1;
    header->heap_count = heaps; header->allocation_count = allocations;
    header->heap_row_bytes = 80; header->allocation_row_bytes = 96;
    header->serial = _gz_hl_control.serial; header->release_serial = _gz_hl_control.release_serial;
    header->frame = _gz_hl_frame; header->active_heaps = _gz_hl_control.active_heaps;
    header->active_allocations = _gz_hl_control.active_allocations; header->mode = mode;
    header->missing = HL_MISSING_ALL; header->session = header->reserved = 0;
}

OBD_API ObU32 _gz_ob_try_directory(void *output, u32 capacity) {
    ObDirectoryHeader header;
    ObTryWork work = { OB_DIRECTORY_WORK_LIMIT, 0 };
    u32 heaps = 0, allocations = 0, bytes, z = OB_OBSERVER;
    if (!obd_span((u32)output, capacity) || !_gz_image_write_allowed((u32)output, capacity)) return OB_ARGUMENT;
    if (!obt_real_try_lock()) return OB_BUSY;
    if (!obt_healthy()) goto done;
    if (!obd_private((u32)output, capacity, &work)) { z = OB_ARGUMENT; goto done; }
    if (!obd_census(&heaps, &allocations, &work)) goto done;
    bytes = obd_bytes(heaps, allocations);
    if (bytes > capacity) { z = OB_SPACE; goto done; }
    if (!obd_reserve(&work, obd_copy_units(heaps, allocations))) goto done;
    obd_header(&header, heaps, allocations, OB_DIRECTORY_FULL);
    obd_copy(output, &header);
    z = OB_OK;
done:
    if (work.exhausted) z = OB_BUDGET;
    obt_real_unlock();
    return z;
}

/* Exact same validated census, but no row storage is needed to learn size. */
OBD_API ObU32 _gz_ob_try_directory_measure(ObDirectoryHeader *output) {
    ObDirectoryHeader header;
    ObTryWork work = { OB_DIRECTORY_WORK_LIMIT, 0 };
    u32 heaps = 0, allocations = 0, k, z = OB_OBSERVER;
    if (!obd_span((u32)output, 64) || !_gz_image_write_allowed((u32)output, 64)) return OB_ARGUMENT;
    if (!obt_real_try_lock()) return OB_BUSY;
    if (!obt_healthy()) goto done;
    if (!obd_private((u32)output, 64, &work)) { z = OB_ARGUMENT; goto done; }
    if (!obd_census(&heaps, &allocations, &work)) goto done;
    if (!obd_reserve(&work, 16)) goto done;
    obd_header(&header, heaps, allocations, OB_DIRECTORY_MEASURE);
    for (k = 0; k < 16; ++k) ((u32 *)output)[k] = ((const u32 *)&header)[k];
    z = OB_OK;
done:
    if (work.exhausted) z = OB_BUDGET;
    obt_real_unlock();
    return z;
}

#undef OBD_FN
#undef OBD_API
