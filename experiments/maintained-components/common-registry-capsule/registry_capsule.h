#ifndef GZ_COMMON_REGISTRY_CAPSULE_H
#define GZ_COMMON_REGISTRY_CAPSULE_H

typedef unsigned int GrcU32;
typedef unsigned char GrcU8;

enum {
    GRC_MAGIC = 0x47524331u, GRC_INPUT_MAGIC = 0x47524931u,
    GRC_VERSION = 1, GRC_NONE = 0xffffffffu,
    GRC_MAX_LAYERS = 128, GRC_MAX_PROCESSES = 512,
    GRC_MAX_LISTS = 2200, GRC_MAX_NODES = 8192,
    GRC_MISSING_OWNER = 1, GRC_MISSING_RESOURCES = 2,
    GRC_MISSING_LIFETIME = 4, GRC_MISSING_EXTERNAL = 8,
    GRC_MISSING_ROOT_BINDINGS = 16, GRC_MISSING_REPLAY = 32,
    GRC_MISSING_ALL = 63,
    GRC_LAYER = 1, GRC_ACTOR = 2, GRC_LINE = 3, GRC_DRAW = 4,
    GRC_CREATE = 5, GRC_DELETE = 6, GRC_PRIORITY = 7,
    GRC_OK = 0, GRC_ARGUMENT = 1, GRC_LIMIT = 2, GRC_LAYOUT = 3,
    GRC_IDENTITY = 4, GRC_OBSERVER = 5, GRC_UNSUPPORTED = 6,
    GRC_SPACE = 7, GRC_CORRUPT = 8
};

/* These are detached byte/metadata observations, not an issuer ABI. Native
 * EAs are opaque labels: this component never dereferences one. The provider
 * must own all input byteviews continuously and join the actual current PO/HL
 * records, native root extents, and real barriers before copying native data.
 */
typedef struct {
    GrcU32 frame, scene_epoch, po_serial, hl_serial, hl_release_serial;
    GrcU32 po_flags, hl_flags, observer_busy, code_identity[2], observation_serial;
    GrcU32 current_layer, next_layer_id, root_layer, root_lists, layer_registry;
    GrcU32 registry[3], actor_root, create_root, delete_root, priority_root;
    GrcU32 line_base, line_count, draw_base, draw_count, base_type, node_type;
    GrcU32 reserved[3];
} GrcContext;

/* Native layer is the complete44-byte layer body, including registry node,
 * ID/tree/owner, cancel descriptor, and creating/deleting counters. */
typedef struct {
    GrcU32 ea, first_list, owner_process, parent_layer;
    GrcU8 native[44];
} GrcLayer;

/* Exact selected public PO64/HL allocation80/HL heap64 projections, copied
 * under their genuine metadata protocols. The native prefix is0xC0 bytes.
 * This first lane refuses smaller base processes rather than reading past one.
 */
typedef struct {
    GrcU32 po[16], allocation[20], heap[16];
    GrcU8 native[0xc0];
} GrcProcess;

/* List native bytes are head/tail/count. Input order is canonical: every
 * layer's10(root)/16(node) lists, actor/create/delete/priority, line[],draw[].
 * first_node indexes a consecutive run in nodes; empty lists still advance
 * neither the cursor nor invent a member. */
typedef struct {
    GrcU32 ea, kind, owner_layer, slot, first_node;
    GrcU8 native[12];
} GrcList;

typedef struct {
    GrcU32 ea, process_index, list_index;
    GrcU8 native[20];
} GrcNode;

typedef struct {
    GrcU32 magic, bytes, version, reserved;
    GrcContext context;
    GrcU32 layer_count, process_count, list_count, node_count;
    const GrcLayer *layers;
    const GrcProcess *processes;
    const GrcList *lists;
    const GrcNode *nodes;
} GrcInput;

/* Pointer-free image: header64, context128, then the four exact record arrays.
 * Header words: magic,bytes,version,ready,missing,hash,context,layers,processes,
 * lists,nodes,reserved,layer_count,process_count,list_count,node_count.
 * ready means completed FORMAT publication only; missing is ALWAYS63.
 * Hash is FNV1a over every byte except ready/hash words (treated as zero).
 */
GrcU32 grc_required_bytes(GrcU32 layers, GrcU32 processes,
                         GrcU32 lists, GrcU32 nodes);
GrcU32 grc_build_detached(const GrcInput *, void *output, GrcU32 capacity);
GrcU32 grc_validate_detached(const void *image, GrcU32 bytes);
/* Structural/identity comparison only. No actor/native store or grant; fresh
 * native ownership and independent undo are requirements of a future caller. */
GrcU32 grc_compare_retained(const void *source, GrcU32 source_bytes,
                          const void *current, GrcU32 current_bytes);
GrcU32 grc_read_ready(const void *image, GrcU32 bytes);

typedef char GrcContextSize[sizeof(GrcContext) == 128 ? 1 : -1];
typedef char GrcLayerSize[sizeof(GrcLayer) == 60 ? 1 : -1];
typedef char GrcProcessSize[sizeof(GrcProcess) == 400 ? 1 : -1];
typedef char GrcListSize[sizeof(GrcList) == 32 ? 1 : -1];
typedef char GrcNodeSize[sizeof(GrcNode) == 32 ? 1 : -1];
typedef char GrcInputSize[sizeof(GrcInput) == 176 ? 1 : -1];
#endif
