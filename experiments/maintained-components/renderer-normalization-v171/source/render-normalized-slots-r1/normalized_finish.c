#include "normalized_slot_registry.h"
/* Existing native layouts only. These declarations allocate no owner. */
#include "workspace_overlay_private.h"
#include "owned_heap_held_view.h"
#include "owned_heap_dual_cut.h"
#include "owned_heap_cut_policy.h"
typedef unsigned int U;
typedef unsigned char B;
typedef WsRegion Region;
typedef WsSlot Slot;
typedef WsState State;
typedef WsHeap OisHeap;
typedef WsCut OisCurrentCut;
_Static_assert(GHC_PROCESS_VERSION==10,"Actual native wire ABI10 required");
_Static_assert(sizeof(State)==2420 && sizeof(Slot)==80 && sizeof(Region)==16,"Genuine owner ABI");
_Static_assert(sizeof(OisHeap)==40 && sizeof(OisCurrentCut)==128 && sizeof(OisHeldView)==176,"Genuine private ABI");
extern State _gz_ois_mem_state;
extern OisHeap _gz_ois_mem_heap;
extern OisCurrentCut _gz_ois_mem_cut;
extern U _gz_ois_mem_retained;
extern OisHeldView _gz_ois_mem_held;
#define state _gz_ois_mem_state
#define ois_heap _gz_ois_mem_heap
#define ois_cut _gz_ois_mem_cut
#define retained_conflict _gz_ois_mem_retained
#define _gz_ois_held_record _gz_ois_mem_held
extern U _gz_ois_mem_header(void),_gz_ois_mem_cut_header(void);
extern U _gz_ois_mem_region_ok(const Region*),_gz_ois_mem_slot_ok(U);
extern U _gz_ois_mem_heap_lock(U,U,U),_gz_ois_mem_heap_owner(U,U);
extern U _gz_ois_mem_image_identity(void),_gz_ois_mem_abandon(U);
extern U _gz_ois_mem_allocate_part0(Region*,U),_gz_ois_mem_external(U,U);
#include "mapped_ois_backend.h"
#include "split_workspace.h"
#include "mapped_storage_policy.h"
extern U _gz_ois_mm_capacity(U,U,U);
extern U _gz_ois_mem_lock(void);
extern U _gz_ois_mem_pin_acquire(U,OisLease*),_gz_ois_mem_cut_release_saved(void);
extern U _gz_ghs_role(U,U),_gz_ghs_target(U,U),_gz_ghs_image_version(U,U);
extern void _gz_ois_mem_release(Region*);
extern U _gz_gac_heap_scope(U,U,U),_gz_ghs_scope(U,U,U);
extern void *MEMGetBaseHeapHandle(U),*MEMAllocFromExpHeapEx(void*,U,int);
extern void MEMFreeToExpHeap(void*,void*);
extern U _gz_rsa_private_apart(U,U),_gz_hr_storage_private(const void*,U);
#define header _gz_ois_mem_header
#define ois_cut_header _gz_ois_mem_cut_header
#define region_ok _gz_ois_mem_region_ok
#define slot_ok _gz_ois_mem_slot_ok
#define ois_heap_lock _gz_ois_mem_heap_lock
#define ois_heap_owner _gz_ois_mem_heap_owner
#define ois_heap_image_identity _gz_ois_mem_image_identity
#define ois_heap_abandon _gz_ois_split_abandon
#define release_region _gz_ois_mm_release
#define external _gz_ois_mem_external
enum {OIS_CUT_EMPTY=0,OIS_CUT_RESERVED=1,OIS_CUT_PUBLISHED=2,OIS_CUT_RETIRING=4,OIS_CUT_RETAINED=5};
static void copy(void*d,const void*s,U n){U i;for(i=0;i<n;i++)((B*)d)[i]=((const B*)s)[i];}
static void zero(void*d,U n){U i;for(i=0;i<n;i++)((B*)d)[i]=0;}
static U same(const void*a,const void*b,U n){U i;for(i=0;i<n;i++)if(((const B*)a)[i]!=((const B*)b)[i])return 0;return 1;}
static U issue(U*out){if(state.serial==0xffffffffu)return OIS_EXHAUSTED;*out=++state.serial;state.inverse=~state.serial;return 0;}
static U done(U z){state.writer=0;state.thread=0;__sync_synchronize();state.entered=0;return z;}
static U ois_heap_scope(U request, U frame, U phase)
{
    if (ois_heap.purpose == GHC_VERSION)
        return _gz_gac_heap_scope(request, frame, phase);
    if (ois_heap.purpose == GHS_VERSION)
        return _gz_ghs_scope(request, frame, phase);
    return 0;
}

static void ois_held_invalidate(void)
{
    _gz_ois_held_record.valid = 0;
    __sync_synchronize();
}
static U ois_cut_publish(U request, U frame, U used)
{
    U token, result;
    if (ois_cut.phase != OIS_CUT_RESERVED || ois_cut.request != request ||
        ois_cut.frame != frame || ois_cut.thread != state.thread ||
        !ois_cut_header() || !region_ok(&state.staging) || !state.staging.pointer ||
        used < 192 || used > state.staging.capacity || (used & 3) ||
        ois_cut.generation == 0xffffffffu) return GHC_RETAINED;
    result = issue(&token);
    if (result) return result;
    copy(&ois_cut.current, &state.staging, sizeof(Region));
    zero(&state.staging, sizeof(state.staging));
    ois_cut.used = used;
    ++ois_cut.generation;
    ois_cut.token = token;
    __sync_synchronize();
    ois_cut.phase = OIS_CUT_PUBLISHED;
    return OIS_OK;
}
static U ois_cut_pending(void)
{
    return ois_cut.phase != OIS_CUT_EMPTY;
}
static void ois_cut_clear(void)
{
    U generation = ois_cut.generation;
    zero(&ois_cut, sizeof(ois_cut));
    ois_cut.generation = generation;
}
static U ois_cut_begin(U request, U frame, U target)
{
    U result;
    const GhcSaveProducer *saved_producer;
    if (ois_cut_pending() || target >= OIS_SLOTS || !slot_ok(target) ||
        state.slots[target].type != GHC_PROCESS_VERSION) return OIS_TYPE;
    if (ois_cut.generation == 0xffffffffu) return OIS_EXHAUSTED;
    result = _gz_ois_mem_pin_acquire(target, &ois_cut.saved);
    if (result) return result;
    ois_cut.request = request;
    ois_cut.frame = frame;
    ois_cut.thread = state.thread;
    ois_cut.slot = target;
    ois_cut.phase = OIS_CUT_RESERVED;
    result = _gz_ghc_validate((const void *)ois_cut.saved.archive,
        ois_cut.saved.bytes, 1);
    if (!result) {
        saved_producer = (const GhcSaveProducer *)(ois_cut.saved.archive + 128u);
        /* A saved user cut came from a genuine Save, never a current cut.
         * Legacy HSPv1 was Save-only. All new HSPv2 cuts carry command1/2. */
        if (!ois_cut_saved_policy(saved_producer, request, frame, target))
            result = GHC_CHANGED;
    }
    if (!result) return OIS_OK;
    ois_cut.phase = OIS_CUT_RETIRING;
    if (_gz_ois_mem_cut_release_saved()) {
        ois_cut.phase = OIS_CUT_RETAINED;
        return GHC_RETAINED;
    }
    ois_cut_clear();
    return result;
}
static U allocate_region(Region*r,U cap){return _gz_ois_mm_stage_allocate(r,cap);}
/* Volatile reads make this an actual destination/source readback after copy,
 * including with the production optimizer; ordinary metadata same() remains. */
static U ois_heap_clone_equal(const void *a, const void *b, U bytes)
{
    const volatile B *left = a, *right = b;
    U i;
    for (i = 0; i < bytes; ++i) if (left[i] != right[i]) return 0;
    return 1;
}
static U ois_heap_compact(Slot *target)
{
    Region tight = {0}, old;
    Slot before;
    U result, needed = (ois_heap.used + 31u) & ~31u;
    if (target != state.slots + ois_heap.target || !header() ||
        state.entered != 1 || state.writer || state.thread != ois_heap.thread ||
        !ois_heap_scope(ois_heap.request, ois_heap.frame, 3) ||
        !slot_ok(ois_heap.target) || target->readers ||
        target->used != ois_heap.used || target->type != GHC_PROCESS_VERSION ||
        !needed || needed > target->region.capacity) {
        retained_conflict = 1;
        return GHC_RETAINED;
    }
    if (needed == target->region.capacity) return OIS_OK;
    copy(&before, target, sizeof(before));
    if (needed > GZ_MM_SAVED_REQUEST_LIMIT) return OIS_SPACE;
    result = _gz_ois_mm_video_allocate((WsRegion *)&tight, needed);
    if (result) return retained_conflict ? GHC_RETAINED : result;
    copy((void *)tight.pointer, (const void *)before.region.pointer, before.used);
    /* Source is already validated and immutable under the actual owner.
     * Verify every copied byte without a third complete packed decode. */
    if (!ois_heap_clone_equal((const void *)tight.pointer,
            (const void *)before.region.pointer, before.used)) {
        release_region(&tight);
        return GHC_FORMAT;
    }
    if (!header() || state.entered != 1 || state.writer ||
        state.thread != ois_heap.thread ||
        !ois_heap_scope(ois_heap.request, ois_heap.frame, 3) ||
        !slot_ok(ois_heap.target) || !same(target, &before, sizeof(before))) {
        release_region(&tight);
        retained_conflict = 1;
        return GHC_RETAINED;
    }
    /* The externally visible slot generation/used/type/domain do not change.
     * No pin was issued while entered. The genuine new allocation carries its
     * fresh allocation identity; the earlier complete Region is then retired.
     */
    copy(&old, &target->region, sizeof(old));
    copy(&target->region, &tight, sizeof(tight));
    zero(&tight, sizeof(tight));
    release_region(&old);
    return OIS_OK;
}
U _gz_rns_mm_finish(U request, U frame, U success)
{
    U result;
    Slot old, *target;
    if (!ois_heap.phase) return state.staging.pointer ? GHC_RETAINED : OIS_OK;
    if (!ois_heap_scope(request, frame, 3)) return GHC_RETAINED;
    result = ois_heap_lock(request, frame, 3);
    if (result) return result;
    if (!ois_heap_owner(request, frame) || ois_heap.phase == 2 || ois_heap.phase == 4)
        return GHC_RETAINED;
    target = state.slots + ois_heap.target;
    if (!slot_ok(ois_heap.target)) return GHC_RETAINED;
    if (ois_heap.role == OIS_CUT_SAVED &&
        (target->readers || target->generation == 0xffffffffu)) return GHC_RETAINED;
    if (success && !ois_heap.result && ois_heap.used) {
        result = ois_heap_image_identity() &&
            ((const GhcHeader *)state.staging.pointer)->version == ois_heap.image_version ?
            _gz_ghc_validate((const void *)state.staging.pointer, ois_heap.used, 0) : GHC_FORMAT;
        if (result) {
            success = 0;
            ois_heap.result = result;
        }
    } else success = 0;
    if (!success) return ois_heap_abandon(ois_heap.result);

    
    ois_held_invalidate();
    ((GhcHeader *)state.staging.pointer)->valid = 1;
    __sync_synchronize();
    result = _gz_ghc_validate((const void *)state.staging.pointer, ois_heap.used, 1);
    if (result) return ois_heap_abandon(result);
    if (ois_heap.role == OIS_CUT_CURRENT) {
        /* The held writer has returned and its genuine world/job hold has
         * released. Retire only its SDK workspace before CURRENT becomes a
         * post-cleanup pair. A dirty/retained writer never reaches here. */
        result = _gz_ois_sdk_work_release();
        if (result) { ois_heap.phase = 4; return GHC_RETAINED; }
        result = ois_cut_publish(request, frame, ois_heap.used);
        if (result == GHC_RETAINED) {
            ois_heap.phase = 4;
            return result;
        }
        if (result) return ois_heap_abandon(result);
        _gz_ghc_report[23] = ois_cut.current.pointer;
        _gz_ghc_report[24] = ois_cut.current.capacity;
        _gz_ghc_report[25] = ois_cut.used;
        _gz_ghc_report[26] = ois_cut.current.identity;
        _gz_ghc_report[27] = ois_cut.generation;
        _gz_ghc_report[28] = GHC_PROCESS_VERSION;
    } else {
        copy(&old, target, sizeof(old));
        copy(&target->region, &state.staging, sizeof(target->region));
        zero(&state.staging, sizeof(state.staging));
        target->used = ois_heap.used;
        target->type = ois_heap.purpose == GHS_VERSION ? GHC_PROCESS_VERSION : GHC_VERSION;
        ++target->generation;
        zero(&target->domain, sizeof(target->domain));
        _gz_ghc_report[23] = target->region.pointer;
        _gz_ghc_report[24] = target->region.capacity;
        _gz_ghc_report[25] = target->used;
        _gz_ghc_report[26] = target->region.identity;
        _gz_ghc_report[27] = target->generation;
        _gz_ghc_report[28] = target->type;
        release_region(&old.region);
    }
    if (ois_heap.purpose == GHS_VERSION && ois_heap.image_version == GHC_PROCESS_VERSION &&
        ois_heap.role == OIS_CUT_SAVED) {
        result = ois_heap_compact(target);
        if (result == GHC_RETAINED) { ois_heap.phase = 4; return result; }
        _gz_ghc_report[23] = target->region.pointer;
        _gz_ghc_report[24] = target->region.capacity;
        _gz_ghc_report[25] = target->used;
        _gz_ghc_report[26] = target->region.identity;
        _gz_ghc_report[27] = target->generation;
        _gz_ghc_report[28] = target->type;
    }
    /* The authentic slot and final compacted Region now exist, while the
     * original OIS owner remains entered and GHS is genuinely RELEASED.
     * This hook publishes only the private normalization/lifetime witness.
     * A refused seal leaves the new archive unsealed and reports failure;
     * it does not turn an old image into a normalized save. */
    result = OIS_OK;
    if (ois_heap.purpose == GHS_VERSION &&
        ois_heap.image_version == GHC_PROCESS_VERSION &&
        ois_heap.role == OIS_CUT_SAVED)
        result = _gz_rns_publish(request, frame);
    if (result == GHC_RETAINED) { ois_heap.phase = 4; return result; }
    zero(&ois_heap, sizeof(ois_heap));
    result = done(result);
    __sync_synchronize();
    _gz_ghc_report[29] = result ? 0 : 1;
    return result;
}
U _gz_rns_unused_mm_reserve(U request, U frame, U capacity)
{
    U result, purpose, role = OIS_CUT_SAVED, target = 9, image_version = GHC_VERSION;
    result = _gz_ois_mem_lock();
    if (result) return result;
    purpose = _gz_gac_heap_scope(request, frame, 1) ? GHC_VERSION :
        _gz_ghs_scope(request, frame, 1) ? GHS_VERSION : 0;
    if (purpose == GHS_VERSION) {
        role = _gz_ghs_role(request, frame);
        target = _gz_ghs_target(request, frame);
        image_version = _gz_ghs_image_version(request, frame);
        if (!ois_cut_reservation_policy(role, target, OIS_SLOTS,
                image_version, GHC_PROCESS_VERSION))
            return done(GHC_CONTEXT);
        --target;
    }
    if (ois_heap.phase || !purpose || state.tx.phase || state.staging.pointer ||
        !slot_ok(target)) return done(OIS_BUSY);
    if (role == OIS_CUT_SAVED && state.slots[target].readers) return done(OIS_BUSY);
    if (role == OIS_CUT_SAVED && state.slots[target].generation == 0xffffffffu)
        return done(OIS_EXHAUSTED);

    ois_heap.phase = 1;
    ois_heap.request = request;
    ois_heap.frame = frame;
    ois_heap.purpose = purpose;
    ois_heap.thread = state.thread;
    ois_heap.role = role;
    ois_heap.target = target;
    ois_heap.image_version = image_version;
    if (role == OIS_CUT_CURRENT) {
        result = ois_cut_begin(request, frame, target);
        if (result) {
            if (result == GHC_RETAINED) {
                ois_heap.phase = 4;
                return GHC_RETAINED;
            }
            zero(&ois_heap, sizeof(ois_heap));
            return done(result);
        }
    }
    if (purpose == GHS_VERSION && image_version == GHC_PROCESS_VERSION) {
        capacity = _gz_ois_mm_capacity(request, frame, capacity);
        if (capacity < 192) {
            if (retained_conflict) { ois_heap.phase=4; return GHC_RETAINED; }
            return ois_heap_abandon(OIS_SPACE);
        }
    }
    if (role == OIS_CUT_CURRENT) {
        /* Capacity planning placed the exact page+HMD envelope in the
         * existing pre-capture used word. Read replaces it with wire bytes. */
        result = _gz_ois_sdk_work_allocate(ois_heap.used);
        if (result) {
            if (retained_conflict) { ois_heap.phase=4; return GHC_RETAINED; }
            return ois_heap_abandon(result);
        }
    }
    result = allocate_region(&state.staging, capacity);
    if (result) {
        if (retained_conflict) {
            /* The rejected address may belong to somebody else. Original
             * fail-retained policy and saved read ownership both remain. */
            ois_heap.phase = 4;
            return GHC_RETAINED;
        }
        return ois_heap_abandon(result);
    }
    zero((void *)state.staging.pointer, 128);
    _gz_ghc_report[8] = state.staging.pointer;
    _gz_ghc_report[9] = state.staging.capacity;
    return done(OIS_OK);
}
