#include "generic_process_reconcile.h"
#include "owned_heap_native.h"
#include "heap_domain_cut.h"
#include "owned_heap_common_domain.h"
#include <stdint.h>
#include "opaque_chunks.h"

static u32 image_apart(const void *a, u32 n, const void *b, u32 m)
{
    uintptr_t p = (uintptr_t)a, q = (uintptr_t)b;
    return n <= UINTPTR_MAX - p && m <= UINTPTR_MAX - q &&
           (p + n <= q || q + m <= p);
}

static void image_copy(void *a, const void *b, u32 n)
{
    volatile unsigned char *x = a;
    const volatile unsigned char *y = b;
    u32 i;
    for (i = 0; i < n; ++i) x[i] = y[i];
}

static u32 image_equal(const void *a, const void *b, u32 n)
{
    const volatile unsigned char *x = a, *y = b;
    u32 i;
    for (i = 0; i < n; ++i) if (x[i] != y[i]) return 0;
    return 1;
}

static u32 image_hash(const void *a, u32 n, u32 normalize_header)
{
    const volatile unsigned char *p = a;
    u32 i, v, h = 2166136261u;
    for (i = 0; i < n; ++i) {
        v = p[i];
        if (normalize_header && ((i >= 12 && i < 16) || (i >= 108 && i < 112)))
            v = 0;
        h = (h ^ v) * 16777619u;
    }
    return h;
}

u32 grc_project_as8_private(const void *image, u32 bytes, u32 identity,
                            u32 generation, GrcCutStamp *stamp, GrcCutView *view)
{
    const unsigned char *p = image;
    const GhcHeader *h;
    const GhcSaveProducer *producer;
    const GhdCut *cut;
    const GhcProcessDirectory *directory;
    GrcCutStamp next;
    GrcCutView proposed;
    u32 i, root = 0, roots = 0, checksum, plan_checksum, selected;
    GhdLabels labels;
    if (!image || !stamp || !view || !identity || !generation ||
        bytes < 3536 || bytes > 0x40000000u || ((uintptr_t)image & 3u) ||
        !image_apart(image, bytes, stamp, sizeof(*stamp)) ||
        !image_apart(image, bytes, view, sizeof(*view)) ||
        !image_apart(stamp, sizeof(*stamp), view, sizeof(*view)))
        return GRC_ARGUMENT;
    h = image;
    if (h->magic != GHC_MAGIC || h->version != GHCP_IMAGE_VERSION ||
        h->bytes != bytes || h->valid != 1 || h->diagnostic != 1 ||
        h->missing != GHC_MISSING_ALL || !h->count || h->count > HL_HEAPS ||
        h->root_mask != GHC_COMMON_ROOT_MASK || h->cut != 208 || h->table != 3536 ||
        h->reserved[0] != ghc_process_offset(h->count) ||
        h->reserved[0] > bytes || sizeof(*directory) > bytes - h->reserved[0] ||
        h->data > bytes || h->data_bytes != bytes - h->data ||
        !h->domain.span || h->domain.token != h->hg ||
        h->domain.session || h->domain.obligations || h->domain.reserved[0] ||
        h->domain.reserved[1] || h->domain.reserved[2])
        return GRC_FORMAT;
    producer = (const GhcSaveProducer *)(p + 128);
    if (!ghs_provenance_format(producer, h->request, h->frame, h->hg, h->lease,
                               h->parent, h->thread))
        return GRC_FORMAT;
    directory = (const GhcProcessDirectory *)(p + h->reserved[0]);
    if (directory->count > PO_CAP ||
        directory->bytes != sizeof(*directory) + directory->count * sizeof(GrcProcess) ||
        directory->bytes != h->reserved[1] || directory->bytes > bytes - h->reserved[0] ||
        h->data < ghc_common_data(h->count, directory->count, 0))
        return GRC_FORMAT;
    cut = (const GhdCut *)(p + h->cut);
    if (cut->magic != 0x47484431u || cut->bytes != sizeof(*cut) || cut->version != 1 ||
        !cut->nodes || cut->nodes > GHD_NODES || cut->flags != 2 ||
        cut->missing != HL_MISSING_ALL || cut->session ||
        cut->unknown_chain_heaps || cut->unknown_chain_indexed ||
        !cut->root[1].node || cut->root[1].node > cut->nodes ||
        cut->selected_heaps > HL_HEAPS)
        return GRC_FORMAT;
    image_copy(&labels, p + 192, sizeof(labels));
    selected = ghc_common_node(cut, &labels);
    if (!selected || !ghc_common_image_format(h, cut, selected) ||
        oc_validate(p + h->data, h->data_bytes, h->domain.span) != OC_OK)
        return GRC_FORMAT;
    for (i = 0; i < cut->nodes; ++i) {
        u32 n = i + 1, seen = 0;
        if (cut->node[i].parent_node > cut->nodes)
            return GRC_FORMAT;
        while (n) {
            const GhdNode *node;
            if (n > cut->nodes || (seen & (1u << (n - 1))))
                return GRC_FORMAT;
            seen |= 1u << (n - 1); node = cut->node + n - 1;
            if (node->parent_node) {
                const HlHeap *parent;
                if (node->parent_node > cut->nodes)
                    return GRC_FORMAT;
                parent = &cut->node[node->parent_node - 1].heap;
                if (node->heap.parent != parent->heap ||
                    node->heap.parent_generation != parent->generation)
                    return GRC_FORMAT;
            } else if (node->heap.parent || node->heap.parent_generation) {
                return GRC_FORMAT;
            }
            n = node->parent_node;
        }
        if (!cut->node[i].parent_node) { root = i; ++roots; }
    }
    if (roots != 1)
        return GRC_FORMAT; /* Current actual GPA owns one original SDK arena. */
    for (i = 0; i < sizeof(next); ++i) ((volatile unsigned char *)&next)[i] = 0;
    image_copy(&next.producer, producer, sizeof(next.producer));
    image_copy(&next.scene, &directory->scene, sizeof(next.scene));
    image_copy(&next.selected_domain, &cut->node[selected - 1].heap,
                sizeof(next.selected_domain));
    image_copy(&next.sdk_domain, &cut->node[root].heap, sizeof(next.sdk_domain));
    image_copy(&next.processes, &directory->po_before, sizeof(next.processes));
    image_copy(&next.allocations, &directory->hl_before, sizeof(next.allocations));
    next.archive_identity = identity; next.archive_generation = generation;
    next.image_type = next.image_version = GHCP_IMAGE_VERSION; next.image_bytes = bytes;
    next.image_checksum = checksum = h->checksum;
    next.image_plan_checksum = plan_checksum = h->plan_checksum;
    next.row_count = directory->count; next.row_checksum = directory->row_checksum;
    next.missing = h->missing; next.census_flags = directory->flags;
    proposed.stamp = &next; proposed.directory = directory;
    proposed.rows = (const GrcProcess *)(directory + 1);
    proposed.rows_bytes = directory->count * (u32)sizeof(GrcProcess);
    proposed.directory_offset = h->reserved[0];
    if (next.selected_domain.heap != h->domain.heap ||
        next.selected_domain.generation != h->domain.heap_generation ||
        next.allocations.serial != cut->serial ||
        next.allocations.release_serial != cut->release_serial ||
        grc_validate_cut(&proposed) != GRC_OK ||
        image_hash(p + 128, h->data - 128, 0) != plan_checksum ||
        image_hash(p, bytes, 1) != checksum ||
        !image_equal(producer, &next.producer, sizeof(next.producer)) ||
        h->valid != 1 || h->checksum != checksum || h->plan_checksum != plan_checksum)
        return GRC_FORMAT;
    image_copy(stamp, &next, sizeof(next));
    proposed.stamp = stamp;
    image_copy(view, &proposed, sizeof(proposed));
    return GRC_OK;
}
