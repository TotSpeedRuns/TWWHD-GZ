#include "registry_capsule.h"

static GrcU32 word(const void *source, GrcU32 offset)
{
    const GrcU8 *p = (const GrcU8 *)source + offset;
    return ((GrcU32)p[0] << 24) | ((GrcU32)p[1] << 16) |
           ((GrcU32)p[2] << 8) | p[3];
}

static GrcU32 span(GrcU32 address, GrcU32 bytes)
{
    return address >= 0x01800000u && address < 0xc0000000u &&
           bytes && bytes <= 0xc0000000u - address;
}

static GrcU32 metadata(const void *p, GrcU32 bytes)
{
    return !((GrcU32)p & 3) && span((GrcU32)p, bytes);
}

static GrcU32 contains(GrcU32 start, GrcU32 bytes, GrcU32 p, GrcU32 n)
{
    return span(start, bytes) && p >= start && n <= bytes && p - start <= bytes - n;
}

static GrcU32 apart(GrcU32 a, GrcU32 n, GrcU32 b, GrcU32 m)
{
    return a + n <= b || b + m <= a;
}

static void copy(void *target, const void *source, GrcU32 bytes)
{
    GrcU32 i;
    for (i = 0; i < bytes; ++i)
        ((GrcU8 *)target)[i] = ((const GrcU8 *)source)[i];
}

GrcU32 grc_required_bytes(GrcU32 layers, GrcU32 processes,
                         GrcU32 lists, GrcU32 nodes)
{
    if (!layers || layers > GRC_MAX_LAYERS || !processes ||
        processes > GRC_MAX_PROCESSES || !lists || lists > GRC_MAX_LISTS ||
        nodes > GRC_MAX_NODES)
        return 0;
    return 192 + layers * sizeof(GrcLayer) + processes * sizeof(GrcProcess) +
           lists * sizeof(GrcList) + nodes * sizeof(GrcNode);
}

static GrcU32 process(const GrcContext *c, const GrcProcess *p)
{
    const GrcU32 *po = p->po, *a = p->allocation, *h = p->heap;
    GrcU32 bytes = po[3] + po[4];
    if (po[2] != 1 || !po[1] || po[1] > c->po_serial || po[14] || po[11] ||
        po[3] < sizeof(p->native) || bytes < po[3] || bytes != po[5] ||
        !span(po[0], bytes) || po[6] >= 0xfffffffeu || po[7] >= 484 ||
        word(p->native, 0) != c->base_type || word(p->native, 4) != po[6] ||
        (((GrcU32)p->native[8] << 8) | p->native[9]) != po[8] ||
        p->native[12] != 2 || (((GrcU32)p->native[14] << 8) | p->native[15]) != po[7] ||
        word(p->native, 16) != po[9])
        return GRC_IDENTITY;
    if (a[2] != 1 || !a[1] || a[1] > c->hl_serial || a[18] != 31 || a[19] ||
        !contains(a[0], a[9], po[0], bytes) || a[4] != h[0] ||
        a[5] != h[1] || a[17] != h[15] || h[2] != 1 || !h[1] ||
        h[1] > c->hl_serial || h[3] < 1 || h[3] > 4 || a[3] != h[3] ||
        !contains(h[4], h[5], a[0], a[9]))
        return GRC_IDENTITY;
    return GRC_OK;
}

static GrcU32 node_offset(GrcU32 kind)
{
    if (kind == GRC_LAYER) return 0x18;
    if (kind == GRC_ACTOR) return 0xc8;
    if (kind == GRC_LINE) return 0x34;
    if (kind == GRC_DRAW) return 0xdc;
    return GRC_NONE;
}

static GrcU32 graph(const GrcInput *in)
{
    const GrcContext *c = &in->context;
    GrcU32 i, j, k, n, expected = 0, cursor = 0, current = 0;
    GrcU32 layer_members, hits, offset;
    if (c->po_flags || c->hl_flags || c->observer_busy)
        return GRC_OBSERVER;
    if (!c->po_serial || !c->hl_serial || !c->base_type || !c->node_type ||
        !c->next_layer_id || c->next_layer_id >= 0xfffffffdu ||
        c->reserved[0] || c->reserved[1] || c->reserved[2] ||
        c->registry[0] != c->root_layer || c->registry[2] != in->layer_count ||
        c->registry[1] != in->layers[in->layer_count - 1].ea ||
        !span(c->layer_registry, 12) || !span(c->root_lists, 120) ||
        !c->line_count || c->line_count > 256 || !c->draw_count || c->draw_count > 256 ||
        !span(c->line_base, c->line_count * 12) || !span(c->draw_base, c->draw_count * 12))
        return GRC_LAYOUT;
    for (i = 0; i < in->process_count; ++i) {
        GrcU32 z = process(c, in->processes + i);
        if (z) return z;
        for (j = 0; j < i; ++j)
            if (in->processes[j].po[0] == in->processes[i].po[0] ||
                in->processes[j].po[1] == in->processes[i].po[1])
                return GRC_IDENTITY;
    }
    for (i = 0; i < in->layer_count; ++i) {
        const GrcLayer *l = in->layers + i;
        GrcU32 owner = word(l->native, 24), lists = word(l->native, 16);
        n = i ? 16 : 10;
        if (!span(l->ea, 44) || word(l->native, 0) != (i ? in->layers[i - 1].ea : 0) ||
            word(l->native, 4) != c->layer_registry ||
            word(l->native, 8) != (i + 1 < in->layer_count ? in->layers[i + 1].ea : 0) ||
            l->first_list != expected || word(l->native, 20) != n)
            return GRC_LAYOUT;
        for (j = 0; j < i; ++j)
            if (in->layers[j].ea == l->ea || word(in->layers[j].native, 12) == word(l->native, 12))
                return GRC_LAYOUT;
        if (!i) {
            if (l->ea != c->root_layer || owner || word(l->native, 12) ||
                lists != c->root_lists || l->owner_process != GRC_NONE || l->parent_layer != GRC_NONE)
                return GRC_LAYOUT;
        } else {
            const GrcProcess *p;
            if (l->owner_process >= in->process_count || l->parent_layer >= in->layer_count ||
                !word(l->native, 12) || word(l->native, 12) >= c->next_layer_id)
                return GRC_LAYOUT;
            p = in->processes + l->owner_process;
            if (owner != p->po[0] || p->po[5] < 0x1b0 || l->ea != owner + 0xc0 ||
                lists != owner + 0xec || word(p->native, 0xb8) != c->node_type ||
                word(p->native, 0x2c) != in->layers[l->parent_layer].ea)
                return GRC_IDENTITY;
            k = l->parent_layer;
            for (j = 0; j < in->layer_count && k; ++j) {
                if (k >= in->layer_count || k == i) return GRC_LAYOUT;
                k = in->layers[k].parent_layer;
            }
            if (k) return GRC_LAYOUT;
        }
        /* First retained lane has no pending layer cancellation/create/delete.
         * These are unsupported active operations, never interpreted as empty. */
        if (word(l->native, 28) || word(l->native, 32) || word(l->native, 36) ||
            word(l->native, 40))
            return GRC_UNSUPPORTED;
        if (c->current_layer == l->ea) ++current;
        if (expected > in->list_count || n > in->list_count - expected)
            return GRC_LAYOUT;
        for (j = 0; j < n; ++j) {
            const GrcList *x = in->lists + expected + j;
            if (x->ea != lists + 12 * j || x->kind != GRC_LAYER || x->owner_layer != i || x->slot != j)
                return GRC_LAYOUT;
        }
        expected += n;
    }
    if (current != 1 || expected + 4 + c->line_count + c->draw_count != in->list_count)
        return GRC_LAYOUT;
    layer_members = expected;
    for (i = expected; i < in->list_count; ++i) {
        const GrcList *x = in->lists + i;
        GrcU32 ea, kind, slot = 0, relative = i - expected;
        if (relative < 4) {
            const GrcU32 roots[4] = {c->actor_root, c->create_root, c->delete_root, c->priority_root};
            const GrcU32 kinds[4] = {GRC_ACTOR, GRC_CREATE, GRC_DELETE, GRC_PRIORITY};
            ea = roots[relative]; kind = kinds[relative];
        } else if (relative < 4 + c->line_count) {
            slot = relative - 4; ea = c->line_base + 12 * slot; kind = GRC_LINE;
        } else {
            slot = relative - 4 - c->line_count; ea = c->draw_base + 12 * slot; kind = GRC_DRAW;
        }
        if (x->ea != ea || x->kind != kind || x->owner_layer != GRC_NONE || x->slot != slot)
            return GRC_LAYOUT;
    }
    for (i = 0; i < in->list_count; ++i) {
        const GrcList *l = in->lists + i;
        GrcU32 count = word(l->native, 8), head = word(l->native, 0), tail = word(l->native, 4);
        if (!span(l->ea, 12) || l->first_node != cursor || count > in->node_count - cursor ||
            (!count && (head || tail)) || (count && (!head || !tail)))
            return GRC_LAYOUT;
        for (j = 0; j < i; ++j)
            if (in->lists[j].ea == l->ea) return GRC_LAYOUT;
        offset = node_offset(l->kind);
        if (count && offset == GRC_NONE) return GRC_UNSUPPORTED;
        for (j = 0; j < count; ++j) {
            const GrcNode *v = in->nodes + cursor + j;
            const GrcProcess *p;
            if (v->process_index >= in->process_count || v->list_index != i)
                return GRC_LAYOUT;
            p = in->processes + v->process_index;
            if (offset > p->po[5] || 20 > p->po[5] - offset || v->ea != p->po[0] + offset ||
                word(v->native, 0) != (j ? in->nodes[cursor + j - 1].ea : 0) ||
                word(v->native, 4) != l->ea ||
                word(v->native, 8) != (j + 1 < count ? in->nodes[cursor + j + 1].ea : 0) ||
                word(v->native, 12) != p->po[0] || v->native[16] != 1)
                return GRC_LAYOUT;
            if (l->kind == GRC_LAYER && word(p->native, 0x2c) != in->layers[l->owner_layer].ea)
                return GRC_IDENTITY;
            if (offset + 20 <= sizeof(p->native))
                for (k = 0; k < 20; ++k)
                    if (v->native[k] != p->native[offset + k]) return GRC_IDENTITY;
            for (k = 0; k < cursor + j; ++k)
                if (in->nodes[k].ea == v->ea) return GRC_LAYOUT;
        }
        if (count && (head != in->nodes[cursor].ea || tail != in->nodes[cursor + count - 1].ea))
            return GRC_LAYOUT;
        cursor += count;
    }
    if (cursor != in->node_count) return GRC_LAYOUT;
    for (i = 0; i < in->process_count; ++i) {
        hits = 0;
        for (j = 0; j < layer_members; ++j) {
            const GrcList *l = in->lists + j;
            for (k = 0; k < word(l->native, 8); ++k)
                if (in->nodes[l->first_node + k].process_index == i) ++hits;
        }
        if (hits != 1) return GRC_LAYOUT;
    }
    return GRC_OK;
}

static GrcU32 input(const GrcInput *in)
{
    if (!metadata(in, sizeof(*in))) return GRC_ARGUMENT;
    if (in->magic != GRC_INPUT_MAGIC || in->bytes != sizeof(*in) ||
        in->version != 1 || in->reserved)
        return GRC_ARGUMENT;
    if (!grc_required_bytes(in->layer_count, in->process_count, in->list_count, in->node_count))
        return GRC_LIMIT;
    if (!metadata(in->layers, in->layer_count * sizeof(GrcLayer)) ||
        !metadata(in->processes, in->process_count * sizeof(GrcProcess)) ||
        !metadata(in->lists, in->list_count * sizeof(GrcList)) ||
        (in->node_count && !metadata(in->nodes, in->node_count * sizeof(GrcNode))))
        return GRC_ARGUMENT;
    return graph(in);
}

static GrcU32 image_hash(const void *p, GrcU32 n)
{
    GrcU32 i, h = 2166136261u;
    for (i = 0; i < n; ++i) {
        GrcU8 b = (i >= 12 && i < 16) || (i >= 20 && i < 24) ? 0 : ((const GrcU8 *)p)[i];
        h = (h ^ b) * 16777619u;
    }
    return h;
}

GrcU32 grc_build_detached(const GrcInput *in, void *output, GrcU32 capacity)
{
    GrcU32 z = input(in), n, i, at;
    GrcU32 *h = (GrcU32 *)output;
    const void *arrays[4];
    GrcU32 sizes[4];
    if (z) return z;
    n = grc_required_bytes(in->layer_count, in->process_count, in->list_count, in->node_count);
    if (!metadata(output, capacity)) return GRC_ARGUMENT;
    if (capacity < n) return GRC_SPACE;
    arrays[0] = in->layers; sizes[0] = in->layer_count * sizeof(GrcLayer);
    arrays[1] = in->processes; sizes[1] = in->process_count * sizeof(GrcProcess);
    arrays[2] = in->lists; sizes[2] = in->list_count * sizeof(GrcList);
    arrays[3] = in->nodes; sizes[3] = in->node_count * sizeof(GrcNode);
    if (!apart((GrcU32)output, capacity, (GrcU32)in, sizeof(*in))) return GRC_ARGUMENT;
    for (i = 0; i < 4; ++i)
        if (sizes[i] && !apart((GrcU32)output, capacity, (GrcU32)arrays[i], sizes[i])) return GRC_ARGUMENT;
    /* Every refusal above precedes the first output store. Inputs must remain
     * immutable detached views throughout; this function owns no native read. */
    for (i = 0; i < 16; ++i) h[i] = 0;
    h[0] = GRC_MAGIC; h[1] = n; h[2] = 1; h[4] = GRC_MISSING_ALL; h[6] = 64;
    copy((GrcU8 *)output + 64, &in->context, 128);
    at = 192;
    for (i = 0; i < 4; ++i) {
        h[7 + i] = at;
        if (sizes[i]) copy((GrcU8 *)output + at, arrays[i], sizes[i]);
        at += sizes[i];
    }
    h[12] = in->layer_count; h[13] = in->process_count;
    h[14] = in->list_count; h[15] = in->node_count;
    h[5] = image_hash(output, n);
    __asm__ volatile("sync" ::: "memory");
    h[3] = 1;
    return GRC_OK;
}

GrcU32 grc_validate_detached(const void *image, GrcU32 bytes)
{
    const GrcU32 *h = (const GrcU32 *)image;
    GrcInput in;
    GrcU32 at, n;
    if (!metadata(image, bytes) || bytes < 192) return GRC_ARGUMENT;
    if (h[0] != GRC_MAGIC || h[1] != bytes || h[2] != 1 || h[3] != 1 ||
        h[4] != GRC_MISSING_ALL || h[6] != 64 || h[11]) return GRC_CORRUPT;
    n = grc_required_bytes(h[12], h[13], h[14], h[15]);
    if (!n || n != bytes || h[5] != image_hash(image, bytes)) return GRC_CORRUPT;
    at = 192;
    if (h[7] != at) return GRC_CORRUPT;
    at += h[12] * sizeof(GrcLayer);
    if (h[8] != at) return GRC_CORRUPT;
    at += h[13] * sizeof(GrcProcess);
    if (h[9] != at) return GRC_CORRUPT;
    at += h[14] * sizeof(GrcList);
    if (h[10] != at) return GRC_CORRUPT;
    in.magic = GRC_INPUT_MAGIC; in.bytes = sizeof(in); in.version = 1; in.reserved = 0;
    copy(&in.context, (const GrcU8 *)image + 64, 128);
    in.layer_count = h[12]; in.process_count = h[13]; in.list_count = h[14]; in.node_count = h[15];
    in.layers = (const GrcLayer *)((const GrcU8 *)image + h[7]);
    in.processes = (const GrcProcess *)((const GrcU8 *)image + h[8]);
    in.lists = (const GrcList *)((const GrcU8 *)image + h[9]);
    in.nodes = (const GrcNode *)((const GrcU8 *)image + h[10]);
    return input(&in);
}

GrcU32 grc_read_ready(const void *image, GrcU32 bytes)
{
    (void)image;
    (void)bytes;
    return 0;
}

GrcU32 grc_compare_retained(const void *source, GrcU32 source_bytes,
                          const void *current, GrcU32 current_bytes)
{
    const GrcU8 *a = (const GrcU8 *)source, *b = (const GrcU8 *)current;
    GrcU32 i, z = grc_validate_detached(source, source_bytes);
    if (z) return z;
    z = grc_validate_detached(current, current_bytes);
    if (z) return z;
    if (source_bytes != current_bytes) return GRC_IDENTITY;
    /* Frame/observation serial may advance; observer monotonic counters must
     * never move backwards. They are compared, never copied into live state. */
    if (word(a, 68) != word(b, 68) || word(a, 96) != word(b, 96) ||
        word(a, 100) != word(b, 100) || word(b, 72) < word(a, 72) ||
        word(b, 76) < word(a, 76) || word(b, 80) < word(a, 80))
        return GRC_IDENTITY;
    /* All root identities/list order and complete PO/HL/native-prefix rows
     * remain exact. This deliberately refuses a changed current graph. */
    for (i = 24; i < 64; ++i)
        if (a[i] != b[i]) return GRC_IDENTITY;
    for (i = 108; i < source_bytes; ++i)
        if (a[i] != b[i]) return GRC_IDENTITY;
    return GRC_OK;
}
