/* Compact current CPU bridge capture for the three known native layouts.
 * Replay is a separate same-allocation WCT participant.
 * Fixed RKC entry owns the real native read boundary; OB/GH/HL/BG below prove
 * current spans inside it. Observation metadata alone never issues that hold.
 * Unknown variants refuse explicitly; no row is omitted from new coverage.
 * Old scalar-only files remain parseable but cannot supply new geometry.
 */
#include "bridge_capture.h"
#include "bridge_format.h"
#include "bridge_format3.h"
#include "object_binding.h"
#include "background_lifetime.h"
#include "heap_owner.h"
#include "world_slot.h"
#include "native_lifetime/lifetime.h"

enum {
    BRG_CONTEXT = 1800, BRG_PROCESS = 1801, BRG_PHYSICAL = 1802,
    BRG_LAYOUT = 1803, BRG_CHANGED = 1804, BRG_CAPACITY = 1805,
    BRG_ALIAS = 1806, BRG_NUMERIC = 1807, BRG_FORMAT = 1808,
    BRG3_DESCRIPTOR_CAP = 240, BRG_ACTOR_BYTES = 0x14ae4, BRG_CAPTURED = 1, BRG_CONSUMED = 2,
    BRG_VERIFIED = 3, BRG_POISON = 4
};

extern unsigned char _gz_world_bridges_a[], _gz_world_bridges_b[], _gz_v73_state_data[];
extern volatile u32 _gz_world_working_data, _gz_world_working_bridges, _gz_world_committed_bridges;
extern volatile u32 _gz_v73_state_valid;
extern const u32 _gz_world_bridge_offsets[4701];
extern u32 _gz_rkc_read_scope(u32, u32), _gz_krc_private_apart(u32, u32);
extern u32 _gz_osa_pin_id(void), _gz_osa_validate(u32), _gz_osa_source(u32), _gz_osa_bytes(u32);

typedef struct {
    PoEntry process;
    ObBinding actor;
    HlHeap heap;
    BgEntry background;
    u32 world_index, count, type, action, scalar_hash, graph_hash;
    u32 omitted_hash, span_count, geometry_bytes;
} BrgWitness;

typedef struct {
    u32 address, count, width, stride, kind, segment, line, packed;
} BrgDescriptor;

typedef struct {
    u32 state, entered, frame, world_hash, sequence, epoch, working, bank;
    u32 count, entries, coverage, bytes, bank_hash, seal;
    BrgWitness rows[8];
} BrgStage;

typedef struct {u32 address,bytes,strict;} BrgReadRange;
enum {BRG_READ_CAP=384};
static BrgReadRange brg_reads[BRG_READ_CAP];
static u32 brg_read_count;
static BrgStage brg_stage;
static BrgWitness brg_current[8];
static BrgDescriptor brg_descriptors[BRG3_DESCRIPTOR_CAP];
/* OB metadata arguments MUST be in mapped fixed-image storage. The native
 * game stack and counted work arenas can be tracked heaps and are rejected.
 * This scratch is never retained as a lifetime capability. Copy/API/copy runs
 * only inside the owned, nonyielding entered guard below. */
static ObBinding brg_binding_scratch;
static u32 brg_a_bytes = 4, brg_b_bytes = 4;
extern u32 _gz_brr_native_context(void), _gz_brr_source_frame(void);
extern u32 _gz_brr_private_apart(u32,u32);
u32 _gz_brg_report[16] = {0x42524752u, 64, 1};

static void brg_copy(void *destination, const void *source, u32 bytes)
{
    u8 *d = destination;
    const u8 *s = source;
    while(bytes--)*d++ = *s++;
}

static void brg_zero(void *destination, u32 bytes)
{
    u8 *d = destination;
    while(bytes--)*d++ = 0;
}

static u32 brg_equal(const void *left, const void *right, u32 bytes)
{
    const u8 *a = left, *b = right;
    while(bytes--)if(*a++ != *b++)return 0;
    return 1;
}

static u32 brg_hash_word(u32 hash, u32 value)
{
    u32 i;
    for(i = 0; i < 4; i++)hash = (hash ^ ((value >> (24 - i * 8)) & 255)) * 16777619u;
    return hash;
}

static u32 brg_apart(u32 a, u32 n, u32 b, u32 m)
{
    return n <= 0xffffffffu - a && m <= 0xffffffffu - b && (a + n <= b || b + m <= a);
}

u32 _gz_brg_private_apart(u32 p, u32 bytes)
{
    return bytes && bytes <= 0xffffffffu - p && _gz_brr_private_apart(p,bytes) &&
        brg_apart(p,bytes,(u32)brg_reads,sizeof(brg_reads)) &&
        brg_apart(p,bytes,(u32)&brg_read_count,4) &&
        brg_apart(p, bytes, (u32)&brg_stage, sizeof(brg_stage)) &&
        brg_apart(p, bytes, (u32)brg_current, sizeof(brg_current)) &&
        brg_apart(p, bytes, (u32)brg_descriptors, sizeof(brg_descriptors)) &&
        brg_apart(p, bytes, (u32)&brg_binding_scratch, sizeof(brg_binding_scratch)) &&
        brg_apart(p, bytes, (u32)&brg_a_bytes, 4) && brg_apart(p, bytes, (u32)&brg_b_bytes, 4) &&
        brg_apart(p, bytes, (u32)_gz_brg_report, sizeof(_gz_brg_report)) &&
        brg_apart(p, bytes, (u32)_gz_world_bridges_a, BRG3_BANK_BYTES) &&
        brg_apart(p, bytes, (u32)_gz_world_bridges_b, BRG3_BANK_BYTES);
}

static u32 brg_scope(u32 mode, u32 frame)
{
    const VrWorld *world = &_gz_vws_stage;
    return brg_stage.entered == 1 && brg_stage.state < BRG_POISON &&
        (mode == 1 || mode == 2) && frame && frame == _gz_pbac_frame &&
        world->complete == 1 && world->count <= VR_CAPACITY && world->frame == frame &&
        _gz_rkc_read_scope(mode, frame);
}

static u32 brg_capture_binding(u32 p, u32 bytes, ObBinding *retained)
{
    u32 result;
    if(brg_stage.entered != 1 && !_gz_brr_native_context())return BRG_CONTEXT;
    result = _gz_ob_capture(p, bytes, &brg_binding_scratch);
    if(!result)result = _gz_ob_validate(&brg_binding_scratch);
    if(!result && _gz_brr_native_context()) {
        u32 source_frame = _gz_brr_source_frame();
        if(!source_frame)result = BRG_CONTEXT;
        else result = _gz_ob_validate_before(&brg_binding_scratch,source_frame);
    }
    if(result)return BRG_PHYSICAL;
    if(retained)brg_copy(retained, &brg_binding_scratch, sizeof(*retained));
    return 0;
}

static u32 brg_validate_binding(const ObBinding *retained)
{
    if(brg_stage.entered != 1 && !_gz_brr_native_context())return BRG_CONTEXT;
    brg_copy(&brg_binding_scratch, retained, sizeof(*retained));
    return _gz_ob_validate(&brg_binding_scratch) ? BRG_PHYSICAL : 0;
}

static u32 brg_heap_stamp(u32 heap, HlHeap *out)
{
    u32 i, start = ((heap >> 2) * 0x9e3779b1u) >> 20, result = BRG_PHYSICAL;
    if(_gz_hl_control.magic != HL_MAGIC || _gz_hl_control.bytes != 64 ||
        _gz_hl_control.version != 1 || !_gz_hl_meta_try_lock())return result;
    if(_gz_hl_control.flags || _gz_hl_control.active_heaps > HL_HEAPS ||
        _gz_hl_control.active_allocations > HL_ALLOCS)goto done;
    for(i = 0; i < HL_HEAPS; i++) {
        const HlHeap *row = _gz_hl_heaps + ((start + i) & 4095u);
        if(!row->heap)break;
        if(row->heap != heap)continue;
        if(row->state != HL_LIVE || row->kind != HL_SOLID || !row->generation ||
            (row->flags & ~HL_REBORN) || row->end_frame || row->dtor_site)break;
        brg_copy(out, row, sizeof(*out));
        result = 0;
        break;
    }
done:
    _gz_hl_meta_unlock();
    return result;
}

static u32 brg_native_span(const BrgWitness *owner, u32 p, u32 bytes, u32 alignment, u32 external)
{
    u32 actor = owner->process.object, capacity = owner->process.allocation_bytes;
    if(!alignment || p % alignment || !bytes || p < 0x10000000u || p >= 0xbffe0000u ||
        bytes > 0xbffe0000u - p || !_gz_krc_private_apart(p, bytes))return BRG_ALIAS;
    if(!external && !(p >= actor && p - actor <= capacity && bytes <= capacity - (p - actor))) {
        if(!owner->heap.heap || _gz_gh_solid(owner->heap.heap, p, bytes) ||
            !_gz_gh_identity.valid)return BRG_PHYSICAL;
    }
    return brg_capture_binding(p, bytes, 0);
}

static u32 brg_heap_span(const BrgWitness *owner, u32 p, u32 bytes, u32 alignment)
{
    if(!owner->heap.heap || _gz_gh_solid(owner->heap.heap, p, bytes) ||
        !_gz_gh_identity.valid)return BRG_PHYSICAL;
    return brg_native_span(owner, p, bytes, alignment, 0);
}

static u32 brg_actor(const VrActor *saved, u32 index, BrgWitness *out)
{
    PoEntry last;
    u32 actor = saved->actor, i, z;
    brg_zero(out, sizeof(*out));
    if(_gz_po_lookup(actor) || !_gz_po_observation.valid || _gz_po_observation.flags)return BRG_PROCESS;
    brg_copy(&out->process, &_gz_po_observation.entry, sizeof(out->process));
    if(out->process.state != PO_LIVE || out->process.flags || out->process.end_frame ||
        !out->process.generation || out->process.object != actor || out->process.pid != saved->pid ||
        out->process.profile != 89 || out->process.process_name != 89 ||
        out->process.profile_entry != _gz_po_profiles[89] ||
        out->process.allocation_bytes != BRG_ACTOR_BYTES ||
        out->process.base_bytes + out->process.other_bytes != out->process.allocation_bytes)return BRG_PROCESS;
    z = brg_capture_binding(actor, BRG_ACTOR_BYTES, &out->actor);
    if(z)return z;
    if(brg_f_word(actor, 4) != saved->pid || brg_f_half(actor, 8) != 89 ||
        brg_f_half(actor, 14) != 89 || brg_f_word(actor, 16) != out->process.profile_entry ||
        brg_f_byte(actor, 12) != 2 || brg_f_word(actor, 0xb0) != saved->parameters ||
        brg_f_half(actor, 0x2d8) != saved->set_id || brg_f_word(actor, 0x2e8) != saved->parent_pid)return BRG_PROCESS;
    for(i = 0; i < 5; i++)if(brg_f_word(actor, 0x2ec + 4 * i) != saved->home[i])return BRG_PROCESS;
    out->world_index = index;
    out->count = brg_f_byte(actor, 0x544);
    out->type = brg_f_byte(actor, 0x540);
    out->action = brg_f_half(actor, 0x3b4);
    if(out->count < 1 || out->count > 50 || (out->action != 3 && out->action != 5))return BRG_LAYOUT;
    if(_gz_po_lookup(actor) || !_gz_po_observation.valid || _gz_po_observation.flags)return BRG_CHANGED;
    brg_copy(&last, &_gz_po_observation.entry, sizeof(last));
    return !brg_equal(&last, &out->process, sizeof(last)) ? BRG_CHANGED : brg_validate_binding(&out->actor);
}

static u32 brg_add(BrgWitness *owner, u32 *count, u32 p, u32 n, u32 width, u32 stride,
    u32 kind, u32 segment, u32 line, u32 *graph)
{
    BrgDescriptor *d;
    u32 i, bytes, result;
    if(*count >= BRG3_DESCRIPTOR_CAP || !n || width > stride || n > 8192 ||
        stride > 168 || n - 1 > (0xffffffffu - width) / stride)return BRG_LAYOUT;
    bytes = (n - 1) * stride + width;
    result = brg_native_span(owner, p, bytes, kind == 5 || kind == 10 ? 2 : (kind == 14 || kind == 17) ? 1 : 4, 0);
    if(result)return result;
    for(i = 0; i < *count; i++) {
        const BrgDescriptor *old = brg_descriptors + i;
        if(!brg_apart(p, bytes, old->address, (old->count - 1) * old->stride + old->width))return BRG_ALIAS;
    }
    d = brg_descriptors + *count;
    d->address = p; d->count = n; d->width = width; d->stride = stride;
    d->kind = kind; d->segment = segment; d->line = line; d->packed = 0;
    *graph = brg_hash_word(*graph, p);
    *graph = brg_hash_word(*graph, n);
    *graph = brg_hash_word(*graph, width);
    *graph = brg_hash_word(*graph, stride);
    *count += 1;
    return 0;
}

static u32 brg_render_resources(BrgWitness*,u32*);
static u32 brg_render_line(BrgWitness*,u32,u32,u32*);

static u32 brg_lines(BrgWitness *owner, u32 material, u32 segment,
    u32 lines, u32 points, u32 sizes, u32 *count, u32 *graph)
{
    u32 i, table, z;
    z = brg_native_span(owner, material, 0x188, 4, 0);
    if(z)return z;
    if(brg_f_half(material, 0x17c) != lines || brg_f_half(material, 0x17e) != points ||
        brg_f_half(material, 0x180) > points)return BRG_LAYOUT;
    table = brg_f_word(material, 0x184);
    z = brg_heap_span(owner, table, lines * 16, 4);
    if(z)return z;
    *graph = brg_hash_word(*graph, brg_f_hash(material + 0x17c, 4));
    *graph = brg_hash_word(*graph, brg_f_hash(table, lines * 16));
    for(i = 0; i < lines; i++) {
        u32 position = brg_f_word(table, i * 16), size = brg_f_word(table, i * 16 + 4);
        /* Follow only freshly bound current mesh/backing extents. Generated
         * GPU bytes/selectors are preserved, never source replay data. */
        z = brg_render_line(owner,table+i*16,points,graph);
        if(z)return z;
        z = brg_heap_span(owner, position, points * 12, 4);
        if(z)return z;
        z = brg_add(owner, count, position, 1, points * 12, points * 12,
            sizes ? 13 : 15, segment, i, graph);
        if(z)return z;
        if(sizes) {
            z = brg_heap_span(owner, size, points, 1);
            if(z)return z;
            z = brg_add(owner, count, size, 1, points, points, 14, segment, i, graph);
            if(z)return z;
        } else if(size)return BRG_LAYOUT;
    }
    return 0;
}

static u32 brg_geometry_separate(const BrgWitness *owner, u32 world, u32 mesh, u32 count)
{
    static const u32 source_strides[6] = {12,10,2,20,52,16};
    u32 i, j, actor = owner->process.object;
    for(i = 0; i < count; i++) {
        const BrgDescriptor *d = brg_descriptors + i;
        u32 bytes = (d->count - 1) * d->stride + d->width;
        if(!brg_apart(d->address, bytes, actor, 0x300) ||
            (d->kind != 17 && !brg_apart(d->address, bytes, world, 0xc4)) ||
            !brg_apart(d->address, bytes, owner->heap.heap, 0x24) ||
            !brg_apart(d->address, bytes, mesh, 48))return BRG_ALIAS;
        for(j = 0; j < 6; j++)if(!brg_apart(d->address, bytes,
            brg_f_word(mesh, j * 8 + 4), brg_f_word(mesh, j * 8) * source_strides[j]))return BRG_ALIAS;
        for(j = 0; j < owner->count; j++) {
            u32 plank = actor + 0x5a8 + j * 0x680, model = brg_f_word(plank, 0);
            u32 resource = brg_f_word(model, 0xac);
            if(!brg_apart(d->address, bytes, resource, 20))return BRG_ALIAS;
            if(!((d->kind == 11 || d->kind == 12) && d->segment == j) &&
                !brg_apart(d->address, bytes, model, 0xf8))return BRG_ALIAS;
            if(!owner->type && j >= 2 && (j - 2) % 4 == 0) {
                u32 material = plank + 0x1d4;
                if(!brg_apart(d->address, bytes, material, 0x188) ||
                    !brg_apart(d->address, bytes, brg_f_word(material, 0x184), 64))return BRG_ALIAS;
            }
        }
        if(!owner->type && (!brg_apart(d->address, bytes, actor + 0x3b8, 0x188) ||
            !brg_apart(d->address, bytes, brg_f_word(actor, 0x3b8 + 0x184), 32)))return BRG_ALIAS;
    }
    return 0;
}

static u32 brg_geometry(BrgWitness *owner, u32 output, u32 mode)
{
    static const u32 offsets[7] = {0x90,0xc0,0x88,0x8c,0x98,0x9c,0xa0};
    static const u32 counts[7] = {204,204,404,404,66,3,121};
    static const u32 widths[7] = {12,12,16,2,6,24,24};
    static const u32 strides[7] = {12,12,24,8,6,32,28};
    static const u32 source_counts[6] = {204,404,66,121,3,1};
    static const u32 source_strides[6] = {12,10,2,20,52,16};
    u32 actor = owner->process.object, heap, world, mesh, count = 0, graph = 2166136261u;
    u32 omitted = 2166136261u, i, j, z, cursor = 0;
    HlHeap last_heap;
    BgEntry last_background;
    if(!brg3_geometry_bytes(owner->type,owner->count) ||
        brg_f_byte(actor, 0x545) != owner->count ||
        brg_f_word(actor, 0x5a0))return BRG_LAYOUT;
    brg_read_count=0;
    heap = brg_f_word(actor, 0xf4);
    z = brg_heap_stamp(heap, &owner->heap);
    if(z)return z;
    world = brg_f_word(actor, 0x550);
    z = brg_heap_span(owner, world, 0xc4, 4);
    if(z)return z;
    mesh = brg_f_word(world, 0x94);
    z = brg_native_span(owner, mesh, 48, 4, 1);
    if(z)return z;
    /* The metadata query reads this exact qualified resource/header extent.
     * It does not grant general BG/resource replay or resident archive identity. */
    i = brg_f_word(world, 0);
    if(i >= BG_CAP || _gz_bg_lookup(i) || !_gz_bg_observation.valid)return BRG_CHANGED;
    brg_copy(&owner->background, &_gz_bg_observation.entry, sizeof(owner->background));
    if(owner->background.object != world || owner->background.owner != actor ||
        owner->background.pid != owner->process.pid || owner->background.state != BG_LIVE ||
        owner->background.flags || owner->background.end_frame ||
        owner->background.owner_generation != owner->process.generation ||
        owner->background.resource != mesh || brg_f_word(world, 4) != owner->background.bgw_vtable)return BRG_CHANGED;
    graph = brg_hash_word(graph, brg_f_hash_skip(world, 0xc4, 0x74, 1));
    graph = brg_hash_word(graph, brg_f_hash(mesh, 48));
    for(i = 0; i < 6; i++) {
        u32 p = brg_f_word(mesh, i * 8 + 4), bytes = source_counts[i] * source_strides[i];
        if(brg_f_word(mesh, i * 8) != source_counts[i])return BRG_LAYOUT;
        z = brg_native_span(owner, p, bytes, i == 1 || i == 2 || i == 3 ? 2 : 4, 1);
        if(z)return z;
        graph = brg_hash_word(graph, brg_f_hash(p, bytes));
    }
    for(i = 0; i < 7; i++) {
        u32 p = brg_f_word(world, offsets[i]);
        /* Pointer/vtable tails are READ only, but the complete stride still
         * needs its own current extent before those omitted bytes are hashed. */
        z = brg_heap_span(owner, p, counts[i] * strides[i], i == 4 ? 2 : 4);
        if(z)return z;
        z = brg_add(owner, &count, p, counts[i], widths[i], strides[i],
            i + 1, 0xffffffffu, 0xffffffffu, &graph);
        if(z)return z;
        for(j = 0; j < counts[i]; j++) {
            u32 k;
            for(k = widths[i]; k < strides[i]; k++)
                omitted = (omitted ^ brg_f_byte(p + j * strides[i], k)) * 16777619u;
        }
    }
    for(i = 0; i < owner->count; i++) {
        u32 plank = actor + 0x5a8 + i * 0x680;
        u32 model = brg_f_word(plank, 0), resource;
        z = brg_heap_span(owner, model, 0xf8, 4);
        if(z)return z;
        resource = brg_f_word(model, 0xac);
        z = brg_native_span(owner, resource, 20, 4, 1);
        if(z)return z;
        if(brg_f_word(plank, 0x628) || brg_f_word(plank, 0x62c))return BRG_LAYOUT;
        graph = brg_hash_word(graph, model);
        graph = brg_hash_word(graph, resource);
        graph = brg_hash_word(graph, brg_f_hash(resource, 20));
        z = brg_add(owner, &count, plank + 0x35c, 1, 72, 72, 8, i, 0xffffffffu, &graph);
        if(!z)z = brg_add(owner, &count, plank + 0x670, 1, 12, 12, 9, i, 0xffffffffu, &graph);
        if(!z)z = brg_add(owner, &count, plank + 0x67c, 1, 2, 2, 10, i, 0xffffffffu, &graph);
        if(!z)z = brg_add(owner, &count, model + 0xbc, 1, 12, 12, 11, i, 0xffffffffu, &graph);
        if(!z)z = brg_add(owner, &count, model + 0xc8, 1, 48, 48, 12, i, 0xffffffffu, &graph);
        if(z)return z;
        if(!owner->type && i >= 2 && (i - 2) % 4 == 0) {
            z = brg_lines(owner, plank + 0x1d4, i, 4, 5, 1, &count, &graph);
            if(z)return z;
        }
    }
    if(!owner->type) {
        z = brg_lines(owner, actor + 0x3b8, 0xffffffffu, 2, 14, 0, &count, &graph);
        if(!z)z = brg_add(owner, &count, actor + 0x574, 1, 4, 4, 16, 0xffffffffu, 0xffffffffu, &graph);
    } else z = 0;
    if(z)return z;
    z=brg_add(owner,&count,world+0x74,1,1,1,17,0xffffffffu,0xffffffffu,&graph);
    if(z)return z;
    if(count != brg3_descriptor_count(owner->type,owner->count))return BRG_LAYOUT;
    z = brg_geometry_separate(owner, world, mesh, count);
    if(z)return z;
    z = brg_render_resources(owner,&graph);
    if(z)return z;
    for(i=0;i<count;i++)for(j=0;j<brg_read_count;j++) {
        const BrgDescriptor *d=brg_descriptors+i;
        if(!brg_apart(d->address,(d->count-1)*d->stride+d->width,
                      brg_reads[j].address,brg_reads[j].bytes))return BRG_ALIAS;
    }
    owner->graph_hash = graph;
    owner->omitted_hash = omitted;
    owner->span_count = count;
    owner->geometry_bytes = brg3_geometry_bytes(owner->type,owner->count);
    for(i=0;i<count;i++) {
        BrgDescriptor *d=brg_descriptors+i;
        u32 envelope=(d->count-1)*d->stride+d->width;
        d->packed=cursor;
        /* OB qualifies aligned allocation extents, not each packed value.
         * A six-byte block stride has unaligned interior elements. Its whole
         * descriptor envelope was already bounded by brg_add; retain one
         * fresh binding across this byte-only loop and validate it afterward.
         * No unqualified byte is read and the OB API stays unchanged. */
        z=brg_capture_binding(d->address,envelope,0);if(z)return z;
        for(j=0;j<d->count;j++) {
            u32 p=d->address+j*d->stride;
            if(mode==1)brg_copy((void*)(output+cursor),(const void*)p,d->width);
            else if(mode==2 && !brg_equal((const void*)(output+cursor),(const void*)p,d->width))return BRG_CHANGED;
            cursor+=d->width;
        }
        z=brg_validate_binding(&brg_binding_scratch);if(z)return z;
    }
    if(cursor!=owner->geometry_bytes)return BRG_LAYOUT;
    if(mode!=3 && !brg3_geometry(output,cursor,owner->type,owner->count))return BRG_NUMERIC;
    z = brg_heap_stamp(heap, &last_heap);
    if(z || !brg_equal(&last_heap, &owner->heap, sizeof(last_heap)))return BRG_CHANGED;
    if(_gz_bg_lookup(owner->background.index) || !_gz_bg_observation.valid)return BRG_CHANGED;
    brg_copy(&last_background, &_gz_bg_observation.entry, sizeof(last_background));
    return brg_equal(&last_background, &owner->background, sizeof(last_background)) ? 0 : BRG_CHANGED;
}


/* Native DATA alias is the actual static resource-control POINTER SLOT, not a
 * value supplied by a capture or host. ROOT binds this declared alias to
 * 0x101F4F40 using the original CreateHeap/getResource operand proof. */
extern const volatile u32 _gz_brg_native_resource_control_slot;
extern const u8 _gz_brg_native_vertex_view_vtable[];

static u32 brg_record_read(u32 p,u32 bytes,u32 strict)
{
    u32 i;
    for(i=0;i<brg_read_count;i++) {
        const BrgReadRange *r=brg_reads+i;
        if(r->address==p && r->bytes==bytes && r->strict==strict)return 0;
        if(strict && r->strict && !brg_apart(p,bytes,r->address,r->bytes))return BRG_ALIAS;
    }
    if(brg_read_count==BRG_READ_CAP)return BRG_CAPACITY;
    brg_reads[brg_read_count].address=p;brg_reads[brg_read_count].bytes=bytes;
    brg_reads[brg_read_count].strict=strict;brg_read_count++;return 0;
}

static u32 brg_opaque(BrgWitness *owner,u32 p,u32 bytes,u32 *graph)
{
    u32 z=brg_native_span(owner,p,bytes,bytes<4?1:4,1);
    if(z)return z;
    z=brg_record_read(p,bytes,0);if(z)return z;
    /* The observer metadata is captured before a body is read. It remains
     * metadata; RKC/WGP supply the independent real mutator exclusion. */
    *graph=brg_hash_word(*graph,p);
    *graph=brg_hash_word(*graph,brg_f_hash((u32)&brg_binding_scratch,sizeof(brg_binding_scratch)));
    return 0;
}

/* Bounded CURRENT lookup following the original native resource tree.
 * The static slot names a freshly bound control; its nodes are embedded in
 * that exact current control extent. Every dynamic info/table/container/FRES
 * and resource extent has fresh OB before any body read.
 * No saved resource address is followed, and no renderer/resource is written. */
static u32 brg_resource(BrgWitness *owner,u32 key,const char *name,u32 index,u32 *out,u32 *graph)
{
    u32 control=_gz_brg_native_resource_control_slot,pool=control+20,storage=control+40;
    u32 count,node,steps=0,seen[4]={0,0,0,0},low=0,high=0xffffffffu,z,i;
    if(control!=_gz_pr_resource_control)return BRG_LAYOUT;
    z=brg_opaque(owner,control,0x2130,graph);if(z)return z;
    if(brg_f_word(control,0)!=(u32)_gz_lpc_native_res_control_vtable)return BRG_LAYOUT;
    count=brg_f_word(pool,12);
    if(brg_f_word(pool,8)!=storage || brg_f_word(pool,16)!=128 || !count || count>128)return BRG_LAYOUT;
    node=brg_f_word(pool,0);
    while(node) {
        u32 value,info,table,n,container,header,bytes;
        if(node<storage || node>=storage+128*32 || ((node-storage)&31) || ++steps>count)return BRG_LAYOUT;
        i=(node-storage)>>5;
        if(seen[i>>5]&(1u<<(i&31)))return BRG_LAYOUT;
        seen[i>>5]|=1u<<(i&31);
        if(brg_f_word(node,28)!=pool || brg_f_word(node,16)!=(u32)_gz_lpc_native_res_node_vtable ||
           brg_f_byte(node,8)>1)return BRG_LAYOUT;
        value=brg_f_word(node,12);
        if(value!=brg_f_word(node,20) || value<low || value>high)return BRG_LAYOUT;
        if(value==key) {
            info=brg_f_word(node,24);
            z=brg_opaque(owner,info,0x8c,graph);if(z)return z;
            if(brg_f_word(info,12)!=(u32)_gz_lpc_native_res_info_vtable ||
               brg_f_word(info,16)!=info+28 || brg_f_word(info,20)!=(u32)_gz_lpc_native_res_string_vtable ||
               brg_f_word(info,24)!=64)return BRG_LAYOUT;
            for(i=0;i<64;i++) {
                u32 c=brg_f_byte(info,28+i);
                if(c!=(u8)name[i])return BRG_LAYOUT;
                if(!c)break;
            }
            if(i==64)return BRG_LAYOUT;
            table=brg_f_word(info,96);n=brg_f_word(info,132);container=brg_f_word(info,92);
            if(!n || n>4096 || index>=n)return BRG_LAYOUT;
            z=brg_opaque(owner,table,n*4,graph);if(z)return z;
            z=brg_opaque(owner,container,0x24,graph);if(z)return z;
            header=brg_f_word(container,20);
            z=brg_opaque(owner,header,0x6c,graph);if(z)return z;
            if(brg_f_word(header,0)!=0x46524553u || brg_f_word(header,4)!=0x03040001u)return BRG_LAYOUT;
            bytes=brg_f_word(header,12);
            if(bytes<0x6c || bytes>0x4000000u)return BRG_LAYOUT;
            z=brg_opaque(owner,header,bytes,graph);if(z)return z;
            *out=brg_f_word(table,index*4);
            if(!*out)return BRG_LAYOUT;
            *graph=brg_hash_word(*graph,key);*graph=brg_hash_word(*graph,index);
            *graph=brg_hash_word(*graph,*out);
            return brg_validate_binding(&brg_binding_scratch);
        }
        if(value>key) { if(!value)return BRG_LAYOUT;high=value-1;node=brg_f_word(node,0); }
        else { if(value==0xffffffffu)return BRG_LAYOUT;low=value+1;node=brg_f_word(node,4); }
    }
    return BRG_LAYOUT;
}

/* Both generated backings are PRESERVED. Native ordinary Draw rewrites the
 * used position/tangent/normal/W/UV-Y, flushes, flips, then enqueues. Exact
 * native proof and the ROOT next-Draw acceptance remain separate receipts. */
static u32 brg_render_line(BrgWitness *owner,u32 line,u32 points,u32 *graph)
{
    u32 root=brg_f_word(line,8),z,i,j,vertices=points*2;
    z=brg_heap_span(owner,root,0x20,4);if(z)return z;
    z=brg_record_read(root,0x20,1);if(z)return z;
    if(brg_f_word(root,0)!=1 || brg_f_word(root,8)!=1 || brg_f_word(root,0x10)!=1 ||
       brg_f_word(root,0x14)!=vertices || brg_f_word(root,0x1c)>1)return BRG_LAYOUT;
    *graph=brg_hash_word(*graph,root);
    /* Selector +1C is deliberately not a source replay key. Both backings
     * and the constructor UV-X must exist regardless of its current value. */
    for(i=0;i<2;i++) {
        u32 descriptor=brg_f_word(root,i*8+4),data,bytes=vertices*0x98;
        z=brg_heap_span(owner,descriptor,0x250,4);if(z)return z;
        z=brg_record_read(descriptor,0x250,1);if(z)return z;
        data=brg_f_word(descriptor,4);
        if(brg_f_word(descriptor,0)!=vertices || brg_f_word(descriptor,0x148)!=data ||
           brg_f_word(descriptor,0x14c)!=0x98 || brg_f_word(descriptor,0x150)!=vertices ||
           brg_f_word(descriptor,0x154)!=bytes ||
           brg_f_word(descriptor,0x158)!=(u32)_gz_brg_native_vertex_view_vtable)return BRG_LAYOUT;
        z=brg_heap_span(owner,data,bytes,4);if(z)return z;
        z=brg_record_read(data,bytes,1);if(z)return z;
        *graph=brg_hash_word(*graph,descriptor);*graph=brg_hash_word(*graph,data);
        *graph=brg_hash_word(*graph,brg_f_word(descriptor,0x158));
        for(j=0;j<vertices;j++)if(brg_f_word(data+j*0x98,0x38)!=(j&1?0:0x3f800000u))return BRG_LAYOUT;
        if(i && (!brg_apart(data,bytes,brg_f_word(brg_f_word(root,4),4),bytes) ||
                  !brg_apart(descriptor,0x250,brg_f_word(root,4),0x250)))return BRG_ALIAS;
    }
    return 0;
}

static u32 brg_material_resources(BrgWitness *owner,u32 material,u32 rope,u32 *graph)
{
    u32 manager,z,i;
    /* These shader/renderer objects are not replay data. Qualify their current
     * allocation identities without following their opaque internal pointers. */
    z=brg_opaque(owner,brg_f_word(material,4),4,graph);if(z)return z;
    z=brg_opaque(owner,brg_f_word(material,8),4,graph);if(z)return z;
    manager=brg_f_word(material,12);
    z=brg_opaque(owner,manager,0x4c,graph);if(z)return z;
    for(i=0;i<2;i++) {
        u32 binding=brg_f_word(manager,i*4);
        z=brg_opaque(owner,binding,0x198,graph);if(z)return z;
        if(brg_f_word(binding,0x28)!=brg_f_word(material+(i?0xa0:0x10),0x28) ||
           brg_f_word(binding,0x30)!=brg_f_word(material+(i?0xa0:0x10),0x30))return BRG_LAYOUT;
        /* Slot1 is the current normal-map handle, retained rather than
         * replayed. Bind every non-null native image/mip owner it names. */
        if(i) {
            u32 j;
            for(j=0;j<2;j++) {
                u32 image=brg_f_word(binding,j?0x30:0x28);
                if(image) { z=brg_opaque(owner,image,1,graph);if(z)return z; }
            }
        }
    }
    z=brg_opaque(owner,rope,0x24,graph);if(z)return z;
    {
        u32 image=brg_f_word(rope,0x20),relative;
        z=brg_opaque(owner,image,0xb8,graph);if(z)return z;
        /* Current source image/mip owners only. No pixel data is followed or
         * copied and no extent is guessed from texture dimensions/format. */
        for(i=0;i<2;i++) {
            relative=brg_f_word(image,0xb0+i*4);
            if(relative) {
                u32 field=image+0xb0+i*4, resolved;
                if(relative&0x80000000u) {
                    u32 magnitude=(~relative)+1;
                    if(magnitude>field)return BRG_LAYOUT;
                    resolved=field-magnitude;
                } else {
                    if(relative>0xffffffffu-field)return BRG_LAYOUT;
                    resolved=field+relative;
                }
                z=brg_opaque(owner,resolved,1,graph);if(z)return z;
                if(resolved!=brg_f_word(material+0x10,i?0x30:0x28) ||
                   resolved!=brg_f_word(brg_f_word(manager,0),i?0x30:0x28))return BRG_LAYOUT;
            } else if(brg_f_word(material+0x10,i?0x30:0x28) ||
                      brg_f_word(brg_f_word(manager,0),i?0x30:0x28))return BRG_LAYOUT;
        }
    }
    return 0;
}

static u32 brg_render_resources(BrgWitness *owner,u32 *graph)
{
    u32 model,mesh,rope=0,z,i,actor=owner->process.object;
    z=brg_resource(owner,0x6ed40aa9u,"Bridge",owner->type?5:4,&model,graph);if(z)return z;
    z=brg_resource(owner,0x6ed40aa9u,"Bridge",9,&mesh,graph);if(z)return z;
    if(brg_f_word(brg_f_word(actor,0x550),0x94)!=mesh)return BRG_LAYOUT;
    z=brg_opaque(owner,model,20,graph);if(z)return z;
    for(i=0;i<owner->count;i++)if(brg_f_word(brg_f_word(actor,0x5a8+i*0x680),0xac)!=model)return BRG_LAYOUT;
    if(!owner->type) {
        z=brg_resource(owner,0xa108392du,"Always",126,&rope,graph);if(z)return z;
        z=brg_material_resources(owner,actor+0x3b8,rope,graph);if(z)return z;
        for(i=2;i<owner->count;i+=4) {
            z=brg_material_resources(owner,actor+0x5a8+i*0x680+0x1d4,rope,graph);if(z)return z;
        }
    }
    return 0;
}

static u32 brg_component_format(u32 p,u32 bytes)
{
    if(bytes>=64 && brg3_word(p,0)==BRG3_MAGIC)return brg3_component(p,bytes)?4:0;
    return brg_f_component(p,bytes);
}

static u32 brg_archive_format(u32 p,u32 bytes)
{
    u32 offset,length;
    if(bytes<216 || !brg3_span(p,bytes))return 0;
    offset=brg3_word(p,144);length=brg3_word(p,148);
    if(offset>bytes || length>bytes-offset)return 0;
    if(length>=64 && brg3_word(p+offset,0)==BRG3_MAGIC)return brg3_archive(p,bytes)?4:0;
    return brg_f_archive(p,bytes);
}

static u32 brg_stage_hash(void)
{
    u32 i,hash=2166136261u;
    for(i=0;i<sizeof(brg_stage);i++) {
        u32 value=(i>=4&&i<8)||(i>=52&&i<56)?0:brg_f_byte((u32)&brg_stage,i);
        hash=(hash^value)*16777619u;
    }
    return hash;
}

static u32 brg_stage_key(u32 frame)
{
    const VrWorld *world=&_gz_vws_stage;
    return brg_stage.frame==frame && brg_stage.world_hash==world->checksum &&
        brg_stage.sequence==world->sequence && brg_stage.epoch==world->epoch &&
        brg_stage.working==_gz_world_working_data &&
        brg_stage.bank==(brg_stage.working==(u32)_gz_v73_state_data?
                        (u32)_gz_world_bridges_a:(u32)_gz_world_bridges_b);
}

/* Capture and revalidation occur in genuine RKC mode1/mode2. All scalar,
 * geometry, current resource and generated-backing queries share that cut.
 * Unsupported layouts refuse explicitly; no missing row is called complete. */
u32 _gz_brg_root_read(u32 mode,u32 frame)
{
    const VrWorld *world=&_gz_vws_stage;
    u32 n=0,i,j,bytes,bank,cursor,z=BRG_CONTEXT,before_state=brg_stage.state;
    u32 header[16];
    if(brg_stage.entered) { brg_stage.state=BRG_POISON;return BRG_CONTEXT; }
    brg_stage.entered=1;
    _gz_brg_report[3]=mode;_gz_brg_report[4]=frame;_gz_brg_report[5]=BRG_CONTEXT;
    if(!brg_scope(mode,frame))goto done;
    if(mode==1 && before_state)goto done;
    if(mode==2 && (before_state!=BRG_CONSUMED || !brg_stage_key(frame) ||
       brg_stage.seal!=brg_stage_hash() || brg_stage.bank_hash!=brg3_hash(brg_stage.bank,brg_stage.bytes)))goto done;
    bank=_gz_world_working_data==(u32)_gz_v73_state_data?(u32)_gz_world_bridges_a:(u32)_gz_world_bridges_b;
    if(_gz_v73_state_valid && bank==_gz_world_committed_bridges) { z=BRG_ALIAS;goto done; }
    brg_zero(brg_current,sizeof(brg_current));
    for(i=0;i<world->count;i++)if(world->actors[i].profile==89) {
        if(n==8) { z=BRG_CAPACITY;goto done; }
        z=brg_actor(world->actors+i,i,brg_current+n);if(z)goto done;
        if(!brg3_known(brg_current[n].type,brg_current[n].count)) { z=BRG_LAYOUT;goto done; }
        n++;
    }
    bytes=64+64*n;
    for(i=0;i<n;i++) {
        bytes=(bytes+51+93*brg_current[i].count+3)&~3u;
        bytes=(bytes+brg3_geometry_bytes(brg_current[i].type,brg_current[i].count)+3)&~3u;
    }
    if(bytes>BRG3_BANK_BYTES) { z=BRG_CAPACITY;goto done; }
    if(mode==2 && (n!=brg_stage.count || bytes!=brg_stage.bytes)) { z=BRG_CHANGED;goto done; }
    if(mode==1)brg_zero((void*)bank,bytes);
    cursor=64+64*n;
    for(i=0;i<n;i++) {
        BrgWitness *row=brg_current+i;
        u32 wire[16],scalar=cursor,scalar_bytes=51+93*row->count,geometry;
        for(j=0;j<scalar_bytes;j++) {
            u32 offset=_gz_world_bridge_offsets[j],value;
            if(offset!=brg3_scalar_offset(j) || offset>=row->process.allocation_bytes) { z=BRG_LAYOUT;goto done; }
            value=brg_f_byte(row->process.object,offset);
            if(mode==1)*(u8*)(bank+scalar+j)=value;
            else if(brg3_byte(bank+scalar,j)!=value) { z=BRG_CHANGED;goto done; }
        }
        geometry=(scalar+scalar_bytes+3)&~3u;
        z=brg_geometry(row,bank+geometry,mode);if(z)goto done;
        brg_zero(wire,sizeof(wire));
        wire[0]=row->world_index;wire[1]=row->process.pid;wire[2]=row->type;wire[3]=row->count;
        wire[4]=scalar;wire[5]=scalar_bytes;wire[6]=geometry;wire[7]=row->geometry_bytes;
        wire[8]=brg3_hash(bank+scalar,scalar_bytes);wire[9]=brg3_hash(bank+geometry,row->geometry_bytes);
        wire[10]=row->graph_hash;wire[11]=row->omitted_hash;wire[12]=row->type?5:4;
        wire[13]=9;wire[14]=row->type?0:126;wire[15]=2;
        if(mode==1)brg_copy((void*)(bank+64+i*64),wire,64);
        else if(!brg_equal((const void*)(bank+64+i*64),wire,64)) { z=BRG_CHANGED;goto done; }
        row->scalar_hash=wire[8];
        z=brg_validate_binding(&row->actor);if(z)goto done;
        if(mode==2 && !brg_equal(row,brg_stage.rows+i,sizeof(*row))) { z=BRG_CHANGED;goto done; }
        cursor=(geometry+row->geometry_bytes+3)&~3u;
    }
    if(cursor!=bytes) { z=BRG_LAYOUT;goto done; }
    brg_zero(header,sizeof(header));
    header[0]=BRG3_MAGIC;header[1]=3;header[2]=64;header[3]=bytes;header[4]=n;
    header[5]=frame;header[6]=world->sequence;header[7]=world->checksum;
    header[8]=header[9]=(1u<<n)-1;header[10]=64;header[11]=64+64*n;
    header[12]=brg3_hash(bank+64,bytes-64);
    if(mode==1)brg_copy((void*)bank,header,64);
    else if(!brg_equal((const void*)bank,header,64)) { z=BRG_CHANGED;goto done; }
    if(!brg3_component(bank,bytes)) { z=BRG_FORMAT;goto done; }
    if(!brg_scope(mode,frame)) { z=BRG_CONTEXT;goto done; }
    if(mode==1) {
        brg_stage.frame=frame;brg_stage.world_hash=world->checksum;brg_stage.sequence=world->sequence;
        brg_stage.epoch=world->epoch;brg_stage.working=_gz_world_working_data;brg_stage.bank=bank;
        brg_stage.count=brg_stage.entries=n;brg_stage.coverage=(1u<<n)-1;brg_stage.bytes=bytes;
        brg_copy(brg_stage.rows,brg_current,sizeof(brg_current));
        brg_stage.bank_hash=brg3_hash(bank,bytes);brg_stage.state=BRG_CAPTURED;
    } else {
        brg_stage.state=BRG_VERIFIED;
        if(bank==(u32)_gz_world_bridges_a)brg_a_bytes=bytes;else brg_b_bytes=bytes;
    }
    brg_stage.seal=brg_stage_hash();z=0;
done:
    if(z)brg_stage.state=0;
    _gz_brg_report[5]=z;_gz_brg_report[7]=n;_gz_brg_report[8]=n;
    _gz_brg_report[9]=(1u<<n)-1;_gz_brg_report[10]=z?0:brg_stage.bytes;
    brg_stage.entered=0;return z;
}

u32 _gz_brg_consume(u32 bank)
{
    if(brg_stage.entered || brg_stage.state!=BRG_CAPTURED || bank!=brg_stage.bank ||
       brg_stage.seal!=brg_stage_hash() || !brg_stage_key(_gz_pbac_frame) ||
       brg_stage.bank_hash!=brg3_hash(bank,brg_stage.bytes) || !brg3_component(bank,brg_stage.bytes))return BRG_CONTEXT;
    _gz_world_working_bridges=bank;brg_stage.state=BRG_CONSUMED;brg_stage.seal=brg_stage_hash();return 0;
}

u32 _gz_brg_component_bytes(u32 bank)
{
    u32 bytes=bank==(u32)_gz_world_bridges_a?brg_a_bytes:bank==(u32)_gz_world_bridges_b?brg_b_bytes:0;
    return bytes && brg_component_format(bank,bytes)?bytes:0;
}

u32 _gz_brg_parse_component(u32 address,u32 bytes)
{
    return brg_component_format(address,bytes);
}

u32 _gz_brg_install_note(u32 bank,u32 bytes)
{
    u32 pin=_gz_osa_pin_id(),source,used,offset;
    if(brg_stage.entered || bank!=(u32)_gz_world_bridges_a || !pin || _gz_osa_validate(pin))return BRG_CONTEXT;
    source=_gz_osa_source(pin);used=_gz_osa_bytes(pin);
    if(!source || used<256 || brg3_word(source,148)!=bytes)return BRG_FORMAT;
    offset=brg3_word(source,144);
    if(offset>used || bytes>used-offset || !brg_component_format(source+offset,bytes) ||
       !brg_equal((const void*)bank,(const void*)(source+offset),bytes) || _gz_osa_validate(pin))return BRG_FORMAT;
    brg_a_bytes=bytes;return 0;
}

void _gz_brg_discard(void)
{
    if(!brg_stage.entered)brg_zero(&brg_stage,sizeof(brg_stage));
}
